#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    float x, y, vx, vy;
} Ball;

typedef struct {
    bool active;
    SDL_Rect rect;
} Block;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BLOCKS_COLUMNS 10
#define BLOCKS_ROWS 5
#define BLOCKS_WIDTH 70
#define BLOCKS_HEIGHT 20
#define BALL_SIZE 10
#define PANEL_LEN 256

void initBall(Ball* ball) {
    ball->x = SCREEN_WIDTH / 2;
    ball->y = SCREEN_HEIGHT / 2;
    ball->vx = 4.0f;
    ball->vy = -4.0f;
}

void initBlocks(Block* blocks) {
    int start_x = 25;
    int start_y = 40;

    for (int i = 0; i < BLOCKS_ROWS; i++) {
        for (int j = 0; j < BLOCKS_COLUMNS; j++) {
            blocks[i * BLOCKS_COLUMNS + j].rect = (SDL_Rect){
                start_x + j * (BLOCKS_WIDTH + 5),
                start_y + i * (BLOCKS_HEIGHT + 5),
                BLOCKS_WIDTH,
                BLOCKS_HEIGHT
            };

            blocks[i * BLOCKS_COLUMNS + j].active = true;
        }
    }
}

bool allBlocksDestroyed(Block* blocks) {
    for (int i = 0; i < BLOCKS_ROWS * BLOCKS_COLUMNS; i++) {
        if (blocks[i].active)
            return false;
    }

    return true;
}

void handleCollisions(Ball* ball, SDL_Rect* paddle, Block* blocks, int* score) {
    if (ball->x <= 0 || ball->x + BALL_SIZE >= SCREEN_WIDTH)
        ball->vx *= -1;

    SDL_Rect rectBall = { ball->x, ball->y, BALL_SIZE, BALL_SIZE };
    if (ball->y <= 0 || SDL_HasIntersection(paddle, &rectBall))
        ball->vy *= -1;

    for (int i = 0; i < BLOCKS_ROWS * BLOCKS_COLUMNS; i++) {
        if (blocks[i].active && SDL_HasIntersection(&rectBall, &blocks[i].rect)) {
            blocks[i].active = false;
            ball->vy *= -1;
            *score += 10;
        }
    }
}

void drawPanel(SDL_Renderer* renderer, TTF_Font* font, int score) {
    char panel[PANEL_LEN] = { 0 };
    sprintf(panel, "Score: %d", score);

    SDL_Color fontColor = (SDL_Color){ 255, 255, 0, 255 };
    SDL_Surface* panelSurface = TTF_RenderText_Blended(font, panel, fontColor);
    SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(renderer, panelSurface);
    SDL_Rect panelRect = { 25, 10, panelSurface->w, panelSurface->h };
    SDL_RenderCopy(renderer, panelTexture, NULL, &panelRect);
    SDL_FreeSurface(panelSurface);
    SDL_DestroyTexture(panelTexture);
}

void renderGame(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect* paddle, Block* blocks, Ball* ball, int score) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawPanel(renderer, font, score);

    SDL_SetRenderDrawColor(renderer, 51, 204, 204, 255);
    SDL_RenderFillRect(renderer, paddle);

    SDL_SetRenderDrawColor(renderer, 204, 51, 255, 255);
    for (int i = 0; i < BLOCKS_ROWS * BLOCKS_COLUMNS; i++) {
        if (blocks[i].active) {
            SDL_RenderFillRect(renderer, &blocks[i].rect);
        }
    }

    SDL_Rect rectBall = { ball->x, ball->y, BALL_SIZE, BALL_SIZE };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rectBall);

    SDL_RenderPresent(renderer);
}

void waitForKey() {
    SDL_Event e;
    while (true) {
        if (SDL_WaitEvent(&e)) {
            if (e.type == SDL_KEYDOWN)
                break;
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Arkanoid C", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("fonts/Sony_Sketch_Bold_205.ttf", 20);
    if (!font) {
        printf("Error loading font!\n");
        exit(EXIT_FAILURE);
    }
    TTF_Font* bigFont = TTF_OpenFont("fonts/Sony_Sketch_Bold_205.ttf", 60);

    int score = 0;
    bool running = true;
    SDL_Event e;
    while (running) {
        SDL_Rect paddle = {
            (SCREEN_WIDTH / 2) - (PADDLE_WIDTH / 2),
            SCREEN_HEIGHT - 40,
            PADDLE_WIDTH,
            PADDLE_HEIGHT
        };

        Ball ball;
        initBall(&ball);

        Block blocks[BLOCKS_ROWS * BLOCKS_COLUMNS];
        initBlocks(blocks);

        while (running) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }

            const Uint8* keystates = SDL_GetKeyboardState(NULL);
            if (keystates[SDL_SCANCODE_LEFT] && paddle.x > 0) {
                paddle.x -= 8;
            }

            if (keystates[SDL_SCANCODE_RIGHT] && (paddle.x + PADDLE_WIDTH) < SCREEN_WIDTH) {
                paddle.x += 8;
            }

            ball.x += ball.vx;
            ball.y += ball.vy;

            handleCollisions(&ball, &paddle, blocks, &score);

            if (ball.y > SCREEN_HEIGHT) {
                char* gameover = "GAME OVER";
                SDL_Color fontColor = (SDL_Color){ 255, 0, 0, 255 };
                SDL_Surface* gameoverSurface = TTF_RenderText_Blended(bigFont, gameover, fontColor);
                SDL_Texture* gameoverTexture = SDL_CreateTextureFromSurface(renderer, gameoverSurface);
                SDL_Rect panelRect = {
                    (SCREEN_WIDTH / 2) - (gameoverSurface->w / 2),
                    (SCREEN_HEIGHT / 2) - (gameoverSurface->h / 2),
                    gameoverSurface->w,
                    gameoverSurface->h };
                SDL_RenderCopy(renderer, gameoverTexture, NULL, &panelRect);
                SDL_FreeSurface(gameoverSurface);
                SDL_DestroyTexture(gameoverTexture);

                SDL_RenderPresent(renderer);
                waitForKey();
                exit(EXIT_SUCCESS);
            }

            if (allBlocksDestroyed(blocks)) {
                char* youwin = "YOU WIN!!!";
                SDL_Color fontColor = (SDL_Color){ 0, 255, 0, 255 };
                SDL_Surface* youwinSurface = TTF_RenderText_Blended(bigFont, youwin, fontColor);
                SDL_Texture* youwinTexture = SDL_CreateTextureFromSurface(renderer, youwinSurface);
                SDL_Rect panelRect = {
                    (SCREEN_WIDTH / 2) - (youwinSurface->w / 2),
                    (SCREEN_HEIGHT / 2) - (youwinSurface->h / 2),
                    youwinSurface->w,
                    youwinSurface->h };
                SDL_RenderCopy(renderer, youwinTexture, NULL, &panelRect);
                SDL_FreeSurface(youwinSurface);
                SDL_DestroyTexture(youwinTexture);

                SDL_RenderPresent(renderer);
                waitForKey();
                exit(EXIT_SUCCESS);
            }

            renderGame(renderer, font, &paddle, blocks, &ball, score);

            SDL_Delay(20);
        }
    }

    return 0;
}