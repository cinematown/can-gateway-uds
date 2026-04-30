cmake_minimum_required(VERSION 3.16)

project(engine_sim_gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    serialconnection.cpp
    serialconnection.h
    mainwindow.cpp
    mainwindow.h
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(engine_sim
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(engine_sim
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(engine_sim PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(engine_sim)
endif()
