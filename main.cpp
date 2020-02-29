#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_ttf.h>

#include <png.h>

#include "vec.hpp"

template <typename T>
T *stec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError());
        abort();
    }

    return ptr;
}

void stec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError());
        abort();
    }
}

void sec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
}

template <typename T>
T *sec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return ptr;
}

const int TILE_SIZE = 128;
const int TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

enum class Tile
{
    Empty = 0,
    Wall
};

const int LEVEL_WIDTH = 10;
const int LEVEL_HEIGHT = 10;
const SDL_Rect level_boundary = {
    0, 0, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE
};

Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Wall, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, },
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
{Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
};

struct Sprite
{
    SDL_Rect srcrect;
    SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   SDL_Rect destrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    sec(SDL_RenderCopyEx(
            renderer,
            texture.texture,
            &texture.srcrect,
            &destrect,
            0.0,
            nullptr,
            flip));
}

void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   Vec2i pos,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    SDL_Rect destrect = {
        pos.x - texture.srcrect.w / 2, pos.y - texture.srcrect.h / 2,
        texture.srcrect.w, texture.srcrect.h
    };

    sec(SDL_RenderCopyEx(
            renderer,
            texture.texture,
            &texture.srcrect,
            &destrect,
            0.0,
            nullptr,
            flip));
}

static inline
bool is_tile_inbounds(Vec2i p)
{
    return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2i p)
{
    return !is_tile_inbounds(p) || level[p.y][p.x] == Tile::Empty;
}

void render_level(SDL_Renderer *renderer,
                  Sprite top_ground_texture,
                  Sprite bottom_ground_texture)
{
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
            case Tile::Empty: {
            } break;

            case Tile::Wall: {
                if (is_tile_empty(vec2(x, y - 1))) {
                    render_sprite(renderer, top_ground_texture,
                                  {x * TILE_SIZE, y * TILE_SIZE,
                                   TILE_SIZE, TILE_SIZE});
                } else {
                    render_sprite(renderer, bottom_ground_texture,
                                  {x * TILE_SIZE, y * TILE_SIZE,
                                   TILE_SIZE, TILE_SIZE});
                }
            } break;
            }
        }
    }
}

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, const char *image_filename)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    // TODO(#6): implement libpng error checker similar to the SDL one
    // TODO(#7): try stb_image.h instead of libpng
    //   https://github.com/nothings/stb/blob/master/stb_image.h
    if (!png_image_begin_read_from_file(&image, image_filename)) {
        fprintf(stderr, "Could not read file `%s`: %s\n",
                image_filename, image.message);
        abort();
    }
    image.format = PNG_FORMAT_RGBA;
    uint32_t *image_pixels = new uint32_t[image.width * image.height];

    if (!png_image_finish_read(&image, nullptr, image_pixels, 0, nullptr)) {
        fprintf(stderr, "libpng pooped itself: %s\n", image.message);
        abort();
    }

    SDL_Surface* image_surface =
        sec(SDL_CreateRGBSurfaceFrom(image_pixels,
                                     image.width,
                                     image.height,
                                     32,
                                     image.width * 4,
                                     0x000000FF,
                                     0x0000FF00,
                                     0x00FF0000,
                                     0xFF000000));

    SDL_Texture *image_texture =
        sec(SDL_CreateTextureFromSurface(renderer,
                                         image_surface));
    SDL_FreeSurface(image_surface);

    return image_texture;
}

struct Animat
{
    Sprite  *frames;
    size_t   frame_count;
    size_t   frame_current;
    uint32_t frame_duration;
    uint32_t frame_cooldown;
};

static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   SDL_Rect dstrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    render_sprite(
        renderer,
        animat.frames[animat.frame_current % animat.frame_count],
        dstrect,
        flip);
}

static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   Vec2i pos,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    render_sprite(
        renderer,
        animat.frames[animat.frame_current % animat.frame_count],
        pos,
        flip);
}

void update_animat(Animat *animat, uint32_t dt)
{
    if (dt < animat->frame_cooldown) {
        animat->frame_cooldown -= dt;
    } else {
        animat->frame_current = (animat->frame_current + 1) % animat->frame_count;
        animat->frame_cooldown = animat->frame_duration;
    }
}

enum class Entity_Dir
{
    Right = 0,
    Left
};

struct Entity
{
    SDL_Rect texbox;
    SDL_Rect hitbox;
    Vec2i pos;
    Vec2i vel;

    Animat idle;
    Animat walking;
    Animat *current;
    // TODO: special separate type for the Player dir
    Entity_Dir dir;
};



static inline
int sqr_dist(Vec2i p0, Vec2i p1)
{
    auto d = p0 - p1;
    return d.x * d.x + d.y * d.y;
}

void resolve_point_collision(Vec2i *p)
{
    assert(p);

    const auto tile = *p / TILE_SIZE;

    if (is_tile_empty(tile)) {
        return;
    }

    const auto p0 = tile * TILE_SIZE;
    const auto p1 = (tile + 1) * TILE_SIZE;

    struct Side {
        int d;
        Vec2i np;
        Vec2i nd;
        int dd;
    };

    Side sides[] = {
        {sqr_dist({p0.x, 0},    {p->x, 0}),    {p0.x, p->y}, {-1,  0}, TILE_SIZE_SQR},     // left
        {sqr_dist({p1.x, 0},    {p->x, 0}),    {p1.x, p->y}, { 1,  0}, TILE_SIZE_SQR},     // right
        {sqr_dist({0, p0.y},    {0, p->y}),    {p->x, p0.y}, { 0, -1}, TILE_SIZE_SQR},     // top
        {sqr_dist({0, p1.y},    {0, p->y}),    {p->x, p1.y}, { 0,  1}, TILE_SIZE_SQR},     // bottom
        {sqr_dist({p0.x, p0.y}, {p->x, p->y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // top-left
        {sqr_dist({p1.x, p0.y}, {p->x, p->y}), {p1.x, p0.y}, { 1, -1}, TILE_SIZE_SQR * 2}, // top-right
        {sqr_dist({p0.x, p1.y}, {p->x, p->y}), {p0.x, p1.y}, {-1,  1}, TILE_SIZE_SQR * 2}, // bottom-left
        {sqr_dist({p1.x, p1.y}, {p->x, p->y}), {p1.x, p1.y}, { 1,  1}, TILE_SIZE_SQR * 2}  // bottom-right
    };
    const int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

    int closest = -1;
    for (int current = 0; current < SIDES_COUNT; ++current) {
        for (int i = 1;
             !is_tile_empty(tile + (sides[current].nd * i)) ;
             ++i)
        {
            sides[current].d += sides[current].dd;
        }

        if (closest < 0 || sides[closest].d >= sides[current].d) {
            closest = current;
        }
    }

    *p = sides[closest].np;
}

void resolve_entity_collision(Entity *entity)
{
    assert(entity);

    Vec2i p0 = vec2(entity->hitbox.x, entity->hitbox.y) + entity->pos;
    Vec2i p1 = p0 + vec2(entity->hitbox.w, entity->hitbox.h);

    Vec2i mesh[] = {
        p0,
        {p1.x, p0.y},
        {p0.x, p1.y},
        p1,
    };
    const int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);

    for (int i = 0; i < MESH_COUNT; ++i) {
        Vec2i t = mesh[i];
        resolve_point_collision(&t);
        Vec2i d = t - mesh[i];

        const int IMPACT_THRESHOLD = 5;
        if (std::abs(d.y) >= IMPACT_THRESHOLD) entity->vel.y = 0;
        if (std::abs(d.x) >= IMPACT_THRESHOLD) entity->vel.x = 0;

        for (int j = 0; j < MESH_COUNT; ++j) {
            mesh[j] += d;
        }

        entity->pos += d;
    }
}

SDL_Rect entity_dstrect(const Entity entity)
{
    SDL_Rect dstrect = {
        entity.texbox.x + entity.pos.x, entity.texbox.y + entity.pos.y,
        entity.texbox.w, entity.texbox.h
    };
    return dstrect;
}

SDL_Rect entity_hitbox(const Entity entity)
{
    SDL_Rect hitbox = {
        entity.hitbox.x + entity.pos.x, entity.hitbox.y + entity.pos.y,
        entity.hitbox.w, entity.hitbox.h
    };
    return hitbox;
}

void render_entity(SDL_Renderer *renderer, const Entity entity)
{
    const auto dstrect = entity_dstrect(entity);
    const SDL_RendererFlip flip =
        entity.dir == Entity_Dir::Right
        ? SDL_FLIP_NONE
        : SDL_FLIP_HORIZONTAL;
    render_animat(renderer, *entity.current, dstrect, flip);
}

void update_entity(Entity *entity, uint32_t dt)
{
    assert(entity);
    update_animat(entity->current, dt);
}

SDL_Texture *render_text_as_texture(SDL_Renderer *renderer,
                                    TTF_Font *font,
                                    const char *text,
                                    SDL_Color color)
{
    SDL_Surface *surface = stec(TTF_RenderText_Blended(font, text, color));
    SDL_Texture *texture = stec(SDL_CreateTextureFromSurface(renderer, surface));
    SDL_FreeSurface(surface);
    return texture;
}

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, Vec2i p)
{
    int w, h;
    sec(SDL_QueryTexture(texture, NULL, NULL, &w, &h));
    SDL_Rect srcrect = {0, 0, w, h};
    SDL_Rect dstrect = {p.x, p.y, w, h};
    sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
}

void displayf(SDL_Renderer *renderer, TTF_Font *font,
              SDL_Color color, Vec2i p,
              const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char text[256];
    vsnprintf(text, sizeof(text), format, args);

    SDL_Texture *texture =
        render_text_as_texture(renderer, font, text, color);
    render_texture(renderer, texture, p);
    SDL_DestroyTexture(texture);

    va_end(args);
}

enum class Debug_Draw_State {
    Idle,
    Create,
    Delete
};

Animat load_spritesheet_animat(SDL_Renderer *renderer,
                               size_t frame_count,
                               uint32_t frame_duration,
                               const char *spritesheet_filepath)
{
    Animat result = {};
    result.frames = new Sprite[frame_count];
    result.frame_count = frame_count;
    result.frame_duration = frame_duration;

    SDL_Texture *spritesheet = load_texture_from_png_file(renderer, spritesheet_filepath);
    int spritesheet_w = 0;
    int spritesheet_h = 0;
    sec(SDL_QueryTexture(spritesheet, NULL, NULL, &spritesheet_w, &spritesheet_h));
    int sprite_w = spritesheet_w / frame_count;
    int sprite_h = spritesheet_h; // NOTE: we only handle horizontal sprites

    for (int i = 0; i < (int) frame_count; ++i) {
        result.frames[i].texture = spritesheet;
        result.frames[i].srcrect = {i * sprite_w, 0, sprite_w, sprite_h};
    }

    return result;
}

enum class Projectile_State
{
    Ded = 0,
    Active,
    Poof
};

struct Projectile
{
    Projectile_State state;
    Vec2i pos;
    Vec2i vel;
    Animat active_animat;
    Animat poof_animat;
    Entity_Dir dir;
};

const size_t projectiles_count = 69;
Projectile projectiles[projectiles_count] = {};

void init_projectiles(Animat active_animat, Animat poof_animat)
{
    for (size_t i = 0; i < projectiles_count; ++i) {
        projectiles[i].active_animat = active_animat;
        projectiles[i].poof_animat = poof_animat;
    }
}

int count_alive_projectiles(void)
{
    int res = 0;
    for (size_t i = 0; i < projectiles_count; ++i) {
        if (projectiles[i].state != Projectile_State::Ded) ++res;
    }
    return res;
}

void spawn_projectile(Vec2i pos, Vec2i vel, Entity_Dir dir)
{
    for (size_t i = 0; i < projectiles_count; ++i) {
        if (projectiles[i].state == Projectile_State::Ded) {
            projectiles[i].state = Projectile_State::Active;
            projectiles[i].pos = pos;
            projectiles[i].vel = vel;
            projectiles[i].dir = dir;
            return;
        }
    }
}

void render_projectiles(SDL_Renderer *renderer)
{

    for (size_t i = 0; i < projectiles_count; ++i) {
        const SDL_RendererFlip flip =
            projectiles[i].dir == Entity_Dir::Right
            ? SDL_FLIP_NONE
            : SDL_FLIP_HORIZONTAL;

        switch (projectiles[i].state) {
        case Projectile_State::Active: {
            render_animat(renderer,
                          projectiles[i].active_animat,
                          projectiles[i].pos,
                          flip);
        } break;

        case Projectile_State::Poof: {
            render_animat(renderer,
                          projectiles[i].poof_animat,
                          projectiles[i].pos,
                          flip);
        } break;

        case Projectile_State::Ded: {} break;
        }
    }
}

void update_projectiles(uint32_t dt)
{
    for (size_t i = 0; i < projectiles_count; ++i) {
        switch (projectiles[i].state) {
        case Projectile_State::Active: {
            update_animat(&projectiles[i].active_animat, dt);
            projectiles[i].pos += projectiles[i].vel;
            const auto projectile_tile = projectiles[i].pos / TILE_SIZE;
            if (!is_tile_empty(projectile_tile) || !is_tile_inbounds(projectile_tile)) {
                projectiles[i].state = Projectile_State::Poof;
                projectiles[i].poof_animat.frame_current = 0;
            }
        } break;

        case Projectile_State::Poof: {
            update_animat(&projectiles[i].poof_animat, dt);
            if (projectiles[i].poof_animat.frame_current ==
                (projectiles[i].poof_animat.frame_count - 1)) {
                projectiles[i].state = Projectile_State::Ded;
            }
        } break;

        case Projectile_State::Ded: {} break;
        }
    }

}

void dump_level(void)
{
    std::printf("{\n");
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        std::printf("{");
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
            case Tile::Empty: {
                std::printf("Tile::Empty, ");
            } break;

            case Tile::Wall: {
                std::printf("Tile::Wall, ");
            } break;
            }
        }
        std::printf("},");
        std::printf("\n");
    }
    std::printf("}\n");
}

int main(void)
{
    sec(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
        sec(SDL_CreateWindow(
                "Something",
                0, 0, 800, 600,
                SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sec(SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

    // TODO(#8): replace fantasy_tiles.png with our own assets
    SDL_Texture *tileset_texture = load_texture_from_png_file(
        renderer,
        "assets/fantasy_tiles.png");

    Sprite ground_grass_texture = {
        {120, 128, 16, 16},
        tileset_texture
    };

    Sprite ground_texture = {
        {120, 128 + 16, 16, 16},
        tileset_texture
    };

    Animat plasma_pop_animat = load_spritesheet_animat(
        renderer, 4, 70,
        "./assets/Destroy1-sheet.png");
    Animat plasma_bolt_animat = load_spritesheet_animat(
        renderer, 5, 70,
        "./assets/spark1-sheet.png");
    init_projectiles(plasma_bolt_animat, plasma_pop_animat);

    // TODO(#9): baking assets into executable
    SDL_Texture *walking_texture = load_texture_from_png_file(
        renderer,
        "./assets/walking-12px-zoom.png");

    const int walking_frame_count = 4;
    const int walking_frame_size = 48;
    Sprite walking_frames[walking_frame_count];

    for (int i = 0; i < walking_frame_count; ++i) {
        walking_frames[i].srcrect = {
            i * walking_frame_size,
            0,
            walking_frame_size,
            walking_frame_size
        };
        walking_frames[i].texture = walking_texture;
    }

    const int PLAYER_TEXBOX_SIZE = 64;
    const int PLAYER_HITBOX_SIZE = PLAYER_TEXBOX_SIZE - 20;
    const SDL_Rect texbox = {
        - (PLAYER_TEXBOX_SIZE / 2), - (PLAYER_TEXBOX_SIZE / 2),
        PLAYER_TEXBOX_SIZE, PLAYER_TEXBOX_SIZE
    };
    const SDL_Rect hitbox = {
        - (PLAYER_HITBOX_SIZE / 2), - (PLAYER_HITBOX_SIZE / 2),
        PLAYER_HITBOX_SIZE, PLAYER_HITBOX_SIZE
    };

    Animat walking = {};
    walking.frames = walking_frames;
    walking.frame_count = 4;
    walking.frame_duration = 100;

    Animat idle = {};
    idle.frames = walking_frames + 2;
    idle.frame_count = 1;
    idle.frame_duration = 200;

    Entity player = {};
    player.texbox = texbox;
    player.hitbox = hitbox;
    player.walking = walking;
    player.idle = idle;
    player.current = &player.idle;

    Entity supposed_enemy = {};
    supposed_enemy.texbox = texbox;
    supposed_enemy.hitbox = hitbox;
    supposed_enemy.walking = walking;
    supposed_enemy.idle = idle;
    supposed_enemy.current = &supposed_enemy.idle;

    stec(TTF_Init());
    const int DEBUG_FONT_SIZE = 32;
    TTF_Font *debug_font = stec(TTF_OpenFont("assets/UbuntuMono-R.ttf", DEBUG_FONT_SIZE));

    int ddy = 1;
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    bool quit = false;
    bool debug = false;
    const int COLLISION_PROBE_SIZE = 10;
    SDL_Rect collision_probe = {};
    Vec2i mouse_position = {};
    SDL_Rect tile_rect = {};
    Debug_Draw_State state = Debug_Draw_State::Idle;

    Uint32 fps = 0;
    while (!quit) {
        const Uint32 begin = SDL_GetTicks();

        const int PLAYER_SPEED = 4;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            } break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    player.vel.y = -20;
                } break;

                case SDLK_q: {
                    debug = !debug;
                } break;

                case SDLK_e: {
                    if (player.dir == Entity_Dir::Right) {
                        spawn_projectile(player.pos, vec2(10, 0), player.dir);
                    } else {
                        spawn_projectile(player.pos, vec2(-10, 0), player.dir);
                    }
                } break;

                case SDLK_r: {
                    player.pos = vec2(0, 0);
                    player.vel.y = 0;
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                Vec2i p = {event.motion.x, event.motion.y};
                resolve_point_collision(&p);

                collision_probe = {
                    p.x - COLLISION_PROBE_SIZE, p.y - COLLISION_PROBE_SIZE,
                    COLLISION_PROBE_SIZE * 2, COLLISION_PROBE_SIZE * 2
                };

                tile_rect = {
                    event.motion.x / TILE_SIZE * TILE_SIZE,
                    event.motion.y / TILE_SIZE * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE
                };

                mouse_position = {event.motion.x, event.motion.y};

                Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                switch (state) {
                case Debug_Draw_State::Create: {
                    if (is_tile_inbounds(tile)) level[tile.y][tile.x] = Tile::Wall;
                } break;

                case Debug_Draw_State::Delete: {
                    if (is_tile_inbounds(tile)) level[tile.y][tile.x] = Tile::Empty;
                } break;

                default: {}
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                if (debug) {
                    Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                    if (is_tile_inbounds(tile)) {
                        if (level[tile.y][tile.x] == Tile::Empty) {
                            state = Debug_Draw_State::Create;
                            level[tile.y][tile.x] = Tile::Wall;
                        } else {
                            state = Debug_Draw_State::Delete;
                            level[tile.y][tile.x] = Tile::Empty;
                        }
                    }
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                state = Debug_Draw_State::Idle;
            } break;
            }
        }

        if (keyboard[SDL_SCANCODE_D]) {
            player.vel.x = PLAYER_SPEED;
            player.current = &player.walking;
            player.dir = Entity_Dir::Right;
        } else if (keyboard[SDL_SCANCODE_A]) {
            player.vel.x = -PLAYER_SPEED;
            player.current = &player.walking;
            player.dir = Entity_Dir::Left;
        } else {
            player.vel.x = 0;
            player.current = &player.idle;
        }

        player.vel.y += ddy;

        player.pos += player.vel;

        resolve_entity_collision(&player);

        sec(SDL_SetRenderDrawColor(renderer, 18, 8, 8, 255));
        sec(SDL_RenderClear(renderer));

        render_level(renderer, ground_grass_texture, ground_texture);
        render_entity(renderer, player);
        render_projectiles(renderer);

        if (debug) {
            sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

            auto dstrect = entity_dstrect(player);
            sec(SDL_RenderDrawRect(renderer, &dstrect));
            sec(SDL_RenderFillRect(renderer, &collision_probe));
            sec(SDL_RenderDrawRect(renderer, &tile_rect));
            sec(SDL_RenderDrawRect(renderer, &level_boundary));

            const Uint32 t = SDL_GetTicks() - begin;
            const Uint32 fps_snapshot = t ? 1000 / t : 0;
            fps = (fps + fps_snapshot) / 2;

            const int PADDING = 10;
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, PADDING),
                     "FPS: %d", fps);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 50 + PADDING),
                     "Mouse Position: (%d, %d)",
                     mouse_position.x, mouse_position.y);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 2 * 50 + PADDING),
                     "Collision Probe: (%d, %d)",
                     collision_probe.x, collision_probe.y);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 3 * 50 + PADDING),
                     "Projectiles: %d",
                     count_alive_projectiles());


            sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
            auto hitbox = entity_hitbox(player);
            sec(SDL_RenderDrawRect(renderer, &hitbox));
        }


        SDL_RenderPresent(renderer);

        const Uint32 dt = SDL_GetTicks() - begin;

        update_entity(&player, dt);
        update_projectiles(dt);
    }
    SDL_Quit();

    dump_level();

    return 0;
}
