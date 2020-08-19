###########################################################################
# Copyright 2020 IoT.bzh
#
# author: José Bollo <jose.bollo@iot.bzh>
# author: Arthur Guyader <arthur.guyader@iot.bzh>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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




