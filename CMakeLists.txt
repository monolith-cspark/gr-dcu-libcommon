cmake_minimum_required(VERSION 3.13)
project(dcu-libcommon VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# 라이브러리 생성
add_library(${PROJECT_NAME} SHARED
    src/library_anchor.cpp
)

# include 경로
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# SONAME 설정
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# 헤더 설치
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 실제 ELF만 설치
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    NAMELINK_SKIP
)

# .so 심볼릭 링크 생성
install(CODE "
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        lib${PROJECT_NAME}.so.${PROJECT_VERSION}
        lib${PROJECT_NAME}.so
    WORKING_DIRECTORY \$ENV{DESTDIR}${CMAKE_INSTALL_LIBDIR}
)
")
