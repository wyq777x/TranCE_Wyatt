cmake_minimum_required(VERSION 3.21)

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

# AI mode: QtWebEngine is an optional dependency. When unavailable the app
# degrades gracefully and opens the sidecar web UI in the system browser.
option(TRANCE_AI_WEBENGINE "Embed the AI mode web UI with QtWebEngine" ON)
if(TRANCE_AI_WEBENGINE)
    find_package(Qt6 QUIET COMPONENTS WebEngineWidgets)
    if(Qt6WebEngineWidgets_FOUND)
        message(STATUS "AI mode: QtWebEngine found - embedding web UI")
        set(TRANCE_AI_HAVE_WEBENGINE ON)
    else()
        message(STATUS "AI mode: Qt6WebEngineWidgets NOT found - AI page will fall back to the system browser")
    endif()
endif()

# ---------------------------------------------------------------------------
# AI sidecar environment (Python venv + web frontend build)
#
# The sidecar is a hard requirement for AI mode, so missing tooling is a
# fatal configure error. Both steps are self-healing: they only run when
# their outputs (.venv / web/dist) are absent, so incremental builds pay
# nothing. Re-run either by deleting the output directory.
#
# Set TRANCE_AI_SETUP_SIDECAR=OFF to skip entirely (e.g. packaging builds
# that bundle a PyInstaller sidecar instead).
# ---------------------------------------------------------------------------
option(TRANCE_AI_SETUP_SIDECAR "Create the sidecar venv and build the web UI during the build" ON)

if(TRANCE_AI_SETUP_SIDECAR)
    # --- Python interpreter (venv creation needs >= 3.11 per the sidecar README)
    find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)
    set(TRANCE_PYTHON "${Python3_EXECUTABLE}")

    # --- npm for the web frontend build
    find_program(NPM_EXECUTABLE npm REQUIRED)

    set(TRANCE_SIDECAR_SERVER_DIR "${CMAKE_SOURCE_DIR}/aisidecar/server")
    set(TRANCE_SIDECAR_WEB_DIR "${CMAKE_SOURCE_DIR}/aisidecar/web")

    if(WIN32)
        set(TRANCE_VENV_PYTHON "${TRANCE_SIDECAR_SERVER_DIR}/.venv/Scripts/python.exe")
    else()
        set(TRANCE_VENV_PYTHON "${TRANCE_SIDECAR_SERVER_DIR}/.venv/bin/python")
    endif()

    # Self-healing marker: the venv is considered ready when its pip can
    # import the requirements (the .venv dir alone is not enough - a
    # half-finished `python -m venv` has no site-packages yet).
    set(TRANCE_VENV_STAMP "${TRANCE_SIDECAR_SERVER_DIR}/.venv/.trance-deps-stamp")

    add_custom_command(
        OUTPUT "${TRANCE_VENV_STAMP}"
        COMMAND "${TRANCE_PYTHON}" -m venv "${TRANCE_SIDECAR_SERVER_DIR}/.venv"
        COMMAND "${TRANCE_VENV_PYTHON}" -m pip install --upgrade pip
        COMMAND "${TRANCE_VENV_PYTHON}" -m pip install -r "${TRANCE_SIDECAR_SERVER_DIR}/requirements.txt"
        COMMAND "${CMAKE_COMMAND}" -E touch "${TRANCE_VENV_STAMP}"
        DEPENDS "${TRANCE_SIDECAR_SERVER_DIR}/requirements.txt"
        COMMENT "AI sidecar: creating venv and installing Python dependencies"
        VERBATIM
    )

    # Self-healing web build: dist/index.html is the output marker. Rebuild
    # when any source file changes.
    file(GLOB TRANCE_WEB_SOURCES
        "${TRANCE_SIDECAR_WEB_DIR}/src/*"
        "${TRANCE_SIDECAR_WEB_DIR}/src/**/*"
    )
    add_custom_command(
        OUTPUT "${TRANCE_SIDECAR_WEB_DIR}/dist/index.html"
        COMMAND "${NPM_EXECUTABLE}" install
        COMMAND "${NPM_EXECUTABLE}" run build
        WORKING_DIRECTORY "${TRANCE_SIDECAR_WEB_DIR}"
        DEPENDS ${TRANCE_WEB_SOURCES}
                "${TRANCE_SIDECAR_WEB_DIR}/package.json"
                "${TRANCE_SIDECAR_WEB_DIR}/package-lock.json"
        COMMENT "AI sidecar: building web frontend (npm install + vite build)"
        VERBATIM
    )

    # Anchor both steps to the default target so they run automatically
    # with every `cmake --build`, before the app target.
    add_custom_target(trance-sidecar-setup ALL
        DEPENDS "${TRANCE_VENV_STAMP}"
                "${TRANCE_SIDECAR_WEB_DIR}/dist/index.html"
    )
endif()

file(GLOB_RECURSE PROJECT_SOURCES

    src/*.cpp
    src/*.h
)

qt_add_executable(TranCE_Wyatt
    ${PROJECT_SOURCES}
    res/res.qrc
)

add_subdirectory(3rdpart/Ela)

add_subdirectory(3rdpart/SQLiteCpp)

target_include_directories(TranCE_Wyatt PRIVATE
    src
    3rdpart/Ela/ElaWidgetTools/include)

if(TRANCE_AI_HAVE_WEBENGINE)
    target_link_libraries(TranCE_Wyatt PRIVATE Qt6::WebEngineWidgets)
    target_compile_definitions(TranCE_Wyatt PRIVATE TRANCE_AI_WEBENGINE=1)
endif()

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

    add_custom_command(TARGET TranCE_Wyatt POST_BUILD
        COMMAND windeployqt --no-translations --no-quick-import $<TARGET_FILE:TranCE_Wyatt>
        COMMENT "auto set Qt dll to output dir"
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