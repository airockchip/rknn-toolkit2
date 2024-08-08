cmake_minimum_required(VERSION 3.6)

project(rknn_custom_pytorch_op_demo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../../runtime/${CMAKE_SYSTEM_NAME}/librknn_api)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknnrt.so)
else()
  if(CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
  else()
    set(LIB_ARCH armhf)
  endif()

  set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)
endif()

include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/../../3rdparty)

set(CNPY_ROOT ${CMAKE_SOURCE_DIR}/../../3rdparty/cnpy)
include_directories(${CNPY_ROOT})

set(CMAKE_INSTALL_RPATH "lib")

add_executable(rknn_custom_pytorch_op_demo
  src/rknn_api_test_custom_pytorch_op.cpp
  ${CNPY_ROOT}/cnpy.cpp
)

target_link_libraries(rknn_custom_pytorch_op_demo
  ${RKNN_RT_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_custom_pytorch_op_demo_${CMAKE_SYSTEM_NAME})

install(TARGETS rknn_custom_pytorch_op_demo DESTINATION ./)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
file(GLOB NPY_FILES "model/*.npy")
install(FILES ${NPY_FILES} DESTINATION ./model)
file(GLOB BIN_FILES "model/*.bin")
install(FILES ${BIN_FILES} DESTINATION ./model)
message(STATUS "target_soc = ${TARGET_SOC}")
install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
