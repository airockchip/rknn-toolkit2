cmake_minimum_required(VERSION 3.6)

project(rknn_mobilenet_demo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# rknn api

set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../../runtime/${CMAKE_SYSTEM_NAME}/librknn_api)
set(RKNN_RT_LIB ${RKNN_API_PATH}/armhf-uclibc/librknnmrt.so)

include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/../../3rdparty)


set(CMAKE_INSTALL_RPATH "lib")

add_executable(rknn_mobilenet_demo
    src/main.cc
)

target_link_libraries(rknn_mobilenet_demo
  ${RKNN_RT_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_mobilenet_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_mobilenet_demo DESTINATION ./)

if(TARGET_SOC STREQUAL "RV1106_RV1103")
  add_executable(rknn_mobilenet_nhwc_demo
      src/main_nhwc.cc
  )
  target_link_libraries(rknn_mobilenet_nhwc_demo
    ${RKNN_RT_LIB}
  )
  install(TARGETS rknn_mobilenet_nhwc_demo DESTINATION ./)
endif()


install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
file(GLOB IMAGE_FILES "model/*.jpg")
install(FILES ${IMAGE_FILES} DESTINATION ./model/)
