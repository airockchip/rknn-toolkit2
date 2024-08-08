cmake_minimum_required(VERSION 3.6)

project(rknn_api_demo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# skip 3rd-party lib dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../runtime//${CMAKE_SYSTEM_NAME}/librknn_api)
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
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty)

set(CMAKE_INSTALL_RPATH "lib")

# rknn_matmul_api_demo
add_executable(rknn_matmul_api_demo
  src/rknn_matmul_api_demo.cpp
  src/matmul_utils.cpp
)

target_link_libraries(rknn_matmul_api_demo
  ${RKNN_RT_LIB}
)

# rknn_matmul_api_dynshape_demo
add_executable(rknn_matmul_api_dynshape_demo
  src/rknn_matmul_api_dynshape_demo.cpp
  src/matmul_utils.cpp
)

target_link_libraries(rknn_matmul_api_dynshape_demo
  ${RKNN_RT_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_matmul_api_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_matmul_api_demo DESTINATION ./)
install(TARGETS rknn_matmul_api_dynshape_demo DESTINATION ./)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
