/*
 *     Copyright (C) 2024 - 2025  Einholz
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

#ifndef TXC_TEST
#define TXC_TEST

#include "common.h"

#include <errno.h>

#define TXC_TEST_ERROR_CLOSE(fd) fprintf(stderr, "[ERROR] @%s:%u Closing file descriptor %u failed: %s", __FILE__, __LINE__, fd, strerror(errno))
#define TXC_TEST_ERROR_DUP(fd) fprintf(stderr, "[ERROR] @%s:%u Duplicating file descriptor %u failed: %s", __FILE__, __LINE__, fd, strerror(errno))
#define TXC_TEST_ERROR_DUP2(from, to) fprintf(stderr, "[ERROR] @%s:%u Duplicating file descriptor %u to %u failed: %s", __FILE__, __LINE__, from, to, strerror(errno))
#define TXC_TEST_ERROR_PIPE() fprintf(stderr, "[ERROR] @%s:%u Creating pipes failed: %s", __FILE__, __LINE__, strerror(errno))
#define TXC_TEST_ERROR_FORK() fprintf(stderr, "[ERROR] @%s:%u Forking failed: %s", __FILE__, __LINE__, strerror(errno))
#define TXC_TEST_ERROR_READ(size, fd) fprintf(stderr, "[ERROR] @%s:%u Reading %zu bytes from     fd %u failed: %s", __FILE__, __LINE__, size, fd, strerror(errno))
#define TXC_TEST_ERROR_WRITE(size, fd) fprintf(stderr, "[ERROR] @%s:%u Writing %zu bytes to fd %u failed: %s", __FILE__, __LINE__, size, fd, strerror(errno))

#define TXC_TEST_WARN_CLOSE(fd) fprintf(stderr, "[WARN] @%s:%u Closing file descriptor %u failed: %s", __FILE__, __LINE__, fd, strerror(errno))
#define TXC_TEST_WARN_DUP2(from, to) fprintf(stderr, "[WARN] @%s:%u Duplicating file descriptor %u to %u failed: %s", __FILE__, __LINE__, from, to, strerror(errno))
#define TXC_TEST_WARN_FFLUSH() fprintf(stderr, "[WARN] @%s:%u Flushing a stream failed: %s", __FILE__, __LINE__, strerror(errno))

#endif /* TXC_TEST */
