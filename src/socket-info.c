int get_socket_info(int fd, socket_info_t *socket_info) {
    int rc = 0;
    struct ucred uc;
    socklen_t len = sizeof(uc);

    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &uc, &len)) {
        rc = -errno;
        goto ret;
    }

#ifdef WITH_SMACK
    rc = (int)smack_new_label_from_socket(fd, &(socket_info->label));
    if (rc < 0) {
        rc = -errno;
        ERROR("smack_new_label_from_socket");
        goto ret;
    }
#elif WITH_SELINUX
    socket_info->label = strdup("LABELSOCKETSELINUX");
#else
    socket_info->label = strdup("LABELSOCKET");
#endif
    socket_info->uid = uc.uid;
    socket_info->pid = uc.pid;

    socket_info->uid_str = get_uid_str(socket_info->uid);
    if (socket_info->uid_str == NULL) {
        ERROR("get_uid_str %d", socket_info->uid);
        rc = -EINVAL;
        goto ret;
    }

ret:
    return rc;
}
