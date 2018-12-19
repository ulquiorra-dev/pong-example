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

#include <coreinit/memheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/screen.h>
#include <proc_ui/procui.h>
#include <whb/log.h>

#include "graphics.h"

BOOL graphics_initialized = FALSE;
BOOL framebuffer_initialized = FALSE;

MEMHeapHandle mem1_heap;

int tv_size = 0;
int gamepad_size = 0;

void * tv_buffer;
void * gamepad_buffer;

void pong_graphics_clearbuffers()
{
    pong_graphics_clearscreen(0, 0, 0);
    pong_graphics_render();

    pong_graphics_clearscreen(0, 0, 0);
    pong_graphics_render();
}

uint32_t pong_graphics_initbuffers(void * context)
{
    if(framebuffer_initialized) return 0;

    WHBLogPrint("Initializing MEM1 memory heap...");
    mem1_heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    MEMRecordStateForFrmHeap(mem1_heap, 0x504F4E47);

    WHBLogPrint("Initializing OSScreen...");
    OSScreenInit();

    tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    gamepad_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    WHBLogPrintf("Allocating %d bytes for TV framebuffer, %d bytes for "
                 "GamePad framebuffer...", tv_size, gamepad_size);
    tv_buffer = MEMAllocFromFrmHeapEx(mem1_heap, tv_size, 0x100);
    gamepad_buffer = MEMAllocFromFrmHeapEx(mem1_heap, gamepad_size, 0x100);

    if(!tv_buffer || !gamepad_buffer) {
        WHBLogPrint("Failed to allocate a framebuffer in memory. You won't "
                    "see anything while the program runs. :(");
        MEMFreeByStateToFrmHeap(mem1_heap, 0x504F4E47);
        return -1;
    }

    WHBLogPrintf("TV framebuffer is located at 0x%08X, GamePad framebuffer is "
                 "located at 0x%08X.", tv_buffer, gamepad_buffer);

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

    pong_graphics_clearbuffers();

    WHBLogPrint("De-allocating framebuffers...");
    MEMFreeByStateToFrmHeap(mem1_heap, 0x504F4E47);

    framebuffer_initialized = FALSE;
    return 0;
}

void pong_graphics_init()
{
    if(graphics_initialized) return;

    pong_graphics_initbuffers(NULL);

    WHBLogPrint("Registering foreground acquire/release callbacks...");
    ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, pong_graphics_initbuffers,
                           NULL, 100);
    ProcUIRegisterCallback(PROCUI_CALLBACK_RELEASE, pong_graphics_freebuffers,
                           NULL, 100);

    graphics_initialized = TRUE;
}

void pong_graphics_shutdown()
{
    if(!graphics_initialized) return;

    pong_graphics_freebuffers(NULL);

    WHBLogPrint("Clearing foreground acquire/release callbacks...");
    ProcUIClearCallbacks();

    graphics_initialized = FALSE;
}

void pong_graphics_clearscreen(uint8_t red, uint8_t green, uint8_t blue)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    uint32_t colour = (red << 24) | (green < 16) | (blue < 8);

    OSScreenClearBufferEx(SCREEN_TV, colour);
    OSScreenClearBufferEx(SCREEN_DRC, colour);
}

void pong_graphics_render()
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}
