#!/bin/bash

set -e

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

if [ -z ${TARGET_SOC} ];then
  echo "$0 -t <target> -a <arch> [-b <build_type>]"
  echo ""
  echo "    -t : target (rk3566/rk3568/rk3562/rk3576/rk3588/rv1126b/rv1103/rv1106/rv1103b)"
  echo "    -a : arch (aarch64/armhf)"
  echo "    -b : build_type(Debug/Release)"
  echo "such as: $0 -t rk3588 -a aarch64 -b Release"
  echo ""
  exit -1
fi

# RV1106/RV1103 family uses the uclibc toolchain given by RK_RV1106_TOOLCHAIN,
# e.g. export RK_RV1106_TOOLCHAIN=~/opt/arm-rockchip831-linux-uclibcgnueabihf
if [[ ${TARGET_SOC} == "rv1103" || ${TARGET_SOC} == "rv1106" || ${TARGET_SOC} == "rv1103b" ]]; then
  if [[ -z ${RK_RV1106_TOOLCHAIN} ]]; then
    echo "Please set RK_RV1106_TOOLCHAIN for $TARGET_SOC"
    echo "such as export RK_RV1106_TOOLCHAIN=~/opt/arm-rockchip831-linux-uclibcgnueabihf"
    exit
  fi
  GCC_COMPILER=${RK_RV1106_TOOLCHAIN}
fi

if [[ -z ${GCC_COMPILER} ]];then
  echo "Please set GCC_COMPILER for $TARGET_SOC"
  echo "such as export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu"
  exit
fi
echo "$GCC_COMPILER"
export CC=${GCC_COMPILER}-gcc
export CXX=${GCC_COMPILER}-g++

if command -v ${CC} >/dev/null 2>&1; then
    :
else
    echo "${CC} is not available"
    echo "Please set GCC_COMPILER for $TARGET_SOC"
    echo "such as export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu"
    exit
fi

# Debug / Release
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
    rv1126b)
        TARGET_SOC="RV1126B"
        ;;
    rv1103)
        TARGET_SOC="RV1106_RV1103"
        ;;
    rv1106)
        TARGET_SOC="RV1106_RV1103"
        ;;
    rv1103b)
        TARGET_SOC="RV1106B_RV1103B"
        ;;
    *)
        echo "Invalid target: ${TARGET_SOC}"
        echo "Valid target: rk3562,rk3566,rk3568,rk3576,rk3588,rv1126b,rv1103,rv1106,rv1103b"
        exit -1
        ;;
esac

TARGET_PLATFORM=${TARGET_SOC}_linux
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
echo "CC=${CC}"
echo "CXX=${CXX}"
echo "==================================="

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi


cd ${BUILD_DIR}
cmake ../.. \
    -DTARGET_SOC=${TARGET_SOC} \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=${TARGET_ARCH} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_CXX_COMPILER=${CXX}
make -j4
make install
