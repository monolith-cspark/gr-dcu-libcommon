cmake_minimum_required(VERSION 3.13)
project(dcu-libcommon VERSION 1.0.0 LANGUAGES CXX) # 프로젝트 이름을 폴더명과 일치시킴

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# 헤더 경로 설정
include_directories(include)

# 1. 라이브러리 타겟 생성
add_library(${PROJECT_NAME} SHARED src/library_anchor.cpp)

# 2. 설치 규칙 정의
# 헤더 파일 설치
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 빌드된 .so 파일 설치
install(TARGETS ${PROJECT_NAME} 
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})