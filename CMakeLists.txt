cmake_minimum_required(VERSION 3.16)

project(sv_qtouch LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "C:/SvQt/6.10.2/mingw_64")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# My sources: add all .h/.cpp in /src and its subdirectories
file(GLOB_RECURSE MY_SOURCES src/*.cpp src/*.h)

# magic_enum lib dependency
file(GLOB_RECURSE MAGICENUM_SOURCES thirdparty/magic_enum/*.hpp)

add_subdirectory(deps/sv_common)
add_subdirectory(deps/sv_qtcommon)
add_subdirectory(deps/sv_datalayer)

qt_add_executable(${CMAKE_PROJECT_NAME} ${MY_SOURCES} ${MAGICENUM_SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE thirdparty)



target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core sv_common sv_qtcommon sv_datalayer)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    WIN64_EXECUTABLE ON
)

# Setting up deployment, only for Windows
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
