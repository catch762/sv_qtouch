cmake_minimum_required(VERSION 3.16)
project(sv_qtouch LANGUAGES CXX)
include(cmake/CPM.cmake)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QT set up
set(CMAKE_PREFIX_PATH "C:/SvQt/6.10.2/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# This project sources: adding all .h/.cpp in /src and its subdirectories
file(GLOB_RECURSE SV_QTOUCH_SOURCES src/*.cpp src/*.h)

# Besides QT, here's how all other dependencies work:
#
# 1) All thirdparty components
#       - Simply added via CPM (with default path - so multiple components use one copy)
# 2) All sv_components (my own modules)
#       - Added as submodules to /deps. Like this:
#           - git submodule add git@github.com:catch762/sv_common.git deps/sv_common
#           - git submodule update --init
#           - then in root project CMakeLists do:
#               - add_subdirectory(deps/sv_common)
#               - add lib name to target_link_libraries()
#       - Their thirdparty dependencies added via CPM, as usual
#       - When sv_component is depending on other sv_component, it adds its name to self CMakeLists
#         target_link_libraries() but doesnt do anything else. It is root project CMakeLists' job
#         to actually add that dependency.
#       - sv_component's deps listed in target_link_libraries() are always PUBLIC - this means
#         when you include sv_component in target_link_libraries, you dont need to also include
#         things that component depends on.

# Listing all sv_components:
add_subdirectory(deps/sv_common)
add_subdirectory(deps/sv_qtcommon)
add_subdirectory(deps/sv_datalayer)

qt_add_executable(${CMAKE_PROJECT_NAME} ${SV_QTOUCH_SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core sv_common sv_qtcommon sv_datalayer)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    WIN64_EXECUTABLE ON
)


# Setting up deployment target, only for Windows. You must manually build release version first.
if(WIN32)
    # After qt_standard_project_setup() there should be ${QT_DEPLOY_TOOL}
    # but for some reason its empty, so heres workaround.
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE 
        NAMES windeployqt.exe windeployqt6.exe 
        PATHS "${QT_BIN_DIR}"
        NO_DEFAULT_PATH
    )

    # Build release version, then run this target, and it will deploy project to /out/release
    # Todo: it copies too many QT dll files for some reason, not all of those are actually needed
    add_custom_target(release-deploy
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_SOURCE_DIR}/out/release"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/out/release"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>" "${PROJECT_SOURCE_DIR}/out/release/"
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" 
            --dir "${PROJECT_SOURCE_DIR}/out/release"
            --compiler-runtime 
            --no-translations 
            --release
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
        COMMENT "Creating standalone release package in out/release/ with ${WINDEPLOYQT_EXECUTABLE}"
        VERBATIM
    )

    add_dependencies(release-deploy ${CMAKE_PROJECT_NAME})
endif()
