#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>
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

const float MIN_BOX_SIZE_MULTIPLIER = 1.0f;
float boxSizeMultiplier = MIN_BOX_SIZE_MULTIPLIER;
const float MAX_BOX_SIZE_MULTIPLIER = 2.0f;

float BOX_SIZE = 1.0f;
int PIXEL_BOX_SIZE = static_cast<int>(BOX_SIZE * MET2PIX * boxSizeMultiplier);

SDL_Rect cursorRect = {WIDTH / 2, HEIGHT / 2, PIXEL_BOX_SIZE, PIXEL_BOX_SIZE};

void printHelp(void)
{
    printf("box2d-test-01: example program for Box2D Engine (v2.4.2) + Dear ImGui (v1.91.1).\n\n");
    printf("\nby gen04177 - v0.2 \n");
}

void createDynamicBoxAtPosition(float x, float y) {
    b2BodyDef boxBodyDef;
    boxBodyDef.type = b2_dynamicBody;
    boxBodyDef.position.Set(x / MET2PIX, y / MET2PIX);
    b2Body* boxBody = world->CreateBody(&boxBodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(BOX_SIZE / 2 * boxSizeMultiplier, BOX_SIZE / 2 * boxSizeMultiplier);

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
    staticBox.SetAsBox(BOX_SIZE / 2 * boxSizeMultiplier, BOX_SIZE / 2 * boxSizeMultiplier);

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

void removeOffScreenBodies() {
    for (b2Body* body = world->GetBodyList(); body; ) {
        b2Body* nextBody = body->GetNext();

        b2Vec2 position = body->GetPosition();
        if ((position.x * MET2PIX < 0 || position.x * MET2PIX > WIDTH ||
             position.y * MET2PIX < 0 || position.y * MET2PIX > HEIGHT) &&
            body->GetType() == b2_dynamicBody) {
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    world = new b2World(b2Vec2(0.0f, 9.81f));

    bool running = true;
    SDL_Event event;
    int frameCounter = 0;

    srand(static_cast<unsigned>(time(0)));

    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
                    removeAllStaticBarriers();
                }
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
                    if (boxSizeMultiplier > MIN_BOX_SIZE_MULTIPLIER) {
                        boxSizeMultiplier -= 1.0f;
                        PIXEL_BOX_SIZE = static_cast<int>(BOX_SIZE * MET2PIX * boxSizeMultiplier);

                        for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
                            if (body->GetType() == b2_dynamicBody || body->GetType() == b2_staticBody) {
                                b2PolygonShape* shape = (b2PolygonShape*)body->GetFixtureList()->GetShape();
                                shape->SetAsBox(BOX_SIZE / 2 * boxSizeMultiplier, BOX_SIZE / 2 * boxSizeMultiplier);

                                b2Vec2 position = body->GetPosition();
                                position.x = roundf(position.x * MET2PIX) / MET2PIX;
                                position.y = roundf(position.y * MET2PIX) / MET2PIX;
                                body->SetTransform(position, body->GetAngle());
                            }
                        }
                    }
                }
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
                    if (boxSizeMultiplier < MAX_BOX_SIZE_MULTIPLIER) {
                        boxSizeMultiplier += 1.0f;
                        PIXEL_BOX_SIZE = static_cast<int>(BOX_SIZE * MET2PIX * boxSizeMultiplier);

                        for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
                            if (body->GetType() == b2_dynamicBody || body->GetType() == b2_staticBody) {
                                b2PolygonShape* shape = (b2PolygonShape*)body->GetFixtureList()->GetShape();
                                shape->SetAsBox(BOX_SIZE / 2 * boxSizeMultiplier, BOX_SIZE / 2 * boxSizeMultiplier);

                                b2Vec2 position = body->GetPosition();
                                position.x = roundf(position.x * MET2PIX) / MET2PIX;
                                position.y = roundf(position.y * MET2PIX) / MET2PIX;
                                body->SetTransform(position, body->GetAngle());
                            }
                        }
                    }
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

        if (cursorRect.x < 0) {
            cursorRect.x = 0;
        }
        if (cursorRect.x > WIDTH) {
            cursorRect.x = WIDTH;
        }
        if (cursorRect.y < 0) {
            cursorRect.y = 0;
        }
        if (cursorRect.y > HEIGHT) {
            cursorRect.y = HEIGHT;
        }

        removeOffScreenBodies();

        world->Step(1.0f / 60.0f, 6, 2);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Desenha as caixas
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

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin("Box Size Controller", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImGui::Text("Adjust the size of the boxes:");
        if (ImGui::Button("Decrease") && boxSizeMultiplier > MIN_BOX_SIZE_MULTIPLIER) {
            boxSizeMultiplier -= 1.0f;
            PIXEL_BOX_SIZE = static_cast<int>(BOX_SIZE * MET2PIX * boxSizeMultiplier);

            for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
                if (body->GetType() == b2_dynamicBody || body->GetType() == b2_staticBody) {
                    b2PolygonShape* shape = (b2PolygonShape*)body->GetFixtureList()->GetShape();
                    shape->SetAsBox(BOX_SIZE / 2 * boxSizeMultiplier, BOX_SIZE / 2 * boxSizeMultiplier);

                    b2Vec2 position = body->GetPosition();
                    position.x = roundf(position.x * MET2PIX) / MET2PIX;
                    position.y = roundf(position.y * MET2PIX) / MET2PIX;
                    body->SetTransform(position, body->GetAngle());
                }
            }
        }
        ImGui::SameLine();
        ImGui::Text("(L1)");

        if (ImGui::Button("Increase") && boxSizeMultiplier < MAX_BOX_SIZE_MULTIPLIER) {
            boxSizeMultiplier += 1.0f;
            PIXEL_BOX_SIZE = static_cast<int>(BOX_SIZE * MET2PIX * boxSizeMultiplier);

            for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
                if (body->GetType() == b2_dynamicBody || body->GetType() == b2_staticBody) {
                    b2PolygonShape* shape = (b2PolygonShape*)body->GetFixtureList()->GetShape();
                    shape->SetAsBox(BOX_SIZE / 2 * boxSizeMultiplier, BOX_SIZE / 2 * boxSizeMultiplier);

                    b2Vec2 position = body->GetPosition();
                    position.x = roundf(position.x * MET2PIX) / MET2PIX;
                    position.y = roundf(position.y * MET2PIX) / MET2PIX;
                    body->SetTransform(position, body->GetAngle());
                }
            }
        }
        ImGui::SameLine();
        ImGui::Text("(R1)");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (controller) SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    delete world;
    SDL_Quit();

    return EXIT_SUCCESS;
}