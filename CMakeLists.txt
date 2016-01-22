cmake_minimum_required(VERSION 2.8.8)


#Déclaration du projet
project(header-expander)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)
  
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories( ${LLVM_LIBRARY_DIRS} )


set(EXECUTABLE_OUTPUT_PATH bin/  ${CMAKE_BUILD_TYPE})

execute_process(COMMAND "llvm-config --cppflags" OUTPUT_VARIABLE RESULT_VARIABLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RESULT_VARIABLE} -std=c++14")


set (LIBS
    #clang
    clangFrontend 
    clangSerialization 
    clangDriver 
    clangTooling 
    clangParse
    clangSema 
    clangAnalysis 
    clangRewriteFrontend
    clangEdit 
    clangAST 
    clangLex 
    clangBasic 
    clang
    clangCodeGen
    clangIndex
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers 
    clangStaticAnalyzerCore
    clangAnalysis
    clangRewriteFrontend 
    clangRewrite 

    #llvm
    LLVMOption 
    LLVMMC 
    LLVMObject 
    LLVMBitReader 
    LLVMCore 
    LLVMSupport 
    LLVMMCParser
 )


#Génération de la liste des fichiers sources
file(
        GLOB_RECURSE
        source_files
        src/*
)

##Déclaration de l'exécutable
add_executable(
        app
        ${source_files}
)

##Configuration de l'édition de liens
target_link_libraries(
  app
  ${LIBS}
)
