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
 *
 */

#ifndef TXC_COMMON
#define TXC_COMMON

#define TXC_VERSION 000001

#ifndef __STDC__
#pragma GCC warning "This environment seems to not comply with any C standard and is thus not supported."
#endif /* __STDC__ */

#if (__STDC_VERSION__ < 199901L)
#pragma GCC warning "This environment seems to not comply with the C99 standard which is required by POSIX.1-2001 and is thus not supported."
#endif /* (__STDC_VERSION__ < 199901L) */

#ifdef _WIN32
#define TXC_WIN
#include <stdio.h>
#include <io.h>
#define fileno _fileno
#define isatty _isatty
#else /* _WIN32 */
#include <unistd.h>
#if (_POSIX_VERSION >= 200112L)
#define TXC_POSIX
#endif /* (_POSIX_VERSION >= 200112L) */
#endif /* _WIN32 */

#if !defined(TXC_WIN) && !defined(TXC_POSIX)
#pragma GCC warning "This environment seems to not comply with neither Windows nor POSIX.1-2001 or newer and is thus not supported."
#endif /* !defined(TXC_WIN) && !defined(TXC_POSIX) */

#endif /* TXC_COMMON */
