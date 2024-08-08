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

# stb
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty/)

# rga
# comes from https://github.com/airockchip/librga
set(RGA_PATH ${CMAKE_SOURCE_DIR}/../3rdparty/rga/)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(RGA_LIB ${RGA_PATH}/libs/AndroidNdk/${CMAKE_ANDROID_ARCH_ABI}/librga.so)
else()
  if(CMAKE_C_COMPILER MATCHES "aarch64")
    set(LIB_ARCH aarch64)
  else()
    set(LIB_ARCH armhf)
  endif()

  set(RGA_LIB ${RGA_PATH}/libs/Linux//gcc-${LIB_ARCH}/librga.so)
endif()

include_directories(${RGA_PATH}/include)

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

set(CMAKE_INSTALL_RPATH "lib")

# rknn_create_mem_demo
add_executable(rknn_create_mem_demo
  src/rknn_create_mem_demo.cpp
)

target_link_libraries(rknn_create_mem_demo
  ${RKNN_RT_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_api_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_create_mem_demo DESTINATION ./)

# rknn_create_mem_with_rga_demo
add_executable(rknn_create_mem_with_rga_demo
  src/rknn_create_mem_with_rga_demo.cpp
)

target_link_libraries(rknn_create_mem_with_rga_demo
  ${RKNN_RT_LIB}
  ${RGA_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_api_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_create_mem_with_rga_demo DESTINATION ./)

# At present, mmz　demo is only available under Android, but not for Linux temporarily,
# mainly because libmpimmz.so has no Linux implementation now. The API of the NPU itself supports Linux.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  # rknn_with_mmz_demo
  add_executable(rknn_with_mmz_demo
    src/rknn_with_mmz_demo.cpp
  )

  target_link_libraries(rknn_with_mmz_demo
    ${RKNN_RT_LIB}
    ${MPI_MMZ_LIB}
  )

  # install target and libraries
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_api_demo_${CMAKE_SYSTEM_NAME})
  install(TARGETS rknn_with_mmz_demo DESTINATION ./)

  # rknn_set_internal_mem_from_fd_demo
  add_executable(rknn_set_internal_mem_from_fd_demo
    src/rknn_set_internal_mem_from_fd_demo.cpp
  )

  target_link_libraries(rknn_set_internal_mem_from_fd_demo
    ${RKNN_RT_LIB}
    ${MPI_MMZ_LIB}
  )

  # install target and libraries
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_api_demo_${CMAKE_SYSTEM_NAME})
  install(TARGETS rknn_set_internal_mem_from_fd_demo DESTINATION ./)
endif()

install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
file(GLOB IMAGE_FILES "model/*.jpg")
install(FILES ${IMAGE_FILES} DESTINATION ./model/)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
install(PROGRAMS ${RGA_LIB} DESTINATION lib)
