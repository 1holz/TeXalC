/*
 *     Copyright (C) 2024  Einholz
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published
 *     by the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 *
 *     You should have received a copy of the GNU Affero General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "node.h"

#ifndef TXC_GC_INIT_SIZE
#define TXC_GC_INIT_SIZE 16
#endif /* TXC_GC_INIT_SIZE */
#ifndef TXC_GC_HOLES_INIT_SIZE
#define TXC_GC_HOLES_INIT_SIZE 16
#endif /* TXC_GC_HOLES_INIT_SIZE */
#ifndef TXC_GC_GROWTH_FACTOR
#define TXC_GC_GROWTH_FACTOR 2
#endif /* TXC_GC_GROWTH_FACTOR */
#ifndef TXC_GC_SHRINK_TRESHOLD
#define TXC_GC_SHRINK_TRESHOLD 4
#endif /* TXC_GC_SHRINK_TRESHOLD */

struct collected_entity {
    size_t ref_count;
    const struct txc_node *node;
    char *str;
};

struct txc_mem_gc {
    struct collected_entity *entities;
    size_t entities_size;
    size_t amount;
    size_t last;
    size_t holes_size;
    size_t height;
    size_t holes[];
};

struct txc_mem_gc *txc_mem_gc_init(void)
{
    struct txc_mem_gc *const gc = malloc(sizeof *gc + sizeof *gc->holes * TXC_GC_HOLES_INIT_SIZE);
    if (gc == NULL)
        return NULL;
    gc->entities = malloc(sizeof *gc->entities * TXC_GC_INIT_SIZE);
    if (gc->entities == NULL) {
        free(gc);
        return NULL;
    }
    for (size_t i = 0; i < TXC_GC_INIT_SIZE; i++)
        gc->entities[i].ref_count = 0;
    gc->entities_size = TXC_GC_INIT_SIZE;
    gc->last = 0;
    gc->holes_size = TXC_GC_HOLES_INIT_SIZE;
    gc->height = 0;
    return gc;
}

const struct txc_node *txc_mem_gc_bake(struct txc_mem_gc *gc, const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    if (txc_node_get_read_only(node))
        return node;
    if (gc->height == 0 && gc->last == gc->entities_size - 1) {
        if (SIZE_MAX / TXC_GC_GROWTH_FACTOR < sizeof *gc->entities * gc->entities_size) {
            txc_node_free(node);
            TXC_ERROR_OVERFLOW("gc entities");
            return &TXC_NAN_ERROR_OVERFLOW;
        }
        struct collected_entity *const tmp = realloc(gc->entities, sizeof *gc->entities * gc->entities_size * TXC_GC_GROWTH_FACTOR);
        if (tmp == NULL) {
            txc_node_free(node);
            TXC_ERROR_ALLOC(sizeof *gc->entities * gc->entities_size * TXC_GC_GROWTH_FACTOR, "larger gc");
            return &TXC_NAN_ERROR_ALLOC;
        }
        gc->entities = tmp;
        for (size_t i = gc->entities_size; i < gc->entities_size * TXC_GC_GROWTH_FACTOR; i++)
            gc->entities[i].ref_count = 0;
        gc->entities_size *= TXC_GC_GROWTH_FACTOR;
    }
    size_t i;
    if (gc->height == 0) {
        i = gc->last;
        gc->last++;
    } else {
        do {
            gc->height--;
            i = gc->holes[gc->height];
        } while (i > gc->last);
        if (gc->height < gc->holes_size / TXC_GC_SHRINK_TRESHOLD) {
            struct txc_mem_gc *const tmp = realloc(gc, sizeof *tmp + sizeof *gc->holes * (gc->holes_size / TXC_GC_GROWTH_FACTOR));
            if (tmp != NULL) {
                gc = tmp;
                gc->holes_size = gc->holes_size / TXC_GC_GROWTH_FACTOR;
            }
        }
    }
    const struct collected_entity entity = { .ref_count = 1, .node = node, .str = NULL };
    gc->entities[i] = entity;
    return txc_node_set_gc_i((struct txc_node *)node, i);
}

const struct txc_node *txc_mem_gc_copy(struct txc_mem_gc *gc, const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    if (txc_node_get_read_only(node))
        return node;
    size_t i = txc_node_get_gc_i(node);
    assert(gc->entities[i].node == node);
    gc->entities[i].ref_count++;
    return node;
}

struct txc_mem_gc *txc_mem_gc_free(struct txc_mem_gc *gc, const struct txc_node *const node)
{
    assert(txc_node_test_valid(node, true));
    if (txc_node_get_read_only(node))
        return gc;
    size_t i = txc_node_get_gc_i(node);
    assert(gc->entities[i].node == node);
    if (gc->entities[i].ref_count > 1) {
        gc->entities[i].ref_count--;
        return gc;
    }
    gc->entities[i].ref_count = 0;
    for (size_t i = 0; i < gc->entities[i].node->children_amount; i++)
        gc = txc_mem_gc_free(gc, gc->entities[i].node->children[i]);
    txc_node_free(gc->entities[i].node);
    free(gc->entities[i].str);
    // TODO decrease entities?
    if (i == gc->last) {
        gc->last--;
    } else {
        if (gc->height == gc->holes_size) {
            if ((SIZE_MAX - sizeof *gc) / TXC_GC_GROWTH_FACTOR < sizeof *gc->holes * gc->holes_size) {
                TXC_ERROR_OVERFLOW("gc holes");
                return gc;
            }
            struct txc_mem_gc *const tmp = realloc(gc, sizeof *gc + sizeof *gc->holes * gc->holes_size * TXC_GC_GROWTH_FACTOR);
            if (tmp == NULL) {
                TXC_ERROR_ALLOC(sizeof *gc->entities * gc->entities_size * TXC_GC_GROWTH_FACTOR, "larger gc");
                return gc;
            }
        }
        gc->holes[gc->height] = i;
        gc->height++;
    }

    return gc;
}

void txc_mem_gc_clean(struct txc_mem_gc *gc)
{
    for (size_t i = 0; i < gc->last; i++) {
        if (gc->entities[i].ref_count == 0)
            continue;
        txc_node_free(gc->entities->node);
        free(gc->entities->str);
    }
    free(gc->entities);
    free(gc);
}
