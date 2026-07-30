#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480

SDL_Window *window;
SDL_Renderer *renderer;

SDL_Color BG_COLOR = {0x00, 0x00, 0x00, 0xFF};
SDL_Color FG_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_Log("Starting pong...");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("Pong", WIN_WIDTH, WIN_HEIGHT,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Failed to initialize SDL window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderLogicalPresentation(renderer, WIN_WIDTH, WIN_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {

  SDL_SetRenderDrawColor(renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b,
                         BG_COLOR.a);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, FG_COLOR.r, FG_COLOR.g, FG_COLOR.b,
                         FG_COLOR.a);
  SDL_RenderRect(renderer, NULL);
  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
