#include "parser.h"
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>

#define triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2) \
    Triangle_new(                                    \
        Vertex_new(Vector3_new(x0, y0, z0)),         \
        Vertex_new(Vector3_new(x1, y1, z1)),         \
        Vertex_new(Vector3_new(x2, y2, z2)))

int main(int argc, char **argv)
{
    Context_t *ctx = malloc(sizeof(Context_t));
    ctx->width = 600;
    ctx->height = 600;
    ctx->framebuffer = NULL;
    ctx->faces = NULL;
    ctx->face_count = 0;
    ctx->camera = Vector3_new(0, 1.5, -5);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 600, 600);

    size_t len;
    char *data = read_file("Stormtrooper.obj", &len);
    if(!data)
        printf_s("error");
    read_obj(data, len, ctx);

    for (;;)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return 0;

            if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_DOWN)
                    ctx->camera.y -= 0.1;
                else if(event.key.keysym.sym == SDLK_UP)
                    ctx->camera.y += 0.1;
				else if (event.key.keysym.sym == SDLK_RIGHT)
                    ctx->camera.x += 0.1;
				else if (event.key.keysym.sym == SDLK_LEFT)
                    ctx->camera.x -= 0.1;
                else if (event.key.keysym.sym == SDLK_w)
                    ctx->camera.z += 0.1;
				else if (event.key.keysym.sym == SDLK_s)
                    ctx->camera.z -= 0.1;
			}
        }

        int pitch;
        SDL_LockTexture(texture, NULL, (void **)&ctx->framebuffer, &pitch);

        ClearBuffer(ctx, pitch * ctx->height);

        Rasterize(ctx);

        SDL_UnlockTexture(texture);

        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    return 0;
}