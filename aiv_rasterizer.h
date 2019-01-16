#include "aiv_math.h"

typedef struct Vertex
{
    Vector3_t position;
    Vector3_t normal;
    Vector3_t color;
    Vector3_t view;
    float uv_x;
    float uv_y;

    int raster_x;
    int raster_y;
} Vertex_t;

typedef struct Triangle
{
    Vertex_t a;
    Vertex_t b;
    Vertex_t c;
} Triangle_t;

typedef struct Context
{
    int width;
    int height;
    int face_count;
    Vector3_t camera;

    unsigned char *framebuffer;
    Triangle_t *faces;
} Context_t;

void *Init_Context();

void Append_Vector(Context_t *ctx, Triangle_t value);

Vertex_t Vertex_new(Vector3_t position);

Triangle_t Triangle_new(Vertex_t a, Vertex_t b, Vertex_t c);

void Rasterize(Context_t *ctx);

void PutPixel(Context_t *ctx, int x, int y, Vector3_t color);

void ClearBuffer(Context_t *ctx, size_t size);

float Gradient(int A, int B, int C);

void YOrderTriangle(Triangle_t *triangle);

float Slope(float X0, float Y0, float X1, float Y1);

void DrawTriangle(Context_t *ctx, Triangle_t *triangle);