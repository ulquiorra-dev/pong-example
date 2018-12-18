cmake_minimum_required(VERSION 3.2)
set(CMAKE_TOOLCHAIN_FILE "$ENV{WUT_ROOT}/share/wut.toolchain.cmake")
project(pong-example C)
include("${WUT_ROOT}/share/wut.cmake" REQUIRED)

add_executable(pong
    main.c
)

target_link_libraries(pong
    whb
    coreinit
    nsysnet
    proc_ui
    sysapp
)

wut_create_rpx(pong.rpx pong)
