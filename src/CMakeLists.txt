###########################################################################
# Copyright (C) 2018-2021 IoT.bzh Company
#
# Author: José Bollo <jose.bollo@iot.bzh>
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


##########################################################################
# define server sources

set(SERVER_SOURCES
    log.c
    utils.c
    paths.c
    permissions.c
    cynagora-interface.c
    secure-app.c
    socket.c
    pollitem.c
    prot.c
    security-manager-protocol.c
    security-manager-server.c
)

if(WITH_SMACK)
    set(SERVER_SOURCES ${SERVER_SOURCES} smack-label.c smack-template.c smack.c)
endif()
if(WITH_SELINUX)
    set(SERVER_SOURCES ${SERVER_SOURCES} selinux-label.c selinux.c selinux-template.c selinux-compile.c)
endif()

if(SIMULATE_CYNAGORA)
    set(SERVER_SOURCES ${CORE_SOURCES} simulation/cynagora/cynagora.c)
endif()

##########################################################################
# define client sources

set(LIBCLI_SOURCES
    prot.c
    socket.c
    log.c
    paths.c
    security-manager-protocol.c
    security-manager.c
)

##############################################
# build and install libsecurity-manager-client
##############################################

add_library(security-manager SHARED ${LIBCLI_SOURCES})

set_target_properties(security-manager PROPERTIES
    VERSION ${SECURITY_MANAGER_VERSION}
    SOVERSION ${SECURITY_MANAGER_SOVERSION}
    LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export-security-manager.map
)

install(TARGETS security-manager LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
install(FILES security-manager.h DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})


#####################################
# build and install security-managerd
#####################################

add_executable(security-managerd main-security-managerd.c ${SERVER_SOURCES})

target_compile_definitions(security-managerd PRIVATE
    DEFAULT_SOCKET_DIR="${DEFAULT_SOCKET_DIR}"
)

target_link_libraries(security-managerd cap)

if(WITH_SMACK)
    target_compile_definitions(security-managerd PRIVATE WITH_SMACK)
    target_link_libraries(security-managerd ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
    target_include_directories(security-managerd PRIVATE ${libsmack_INCLUDE_DIRS})
    target_compile_options(security-managerd PRIVATE ${libsmack_CFLAGS})
elseif(WITH_SELINUX)
    target_compile_definitions(security-managerd PRIVATE WITH_SELINUX)
    target_link_libraries(security-managerd ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(security-managerd PRIVATE ${libselinux_INCLUDE_DIRS})
    target_compile_options(security-managerd PRIVATE ${libselinux_CFLAGS})
    target_link_libraries(security-managerd ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
    target_include_directories(security-managerd PRIVATE ${libsemanage_INCLUDE_DIRS})
    target_compile_options(security-managerd PRIVATE ${libsemanage_CFLAGS})
endif()

if(NOT SIMULATE_CYNAGORA)
    target_link_libraries(security-managerd ${cynagora_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(security-managerd PRIVATE ${cynagora_INCLUDE_DIRS})
    target_compile_options(security-managerd PRIVATE ${cynagora_CFLAGS})
endif()

if(WITH_SYSTEMD)
    target_compile_definitions(security-managerd PRIVATE WITH_SYSTEMD)
    target_link_libraries(security-managerd ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
    target_include_directories(security-managerd PRIVATE ${libsystemd_INCLUDE_DIRS})
    target_compile_options(security-managerd PRIVATE ${libsystemd_CFLAGS})
endif()

install(TARGETS security-managerd
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})


###########################################
# build and install security-manager-cmd
###########################################

add_executable(security-manager-cmd main-security-manager-cmd.c log.c utils.c)

target_link_libraries(security-manager-cmd security-manager)

install(TARGETS security-manager-cmd
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

##############
# build tests
##############

if(COMPILE_TEST)
    add_subdirectory(tests)
endif()
