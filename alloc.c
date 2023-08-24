#include <stdio.h>
#include <unistd.h>

#define align4(x) (((((x)-1) >> 2) << 2) + 4)
#define BLOCK_SIZE 20

void *base = NULL;

typedef struct block_t
{
    size_t size;
    struct block_t *next;
    struct block_t *prev;
    int free;
    void *ptr; // used to verify block integrity
    char data[1];
} block;

block *find_block(block **last, size_t size)
{
    block *b = base;
    while (b && !(b->free && b->size >= size))
    {
        *last = b;
        b = b->next;
    }

    return b;
}

block *extend_heap(block *last, size_t size)
{
    block *b = sbrk(0);

    if (sbrk(BLOCK_SIZE + size) == (void *)-1)
    {
        return NULL;
    }

    b->size = size;
    b->free = 0;
    b->ptr = b->data;
    if (last)
    {
        last->next = b;
        b->prev = last;
    }
    return b;
}

void split_block(block *b, size_t size)
{
    block *new = (block *)b->data + size;
    new->size = b->size - size - BLOCK_SIZE;
    new->free = 1;

    new->next = b->next;
    new->prev = b;

    if (b->next)
    {
        b->next->prev = new;
    }

    b->size = size;
    b->next = new;
}

void *t_malloc(size_t size)
{
    block *b, *last;
    size_t s = align4(size);

    if (base)
    {
        last = base;
        b = find_block(&last, s);

        if (b)
        {
            // split
            if ((b->size - s) >= (BLOCK_SIZE + 4))
            {
                split_block(b, s);
            }
            b->free = 0;
        }
        else
        {
            b = extend_heap(last, s);
            if (!b)
            {
                return NULL;
            }
        }
    }
    else
    {
        b = extend_heap(NULL, s);
        if (!b)
        {
            return NULL;
        }
        base = b;
    }

    return b->data;
}

block *funsion(block *b)
{
    if (b->next && b->next->free)
    {
        b->size += b->next->size + BLOCK_SIZE;
        b->next = b->next->next;

        if (b->next)
        {
            b->next->prev = b;
        }
    }

    return b;
}

block *get_block(void *p)
{
    char *tmp = p;
    return (p = tmp -= BLOCK_SIZE);
}

int valid_addr(void *p)
{
    if (base)
    {
        if (p > base && p < sbrk(0))
        {
            return (p == (get_block(p))->ptr);
        }
    }

    return 0;
}

void t_free(void *p)
{
    if (!valid_addr(p))
    {
        return;
    }

    block *b = get_block(p);
    b->free = 1;
    b = funsion(b);

    if (b->prev && b->prev->free)
    {
        b = b->prev;
        b = funsion(b);
    }

    if (b->prev)
    {
        b->prev->next = NULL;
    }
    else
    {
        base = NULL;
    }

    // release memory
    if (!b->next)
    {
        brk(b);
    }
}

void copy_block(block *src, block *dst)
{
    int *sptr = src->ptr;
    int *dptr = src->ptr;

    for (size_t i = 0; i * 4 < src->size && i * 4 < dst->size; i++)
    {
        dptr[i] = sptr[i];
    }
}

void *t_realloc(void *ptr, size_t size)
{
    if (!ptr)
    {
        return t_malloc(size);
    }

    if (!valid_addr(ptr))
    {
        return NULL;
    }

    size_t s = align4(size);
    block *b = get_block(ptr);

    if (b->size >= s)
    {
        if (b->size - s >= (BLOCK_SIZE + 4))
        {
            split_block(b, s);
        }
    }
    else
    {
        // Try fusion with next block
        if (b->next && b->next->free && (b->size + b->next->size + BLOCK_SIZE) >= s)
        {
            funsion(b);

            if (b->size - s >= (BLOCK_SIZE + 4))
            {
                split_block(b, s);
            }
        }
        else
        {
            // lets malloc a new block and copy the old one data to it

            void *new = t_malloc(s);
            if (!new)
            {
                return NULL;
            }

            block *new_b = get_block(new);
            copy_block(b, new_b);
            t_free(ptr);

            return new;
        }
    }
    return b;
}

void *calloc(size_t number, size_t size)
{
    size_t *new = t_malloc(number * size);
    if (new)
    {
        // a small trick to reduce the loops number (zeros in the size of size_t)
        size_t s4 = align4(number * size) >> 2;
        for (size_t i = 0; i < s4; i++)
        {
            new[i] = 0;
        }
    }
    return new;
}
