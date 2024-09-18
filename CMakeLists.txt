cmake_minimum_required(VERSION 3.20)
project(httpserver LANGUAGES  CXX)


if (MSVC)
    add_compile_options(/Zc:preprocessor /utf-8 /DNOMINMAX /D_USE_MATH_DEFINES /EHsc /bigobj)
else()
    if (WIN32)
        add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8)
    endif()
    add_compile_options(-Wall -Wextra -Werror=return-type)
endif()


set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 获取所有源文件
file(GLOB SOURCES "./example/*.cpp")

foreach(example ${SOURCES})
    # 获取文件名（不包括路径）
    get_filename_component(FILE_NAME ${example} NAME_WE)
    
    # 生成目标名称
    set(TARGET_NAME ${CMAKE_PROJECT_NAME}_${FILE_NAME})
    
    # 添加可执行文件
    add_executable(${TARGET_NAME} ${example})
    
    # 输出目标名称
    message("target: ${TARGET_NAME}")
    
    # 设置包含目录
    target_include_directories(${TARGET_NAME} PUBLIC "./")

    target_link_libraries(${TARGET_NAME} PRIVATE Ws2_32)
endforeach()