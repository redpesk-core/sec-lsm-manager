###########################################################################
# Copyright (C) 2018-2025 IoT.bzh Company
#
# Author: José Bollo <jose.bollo@iot.bzh>
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

set(SYSTEMD_UNIT_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system"
                  CACHE PATH "Path to systemd system unit files")

CONFIGURE_FILE(${CMAKE_PROJECT_NAME}.service.in      ${CMAKE_PROJECT_NAME}.service @ONLY)
CONFIGURE_FILE(${CMAKE_PROJECT_NAME}.socket.in       ${CMAKE_PROJECT_NAME}.socket @ONLY)

INSTALL(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.service
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.socket
    ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.target
    DESTINATION
    ${SYSTEMD_UNIT_DIR}
)

INSTALL(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/sockets.target.wants
    DESTINATION
    ${SYSTEMD_UNIT_DIR}
)


