cmake_minimum_required(VERSION 3.6)

project(rknn_dynshape_demo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# skip 3rd-party lib dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../runtime//${CMAKE_SYSTEM_NAME}/librknn_api)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknnrt.so)
else()
  if (CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
  else()
    set(LIB_ARCH armhf)
  endif()
  set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)
endif()
include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty)

# opencv
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../3rdparty/opencv/OpenCV-android-sdk/sdk/native/jni/abi-${CMAKE_ANDROID_ARCH_ABI})
else()
  if(LIB_ARCH STREQUAL "armhf")
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../3rdparty/opencv/opencv-linux-armhf/share/OpenCV)
  else()
    set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/../3rdparty/opencv/opencv-linux-aarch64/share/OpenCV)
  endif()
endif()
find_package(OpenCV REQUIRED)

# mmz
set(MPI_MMZ_PATH ${CMAKE_SOURCE_DIR}/../3rdparty/rk_mpi_mmz)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(MPI_MMZ_LIB ${MPI_MMZ_PATH}/lib/Android/${CMAKE_ANDROID_ARCH_ABI}/libmpimmz.so)
else()
  if(CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
  else()
    set(LIB_ARCH armhf)
  endif()

  set(MPI_MMZ_LIB ${MPI_MMZ_PATH}/lib/Linux//${LIB_ARCH}/libmpimmz.so)
endif()
include_directories(${MPI_MMZ_PATH}/include)

#### cnpy
set(CNPY_ROOT ${CMAKE_SOURCE_DIR}/../3rdparty/cnpy)
include_directories(${CNPY_ROOT})

set(CMAKE_INSTALL_RPATH "lib")

### 普通API Demo
add_executable(rknn_dynshape_inference
    src/rknn_dynshape_inference.cc
    ${CNPY_ROOT}/cnpy.cpp
)
target_link_libraries(rknn_dynshape_inference
  ${RKNN_RT_LIB}
  ${OpenCV_LIBS}
)
# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_dynshape_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_dynshape_inference DESTINATION ./)

## 零拷贝API Demo
add_executable(rknn_dynshape_inference_zero_copy
    src/rknn_dynshape_inference_zero_copy.cc
    ${CNPY_ROOT}/cnpy.cpp
)
target_link_libraries(rknn_dynshape_inference_zero_copy
  ${RKNN_RT_LIB}
  ${OpenCV_LIBS}
)
# install target and libraries
install(TARGETS rknn_dynshape_inference_zero_copy DESTINATION ./)

install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
install(DIRECTORY images DESTINATION ./)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)



# At present, mmz　demo is only available under Android, but not for Linux temporarily,
# mainly because libmpimmz.so has no Linux implementation now. The API of the NPU itself supports Linux.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  # rknn_dynshape_inference_zero_copy_alloc_outside
  add_executable(rknn_dynshape_inference_zero_copy_alloc_outside
    src/rknn_dynshape_inference_zero_copy_alloc_outside.cc
    ${CNPY_ROOT}/cnpy.cpp
  )

  target_link_libraries(rknn_dynshape_inference_zero_copy_alloc_outside
    ${RKNN_RT_LIB}
    ${MPI_MMZ_LIB}
  )

  # install target and libraries
  install(TARGETS rknn_dynshape_inference_zero_copy_alloc_outside DESTINATION ./)
endif()
