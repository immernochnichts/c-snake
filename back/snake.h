#define ARRAYSNAKE

struct grid
{
    int *array;
    int size;
};

enum
{
    RIGHT = 1, UP, LEFT, DOWN
};

enum
{
    CONTINUE, GAMEOVER, VICTORY
};

struct snake
{
    int dir;
    int len;
    struct part *parts;
};

struct part
{
    int x, y;
};

int*        at(struct grid *g, int x, int y, int *out_x, int *out_y);
//void        display(struct grid *g);
int         step(struct grid *g, struct snake *s, int dir);
void        genobj(struct grid *g, int val, int* rx, int *ry);
void        update_coords(int dir, int* x, int* y);
void        grid_free(struct grid *g);
void        snake_free(struct snake *s);
void        init(struct grid *g, int size, struct snake *s);