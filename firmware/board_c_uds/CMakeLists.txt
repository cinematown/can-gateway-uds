cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME board_c_uds)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME})
add_subdirectory(cmake/stm32cubemx)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(COMMON_DIR ${REPO_ROOT}/common)
set(BOARD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_DIR}/can_bsp.c
    ${COMMON_DIR}/cli.c
    ${COMMON_DIR}/uart.c
    ${BOARD_SRC_DIR}/ap_main.c
    ${BOARD_SRC_DIR}/uds_cli_cmd.c
    ${BOARD_SRC_DIR}/uds_server.c
    ${BOARD_SRC_DIR}/uds_service.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_DIR}
    ${BOARD_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    BOARD_C_UDS
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
)
