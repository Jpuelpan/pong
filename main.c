#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define START_SPEED 400

SDL_Window *window;
SDL_Renderer *renderer;

SDL_FColor BG_COLOR = {0x00, 0x00, 0x00, 0xFF};
SDL_FColor FG_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};
SDL_FColor BALL_COLOR = {0x00, 0x00, 0xFF, 0xFF};

typedef struct {
  SDL_FRect rect;
  float dx;
  float dy;
  float speed;
} Ball;

typedef struct {
  SDL_FRect rect;
  float dy;
  float speed;
  int score;
} Player;

typedef struct {
  bool is_paused;
  bool next_step;

  Ball ball;
  Player left_player;
  Player right_player;
  Uint64 last_ticks;
} GameState;

void initialize_ball(Ball *ball) {
  ball->rect.x = WIN_WIDTH / 2.0;
  ball->rect.y = WIN_HEIGHT / 2.0;
  ball->rect.w = 20;
  ball->dx = -1.0;
  ball->dy = -1.0;
  ball->speed = START_SPEED;
}

void initialize_game(GameState *game) {
  initialize_ball(&game->ball);
  game->is_paused = false;

  game->left_player.rect.h = 100;
  game->left_player.rect.w = 20;
  game->left_player.rect.x = 5;
  game->left_player.rect.y = WIN_HEIGHT / 2.0 - game->left_player.rect.h / 2.0;
  game->left_player.speed = START_SPEED;
  game->left_player.dy = 0.0;
  game->left_player.score = 0;

  game->right_player.rect.h = 100;
  game->right_player.rect.w = 20;
  game->right_player.rect.x = WIN_WIDTH - game->left_player.rect.w - 5;
  game->right_player.rect.y = WIN_HEIGHT / 2.0 - game->left_player.rect.h / 2.0;
  game->right_player.speed = START_SPEED;
  game->right_player.dy = 0.0;
  game->right_player.score = 0;

  game->last_ticks = SDL_GetTicks();
}

void RenderBall(float x, float y, float size) {
  SDL_FRect r = {x - size / 2.0, y - size / 2.0, size, size};
  SDL_RenderRect(renderer, &r);
  SDL_RenderFillRect(renderer, &r);
}

void RenderPlayer(Player *player) {
  SDL_FRect r = {player->rect.x, player->rect.y, player->rect.w,
                 player->rect.h};
  SDL_RenderRect(renderer, &r);
  SDL_RenderFillRect(renderer, &r);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_Log("Starting pong...");
  GameState *game = SDL_calloc(1, sizeof(GameState));
  initialize_game(game);

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

  *appstate = game;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  GameState *game = (GameState *)appstate;
  Uint64 now = SDL_GetTicks();
  float delta = (float)(now - game->last_ticks) /
                1000.0; // Delta since last iteration in seconds

  SDL_SetRenderDrawColor(renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b,
                         BG_COLOR.a);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, FG_COLOR.r, FG_COLOR.g, FG_COLOR.b,
                         FG_COLOR.a);
  SDL_RenderRect(renderer, NULL);
  SDL_RenderLine(renderer, WIN_WIDTH / 2.0, 0, WIN_WIDTH / 2.0, WIN_HEIGHT);

  if (!game->is_paused || game->next_step) {
    // Update left player
    if (game->left_player.rect.y <= 5.0) {
      game->left_player.rect.y = 5.0;
    } else if (game->left_player.rect.y >=
               WIN_HEIGHT - game->left_player.rect.h - 5.0) {
      game->left_player.rect.y = WIN_HEIGHT - game->left_player.rect.h - 5.0;
    }
    game->left_player.rect.y +=
        game->left_player.dy * game->left_player.speed * delta;

    // Update right player
    if (game->right_player.rect.y <= 5.0) {
      game->right_player.rect.y = 5.0;
    } else if (game->right_player.rect.y >=
               WIN_HEIGHT - game->right_player.rect.h - 5.0) {
      game->right_player.rect.y = WIN_HEIGHT - game->right_player.rect.h - 5.0;
    }
    game->right_player.rect.y +=
        game->right_player.dy * game->right_player.speed * delta;

    // Ball state
    float len = SDL_sqrtf(game->ball.dx * game->ball.dx +
                          game->ball.dy * game->ball.dy);
    if (len > 0) {
      game->ball.dx /= len;
      game->ball.dy /= len;
    }
    game->ball.rect.x += game->ball.dx * game->ball.speed * delta;
    game->ball.rect.y += game->ball.dy * game->ball.speed * delta;

    if (game->ball.rect.y <= 0) {
      game->ball.dy = 1.0;
    } else if (game->ball.rect.y > WIN_HEIGHT) {
      game->ball.dy = -1.0;
    }

    if (SDL_HasRectIntersectionFloat(&game->ball.rect,
                                     &game->right_player.rect)) {
      game->ball.dx = -1.0;
    } else if (SDL_HasRectIntersectionFloat(&game->ball.rect,
                                            &game->left_player.rect)) {
      game->ball.dx = 1.0;
    }

    if (game->ball.rect.x < 0 || game->ball.rect.x > WIN_WIDTH) {
      initialize_ball(&game->ball);
    }

    SDL_Log("%f,%f | %f,%f", game->ball.rect.x, game->ball.rect.y,
            game->ball.dx, game->ball.dy);
    game->next_step = false;
  }

  // Render
  RenderBall(game->ball.rect.x, game->ball.rect.y, game->ball.rect.w);

  // Render player
  RenderPlayer(&game->left_player);
  RenderPlayer(&game->right_player);

  SDL_RenderPresent(renderer);
  game->last_ticks = SDL_GetTicks();

  SDL_Delay(1);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  GameState *game = (GameState *)appstate;

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  } else if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.scancode == SDL_SCANCODE_Q) {
      return SDL_APP_SUCCESS;
    } else if (event->key.scancode == SDL_SCANCODE_R) {
      initialize_game(game);
    } else if (event->key.scancode == SDL_SCANCODE_SPACE) {
      game->is_paused = !game->is_paused;
    } else if (event->key.scancode == SDL_SCANCODE_PERIOD) {
      game->next_step = true;
    } else if (event->key.scancode == SDL_SCANCODE_W) {
      game->left_player.dy = -1.0;
    } else if (event->key.scancode == SDL_SCANCODE_S) {
      game->left_player.dy = 1.0;
    } else if (event->key.scancode == SDL_SCANCODE_UP) {
      game->right_player.dy = -1.0;
    } else if (event->key.scancode == SDL_SCANCODE_DOWN) {
      game->right_player.dy = 1.0;
    }
  } else if (event->type == SDL_EVENT_KEY_UP) {
    if (event->key.scancode == SDL_SCANCODE_W) {
      game->left_player.dy = 0.0;
    } else if (event->key.scancode == SDL_SCANCODE_S) {
      game->left_player.dy = 0.0;
    } else if (event->key.scancode == SDL_SCANCODE_UP) {
      game->right_player.dy = 0.0;
    } else if (event->key.scancode == SDL_SCANCODE_DOWN) {
      game->right_player.dy = 0.0;
    }
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {}

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
