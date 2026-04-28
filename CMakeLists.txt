cmake_minimum_required(VERSION 3.22)

project(can_gateway_uds_workspace NONE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(FIRMWARE_BOARDS
    board_a_engine
    board_b_gateway
    board_c_uds
)

foreach(board IN LISTS FIRMWARE_BOARDS)
    set(board_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/firmware/${board})
    set(board_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/${board})
    set(board_toolchain ${board_source_dir}/cmake/gcc-arm-none-eabi.cmake)

    add_custom_target(configure_${board}
        COMMAND ${CMAKE_COMMAND}
            -S ${board_source_dir}
            -B ${board_binary_dir}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE=${board_toolchain}
        COMMENT "Configure ${board}"
        VERBATIM
    )

    add_custom_target(build_${board}
        COMMAND ${CMAKE_COMMAND} --build ${board_binary_dir}
        DEPENDS configure_${board}
        COMMENT "Build ${board}"
        VERBATIM
    )
endforeach()

add_custom_target(configure_firmware
    DEPENDS
        configure_board_a_engine
        configure_board_b_gateway
        configure_board_c_uds
)

add_custom_target(build_firmware
    DEPENDS
        build_board_a_engine
        build_board_b_gateway
        build_board_c_uds
)
