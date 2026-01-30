cmake_minimum_required(VERSION 3.10)

project(dcu-libcommon
    VERSION 1.0.0
    LANGUAGES CXX
)

# ===============================
# 1. 기본 설정
# ===============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ===============================
# 2. 라이브러리 생성
# ===============================
add_library(${PROJECT_NAME} SHARED
    src/library_anchor.cpp
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# ===============================
# 3. include 경로
# ===============================
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ===============================
# 4. SONAME / VERSION
# ===============================
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   1
)

# ===============================
# 5. 헤더 설치
# ===============================
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ===============================
# 6. 라이브러리 설치 (Targets 등록)
# ===============================
install(
    TARGETS ${PROJECT_NAME}
    EXPORT  ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ===============================
# 7. Config 파일 생성
# ===============================
# .in 파일 경로를 절대경로로 확실히 지정
set(DOT_IN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in")

configure_package_config_file(
    ${DOT_IN_FILE}
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# ===============================
# 8. 모든 CMake 파일 설치 
# ===============================
# 설치 경로를 직접 문자열로 지정하여 꼬임 방지
set(INSTALL_CONFIG_DEST "lib/cmake/${PROJECT_NAME}")

# Config 및 Version 파일 설치
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${INSTALL_CONFIG_DEST}
)

# Targets export 파일 설치
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${INSTALL_CONFIG_DEST}
)