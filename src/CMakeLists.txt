###########################################################################
# Copyright (C) 2018-2023 IoT.bzh Company
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

add_compile_definitions(VERSION="${PROJECT_VERSION}")

##########################################################
# build and install libsec-lsm-manager the client library
##########################################################

message("\n[*] Create : libsec-lsm-manager")

add_library(sec-lsm-manager SHARED
    utils.c
    prot.c
    socket.c
    log.c
    sec-lsm-manager-protocol.c
    sec-lsm-manager.c
)

if(WITH_SYSTEMD)
    target_link_libraries(sec-lsm-manager ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
    target_include_directories(sec-lsm-manager PRIVATE ${libsystemd_INCLUDE_DIRS})
    target_compile_options(sec-lsm-manager PRIVATE ${libsystemd_CFLAGS})
    message("[-] Link : libsystemd")
endif()

set_target_properties(sec-lsm-manager PROPERTIES
    VERSION ${SEC_LSM_MANAGER_VERSION}
    SOVERSION ${SEC_LSM_MANAGER_SOVERSION}
    LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export-sec-lsm-manager.map
)

install(TARGETS sec-lsm-manager LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
install(FILES sec-lsm-manager.h DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

message("[x] Done : libsec-lsm-manager\n")

#######################################################################
# build and install sec-lsm-manager-smackd and sec-lsm-manager-selinuxd
#######################################################################

##########################################################################
# define server sources

set(SERVER_SOURCES
    cynagora-interface.c
    log.c
    mustach/mustach.c
    paths.c
    permissions.c
    plugs.c
    pollitem.c
    prot.c
    secure-app.c
    socket.c
    template.c
    utils.c
    sec-lsm-manager-protocol.c
    sec-lsm-manager-server.c
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


function(build_daemon_for_mac MAC_NAME)
    message("[*] Create : sec-lsm-manager-${MAC_NAME}d")
    if(${MAC_NAME} STREQUAL "smack")
        add_executable(sec-lsm-manager-smackd main-sec-lsm-managerd.c ${SERVER_SOURCES_SMACK})
        target_compile_definitions(sec-lsm-manager-smackd PRIVATE WITH_SMACK)
        if(NOT SIMULATE_SMACK)
            target_link_libraries(sec-lsm-manager-smackd ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
            target_include_directories(sec-lsm-manager-smackd PRIVATE ${libsmack_INCLUDE_DIRS})
            target_compile_options(sec-lsm-manager-smackd PRIVATE ${libsmack_CFLAGS})
            message("[-] Link : libsmack")
        endif()
    endif()

    if(${MAC_NAME} STREQUAL "selinux")
        add_executable(sec-lsm-manager-selinuxd main-sec-lsm-managerd.c ${SERVER_SOURCES_SELINUX})
        target_compile_definitions(sec-lsm-manager-selinuxd PRIVATE WITH_SELINUX)
        if(NOT SIMULATE_SELINUX)
            target_link_libraries(sec-lsm-manager-selinuxd ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
            target_include_directories(sec-lsm-manager-selinuxd PRIVATE ${libselinux_INCLUDE_DIRS})
            target_compile_options(sec-lsm-manager-selinuxd PRIVATE ${libselinux_CFLAGS})
            target_link_libraries(sec-lsm-manager-selinuxd ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
            target_include_directories(sec-lsm-manager-selinuxd PRIVATE ${libsemanage_INCLUDE_DIRS})
            target_compile_options(sec-lsm-manager-selinuxd PRIVATE ${libsemanage_CFLAGS})
            message("[-] Link : libselinux")
            message("[-] Link : libsemanage")
        endif()
    endif()

    target_link_libraries(sec-lsm-manager-${MAC_NAME}d cap)

    if(NOT SIMULATE_CYNAGORA)
        target_link_libraries(sec-lsm-manager-${MAC_NAME}d ${cynagora_LDFLAGS} ${cynagora_LINK_LIBRARIES})
        target_include_directories(sec-lsm-manager-${MAC_NAME}d PRIVATE ${cynagora_INCLUDE_DIRS})
        target_compile_options(sec-lsm-manager-${MAC_NAME}d PRIVATE ${cynagora_CFLAGS})
        message("[-] Link : cynagora")
    endif()

    if(WITH_SYSTEMD)
        target_link_libraries(sec-lsm-manager-${MAC_NAME}d ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
        target_include_directories(sec-lsm-manager-${MAC_NAME}d PRIVATE ${libsystemd_INCLUDE_DIRS})
        target_compile_options(sec-lsm-manager-${MAC_NAME}d PRIVATE ${libsystemd_CFLAGS})
        message("[-] Link : libsystemd")
    endif()

    install(TARGETS sec-lsm-manager-${MAC_NAME}d
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

    message("[x] Done : sec-lsm-manager-${MAC_NAME}d\n")
endfunction()

if(WITH_SMACK)
    build_daemon_for_mac("smack")
endif()

if(WITH_SELINUX)
    build_daemon_for_mac("selinux")
endif()

####################################
# build and install sec-lsm-managerd
####################################

message("[*] Create : sec-lsm-managerd")

add_executable(sec-lsm-managerd main-sec-lsm-managerd-launch.c)

if(WITH_SMACK)
    target_compile_definitions(sec-lsm-managerd PRIVATE WITH_SMACK)
    target_compile_definitions(sec-lsm-managerd PRIVATE SMACK_BYNARY_NAME="sec-lsm-manager-smackd")
endif()

if(WITH_SELINUX)
    target_compile_definitions(sec-lsm-managerd PRIVATE WITH_SELINUX)
    target_compile_definitions(sec-lsm-managerd PRIVATE SELINUX_BYNARY_NAME="sec-lsm-manager-selinuxd")
endif()

install(TARGETS sec-lsm-managerd
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

message("[x] Done : sec-lsm-managerd\n")

###########################################
# build and install sec-lsm-manager-cmd
###########################################

message("[*] Create : sec-lsm-manager-cmd")

add_executable(sec-lsm-manager-cmd main-sec-lsm-manager-cmd.c log.c utils.c)

target_link_libraries(sec-lsm-manager-cmd sec-lsm-manager)

install(TARGETS sec-lsm-manager-cmd
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

message("[x] Done : sec-lsm-manager-cmd\n")

##############
# build tests
##############

if(COMPILE_TEST)
    add_subdirectory(tests)
endif()
