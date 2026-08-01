#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define START_SPEED 400

SDL_Window *window;
SDL_Renderer *renderer;

SDL_FColor BG_COLOR = {0x00, 0x00, 0x00, 0xFF};
SDL_FColor FG_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};
SDL_FColor BALL_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};

SDL_Texture *NUMBERS_TEXTURE = NULL;
float DIGIT_WIDTH = 32.0;
float DIGIT_HEIGHT = 64.0;

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
  ball->dx = SDL_randf() * (SDL_rand(2) == 1 ? 1 : -1);
  ball->dy = SDL_randf() * (SDL_rand(2) == 1 ? 1 : -1);
  ball->speed = START_SPEED;

  SDL_Log("Initialize ball - x: %.2f - y: %.2f - dx: %.2f - dy: %.2f - s: %.2f",
          ball->rect.x, ball->rect.y, ball->dx, ball->dy, ball->speed);
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

void initialize_textures() {
  SDL_Surface *numbers_surface = SDL_LoadBMP("assets/numbers.bmp");
  SDL_SetSurfaceBlendMode(numbers_surface, SDL_BLENDMODE_BLEND);
  SDL_SetSurfaceColorKey(numbers_surface, true, 0x000000);
  NUMBERS_TEXTURE = SDL_CreateTextureFromSurface(renderer, numbers_surface);
  SDL_SetTextureAlphaMod(NUMBERS_TEXTURE, 0xAA);
}

void RenderBall(float x, float y, float size) {
  SDL_FRect r = {x - size / 2.0, y - size / 2.0, size, size};
  SDL_SetRenderDrawColor(renderer, BALL_COLOR.r, BALL_COLOR.g, BALL_COLOR.b,
                         BALL_COLOR.a);
  SDL_RenderRect(renderer, &r);
  SDL_RenderFillRect(renderer, &r);
}

void RenderPlayer(Player *player) {
  SDL_FRect r = {player->rect.x, player->rect.y, player->rect.w,
                 player->rect.h};
  SDL_SetRenderDrawColor(renderer, FG_COLOR.r, FG_COLOR.g, FG_COLOR.b,
                         FG_COLOR.a);
  SDL_RenderRect(renderer, &r);
  SDL_RenderFillRect(renderer, &r);
}

void RenderScore(int score, SDL_FPoint *center, float char_w, float char_h) {
  char str_score[12];
  sprintf(str_score, "%d", score);
  int score_len = (int)strlen(str_score);
  float score_width = score_len * char_w;
  float sx = center->x - (score_width / 2.0);
  float sy = center->y - (char_h / 2.0);

  for (int i = 0; i < score_len; i++) {
    int digit = str_score[i] - 48;

    SDL_FRect src = {
        .x = DIGIT_WIDTH * digit,
        .y = 0,
        .w = DIGIT_WIDTH,
        .h = DIGIT_HEIGHT,
    };

    SDL_FRect dst = {
        .x = sx + (char_w * i),
        .y = sy,
        .w = char_w,
        .h = char_h,
    };

    if (!SDL_RenderTexture(renderer, NUMBERS_TEXTURE, &src, &dst)) {
      SDL_Log("Failed to render texture: %s", SDL_GetError());
    }
  }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_Log("Starting pong...");

  GameState *game = SDL_calloc(1, sizeof(GameState));
  initialize_game(game);

#ifdef __EMSCRIPTEN__
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
#endif

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

  initialize_textures();
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
      SDL_Log("Right player hit - ball: %.2f,%.2f,%.2f,%.2f - player: "
              "%.2f,%.2f,%.2f,%.2f",
              game->ball.rect.x, game->ball.rect.y, game->ball.rect.w,
              game->ball.rect.h, game->right_player.rect.x,
              game->right_player.rect.y, game->right_player.rect.w,
              game->right_player.rect.h);
      game->ball.dx = -1.0;
    } else if (SDL_HasRectIntersectionFloat(&game->ball.rect,
                                            &game->left_player.rect)) {
      SDL_Log("Left  player hit - ball: %.2f,%.2f,%.2f,%.2f - player: "
              "%.2f,%.2f,%.2f,%.2f",
              game->ball.rect.x, game->ball.rect.y, game->ball.rect.w,
              game->ball.rect.h, game->left_player.rect.x,
              game->left_player.rect.y, game->left_player.rect.w,
              game->left_player.rect.h);
      game->ball.dx = 1.0;
    }

    // Update score
    if (game->ball.rect.x < 0) {
      game->right_player.score++;
      SDL_Log("Score for right %d", game->right_player.score);
      initialize_ball(&game->ball);
    } else if (game->ball.rect.x > WIN_WIDTH) {
      game->left_player.score++;
      SDL_Log("Score for left %d", game->left_player.score);
      initialize_ball(&game->ball);
    }

    game->next_step = false;
  }

  // Render Score
  SDL_FPoint left_score = {
      .x = (float)WIN_WIDTH / 4.0,
      .y = (float)WIN_HEIGHT / 4.0,
  };

  SDL_FPoint right_score = {
      .x = (float)WIN_WIDTH - (float)WIN_WIDTH / 4.0,
      .y = (float)WIN_HEIGHT / 4.0,
  };

  RenderScore(game->left_player.score, &left_score, 32.0, 64.0);
  RenderScore(game->right_player.score, &right_score, 32.0, 64.0);

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
