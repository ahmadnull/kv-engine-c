default: workflow

preset := "ninja-multi"
target := "all"
exe := "kv-engine"
clean := "0"

release := "0"
config := if release == "1" {"Release"} else {"Debug"}

verbose := "0"
verbose_flag := if verbose == "1" { "--verbose" } else { "" }

memcheck := "0"
memcheck_command := if memcheck == "1" {"valgrind"} else { "" }
memcheck_args := if memcheck == "1" {"--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"} else {""}
memcheck_ctest_flag := if memcheck == "1" {"-T memcheck"} else { "" }

cat := `if command -v bat >/dev/null 2>&1; then echo bat; elif command -v batcat >/dev/null 2>&1; then echo batcat; else echo cat; fi`

# Remove the build directory
clean-build:
    rm -rf build/

_clean-hook:
    @if [ "{{clean}}" = "1" ]; then just clean-build; fi

# Configure, build and test via the CMake workflow preset
workflow *args: _clean-hook
    cmake --workflow --preset {{preset}} {{args}}

# Configure project
configure *args: _clean-hook
    cmake --preset {{preset}} {{args}}

# Build project
build *args: _clean-hook
    cmake --build --preset {{preset}} --config {{config}} --target {{target}} {{verbose_flag}} {{args}}

# Build and run the kv_engine_app executable
run *args: (build "--target" exe)
    {{memcheck_command}} {{memcheck_args}} ./build/{{preset}}/{{config}}/{{exe}} {{args}}

# Run unit tests, with Valgrind memcheck when valgrind=1
test *args: build
    ctest --test-dir build/{{preset}} -C {{config}} {{verbose_flag}} {{memcheck_ctest_flag}} {{args}}
    if [ {{memcheck}} = 1 ]; then {{cat}} -P build/ninja-multi/Testing/Temporary/MemoryChecker.*.log; fi

# Format all source files in place with clang-format
format:
    clang-format -i core/include/*.h core/src/*.c core/unit_tests/*.c api/include/*.h api/src/*.c app/src/*.c

# Check that all source files are properly formatted
format-check:
    clang-format --dry-run -Werror core/include/*.h core/src/*.c core/unit_tests/*.c api/include/*.h api/src/*.c app/src/*.c
