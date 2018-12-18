/*******************************************************************************
 * Basic Wii U Pong Game
 * This is a version of the long-existing Wii U Pong example project, with a
 * greatly simplified codebase and build system that is both friendlier to
 * beginners and more future-proof in terms of running the software.
 *
 * Copyright (C) 2015-2016  Relys, crowell, Hykem, Marionumber1, FIX94,
 *                          TheJosamilu, gudenau
 * Copyright (C) 2016       dimok
 * Copyright (C) 2016-2017  Shiny Quagsire, dibas, exjam, CreeperMario
 * Copyright (C) 2018       CreeperMario
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

#include <whb/crash.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

int main(int argc, char ** argv)
{
    WHBLogUdpInit();
    WHBLogPrint("Pong is starting up...");

    WHBLogPrint("Starting crash handler...");
    WHBInitCrashHandler();

    WHBLogPrint("Starting process management...");
    WHBProcInit();

    WHBLogPrint("Starting main loop...");
    while(WHBProcIsRunning())
    {

    }

    WHBLogPrint("Shutting down process management...");
    WHBProcShutdown();

    WHBLogPrint("Pong is shutting down...");
    WHBLogUdpDeinit();

    return 0;
}
