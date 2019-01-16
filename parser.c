#include <stdio.h>
#include "parser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char *read_file(const char *filename, size_t *file_size)
{
    FILE *fhandle = fopen(filename, "rb");
    if (!fhandle)
        return NULL;

    fseek(fhandle, 0, SEEK_END);
    *file_size = ftell(fhandle);
    fseek(fhandle, 0, SEEK_SET);

    char *data = malloc(*file_size);
    if (!data)
    {
        fclose(fhandle);
        return NULL;
    }

    fread(data, 1, *file_size, fhandle);
    fclose(fhandle);

    return data;
}

static void Append_Vertex(ObjParserStruct_t *obj, Vertex_t value)
{
    obj->v_index++;
    Vertex_t *resized_area = realloc(obj->vertices, sizeof(Vertex_t) * obj->v_index);
    if (!resized_area)
    {
        printf_s("error");
        return;
    }

    obj->vertices = resized_area;
    obj->vertices[obj->v_index - 1] = value;
}

static void Append_Triangle(ObjParserStruct_t *obj, Triangle_t value)
{
    obj->t_index++;
    Triangle_t *resized_area = realloc(obj->triangles, sizeof(Triangle_t) * obj->t_index);
    if (!resized_area)
    {
        printf_s("error");
        return;
    }

    obj->triangles = resized_area;
    obj->triangles[obj->t_index - 1] = value;
}

static int search_data(char *data, size_t index, ObjParserStruct_t *obj)
{
    if (data[index] == 'v' && data[index + 1] == ' ')
    {
        index += 2;
        obj->state = VERTEX;
        return index;
    }
    else if (data[index] == 'f' && data[index + 1] == ' ')
    {
        index += 2;
        obj->state = FACE;
        return index;
    }
    else if(data[index] == 'v' && data[index + 1] == 't' && data[index + 2] == ' ')
    {
        index += 3;
        obj->state = NOP;
        return index;
    }
    else if(data[index] == 'v' && data[index + 1] == 'n' && data[index + 2] == ' ')
    {
        index += 3;
        obj->state = NORMAL;
        return index;
    }
    return ++index;
}

static float get_float_from_data(char *data, size_t index, size_t start_index)
{
    size_t diff = index - start_index;
    char val[1]; // = malloc(diff);
    _memccpy(val, data + start_index, 0, diff);
    float v = atof(val);
    //free(val);
    return v;
}

static int get_int_from_data(char *data, size_t index, size_t start_index)
{
    size_t diff = index - start_index;
    char val[1]; // = malloc(diff);
    _memccpy(val, data + start_index, 0, diff);
    int v = atoi(val);
    //free(val);
    return v;
}

static Vector3_t read_vector3(char *data, size_t *index, ObjParserStruct_t *obj)
{
    size_t start_index = *index;
    float vertices[3] = { 0, 0, 0 };
    int i = 0;

    while (data[*index] == ' ' || data[*index] == '-' || data[*index] == '.' || isdigit(data[*index]))
    {
        if (data[*index] == ' ')
        {
            vertices[i] = get_float_from_data(data, *index, start_index);
            start_index = *index;
            i++;
        }
        *index = *index + 1;
    }
    vertices[i] = get_float_from_data(data, *index, start_index);

    Vector3_t vec = Vector3_new(vertices[0], vertices[1], vertices[2]);
    return vec;
}

static int read_vertex(char *data, size_t index, ObjParserStruct_t *obj)
{
    Vector3_t vec = read_vector3(data, &index, obj);
    Vertex_t v = Vertex_new(vec);
    Append_Vertex(obj, v);

    obj->state = NOP;
    return index;
}

static int read_normal(char *data, size_t index, ObjParserStruct_t *obj)
{
    Vector3_t vec = read_vector3(data, &index, obj);
    obj->vertices[obj->vn_index].normal = vec;
    obj->vn_index++;

    float f0[3] = { vec.x, vec.y, vec.z };
    float f1[3] = { 0, 0, -1 };
    obj->vertices[obj->vn_index - 1].color = LerpVector3(Vector3_new(0, 0, 255), Vector3_new(255, 0, 0), (dot_product(f0, f1, 3) + 1) / 2);

    obj->state = NOP;
    return index;
}

static int read_uv(char *data, size_t index, ObjParserStruct_t *obj)
{
    Vector3_t vec = read_vector3(data, &index, obj);
    obj->vertices[obj->vt_index].uv_x = vec.x;
    obj->vertices[obj->vt_index].uv_y = vec.y;
    obj->vt_index++;

    obj->state = NOP;
    return index;
}

static int read_face(char *data, size_t index, ObjParserStruct_t *obj)
{
    size_t start_index = index;
    size_t info[9];
    int i = 0;

    while (data[index] == ' ' || data[index] == '/' || isdigit(data[index]))
    {
        while (isdigit(data[index]))
            index++;

        if (data[index] == '/' || data[index] == ' ')
        {
            info[i] = get_int_from_data(data, index, start_index);
            i++;
            index++;
            start_index = index;
        }
    }
    info[i] = get_int_from_data(data, index, start_index);

    Triangle_t t = Triangle_new(obj->vertices[info[0] - 1], obj->vertices[info[3] - 1], obj->vertices[info[6] - 1]);
    Append_Triangle(obj, t);

    obj->state = NOP;
    return index;
}

void read_obj(char *data, size_t data_len, Context_t *ctx)
{
    ObjParserStruct_t parse_obj;
    parse_obj.state = NOP;
    parse_obj.triangles = NULL;
    parse_obj.vertices = NULL;
    parse_obj.t_index = 0;
    parse_obj.v_index = 0;

    int (*states[5])(char *, size_t, ObjParserStruct_t *);
    states[NOP] = *search_data;
    states[VERTEX] = *read_vertex;
    states[UV] = *read_uv;
    states[NORMAL] = *read_normal;
    states[FACE] = *read_face;

    size_t i = 0;
    while (i < data_len)
        i = states[parse_obj.state](data, i, &parse_obj);

    int j = 0;
    for (j = 0; j < parse_obj.t_index; j++)
        Append_Vector(ctx, parse_obj.triangles[j]);
}