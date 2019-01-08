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

#include <coreinit/cache.h>
#include <coreinit/memheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/screen.h>
#include <proc_ui/procui.h>
#include <string.h>
#include <whb/log.h>

#include "graphics.h"

#define CALLBACK_PRIORITY     100
#define FRAMEBUFFER_ALIGNMENT 0x100
#define PONG_MEMORY_STATE     0x504F4E47

#define TEXT_CONSOLE_WIDTH    69
#define TEXT_LEFT_BOUNDARY    -4

#define COLOUR_BLACK          0x00000000

BOOL graphics_initialized = FALSE;
BOOL framebuffer_initialized = FALSE;

MEMHeapHandle mem1_heap;

int tv_size = 0;
int gamepad_size = 0;

void * tv_buffer;
void * gamepad_buffer;

void pong_graphics_clearbuffers()
{
    pong_graphics_clearscreen(COLOUR_BLACK);
    pong_graphics_render();

    pong_graphics_clearscreen(COLOUR_BLACK);
    pong_graphics_render();
}

uint32_t pong_graphics_initbuffers(void * context)
{
    if(framebuffer_initialized) return 0;

    WHBLogPrint("[graphics] Acquiring foreground...");

    WHBLogPrint("[graphics] Initializing MEM1 memory heap...");
    mem1_heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    MEMRecordStateForFrmHeap(mem1_heap, PONG_MEMORY_STATE);

    WHBLogPrint("[graphics] Initializing OSScreen...");
    OSScreenInit();

    tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    gamepad_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    WHBLogPrintf("[graphics] Allocating %d bytes for TV framebuffer, %d bytes "
                 "for GamePad framebuffer...", tv_size, gamepad_size);
    tv_buffer = MEMAllocFromFrmHeapEx(mem1_heap, tv_size,
                                      FRAMEBUFFER_ALIGNMENT);
    gamepad_buffer = MEMAllocFromFrmHeapEx(mem1_heap, gamepad_size,
                                           FRAMEBUFFER_ALIGNMENT);

    if(!tv_buffer || !gamepad_buffer) {
        WHBLogPrint("[graphics] Failed to allocate a framebuffer in memory. "
                    "You won't see anything while the program runs. :(");
        MEMFreeByStateToFrmHeap(mem1_heap, PONG_MEMORY_STATE);
        return -1;
    }

    WHBLogPrintf("[graphics] TV framebuffer is located at 0x%08X, GamePad "
                 "framebuffer is located at 0x%08X.", tv_buffer,
                 gamepad_buffer);

    OSScreenSetBufferEx(SCREEN_TV, tv_buffer);
    OSScreenSetBufferEx(SCREEN_DRC, gamepad_buffer);

    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    framebuffer_initialized = TRUE;

    pong_graphics_clearbuffers();
    return 0;
}

uint32_t pong_graphics_freebuffers(void * context)
{
    if(!framebuffer_initialized) return 0;

    WHBLogPrint("[graphics] Releasing foreground...");

    pong_graphics_clearbuffers();

    WHBLogPrint("[graphics] De-allocating framebuffers...");
    MEMFreeByStateToFrmHeap(mem1_heap, PONG_MEMORY_STATE);

    framebuffer_initialized = FALSE;
    return 0;
}

void pong_graphics_init()
{
    if(graphics_initialized) return;

    pong_graphics_initbuffers(NULL);

    WHBLogPrint("[graphics] Registering foreground callbacks...");
    ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, pong_graphics_initbuffers,
                           NULL, CALLBACK_PRIORITY);
    ProcUIRegisterCallback(PROCUI_CALLBACK_RELEASE, pong_graphics_freebuffers,
                           NULL, CALLBACK_PRIORITY);

    graphics_initialized = TRUE;
}

void pong_graphics_shutdown()
{
    if(!graphics_initialized) return;

    pong_graphics_freebuffers(NULL);

    WHBLogPrint("[graphics] Clearing foreground callbacks...");
    ProcUIClearCallbacks();

    graphics_initialized = FALSE;
}

void pong_graphics_clearscreen(uint32_t colour)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    OSScreenClearBufferEx(SCREEN_TV, colour);
    OSScreenClearBufferEx(SCREEN_DRC, colour);
}

void pong_graphics_render()
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    DCFlushRange(tv_buffer, tv_size);
    DCFlushRange(gamepad_buffer, gamepad_size);

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

void pong_graphics_draw_pixel(uint32_t x, uint32_t y, uint32_t colour)
{
    if(!graphics_initialized || !framebuffer_initialized) return;
    //TODO: Draw to TV screen
    OSScreenPutPixelEx(SCREEN_DRC, x, y, colour);
}

void pong_graphics_draw_rectangle(int x, int y, int width, int height,
                                  uint32_t colour)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    for(int i = (x - (width / 2)); i < (x + (width / 2)); i++) {
        for(int j = (y - (height / 2)); j < (y + (height / 2)); j++) {
            pong_graphics_draw_pixel(i, j, colour);
        }
    }
}

void pong_graphics_draw_text(const char * string, int32_t x, int32_t y)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    //TODO: Draw to TV screen
    OSScreenPutFontEx(SCREEN_DRC, x, y, string);
}

void pong_graphics_draw_text_centre(const char * string, int32_t line)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    int x = ((TEXT_CONSOLE_WIDTH - strlen(string)) / 2) + TEXT_LEFT_BOUNDARY;
    pong_graphics_draw_text(string, x, line);
}
