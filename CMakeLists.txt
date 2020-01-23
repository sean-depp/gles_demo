cmake_minimum_required(VERSION 3.15)
project(gles_demo)
set(CMAKE_CXX_STANDARD 14)

# CMAKE_BINARY_DIR
# 如果是 in source 编译,指得就是工程顶层目录,
# 如果是 out-of-source 编译,指的是工程编译发生的目录。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SDK_ROOT /opt/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_2019_R2) # sdk目录
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/cmake/modules")
set(CMAKE_MODULE_PATH ${SDK_ROOT}/cmake/modules)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# CMAKE_CURRENT_SOURCE_DIR
# 指的是当前处理的CMakeLists.txt所在的路径。
#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gles_demo)
set(INCLUDE_DIR ${SDK_ROOT}/include/)

# FreeImage库
find_library(FI_LIBRARY freeimage "/usr/lib/")
# gles lib
find_library(EGL_LIBRARY EGL "/opt/Imagination/PowerVR_Graphics/PowerVR_Tools/PVRVFrame/Library/Linux_x86_64/")
find_library(GLES_LIBRARY GLESv2 "/opt/Imagination/PowerVR_Graphics/PowerVR_Tools/PVRVFrame/Library/Linux_x86_64/")

# CMAKE_DL_LIBS: 包含dlopen和dlclose的库的名称
list(APPEND PLATFORM_LIBS ${GLES_LIBRARY} ${EGL_LIBRARY} ${FI_LIBRARY} ${CMAKE_DL_LIBS})

if (UNIX)
    set(WS_DEFINE "")
    if (NOT WS)
        set(WS "X11")
        set(WS_DEFINE "${WS}")
    endif ()

    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH}) # 环境变量获取
    endif ()

    # 增加编译参数
    add_definitions(-D${WS_DEFINE})

    if (${WS} STREQUAL X11)
        find_package(X11 REQUIRED)

        if (NOT ${X11_FOUND})
            message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
        endif ()

        list(APPEND PLATFORM_LIBS ${X11_LIBRARIES})
        include_directories(${X11_INCLUDE_DIR})

        set(SRC_FILES glesX11.cpp GLESUtils.cpp GLESUtils.h) # 源码
    else ()
        message(FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen.")
    endif ()

    add_definitions(-D${WS}) #Add a compiler definition so that our header files know what we're building for
    add_executable(gles_demo ${SRC_FILES})
endif ()

if (PLATFORM_LIBS)
    target_link_libraries(gles_demo ${PLATFORM_LIBS})
endif ()

target_include_directories(gles_demo PUBLIC ${INCLUDE_DIR}) # include目录
target_compile_definitions(gles_demo PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1>) # Defines DEBUG=1 or RELEASE=1
