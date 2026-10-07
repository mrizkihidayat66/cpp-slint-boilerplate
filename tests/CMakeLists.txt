# tests/CMakeLists.txt — Catch2 v3 unit tests for the non-UI layers.
#
# UI correctness is enforced by slint_target_sources compiling every .slint
# file at build time; these tests cover models and services logic only.

include(FetchContent)

# Static Catch2: the test exe then runs standalone (catch_discover_tests
# executes it during the build, where the DLL paths are not on PATH).
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1)
FetchContent_MakeAvailable(catch2)

add_executable(slint-boilerplate-tests
    unit/TestModels.cpp
    unit/TestServices.cpp
    unit/TestTasks.cpp)

target_link_libraries(slint-boilerplate-tests
    PRIVATE
        Catch2::Catch2WithMain
        boilerplate-core)

# catch_discover_tests runs the exe at BUILD time, before anything can set
# PATH; copy the slint runtime DLL next to it so it loads.
add_custom_command(
    TARGET slint-boilerplate-tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:slint_cpp-shared>
        $<TARGET_FILE_DIR:slint-boilerplate-tests>)

include(Catch)
catch_discover_tests(slint-boilerplate-tests)
