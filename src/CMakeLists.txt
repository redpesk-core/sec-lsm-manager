###########################################################################
# Copyright (C) 2018-2025 IoT.bzh Company
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

add_compile_definitions(
    VERSION="${PROJECT_VERSION}"
    PREFIX="${CMAKE_INSTALL_PREFIX}"
)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

##########################################################
# build and install libsec-lsm-manager the client library
##########################################################

message("\n[*] Create : libsec-lsm-manager")

add_library(sec-lsm-manager STATIC
    protocol/prot.c
    protocol/sec-lsm-manager.c
    protocol/sec-lsm-manager-protocol.c
    protocol/socket.c
)
install(TARGETS sec-lsm-manager LIBRARY
        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
install(FILES protocol/sec-lsm-manager.h
        DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

message("[x] Done : libsec-lsm-manager\n")

#######################################################################
# build and install sec-lsm-manager-smackd and sec-lsm-manager-selinuxd
#######################################################################

# common sources
add_library(common-lib OBJECT
    action/action.c
    context/context.c
    context/paths.c
    context/permissions.c
    context/plugs.c
    file-utils.c
    log.c
    path-utils.c
    perm-cynagora/cynagora-interface.c
    protocol/client.c
    protocol/pollitem.c
    protocol/prot.c
    protocol/sec-lsm-manager-protocol.c
    protocol/sec-lsm-manager-server.c
    templating/mustach.c
    templating/template.c
    utf8-utils.c
    xattr-utils.c
)
if(SIMULATE_CYNAGORA)
    message("[-] Simul : cynagora")
else()
    target_link_libraries(common-lib PUBLIC ${cynagora_LDFLAGS} ${cynagora_LINK_LIBRARIES})
    target_include_directories(common-lib PUBLIC ${cynagora_INCLUDE_DIRS})
    target_compile_options(common-lib PUBLIC ${cynagora_CFLAGS})
    message("[-] Link : cynagora")
endif()

# socket (without or with systemd)
add_library(socket-std  OBJECT protocol/socket.c)
add_library(socket-srv  OBJECT protocol/socket.c)
if(WITH_SYSTEMD)
    target_compile_definitions(socket-srv PUBLIC WITH_SYSTEMD=1)
    target_link_libraries(socket-srv PUBLIC ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
    target_include_directories(socket-srv PUBLIC ${libsystemd_INCLUDE_DIRS})
    target_compile_options(socket-srv PUBLIC ${libsystemd_CFLAGS})
    message("[-] Link : libsystemd")
endif()

# smack part
if(WITH_SMACK)
    add_library(app-smack OBJECT lsm-smack/smack-template.c lsm-smack/smack.c)
    target_compile_definitions(app-smack PUBLIC WITH_SMACK=1)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lsm-smack)

    if(SIMULATE_SMACK)
        message("[-] Simul : smack")
    else()
        target_link_libraries(app-smack PUBLIC ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
        target_include_directories(app-smack PUBLIC ${libsmack_INCLUDE_DIRS})
        target_include_directories(app-smack PUBLIC $<TARGET_PROPERTY:common-lib,INTERFACE_INCLUDE_DIRECTORIES>)
        target_compile_options(app-smack PUBLIC ${libsmack_CFLAGS})
        message("[-] Link : libsmack")
    endif()
endif()

# selinux part
if(WITH_SELINUX)
    add_library(app-selinux OBJECT lsm-selinux/selinux.c lsm-selinux/selinux-template.c)
    target_compile_definitions(app-selinux PUBLIC WITH_SELINUX=1)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lsm-selinux)

    if(SIMULATE_SELINUX)
        message("[-] Simul : selinux")
    else()
        target_sources(app-selinux PRIVATE lsm-selinux/selinux-compile.c)
        target_link_libraries(app-selinux PUBLIC ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
        target_link_libraries(app-selinux PUBLIC ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
        target_include_directories(app-selinux PUBLIC ${libselinux_INCLUDE_DIRS})
        target_include_directories(app-selinux PUBLIC ${libsemanage_INCLUDE_DIRS})
        target_include_directories(app-selinux PUBLIC $<TARGET_PROPERTY:common-lib,INTERFACE_INCLUDE_DIRECTORIES>)
        target_compile_options(app-selinux PUBLIC ${libselinux_CFLAGS})
        target_compile_options(app-selinux PUBLIC ${libsemanage_CFLAGS})
        message("[-] Link : libselinux")
        message("[-] Link : libsemanage")
    endif()
endif()

function(daemon_for_mac MAC_NAME)
    message("[*] Create : sec-lsm-manager-${MAC_NAME}d")
    add_executable(sec-lsm-manager-${MAC_NAME}d main-sec-lsm-managerd.c)
    target_link_libraries(sec-lsm-manager-${MAC_NAME}d PUBLIC cap common-lib socket-srv)
    target_link_libraries(sec-lsm-manager-${MAC_NAME}d PUBLIC app-${MAC_NAME})
    install(TARGETS sec-lsm-manager-${MAC_NAME}d RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
    message("[x] Done : sec-lsm-manager-${MAC_NAME}d\n")
endfunction()

if(WITH_SMACK)
    daemon_for_mac(smack)
endif()

if(WITH_SELINUX)
    daemon_for_mac(selinux)
endif()

####################################
# build and install sec-lsm-managerd
####################################

message("[*] Create : sec-lsm-managerd")

add_executable(sec-lsm-managerd main-sec-lsm-managerd-launch.c)

if(WITH_SMACK)
    target_compile_definitions(sec-lsm-managerd PRIVATE WITH_SMACK=1)
    target_compile_definitions(sec-lsm-managerd PRIVATE SMACK_BYNARY_NAME="sec-lsm-manager-smackd")
endif()

if(WITH_SELINUX)
    target_compile_definitions(sec-lsm-managerd PRIVATE WITH_SELINUX=1)
    target_compile_definitions(sec-lsm-managerd PRIVATE SELINUX_BYNARY_NAME="sec-lsm-manager-selinuxd")
endif()

install(TARGETS sec-lsm-managerd
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

message("[x] Done : sec-lsm-managerd\n")

###########################################
# build and install sec-lsm-manager-cmd
###########################################

message("[*] Create : sec-lsm-manager-cmd")

add_executable(sec-lsm-manager-cmd main-sec-lsm-manager-cmd.c log.c)

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

