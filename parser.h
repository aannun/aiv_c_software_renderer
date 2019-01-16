#include "aiv_rasterizer.h"

enum ObjParserState
{
    NOP = 0, 
    VERTEX = 1,
    UV = 2,
    NORMAL = 3,
    FACE = 4,
};

typedef struct ObjParserStruct
{
    enum ObjParserState state;
    size_t v_index;
    size_t t_index;
    size_t vt_index;
    size_t vn_index;

    Vertex_t *vertices;
    Triangle_t *triangles;

}ObjParserStruct_t;

char *read_file(const char *filename, size_t *file_size);

void read_obj(char *data, size_t data_len, Context_t *ctx);