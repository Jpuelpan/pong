#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480

SDL_Window *window;
SDL_Renderer *renderer;

SDL_FColor BG_COLOR = {0x00, 0x00, 0x00, 0xFF};
SDL_FColor FG_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};
SDL_FColor BALL_COLOR = {0x00, 0x00, 0xFF, 0xFF};

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

// Circle drawing using the Midpoint Circle Algorithm
// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
// https://stackoverflow.com/a/48291620
void RenderCircleMidpoint(float x0, float y0, float r) {
  float x = r - 1.0;
  float y = 0.0;
  float dx = 1.0;
  float dy = 1.0;
  int err = dx - ((int)r << 1);

  while (x >= y) {
    SDL_RenderLine(renderer, x + x0, y + y0, -x + x0, y + y0);
    SDL_RenderLine(renderer, y + x0, x + y0, -y + x0, x + y0);
    SDL_RenderLine(renderer, -x + x0, -y + y0, x + x0, -y + y0);
    SDL_RenderLine(renderer, -y + x0, -x + y0, y + x0, -x + y0);

    if (err <= 0) {
      y++;
      err += dy;
      dy += 2;
    }

    if (err > 0) {
      x--;
      dx += 2;
      err += dx - ((int)r << 1);
    }
  }
}

// Circle drawing with triangles?
// TODO: Learn trigonometry
void RenderCircle(float x, float y, float r) {
  int len = 3;

  SDL_Vertex vertices[len];
  vertices[0].position.x = x;
  vertices[0].position.y = y;
  vertices[0].color = BALL_COLOR;

  vertices[1].position.x = x;
  vertices[1].position.y = y - r;
  vertices[1].color = BALL_COLOR;

  vertices[2].position.x = x + r;
  vertices[2].position.y = y - r;
  vertices[2].color = BALL_COLOR;

  SDL_RenderGeometry(renderer, NULL, vertices, len, NULL, 0);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_SetRenderDrawColor(renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b,
                         BG_COLOR.a);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, FG_COLOR.r, FG_COLOR.g, FG_COLOR.b,
                         FG_COLOR.a);
  SDL_RenderRect(renderer, NULL);

  SDL_RenderLine(renderer, WIN_WIDTH / 2.0, 0, WIN_WIDTH / 2.0, WIN_HEIGHT);

  // RenderCircle(100, 100, 5);
  RenderCircle(100, 100, 20);

  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  } else if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.scancode == SDL_SCANCODE_Q) {
      return SDL_APP_SUCCESS;
    }
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
