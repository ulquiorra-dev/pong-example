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

// game.c
//
// This file contains everything related to reading controller input, handling
// the Pong game mechanics and using the graphics functions to draw the game
// state to the screen.
//
// To play the game, here are the controls:
// - Press + to pause/unpause the game
// - Use the up/down buttons on the directional pad to move the left paddle
// - Use X and B to move the right paddle
// - Use HOME to exit (Homebrew Launcher) or open the HOME Menu (app launched
//   from the Wii U Menu)
//
// A ball will be at the centre of the screen, and will bounce off the top/
// bottom of the screen. You must use your paddle to prevent the ball from
// moving off-screen, if the ball moves off your side of the screen, the
// opponent will earn a point.

// Used to access OSGetTime() for seeding the pseudo-random number generator.
#include <coreinit/time.h>

// Used to access implementations of the trigonometric functions sin() and
// cos() provided by the standard maths library, for rotation.
#include <math.h>

// Used to access the snprintf() standard library function.
#include <stdio.h>

// Used to access the pseudo-random number generation functions rand() and
// srand() provided by the standard C library.
#include <stdlib.h>

// Provides functions and structs to determine the current state of the
// GamePad, including button presses, rotation and touch screen information.
#include <vpad/input.h>

// Used to send messages through WUT's logging system.
#include <whb/log.h>

#include "game.h"
#include "graphics.h"

// Exact value of Pi (3.1415926...) - used to convert from degrees to radians.
#define PI (22.0 / 7.0)

// These represent the dimensions of the GamePad screen, 854x480 in size.
#define SCREEN_LEFT_BOUNDARY      0
#define SCREEN_RIGHT_BOUNDARY     854
#define SCREEN_TOP_BOUNDARY       0
#define SCREEN_BOTTOM_BOUNDARY    480

// These represent the dimensions of the GamePad screen's text interface - see
// graphics.c for more information.
#define TEXT_LEFT_BOUNDARY        -4
#define TEXT_RIGHT_BOUNDARY       65
#define TEXT_TOP_BOUNDARY         -1
#define TEXT_BOTTOM_BOUNDARY      17

// Miscellaneous constant values related to how the scores for both players are
// displayed on screen. SCORE_BUFFER_SIZE is the number of characters to be
// displayed, plus a null-terminator as required by a normal C string.
// SCORE_DISPLAY_FORMAT is a format specifier, see documentation for the
// printf() family of functions for more details.
#define SCORE_NUM_CHARACTERS      3
#define SCORE_BUFFER_SIZE         (SCORE_NUM_CHARACTERS) + 1
#define SCORE_DISPLAY_FORMAT      "%03d"
#define SCORE_TEXT_X_POSITION     12
#define SCORE_TEXT_Y_POSITION     0

// The speed of movement of the player paddles and ball. For the paddles, this
// determines the number of pixels to move the paddle per frame when a button
// is held down. For the ball, this is multiplied by the calculated movement
// factors to determine the speed of the ball in separate x and y directions.
// See pong_game_generate_ball_direction() for more information.
#define PADDLE_SPEED              10
#define BALL_SPEED                7.5

// Constant values that determine the size and initial location of the player
// paddles at the beginning of a game.
#define PADDLE_X_POSITION         50
#define PADDLE_INITIAL_Y_POSITION 240
#define PADDLE_WIDTH              20
#define PADDLE_HEIGHT             100
#define PADDLE_HIGHEST_POSITION   (SCREEN_TOP_BOUNDARY + (PADDLE_HEIGHT / 2))
#define PADDLE_LOWEST_POSITION    (SCREEN_BOTTOM_BOUNDARY - (PADDLE_HEIGHT / 2))

// The colours of the paddles - see main.c for more information on the colour
// encoding format.
#define PADDLE_ONE_COLOUR         0x7FFF7F00
#define PADDLE_TWO_COLOUR         0x7F7FFF00

// The initial location, size and colour of the ball.
#define BALL_INITIAL_X_POSITION   (SCREEN_RIGHT_BOUNDARY / 2)
#define BALL_INITIAL_Y_POSITION   (SCREEN_BOTTOM_BOUNDARY / 2)
#define BALL_CIRCUMFERENCE        15
#define BALL_COLOUR               0xFF7F7F00

// Used to store information about the GamePad's state at any given point
// in time. A lot of information is available in this struct, but we are
// only interested in gamepad_status.hold, which is a bitmask containing
// all the buttons on the GamePad that are currently being held down.
// You can use a bitwise AND operation to determine whether or not a specific
// button is being held down, e.g. (gamepad_status.hold & VPAD_BUTTON_A) will
// be TRUE if the GamePad's A button is currently being held down.
// We also use a similar bitmask called gamepad_status.trigger which contains
// all the buttons that have only just been pressed since the last time the
// GamePad was queried, so querying the GamePad again will result in the button
// press being listed in .hold rather than .trigger.
VPADStatus gamepad_status;

// Remember that the GamePad is connected to the Wii U console using a Wi-Fi
// hotspot, and so there could be intermittent errors in retrieving the data
// from the GamePad. When an error occurs, there is no guarantee that
// gamepad_status will contain correct data about the GamePad's state.
// When an error occurs, this variable is set accordingly, and so can be used
// to prevent the program from reading false GamePad data.
VPADReadError gamepad_communication_status;

// Determines whether or not the states of the paddles/ball should be reset,
// e.g. after the ball moves off the left/right sides of the screen.
BOOL game_should_reset;

// Determines whether or not the game should be calculating movement - the game
// should be halted when a player has won or when the game is paused (by
// pressing the + button).
BOOL game_halted;

// When the game is halted or paused, a message is displayed at the bottom of
// the screen, this is a pointer to that message.
char * screen_message;

// Variables used to store the current scores for both players. The score is
// stored both as a number, and as a 3-digit string that is displayed on the
// screen.
int player_one_score;
int player_two_score;
char player_one_score_string[SCORE_BUFFER_SIZE];
char player_two_score_string[SCORE_BUFFER_SIZE];

// Variables that determine the ball's current location in 2D space, and the
// "movement value" for the x and y axis, which determines how many pixels in
// both the x and y directions the ball should move per frame.
int ball_position_x;
int ball_position_y;
int ball_movement_x;
int ball_movement_y;

// The vertical position of the player paddles (the paddles can only be moved
// up or down).
int player_one_paddle_position;
int player_two_paddle_position;

// This function is used at the beginning of a game to calculate a new
// pseudo-random direction for the ball to travel.
void pong_game_generate_ball_direction()
{
    BOOL valid_rotation = FALSE;
    int degrees;

    // This loop is responsible for generating a random number between 0 and
    // 359, to represent the direction of the ball in degrees. However, the
    // game would get very boring very quickly if the ball was moving almost
    // directly up/down or left/right. So there's a range of directions which
    // are "whitelisted" or allowed to be used, such that the ball does not
    // spend too much time bouncing around.
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

    // This converts the direction value from degrees to radians, for use by
    // the trigonometric functions below.
    double radians = degrees * (PI / 180);
    WHBLogPrintf("[  game  ] Initial ball direction: %d degrees (%f radians)",
                 degrees, radians);

    // This is the most difficult part of the function to understand. I could
    // begin diving into all the low-level aspects of trigonometry and the
    // unit circle and how this relates to rotation, but basically, once you
    // have a direction value in radians, you can use the cosine and sine
    // functions respectively to calculate the respective x and y translation
    // that must occur in order to move the object one unit in that direction.
    // Note that this number can be negative, meaning the ball moves to the
    // left or top of the screen rather than towards the right/bottom.
    // Also note that the value will most likely be a decimal number, which is
    // rounded to a full number here because you cannot move an object by a
    // fraction of a pixel.
    ball_movement_x = cos(radians) * BALL_SPEED;
    ball_movement_y = sin(radians) * BALL_SPEED;
    WHBLogPrintf("[  game  ] Initial ball x/y movement: %d/%d pixels/frame",
                 ball_movement_x, ball_movement_y);
}

// This is called from main() to set up the game's initial state.
void pong_game_init()
{
    // Seed the pseudo-random number generator.
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

// This is called from the main loop to continuously query the GamePad's
// state at that point in time.
void pong_game_update_inputs()
{
    // Get the GamePad's state
    VPADRead(VPAD_CHAN_0, &gamepad_status, 1, &gamepad_communication_status);

    // If there were no errors retrieving information from the GamePad...
    if(gamepad_communication_status == VPAD_READ_SUCCESS) {
        // If the + button has just been pressed...
        if(gamepad_status.trigger & VPAD_BUTTON_PLUS) {
            // If the game is paused/halted...
            if(game_halted) {
                game_halted = FALSE;
                if(!game_should_reset) {
                    WHBLogPrint("[  game  ] Game is resuming...");
                }
            } else {
                WHBLogPrint("[  game  ] Game is pausing...");
                screen_message = "Game is paused";
                game_halted = TRUE;
            }
        }
    }
}

// Used to determine whether or not the left paddle should move up or down,
// based on its current position (it should not move off-screen) and whether
// the up or down buttons of the GamePad's directional pad are being pressed.
void pong_game_update_player_one_location()
{
    if(game_halted) return;
    if(game_should_reset) {
        player_one_paddle_position = PADDLE_INITIAL_Y_POSITION;
        return;
    }

    // If there were no errors communicating with the GamePad...
    if(gamepad_communication_status == VPAD_READ_SUCCESS) {
        // If the up button is being held down...
        if(gamepad_status.hold & VPAD_BUTTON_UP) {
            player_one_paddle_position -= PADDLE_SPEED;
            // If the paddle has partially moved off the top of the screen,
            // move it back to the highest point without it moving off-screen.
            if(player_one_paddle_position < PADDLE_HIGHEST_POSITION) {
                player_one_paddle_position = PADDLE_HIGHEST_POSITION;
            }
        // If the down button is being held down...
        } else if(gamepad_status.hold & VPAD_BUTTON_DOWN) {
            player_one_paddle_position += PADDLE_SPEED;

            if(player_one_paddle_position > PADDLE_LOWEST_POSITION) {
                player_one_paddle_position = PADDLE_LOWEST_POSITION;
            }
        }
    }
}

// Same as above, but for the right-side paddle.
void pong_game_update_player_two_location()
{
    if(game_halted) return;
    if(game_should_reset) {
        player_two_paddle_position = PADDLE_INITIAL_Y_POSITION;
        return;
    }

    if(gamepad_communication_status == VPAD_READ_SUCCESS) {
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

// Handles moving the ball - pretty self-explanatory.
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

// This function is used to determine whether or not the ball needs to "bounce"
// in order to stay on screen, whether that means bouncing off the top/bottom
// walls or off of a player's paddle.
void pong_game_check_ball_collision()
{
    // This is TRUE if the ball is either moving out of the top/bottom
    // boundaries of the screen - to make the ball "bounce" we simply negate
    // its y-axis movement factor.
    if(ball_position_y < (SCREEN_TOP_BOUNDARY + (BALL_CIRCUMFERENCE / 2)) ||
       ball_position_y > (SCREEN_BOTTOM_BOUNDARY - (BALL_CIRCUMFERENCE / 2)))
    {
        ball_movement_y = -ball_movement_y;
        WHBLogPrint("[  game  ] Boing! (ball collided with wall)");
    }

    // In a similar manner, if the ball is partially touching the left paddle,
    // negate its movement along the x-axis to make it bounce.
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

    // Same as above, but for the right-side paddle.
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

// Checks if the ball has moved off of the left/right side of the screen, and
// if so, determines which player won the game and increments the score
// appropriately. It is also responsible for the final part of resetting into
// a new Pong game.
void pong_game_check_win_and_reset()
{
    if(game_halted) return;
    if(game_should_reset) {
        game_should_reset = FALSE;
        game_halted = TRUE;
        screen_message = "New game is ready";
        WHBLogPrint("[  game  ] New pong game is ready");
    }

    // The ball has moved past the left boundary of the screen
    if(ball_position_x < (SCREEN_LEFT_BOUNDARY - 10)) {
        game_should_reset = TRUE;
        game_halted = TRUE;
        screen_message = "Player 2 wins!";
        WHBLogPrint("[  game  ] Player 2 won the game");
        player_two_score++;
    }

    // The ball has moved past the right boundary of the screen
    if(ball_position_x > (SCREEN_RIGHT_BOUNDARY + 10)) {
        game_should_reset = TRUE;
        game_halted = TRUE;
        screen_message = "Player 1 wins!";
        WHBLogPrint("[  game  ] Player 1 won the game");
        player_one_score++;
    }
}

// Draws the square representing the ball
void pong_game_draw_ball()
{
    pong_graphics_draw_rectangle(ball_position_x, ball_position_y,
                                 BALL_CIRCUMFERENCE, BALL_CIRCUMFERENCE,
                                 BALL_COLOUR);
}

// Draws the rectangle representing the left-side paddle.
void pong_game_draw_player_one_paddle()
{
    pong_graphics_draw_rectangle(SCREEN_LEFT_BOUNDARY + PADDLE_X_POSITION,
                                 player_one_paddle_position, PADDLE_WIDTH,
                                 PADDLE_HEIGHT, PADDLE_ONE_COLOUR);
}

// Draws the rectangle representing the right-side paddle.
void pong_game_draw_player_two_paddle()
{
    pong_graphics_draw_rectangle(SCREEN_RIGHT_BOUNDARY - PADDLE_X_POSITION,
                                 player_two_paddle_position, PADDLE_WIDTH,
                                 PADDLE_HEIGHT, PADDLE_TWO_COLOUR);
}

// Converts the score text to a 3-digit string and displays it.
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

// Draws the message at the bottom of the screen for when the game is paused
// or otherwise stopped (beginning/end of a game).
void pong_game_draw_messages()
{
    if(game_halted) {
        pong_graphics_draw_text_centre(screen_message,
                                       (TEXT_BOTTOM_BOUNDARY - 2));
        pong_graphics_draw_text_centre("Press + to start",
                                       (TEXT_BOTTOM_BOUNDARY - 1));
    }
}
