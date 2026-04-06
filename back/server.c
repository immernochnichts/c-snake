#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "snake.h"

#define PORT 8888

struct request_data {
    char *data;
    size_t size;
};

void parse_grid(cJSON *json, struct grid *g)
{
    int size = cJSON_GetObjectItem(json, "size")->valueint;
    g->size = size;

    g->array = malloc(size * size * sizeof(int));

    cJSON *grid = cJSON_GetObjectItem(json, "grid");

    for (int i = 0; i < size * size; i++)
    {
        g->array[i] = cJSON_GetArrayItem(grid, i)->valueint;
    }
}

void parse_snake(cJSON *json, struct snake *s)
{
    cJSON *snake = cJSON_GetObjectItem(json, "snake");

    int len = cJSON_GetObjectItem(snake, "len")->valueint;
    s->len = len;
    s->dir = cJSON_GetObjectItem(snake, "dir")->valueint;
    s->parts = malloc((len + 1) * sizeof(struct part));

    cJSON *parts = cJSON_GetObjectItem(snake, "parts");
    int arr_size = cJSON_GetArraySize(parts);
    if (len != arr_size) {
        fprintf(stderr, "Warning: len %d != snake array size %d\n", len, arr_size);
        len = arr_size;
    }

    for (int i = 0; i < len; i++)
    {
        cJSON *p = cJSON_GetArrayItem(parts, i);
        s->parts[i].x = cJSON_GetObjectItem(p, "x")->valueint;
        s->parts[i].y = cJSON_GetObjectItem(p, "y")->valueint;
    }
}

cJSON* serialize_grid(struct grid *g)
{
    cJSON *arr = cJSON_CreateArray();

    for (int i = 0; i < g->size * g->size; i++)
    {
        cJSON_AddItemToArray(arr, cJSON_CreateNumber(g->array[i]));
    }

    return arr;
}

cJSON* serialize_snake(struct snake *s)
{
    cJSON *jsnake = cJSON_CreateObject();
    cJSON *arr = cJSON_CreateArray();

    for (int i = 0; i < s->len; i++)
    {
        cJSON *p = cJSON_CreateObject();
        cJSON_AddNumberToObject(p, "x", s->parts[i].x);
        cJSON_AddNumberToObject(p, "y", s->parts[i].y);
        cJSON_AddItemToArray(arr, p);
    }

    cJSON_AddItemToObject(jsnake, "parts", arr);
    cJSON_AddNumberToObject(jsnake, "dir", s->dir);
    cJSON_AddNumberToObject(jsnake, "len", s->len);
    return jsnake;
}

static enum MHD_Result handler(void *cls,
                   struct MHD_Connection *connection,
                   const char *url,
                   const char *method,
                   const char *version,
                   const char *upload_data,
                   size_t *upload_data_size,
                   void **con_cls)
{
    if (strcmp(method, "OPTIONS") == 0) {
        struct MHD_Response *response =
            MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);

        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    } else if (strcmp(method, "POST") != 0)
        return MHD_NO;

    struct request_data *req = *con_cls;

    if (!req) {
        req = calloc(1, sizeof(struct request_data));
        *con_cls = req;
        return MHD_YES;
    }

    if (*upload_data_size != 0) {
        req->data = realloc(req->data, req->size + *upload_data_size + 1);
        memcpy(req->data + req->size, upload_data, *upload_data_size);
        req->size += *upload_data_size;
        req->data[req->size] = '\0';

        *upload_data_size = 0;
        return MHD_YES;
    }

    cJSON *json = cJSON_Parse(req->data);
    if (!json)
    {
        free(req->data);
        free(req);
        return MHD_NO;
    }

    int dir = cJSON_GetObjectItem(json, "dir")->valueint;

    struct grid g;
    struct snake s;

    parse_grid(json, &g);
    parse_snake(json, &s);

    int status = step(&g, &s, dir);

    cJSON *res = cJSON_CreateObject();

    cJSON_AddNumberToObject(res, "status", status);
    cJSON_AddNumberToObject(res, "size", g.size);

    cJSON_AddItemToObject(res, "grid", serialize_grid(&g));
    cJSON_AddItemToObject(res, "snake", serialize_snake(&s));

    char *response_str = cJSON_Print(res);

    struct MHD_Response *response =
        MHD_create_response_from_buffer(strlen(response_str),
                                        response_str,
                                        MHD_RESPMEM_MUST_FREE);

    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Content-Type", "application/json");

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    free(g.array);
    free(s.parts);
    free(req->data);
    free(req);

    cJSON_Delete(res);
    cJSON_Delete(json);

    return ret;
}

int main()
{
    struct MHD_Daemon *daemon =
        MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD,
                         PORT,
                         NULL, NULL,
                         &handler, NULL,
                         MHD_OPTION_END);

    printf("started server on localhost:%d\n", PORT);

    getchar();
    MHD_stop_daemon(daemon);
}