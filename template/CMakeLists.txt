###########################################################################
# Copyright (C) 2018-2023 IoT.bzh Company
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
if(NOT M4EXEC)
    message(FATAL_ERROR "m4 required")
endif()

mark_as_advanced(M4EXEC)

configure_file(macros.in macros.in @ONLY)
configure_file(smack/${TEMPLATE_FILE}.in        smack/${TEMPLATE_FILE}.in         @ONLY)
configure_file(selinux/${TE_TEMPLATE_FILE}.in   selinux/${TE_TEMPLATE_FILE}.in    @ONLY)
configure_file(selinux/${IF_TEMPLATE_FILE}.in   selinux/${IF_TEMPLATE_FILE}.in    @ONLY)

if(WITH_SMACK)
    add_custom_command(OUTPUT ${TEMPLATE_FILE}
        COMMAND ${M4EXEC} ${optm4} -I. ${TEMPLATE_FILE}.in > ${TEMPLATE_FILE}
        COMMAND sed -i "'/^$$/d'" ${TEMPLATE_FILE}
        COMMAND sed -i "'/^#/d'" ${TEMPLATE_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/smack
    )
    add_custom_target(conf-smack ALL DEPENDS ${TEMPLATE_FILE})

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/smack/${TEMPLATE_FILE} DESTINATION ${SEC_LSM_MANAGER_DATADIR})
    if(SIMULATE_SMACK)
        INSTALL(DIRECTORY DESTINATION ${SIMULATION_SMACK_POLICY_DIR})
    endif()
endif()

if(WITH_SELINUX)
    add_custom_command(OUTPUT ${TE_TEMPLATE_FILE} ${IF_TEMPLATE_FILE}
        COMMAND ${M4EXEC}  -I. ${TE_TEMPLATE_FILE}.in > ${TE_TEMPLATE_FILE}
        COMMAND ${M4EXEC}  -I. ${IF_TEMPLATE_FILE}.in > ${IF_TEMPLATE_FILE}
        COMMAND sed -i "'/^$$/d'" ${TE_TEMPLATE_FILE} ${IF_TEMPLATE_FILE}
        COMMAND sed -i "'/^#/d'" ${TE_TEMPLATE_FILE} ${IF_TEMPLATE_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/selinux
    )
    add_custom_target(conf-selinux ALL DEPENDS ${TE_TEMPLATE_FILE} ${IF_TEMPLATE_FILE})

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/selinux/${TE_TEMPLATE_FILE} DESTINATION ${SEC_LSM_MANAGER_DATADIR})
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/selinux/${IF_TEMPLATE_FILE} DESTINATION ${SEC_LSM_MANAGER_DATADIR})
    INSTALL(DIRECTORY DESTINATION ${SELINUX_RULES_DIR})
    if(SIMULATE_SELINUX)
        INSTALL(DIRECTORY DESTINATION ${SIMULATION_SELINUX_POLICY_DIR})
    endif()
endif()
