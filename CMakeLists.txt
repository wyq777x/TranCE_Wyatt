cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/Controller)
include_directories(${CMAKE_SOURCE_DIR}/src/Model)
include_directories(${CMAKE_SOURCE_DIR}/src/View)

project(TranCE_Wyatt LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2 REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES

    src/*.cpp
    src/*.h
)

qt_add_executable(TranCE_Wyatt
    ${PROJECT_SOURCES}
)

add_subdirectory(3rdpart/Ela)

target_include_directories(TranCE_Wyatt PRIVATE
    src
    3rdpart/Ela/ElaWidgetTools/include)

target_link_libraries(TranCE_Wyatt PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    ElaWidgetTools)
