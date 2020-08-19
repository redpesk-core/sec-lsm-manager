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

# Compilation Mode (DEBUG, RELEASE)
#----------------------------------
set(CMAKE_BUILD_TYPE "DEBUG")

set(LIBCORE_SOURCES
    log.c
    utils.c
    paths.c
    policies.c
    cynagora-interface.c
    secure-app.c
    security-manager-operation.c
)

if(WITH_SMACK)
    set(LIBCORE_SOURCES ${LIBCORE_SOURCES} smack-label.c smack-template.c smack.c)
endif()
if(WITH_SELINUX)
    set(LIBCORE_SOURCES ${LIBCORE_SOURCES} selinux-label.c selinux.c selinux-template.c selinux-compile.c)
endif()

set(SERVER_SOURCES
    socket.c
    pollitem.c
    prot.c
    security-manager-protocol.c
    security-manager-server.c
    main-security-managerd.c
)

set(LIBCLI_SOURCES
    prot.c
    socket.c
    log.c
    paths.c
    security-manager-protocol.c
    security-manager.c
)

add_compile_definitions(_GNU_SOURCE)


if(OPTIMIZE)
# Beware that this can incorporate unwanted behavior !
add_compile_definitions(_FORTIFY_SOURCE=2)
add_compile_options(-Og)
message("OPTIMIZE")
endif()

add_compile_options(-Og)

if(SIMULATE_CYNAGORA)
    add_compile_definitions(SIMULATE_CYNAGORA)
    set(LIBCORE_SOURCES ${LIBCORE_SOURCES} simulation/cynagora/cynagora.c)
endif()

if(DEVELOPEMENT)
    add_compile_definitions(DEVELOPEMENT)
endif()


###########################################
# build and install libsecurity-manager-core
###########################################
add_library(security-manager-core SHARED ${LIBCORE_SOURCES})
target_include_directories(security-manager-core PUBLIC .)
set_target_properties(security-manager-core PROPERTIES
    VERSION ${SECURITY_MANAGER_VERSION}
    SOVERSION ${SECURITY_MANAGER_SOVERSION}
    LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export-security-manager-core.map
)
if(NOT SIMULATE_MAC)
    if(WITH_SMACK)
        target_compile_definitions(security-manager-core PRIVATE WITH_SMACK)
        target_link_libraries(security-manager-core ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
        target_include_directories(security-manager-core PRIVATE ${libsmack_INCLUDE_DIRS})
        target_compile_options(security-manager-core PRIVATE ${libsmack_CFLAGS})
    endif()
    if(WITH_SELINUX)
        target_compile_definitions(security-manager-core PRIVATE WITH_SELINUX)
        target_link_libraries(security-manager-core ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
        target_include_directories(security-manager-core PRIVATE ${libselinux_INCLUDE_DIRS})
        target_compile_options(security-manager-core PRIVATE ${libselinux_CFLAGS})
        target_link_libraries(security-manager-core ${libsemanage_LDFLAGS} ${libsemanage_LINK_LIBRARIES})
        target_include_directories(security-manager-core PRIVATE ${libsemanage_INCLUDE_DIRS})
        target_compile_options(security-manager-core PRIVATE ${libsemanage_CFLAGS})
    endif()

else()
    target_compile_definitions(security-manager-core PRIVATE SIMULATE_MAC)
endif()

if(NOT SIMULATE_CYNAGORA)
    target_link_libraries(security-manager-core ${cynagora_LDFLAGS} ${libselinux_LINK_LIBRARIES})
    target_include_directories(security-manager-core PRIVATE ${cynagora_INCLUDE_DIRS})
    target_compile_options(security-manager-core PRIVATE ${cynagora_CFLAGS})
endif()


install(TARGETS security-manager-core LIBRARY DESTINATION ${LIBDIR})


###########################################
# build and install libsecurity-manager-client
###########################################
add_library(security-manager SHARED ${LIBCLI_SOURCES})
set_target_properties(security-manager PROPERTIES
    VERSION ${SECURITY_MANAGER_VERSION}
    SOVERSION ${SECURITY_MANAGER_SOVERSION}
    LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export-security-manager.map
)
install(TARGETS security-manager LIBRARY DESTINATION ${LIBDIR})
install(FILES security-manager.h DESTINATION ${INCLUDEDIR})


###########################################
# build and install security-managerd
###########################################
add_executable(security-managerd ${SERVER_SOURCES})
target_compile_definitions(security-managerd PRIVATE
    DEFAULT_SOCKET_DIR="${DEFAULT_SOCKET_DIR}"
)
if(WITH_SYSTEMD)
    target_compile_definitions(security-managerd PRIVATE WITH_SYSTEMD)
    target_link_libraries(security-managerd ${libsystemd_LDFLAGS} ${libsystemd_LINK_LIBRARIES})
    target_include_directories(security-managerd PRIVATE ${libsystemd_INCLUDE_DIRS})
    target_compile_options(security-managerd PRIVATE ${libsystemd_CFLAGS})
endif()
target_link_libraries(security-managerd security-manager-core cap)
install(TARGETS security-managerd
    RUNTIME DESTINATION ${BINDIR})



###########################################
# build and install security-manager-cmd
###########################################
add_executable(security-manager-cmd main-security-manager-cmd.c log.c utils.c)
target_link_libraries(security-manager-cmd security-manager)
install(TARGETS security-manager-cmd
        RUNTIME DESTINATION ${BINDIR})
if(NOT SIMULATE_MAC)
    if(WITH_SMACK)
        target_compile_definitions(security-manager-cmd PRIVATE WITH_SMACK)
        target_link_libraries(security-manager-cmd ${libsmack_LDFLAGS} ${libsmack_LINK_LIBRARIES})
        target_include_directories(security-manager-cmd PRIVATE ${libsmack_INCLUDE_DIRS})
        target_compile_options(security-manager-cmd PRIVATE ${libsmack_CFLAGS})
    endif()
    if(WITH_SELINUX)
        target_compile_definitions(security-manager-cmd PRIVATE WITH_SELINUX)
        target_link_libraries(security-manager-cmd ${libselinux_LDFLAGS} ${libselinux_LINK_LIBRARIES})
        target_include_directories(security-manager-cmd PRIVATE ${libselinux_INCLUDE_DIRS})
        target_compile_options(security-manager-cmd PRIVATE ${libselinux_CFLAGS})
    endif()
else()
    target_compile_definitions(security-manager-cmd PRIVATE SIMULATE_MAC)
endif()

##############
# build tests
##############

if(COMPILE_TEST)
add_subdirectory(tests)
endif()