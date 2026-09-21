cmake_minimum_required(VERSION 3.5)
project(kv-engine VERSION 0.2.0 LANGUAGES C)

# Force standard C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS OFF)

# Fetch External dependency (xxHash)
include(FetchContent)

FetchContent_Declare(
    xxhash
    GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
    GIT_TAG v0.8.2
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(xxhash)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/cofyc/argparse.git
    GIT_TAG 4e30aba50340af7f2d3932492c4bffdef7669db8
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(argparse)

# Apply warning flags to a target
function(kv_engine_enable_warnings target)
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endfunction()

# Library
add_library(kv_engine_lib
    lib/src/hashmap.c
    lib/src/hashmap_iterator.c
    lib/src/db.c
    lib/src/json.c
    ${xxhash_SOURCE_DIR}/xxhash.c
)
add_library(kv_engine::lib ALIAS kv_engine_lib)

target_include_directories(kv_engine_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(kv_engine_lib PRIVATE ${xxhash_SOURCE_DIR})

kv_engine_enable_warnings(kv_engine_lib)

# Application executable
add_executable(kv_engine_app
    app/src/main.c
)
set_target_properties(kv_engine_app PROPERTIES OUTPUT_NAME "kv-engine")
target_link_libraries(kv_engine_app PRIVATE kv_engine::lib argparse_static)
kv_engine_enable_warnings(kv_engine_app)

# Enable testing
enable_testing()

find_program(VALGRIND_BIN valgrind REQUIRED)

# Set Valgrind as the memory checker for CTest
set(MEMORYCHECK_COMMAND "${VALGRIND_BIN}")

# Customize Valgrind flags
set(MEMORYCHECK_COMMAND_OPTIONS
    "--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"
)

# Include CTest module to generate CTestCustom.cmake / DartConfiguration.tcl
include(CTest)

# Function to set up a unit test executable with asserts kept active (Release
# defines NDEBUG, which would otherwise compile out every assert).
function(kv_engine_add_test name source)
    add_executable(kv_engine_${name} ${source})
    set_target_properties(kv_engine_${name} PROPERTIES OUTPUT_NAME "${name}")
    target_link_libraries(kv_engine_${name} PRIVATE kv_engine::lib)
    target_compile_options(kv_engine_${name} PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-UNDEBUG>
    )
    kv_engine_enable_warnings(kv_engine_${name})
    add_test(NAME kv_engine_${name} COMMAND kv_engine_${name})
endfunction()

kv_engine_add_test(test_hashmap lib/unit_tests/test_hashmap.c)
kv_engine_add_test(test_hashmap_iterator lib/unit_tests/test_hashmap_iterator.c)
kv_engine_add_test(test_db lib/unit_tests/test_db.c)
kv_engine_add_test(example_person lib/examples/person.c)
