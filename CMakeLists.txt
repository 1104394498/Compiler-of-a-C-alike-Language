cmake_minimum_required(VERSION 3.15)
project(MIPScode)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(MIPScode GrammerAnalyzer.cpp
        GrammerAnalyzer.h
        LexicalAnalyzer.cpp
        LexicalAnalyzer.h
        main.cpp
        TypeDefine.h SymbolTable.cpp SymbolTable.h Optimizer.cpp Optimizer.h MipsGenerator.cpp MipsGenerator.h Error_GrammarAnalyzer.cpp Error_GrammarAnalyzer.h Error_LexicalAnalyzer.cpp Error_LexicalAnalyzer.h Error_TypeDefine.h)