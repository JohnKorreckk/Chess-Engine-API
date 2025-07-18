cmake_minimum_required(VERSION 3.16)
project(Chess_Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# wxWidgets configuration
set(wxBUILD_PRECOMP OFF)
find_package(wxWidgets REQUIRED COMPONENTS core base xrc html xml)
include(${wxWidgets_USE_FILE})

# Add ChessEngineLib first
add_subdirectory(ChessEngineLib)

# Then add Tests
add_subdirectory(Tests)

# Main executable
add_executable(${PROJECT_NAME}
        main.cpp
        ChessEngineApp.cpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ChessEngineLib
        ${wxWidgets_LIBRARIES}
)

# Configure precompiled headers AFTER target creation
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
endif()

if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Chess Engine")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.chessengine")
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()