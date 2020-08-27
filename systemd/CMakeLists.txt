###########################################################################
# Copyright (C) 2018-2020 IoT.bzh Company
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


CONFIGURE_FILE(security-manager.service.in      security-manager.service @ONLY)
CONFIGURE_FILE(security-manager.socket.in       security-manager.socket @ONLY)

INSTALL(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/security-manager.service
    ${CMAKE_CURRENT_BINARY_DIR}/security-manager.socket
    ${CMAKE_CURRENT_SOURCE_DIR}/security-manager.target
    DESTINATION
    ${SYSTEMD_UNIT_DIR}
)

INSTALL(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/sockets.target.wants
    DESTINATION
    ${SYSTEMD_UNIT_DIR}
)


