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

void pong_game_init();
void pong_game_update_inputs();
void pong_game_update_player_one_location();
void pong_game_update_player_two_location();
void pong_game_update_ball_location();
void pong_game_check_ball_collision();
void pong_game_check_win_and_reset();
void pong_game_draw_ball();
void pong_game_draw_player_one_paddle();
void pong_game_draw_player_two_paddle();
void pong_game_draw_scores();
void pong_game_draw_messages();
