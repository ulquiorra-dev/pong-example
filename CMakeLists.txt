cmake_minimum_required(VERSION 3.2)
set(CMAKE_TOOLCHAIN_FILE "$ENV{WUT_ROOT}/share/wut.toolchain.cmake")
project(pong-example C)
include("${WUT_ROOT}/share/wut.cmake" REQUIRED)

find_program(found-wiiload wiiload)
find_program(found-makefst makefst)

add_executable(pong
    src/graphics.c
    src/main.c)

target_link_libraries(pong
    whb
    coreinit
    nsysnet
    proc_ui
    sysapp)

wut_create_rpx(pong.rpx pong)

if(${found-wiiload} STREQUAL found-wiiload-NOTFOUND)
    message(WARNING
        "The wiiload tool was not found on your system. The 'run' target will "
        "not be created.")
else()
    add_custom_target(run
        COMMAND wiiload "${CMAKE_BINARY_DIR}/pong.rpx"
        DEPENDS pong.rpx
        VERBATIM)
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/hbl/"
    DESTINATION "${CMAKE_BINARY_DIR}/hbl_sd_root/wiiu/apps/pong-example")

install(FILES "${CMAKE_BINARY_DIR}/pong.rpx"
    DESTINATION "${CMAKE_BINARY_DIR}/hbl_sd_root/wiiu/apps/pong-example")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/deploy/channel/"
    DESTINATION "${CMAKE_BINARY_DIR}/unpacked_root")

install(FILES "${CMAKE_BINARY_DIR}/pong.rpx"
    DESTINATION "${CMAKE_BINARY_DIR}/unpacked_root/code")

install(CODE "execute_process(
    WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}/hbl_sd_root\"
    COMMAND cmake -E tar cf ../pong-hbl.zip --format=zip wiiu/apps)")

install(CODE "execute_process(
    WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}/unpacked_root\"
    COMMAND cmake -E tar cf ../pong-channel-unpacked.zip -- code meta)")

if(${found-makefst} STREQUAL found-makefst-NOTFOUND)
    message(WARNING
        "The makefst tool was not found on your system. The 'install' target "
        "will not generate any installable FST/Woomy packages.")
else()

    install(CODE "file(MAKE_DIRECTORY
        \"${CMAKE_BINARY_DIR}/wupinstaller_sd_root/install/pong-example\")")

    install(CODE "execute_process(
        WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
        COMMAND makefst -rawout wupinstaller_sd_root/install/pong-example
        -out pong-channel.woomy -icon unpacked_root/meta/iconTex.tga
        -name \"Wii U Pong Game\" -internal pong-example unpacked_root)")

    install(CODE "execute_process(
        WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}/wupinstaller_sd_root\"
        COMMAND cmake -E tar cf ../pong-channel-wupinstaller.zip install)")
endif()

set(files_to_remove
    "${CMAKE_BINARY_DIR}/hbl_sd_root"
    "${CMAKE_BINARY_DIR}/loadiine_sd_root"
    "${CMAKE_BINARY_DIR}/unpacked_root"
    "${CMAKE_BINARY_DIR}/wupinstaller_sd_root"
    "${CMAKE_BINARY_DIR}/pong.rpx"
    "${CMAKE_BINARY_DIR}/pong-hbl.zip"
    "${CMAKE_BINARY_DIR}/pong-channel-unpacked.zip"
    "${CMAKE_BINARY_DIR}/pong-channel-wupinstaller.zip"
    "${CMAKE_BINARY_DIR}/pong-channel.woomy")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    "${files_to_remove}")
