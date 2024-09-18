cmake_minimum_required(VERSION 3.6)

project(rknn_yolov5_demo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--allow-shlib-undefined")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--allow-shlib-undefined")

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_yolov5_demo_${CMAKE_SYSTEM_NAME})

set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../../runtime/${CMAKE_SYSTEM_NAME}/librknn_api)
set(RKNN_RT_LIB ${RKNN_API_PATH}/armhf-uclibc/librknnmrt.so)

include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/../../3rdparty)

# rknn_yolov5_demo
include_directories(${CMAKE_SOURCE_DIR}/include)

if(TARGET_SOC STREQUAL "RV1106_RV1103")
    add_definitions(-DRV1106_RV1103)
endif()

add_executable(rknn_yolov5_demo
            src/main.cc
            src/postprocess.cc
)

target_link_libraries(rknn_yolov5_demo
${RKNN_RT_LIB}
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_yolov5_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_yolov5_demo DESTINATION ./)


install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
install(DIRECTORY model/${TARGET_SOC} DESTINATION ./model)
file(GLOB IMAGE_FILES "model/*.jpg")
install(FILES ${IMAGE_FILES} DESTINATION ./model/)

file(GLOB TXT_FILES "model/*.txt")
install(FILES ${TXT_FILES} DESTINATION ./model/)