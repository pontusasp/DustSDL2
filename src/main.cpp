#include "SDL.h"
#include <stdio.h>
#include <tgmath.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define NUM_OF_PARTICLES 100000
#define DISTANCE_UNIT 1

#define COLOR_MAX 200
#define COLOR_MIN 10

#define GRAVITY 9.82

SDL_Point mouse;
SDL_Rect pp[NUM_OF_PARTICLES];

SDL_Rect screenRect;

struct Particle {
    double x;
    double y;
    double vx;
    double vy;
} particles[NUM_OF_PARTICLES];

bool force = 0;
bool gravity = 0;
Uint64 lastStartedUpdate = 0;

double G = (double)(6.673f * pow(10, -11));
double mouseMass = pow(8.5f, 17);

int pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        int mouseState;
        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            SDL_GetMouseState(&mouse.x, &mouse.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouseState = SDL_GetMouseState(NULL, NULL);
            if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                force = !force;
                SDL_Log("Left mouse button clicked at (%d, %d)", mouse.x, mouse.y);
            }
            else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                gravity = !gravity;
            }
            break;
        case SDL_QUIT:
            return 1;
        }
    }
    return 0;
}

double timePassed = 0;

void updatePhysics(double deltaTime)
{
    timePassed = int(timePassed + deltaTime) % 10 + (timePassed + deltaTime) - int(timePassed + deltaTime);
    for (int i = 0; i < NUM_OF_PARTICLES; i++)
    {
        double* x = &particles[i].x;
        double* y = &particles[i].y;
        double* vx = &particles[i].vx;
        double* vy = &particles[i].vy;

        double deg = (double)atan2(mouse.y - particles[i].y, mouse.x - particles[i].x);
        double distance = (double)(sqrt(pow(mouse.x - particles[i].x, 2) + pow(mouse.y - particles[i].y, 2)));
        double f = (G * mouseMass) / (distance * DISTANCE_UNIT);

        if (force)
        {
            particles[i].vx += (double)(cos(deg) * f * deltaTime);
            particles[i].vy += (double)(sin(deg) * f * deltaTime);
        }

        if (gravity)
        {
            particles[i].vy += 50 * GRAVITY / DISTANCE_UNIT * deltaTime;
        }

        particles[i].x += particles[i].vx * deltaTime;
        particles[i].y += particles[i].vy * deltaTime;
        if (particles[i].x < 0) {
            x = 0;
            particles[i].vx *= -0.1f;
        }
        else if (particles[i].x > SCREEN_WIDTH) {
            particles[i].x = SCREEN_WIDTH;
            particles[i].vx *= -0.1f;
        }
        if (particles[i].y < 0) {
            particles[i].y = 0;
            particles[i].vy *= -0.1f;
        }
        else if (particles[i].y > SCREEN_HEIGHT) {
            particles[i].y = SCREEN_HEIGHT;
            particles[i].vy *= -0.1f;
        }
        pp[i].x = (int)particles[i].x;
        pp[i].y = (int)particles[i].y;
    }
}

void drawToScreen(SDL_Renderer* renderer, double deltaTime)
{

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MUL);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 20);
    //SDL_RenderClear(renderer);
    SDL_RenderFillRect(renderer, &screenRect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    SDL_SetRenderDrawColor(renderer,
        COLOR_MIN,
        gravity? COLOR_MAX * 0.2 : COLOR_MIN * 0.6,
        gravity? COLOR_MIN * 0.3 : COLOR_MAX * 0.4,
        255
    );

    //SDL_RenderDrawPoints(renderer, pp, NUM_OF_PARTICLES);
    SDL_RenderFillRects(renderer, pp, NUM_OF_PARTICLES);

    SDL_RenderPresent(renderer);
}

int gameloop(SDL_Renderer* renderer)
{
    Uint64 time = lastStartedUpdate;
    lastStartedUpdate = SDL_GetPerformanceCounter();
    double deltaTime = (double)(lastStartedUpdate - time) / SDL_GetPerformanceFrequency();

    if (pollEvents())
        return 1;

    updatePhysics(deltaTime);
    drawToScreen(renderer, deltaTime);

    return 0;
}

int main(int argc, char* argv[]) {

    SDL_Window* window;                    // Declare a pointer
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "DustSDL2",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        SCREEN_WIDTH,                      // width, in pixels
        SCREEN_HEIGHT,                     // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    for (int i = 0; i < NUM_OF_PARTICLES; i++)
    {
        particles[i].x = pp[i].x = (i % SCREEN_WIDTH);
        particles[i].y = pp[i].y = SCREEN_HEIGHT - (i / SCREEN_WIDTH);

        pp[i].w = pp[i].h = 2;
    }

    while (!gameloop(renderer));

    // Close and destroy the window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return 0;
}