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
    ${CMAKE_PROJECT_NAME}-protocol.c
    ${CMAKE_PROJECT_NAME}-server.c
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
    ${CMAKE_PROJECT_NAME}-protocol.c
    ${CMAKE_PROJECT_NAME}.c
)

##############################################
# build and install libsec-lsm-manager-client
##############################################

add_library(${CMAKE_PROJECT_NAME} SHARED ${LIBCLI_SOURCES})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    VERSION ${SEC_LSM_MANAGER_VERSION}
    SOVERSION ${SEC_LSM_MANAGER_SOVERSION}
    LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export-${CMAKE_PROJECT_NAME}.map
)

install(TARGETS ${CMAKE_PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
install(FILES ${CMAKE_PROJECT_NAME}.h DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})


#####################################
# build and install sec-lsm-managerd
#####################################

add_executable(${CMAKE_PROJECT_NAME}d main-${CMAKE_PROJECT_NAME}d.c ${SERVER_SOURCES})

target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE
    DEFAULT_SOCKET_DIR="${DEFAULT_SOCKET_DIR}"
)

target_link_libraries(${CMAKE_PROJECT_NAME}d cap)

if(WITH_SMACK)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE WITH_SMACK)
    target_link_libraries(${CMAKE_PROJECT_NAME}d ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME}d PRIVATE ${libsmack_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME}d PRIVATE ${libsmack_CFLAGS})
elseif(WITH_SELINUX)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE WITH_SELINUX)
    target_link_libraries(${CMAKE_PROJECT_NAME}d ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME}d PRIVATE ${libselinux_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME}d PRIVATE ${libselinux_CFLAGS})
    target_link_libraries(${CMAKE_PROJECT_NAME}d ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME}d PRIVATE ${libsemanage_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME}d PRIVATE ${libsemanage_CFLAGS})
endif()

if(NOT SIMULATE_CYNAGORA)
    target_link_libraries(${CMAKE_PROJECT_NAME}d ${cynagora_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME}d PRIVATE ${cynagora_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME}d PRIVATE ${cynagora_CFLAGS})
endif()

if(WITH_SYSTEMD)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE WITH_SYSTEMD)
    target_link_libraries(${CMAKE_PROJECT_NAME}d ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME}d PRIVATE ${libsystemd_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME}d PRIVATE ${libsystemd_CFLAGS})
endif()

install(TARGETS ${CMAKE_PROJECT_NAME}d
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})


###########################################
# build and install sec-lsm-manager-cmd
###########################################

add_executable(${CMAKE_PROJECT_NAME}-cmd main-${CMAKE_PROJECT_NAME}-cmd.c log.c utils.c)

target_link_libraries(${CMAKE_PROJECT_NAME}-cmd ${CMAKE_PROJECT_NAME})

install(TARGETS ${CMAKE_PROJECT_NAME}-cmd
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

##############
# build tests
##############

if(COMPILE_TEST)
    add_subdirectory(tests)
endif()
