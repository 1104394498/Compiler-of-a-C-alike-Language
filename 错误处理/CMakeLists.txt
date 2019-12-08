cmake_minimum_required(VERSION 3.15)
project(Project2__3_)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Project2__3_
        Error_GrammarAnalyzer.cpp
        Error_GrammarAnalyzer.h
        Error_LexicalAnalyzer.cpp
        Error_LexicalAnalyzer.h
        main.cpp
        Error_TypeDefine.h Error_SymbolTable.cpp Error_SymbolTable.h)
