cmake_minimum_required(VERSION 3.14)

project(
    prism
    VERSION 0.1.0
    DESCRIPTION "C++17 header-only universal print with colors"
    HOMEPAGE_URL "https://github.com/jojo8356/prism"
    LANGUAGES CXX
)

# Header-only INTERFACE target
add_library(prism INTERFACE)
add_library(prism::prism ALIAS prism)

target_include_directories(prism INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(prism INTERFACE cxx_std_17)

# Options only when this is the root project (not via FetchContent)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(PRISM_BUILD_TESTS "Build tests" OFF)
    option(PRISM_BUILD_EXAMPLES "Build examples" OFF)

    if(PRISM_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()

    if(PRISM_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
endif()

# ── Install rules ──────────────────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
    TARGETS prism
    EXPORT prismTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT prismTargets
    FILE prismTargets.cmake
    NAMESPACE prism::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/prism
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prismConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/prismConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/prism
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/prismConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/prismConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/prismConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/prism
)

install(
    FILES LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
