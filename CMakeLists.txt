cmake_minimum_required(VERSION 2.6)

#Déclaration du projet
project(header-expander)

set(EXECUTABLE_OUTPUT_PATH bin/    ${CMAKE_BUILD_TYPE})

add_definitions(-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Wall -fexceptions -fstack-protector --param=ssp-buffer-size=4 -grecord-gcc-switches  -m64 -mtune=generic -fomit-frame-pointer -std=c++11 -fvisibility-inlines-hidden -fno-exceptions -fPIC -Woverloaded-virtual -Wcast-qual")

#include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )

set (LIBS
    clangFrontend 
    clangSerialization 
    clangDriver 
    clangTooling 
    clangParse
    clangSema 
    clangAnalysis 
    clangRewriteFrontend
    clangRewriteCore 
    clangEdit 
    clangAST 
    clangLex 
    clangBasic 
    clang 
    llvm
    LLVMOption 
    LLVMMC 
    LLVMObject 
    LLVMBitReader 
    LLVMAsmParser
    LLVMCore 
    LLVMSupport 
    LLVM-3.3
    z pthread ffi dl m 
)


#Génération de la liste des fichiers sources
file(
        GLOB_RECURSE
        source_files
        src/*
)



#Déclaration de l'exécutable
add_executable(
        app
        ${source_files}
)

#Configuration de l'édition de liens
target_link_libraries(
        app
        ${LIBS}
)
