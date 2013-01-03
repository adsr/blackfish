#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_getenv.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_ttf.h>
#include <portmidi.h>
#include <lua5.2/lua.h>
#include <lua5.2/lauxlib.h>
#include <lua5.2/lualib.h>

#include "sequencer.h"
#include "seqview.h"
#include "util.h"
#include "config.h"

// Init argp globals
const char *argp_program_version = "blackfish 0.1";
const char *argp_program_bug_address = "<adam@atoi.cc>";

// Declare function prototypes
int main_loop();
int main_init_screen(SDL_Surface** screen, int width, int height);

// Crap fix for MSVC
#ifdef _WIN32
#undef main
#endif

/**
 * Program entry point
 *
 * @param int argc
 * @param char** argv
 * @return int exit status
 */
int main(int argc, char* argv[]) {
    SDL_Surface* screen;
    sequencer_t* sequencer;
    seqview_t* seqview;
    lua_State* lua_state;

    // TODO Parse arguments
    // http://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.html#Argp-Example-3
    //config_put("seqview.width", 600);
    //config_put("seqview.height", 400);

    // Init PortMidi
    if (Pm_Initialize() != pmNoError) {
        fprintf(stderr, "Could not init PortMidi\n");
        exit(EXIT_FAILURE);
    }

    // Init Lua
    lua_state = luaL_newstate();
    if (!lua_state) {
        fprintf(stderr, "Could not init Lua\n");
        exit(EXIT_FAILURE);
    }

    // Ensure SDL win is centered
    putenv("SDL_VIDEO_CENTERED=1");

    // Init SDL
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not init SDL\n");
        exit(EXIT_FAILURE);
    }
    atexit(SDL_Quit);

    // Init SDL ttf
    if (TTF_Init() < 0) {
        fprintf(stderr, "Could not init SDL ttf\n");
        exit(EXIT_FAILURE);
    }
    atexit(TTF_Quit);

    // Create screen
    main_init_screen(
        &screen,
        config_get("seqview.width", 1000),
        config_get("seqview.height", 700)
    );

    // Set window title
    SDL_WM_SetCaption(argp_program_version, 0);

    // Enable unicode
    SDL_EnableUNICODE(1);

    // Create sequencer
    sequencer = sequencer_new();
    sequencer_start(sequencer);

    // Create sequencer view
    seqview = seqview_new(screen, sequencer);

    // Invoke main program loop
    main_loop(screen, seqview);

    // Cleanup
    Pm_Terminate();
    exit(EXIT_SUCCESS);
}

/**
 * Main rendering loop
 *
 * @param SDL_Surface* screen
 * @param seqview_t* seqview
 */
int main_loop(SDL_Surface* screen, seqview_t* seqview) {
    Uint32 frame_start;
    Uint32 frame_time;
    Sint32 delay_time;
    SDL_Event event;
    SDLKey key;
    SDL_keysym keysym;
    bool done = FALSE;
    SDL_Rect r;

    frame_time = 1000.0f / config_get("seqview.fps", 32);
    while (1) {
        frame_start = SDL_GetTicks();

        // Handle events
        while (SDL_PollEvent(&event) && !done) {
            switch (event.type) {
                case SDL_QUIT:
                    done = TRUE;
                    break;
                case SDL_VIDEORESIZE:
                    main_init_screen(&screen, event.resize.w, event.resize.h);
                    r.x = 0;
                    r.y = 0;
                    r.w = event.resize.w;
                    r.h = event.resize.h;
                    seqview->viewport_rect = r;
                    frame_start = SDL_GetTicks();
                    break;
                case SDL_KEYDOWN:
                    seqview_handle_input_key(seqview, event.key);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    seqview_handle_input_mouse(seqview, event.button);
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

    return 0;
}

/**
 * (Re)inits screen to size width x height
 *
 * @param SDL_Surface** screen
 * @param int width
 * @param int height
 */
int main_init_screen(SDL_Surface** screen, int width, int height) {
    *screen = SDL_SetVideoMode(
        width,
        height,
        config_get("seqview.surface_bpp", 16),
        SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ANYFORMAT
    );
    return 0;
}
