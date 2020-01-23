# 一篇极度舒适的OpenGL_ES环境搭建(Ubuntu 18.04 LTS)

-----

## 目录

* 前言
* PowerVR
* CMake环境
* FreeImage
* CMakeLists.txt解析
* 源码
* 最后

-----

## 前言

> 作为一个梦想成为游戏制作人的菜鸟程序员, 我终究没悬念地踏上了撰写shader的道路(手动滑稽). 这是一篇比较细致的Ubuntu18.04下OpenGL_ES环境搭建的文件, 也是我爬过n多个坑之后的总结, 希望能帮助到Mac背后的你(手动滑稽).

-----

## PowerVR

> 模拟器方面, 我选择PowerVR, 当然, 你可以选择别的, 来到[官网](https://www.imgtec.com/developers/powervr-sdk-tools/installers/), 下载对应的SDK, 运行下载后的文件, 即可安装. 当然了, 如果不能运行, 用chmod添加运行权限即可. 我的第一个**Hello, World案例**也是基于PowerVR的Hello案例魔改的.

![](https://imgconvert.csdnimg.cn/aHR0cHM6Ly90dmExLnNpbmFpbWcuY24vbGFyZ2UvMDA2dE5iUndseTFnYjJlaHpqMnpwajMwc2cwbWVoMHouanBn?x-oss-process=image/format,png)

> 值得一提的就是安装目录记一下, 之后要用到:

![](https://imgconvert.csdnimg.cn/aHR0cHM6Ly90dmExLnNpbmFpbWcuY24vbGFyZ2UvMDA2dE5iUndseTFnYjJldW5xMHY0ajMwc2cwbWVrNGsuanBn?x-oss-process=image/format,png)

-----

## CMake环境

> 这里我选用CLion开发, 这样更友好, 尽管CLion的vim似乎不够强大, 但是考虑到调试等功能, 还是值得拥有的. 当然, 自己用vim搭建IDE也是完全OK的. 先确保有安装了**build-essential, libx11-dev**, 当然了, 你的Linux可能不是X11, 做出相应修改即可.

```
sudo apt-get install build-essential libx11-dev
```

-----

## FreeImage

> 纹理加载我选用的是FreeImage, 使用也比较简单, 下载, 编译, 安装即可.

```
make
make install
```

![](https://imgconvert.csdnimg.cn/aHR0cHM6Ly90dmExLnNpbmFpbWcuY24vbGFyZ2UvMDA2dE5iUndseTFnYjJmM2l1c2lpajMxc3YwdTB3bWwuanBn?x-oss-process=image/format,png)

> 不知道为啥, 我打不开官网, 不过有人在[github备份](https://github.com/WinMerge/freeimage)了.

-----

## CMakeLists.txt解析

> 先贴出CMakeLists.txt全文.

```
cmake_minimum_required(VERSION 3.15)
project(gles_demo)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SDK_ROOT /opt/Imagination/PowerVR_Graphics/PowerVR_SDK/SDK_2020_R2) # sdk目录
set(CMAKE_MODULE_PATH ${SDK_ROOT}/cmake/modules)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(INCLUDE_DIR ${SDK_ROOT}/include/)

# gles库
find_library(EGL_LIBRARY EGL "/opt/Imagination/PowerVR_Graphics/PowerVR_Tools/PVRVFrame/Library/Linux_x86_64/")
find_library(GLES_LIBRARY GLESv2 "/opt/Imagination/PowerVR_Graphics/PowerVR_Tools/PVRVFrame/Library/Linux_x86_64/")
# FreeImage库
find_library(FI_LIBRARY freeimage "/usr/lib/")

list(APPEND PLATFORM_LIBS ${EGL_LIBRARY} ${GLES_LIBRARY} ${FI_LIBRARY} ${CMAKE_DL_LIBS})

if (UNIX)
    set(WS_DEFINE "")
    if (NOT WS)
        set(WS "X11")
        set(WS_DEFINE "${WS}")
    endif ()

    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
    endif ()

    add_definitions(-D${WS_DEFINE})

    if (${WS} STREQUAL X11)
        find_package(X11 REQUIRED)

        if (NOT ${X11_FOUND})
            message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
        endif ()

        list(APPEND PLATFORM_LIBS ${X11_LIBRARIES})
        include_directories(${X11_INCLUDE_DIR})

        set(SRC_FILES gles_x11.cpp) # 源码
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
```

> 可以看到, 我手动添加了FreeImage库和模拟器给的两个库, libEGL.so, libGLESv2.so. 这里我想吐槽一下win, 非要搞出一个.lib, 又一个.dll, 明明一个.so就搞定的事情.
> 至于X11的库, 之前也说了, 如果你是其他的Linux, 找对应的库, 修改CMake内容即可, 当然了, cpp文件也要重写. 所以, 这里才用了PowerVR的例子, 他们已经把全平台的CMake和源码都写好了, 改改就行(手机狗头). 当然, OpenGL_ES指南有一份跨平台的源码, 我也尝试过, 缺点是似乎只能使用c语言, 我反复修改构建也是如此, 可能是我对编译原理的理解还不到位, 所以就放弃了指南的源码. 毕竟都是要二次封装的, 只用c的话, 臣妾做不到啊(手动无奈).

-----

## 源码

> 源码部分, 我先用一个Util类封装了大部分不需要过多关系的操作, 把处理重心放在初始化和绘制上面. 所以先来看下main函数的内容.

```
int main(int /*argc*/, char ** /*argv*/) {
    start = clock();

    // opengl_es工具类实例
    GLESUtils glesUtils;

    // 设置窗口大小和名称
    glesUtils.setWindowWH(1600, 900);
    std::string appName = std::string("GLES Demo");
    glesUtils.setAppName(appName);

    // 初始化本地和EGL相关
    glesUtils.initNativeAndEGL();

    // 初始化shader
    if (!glesUtils.initShaders()) { glesUtils.cleanProc(); }

    // 绘图, 循环次数为帧数
    for (int i = 0; i < 80000; ++i) {
        if (!glesUtils.renderScene()) {
            break;
        }
    }

    // 释放资源
    glesUtils.deInitGLState();

    return 0;
}
```

> * 生成工具类实例
> * 设置窗口大小和名称
> * 初始化本地和EGL相关变量
> * 然后是关键的初始化shader和绘图

-----

## 最后

> 至于更多有关OpenGL_ES的内容, 就要等后续的部分啦. 喜欢记得点赞或者关注哦~
