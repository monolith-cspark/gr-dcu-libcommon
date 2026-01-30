cmake_minimum_required(VERSION 3.10)

project(dcu-libcommon
    VERSION 1.0.0
    LANGUAGES CXX
)

# ==============================================================================
# 1. 기본 설정 (Standard Settings)
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Yocto/GNU 표준 설치 경로 및 도움말 매크로 로드
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ==============================================================================
# 2. 라이브러리 생성 (Library Target)
# ==============================================================================
add_library(${PROJECT_NAME} SHARED
    src/library_anchor.cpp
)

# 내부 참조용 Alias (Modern CMake Style)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# ==============================================================================
# 3. include 경로 설정 (Usage Requirements)
# ==============================================================================
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ==============================================================================
# 4. SONAME 및 버전 관리
# ==============================================================================
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   1
)

# ==============================================================================
# 5. 헤더 파일 설치
# ==============================================================================
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ==============================================================================
# 6. 라이브러리 설치 및 Export 등록
# ==============================================================================
install(
    TARGETS ${PROJECT_NAME}
    EXPORT  ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ==============================================================================
# 7. CMake Config 파일 생성
# ==============================================================================
# 설치될 상대 경로 정의 (예: usr/lib/cmake/dcu-libcommon)
set(INSTALL_CONFIG_DEST "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# Config 파일 생성 (절대 경로 사용 권장)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${INSTALL_CONFIG_DEST}
)

# 버전 파일 생성
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# ==============================================================================
# 8. 모든 CMake 관련 파일 설치
# ==============================================================================

# 1) 생성된 Config 및 Version 파일 설치
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${INSTALL_CONFIG_DEST}
)

# 2) Export 된 Targets 정보 설치
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${INSTALL_CONFIG_DEST}
)