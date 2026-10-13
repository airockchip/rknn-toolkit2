cmake_minimum_required(VERSION 3.6)

project(rknn_encrypt_demo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(TARGET_SOC STREQUAL "RV1106_RV1103")
  add_definitions(-DRV1106_RV1103)
elseif(TARGET_SOC STREQUAL "RV1106B_RV1103B")
  add_definitions(-DRV1106B_RV1103B)
endif()

# skip 3rd-party lib dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../runtime//${CMAKE_SYSTEM_NAME}/librknn_api)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknnrt.so)
else()
  if(TARGET_SOC STREQUAL "RV1106_RV1103" OR TARGET_SOC STREQUAL "RV1106B_RV1103B")
    # RV1106/RV1103 family uses the mini runtime with uclibc toolchain
    set(RKNN_RT_LIB ${RKNN_API_PATH}/armhf-uclibc/librknnmrt.so)
  elseif(CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
    set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)
  else()
    set(LIB_ARCH armhf)
    set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)
  endif()
endif()

include_directories(${RKNN_API_PATH}/include)

# stb
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty/)

set(CMAKE_INSTALL_RPATH "lib")

add_executable(rknn_encrypt_demo
    src/main.cpp
)

target_link_libraries(rknn_encrypt_demo
  ${RKNN_RT_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_encrypt_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_encrypt_demo DESTINATION ./)


message(STATUS "target_soc = ${TARGET_SOC}")
install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
file(GLOB IMAGE_FILES "model/*.jpg")
install(FILES ${IMAGE_FILES} DESTINATION ./model/)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
