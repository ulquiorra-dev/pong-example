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

// graphics.c
//
// This file contains everything related to graphics and modifying what is
// shown on screen, as well as managing the memory for the screen contents.
// This is one of the more involved aspects of programming on the Wii U, as
// there is a lot of background information that must be understood.
//
// On the Wii U, there are two graphics systems - GX2 and OSScreen. GX2 is
// kind-of like DirectX or Vulkan in that it works by directly interfacing
// with the GPU in order to create 3D environments or smooth-running 2D games.
// OSScreen gives you a simple framebuffer in which you can only change the
// colour of individual pixels, one-at-a-time. It does also provide a method
// of displaying text on screen in a monospace font. Although OSScreen does
// not provide nearly as much functionality as GX2, it is much easier to use
// and understand, and so will be used in this example project.
//
// OSScreen provides two separate framebuffers for the TV screen, and two
// buffers for the GamePad screen. At any point, one buffer for each device
// will be displayed on-screen, and the other is being edited using the
// OSScreen functions. We'll call these the "display buffer" and "work buffer"
// respectively. Once the program has finished editing the work buffer, the
// buffers can be "flipped", such that the changes that were made to the work
// buffer are now being displayed on screen, and the other buffer can now be
// edited. So flipping is like asking the console to "render" all the changes
// you made. This is necessary because changing the display buffer while the
// hardware is trying to read the buffer would result in some old and some new
// data being displayed on screen, or other kinds of flickering or visual
// artifacts.
//
// The Wii U has multiple memory regions, and one of these is the MEM1 chip.
// It provides 32MB of fast RAM, perfect for storage of things like graphics
// framebuffers that may struggle with performance issues if stored on slower
// memory regions. The catch with using MEM1 is that it can only be used by
// one program at a time, and so when ProcUI transitions our app to the
// background, we must clear anything we have in MEM1 so that it can be used
// by other apps. Once our program moves back into the foreground, we can once
// again use MEM1. This is only a minor inconvenience because our program
// cannot draw to the screen while it is in the background anyway, and this is
// all we use MEM1 for.
//
// RAM operations are generally slow, and so in order to improve performance,
// modern processors include their own small memory regions that store
// commonly accessed data, such that RAM does not need to be utilised as much.
// This process is called caching, and is done with both data and instructions.
// However, the Wii U's caching model is not coherent, which means that
// information that gets stored in the cache may not be automatically written
// into RAM. The display hardware in the Wii U can only operate with
// information stored in RAM, and so if parts of our framebuffer get stored
// in the cache, the display hardware will not see the changes, and will show
// outdated information. I have never personally seen this happen, but to be
// on the safe side, during program execution we regularly flush the PowerPC
// data cache, which forces all changes that were stored in the cache to be
// immediately written to RAM, so that the display hardware is always working
// with up-to-date data.
//
// I know, that's a lot to take in. When I started programming, I struggled to
// understand a lot of this as well. Keep at it, you'll get there. :)

// This provides the ability to interact with the PowerPC processor's caching
// mechanism.
#include <coreinit/cache.h>

// This provides structs that can be used to map memory regions on the console.
// We use this to obtain a handle to MEM1 memory.
#include <coreinit/memheap.h>

// This provides functions that allow the creation of frame heaps, allowing a
// program to allocate and free memory in a particular region. Again, this is
// used to interface with MEM1.
#include <coreinit/memfrmheap.h>

// This provides access to the OSScreen family of functions, for creating,
// drawing to and displaying graphics framebuffers.
#include <coreinit/screen.h>

// This is used to provide callbacks to the ProcUI system that allow the
// program to automatically remove and re-allocate things in MEM1 when
// transitioning into the background and foreground, respectively.
// This functionality is not ptovided by WUT's ProcUI wrapper (whb/proc.h)
// and so must be implemented manually.
#include <proc_ui/procui.h>

// Provides access to the strlen() standard library function.
#include <string.h>

// Allows us to send messages through WUT's logging interface.
#include <whb/log.h>

#include "graphics.h"

// ProcUI asks for a "priority" value when creating callbacks - 100 is usually
// a good value for this, you may wish to use different values if you have
// multiple callbacks for the same event.
#define CALLBACK_PRIORITY     100

// With OSScreen, the framebuffer must be stored in memory with an address
// starting at a 256-byte interval. Otherwise, the buffer may not display
// on-screen correctly. This is seen in some older, unmaintained Wii U
// software, in which the contents of the screen are further to the right of
// the screen than they should be (so the buffer is not centred correctly).
#define FRAMEBUFFER_ALIGNMENT 0x100

// Frame heaps provide the functionality to set a particular 'state' for future
// allocations, giving the ability to later free all allocations made while
// in that state in a very simple manner.
// 0x504F4E47 is the hexadecimal representation of "PONG". :P
#define PONG_MEMORY_STATE     0x504F4E47

// Text is drawn to the screen in a monospaced (fixed-with) font, and so
// individual characters are aligned to a grid. On the GamePad, this grid
// covers the whole screen. A total of 69 (stop laughing) characters can be
// displayed horizontally across the visible screen area, from x-coordinates
// -4 (very left side) to 65 (very right side).
#define TEXT_CONSOLE_WIDTH    69
#define TEXT_LEFT_BOUNDARY    -4

// When removing the contents of MEM1, it helps to clear the framebuffers.
// Writing black to the buffers is an easy way to do this, as the HTML code
// for black is all zeroes.
#define COLOUR_BLACK          0x00000000

// These variables are used internally by the below functions to determine
// whether or not the program has tried to initialize the library, and whether
// or not the framebuffers exist in memory. It is possible for the graphics
// library to be initialized without a framebuffer existing in memory. In this
// case, the program will keep running but nothing will be displayed on screen.
BOOL graphics_initialized = FALSE;
BOOL framebuffer_initialized = FALSE;

// Used to obtain a handle to MEM1.
MEMHeapHandle mem1_heap;

// Used to store the sizes of the TV and GamePad framebuffers, in bytes.
int tv_size = 0;
int gamepad_size = 0;

// Pointers to the framebuffers in memory.
void * tv_buffer;
void * gamepad_buffer;

// This function is used to clear both the framebuffers before MEM1 is released
// to the next program. It does so by erasing the work buffer, and then
// flipping so that both buffers are cleared.
void pong_graphics_clearbuffers()
{
    pong_graphics_clearscreen(COLOUR_BLACK);
    pong_graphics_render();

    pong_graphics_clearscreen(COLOUR_BLACK);
    pong_graphics_render();
}

// This function is responsible for setting up the MEM1 frame heap and
// allocating the memory for framebuffers. This function is called under two
// situations - when pong_graphics_init() is called from main(), and when
// ProcUI transitions our program into the foreground and continues running it
// (so this function is a callback for a ProcUI event). Returning an unsigned
// integer and taking a context pointer as an argument are required for ProcUI
// callbacks, although this functionality is not used in this example.
uint32_t pong_graphics_initbuffers(void * context)
{
    // If the framebuffers are already in memory, don't try to allocate them
    // again. That would be a disaster.
    if(framebuffer_initialized) return 0;

    WHBLogPrint("[graphics] Acquiring foreground...");

    // Obtain a handle to MEM1, and set up a frame heap within it. Set the
    // state of the heap for future allocations.
    WHBLogPrint("[graphics] Initializing MEM1 memory heap...");
    mem1_heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    MEMRecordStateForFrmHeap(mem1_heap, PONG_MEMORY_STATE);

    // Initialize the OSScreen library
    WHBLogPrint("[graphics] Initializing OSScreen...");
    OSScreenInit();

    // Get the amount of memory that must be allocated to fit the TV and
    // GamePad screen framebuffers.
    tv_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    gamepad_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    // Attempt to allocate the memory for the framebuffers, ensuring that the
    // allocated memory is aligned correctly. MEMAllocFromFrmHeapEx() functions
    // much like malloc() but instead of only operating on the default heap, it
    // can operate on any memory heap we have control of.
    WHBLogPrintf("[graphics] Allocating %d bytes for TV framebuffer, %d bytes "
                 "for GamePad framebuffer...", tv_size, gamepad_size);
    tv_buffer = MEMAllocFromFrmHeapEx(mem1_heap, tv_size,
                                      FRAMEBUFFER_ALIGNMENT);
    gamepad_buffer = MEMAllocFromFrmHeapEx(mem1_heap, gamepad_size,
                                           FRAMEBUFFER_ALIGNMENT);

    // The memory allocations should never fail, but if they do, ensure that
    // the heap is empty and then return with an error code.
    if(!tv_buffer || !gamepad_buffer) {
        WHBLogPrint("[graphics] Failed to allocate a framebuffer in memory. "
                    "You won't see anything while the program runs. :(");
        MEMFreeByStateToFrmHeap(mem1_heap, PONG_MEMORY_STATE);
        return -1;
    }

    WHBLogPrintf("[graphics] TV framebuffer is located at 0x%08X, GamePad "
                 "framebuffer is located at 0x%08X.", tv_buffer,
                 gamepad_buffer);

    // Tell OSScreen where our framebuffers are.
    OSScreenSetBufferEx(SCREEN_TV, tv_buffer);
    OSScreenSetBufferEx(SCREEN_DRC, gamepad_buffer);

    // Tell OSScreen that both screens should be enabled.
    OSScreenEnableEx(SCREEN_TV, TRUE);
    OSScreenEnableEx(SCREEN_DRC, TRUE);

    framebuffer_initialized = TRUE;

    // Remove any garbage data left in MEM1 from previous processes by clearing
    // the framebuffers.
    pong_graphics_clearbuffers();
    return 0;
}

// This function is responsible for freeing the contents of the MEM1 heap. Much
// like pong_graphics_initbuffers(), this function is called when ProcUI
// transitions our program to the background, and during
// pong_graphics_shutdown().
uint32_t pong_graphics_freebuffers(void * context)
{
    // You can't free a framebuffer from memory if it doesn't exist!
    if(!framebuffer_initialized) return 0;

    WHBLogPrint("[graphics] Releasing foreground...");

    // Clear both buffers before freeing them.
    pong_graphics_clearbuffers();

    // Instead of calling free() on both the individual buffers, frame heaps
    // allow freeing all objects allocated with a particular state value.
    WHBLogPrint("[graphics] De-allocating framebuffers...");
    MEMFreeByStateToFrmHeap(mem1_heap, PONG_MEMORY_STATE);

    framebuffer_initialized = FALSE;
    return 0;
}

// This function is called at the beginning of main() and is respobsible for
// both initializing the framebuffers and telling ProcUI about our callback
// functions for process transitions.
void pong_graphics_init()
{
    if(graphics_initialized) return;

    // Set up the framebuffers.
    pong_graphics_initbuffers(NULL);

    // Register the callbacks for ProcUI. The event PROCUI_CALLBACK_ACQUIRE is
    // called when the program moves into the foreground and now has access to
    // the display functions and MEM1, and so when this happens, the MEM1
    // heap and framebuffers should be set up.
    // Likewise, PROCUI_CALLBACK_RELEASE occurs when the program should remove
    // its MEM1 contents and stop using the display functions, as it is about
    // to be moved into the background.
    WHBLogPrint("[graphics] Registering foreground callbacks...");
    ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, pong_graphics_initbuffers,
                           NULL, CALLBACK_PRIORITY);
    ProcUIRegisterCallback(PROCUI_CALLBACK_RELEASE, pong_graphics_freebuffers,
                           NULL, CALLBACK_PRIORITY);

    graphics_initialized = TRUE;
}

// This function is called at the end of main() and is responsible for
// immediately freeing the framebuffers, and then removing the ProcUI
// callbacks.
void pong_graphics_shutdown()
{
    if(!graphics_initialized) return;

    pong_graphics_freebuffers(NULL);

    WHBLogPrint("[graphics] Clearing foreground callbacks...");
    ProcUIClearCallbacks();

    graphics_initialized = FALSE;
}

// This function is used to clear the work buffer with a specific colour.
void pong_graphics_clearscreen(uint32_t colour)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    OSScreenClearBufferEx(SCREEN_TV, colour);
    OSScreenClearBufferEx(SCREEN_DRC, colour);
}

// This function is used to flip the display and work buffers.
void pong_graphics_render()
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    // Flush the PowerPC data cache
    DCFlushRange(tv_buffer, tv_size);
    DCFlushRange(gamepad_buffer, gamepad_size);

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

// This function changes the colour of a single pixel in the work buffer.
void pong_graphics_draw_pixel(uint32_t x, uint32_t y, uint32_t colour)
{
    if(!graphics_initialized || !framebuffer_initialized) return;
    //TODO: Draw to TV screen
    OSScreenPutPixelEx(SCREEN_DRC, x, y, colour);
}

// This function calls pong_graphics_draw_pixel() in a loop to draw a rectangle
// on the work buffer. It draws a rectangle of the specified width and height
// centred at the specified coordinates (so x and y refer to the centre of the
// rectangle, not a corner).
void pong_graphics_draw_rectangle(int x, int y, int width, int height,
                                  uint32_t colour)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    // x is half-way between the left and right sides of the rectangle, and so
    // pixels must be drawn between (x - (width / 2)) and (x + (width / 2))
    // pixels. The same logic is applied for the y-axis.
    for(int i = (x - (width / 2)); i < (x + (width / 2)); i++) {
        for(int j = (y - (height / 2)); j < (y + (height / 2)); j++) {
            pong_graphics_draw_pixel(i, j, colour);
        }
    }
}

// This function uses OSScreen's built-in text drawing utilities to put a line
// of text on the screen starting at the specified x coordinate.
void pong_graphics_draw_text(const char * string, int32_t x, int32_t y)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    //TODO: Draw to TV screen
    OSScreenPutFontEx(SCREEN_DRC, x, y, string);
}

// Rather than taking an x and y coordinate to begin drawing a line of text,
// this function only takes a y coordinate and calculates the x coordinate
// such that the string is displayed in the centre of the screen.
void pong_graphics_draw_text_centre(const char * string, int32_t line)
{
    if(!graphics_initialized || !framebuffer_initialized) return;

    int x = ((TEXT_CONSOLE_WIDTH - strlen(string)) / 2) + TEXT_LEFT_BOUNDARY;
    pong_graphics_draw_text(string, x, line);
}
