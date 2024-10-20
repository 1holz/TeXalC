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

#ifndef TXC_MEMORY
#define TXC_MEMORY

#include "common.h"

#include "util.h"

extern txc_mem_gc *txc_mem_gc_init(void);

extern const struct txc_node *txc_mem_gc_bake(txc_mem_gc *gc, const struct txc_node *const node);

extern const struct txc_node *txc_mem_gc_copy(txc_mem_gc *gc, const struct txc_node *const node);

extern txc_mem_gc *txc_mem_gc_free(txc_mem_gc *gc, const struct txc_node *const node);

extern void txc_mem_gc_clean(struct txc_mem_gc *gc);

#endif /* TXC_MEMORY */
