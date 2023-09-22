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
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "log.h"
#include "sec-lsm-manager.h"
#include "utils.h"

#define _ECHO_ 'e'
#define _HELP_ 'h'
#define _KEEP_GOING_ 'k'
#define _SOCKET_ 's'
#define _VERSION_ 'v'

static const char shortopts[] = "c:ehks:v";

static const struct option longopts[] = {{"echo", 0, NULL, _ECHO_},
                                         {"help", 0, NULL, _HELP_},
                                         {"keep-going", 0, NULL, _KEEP_GOING_},
                                         {"socket", 1, NULL, _SOCKET_},
                                         {"version", 0, NULL, _VERSION_},
                                         {NULL, 0, NULL, 0}};

static const char helptxt[] =
    "\n"
    "usage: sec-lsm-manager-cmd [options]... [action [arguments]]...\n"
    "\n"
    "otpions:\n"
    "    -s, --socket xxx      set the base xxx for sockets\n"
    "    -e, --echo            print the evaluated command\n"
    "    -h, --help            print this help and exit\n"
    "    -v, --version         print the version and exit\n"
    "    -k, --keep-going      don't stop on error if actions given\n"
    "\n"
    "When action is given, sec-lsm-manager-cmd performs the action and exits.\n"
    "Otherwise sec-lsm-manager-cmd continuously read its input to get the actions.\n"
    "For a list of actions type 'sec-lsm-manager-cmd help'.\n"
    "\n";

static const char versiontxt[] = "sec-lsm-manager-cmd version " VERSION;

static const char help_log_text[] =
    "\n"
    "Command: log [on|off]\n"
    "\n"
    "With the 'on' or 'off' arguments, set the logging state to what required.\n"
    "In all cases, prints the logging state.\n"
    "\n"
    "Examples:\n"
    "\n"
    "  log on                  activates the logging\n"
    "\n";

static const char help_clear_text[] =
    "\n"
    "Command: clear\n"
    "\n"
    "Clear the actual handle of application\n"
    "\n";

static const char help_display_text[] =
    "\n"
    "Command: display\n"
    "\n"
    "Display current state\n"
    "\n";

static const char help_id_text[] =
    "\n"
    "Command: id app_id\n"
    "\n"
    "Set the id of the application\n"
    "\n"
    "Example : id agl-service-can-low-level\n"
    "\n";

static const char help_path_text[] =
    "\n"
    "Command: path path path_type\n"
    "\n"
    "Add a path for the application\n"
    "\n"
    "Path type value :\n"
    "   - conf\n"
    "   - data\n"
    "   - exec\n"
    "   - http\n"
    "   - icon\n"
    "   - id\n"
    "   - lib\n"
    "   - public\n"
    "\n"
    "Example : path /tmp/file data\n"
    "\n";

static const char help_plug_text[] =
    "\n"
    "Command: plug exported-path import-id import-path\n"
    "\n"
    "Setup a plug export\n"
    "\n"
    "Example : plug /tmp/file data /tmp/file\n"
    "\n";

static const char help_permission_text[] =
    "\n"
    "Command: permission permission\n"
    "\n"
    "Add a permission for the application\n"
    "\n"
    "Example : permission urn:AGL:permission::partner:scope-platform\n"
    "\n";

static const char help_install_text[] =
    "\n"
    "Command: install\n"
    "\n"
    "Install application\n"
    "WARNING : You need to set id before\n"
    "\n";

static const char help_uninstall_text[] =
    "\n"
    "Command: uninstall\n"
    "\n"
    "Uninstall application\n"
    "WARNING : You need to set id before\n"
    "\n";

static const char help__text[] =
    "\n"
    "Commands are: log, clear, display, id, path, plug, permission,\n"
    "              install, uninstall, quit, help\n"
    "Type 'help command' to get help on the command\n"
    "\n"
    "Example 'help log' to get help on log\n"
    "\n";

static const char help_quit_text[] =
    "\n"
    "Command: quit\n"
    "\n"
    "Quit the program\n"
    "\n";

static const char help_help_text[] =
    "\n"
    "Command: help [command]\n"
    "\n"
    "Gives help on the command.\n"
    "\n"
    "Available commands: log, clear, display, id, path, permission, install, uninstall, quit, help\n"
    "\n";

static sec_lsm_manager_t *sec_lsm_manager = NULL;
static char buffer[4000] = {0};
static char *str[40] = {0};
static size_t bufill = 0;
static int nstr = 0;
static int echo = 0;
static int last_status = 0;

static int plink(int ac, char **av, int *used, int maxi) {
    int r = 0;

    if (maxi < ac)
        ac = maxi;
    while (r < ac && strcmp(av[r], ";")) r++;

    *used = r + (r < ac);
    return r;
}

static int show_status(int used_count, const char *oper, int rc, const char *extra)
{
    last_status = rc;
    if (rc >= 0) {
        LOG(extra != NULL ? "ok %s" : "ok", extra);
    }
    else if (rc == -EPROTO) {
        char *message;
        sec_lsm_manager_error_message(sec_lsm_manager, &message);
        LOG("%s, error: %s", oper, message);
        free(message);
    }
    else {
        ERROR("%s, unexpected error: %s", oper, strerror(-rc));
    }
    return used_count;
}

static int do_clear(int ac, char **av) {
    int used_count, rc;
    int n = plink(ac, av, &used_count, 1);

    if (n < 1) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    rc = sec_lsm_manager_clear(sec_lsm_manager);
    return show_status(used_count, "clear", rc, NULL);
}

static int do_display(int ac, char **av) {
    int used_count, rc;
    int n = plink(ac, av, &used_count, 1);

    if (n < 1) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    rc = sec_lsm_manager_display(sec_lsm_manager);
    return show_status(used_count, "display", rc, NULL);
}

static int do_id(int ac, char **av) {
    int used_count, rc;
    char *id = NULL;
    int n = plink(ac, av, &used_count, 2);

    if (n < 2) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    if (strlen(av[1]) <= 0) {
        ERROR("bad argument %s", av[1]);
        last_status = -EINVAL;
        return used_count;
    }

    id = av[1];
    rc = sec_lsm_manager_set_id(sec_lsm_manager, id);
    return show_status(used_count, "id", rc, NULL);
}

static int do_path(int ac, char **av) {
    int used_count, rc;
    char *path = NULL;
    char *path_type = NULL;
    int n = plink(ac, av, &used_count, 3);

    if (n < 3) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    if (strlen(av[1]) <= 0) {
        ERROR("bad argument %s", av[1]);
        last_status = -EINVAL;
        return used_count;
    }

    path = av[1];
    path_type = av[2];

    rc = sec_lsm_manager_add_path(sec_lsm_manager, path, path_type);
    return show_status(used_count, "path", rc, NULL);
}

static int do_plug(int ac, char **av) {
    int used_count, rc;
    char *export_path = NULL;
    char *import_id = NULL;
    char *import_path = NULL;
    int n = plink(ac, av, &used_count, 4);

    if (n < 4) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    if (strlen(av[1]) <= 0) {
        ERROR("bad argument %s", av[1]);
        last_status = -EINVAL;
        return used_count;
    }

    export_path = av[1];
    import_id = av[2];
    import_path = av[3];

    rc = sec_lsm_manager_add_plug(sec_lsm_manager, export_path, import_id, import_path);
    return show_status(used_count, "plug", rc, NULL);
}

static int do_permission(int ac, char **av) {
    int used_count, rc;
    char *permission = NULL;
    int n = plink(ac, av, &used_count, 2);

    if (n < 2) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    if (strlen(av[1]) <= 0) {
        ERROR("bad argument %s", av[1]);
        last_status = -EINVAL;
        return used_count;
    }

    permission = av[1];

    rc = sec_lsm_manager_add_permission(sec_lsm_manager, permission);
    return show_status(used_count, "permission", rc, NULL);
}

static int do_install(int ac, char **av) {
    int used_count, rc;
    int n = plink(ac, av, &used_count, 1);

    if (n < 1) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    rc = sec_lsm_manager_install(sec_lsm_manager);
    return show_status(used_count, "install", rc, NULL);
}

static int do_uninstall(int ac, char **av) {
    int used_count, rc;
    int n = plink(ac, av, &used_count, 1);

    if (n < 1) {
        ERROR("not enough arguments");
        last_status = -EINVAL;
        return used_count;
    }

    rc = sec_lsm_manager_uninstall(sec_lsm_manager);
    return show_status(used_count, "uninstall", rc, NULL);
}

static int do_log(int ac, char **av) {
    int used_count, rc;
    int on = 0, off = 0;
    int n = plink(ac, av, &used_count, 2);

    if (n > 1) {
        on = !strcmp(av[1], "on");
        off = !strcmp(av[1], "off");
        if (!on && !off) {
            fprintf(stderr, "bad argument '%s'\n", av[1]);
            return used_count;
        }
    }

    rc = sec_lsm_manager_log(sec_lsm_manager, on, off);
    return show_status(used_count, "log", rc, rc == 1 ? "on" : "off");
}

static int do_help(int ac, char **av) {
    const char *help = help__text;
    int rc = 2;
    if (ac > 1 && !strcmp(av[1], "log"))
        help = help_log_text;
    else if (ac > 1 && !strcmp(av[1], "quit"))
        help = help_quit_text;
    else if (ac > 1 && !strcmp(av[1], "help"))
        help = help_help_text;
    else if (ac > 1 && !strcmp(av[1], "clear"))
        help = help_clear_text;
    else if (ac > 1 && !strcmp(av[1], "display"))
        help = help_display_text;
    else if (ac > 1 && !strcmp(av[1], "id"))
        help = help_id_text;
    else if (ac > 1 && !strcmp(av[1], "path"))
        help = help_path_text;
    else if (ac > 1 && !strcmp(av[1], "plug"))
        help = help_plug_text;
    else if (ac > 1 && !strcmp(av[1], "permission"))
        help = help_permission_text;
    else if (ac > 1 && !strcmp(av[1], "install"))
        help = help_install_text;
    else if (ac > 1 && !strcmp(av[1], "uninstall"))
        help = help_uninstall_text;
    else
        rc = 1;
    fprintf(stdout, "%s", help);
    return rc;
}

static int do_any(int ac, char **av) {
    if (!ac)
        return 0;

    if (!strcmp(av[0], "log"))
        return do_log(ac, av);

    if (!strcmp(av[0], "clear"))
        return do_clear(ac, av);

    if (!strcmp(av[0], "display"))
        return do_display(ac, av);

    if (!strcmp(av[0], "id"))
        return do_id(ac, av);

    if (!strcmp(av[0], "path"))
        return do_path(ac, av);

    if (!strcmp(av[0], "plug"))
        return do_plug(ac, av);

    if (!strcmp(av[0], "permission"))
        return do_permission(ac, av);

    if (!strcmp(av[0], "install"))
        return do_install(ac, av);

    if (!strcmp(av[0], "uninstall"))
        return do_uninstall(ac, av);

    if (!strcmp(av[0], "quit"))
        exit(0);

    if (!strcmp(av[0], "help") || !strcmp(av[0], "?"))
        return do_help(ac, av);

    fprintf(stderr, "unknown command %s (try help)\n", av[0]);
    return 1;
}

static void do_all(int ac, char **av, int quit) {
    int rc;

    if (echo) {
        for (rc = 0; rc < ac; rc++) fprintf(stdout, "%s%s", rc ? " " : "", av[rc]);
        fprintf(stdout, "\n");
    }
    while (ac) {
        last_status = 0;
        rc = do_any(ac, av);
        if (quit && (rc <= 0 || last_status < 0))
            exit(1);
        ac -= rc;
        av += rc;
    }
}

int main(int ac, char **av) {
    int opt;
    int rc;
    int help = 0;
    int version = 0;
    int keep_going = 0;
    int error = 0;
    char *socket = NULL;
    char *p;

    setlinebuf(stdout);

    /* scan arguments */
    for (;;) {
        opt = getopt_long(ac, av, shortopts, longopts, NULL);
        if (opt == -1)
            break;

        switch (opt) {
            case _ECHO_:
                echo = 1;
                break;
            case _HELP_:
                help = 1;
                break;
            case _KEEP_GOING_:
                keep_going = 1;
                break;
            case _SOCKET_:
                socket = optarg;
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
        puts(helptxt);
        return 0;
    }

    if (version) {
        puts(versiontxt);
        return 0;
    }

    if (error)
        return 1;

    /* initialize server */
    signal(SIGPIPE, SIG_IGN); /* avoid SIGPIPE! */
    rc = sec_lsm_manager_create(&sec_lsm_manager, socket);
    if (rc < 0) {
        ERROR("initialization failed : %d %s", -rc, strerror(-rc));
        return 1;
    }

    LOG("initialization success");

    if (optind < ac) {
        do_all(ac - optind, av + optind, !keep_going);
        return 0;
    }

    bufill = 0;
    for (;;) {
        rc = (int)read(0, &buffer[bufill], sizeof buffer - bufill);
        if (rc == 0)
            break;
        if (rc > 0) {
            bufill += (size_t)rc;
            while ((p = memchr(buffer, '\n', bufill))) {
                /* process one line */
                *p++ = 0;
                str[nstr = 0] = strtok(buffer, " \t");
                while (str[nstr]) str[++nstr] = strtok(NULL, " \t");
                do_all(nstr, str, 0);
                bufill -= (size_t)(p - buffer);
                if (!bufill)
                    break;
                memmove(buffer, p, bufill);
            }
        }
    }
    return 0;
}
