#include <stdio.h>
#include <portmidi.h>
#include <SDL/SDL_thread.h>
#include <lua5.2/lua.h>
#include <lua5.2/lauxlib.h>
#include <lua5.2/lualib.h>

int thread_func(void *unused);

int global_data = 0;
lua_State* lua_state;

int main(void) {

    SDL_Thread *thread;
    int i;

    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);
    lua_settop(lua_state, 0);

    Pm_Initialize();

    thread = SDL_CreateThread(thread_func, NULL);
    if ( thread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        return;
    }

    for ( i=0; i<5; ++i ) {
        printf("Changing value to %d\n", i);
        global_data = i;
        SDL_Delay(1000);
    }

    printf("Signaling thread to quit\n");
    global_data = -1;
    SDL_WaitThread(thread, NULL);
    return 0;
}

int thread_func(void *unused) {
    int last_value = 0;
    int n = Pm_CountDevices();
    luaL_dostring(lua_state, "1+1");
    while ( global_data != -1 ) {
        if ( global_data != last_value ) {
            printf("Data value changed to %d, midi devices=%d\n", global_data, n);
            last_value = global_data;
        }
        SDL_Delay(100);
    }
    printf("Thread quitting\n");
    return(0);
}
