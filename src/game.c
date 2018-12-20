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

#include <vpad/input.h>
#include <whb/log.h>

#include "game.h"
#include "graphics.h"

VPADStatus gamepad_status;
VPADReadError gamepad_communication_error;

BOOL pong_game_running = FALSE;

void pong_game_update_inputs()
{
    VPADRead(VPAD_CHAN_0, &gamepad_status, 1, &gamepad_communication_error);

    if(!pong_game_running && (gamepad_status.trigger & VPAD_BUTTON_A)) {
        WHBLogPrint("[  game  ] Pong game is starting...");
        pong_game_running = TRUE;
    }
}

void pong_game_update_player_one_location()
{

}

void pong_game_update_player_two_location()
{

}

void pong_game_update_ball_location()
{

}

void pong_game_check_ball_collision()
{

}

void pong_game_check_ball_off_screen()
{
    pong_game_running = FALSE;
}

void pong_game_draw()
{
    pong_graphics_clearscreen(16, 32, 32);
    pong_graphics_render();
}
