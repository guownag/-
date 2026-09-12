cmake_minimum_required(VERSION 3.20)
project(embedded_cpp_lab LANGUAGES CXX)

# ---------------------------------------------------------------------------
# 全局标准：C++17。
# 为什么不直接上 C++20/23？因为很多 MCU 工具链（老版 arm-none-eabi-gcc、
# IAR、Keil）对 C++20 支持不全。把 17 当基线，写出来的代码到哪都能用。
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(LAB_BUILD_SOLUTIONS "编译参考答案（用来对照，不要先看）" ON)
option(LAB_WERROR "把警告当成错误" ON)

# ---------------------------------------------------------------------------
# 公共编译选项。嵌入式要求：警告开到最狠，并且不许有警告。
# 如果某条警告把你卡住了，可以先用 -DLAB_WERROR=OFF 过关，但别养成习惯。
# ---------------------------------------------------------------------------
add_library(lab_flags INTERFACE)

if(MSVC)
  target_compile_options(lab_flags INTERFACE
    /W4            # 高警告等级
    /permissive-   # 关闭 MSVC 的宽容模式，向标准靠拢
    /utf-8         # 源码按 UTF-8 读
    /Zc:__cplusplus
  )
  if(LAB_WERROR)
    target_compile_options(lab_flags INTERFACE /WX)
  endif()
else()
  target_compile_options(lab_flags INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wcast-align -Wcast-qual -Wundef -Wdouble-promotion
  )
  if(LAB_WERROR)
    target_compile_options(lab_flags INTERFACE -Werror)
  endif()
endif()

add_library(lab_headers INTERFACE)
target_include_directories(lab_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 每一课都调用这个函数注册，统一选项，避免每课重复写。
function(lab_add_lesson target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PRIVATE lab_headers lab_flags)
endfunction()

enable_testing()

add_subdirectory(lessons/00-toolchain)
add_subdirectory(lessons/01-types-and-bits)
add_subdirectory(lessons/02-pointers-and-memory)
add_subdirectory(lessons/03-functions-and-modules)
add_subdirectory(lessons/04-classes-and-raii)
add_subdirectory(lessons/05-templates-and-compile-time)
