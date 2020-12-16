###########################################################################
# Copyright 2020 IoT.bzh Company
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

project(security-manager
	DESCRIPTION "Security Manager (MAC)"
	HOMEPAGE_URL ""
	VERSION 0.1
	LANGUAGES C
)

include(FindPkgConfig)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(GNUInstallDirs)
include(CTest)

###########################################################################
# Define install directory

SET(LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}" CACHE PATH "library dir")
SET(BINDIR "${CMAKE_INSTALL_FULL_BINDIR}" CACHE PATH "binary dir")
SET(INCLUDEDIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}" CACHE PATH "include dir")
SET(DATADIR "${CMAKE_INSTALL_FULL_DATADIR}" CACHE PATH "data dir")
SET(SYSTEMD_UNIT_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/systemd/system" CACHE PATH "systemd dir")


if(NOT CMAKE_INSTALL_FULL_RUNSTATEDIR)
	set(CMAKE_INSTALL_FULL_RUNSTATEDIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/run")
endif()

###########################################################################
# possible settings

set(SECURITY_MANAGER_VERSION ${PROJECT_VERSION})
set(SECURITY_MANAGER_SOVERSION ${PROJECT_VERSION_MAJOR})

option(WITH_SYSTEMD         "should include systemd compatibility" ON)
option(WITH_SMACK           "should include smack compatibility" ON)
option(WITH_SELINUX         "should include selinux compatibility" OFF)

option(SIMULATE_MAC		    "simulate mandatory access control" OFF)
option(SIMULATE_CYNAGORA	"simulate cynagora" OFF)

option(OPTIMIZE	"optimize" OFF)
option(COMPILE_TEST	"compile test" OFF)



set(USER  security-manager CACHE STRING "user of the daemon")
set(GROUP security-manager CACHE STRING "group of the daemon")

set(DEFAULT_SOCKET_DIR "${CMAKE_INSTALL_FULL_RUNSTATEDIR}"
	CACHE PATH "directory path of the sockets")


option(DEVELOPEMENT         "use for developement process, need remove at the end !!!!" ON)

###########################################################################

link_libraries(-Wl,--as-needed -Wl,--gc-sections -Wl,--no-undefined)

add_compile_options(-Wall -Wextra -Wconversion -Wshadow)
add_compile_options(-Werror=maybe-uninitialized)
add_compile_options(-Werror=implicit-function-declaration)
add_compile_options(-ffunction-sections -fdata-sections)
#add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)

###########################################################################

PKG_CHECK_MODULES(libcap REQUIRED libcap)

if(WITH_SYSTEMD)
	PKG_CHECK_MODULES(libsystemd REQUIRED libsystemd>=222)
	add_subdirectory(systemd)
endif()

if(NOT SIMULATE_MAC)
    if(WITH_SMACK)
        PKG_CHECK_MODULES(libsmack REQUIRED libsmack)
    endif()

    if(WITH_SELINUX)
        PKG_CHECK_MODULES(libselinux REQUIRED libselinux)
		PKG_CHECK_MODULES(libsemanage REQUIRED libsemanage)
    endif()
endif()

if(NOT SIMULATE_CYNAGORA)
    PKG_CHECK_MODULES(cynagora REQUIRED cynagora)
endif()

add_subdirectory(src)
add_subdirectory(pkgconfig)
add_subdirectory(template)
add_subdirectory(script)




