cmake_minimum_required(VERSION 3.10)

project(dcu-libcommon
    VERSION 1.0.0
    LANGUAGES CXX
)

# ===============================
# 기본 설정
# ===============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ===============================
# 라이브러리 생성
# ===============================
add_library(${PROJECT_NAME} SHARED
    src/library_anchor.cpp
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# ===============================
# include 경로
# ===============================
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ===============================
# SONAME / VERSION
# ===============================
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   1
)

# ===============================
# 설치 경로
# ===============================
set(CONFIG_INSTALL_DIR
    ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# ===============================
# 헤더 설치
# ===============================
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ===============================
# 라이브러리 + Targets
# ===============================
install(
    TARGETS ${PROJECT_NAME}
    EXPORT  ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ===============================
# Config.cmake 생성
# ===============================
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# ===============================
# Config / Version install
# ===============================
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "lib/cmake/${PROJECT_NAME}"
)

# ===============================
# Targets install
# ===============================
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CONFIG_INSTALL_DIR}
)
