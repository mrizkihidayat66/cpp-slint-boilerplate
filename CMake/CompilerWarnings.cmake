# CompilerWarnings.cmake — central warning policy.
#
# Usage:
#   include(CMake/CompilerWarnings.cmake)
#   target_apply_warnings(<target>)

function(target_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4          # high warning level
            /permissive-  # strict standards conformance
            /w14242       # conversion from 'type' to 'type', possible loss of data
            /w14254       # operator +: conversion from 'type' to 'type', possible loss of data
            /w14263       # member function does not override any base class virtual member function
            /w14265       # class has virtual functions, but destructor is not virtual
            /w14287       # unsigned/negative constant mismatch
            /w14296       # expression is always false
            /w14311       # variable truncation
            /w14545       # expression before comma evaluates to a function
            /w14546       # function call before comma, missing argument list
            /w14547       # operator: operator before/after has no effect
            /w14549       # operator before comma has no effect
            /w14555       # expression has no effect
            /w14619       # pragma warning: there is no warning number
            /w14640       # thread-unsafe static member initialization
            /w14826       # conversion is sign-extended
            /w14905       # wide string literal cast
            /w14906       # string literal cast
            /w14928)      # illegal copy-initialization
        # Warnings-as-errors in CI (and locally via -DWARNING_AS_ERROR=ON).
        # Third-party code (Slint via FetchContent) is unaffected because
        # this is applied per-target, not globally.
        option(WARNING_AS_ERROR "Treat compiler warnings as errors" OFF)
        if(WARNING_AS_ERROR OR DEFINED ENV{CI})
            target_compile_options(${target} PRIVATE /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wunused
            -Woverloaded-virtual)
    endif()
endfunction()
