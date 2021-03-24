###########################################################################
# Copyright 2020-2021 IoT.bzh Company
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

cmake_minimum_required(VERSION 3.0)

project(sec-lsm-manager
    DESCRIPTION "Security LSM Manager"
    HOMEPAGE_URL ""
    VERSION 1.0
    LANGUAGES C
)

include(FindPkgConfig)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(GNUInstallDirs)
include(CTest)

##########################################################################
# define socket and systemd directory

if(NOT CMAKE_INSTALL_FULL_RUNSTATEDIR)
    set(CMAKE_INSTALL_FULL_RUNSTATEDIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/run")
endif()

set(DEFAULT_SOCKET_DIR "${CMAKE_INSTALL_FULL_RUNSTATEDIR}"
    CACHE PATH "directory path of the sockets")

if(NOT SYSTEMD_UNIT_DIR)
    set(SYSTEMD_UNIT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/systemd/system)
endif()

##########################################################################
# possible settings

set(SEC_LSM_MANAGER_VERSION ${PROJECT_VERSION})
set(SEC_LSM_MANAGER_SOVERSION ${PROJECT_VERSION_MAJOR})

option(WITH_SYSTEMD         "should include systemd compatibility" ON)
option(WITH_SMACK           "should include smack compatibility" OFF)
option(WITH_SELINUX         "should include selinux compatibility" ON)

option(SIMULATE_CYNAGORA    "simulate cynagora" OFF)

option(FORTIFY              "fortify" OFF)
option(COMPILE_TEST         "compile test" OFF)
option(DEBUG                "debug" OFF)

##########################################################################
# check valid MAC

if(WITH_SMACK AND WITH_SELINUX)
    message("==> MAC = SMACK & SELINUX")
elseif(WITH_SMACK)
    message("==> MAC = SMACK")
elseif(WITH_SELINUX)
    message("==> MAC = SELINUX")
else()
    message( FATAL_ERROR "Choose one MAC")
endif()

##########################################################################
# define user and group

set(USER  ${CMAKE_PROJECT_NAME} CACHE STRING "user of the daemon")
set(GROUP ${CMAKE_PROJECT_NAME} CACHE STRING "group of the daemon")
set(CYNAGORA_GROUP cynagora CACHE STRING "group of the daemon")

##########################################################################
# define compile options

if(DEBUG)
    set(CMAKE_BUILD_TYPE "DEBUG")
elseif()
    set(CMAKE_BUILD_TYPE "RELEASE")
endif()

link_libraries(-Wl,--as-needed -Wl,--gc-sections -Wl,--no-undefined)

add_compile_options(-Wall -Wextra -Wconversion -Wshadow)
add_compile_options(-Werror=maybe-uninitialized)
add_compile_options(-Werror=implicit-function-declaration)
add_compile_options(-ffunction-sections -fdata-sections)
add_compile_options(-Wno-nonnull-compare)
#add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)

add_compile_definitions(_GNU_SOURCE)

if(FORTIFY)
    add_compile_definitions(_FORTIFY_SOURCE=2)
    add_compile_options(-O3)
    message("FORTIFY")
endif()

if(SIMULATE_CYNAGORA)
    add_compile_definitions(SIMULATE_CYNAGORA)
endif()


###########################################################################
# Check libraries

PKG_CHECK_MODULES(libcap REQUIRED libcap)

if(WITH_SYSTEMD)
    PKG_CHECK_MODULES(libsystemd REQUIRED libsystemd>=222)
    add_subdirectory(systemd)
endif()

if(WITH_SMACK)
    PKG_CHECK_MODULES(libsmack REQUIRED libsmack)
endif()

if(WITH_SELINUX)
    PKG_CHECK_MODULES(libselinux REQUIRED libselinux)
    PKG_CHECK_MODULES(libsemanage REQUIRED libsemanage)
endif()

if(NOT SIMULATE_CYNAGORA)
    PKG_CHECK_MODULES(cynagora REQUIRED cynagora)
endif()

###########################################################################

add_subdirectory(src)
add_subdirectory(pkgconfig)
add_subdirectory(template)
add_subdirectory(script)




