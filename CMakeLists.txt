cmake_minimum_required(VERSION 3.25)
project(sv_qtouch)

if (MSVC)
    add_compile_options(/fsanitize=address)
    add_link_options(/fsanitize=address)
endif()

# 1. Force the C++20 Standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##########################
# < DEPENDENCIES SETUP > #
##########################

# Note to self: adding submodule dependencies is just 2 commands:
# git submodule add https://github.com/boostorg/boost.git deps/boost
# git submodule update --init --recursive

# Thats all i need to setup Qt, i assume:
set(CMAKE_PREFIX_PATH "C:/SvQt/6.10.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# todo submodule
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/doctest)

# boost
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES bimap)
set(BOOST_SKIP_INSTALL_RULES ON)
add_subdirectory(deps/boost EXCLUDE_FROM_ALL)

add_subdirectory(deps/ordered-map)

add_subdirectory(deps/doctest)

# Listing all sv_components:
add_subdirectory(deps/sv_common)
add_subdirectory(deps/sv_qtcommon)
add_subdirectory(deps/sv_datalayer)

############################
# < / DEPENDENCIES SETUP > #
############################

# This project sources: adding all .h/.cpp in /src and its subdirectories
file(GLOB_RECURSE SV_QTOUCH_SOURCES src/*.cpp src/*.h)

qt_add_executable(sv_qtouch_app ${SV_QTOUCH_SOURCES})

# Automatically handle full deployment (DLLs + Platforms + Styles) post-build
#todo replace with variable
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

# 5. Link Qt and other modules
target_link_libraries(sv_qtouch_app PRIVATE Qt6::Widgets Qt6::Core sv_common sv_qtcommon sv_datalayer)

# 6. Explicitly enforce the MSVC C++20 flag for this target
if(MSVC)
    target_compile_options(sv_qtouch_app PRIVATE /std:c++20)
endif()