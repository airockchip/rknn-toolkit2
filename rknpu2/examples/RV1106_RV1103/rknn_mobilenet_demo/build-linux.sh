#!/bin/bash
set -e

if [ -z $RK_RV1106_TOOLCHAIN ]; then
  echo "Please set the RK_RV1106_TOOLCHAIN environment variable!"
  echo "example:"
  echo "  export RK_RV1106_TOOLCHAIN=<path-to-your-dir/arm-rockchip830-linux-uclibcgnueabihf>"
  exit
fi

echo "$0 $@"
while getopts ":t:b" opt; do
  case $opt in
    t)
      TARGET_SOC=$OPTARG
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

# Debug / Release
if [[ -z ${BUILD_TYPE} ]];then
    BUILD_TYPE=Release
fi

if [ -z ${TARGET_SOC} ];then
  echo "$0 -t <target> [-b <build_type>]"
  echo ""
  echo "    -t : target (rv1103/rv1103b/rv1106/rv1106b)"
  echo "    -b : build_type(Debug/Release)"
  echo "such as: $0 -t rv1103 -b Release"
  echo ""
  exit -1
fi

case ${TARGET_SOC} in
    rv1103)
        TARGET_SOC="RV1106_RV1103"
        ;;
    rv1106)
        TARGET_SOC="RV1106_RV1103"
        ;;
    rv1103b)
        TARGET_SOC="RV1106B_RV1103B"
        ;;
    rv1106b)
        TARGET_SOC="RV1106B_RV1103B"
        ;;
    *)
        echo "Invalid target: ${TARGET_SOC}"
        echo "Valid target: rv1103,rv1106,rv1103b,rv1106b"
        exit -1
        ;;
esac

# for arm
GCC_COMPILER=$RK_RV1106_TOOLCHAIN

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

# build
BUILD_DIR=${ROOT_PWD}/build/build_linux_arm_${TARGET_SOC}

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
    -DCMAKE_C_COMPILER=${GCC_COMPILER}-gcc \
    -DCMAKE_CXX_COMPILER=${GCC_COMPILER}-g++\
    -DTARGET_SOC=${TARGET_SOC} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} 
make -j4
make install
cd -
