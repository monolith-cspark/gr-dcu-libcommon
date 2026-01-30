#!/bin/bash
set -e

# 1. SDK 환경 설정 (기존과 동일)
SDK_ENV="/opt/poky/4.0.15/environment-setup-cortexa53-poky-linux"
source "$SDK_ENV"

# 2. 빌드 폴더 초기화
BUILD_DIR="build"
rm -rf "$BUILD_DIR" && mkdir "$BUILD_DIR" && cd "$BUILD_DIR"

# 3. CMake 실행
# -DCMAKE_INSTALL_PREFIX: 나중에 'make install' 했을 때 파일이 모일 위치
cmake -DCMAKE_INSTALL_PREFIX=../install ..

# 4. 빌드 및 로컬 설치
make -j$(nproc)
make install 

echo "[SUCCESS] Library built and installed to $(pwd)/../install"