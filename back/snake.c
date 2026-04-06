#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "snake.h"
#include <time.h>

// get grid value at (x, y)
// (0, 0) is at bottom left;
// x is horizontal
// if out_x and out_y are not null
// store wrapped coords into them
int*
at(struct grid *g, int x, int y, int* out_x, int* out_y)
{
    int size = g->size;

    int wrapped_x = (x % size + size) % size;
    int wrapped_y = (y % size + size) % size;

    if (out_x != 0 && out_y != 0)
    {
        *out_x = wrapped_x;
        *out_y = wrapped_y;
    }

    return &g->array[wrapped_x + wrapped_y * size];
}

// 4 debug
void
display(struct grid *g)
{
    for (int i = g->size - 1; i >= 0; i--)
    {
        for (int j = 0; j < g->size; j++)
        {
            printf("%d \t", *at(g, j, i, 0, 0));
        }
        printf("\n");
    }
    printf("\n");
}

// write val to random point on the grid
// generated coords are saved into rx and ry
// if both of them aren't null
void
genobj(struct grid *g, int val, int* rx, int *ry)
{
    int* cell;
    int _rx, _ry;
    do
    {
        _rx = rand() % g->size;
        _ry = rand() % g->size;
        cell = at(g, _rx, _ry, 0, 0);
    } while (*cell != 0);
    *cell = val;

    if (rx != 0 && ry != 0)
    {
        *rx = _rx;
        *ry = _ry;
    }
}

void
update_coords(int dir, int* x, int* y)
{
    switch (dir)
    {
    case UP:
        *y += 1;
        break;
    case DOWN:
        *y -= 1;
        break;
    case RIGHT:
        *x += 1;
        break;
    case LEFT:
        *x -= 1;
        break;    
    default:
        break;
    }
}

// stateless game state update
int
step(struct grid *g, struct snake *s, int dir)
{
    if (abs(s->dir - dir) == 2 && s->len > 1) // opposite direction check
    {
        dir = s->dir;
    }

    s->dir = dir;

    int nextx = s->parts[s->len - 1].x;
    int nexty = s->parts[s->len - 1].y;

    update_coords(dir, &nextx, &nexty);

    if (*at(g, nextx, nexty, 0, 0) > 0) //snake part
    {
        struct part tail, next;
        at(g, s->parts[0].x, s->parts[0].y, &tail.x, &tail.y); //getting wrapped coords
        at(g, nextx, nexty, &next.x, &next.y);

        if (tail.x == next.x && tail.y == next.y) // stepping into tail is allowed
        {
            goto move;
        }

        return GAMEOVER;
    }
    else if (*at(g, nextx, nexty, 0, 0) < 0) //apple
    {
        struct part *p = &s->parts[s->len];
        p->x = nextx;
        p->y = nexty;
        s->len++;
        *at(g, nextx, nexty, 0, 0) = 1;

        if (s->len == g->size * g->size)
        {
            return VICTORY;
        }

        genobj(g, -1, 0, 0);
    }
    else //move
    {
move:
        *at(g, s->parts[0].x, s->parts[0].y, 0, 0) = 0;
        
        for (int i = 0; i < s->len - 1; i++)
        {
            s->parts[i].x = s->parts[i + 1].x;
            s->parts[i].y = s->parts[i + 1].y;
        }
        
        struct part *head = &s->parts[s->len - 1];
        head->x = nextx;
        head->y = nexty;
        *at(g, head->x, head->y, 0, 0) = 1;
    }

    return CONTINUE;
}


void
grid_free(struct grid *g)
{
    free(g->array);
    g->array = 0;
}

void
snake_free(struct snake *s)
{
    free(s->parts);
    s->parts = 0;
}

// size must be > 0
void
init(struct grid *g, int size, struct snake *s)
{
    g->array = malloc(size * size * sizeof(int));
    for (int i = 0; i < size * size; i++)
    {
        g->array[i] = 0;
    }
    g->size = size;

    s->parts = malloc(size * size * sizeof(struct part));
    s->len = 1;

    struct part* tail = &s->parts[0];
    genobj(g, 1, &tail->x, &tail->y);
    genobj(g, -1, 0, 0);
}

/*
int main()
{
    srand(time(0));
    struct grid g;
    struct snake s;
    init(&g, 5, &s);
    display(&g);
    char c;
    int rs;
    while ((c = getchar()) != 'Q')
    {
        if (c == 'W')
        {
            rs = step(&g, &s, UP);
            display(&g);
        }
        if (c == 'A')
        {
            rs = step(&g, &s, LEFT);
            display(&g);
        }
        if (c == 'S')
        {
            rs = step(&g, &s, DOWN);
            display(&g);
        }
        if (c == 'D')
        {
            rs = step(&g, &s, RIGHT);
            display(&g);
        }

        switch (rs)
        {
        case CONTINUE:
            printf("CONTINUE\n");
            break;
        case GAMEOVER:
            printf("GAMEOVER\n");
            break;
        case VICTORY:
            printf("VICTORY\n");
            break;        
        default:
            break;
        }
    }

    snake_free(&s);
    grid_free(&g);
} */