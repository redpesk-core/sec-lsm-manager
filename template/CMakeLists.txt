INSTALL(FILES "app-template.smack" DESTINATION ${DATADIR}/security-manager/)
INSTALL(FILES "app-template.te" DESTINATION ${DATADIR}/security-manager/)
INSTALL(FILES "app-template.if" DESTINATION ${DATADIR}/security-manager/)
INSTALL(DIRECTORY DESTINATION ${DATADIR}/security-manager/selinux-policy/)