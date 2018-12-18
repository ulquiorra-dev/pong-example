cmake_minimum_required(VERSION 3.2)
set(CMAKE_TOOLCHAIN_FILE "$ENV{WUT_ROOT}/share/wut.toolchain.cmake")
project(pong-example C)
include("${WUT_ROOT}/share/wut.cmake" REQUIRED)

find_program(found-wiiload wiiload)

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

if(${found-wiiload} STREQUAL found-wiiload-NOTFOUND)
    message(WARNING
        "The wiiload tool was not found on your system. The 'run' target will "
        "not be created."
    )
else()
    add_custom_target(run
        COMMAND wiiload pong.rpx
        DEPENDS pong.rpx
        VERBATIM
    )
endif()

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    ${CMAKE_BINARY_DIR}/pong.rpx
)
