#include "aiv_rasterizer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void *Init_Context()
{
    Context_t *ctx = malloc(sizeof(Context_t));
    return ctx;
}

void Append_Vector(Context_t *ctx, Triangle_t value)
{
    ctx->face_count++;
    Triangle_t *resized_area = realloc(ctx->faces, sizeof(Triangle_t) * ctx->face_count);
    if (!resized_area)
    {
        //error
        return;
    }

    ctx->faces = resized_area;
    ctx->faces[ctx->face_count - 1] = value;
}

Vertex_t Vertex_new(Vector3_t position)
{
    Vertex_t vertex;
    memset(&vertex, 0, sizeof(Vertex_t));
    vertex.position = position;
    return vertex;
}

Triangle_t Triangle_new(Vertex_t a, Vertex_t b, Vertex_t c)
{
    Triangle_t triangle = {.a = a, .b = b, .c = c};
    return triangle;
}

void PutPixel(Context_t *ctx, int x, int y, Vector3_t color)
{
    if (x < 0 || y < 0 || x >= ctx->width || y >= ctx->height)
        return;

    int offset = ((y * ctx->width) + x) * 4;
    ctx->framebuffer[offset] = 0;
    ctx->framebuffer[offset + 1] = color.x;
    ctx->framebuffer[offset + 2] = color.y;
    ctx->framebuffer[offset + 3] = color.z;
}

void PixelConverter(Vertex_t *vertex, Context_t *ctx)
{
    vertex->view.x = vertex->position.x - ctx->camera.x;
    vertex->view.y = vertex->position.y - ctx->camera.y; 

    vertex->raster_x = (vertex->view.x + 1) / 2 * ctx->width;
    vertex->raster_y = ((vertex->view.y * -1) + 1) / 2 * ctx->height;
}

void Rasterize(Context_t *ctx)
{
    int i = 0;
    for (i = 0; i < ctx->face_count; i++)
        DrawTriangle(ctx, &ctx->faces[i]);
}

void ClearBuffer(Context_t *ctx, size_t size)
{
    memset(ctx->framebuffer, 0, size);
}

float Gradient(int A, int B, int C)
{
    int dist = abs(A - B);
    return abs(A - C) / (float)dist;
    ;
}

void YOrderTriangle(Triangle_t *triangle)
{
    Vertex_t v = triangle->a;

    if (triangle->a.raster_y >= triangle->b.raster_y)
    {
        triangle->a = triangle->b;
        triangle->b = v;
    }

    v = triangle->b;
    if (triangle->b.raster_y >= triangle->c.raster_y)
    {
        triangle->b = triangle->c;
        triangle->c = v;
    }

    v = triangle->a;
    if (triangle->a.raster_y >= triangle->b.raster_y)
    {
        triangle->a = triangle->b;
        triangle->b = v;
    }
}

float Slope(float X0, float Y0, float X1, float Y1)
{
    return (X1 - X0) / (Y1 - Y0);
}

static void DrawTriangle_Slope(Context_t *ctx, Triangle_t *triangle, void (**ptr)(Context_t *, int, int, int, Vector3_t, Vector3_t))
{
    float slope0 = Slope(triangle->a.raster_x, triangle->a.raster_y, triangle->b.raster_x, triangle->b.raster_y);
    float slope1 = Slope(triangle->a.raster_x, triangle->a.raster_y, triangle->c.raster_x, triangle->c.raster_y);
    int slope = slope0 >= slope1 ? 1 : 0;

    int i = 0;
    int dist = triangle->b.raster_y - triangle->a.raster_y;
    int dist2 = triangle->c.raster_y - triangle->a.raster_y;
    int dist3 = 0;

    for (i = triangle->a.raster_y; i < triangle->a.raster_y + dist; i++)
    {
        float gradient = 1;
        if (triangle->b.raster_y != triangle->a.raster_y)
            gradient = (i - triangle->a.raster_y) / (float)dist;

        float X0 = Lerp(triangle->a.raster_x, triangle->b.raster_x, gradient);

        Vector3_t c0 = LerpVector3(triangle->a.color, triangle->b.color, gradient);
        PutPixel(ctx, X0, i, c0);

        gradient = 1;
        if (triangle->c.raster_y != triangle->a.raster_y)
            gradient = (i - triangle->a.raster_y) / (float)dist2;
        float X1 = Lerp(triangle->a.raster_x, triangle->c.raster_x, gradient);

        Vector3_t c1 = LerpVector3(triangle->a.color, triangle->c.color, gradient);
        PutPixel(ctx, X1, i, c1);

        ptr[slope](ctx, i, X0, X1, c1, c0);
    }

    dist = triangle->c.raster_y - triangle->b.raster_y;
    dist2 = triangle->c.raster_y - triangle->a.raster_y;
    dist3 = 0;

    for (i = triangle->b.raster_y; i < triangle->b.raster_y + dist; i++)
    {
        float gradient = 1;
        if (triangle->c.raster_y != triangle->b.raster_y)
            gradient = (i - triangle->b.raster_y) / (float)dist;

        float X0 = Lerp(triangle->b.raster_x, triangle->c.raster_x, gradient);

        Vector3_t c0 = LerpVector3(triangle->b.color, triangle->c.color, gradient);
        PutPixel(ctx, X0, i, c0);

        gradient = 1;
        if (triangle->c.raster_y != triangle->a.raster_y)
            gradient = (i - triangle->a.raster_y) / (float)dist2;
        float X1 = Lerp(triangle->a.raster_x, triangle->c.raster_x, gradient);

        Vector3_t c1 = LerpVector3(triangle->a.color, triangle->c.color, gradient);
        PutPixel(ctx, X1, i, c1);

        ptr[slope](ctx, i, X0, X1, c1, c0);
    }
}

static void FullTriangleDX(Context_t *ctx, int i, int X0, int X1, Vector3_t color, Vector3_t color2)
{
    int dist3 = X0 - X1;
    int j = 0;
    for (j = X1; j < X1 + dist3; j++)
    {
        float gradient = (j - X1) / (float)dist3;
        Vector3_t color3 = LerpVector3(color, color2, gradient);

        PutPixel(ctx, j, i, color3);
    }
}

static void FullTriangleSX(Context_t *ctx, int i, int X0, int X1, Vector3_t color, Vector3_t color2)
{
    int dist3 = X1 - X0;
    int j = 0;
    for (j = X0; j < X0 + dist3; j++)
    {
        float gradient = (j - X0) / (float)dist3;
        Vector3_t color3 = LerpVector3(color2, color, gradient);

        PutPixel(ctx, j, i, color3);
    }
}

void DrawTriangle(Context_t *ctx, Triangle_t *triangle)
{
    PixelConverter(&triangle->a, ctx);
    PixelConverter(&triangle->b, ctx);
    PixelConverter(&triangle->c, ctx);
    YOrderTriangle(triangle);

    void (*ptr[2])(Context_t *, int, int, int, Vector3_t, Vector3_t);
    ptr[0] = &FullTriangleSX;
    ptr[1] = &FullTriangleDX;
    DrawTriangle_Slope(ctx, triangle, ptr);
}
