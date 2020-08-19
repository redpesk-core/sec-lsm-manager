typedef struct socket_info {
    uid_t uid;
    char *uid_str;
    pid_t pid;
    char *label;
} socket_info_t;

int get_socket_info(int fd, socket_info_t *socket_info);