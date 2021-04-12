###########################################################################
# Copyright (C) 2018-2021 IoT.bzh Company
#
# Author: Arthur Guyader <arthur.guyader@iot.bzh>
#
# $RP_BEGIN_LICENSE$
# Commercial License Usage
#  Licensees holding valid commercial IoT.bzh licenses may use this file in
#  accordance with the commercial license agreement provided with the
#  Software or, alternatively, in accordance with the terms contained in
#  a written agreement between you and The IoT.bzh Company. For licensing terms
#  and conditions see https://www.iot.bzh/terms-conditions. For further
#  information use the contact form at https://www.iot.bzh/contact.
#
# GNU General Public License Usage
#  Alternatively, this file may be used under the terms of the GNU General
#  Public license version 3. This license is as published by the Free Software
#  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
#  of this file. Please review the following information to ensure the GNU
#  General Public License requirements will be met
#  https://www.gnu.org/licenses/gpl-3.0.html.
# $RP_END_LICENSE$
###########################################################################

find_program(M4EXEC m4)
mark_as_advanced(M4EXEC)

configure_file(macros.in macros.in @ONLY)
configure_file(smack/app-template.smack.in  smack/app-template.smack.in   @ONLY)
configure_file(selinux/app-template.te.in   selinux/app-template.te.in    @ONLY)
configure_file(selinux/app-template.if.in   selinux/app-template.if.in    @ONLY)

if(WITH_SMACK)
    add_custom_command(OUTPUT app-template.smack
        COMMAND ${M4EXEC} ${optm4} -I. app-template.smack.in > app-template.smack
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/smack
    )
    add_custom_target(unitconf ALL DEPENDS app-template.smack)

    INSTALL(FILES "smack/app-template.smack" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
    if(SIMULATE_SMACK)
        INSTALL(DIRECTORY DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/smack-simulation/)
    endif()
endif()

if(WITH_SELINUX)
    add_custom_command(OUTPUT app-template.te app-template.if
        COMMAND ${M4EXEC} ${optm4} -I. app-template.te.in > app-template.te
        COMMAND ${M4EXEC} ${optm4} -I. app-template.if.in > app-template.if
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/selinux
    )
    add_custom_target(unitconf ALL DEPENDS app-template.te app-template.if)

    INSTALL(FILES "selinux/app-template.te" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
    INSTALL(FILES "selinux/app-template.if" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/)
    INSTALL(DIRECTORY DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/selinux-policy/)
    if(SIMULATE_SELINUX)
        INSTALL(DIRECTORY DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/selinux-simulation/)
    endif()
endif()
