#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BLOCKS_COLLUMNS 10
#define BLOCKS_ROWS 5
#define BLOCKS_WIDTH 70
#define BLOCKS_HEIGHT 20

typedef struct {
    float x, y, vx, vy;
} Ball;   

typedef struct{
    bool active;
    SDL_Rect rect;
}Block;

void initBlocks(Block *blocks){
    int start_x = 20;
    int start_y = 20;

    for(int i = 0; i < BLOCKS_ROWS; i++){
        for(int j = 0; j < BLOCKS_COLLUMNS; j++){
            blocks[i * BLOCKS_COLLUMNS + j].rect = (SDL_Rect){
                start_x + j * (BLOCKS_WIDTH + 5),
                start_y + i * (BLOCKS_HEIGHT + 5),
                BLOCKS_WIDTH,
                BLOCKS_HEIGHT
            };
            blocks[i * BLOCKS_ROWS + j].active = true;
            
        }
    }
}

void renderGame(SDL_Renderer* renderer, TTF_Font *font, SDL_Rect *paddle, Block *blocks){
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw paddle
    SDL_SetRenderDrawColor(renderer, 51, 204, 204, 255);
    SDL_RenderFillRect(renderer, paddle);

    SDL_SetRenderDrawColor(renderer, 204, 51, 255, 255);
    for(int i=0; i < BLOCKS_ROWS * BLOCKS_COLLUMNS; i++){
            SDL_RenderFillRect(renderer, &blocks[i].rect);
        
    }


    SDL_RenderPresent(renderer);
}


int main(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Arkanoid ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_HEIGHT, SCREEN_WIDTH,  0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("fonts/Sony_Sketch_Bold_205.ttf", 20);

    if(!font ){
        printf("Error loanding font!\n");
        exit(EXIT_FAILURE);
    }

    bool running = true;
    SDL_Event e;
    int counter = 1000;
    while(running){
        SDL_Rect paddle = {
            (SCREEN_WIDTH / 2) - (PADDLE_WIDTH / 2), 
            SCREEN_HEIGHT - 40,
            PADDLE_WIDTH, 
            PADDLE_HEIGHT};

            Block blocks[BLOCKS_ROWS * BLOCKS_COLLUMNS];
            initBlocks(blocks);
    

        while(running){
            while(SDL_PollEvent(&e)){
                if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
                    running = false;
                }
            }

            const Uint8* keystates = SDL_GetKeyboardState(NULL);
            if(keystates[SDL_SCANCODE_LEFT] && paddle.x > 0)
                paddle.x -= 8;

            if(keystates[SDL_SCANCODE_RIGHT] && (paddle.x + PADDLE_WIDTH) < SCREEN_WIDTH)
                paddle.x += 8;
                
                

                renderGame(renderer, font, &paddle, blocks);
            SDL_Delay(20);
        }

    }

    return 0;

}
