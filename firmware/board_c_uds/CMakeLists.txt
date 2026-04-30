cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32f429zi_cli)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# ========================================================
set(BOARD_ROLE "BOARD_C")
# 

# 1. 공통 소스 파일 묶기
file(GLOB_RECURSE COMMON_SOURCES "common/*.c")

# 2. 보드별 소스 파일 선택
if(BOARD_ROLE STREQUAL "TX_ENGINE")
    file(GLOB_RECURSE BOARD_SOURCES "board_a_engine/*.c")
    message(STATUS "[Build Mode] 보드 A (Engine) 빌드")
elseif(BOARD_ROLE STREQUAL "GATEWAY")
    file(GLOB_RECURSE BOARD_SOURCES "board_b_gateway/*.c")
    message(STATUS "[Build Mode] 보드 B (Gateway) 빌드")
else()
    file(GLOB_RECURSE BOARD_SOURCES "board_c_uds/*.c")
    message(STATUS "[Build Mode] 보드 C (UDS Server) 빌드")
endif()

# 3. 소스 파일 추가
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_SOURCES}
    ${BOARD_SOURCES}
)

# 4. 헤더 파일 경로(Include paths) 추가
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    common
    board_c_uds
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
