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
    mustach/mustach.c
    template.c
    cynagora-interface.c
    secure-app.c
    socket.c
    pollitem.c
    prot.c
    ${CMAKE_PROJECT_NAME}-protocol.c
    ${CMAKE_PROJECT_NAME}-server.c
)

if(SIMULATE_CYNAGORA)
    set(SERVER_SOURCES ${SERVER_SOURCES} simulation/cynagora/cynagora.c)
endif()

if(SIMULATE_SMACK)
    set(SERVER_SOURCES ${SERVER_SOURCES} simulation/smack/smack.c)
endif()

if(SIMULATE_SELINUX)
    set(SERVER_SOURCES ${SERVER_SOURCES} simulation/selinux/selinux.c)
endif()

if(WITH_SMACK)
    set(SERVER_SOURCES_SMACK ${SERVER_SOURCES} smack-template.c smack.c)
endif()

if(WITH_SELINUX)
    set(SERVER_SOURCES_SELINUX ${SERVER_SOURCES} selinux.c selinux-template.c)
endif()

if((NOT SIMULATE_SELINUX) AND WITH_SELINUX)
    set(SERVER_SOURCES_SELINUX ${SERVER_SOURCES_SELINUX} selinux-compile.c)
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

######################################
# build and install libsec-lsm-manager
######################################

message("\n[*] Create : lib${CMAKE_PROJECT_NAME}")

add_library(${CMAKE_PROJECT_NAME} SHARED ${LIBCLI_SOURCES})

if(WITH_SYSTEMD)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${libsystemd_INCLUDE_DIRS})
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${libsystemd_CFLAGS})
    message("[-] Link : libsystemd")
endif()

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    VERSION ${SEC_LSM_MANAGER_VERSION}
    SOVERSION ${SEC_LSM_MANAGER_SOVERSION}
    LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export-${CMAKE_PROJECT_NAME}.map
)

install(TARGETS ${CMAKE_PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
install(FILES ${CMAKE_PROJECT_NAME}.h DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

message("[x] Done : lib${CMAKE_PROJECT_NAME}\n")

#######################################################################
# build and install sec-lsm-manager-smackd and sec-lsm-manager-selinuxd
#######################################################################


function(build_daemon_for_mac MAC_NAME)
    message("[*] Create : ${CMAKE_PROJECT_NAME}-${MAC_NAME}d")
    if(${MAC_NAME} STREQUAL "smack")
        add_executable(${CMAKE_PROJECT_NAME}-smackd main-${CMAKE_PROJECT_NAME}d.c ${SERVER_SOURCES_SMACK})
        if(NOT SIMULATE_SMACK)
            target_link_libraries(${CMAKE_PROJECT_NAME}-smackd ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
            target_include_directories(${CMAKE_PROJECT_NAME}-smackd PRIVATE ${libsmack_INCLUDE_DIRS})
            target_compile_options(${CMAKE_PROJECT_NAME}-smackd PRIVATE ${libsmack_CFLAGS})
            message("[-] Link : libsmack")
        endif()
    endif()

    if(${MAC_NAME} STREQUAL "selinux")
        add_executable(${CMAKE_PROJECT_NAME}-selinuxd main-${CMAKE_PROJECT_NAME}d.c ${SERVER_SOURCES_SELINUX})
        if(NOT SIMULATE_SELINUX)
            target_link_libraries(${CMAKE_PROJECT_NAME}-selinuxd ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
            target_include_directories(${CMAKE_PROJECT_NAME}-selinuxd PRIVATE ${libselinux_INCLUDE_DIRS})
            target_compile_options(${CMAKE_PROJECT_NAME}-selinuxd PRIVATE ${libselinux_CFLAGS})
            target_link_libraries(${CMAKE_PROJECT_NAME}-selinuxd ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
            target_include_directories(${CMAKE_PROJECT_NAME}-selinuxd PRIVATE ${libsemanage_INCLUDE_DIRS})
            target_compile_options(${CMAKE_PROJECT_NAME}-selinuxd PRIVATE ${libsemanage_CFLAGS})
            message("[-] Link : libselinux")
            message("[-] Link : libsemanage")
        endif()
    endif()

    target_link_libraries(${CMAKE_PROJECT_NAME}-${MAC_NAME}d cap)

    if(NOT SIMULATE_CYNAGORA)
        target_link_libraries(${CMAKE_PROJECT_NAME}-${MAC_NAME}d ${cynagora_LDFLAGS} ${cynagora_LINK_LIBRARIES})
        target_include_directories(${CMAKE_PROJECT_NAME}-${MAC_NAME}d PRIVATE ${cynagora_INCLUDE_DIRS})
        target_compile_options(${CMAKE_PROJECT_NAME}-${MAC_NAME}d PRIVATE ${cynagora_CFLAGS})
        message("[-] Link : cynagora")
    endif()

    if(WITH_SYSTEMD)
        target_link_libraries(${CMAKE_PROJECT_NAME}-${MAC_NAME}d ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
        target_include_directories(${CMAKE_PROJECT_NAME}-${MAC_NAME}d PRIVATE ${libsystemd_INCLUDE_DIRS})
        target_compile_options(${CMAKE_PROJECT_NAME}-${MAC_NAME}d PRIVATE ${libsystemd_CFLAGS})
        message("[-] Link : libsystemd")
    endif()

    install(TARGETS ${CMAKE_PROJECT_NAME}-${MAC_NAME}d
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

    message("[x] Done : ${CMAKE_PROJECT_NAME}-${MAC_NAME}d\n")
endfunction()

if(WITH_SMACK)
    build_daemon_for_mac("smack")
    target_compile_definitions(${CMAKE_PROJECT_NAME}-smackd PRIVATE WITH_SMACK)
endif()

if(WITH_SELINUX)
    build_daemon_for_mac("selinux")
    target_compile_definitions(${CMAKE_PROJECT_NAME}-selinuxd PRIVATE WITH_SELINUX)
endif()

####################################
# build and install sec-lsm-managerd
####################################

message("[*] Create : ${CMAKE_PROJECT_NAME}d")

add_executable(${CMAKE_PROJECT_NAME}d main-${CMAKE_PROJECT_NAME}d-setup.c)

if(WITH_SMACK)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE WITH_SMACK)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE SMACK_BYNARY_NAME="${CMAKE_PROJECT_NAME}-smackd")
endif()

if(WITH_SELINUX)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE WITH_SELINUX)
    target_compile_definitions(${CMAKE_PROJECT_NAME}d PRIVATE SELINUX_BYNARY_NAME="${CMAKE_PROJECT_NAME}-selinuxd")
endif()

install(TARGETS ${CMAKE_PROJECT_NAME}d
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

message("[x] Done : ${CMAKE_PROJECT_NAME}d\n")

###########################################
# build and install sec-lsm-manager-cmd
###########################################

message("[*] Create : ${CMAKE_PROJECT_NAME}-cmd")

add_executable(${CMAKE_PROJECT_NAME}-cmd main-${CMAKE_PROJECT_NAME}-cmd.c log.c utils.c)

target_link_libraries(${CMAKE_PROJECT_NAME}-cmd ${CMAKE_PROJECT_NAME})

install(TARGETS ${CMAKE_PROJECT_NAME}-cmd
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

message("[x] Done : ${CMAKE_PROJECT_NAME}-cmd\n")

##############
# build tests
##############

if(COMPILE_TEST)
    add_subdirectory(tests)
endif()
