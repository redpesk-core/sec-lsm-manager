###########################################################################
# Copyright 2020 IoT.bzh Company
#
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

cmake_minimum_required(VERSION 3.3)

PKG_CHECK_MODULES(check REQUIRED check)

if(NOT check_FOUND)
    message( FATAL_ERROR "libcheck required")
endif()

list(TRANSFORM SERVER_SOURCES PREPEND "../")

set(TESTS_SOURCES
    tests.c
    test_utils.c
    test_paths.c
    test_permissions.c
    test_secure_app.c
)

if(WITH_SMACK)
    set(TESTS_SOURCES ${TESTS_SOURCES} test_smack.c)
endif()

add_executable(tests ${TESTS_SOURCES} ${SERVER_SOURCES})
target_link_libraries(tests security-manager)
target_link_libraries(tests ${check_LDFLAGS} ${check_LINK_LIBRARIES})
target_include_directories(tests PRIVATE ${check_INCLUDE_DIRS})
target_compile_options(tests PRIVATE ${check_CFLAGS})

add_test (tests ${CMAKE_CURRENT_BINARY_DIR}/tests)

if(WITH_SMACK)
    target_compile_definitions(tests PRIVATE WITH_SMACK)
    target_link_libraries(tests ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
    target_include_directories(tests PRIVATE ${libsmack_INCLUDE_DIRS})
    target_compile_options(tests PRIVATE ${libsmack_CFLAGS})
endif()
if(WITH_SELINUX)
    target_compile_definitions(tests PRIVATE WITH_SELINUX)
    target_link_libraries(tests ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(tests PRIVATE ${libselinux_INCLUDE_DIRS})
    target_compile_options(tests PRIVATE ${libselinux_CFLAGS})
    target_link_libraries(tests ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
    target_include_directories(tests PRIVATE ${libsemanage_INCLUDE_DIRS})
    target_compile_options(tests PRIVATE ${libsemanage_CFLAGS})
endif()
if(NOT SIMULATE_CYNAGORA)
    target_link_libraries(tests ${cynagora_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(tests PRIVATE ${cynagora_INCLUDE_DIRS})
    target_compile_options(tests PRIVATE ${cynagora_CFLAGS})
endif()


