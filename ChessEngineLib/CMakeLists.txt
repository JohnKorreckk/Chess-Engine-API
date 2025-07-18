cmake_minimum_required(VERSION 3.16)

add_library(ChessEngineLib STATIC
        Board.cpp
        Board.h
        Engine.cpp
        Engine.h
)

target_include_directories(ChessEngineLib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Configure precompiled headers AFTER target creation
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(ChessEngineLib PRIVATE pch.h)
endif()

target_compile_features(ChessEngineLib PUBLIC cxx_std_23)