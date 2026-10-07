# tests/CMakeLists.txt — Catch2 v3 unit tests for the non-UI layers.
#
# UI correctness is enforced by slint_target_sources compiling every .slint
# file at build time; these tests cover models and services logic only.

include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1)
FetchContent_MakeAvailable(catch2)

add_executable(slint-boilerplate-tests
    unit/TestModels.cpp
    unit/TestServices.cpp)

target_include_directories(slint-boilerplate-tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR})

target_link_libraries(slint-boilerplate-tests
    PRIVATE
        Catch2::Catch2WithMain
        Slint::Slint)

# The test executable compiles the same .slint sources so the generated
# structs used by the toSlint adapters exist in this translation context.
slint_target_sources(slint-boilerplate-tests PRIVATE ${CMAKE_SOURCE_DIR}/src/ui/app-window.slint)

include(Catch)
catch_discover_tests(slint-boilerplate-tests)
