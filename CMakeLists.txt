cmake_minimum_required(VERSION 3.16)
project(Chess_Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add ChessEngineLib first
add_subdirectory(ChessEngineLib)

# Then add Tests
add_subdirectory(Tests)

# ========================
# 📝 PYBIND11 CONFIGURATION
# ========================

include(FetchContent)

# Force CMake to use Homebrew Python
set(Python3_ROOT_DIR "/opt/homebrew/opt/python@3.12")
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)


FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.11.1  # or latest stable
)
FetchContent_MakeAvailable(pybind11)

# ========================
# PYBIND11 MODULE TARGET
# ========================

pybind11_add_module(chessengine
        bindings.cpp
        ChessEngineLib/Engine.cpp
        ChessEngineLib/Board.cpp
        # Add other source files needed by Engine
)

target_include_directories(chessengine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ChessEngineLib
)

target_link_libraries(chessengine PRIVATE ChessEngineLib)

# ========================
# MAIN EXECUTABLE
# ========================

add_executable(${PROJECT_NAME}
        main.cpp
        ChessEngineApp.cpp
        # DO NOT include bindings.cpp here if only for pybind11
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ChessEngineLib
)


# Configure precompiled headers AFTER target creation
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
endif()

# macOS bundle settings
if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Chess Engine")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.chessengine")
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()
