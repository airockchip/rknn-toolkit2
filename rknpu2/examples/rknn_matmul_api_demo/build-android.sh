#!/bin/bash

set -e

if [[ -z ${ANDROID_NDK_PATH} ]];then
    echo "Please set ANDROID_NDK_PATH, such as export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b"
    echo "NDK Version r18, r19 is recommanded. Other version may cause build failure."
    exit
fi

echo "$0 $@"
while getopts ":t:a:b" opt; do
  case $opt in
    t)
      TARGET_SOC=$OPTARG
      ;;
    a)
      TARGET_ARCH=$OPTARG
      ;;
    b)
      BUILD_TYPE=$OPTARG
      ;;
    :)
      echo "Option -$OPTARG requires an argument." 
      exit 1
      ;;
    ?)
      echo "Invalid option: -$OPTARG index:$OPTIND"
      ;;
  esac
done

if [ -z ${TARGET_SOC} ]  || [ -z ${TARGET_ARCH} ] ; then
  echo "$0 -t <target> -a <arch> [-b <build_type>]"
  echo ""
  echo "    -t : target (rk3566/rk3568/rk3562/rk3576/rk3588)"
  echo "    -a : arch (arm64-v8a/armeabi-v7a)"
  echo "    -b : build_type (Debug/Release)"
  echo "such as: $0  -t rk3588 -a arm64-v8a -b Release"
  echo ""
  exit -1
fi

# Debug / Release / RelWithDebInfo
if [[ -z ${BUILD_TYPE} ]];then
    BUILD_TYPE=Release
fi

case ${TARGET_SOC} in
    rk356x)
        TARGET_SOC="RK3566_RK3568"
        ;;
    rk3566_rk3568)
        TARGET_SOC="RK3566_RK3568"
        ;;
    rk3588)
        TARGET_SOC="RK3588"
        ;;
    rk3576)
        TARGET_SOC="RK3576"
        ;;
    rk3566)
        TARGET_SOC="RK3566_RK3568"
        ;;
    rk3568)
         TARGET_SOC="RK3566_RK3568"
        ;;
    rk3562)
        TARGET_SOC="RK3562"
        ;;
    *)
        echo "Invalid target: ${TARGET_SOC}"
        echo "Valid target: rk3562,rk3566,rk3568,rk3576,rk3588"
        exit -1
        ;;
esac

TARGET_PLATFORM=${TARGET_SOC}_android
if [[ -n ${TARGET_ARCH} ]];then
TARGET_PLATFORM=${TARGET_PLATFORM}_${TARGET_ARCH}
fi
ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )
BUILD_DIR=${ROOT_PWD}/build/build_${TARGET_PLATFORM}_${BUILD_TYPE}

echo "==================================="
echo "TARGET_SOC=${TARGET_SOC}"
echo "TARGET_ARCH=${TARGET_ARCH}"
echo "BUILD_TYPE=${BUILD_TYPE}"
echo "BUILD_DIR=${BUILD_DIR}"
echo "ANDROID_NDK_PATH=${ANDROID_NDK_PATH}"
echo "==================================="

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi


cd ${BUILD_DIR}
cmake ../.. \
        -DANDROID_TOOLCHAIN=clang \
        -DCMAKE_SYSTEM_NAME=Android \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=${TARGET_ARCH} \
        -DANDROID_STL=c++_static \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
# make VERBOSE=1
make -j4
make install
cd ..

