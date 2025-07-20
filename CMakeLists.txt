cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/Controller)
include_directories(${CMAKE_SOURCE_DIR}/src/Model)
include_directories(${CMAKE_SOURCE_DIR}/src/View)
include_directories(${CMAKE_SOURCE_DIR}/3rdpart/Ela/ElaWidgetTools/include)

project(TranCE_Wyatt LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2 LinguistTools REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES

    src/*.cpp
    src/*.h
)

qt_add_executable(TranCE_Wyatt
    ${PROJECT_SOURCES}
    res/res.qrc
)

# Add translation support
set(TS_FILES
    translations/TranCE_Wyatt_zh_CN.ts
    translations/TranCE_Wyatt_en_US.ts
)

qt_add_translations(TranCE_Wyatt
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE qm_files
)

add_subdirectory(3rdpart/Ela)

add_subdirectory(3rdpart/SQLiteCpp)

target_include_directories(TranCE_Wyatt PRIVATE
    src
    3rdpart/Ela/ElaWidgetTools/include)

if(WIN32)
    target_link_libraries(TranCE_Wyatt PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2
        ElaWidgetTools
        SQLiteCpp
        sqlite3
    )
    add_custom_command(TARGET TranCE_Wyatt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ElaWidgetTools>
        $<TARGET_FILE_DIR:TranCE_Wyatt>
    )
endif()

if(NOT WIN32)
    target_link_libraries(TranCE_Wyatt PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2
        ElaWidgetTools
        SQLiteCpp
        sqlite3
        pthread
        dl
    )
endif()

function(auto_generate_qrc)
    file(GLOB_RECURSE IMAGE_FILES
        ${CMAKE_SOURCE_DIR}/res/image/*.png
        ${CMAKE_SOURCE_DIR}/res/image/*.jpg
        ${CMAKE_SOURCE_DIR}/res/image/*.svg
        ${CMAKE_SOURCE_DIR}/res/image/*.jpeg
        ${CMAKE_SOURCE_DIR}/res/image/*.gif
        ${CMAKE_SOURCE_DIR}/res/image/*.bmp
        ${CMAKE_SOURCE_DIR}/res/image/*.webp
        ${CMAKE_SOURCE_DIR}/res/image/*.ico
        ${CMAKE_SOURCE_DIR}/res/image/*.svg
    )

    set(QRC_CONTENT "<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n<qresource>\n")

    foreach(IMAGE_FILE ${IMAGE_FILES})
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/res" "${IMAGE_FILE}")
        string(REPLACE "\\" "/" REL_PATH "${REL_PATH}")
        set(QRC_CONTENT "${QRC_CONTENT}    <file>${REL_PATH}</file>\n")
    endforeach()

    set(QRC_CONTENT "${QRC_CONTENT}</qresource>\n</RCC>")

    file(WRITE "${CMAKE_SOURCE_DIR}/res/res.qrc" "${QRC_CONTENT}")
endfunction()

auto_generate_qrc()