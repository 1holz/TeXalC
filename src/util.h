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

#ifndef TXC_UTIL
#define TXC_UTIL

#define TXC_ERROR_ALLOC(bytes, purpose) fprintf(stderr, "Could not allocate %zu bytes of memory for %s at %s:%d.\n", bytes, purpose, __FILE__, __LINE__)
#define TXC_ERROR_INVALID_NODE_TYPE(type) fprintf(stderr, "Node type %u is invalid at %s:%d.\n", type, __FILE__, __LINE__)
#define TXC_ERROR_NYI() fprintf(stderr, "Not yet implemented at %s:%d.\n", __FILE__, __LINE__)
#define TXC_ERROR_OVERFLOW(cause) fprintf(stderr, "Overflow was caught for %s in %s line %d.\n", cause, __FILE__, __LINE__)

struct txc_size_t_tuple {
    size_t a;
    size_t b;
};

extern char *txc_strdup(const char *const str);

extern char *txc_strrev(char *const str);

extern char *txc_stpcpy(char *restrict dst, const char *restrict src);

#endif /* TXC_UTIL */
