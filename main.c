#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <argp.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <portmidi.h>
#include <lua5.2/lua.h>
#include <lua5.2/lauxlib.h>
#include <lua5.2/lualib.h>

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

    // Create a 16 bpp 640x480 resizable window
    SDL_Surface* screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ANYFORMAT);

    // TODO Spawn sequencer loop in separate thread

    // Enter rendering loop
    Uint32 frame_start = SDL_GetTicks();
    Uint32 frame_time = 1000.0f / FPS;
    Uint32 delay_time;
    SDL_Event event;
    bool done = FALSE;
    while (1) {

        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = TRUE;
                    break;
                case SDL_VIDEORESIZE:
                    screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ANYFORMAT);
                    break;
            }
        }
        if (done) {
            break;
        }

        // TODO Render screen

        // Flip buffer
        SDL_Flip(screen);

        // Wait for next frame
        delay_time = frame_time - (SDL_GetTicks() - frame_start);
        if (delay_time > 0) {
            SDL_Delay(delay_time);
        }
        frame_start = SDL_GetTicks();
    }

    Pm_Terminate();
    exit(EXIT_SUCCESS);

}
