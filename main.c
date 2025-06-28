#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    float x, y, vx, vy;
} Ball;

typedef struct {
    bool active;
    SDL_Rect rect;
} Block;

// Estrutura para o sistema de dificuldade dinâmica
typedef struct {
    int frameCount;
    float recentSpeeds[100];  // Velocidades dos últimos 100 frames
    int speedIndex;
    float baseSpeed;
    float difficultyMultiplier;
    int timeBasedDifficulty;  // Contador para dificuldade baseada no tempo
    int stabilityCounter;     // Contador para estabilizar mudanças
    float verticalAcceleration; // Aceleração vertical para dificultar
} DynamicDifficulty;

// Estrutura para o sistema de pontuação inteligente
typedef struct {
    int recentScores[20];
    int scoreIndex;
    int scoreCount;
} SmartScoring;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BLOCKS_COLUMNS 10
#define BLOCKS_ROWS 10
#define BLOCKS_WIDTH 70
#define BLOCKS_HEIGHT 20
#define BALL_SIZE 10
#define PANEL_LEN 256
#define SPEED_HISTORY_SIZE 100

// Algoritmo da Mediana das Medianas
void insertionSort(int arr[], int left, int right) {
    int i, key, j;
    for (i = left + 1; i <= right; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

int selectKth(int arr[], int left, int right, int k) {
    if (k > 0 && k <= right - left + 1) {
        int n = right - left + 1;
        int i, median[(n + 4) / 5];
        for (i = 0; i < n / 5; i++) {
            insertionSort(arr, left + i * 5, left + i * 5 + 4);
            median[i] = arr[left + i * 5 + 2];
        }
        if (i * 5 < n) {
            insertionSort(arr, left + i * 5, left + i * 5 + (n % 5) - 1);
            median[i] = arr[left + i * 5 + (n % 5) / 2];
            i++;
        }
        int medOfMed = (i == 1) ? median[0] : selectKth(median, 0, i - 1, i / 2);

        // Particionamento
        int temp, j;
        for (j = left; j <= right; j++) {
            if (arr[j] == medOfMed) {
                temp = arr[j];
                arr[j] = arr[right];
                arr[right] = temp;
                break;
            }
        }
        int pivot = arr[right], pIndex = left;
        for (j = left; j < right; j++) {
            if (arr[j] <= pivot) {
                temp = arr[pIndex];
                arr[pIndex] = arr[j];
                arr[j] = temp;
                pIndex++;
            }
        }
        temp = arr[pIndex]; arr[pIndex] = arr[right]; arr[right] = temp;
        int pos = pIndex - left + 1;
        if (pos == k) return arr[pIndex];
        else if (pos > k) return selectKth(arr, left, pIndex - 1, k);
        else return selectKth(arr, pIndex + 1, right, k - pos);
    }
    return -1;
}

// Versão para float do algoritmo da mediana das medianas
void insertionSortFloat(float arr[], int left, int right) {
    int i, j;
    float key;
    for (i = left + 1; i <= right; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

float selectKthFloat(float arr[], int left, int right, int k) {
    if (k > 0 && k <= right - left + 1) {
        int n = right - left + 1;
        int i;
        float median[(n + 4) / 5];
        for (i = 0; i < n / 5; i++) {
            insertionSortFloat(arr, left + i * 5, left + i * 5 + 4);
            median[i] = arr[left + i * 5 + 2];
        }
        if (i * 5 < n) {
            insertionSortFloat(arr, left + i * 5, left + i * 5 + (n % 5) - 1);
            median[i] = arr[left + i * 5 + (n % 5) / 2];
            i++;
        }
        float medOfMed = (i == 1) ? median[0] : selectKthFloat(median, 0, i - 1, i / 2);

        // Particionamento
        float temp;
        int j;
        for (j = left; j <= right; j++) {
            if (arr[j] == medOfMed) {
                temp = arr[j];
                arr[j] = arr[right];
                arr[right] = temp;
                break;
            }
        }
        float pivot = arr[right];
        int pIndex = left;
        for (j = left; j < right; j++) {
            if (arr[j] <= pivot) {
                temp = arr[pIndex];
                arr[pIndex] = arr[j];
                arr[j] = temp;
                pIndex++;
            }
        }
        temp = arr[pIndex]; arr[pIndex] = arr[right]; arr[right] = temp;
        int pos = pIndex - left + 1;
        if (pos == k) return arr[pIndex];
        else if (pos > k) return selectKthFloat(arr, left, pIndex - 1, k);
        else return selectKthFloat(arr, pIndex + 1, right, k - pos);
    }
    return -1.0f;
}

// Função para calcular a mediana de um array de floats
float calculateMedian(float arr[], int size) {
    if (size <= 0) return 0.0f;
    
    float* tempArr = malloc(size * sizeof(float));
    if (!tempArr) return 0.0f;
    
    memcpy(tempArr, arr, size * sizeof(float));
    
    float median;
    if (size % 2 == 0) {
        // Se o tamanho é par, pega a média dos dois elementos do meio
        float med1 = selectKthFloat(tempArr, 0, size - 1, size / 2);
        float med2 = selectKthFloat(tempArr, 0, size - 1, size / 2 + 1);
        median = (med1 + med2) / 2.0f;
    } else {
        // Se o tamanho é ímpar, pega o elemento do meio
        median = selectKthFloat(tempArr, 0, size - 1, size / 2 + 1);
    }
    
    free(tempArr);
    return median;
}

// Inicializa o sistema de dificuldade dinâmica
void initDynamicDifficulty(DynamicDifficulty* diff) {
    diff->frameCount = 0;
    diff->speedIndex = 0;
    diff->baseSpeed = 4.0f;
    diff->difficultyMultiplier = 1.0f;
    diff->timeBasedDifficulty = 0;
    diff->stabilityCounter = 0;
    diff->verticalAcceleration = 0.0f;
    memset(diff->recentSpeeds, 0, sizeof(diff->recentSpeeds));
}

// Atualiza a dificuldade baseada na mediana das velocidades
void updateDynamicDifficulty(DynamicDifficulty* diff, Ball* ball) {
    // Calcula a velocidade atual da bola
    float currentSpeed = sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
    
    // Adiciona à lista de velocidades recentes
    diff->recentSpeeds[diff->speedIndex] = currentSpeed;
    diff->speedIndex = (diff->speedIndex + 1) % SPEED_HISTORY_SIZE;
    diff->frameCount++;
    
    // A cada 60 frames (mais lento para estabilidade), recalcula a dificuldade
    if (diff->frameCount % 60 == 0 && diff->frameCount >= 100) {
        int actualSize = (diff->frameCount < SPEED_HISTORY_SIZE) ? diff->frameCount : SPEED_HISTORY_SIZE;
        float medianSpeed = calculateMedian(diff->recentSpeeds, actualSize);
        
        // Debug: imprime informações para verificar se está funcionando
        printf("Frame: %d, Current Speed: %.2f, Median Speed: %.2f, Base Speed: %.2f\n", 
               diff->frameCount, currentSpeed, medianSpeed, diff->baseSpeed);
        
        // SOLUÇÃO 3: Dificuldade baseada no tempo de jogo (mais agressiva)
        diff->timeBasedDifficulty++;
        if (diff->timeBasedDifficulty >= 15) { // A cada 15 ajustes (15 segundos)
            diff->difficultyMultiplier = 1.08f; // Aumenta 8% a cada 15 segundos
            printf("Dificuldade baseada no TEMPO: +8%% (%.2f)\n", diff->difficultyMultiplier);
            diff->timeBasedDifficulty = 0; // Reset do contador
            
            // Aumenta a aceleração vertical para dificultar
            diff->verticalAcceleration += 0.1f;
            printf("Aceleração vertical aumentada para: %.2f\n", diff->verticalAcceleration);
        } else {
            // Sistema mais estável: só muda se a diferença for significativa
            diff->stabilityCounter++;
            
            // Só muda a dificuldade a cada 3 verificações (mais estável)
            if (diff->stabilityCounter >= 3) {
                if (medianSpeed > diff->baseSpeed * 1.3f) { // Limite mais alto
                    // Se a mediana está muito alta, diminui a velocidade
                    diff->difficultyMultiplier = 0.85f; // Diminuição mais agressiva
                    printf("Dificuldade DIMINUINDO: %.2f\n", diff->difficultyMultiplier);
                } else if (medianSpeed < diff->baseSpeed * 0.7f) { // Limite mais baixo
                    // Se a mediana está muito baixa, aumenta a velocidade
                    diff->difficultyMultiplier = 1.15f; // Aumento mais agressivo
                    printf("Dificuldade AUMENTANDO: %.2f\n", diff->difficultyMultiplier);
                } else {
                    // Mantém a dificuldade estável
                    diff->difficultyMultiplier = 1.0f;
                    printf("Dificuldade ESTÁVEL: %.2f\n", diff->difficultyMultiplier);
                }
                diff->stabilityCounter = 0; // Reset do contador de estabilidade
            }
        }
        
        // SOLUÇÃO 1: Aplica o multiplicador de dificuldade de forma acumulativa
        ball->vx *= diff->difficultyMultiplier;
        ball->vy *= diff->difficultyMultiplier;
        
        // SOLUÇÃO 2: Aumenta gradualmente a baseSpeed para dificuldade crescente
        if (diff->baseSpeed < 10.0f) { // Limite máximo aumentado
            diff->baseSpeed += 0.05f; // Aumenta mais rapidamente
            printf("Base Speed aumentada para: %.2f\n", diff->baseSpeed);
        }
        
        printf("Velocidade ajustada: vx=%.2f, vy=%.2f\n", ball->vx, ball->vy);
    }
    
    // Aplica aceleração vertical continuamente para dificultar
    if (ball->vy > 0) { // Se a bola está descendo
        ball->vy += diff->verticalAcceleration * 0.01f; // Acelera a descida
    }
}

// Sistema de pontuação inteligente baseado na mediana
int calculateSmartScore(int baseScore, int recentScores[], int scoreCount) {
    if (scoreCount < 5) return baseScore;
    
    // Calcula a mediana dos scores recentes
    int* tempScores = malloc(scoreCount * sizeof(int));
    if (!tempScores) return baseScore;
    
    memcpy(tempScores, recentScores, scoreCount * sizeof(int));
    int medianScore = selectKth(tempScores, 0, scoreCount - 1, scoreCount / 2 + 1);
    free(tempScores);
    
    // Se o jogador está consistentemente fazendo muitos pontos, aumenta o bônus
    if (medianScore > 50) {
        return baseScore + (baseScore / 2); // 50% de bônus
    } else if (medianScore > 30) {
        return baseScore + (baseScore / 4); // 25% de bônus
    }
    
    return baseScore;
}

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
                start_y + i * (BLOCKS_HEIGHT + 2),
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

void handleCollisions(Ball* ball, SDL_Rect* paddle, Block* blocks, int* score, SmartScoring* smartScore) {
    if (ball->x <= 0 || ball->x + BALL_SIZE >= SCREEN_WIDTH)
        ball->vx *= -1;

    SDL_Rect rectBall = { ball->x, ball->y, BALL_SIZE, BALL_SIZE };
    if (ball->y <= 0 || SDL_HasIntersection(paddle, &rectBall))
        ball->vy *= -1;

    for (int i = 0; i < BLOCKS_ROWS * BLOCKS_COLUMNS; i++) {
        if (blocks[i].active && SDL_HasIntersection(&rectBall, &blocks[i].rect)) {
            blocks[i].active = false;
            ball->vy *= -1;
            
            // Calcula score inteligente baseado na mediana
            int baseScore = 10;
            int smartPoints = calculateSmartScore(baseScore, smartScore->recentScores, smartScore->scoreCount);
            *score += smartPoints;
            
            // Adiciona o score atual ao histórico
            smartScore->recentScores[smartScore->scoreIndex] = smartPoints;
            smartScore->scoreIndex = (smartScore->scoreIndex + 1) % 20;
            if (smartScore->scoreCount < 20) smartScore->scoreCount++;
        }
    }
}

void drawPanel(SDL_Renderer* renderer, TTF_Font* font, int score, DynamicDifficulty* diff) {
    char panel[PANEL_LEN] = { 0 };
    sprintf(panel, "Score: %d | Difficulty: %.2f", score, diff->difficultyMultiplier);

    SDL_Color fontColor = (SDL_Color){ 255, 255, 0, 255 };
    SDL_Surface* panelSurface = TTF_RenderText_Blended(font, panel, fontColor);
    SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(renderer, panelSurface);
    SDL_Rect panelRect = { 25, 10, panelSurface->w, panelSurface->h };
    SDL_RenderCopy(renderer, panelTexture, NULL, &panelRect);
    SDL_FreeSurface(panelSurface);
    SDL_DestroyTexture(panelTexture);
}

void renderGame(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect* paddle, Block* blocks, Ball* ball, int score, DynamicDifficulty* diff) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawPanel(renderer, font, score, diff);

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
    
    // Inicializa o sistema de dificuldade dinâmica (FORA do loop principal)
    DynamicDifficulty diff;
    initDynamicDifficulty(&diff);
    
    // Inicializa o sistema de pontuação inteligente (FORA do loop principal)
    SmartScoring smartScore = { {0}, 0, 0 };
    
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

            // Atualiza a dificuldade dinâmica
            updateDynamicDifficulty(&diff, &ball);
            
            // Processa colisões com pontuação inteligente
            handleCollisions(&ball, &paddle, blocks, &score, &smartScore);

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

            renderGame(renderer, font, &paddle, blocks, &ball, score, &diff);

            SDL_Delay(20);
        }
    }

    return 0;
}