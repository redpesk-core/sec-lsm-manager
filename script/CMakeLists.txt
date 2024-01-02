###########################################################################
# Copyright (C) 2018-2024 IoT.bzh Company
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

if((NOT SIMULATE_SELINUX) AND WITH_SELINUX)

    CONFIGURE_FILE(${COMPILE_SCRIPT_NAME}.in      ${COMPILE_SCRIPT_NAME} @ONLY)

    INSTALL(PROGRAMS
        ${CMAKE_CURRENT_BINARY_DIR}/${COMPILE_SCRIPT_NAME}
        DESTINATION
        ${COMPILE_SCRIPT_DIR}
    )

endif()

