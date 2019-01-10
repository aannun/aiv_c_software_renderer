#include "aiv_rasterizer.h"
#include <string.h>
#include <stdio.h>

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
    ctx->framebuffer[offset] = (char)color.x;
    ctx->framebuffer[offset + 1] = (char)color.y;
    ctx->framebuffer[offset + 2] = (char)color.z;
    ctx->framebuffer[offset + 3] = 255;
}

void PixelConverter(Vertex_t *vertex, Context_t *ctx)
{
    vertex->raster_x = (vertex->position.x + 1) / 2 * ctx->width;
    vertex->raster_y = ((vertex->position.y * -1) + 1) / 2 * ctx->height;
}

void rasterize(Context_t *ctx, Triangle_t *triangle)
{
    PixelConverter(&triangle->a, ctx);
    PixelConverter(&triangle->b, ctx);
    PixelConverter(&triangle->c, ctx);
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

static void DrawTriangle_Slope(Context_t ctx, Triangle_t triangle, void (**ptr)(Context_t, int, int, int, Vector3_t, Vector3_t))
{
    float slope0 = Slope(triangle.a.raster_x, triangle.a.raster_y, triangle.b.raster_x, triangle.b.raster_y);
    float slope1 = Slope(triangle.a.raster_x, triangle.a.raster_y, triangle.c.raster_x, triangle.c.raster_y);
    int slope = slope0 >= slope1 ? 1 : 0;

    int i = 0;
    int dist = triangle.b.raster_y - triangle.a.raster_y;
    int dist2 = triangle.c.raster_y - triangle.a.raster_y;
    int dist3 = 0;

    for (i = triangle.a.raster_y; i < triangle.a.raster_y + dist; i++)
    {
        float gradient = (i - triangle.a.raster_y) / (float)dist;
        float X0 = Lerp(triangle.a.raster_x, triangle.b.raster_x, gradient);

        Vector3_t color;
        color.x = Lerp(triangle.a.color.x, triangle.b.color.x, gradient);
        color.y = Lerp(triangle.a.color.y, triangle.b.color.y, gradient);
        color.z = Lerp(triangle.a.color.z, triangle.b.color.z, gradient);

        PutPixel(&ctx, X0, i, color);

        gradient = (i - triangle.a.raster_y) / (float)dist2;
        float X1 = Lerp(triangle.a.raster_x, triangle.c.raster_x, gradient);
        
        Vector3_t color2;
        color2.x = Lerp(triangle.a.color.x, triangle.c.color.x, gradient);
        color2.y = Lerp(triangle.a.color.y, triangle.c.color.y, gradient);
        color2.z = Lerp(triangle.a.color.z, triangle.c.color.z, gradient);

        PutPixel(&ctx, X1, i, color2);

        ptr[slope](ctx, i, X0, X1, color2, color);
    }

    dist = triangle.c.raster_y - triangle.b.raster_y;
    dist2 = triangle.c.raster_y - triangle.a.raster_y;
    dist3 = 0;

    for (i = triangle.b.raster_y; i < triangle.b.raster_y + dist; i++)
    {
        float gradient = (i - triangle.b.raster_y) / (float)dist;

        float X0 = Lerp(triangle.b.raster_x, triangle.c.raster_x, gradient);

        Vector3_t color;
        color.x = Lerp(triangle.b.color.x, triangle.c.color.x, gradient);
        color.y = Lerp(triangle.b.color.y, triangle.c.color.y, gradient);
        color.z = Lerp(triangle.b.color.z, triangle.c.color.z, gradient);

        PutPixel(&ctx, X0, i, color);

        gradient = (i - triangle.b.raster_y) / (float)dist2;
        float X1 = Lerp(triangle.a.raster_x, triangle.c.raster_x, gradient);

        Vector3_t color2;
        color2.x = Lerp(triangle.c.color.x, triangle.a.color.x, gradient);
        color2.y = Lerp(triangle.c.color.y, triangle.a.color.y, gradient);
        color2.z = Lerp(triangle.c.color.z, triangle.a.color.z, gradient);

        PutPixel(&ctx, X1, i, color2);

        ptr[slope](ctx, i, X0, X1, color2, color);
    }
}

static void FullTriangleDX(Context_t ctx, int i, int X0, int X1, Vector3_t color, Vector3_t color2)
{
    int dist3 = X0 - X1;
    int j = 0;
    for (j = X1; j < X1 + dist3; j++)
    {
        float gradient = (j - X1) / (float)dist3;

        Vector3_t color3;
        color3.x = Lerp(color.x, color2.x, gradient);
        color3.y = Lerp(color.y, color2.y, gradient);
        color3.z = Lerp(color.z, color2.z, gradient);
        PutPixel(&ctx, j, i, color3);
    }
}

static void FullTriangleSX(Context_t ctx, int i, int X0, int X1, Vector3_t color, Vector3_t color2)
{
    int dist3 = X1 - X0;
    int j = 0;
    for (j = X0; j < X0 + dist3; j++)
    {
        float gradient = (j - X0) / (float)dist3;

        Vector3_t color3;
        color3.x = Lerp(color2.x, color.x, gradient);
        color3.y = Lerp(color2.y, color.y, gradient);
        color3.z = Lerp(color2.z, color.z, gradient);

        PutPixel(&ctx, j, i, color3);
    }
}

void DrawTriangle(Context_t ctx, Triangle_t triangle)
{
    rasterize(&ctx, &triangle);
    YOrderTriangle(&triangle);

    void (*ptr[2])(Context_t, int, int, int, Vector3_t, Vector3_t);
    ptr[0] = &FullTriangleSX;
    ptr[1] = &FullTriangleDX;
    DrawTriangle_Slope(ctx, triangle, ptr);
}
