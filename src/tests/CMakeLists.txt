###########################################################################
# Copyright 2020-2023 IoT.bzh Company
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

message("\n######################## COMPILE TESTS ########################\n")

if(SIMULATE_SMACK OR SIMULATE_SELINUX)
    message("WARNING : Tests can't work when SMACK or SELinux are simulated\n")
endif()

PKG_CHECK_MODULES(check REQUIRED check)

set(CMAKE_BUILD_TYPE "DEBUG")
message("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}\n")

if(NOT check_FOUND)
    message( FATAL_ERROR "check required")
endif()

set(TEST_SOURCES
    setup-tests.c
    test-prot.c
    test-paths.c
    test-plugs.c
    test-permissions.c
    test-secure-app.c
    test-utils.c
)

if(NOT SIMULATE_CYNAGORA)
    set(TEST_SOURCES ${TEST_SOURCES} test-cynagora.c)
endif()

if(WITH_SMACK)
    set(TEST_SOURCES_smack ${TEST_SOURCES} test-smack.c test-smack-label.c)
endif()

if(WITH_SELINUX)
    set(TEST_SOURCES_selinux ${TEST_SOURCES} test-selinux.c test-selinux-template.c)
endif()

function(build_tests_for_mac MAC_NAME)
    message("[*] Create : tests-${MAC_NAME}")
    add_executable(tests-${MAC_NAME} ${TEST_SOURCES} ${TEST_SOURCES_${MAC_NAME}})
    target_link_libraries(tests-${MAC_NAME} PRIVATE cap common-lib socket-std app-${MAC_NAME})

    target_compile_options(tests-${MAC_NAME} PRIVATE --coverage)
    target_link_libraries(tests-${MAC_NAME} PRIVATE --coverage)

    target_link_libraries(tests-${MAC_NAME} PRIVATE ${check_LDFLAGS} ${check_LINK_LIBRARIES})
    target_include_directories(tests-${MAC_NAME} PRIVATE ${check_INCLUDE_DIRS})
    target_include_directories(tests-${MAC_NAME} PRIVATE ..)
    message("[-] Link : check")

    add_test(tests-${MAC_NAME} ${CMAKE_CURRENT_BINARY_DIR}/tests-${MAC_NAME} test-${MAC_NAME}.tap)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/tests-${MAC_NAME} DESTINATION ${SEC_LSM_MANAGER_DATADIR}/tests/)
    message("[x] Done : tests-${MAC_NAME}\n")
endfunction()

if(WITH_SMACK)
    build_tests_for_mac("smack")
endif()

if(WITH_SELINUX)
    build_tests_for_mac("selinux")
endif()

find_program(GVOVR gcovr)
message("GCOVR=${GVOVR}")

add_custom_target(coverage
                    COMMAND rm -rf ${CMAKE_BINARY_DIR}/coverage && mkdir ${CMAKE_BINARY_DIR}/coverage
                    COMMAND ${GVOVR} --exclude ${CMAKE_SOURCE_DIR}/src/mustach -r ${CMAKE_SOURCE_DIR}/src --object-directory=${CMAKE_BINARY_DIR} --html --html-details -o ${CMAKE_BINARY_DIR}/coverage/index.html
                    COMMAND ${GVOVR} --exclude ${CMAKE_SOURCE_DIR}/src/mustach -r ${CMAKE_SOURCE_DIR}/src --object-directory=${CMAKE_BINARY_DIR}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
