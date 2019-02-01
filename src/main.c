/*******************************************************************************
 * Basic Wii U Pong Game
 * This is a re-write of the long-existing Wii U Pong example project, with a
 * greatly simplified codebase that should be easier for less-experienced
 * programmers to understand, and a revised build system that allows building
 * the program using the latest Wii U tools.
 *
 * Copyright (C) 2015-2016  Relys, crowell, Hykem, Marionumber1, FIX94,
 *                          TheJosamilu, gudenau
 * Copyright (C) 2016       dimok
 * Copyright (C) 2016-2017  Shiny Quagsire, dibas, exjam, CreeperMario
 * Copyright (C) 2018-2019  CreeperMario
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see https://www.gnu.org/licenses/
 ******************************************************************************/

// main.c
//
// This file contains the main initialization and shutdown routines that are
// common to most Wii U software. This is where the program begins and finishes
// execution. It also contains the main loop that keeps the program running.

// This is used to set up WUT's built-in crash handler - in the event of a
// crash, low-level information about the crash (e.g. stack dump) will be
// sent through WUT's logging utilities.
#include <whb/crash.h>

// This is used to send messages through WUT's logging interface - a method of
// sending messages to external sources about what the program is doing at any
// given time. WUT can be configured to use a number of different interfaces
// for logging, see below for some examples.
#include <whb/log.h>

// Used to enable WUT to store log messages in the Cafe OS system log, which is
// mostly useful for running the program using decaf-cli, as Cafe system log
// messages are output by Decaf. In certain circumstances, the Cafe OS log is
// dumped to NAND after an error, and can be read later using a custom firmware
// that enables access to the NAND filesystem (specifically the 'logs' folder
// on the SLC NAND chip).
#include <whb/log_cafe.h>

// Used to enable WUT to broadcast log messages over the local network using
// UDP port 4405. This is useful when the program is running on a Wii U
// console, as you can see the log messages by connecting your computer to the
// same network and running the udplogserver tool included with WUT.
#include <whb/log_udp.h>

// This is WUT's high-level interface to ProcUI, the Wii U's method of enabling
// multi-tasking. When running the program using the Homebrew Launcher, ProcUI
// merely serves the purpose of signalling the program to exit when the user
// presses the HOME Button. If the program is installed as an app on the
// console, when the user presses the HOME Button, ProcUI will suspend the
// program and move it from the foreground into the background, and start
// displaying the HOME Menu, allowing the user to run apps like the Friends
// List and Internet Browser. Once the HOME Menu is closed, or the secondary
// app has finished running, ProcUI will move our program back into the
// foreground and continue running it. When the user presses the "Close
// software" button on the HOME Menu, ProcUI will signal to our app that it
// needs to begin exiting.
#include <whb/proc.h>

// These provide the definitions for the functions in game.c and graphics.c
#include "game.h"
#include "graphics.h"

// On the Wii U, colours are represented as unsigned 32-bit numbers which,
// when represented in hexadecimal format, resemble HTML colour codes.
// In this case, the first two hexadecimal characters represent the amount of
// red in the colour, the next two represent green, and the next two represent
// blue. The final two characters have no effect on the image. So the colour
// defined here can be represented with the HTML colour code #203030, which
// means 32 red, 48 green and 48 blue, which is a dark teal colour.
// Feel free to experiment with new colour values. :)
#define BACKGROUND_COLOUR 0x20303000

// After WUT runs its behind-the-scenes initialization stuff, this is where
// the program will begin running.
int main(int argc, char ** argv)
{
    // Enable the logging interfaces.
    WHBLogCafeInit();
    WHBLogUdpInit();
    WHBLogPrint("[  main  ] Pong is starting up...");

    // Enable the crash handler.
    WHBLogPrint("[  main  ] Starting crash handler...");
    WHBInitCrashHandler();

    // Enable ProcUI.
    WHBLogPrint("[  main  ] Starting process management...");
    WHBProcInit();

    // Set up the included graphics library. See graphics.c for more
    // information.
    WHBLogPrint("[  main  ] Starting graphics system...");
    pong_graphics_init();

    // Set up the Pong game itself. See game.c for more information.
    WHBLogPrint("[  main  ] Initializing pong game...");
    pong_game_init();

    // The program runs in a loop, continuously checking controller inputs,
    // adjusting game elements and drawing the game status to the screen.
    // This loop runs until ProcUI instructs our program to begin exiting,
    // which occurs immediately once the user presses the HOME Button
    // (Homebrew Launcher users) or once the user presses "Close software"
    // on the HOME Menu (installable app users).
    WHBLogPrint("[  main  ] Starting main loop...");
    while(WHBProcIsRunning())
    {
        pong_game_update_inputs();
        pong_game_update_player_one_location();
        pong_game_update_player_two_location();
        pong_game_update_ball_location();
        pong_game_check_ball_collision();
        pong_game_check_win_and_reset();

        pong_graphics_clearscreen(BACKGROUND_COLOUR);
        pong_game_draw_ball();
        pong_game_draw_player_one_paddle();
        pong_game_draw_player_two_paddle();
        pong_game_draw_scores();
        pong_game_draw_messages();
        pong_graphics_render();
    }

    // At this point, the loop has finished, and so the program must begin
    // exiting. Pong does not need to do much in order to properly shut down,
    // but for other programs, shutdown may be more complicated.

    // Shut down the included graphics system.
    WHBLogPrint("[  main  ] Shutting down graphics system...");
    pong_graphics_shutdown();

    // Shut down ProcUI.
    WHBLogPrint("[  main  ] Shutting down process management...");
    WHBProcShutdown();

    // Shut down the logging interfaces, this includes closing the networking
    // socket that was used by the network logging interface.
    WHBLogPrint("[  main  ] Pong is shutting down...");
    WHBLogUdpDeinit();
    WHBLogCafeDeinit();

    // Our program stops here. WUT will perform some behind-the-scenes shutdown
    // of its own, before returning to the Homebrew Launcher or the Wii U Menu.
    return 0;
}
