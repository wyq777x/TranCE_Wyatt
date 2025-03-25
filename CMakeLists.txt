cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

project(TranCE_Wyatt LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2 REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES

    src/*.cpp
    src/*.h
    src/*.ui
    src/*.qml
    src/*.qrc
)

qt_add_executable(TranCE_Wyatt
    ${PROJECT_SOURCES}
)

add_subdirectory(src)

add_subdirectory(3rdpart/Ela)

target_link_libraries(TranCE_Wyatt PRIVATE Qt6::Core Qt6::Widgets Qt6::Quick Qt6::QuickControls2)
