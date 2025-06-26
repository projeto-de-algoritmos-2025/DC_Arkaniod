#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define SCREN_WIDTH 800
#define SCREEN_HEIGHT 600


int main(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Arkanoid ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_HEIGHT, SCREN_WIDTH,  0);
    bool running = true;
    int counter = 1000;
    while(running){
        if(counter == 0) break;

        SDL_Delay(20);
    }

    return 0;

}
