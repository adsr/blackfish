#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <argp.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <portmidi.h>
#include <lua5.2/lua.h>
#include <lua5.2/lauxlib.h>
#include <lua5.2/lualib.h>

#include "sequencer.h"
#include "seqview.h"
#include "util.h"

#define FPS 32

const char *argp_program_version = "blackfish 0.1";
const char *argp_program_bug_address = "<adam@atoi.cc>";

/**
 * Program entry point
 *
 * @param int argc
 * @param char** argv
 * @return exit status
 */
int main(int argc, char** argv) {
    // TODO Parse arguments
    // http://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.html#Argp-Example-3

    // Init PortMidi
    if (Pm_Initialize() != pmNoError) {
        fprintf(stderr, "Could not init PortMidi\n");
        exit(EXIT_FAILURE);
    }

    // Init Lua
    lua_State* lua_state = luaL_newstate();
    if (!lua_state) {
        fprintf(stderr, "Could not init Lua\n");
        exit(EXIT_FAILURE);
    }

    // Init SDL
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not init SDL\n");
        exit(EXIT_FAILURE);
    }
    atexit(SDL_Quit);

    // Create an 8-bpp 1000x600 resizable window
    SDL_Surface* screen = SDL_SetVideoMode(
        1000,
        600,
        8,
        SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ANYFORMAT
    );

    // Set window title
    SDL_WM_SetCaption(argp_program_version, 0);

    // Create sequencer
    sequencer_t* sequencer = sequencer_new();
    sequencer_start(sequencer);

    // Create sequencer view
    seqview_t* seqview = seqview_new(screen, sequencer);

    // Enter rendering loop
    Uint32 frame_start;
    Uint32 frame_time = 1000.0f / FPS;
    Uint32 delay_time;
    SDL_Event event;
    bool done = FALSE;
    while (1) {
        frame_start = SDL_GetTicks();

        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = TRUE;
                    break;
                case SDL_VIDEORESIZE:
                    screen = SDL_SetVideoMode(
                        event.resize.w,
                        event.resize.h,
                        8,
                        SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ANYFORMAT
                    );
                    SDL_Rect r = {0, 0, event.resize.w, event.resize.h};
                    seqview->viewport_rect = r;
                    frame_start = SDL_GetTicks();
                    break;
            }
        }
        if (done) {
            break;
        }

        // Render view
        seqview_render(seqview, screen);

        // Wait for next frame
        delay_time = frame_time - (SDL_GetTicks() - frame_start);
        if (delay_time > 0) {
            SDL_Delay(delay_time);
        }
    }

    Pm_Terminate();
    exit(EXIT_SUCCESS);
}
