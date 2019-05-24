################################################################################
# Basic Wii U Pong Game
# This is a re-write of the long-existing Wii U Pong example project, with a
# greatly simplified codebase that should be easier for less-experienced
# programmers to understand, and a revised build system that allows building
# the program using the latest Wii U tools.
#
# Copyright (C) 2015-2016  Relys, crowell, Hykem, Marionumber1, FIX94,
#                          TheJosamilu, gudenau
# Copyright (C) 2016       dimok
# Copyright (C) 2016-2017  Shiny Quagsire, dibas, exjam, CreeperMario
# Copyright (C) 2018-2019  CreeperMario
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see https://www.gnu.org/licenses/
################################################################################

# CMakeLists.txt
# This is the build script that is run through CMake to create a series of
# Makefiles for building and deploying the project. It loosely resembles a
# CMake script found in the WUT samples directory, but with some extra special
# sauce to automate deployment of the app to HBL/installable channel packages

# This is all pretty self-explanatory.
cmake_minimum_required(VERSION 3.2)
set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/wut/share/wut.toolchain.cmake")
project(pong-example C)
include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)

# Determine whether or not wiiload and makefst are installed.
find_program(found-wiiload wiiload)
find_program(found-makefst makefst)

add_executable(pong
    src/game.c
    src/graphics.c
    src/main.c)

# Link to the standard maths library, provides implementations of the sine and
# cosine trigonometric functions.
target_link_libraries(pong m)

# Transforms the executable created above into an RPX file for the Wii U
wut_create_rpx(pong.rpx pong)

# If wiiload was found, add a 'run' target that executes wiiload on the
# created RPX file
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

# What follows is a set of install commands that copy both the RPX and files
# in the deploy folder into multiple packages for later deployment of the app
# as a Homebrew Launcher app or an installable channel app. See the README
# for more information.

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

# By default, 'make clean' will only remove the build assets and original
# ELF executable - this allows 'make clean' to also remove the RPX and
# deployment assets from the build directory.
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
