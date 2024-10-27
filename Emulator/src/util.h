/*
Copyright (©) 2023-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _EMULATOR_UTIL_H
#define _EMULATOR_UTIL_H

#include <stdint.h>
#include <stddef.h>

#define KiB(x) ((uint64_t)x * 1024)
#define MiB(x) (KiB(x) * 1024)
#define GiB(x) (MiB(x) * 1024)

#define DIV_ROUNDUP(VALUE, DIV) ((VALUE + (DIV - 1)) / DIV)
#define DIV_ROUNDUP_ADDRESS(ADDR, DIV) ((void*)DIV_ROUNDUP(((unsigned long)ADDR), DIV) * DIV)

#define ALIGN_UP(VALUE, ALIGN) (DIV_ROUNDUP(VALUE, ALIGN) * ALIGN)
#define ALIGN_UP_ADDRESS(ADDR, ALIGN) ((void*)ALIGN_UP(((unsigned long)ADDR), ALIGN))

#define ALIGN_DOWN(VALUE, ALIGN) ((VALUE / ALIGN) * ALIGN)
#define ALIGN_DOWN_ADDRESS(ADDR, ALIGN) ((void*)ALIGN_DOWN(((unsigned long)ADDR), ALIGN))

#define IN_BOUNDS(VALUE, MIN, MAX) ((VALUE >= MIN) && (VALUE <= MAX))

#define FLAG_SET(x, flag) (x |= flag)
#define FLAG_UNSET(x, flag) (x &= ~flag)

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#ifdef __cplusplus
extern "C" {
#endif

/* Implemented in x86_64 assembly */

void* fast_memset(void* ptr, uint8_t c, size_t n);
void* fast_memcpy(void* dst, void* src, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* _EMULATOR_UTIL_H */
