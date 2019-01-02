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

#include <coreinit/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vpad/input.h>
#include <whb/log.h>

#include "game.h"
#include "graphics.h"

#define PI (22.0 / 7.0)

#define SCREEN_LEFT_BOUNDARY      0
#define SCREEN_RIGHT_BOUNDARY     854
#define SCREEN_TOP_BOUNDARY       0
#define SCREEN_BOTTOM_BOUNDARY    480

#define TEXT_LEFT_BOUNDARY        -4
#define TEXT_RIGHT_BOUNDARY       65
#define TEXT_TOP_BOUNDARY         -1
#define TEXT_BOTTOM_BOUNDARY      19

#define SCORE_BUFFER_SIZE         4
#define SCORE_NUM_CHARACTERS      3
#define SCORE_DISPLAY_FORMAT      "%03d"
#define SCORE_TEXT_X_POSITION     12
#define SCORE_TEXT_Y_POSITION     0

#define PADDLE_SPEED              10
#define BALL_SPEED                7.5

#define PADDLE_X_POSITION         50
#define PADDLE_INITIAL_Y_POSITION 240
#define PADDLE_WIDTH              20
#define PADDLE_HEIGHT             100
#define PADDLE_HIGHEST_POSITION   (SCREEN_TOP_BOUNDARY + (PADDLE_HEIGHT / 2))
#define PADDLE_LOWEST_POSITION    (SCREEN_BOTTOM_BOUNDARY - (PADDLE_HEIGHT / 2))

#define PADDLE_ONE_COLOUR         0x7FFF7F00
#define PADDLE_TWO_COLOUR         0x7F7FFF00

#define BALL_INITIAL_X_POSITION   (SCREEN_RIGHT_BOUNDARY / 2)
#define BALL_INITIAL_Y_POSITION   (SCREEN_BOTTOM_BOUNDARY / 2)
#define BALL_CIRCUMFERENCE        15
#define BALL_COLOUR               0xFF7F7F00

VPADStatus gamepad_status;
VPADReadError gamepad_communication_error;

BOOL game_should_reset;
BOOL game_halted;
char * screen_message;

int player_one_score;
int player_two_score;
char player_one_score_string[SCORE_BUFFER_SIZE];
char player_two_score_string[SCORE_BUFFER_SIZE];

int ball_position_x;
int ball_position_y;
int ball_movement_x;
int ball_movement_y;

int player_one_paddle_position;
int player_two_paddle_position;

void pong_game_generate_ball_direction()
{
    BOOL valid_rotation = FALSE;
    int degrees;

    while(!valid_rotation)
    {
        degrees = rand() % 360;

        if((degrees > 15 && degrees < 75) ||
           (degrees > 105 && degrees < 165) ||
           (degrees > 195 && degrees < 255) ||
           (degrees > 285 && degrees < 345))
        {
            valid_rotation = TRUE;
        }
    }

    double radians = degrees * (PI / 180);
    WHBLogPrintf("[  game  ] Initial ball direction: %d degrees (%f radians)",
                 degrees, radians);

    ball_movement_x = cos(radians) * BALL_SPEED;
    ball_movement_y = sin(radians) * BALL_SPEED;
    WHBLogPrintf("[  game  ] Initial ball x/y movement: %d/%d pixels/frame",
                 ball_movement_x, ball_movement_y);
}

void pong_game_init()
{
    srand(OSGetTime());

    screen_message = "Wii U Pong Game";
    game_halted = TRUE;
    game_should_reset = FALSE;
    player_one_paddle_position = PADDLE_INITIAL_Y_POSITION;
    player_two_paddle_position = PADDLE_INITIAL_Y_POSITION;
    ball_position_x = BALL_INITIAL_X_POSITION;
    ball_position_y = BALL_INITIAL_Y_POSITION;
    pong_game_generate_ball_direction();
}

void pong_game_update_inputs()
{
    VPADRead(VPAD_CHAN_0, &gamepad_status, 1, &gamepad_communication_error);

    if(gamepad_communication_error == VPAD_READ_SUCCESS) {
        if(gamepad_status.trigger & VPAD_BUTTON_PLUS) {
            if(game_halted) {
                game_halted = FALSE;
                if(!game_should_reset) {
                    WHBLogPrint("[  game  ] Game is now starting...");
                }
            } else {
                WHBLogPrint("[  game  ] Game is pausing...");
                screen_message = "Game is paused";
                game_halted = TRUE;
            }
        }
    }
}

void pong_game_update_player_one_location()
{
    if(game_halted) return;
    if(game_should_reset) {
        player_one_paddle_position = PADDLE_INITIAL_Y_POSITION;
        return;
    }

    if(gamepad_communication_error == VPAD_READ_SUCCESS) {
        if(gamepad_status.hold & VPAD_BUTTON_UP) {
            player_one_paddle_position -= PADDLE_SPEED;
            if(player_one_paddle_position < PADDLE_HIGHEST_POSITION) {
                player_one_paddle_position = PADDLE_HIGHEST_POSITION;
            }
        } else if(gamepad_status.hold & VPAD_BUTTON_DOWN) {
            player_one_paddle_position += PADDLE_SPEED;
            if(player_one_paddle_position > PADDLE_LOWEST_POSITION) {
                player_one_paddle_position = PADDLE_LOWEST_POSITION;
            }
        }
    }
}

void pong_game_update_player_two_location()
{
    if(game_halted) return;
    if(game_should_reset) {
        player_two_paddle_position = PADDLE_INITIAL_Y_POSITION;
        return;
    }

    if(gamepad_communication_error == VPAD_READ_SUCCESS) {
        if(gamepad_status.hold & VPAD_BUTTON_X) {
            player_two_paddle_position -= PADDLE_SPEED;
            if(player_two_paddle_position < PADDLE_HIGHEST_POSITION) {
                player_two_paddle_position = PADDLE_HIGHEST_POSITION;
            }
        } else if(gamepad_status.hold & VPAD_BUTTON_B) {
            player_two_paddle_position += PADDLE_SPEED;
            if(player_two_paddle_position > PADDLE_LOWEST_POSITION) {
                player_two_paddle_position = PADDLE_LOWEST_POSITION;
            }
        }
    }
}

void pong_game_update_ball_location()
{
    if(game_halted) return;
    if(game_should_reset) {
        ball_position_x = BALL_INITIAL_X_POSITION;
        ball_position_y = BALL_INITIAL_Y_POSITION;
        pong_game_generate_ball_direction();
        return;
    }

    ball_position_x += ball_movement_x;
    ball_position_y += ball_movement_y;
}

void pong_game_check_ball_collision()
{
    if(ball_position_y < (SCREEN_TOP_BOUNDARY + (BALL_CIRCUMFERENCE / 2)) ||
       ball_position_y > (SCREEN_BOTTOM_BOUNDARY - (BALL_CIRCUMFERENCE / 2)))
    {
        ball_movement_y = -ball_movement_y;
        WHBLogPrint("[  game  ] Boing! (ball collided with wall)");
    }

    if(ball_position_y > (player_one_paddle_position - (PADDLE_HEIGHT / 2)) &&
       ball_position_y < (player_one_paddle_position + (PADDLE_HEIGHT / 2)))
    {
        if(ball_position_x < ((SCREEN_LEFT_BOUNDARY + PADDLE_X_POSITION) +
           (PADDLE_WIDTH / 2)) &&
           ball_position_x > (SCREEN_LEFT_BOUNDARY + PADDLE_X_POSITION))
        {
            if(ball_movement_x < 0) {
                ball_movement_x = -ball_movement_x;
                WHBLogPrint("[  game  ] Boing! (ball collided with P1 paddle)");
            }
        }
    }

    if(ball_position_y > (player_two_paddle_position - (PADDLE_HEIGHT / 2)) &&
       ball_position_y < (player_two_paddle_position + (PADDLE_HEIGHT / 2)))
    {
        if(ball_position_x > ((SCREEN_RIGHT_BOUNDARY - PADDLE_X_POSITION) -
           (PADDLE_WIDTH / 2)) &&
           ball_position_x < (SCREEN_RIGHT_BOUNDARY - PADDLE_X_POSITION))
        {
            if(ball_movement_x > 0) {
                ball_movement_x = -ball_movement_x;
                WHBLogPrint("[  game  ] Boing! (ball collided with P2 paddle)");
            }
        }
    }
}

void pong_game_check_ball_off_screen()
{
    if(game_halted) return;
    if(game_should_reset) {
        game_should_reset = FALSE;
        game_halted = TRUE;
        screen_message = "New game is ready";
        WHBLogPrint("[  game  ] New pong game is ready");
    }

    if(ball_position_x < (SCREEN_LEFT_BOUNDARY - 10)) {
        game_should_reset = TRUE;
        game_halted = TRUE;
        screen_message = "Player 2 wins!";
        WHBLogPrint("[  game  ] Player 2 won the game");
        player_two_score++;
    }

    if(ball_position_x > (SCREEN_RIGHT_BOUNDARY + 10)) {
        game_should_reset = TRUE;
        game_halted = TRUE;
        screen_message = "Player 1 wins!";
        WHBLogPrint("[  game  ] Player 1 won the game");
        player_one_score++;
    }
}

void pong_game_draw_ball()
{
    pong_graphics_draw_rectangle(ball_position_x, ball_position_y,
                                 BALL_CIRCUMFERENCE, BALL_CIRCUMFERENCE, BALL_COLOUR);
}

void pong_game_draw_player_one_paddle()
{
    pong_graphics_draw_rectangle(SCREEN_LEFT_BOUNDARY + PADDLE_X_POSITION,
                                 player_one_paddle_position, PADDLE_WIDTH,
                                 PADDLE_HEIGHT, PADDLE_ONE_COLOUR);
}

void pong_game_draw_player_two_paddle()
{
    pong_graphics_draw_rectangle(SCREEN_RIGHT_BOUNDARY - PADDLE_X_POSITION,
                                 player_two_paddle_position, PADDLE_WIDTH,
                                 PADDLE_HEIGHT, PADDLE_TWO_COLOUR);
}

void pong_game_draw_scores()
{
    snprintf(player_one_score_string, SCORE_BUFFER_SIZE, SCORE_DISPLAY_FORMAT,
             player_one_score);
    snprintf(player_two_score_string, SCORE_BUFFER_SIZE, SCORE_DISPLAY_FORMAT,
             player_two_score);
    pong_graphics_draw_text(player_one_score_string,
                            TEXT_LEFT_BOUNDARY + SCORE_TEXT_X_POSITION,
                            SCORE_TEXT_Y_POSITION);
    pong_graphics_draw_text(player_two_score_string,
                            TEXT_RIGHT_BOUNDARY - SCORE_TEXT_X_POSITION -
                            SCORE_NUM_CHARACTERS, SCORE_TEXT_Y_POSITION);
}

void pong_game_draw_messages()
{
    if(game_halted) {
        pong_graphics_draw_text_centre(screen_message, 15);
        pong_graphics_draw_text_centre("Press + to start", 16);
    }
}
