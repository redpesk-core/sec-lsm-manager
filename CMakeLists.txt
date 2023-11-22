###########################################################################
# Copyright 2020-2023 IoT.bzh Company
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

cmake_minimum_required(VERSION 3.14)

project(sec-lsm-manager
    VERSION 2.6.0
    DESCRIPTION "Security LSM Manager"
    HOMEPAGE_URL "https://github.com/redpesk-core/sec-lsm-manager"
    LANGUAGES C
)

include(FindPkgConfig)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(GNUInstallDirs)
include(CTest)

##########################################################################
# possible settings

set(SEC_LSM_MANAGER_VERSION ${PROJECT_VERSION})
set(SEC_LSM_MANAGER_SOVERSION ${PROJECT_VERSION_MAJOR})

option(WITH_SYSTEMD         "should include systemd compatibility" ON)
option(WITH_SMACK           "should include smack compatibility" OFF)
option(WITH_SELINUX         "should include selinux compatibility" OFF)

option(WITH_SIMULATION      "simulate cynagora, smack and selinux" OFF)
option(SIMULATE_CYNAGORA    "simulate cynagora" OFF)
option(SIMULATE_SMACK       "simulate smack" OFF)
option(SIMULATE_SELINUX     "simulate selinux" OFF)

option(FORTIFY              "fortify" ON)
option(COMPILE_TEST         "compile test" ON)
option(DEBUG                "debug" OFF)
option(COVERAGE             "coverage" OFF)

##########################################################################
# define user and group

set(USER  ${CMAKE_PROJECT_NAME} CACHE STRING "user of the daemon")
set(GROUP ${CMAKE_PROJECT_NAME} CACHE STRING "group of the daemon")
set(CYNAGORA_GROUP cynagora CACHE STRING "group of the daemon")

##########################################################################
# define variables

set(SEC_LSM_MANAGER_DATADIR         "${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}")
set(SEC_LSM_MANAGER_SOCKET_NAME     "sec-lsm-manager.socket")

set(PREFIX_PERMISSION               "urn:AGL:")

# SELINUX

set(SELINUX_MAKEFILE                "${CMAKE_INSTALL_FULL_DATADIR}/selinux/devel/Makefile")
set(SELINUX_RULES_DIR               "${SEC_LSM_MANAGER_DATADIR}/selinux-rules")
set(COMPILE_SCRIPT_DIR              "${SEC_LSM_MANAGER_DATADIR}/script")
set(COMPILE_SCRIPT_NAME             "build-module.sh")
set(TE_TEMPLATE_FILE                "app-template.te")
set(IF_TEMPLATE_FILE                "app-template.if")

set(SELINUX_FS_PATH                 "/sys/fs/selinux")
set(SIMULATION_SELINUX_POLICY_DIR   "${SEC_LSM_MANAGER_DATADIR}/selinux-simulation")

# SMACK

set(TEMPLATE_FILE                   "app-template.smack")

set(SMACK_FS_PATH                   "/sys/fs/smackfs")
set(DEFAULT_SMACK_POLICY_DIR        "/etc/smack/accesses.d")
set(SIMULATION_SMACK_POLICY_DIR     "${SEC_LSM_MANAGER_DATADIR}/smack-simulation")

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
# define compile definition variable

if(WITH_SIMULATION)
    set(SIMULATE_SMACK      ON)
    set(SIMULATE_SELINUX    ON)
    set(SIMULATE_CYNAGORA   ON)
endif()

message("\n########################### VARIABLES ###########################\n")

function(add_compile_definitions_and_print args)
    add_compile_definitions(${args})
    message("${args}")
endfunction()


add_compile_definitions_and_print(SEC_LSM_MANAGER_DATADIR="${SEC_LSM_MANAGER_DATADIR}")
add_compile_definitions_and_print(SEC_LSM_MANAGER_SOCKET_NAME="${SEC_LSM_MANAGER_SOCKET_NAME}")

# CYNAGORA

if(SIMULATE_CYNAGORA)
    add_compile_definitions_and_print(SIMULATE_CYNAGORA=1)
endif()

# SMACK

if(WITH_SMACK)
    add_compile_definitions_and_print(TEMPLATE_FILE="${TEMPLATE_FILE}")
    add_compile_definitions_and_print(SMACK_FS_PATH="${SMACK_FS_PATH}")
    if(SIMULATE_SMACK)
        add_compile_definitions_and_print(SIMULATE_SMACK=1)
        add_compile_definitions_and_print(SMACK_POLICY_DIR="${SIMULATION_SMACK_POLICY_DIR}")
    else()
        add_compile_definitions_and_print(SMACK_POLICY_DIR="${DEFAULT_SMACK_POLICY_DIR}")
    endif()
endif()

# SELINUX

if(WITH_SELINUX)
    add_compile_definitions_and_print(SELINUX_RULES_DIR="${SELINUX_RULES_DIR}")
    add_compile_definitions_and_print(TE_TEMPLATE_FILE="${TE_TEMPLATE_FILE}")
    add_compile_definitions_and_print(IF_TEMPLATE_FILE="${IF_TEMPLATE_FILE}")
    add_compile_definitions_and_print(SELINUX_FS_PATH="${SELINUX_FS_PATH}")
    if(SIMULATE_SELINUX)
        add_compile_definitions_and_print(SIMULATE_SELINUX=1)
        add_compile_definitions_and_print(SIMULATION_SELINUX_POLICY_DIR="${SIMULATION_SELINUX_POLICY_DIR}")
    endif()
endif()

##########################################################################
# define sockets, binaries and systemd directory

if(NOT CMAKE_INSTALL_FULL_RUNSTATEDIR)
    set(CMAKE_INSTALL_FULL_RUNSTATEDIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/run")
endif()

set(SOCKET_DIR "${CMAKE_INSTALL_FULL_RUNSTATEDIR}" CACHE PATH "directory path of the sockets")
add_compile_definitions(SEC_LSM_MANAGER_SOCKET_DIR="${SOCKET_DIR}")


set(BINARY_DIR "${CMAKE_INSTALL_FULL_BINDIR}" CACHE PATH "directory path of binaries")
add_compile_definitions(BINARY_DIR="${BINARY_DIR}")

if(NOT SYSTEMD_UNIT_DIR)
    set(SYSTEMD_UNIT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/systemd/system)
endif()
add_compile_definitions(SYSTEMD_NAME="${CMAKE_PROJECT_NAME}")

##########################################################################
# define compile options

message("\n######################## COMPILE OPTIONS ########################\n")

if(DEBUG)
    add_compile_definitions(DEBUG_MODE=1)
endif()

message("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

add_link_options(-Wl,--as-needed -Wl,--gc-sections -Wl,--no-undefined)

add_compile_options(
	-fdata-sections
	-ffunction-sections

	-Wall
	-Wcast-align
	-Wcast-qual
	-Wconversion
	-Wextra
	-Wmissing-prototypes
	-Wno-nonnull-compare
	-Wpedantic
	-Wpointer-arith
	-Wshadow
	-Wstrict-prototypes

	-Werror=maybe-uninitialized
	-Werror=implicit-function-declaration
)

#add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)

add_compile_definitions(_GNU_SOURCE)
message("_GNU_SOURCE")

if(FORTIFY)
    add_compile_definitions(_FORTIFY_SOURCE=2)
    add_compile_options(-O3)
    message("FORTIFY")
endif()

message("\n#################################################################\n")

###########################################################################

if(COVERAGE)
	add_compile_definitions(COVERAGE=1)
	add_compile_options(--coverage)
	add_link_options(--coverage)

	find_program(GVOVR gcovr)
	message("Adding coverage target with GCOVR=${GVOVR}")

	add_custom_target(coverage
                    COMMAND rm -rf ${CMAKE_SOURCE_DIR}/coverage && mkdir ${CMAKE_SOURCE_DIR}/coverage
                    COMMAND ${GVOVR} -r ${CMAKE_SOURCE_DIR}/src --object-directory=${CMAKE_BINARY_DIR} --html --html-details -o ${CMAKE_SOURCE_DIR}/coverage/index.html
                    COMMAND ${GVOVR} -r ${CMAKE_SOURCE_DIR}/src --object-directory=${CMAKE_BINARY_DIR}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

###########################################################################
# Check libraries

PKG_CHECK_MODULES(libcap REQUIRED libcap)

if(WITH_SYSTEMD)
    PKG_CHECK_MODULES(libsystemd REQUIRED libsystemd>=222)
    add_subdirectory(systemd)
endif()

if((NOT SIMULATE_SMACK) AND WITH_SMACK)
    PKG_CHECK_MODULES(libsmack REQUIRED libsmack)
endif()

if((NOT SIMULATE_SELINUX) AND WITH_SELINUX)
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




