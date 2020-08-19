###########################################################################
# Copyright 2020 IoT.bzh
#
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

cmake_minimum_required(VERSION 3.3)

list(TRANSFORM LIBCORE_SOURCES PREPEND "../")


set(TESTS_SOURCES
    tests.c
    test_utils.c
    test_paths.c
    test_policies.c
    test_secure_app.c
)


add_executable(tests ${TESTS_SOURCES} ${LIBCORE_SOURCES})
target_link_libraries(tests cunit)
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


