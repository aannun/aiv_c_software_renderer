#include "aiv_renderer.h"
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>

#define triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2) \
    Triangle_new(                                    \
        Vertex_new(Vector3_new(x0, y0, z0)),         \
        Vertex_new(Vector3_new(x1, y1, z1)),         \
        Vertex_new(Vector3_new(x2, y2, z2)))


static void Rotate(Vertex_t *vertex, Vector3_t pivot, float value)
{
    float X = vertex->position.x - pivot.x;
    float Y = vertex->position.y - pivot.y;

    float length = sqrt(X * X + Y * Y);
    float normX = (float)X / length;
    float normY = (float)Y / length;

    float ang = atan2(normY, normX);
    ang += value;

    float newSin = sin(ang);
    float newCos = cos(ang);

    Y = newSin * length;
    X = newCos * length;

    vertex->position.x = X;
    vertex->position.y = Y;
}

int main(int argc, char **argv)
{
    Context_t *ctx = malloc(sizeof(Context_t));// Init_Context();
    ctx->width = 600;
    ctx->height = 600;
    ctx->framebuffer = NULL;
    ctx->faces = NULL;
    ctx->face_count = 0;
    ctx->camera = Vector3_zero();

    Triangle_t triangle = triangle(-1, 1, 0, -0.5, 1, 0, -1, 0.5, 0);
    triangle.a.color = Vector3_new(255, 0, 0);
    triangle.b.color = Vector3_new(0, 255, 0);
    triangle.c.color = Vector3_new(0, 0, 255);

    Triangle_t triangle2 = triangle(1, 1, 0, 0.5, 1, 0, 1, 0.5, 0);
    triangle2.a.color = Vector3_new(255, 0, 0);
    triangle2.b.color = Vector3_new(0, 255, 0);
    triangle2.c.color = Vector3_new(0, 0, 255);

    Triangle_t triangle3 = triangle(-1, -1, 0, -0.5, -1, 0, -1, -0.5, 0);
    triangle3.a.color = Vector3_new(255, 0, 0);
    triangle3.b.color = Vector3_new(0, 255, 0);
    triangle3.c.color = Vector3_new(0, 0, 255);

    Triangle_t triangle4 = triangle(1, -1, 0, 0.5, -1, 0, 1, -0.5, 0);
    triangle4.a.color = Vector3_new(255, 0, 0);
    triangle4.b.color = Vector3_new(0, 255, 0);
    triangle4.c.color = Vector3_new(0, 0, 255);

    Append_Vector(ctx, triangle);
    Append_Vector(ctx, triangle2);
    Append_Vector(ctx, triangle3);
    Append_Vector(ctx, triangle4);
    
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 600, 600);

    Vector3_t pivot = Vector3_zero();

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