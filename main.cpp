#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

b2World* world;

const int MET2PIX = 80;
const int WIDTH = 1920;
const int HEIGHT = 1080;
const float RAD2DEG = 180.0f / M_PI;

const int SCALED_WIDTH = WIDTH / MET2PIX;
const int SCALED_HEIGHT = HEIGHT / MET2PIX;

const float BOX_SIZE = 0.5f;
const int PIXEL_BOX_SIZE = static_cast<int>(BOX_SIZE * MET2PIX);

SDL_Rect cursorRect = {WIDTH / 2, HEIGHT / 2, PIXEL_BOX_SIZE, PIXEL_BOX_SIZE};

void printHelp(void)
{
    printf("box2d-test-01: example program for Box2D Engine (v2.4.2).\n\n");
    printf("\nby gen04177 - v0.1 \n");
}

void createDynamicBoxAtPosition(float x, float y) {
    b2BodyDef boxBodyDef;
    boxBodyDef.type = b2_dynamicBody;
    boxBodyDef.position.Set(x / MET2PIX, y / MET2PIX);
    b2Body* boxBody = world->CreateBody(&boxBodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(BOX_SIZE / 2, BOX_SIZE / 2);

    b2FixtureDef boxFixtureDef;
    boxFixtureDef.shape = &dynamicBox;
    boxFixtureDef.density = 1.0f;
    boxFixtureDef.friction = 0.3f;
    boxFixtureDef.restitution = 0.1f;
    boxBody->CreateFixture(&boxFixtureDef);
}

void createStaticBarrierAtPosition(float x, float y) {
    b2BodyDef barrierBodyDef;
    barrierBodyDef.type = b2_staticBody;
    barrierBodyDef.position.Set(x / MET2PIX, y / MET2PIX);
    b2Body* barrierBody = world->CreateBody(&barrierBodyDef);

    b2PolygonShape staticBox;
    staticBox.SetAsBox(BOX_SIZE / 2, BOX_SIZE / 2);

    b2FixtureDef barrierFixtureDef;
    barrierFixtureDef.shape = &staticBox;
    barrierFixtureDef.density = 1.0f;
    barrierFixtureDef.friction = 0.3f;
    barrierFixtureDef.restitution = 0.1f;
    barrierBody->CreateFixture(&barrierFixtureDef);
}

void removeAllStaticBarriers() {
    for (b2Body* body = world->GetBodyList(); body; ) {
        b2Body* nextBody = body->GetNext();
        if (body->GetType() == b2_staticBody) {
            world->DestroyBody(body);
        }
        body = nextBody;
    }
}

int main() {
    printHelp();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("b2d-test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_GameController* controller = nullptr;
    if (SDL_NumJoysticks() > 0) {
        controller = SDL_GameControllerOpen(0);
    }

    world = new b2World(b2Vec2(0.0f, 9.81f));

    bool running = true;
    SDL_Event event;
    int frameCounter = 0;

    srand(static_cast<unsigned>(time(0)));

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
                    removeAllStaticBarriers();
                }
            }
        }

        frameCounter++;

        if (frameCounter % 2 == 0) { 
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
                createDynamicBoxAtPosition(cursorRect.x, cursorRect.y);
            }
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) {
                createStaticBarrierAtPosition(cursorRect.x, cursorRect.y);
            }
        }

        int xDir = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        int yDir = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        cursorRect.x += xDir / 32768.0 * 10;
        cursorRect.y += yDir / 32768.0 * 10;

        world->Step(1.0f / 60.0f, 6, 2);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
            b2Vec2 pos = body->GetPosition();
            SDL_Rect boxRect = {static_cast<int>(pos.x * MET2PIX - PIXEL_BOX_SIZE / 2), static_cast<int>(pos.y * MET2PIX - PIXEL_BOX_SIZE / 2), PIXEL_BOX_SIZE, PIXEL_BOX_SIZE};

            if (body->GetType() == b2_dynamicBody) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else if (body->GetType() == b2_staticBody) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }

            SDL_RenderFillRect(renderer, &boxRect);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderDrawLine(renderer, cursorRect.x - 10, cursorRect.y, cursorRect.x + 10, cursorRect.y);
        SDL_RenderDrawLine(renderer, cursorRect.x, cursorRect.y - 10, cursorRect.x, cursorRect.y + 10);

        SDL_RenderPresent(renderer);
    }

    if (controller) SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    delete world;
    SDL_Quit();

    return EXIT_SUCCESS;
}