cmake_minimum_required(VERSION 3.25)
project(sv_qtouch)

# 1. Force the C++20 Standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Thats all i need for qt i assume
set(CMAKE_PREFIX_PATH "C:/SvQt/6.10.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# This project sources: adding all .h/.cpp in /src and its subdirectories
file(GLOB_RECURSE SV_QTOUCH_SOURCES src/*.cpp src/*.h)

# 4. Declare the executable target
#add_executable(qt_hello_app hi.h hi.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/doctest)

# Listing all sv_components:
add_subdirectory(deps/sv_common)

qt_add_executable(sv_qtouch_app ${SV_QTOUCH_SOURCES})

# Automatically handle full deployment (DLLs + Platforms + Styles) post-build
if(WIN32)
    add_custom_command(TARGET sv_qtouch_app POST_BUILD
        COMMAND "C:/SvQt/6.10.3/msvc2022_64/bin/windeployqt.exe"
                --verbose 1
                --no-compiler-runtime
                --dir "${CMAKE_CURRENT_BINARY_DIR}"
                "$<TARGET_FILE:sv_qtouch_app>"
        COMMENT "Running windeployqt to copy required modules and platform plugins..."
    )
endif()

# 5. Link Qt modules
target_link_libraries(sv_qtouch_app PRIVATE Qt6::Widgets Qt6::Core sv_common)

# 6. Explicitly enforce the MSVC C++20 flag for this target
if(MSVC)
    target_compile_options(sv_qtouch_app PRIVATE /std:c++20)
endif()