/*
 * Copyright (C) 2018-2023 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * $RP_BEGIN_LICENSE$
 * Commercial License Usage
 *  Licensees holding valid commercial IoT.bzh licenses may use this file in
 *  accordance with the commercial license agreement provided with the
 *  Software or, alternatively, in accordance with the terms contained in
 *  a written agreement between you and The IoT.bzh Company. For licensing terms
 *  and conditions see https://www.iot.bzh/terms-conditions. For further
 *  information use the contact form at https://www.iot.bzh/contact.
 *
 * GNU General Public License Usage
 *  Alternatively, this file may be used under the terms of the GNU General
 *  Public license version 3. This license is as published by the Free Software
 *  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
 *  of this file. Please review the following information to ensure the GNU
 *  General Public License requirements will be met
 *  https://www.gnu.org/licenses/gpl-3.0.html.
 * $RP_END_LICENSE$
 */

#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sizes.h"

#if WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include "sec-lsm-manager-protocol.h"
#include "sec-lsm-manager-server.h"

#if !defined(SYSTEMD_NAME)
#define SYSTEMD_NAME "sec-lsm-manager"
#endif

#if !defined(SYSTEMD_SOCKET)
#define SYSTEMD_SOCKET "sd:" SYSTEMD_NAME
#endif

#if !defined(SHUTOFF_TIME)
#define SHUTOFF_TIME (60 * 3) /* 3 minutes */
#endif

#if !defined(SUPL_GROUPS_MAX)
#define SUPL_GROUPS_MAX 10
#endif

#define DELIM_GROUPS ", "

#define CAP_COUNT (sizeof cap_vector / sizeof cap_vector[0])

#define _GROUP_ 'g'
#define _GROUPS_ 'G'
#define _HELP_ 'h'
#define _KEEPGOING_ 'k'
#define _LOG_ 'l'
#define _MAKESOCKDIR_ 'M'
#define _OWNSOCKDIR_ 'O'
#define _OWNDBDIR_ 'o'
#define _SOCKETDIR_ 'S'
#define _SHUTOFF_ 's'
#define _USER_ 'u'
#define _VERSION_ 'v'

static const char shortopts[] = "d:g:hi:klmMOoS:s:u:v";

static const struct option longopts[] = {{"group", 1, NULL, _GROUP_},
                                         {"groups", 1, NULL, _GROUPS_},
                                         {"help", 0, NULL, _HELP_},
                                         {"keep-going", 0, NULL, _KEEPGOING_},
                                         {"log", 0, NULL, _LOG_},
                                         {"make-socket-dir", 0, NULL, _MAKESOCKDIR_},
                                         {"own-socket-dir", 0, NULL, _OWNSOCKDIR_},
                                         {"shutoff", 1, NULL, _SHUTOFF_ },
                                         {"socketdir", 1, NULL, _SOCKETDIR_},
                                         {"user", 1, NULL, _USER_},
                                         {"version", 0, NULL, _VERSION_},
                                         {NULL, 0, NULL, 0}};

static const char helptxt[] =
    "\n"
    "usage: sec-lsm-managerd [options]...\n"
    "\n"
    "otpions:\n"
    "    -u, --user xxx        set the user\n"
    "    -g, --group xxx       set the group\n"
    "    -G  --groups xxx,yyy  set additional groups\n"
    "    -l, --log             activate log of transactions\n"
    "    -k, --keep-going      continue to run on some errors\n"
    "    -s, --shutoff VALUE   shutting off time in seconds\n"
    "\n"
    "    -S, --socketdir xxx   set the base directory xxx for sockets\n"
    "                            (default: %s)\n"
    "    -M, --make-socket-dir make the socket directory\n"
    "    -O, --own-socket-dir  set user and group on socket directory\n"
    "\n"
    "    -h, --help            print this help and exit\n"
    "    -v, --version         print the version and exit\n"
    "\n";

static const char versiontxt[] = "sec-lsm-managerd version " VERSION;

static int isid(const char *text);
static int ensure_directory(const char *path, int uid, int gid);

int main(int ac, char **av) {
    int opt;
    int rc;
    int makesockdir = 0;
    int ownsockdir = 0;
    int flog = 0;
    int keepgoing = 0;
    int help = 0;
    int version = 0;
    int error = 0;
    int uid = -1;
    int gid = -1;
    int g;
    int soff = SHUTOFF_TIME;
    const char *shutoff = NULL;
    const char *socketdir = NULL;
    const char *user = NULL;
    const char *group = NULL;
    char *groups = NULL;
    char *curg = NULL;
    char *nxtg = NULL;
    struct passwd *pw;
    struct group *gr;
    sec_lsm_manager_server_t *server;
    char *spec_socket;
    gid_t gids[SUPL_GROUPS_MAX] = {0};
    size_t number_groups = 0;
    cap_value_t cap_vector[] = {CAP_MAC_ADMIN,       CAP_DAC_OVERRIDE, CAP_MAC_OVERRIDE, CAP_SYS_ADMIN,
                                CAP_DAC_READ_SEARCH, CAP_SETFCAP,      CAP_FOWNER};
    cap_t cap = {0};

    setlinebuf(stdout);
    setlinebuf(stderr);

    /* scan arguments */
    for (;;) {
        opt = getopt_long(ac, av, shortopts, longopts, NULL);
        if (opt == -1)
            break;

        switch (opt) {
            case _GROUP_:
                group = optarg;
                break;
            case _GROUPS_:
                groups = optarg;
                break;
            case _HELP_:
                help = 1;
                break;
            case _KEEPGOING_:
                keepgoing = 1;
                break;
            case _LOG_:
                flog = 1;
                break;
            case _MAKESOCKDIR_:
                makesockdir = 1;
                break;
            case _OWNSOCKDIR_:
                ownsockdir = 1;
                break;
            case _SHUTOFF_:
                shutoff = optarg;
                break;
            case _SOCKETDIR_:
                socketdir = optarg;
                break;
            case _USER_:
                user = optarg;
                break;
            case _VERSION_:
                version = 1;
                break;
            default:
                error = 1;
                break;
        }
    }

    /* handles help, version, error */
    if (help) {
        fprintf(stdout, helptxt, sec_lsm_manager_default_socket_dir);
        return 0;
    }
    if (version) {
        puts(versiontxt);
        return 0;
    }
    if (error)
        return EXIT_FAILURE;

    /* set the defaults */
    if (socketdir == NULL)
        socketdir = sec_lsm_manager_default_socket_dir;

    /* compute shutoff delay */
    if (shutoff != NULL) {
        if (!strcmp(shutoff, "never"))
            soff = -1;
        else {
            soff = isid(&shutoff[shutoff[0] == '-']);
            if (soff < 0) {
                fprintf(stderr, "not a valid shutoff '%s'\n", shutoff);
                return EXIT_FAILURE;
            }
            if (shutoff[0] == '-')
                soff = -1;
        }
    }

    /* compute socket specs */
    spec_socket = 0;
#if WITH_SYSTEMD
    {
        char **names = 0;
        rc = sd_listen_fds_with_names(0, &names);
        if (rc >= 0 && names) {
            for (rc = 0; names[rc]; rc++) {
                if (!strcmp(names[rc], SYSTEMD_NAME))
                    spec_socket = strdup(SYSTEMD_SOCKET);
                free(names[rc]);
            }
            free(names);
        }
    }
#endif
    if (!spec_socket)
        rc = asprintf(&spec_socket, "%s:%s/%s", sec_lsm_manager_default_socket_scheme, socketdir,
                      sec_lsm_manager_default_socket_name);
    if (!spec_socket) {
        fprintf(stderr, "can't make socket paths\n");
        return EXIT_FAILURE;
    }

    /* compute user and group */
    if (user) {
        uid = isid(user);
        if (uid < 0) {
            pw = getpwnam(user);
            if (pw == NULL) {
                fprintf(stderr, "can not find user '%s'\n", user);
                return EXIT_FAILURE;
            }
            uid = (int)pw->pw_uid;
            gid = (int)pw->pw_gid;
        }
    }
    if (group) {
        gid = isid(group);
        if (gid < 0) {
            gr = getgrnam(group);
            if (gr == NULL) {
                fprintf(stderr, "can not find group '%s'\n", group);
                return EXIT_FAILURE;
            }
            gid = (int)gr->gr_gid;
        }
    }
    if (groups) {
        curg = &groups[strspn(groups, DELIM_GROUPS)];
        while (curg[0] != '\0' && number_groups < SUPL_GROUPS_MAX) {
            nxtg = &curg[strcspn(curg, DELIM_GROUPS)];
            if (nxtg[0] != '\0')
                (nxtg++)[0] = '\0';
            g = isid(curg);
            if (g < 0) {
                gr = getgrnam(curg);
                if (gr == NULL) {
                    fprintf(stderr, "can not find group '%s'\n", curg);
                    return EXIT_FAILURE;
                }
                g = (int)gr->gr_gid;
            }
            gids[number_groups++] = (gid_t)g;
            curg = &nxtg[strspn(nxtg, DELIM_GROUPS)];        }
    }

    /* handle directories */
    if (makesockdir && socketdir[0] != '@') {
        if (ensure_directory(socketdir, ownsockdir ? uid : -1, ownsockdir ? gid : -1) < 0) {
            fprintf(stderr, "error : ensure_directory");
            return EXIT_FAILURE;
        }
    }

    // set flag to keep caps after setuid
    prctl(PR_SET_KEEPCAPS, 1);

    /* drop privileges */
    if (groups != NULL) {
        rc = setgroups(number_groups, gids);
        if (rc < 0) {
            fprintf(stderr, "can not change groups: %s\n", strerror(errno));
            if (!keepgoing)
                return EXIT_FAILURE;
        }
    }
    if (gid >= 0) {
        rc = setgid((gid_t)gid);
        if (rc < 0) {
            fprintf(stderr, "can not change group: %s\n", strerror(errno));
            if (!keepgoing)
                return EXIT_FAILURE;
        }
    }
    if (uid >= 0) {
        rc = setuid((uid_t)uid);
        if (rc < 0) {
            fprintf(stderr, "can not change user: %s\n", strerror(errno));
            if (!keepgoing)
                return EXIT_FAILURE;
        }
    }

    // set capabilities
    cap = cap_init();
    cap_set_flag(cap, CAP_EFFECTIVE, CAP_COUNT, cap_vector, CAP_SET);
    cap_set_flag(cap, CAP_PERMITTED, CAP_COUNT, cap_vector, CAP_SET);
    cap_set_flag(cap, CAP_INHERITABLE, CAP_COUNT, cap_vector, CAP_SET);

    if (cap_set_proc(cap) != 0) {
        fprintf(stderr, "can not change cap: %s\n", strerror(errno));
        if (!keepgoing)
            return EXIT_FAILURE;
    }

    for (size_t i = 0; i < CAP_COUNT; i++) {
        if (cap_set_ambient(cap_vector[i], CAP_SET) != 0) {
            fprintf(stderr, "can not change cap amb: %s\n", strerror(errno));
            if (!keepgoing)
                return EXIT_FAILURE;
        }
    }

    // unset flag
    prctl(PR_SET_KEEPCAPS, 0);

    /* initialize server */
    setvbuf(stderr, NULL, _IOLBF, 1000);
    sec_lsm_manager_server_log = (bool)flog;

#if DEBUG_MODE
    puts("DEBUG_MODE = 1");
#endif

    signal(SIGPIPE, SIG_IGN); /* avoid SIGPIPE! */
    rc = sec_lsm_manager_server_create(&server, spec_socket);
    if (rc < 0) {
        fprintf(stderr, "can't initialize server: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    /* ready ! */
#if WITH_SYSTEMD
    sd_notify(0, "READY=1");
#endif

    /* serve */
    rc = sec_lsm_manager_server_serve(server, soff);
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

/** returns the value of the id for 'text' (positive) or a negative value (-1) */
static int isid(const char *text) {
    long long int value = 0;
    while (*text && value < INT_MAX)
        if (*text < '0' || *text > '9' || value >= INT_MAX)
            return -1;
        else
            value = 10 * value + (*text++ - '0');
    return value <= INT_MAX ? (int)value : -1;
}

/** returns a pointer to the first last / of the path if it is meaningful */
static char *enddir(char *path) {
    /*
     * /       -> NULL
     * /xxx    -> NULL
     * /xxx/   -> NULL
     * /xxx/y  -> /y
     * /xxx//y -> //y
     */
    char *c = NULL, *r = NULL, *i = path;
    for (;;) {
        while (*i == '/') i++;
        if (*i)
            r = c;
        while (*i != '/')
            if (!*i++)
                return r;
        c = i;
    }
}

/** ensure that 'path' is a directory for the user and group */
static int ensuredir(char *path, int length, int uid, int gid) {
    struct stat st;
    int rc, n;
    char *e;

    n = length;
    for (;;) {
        path[n] = 0;
        rc = mkdir(path, 0755);
        if (rc == 0 || errno == EEXIST) {
            /* exists */
            if (n == length) {
                rc = stat(path, &st);
                if (rc < 0) {
                    fprintf(stderr, "can not check %s: %s\n", path, strerror(errno));
                    return -1;
                } else if ((st.st_mode & S_IFMT) != S_IFDIR) {
                    fprintf(stderr, "not a directory %s: %s\n", path, strerror(errno));
                    return -1;
                }
                /* set ownership */
                if (((uid_t)uid != st.st_uid && uid >= 0) || ((gid_t)gid != st.st_gid && gid >= 0)) {
                    rc = chown(path, (uid_t)uid, (gid_t)gid);
                    if (rc < 0) {
                        fprintf(stderr, "can not own directory %s for uid=%d & gid=%d: %s\n", path, uid, gid, strerror(errno));
                        return -1;
                    }
                }
                return 0;
            }
            path[n] = '/';
            n = (int)strlen(path);
        } else if (errno == ENOENT) {
            /* a part of the path doesn't exist, try to create it */
            e = enddir(path);
            if (!e) {
                /* can't create it because at root */
                fprintf(stderr, "can not ensure directory %s\n", path);
                return -1;
            }
            n = (int)(e - path);
        } else {
            fprintf(stderr, "can not ensure directory %s: %s\n", path, strerror(errno));
            return -1;
        }
    }
    return 0;
}

/** ensure that 'path' is a directory for the user and group */
static int ensure_directory(const char *path, int uid, int gid) {
    size_t l;
    char *p;

    l = strlen(path);
    if (l > INT_MAX) {
        /* ?!?!?!? *#@! */
        fprintf(stderr, "path toooooo long (%s)\n", path);
        return -1;
    }
    p = strndupa(path, l);
    return ensuredir(p, (int)l, uid, gid);
}
