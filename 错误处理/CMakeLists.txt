cmake_minimum_required(VERSION 3.15)
project()

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(
        GrammerAnalyzer.cpp
        GrammerAnalyzer.h
        LexicalAnalyzer.cpp
        LexicalAnalyzer.h
        main.cpp
        SymbolTable.cpp
        SymbolTable.h
        TypeDefine.h)
