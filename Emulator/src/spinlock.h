/*
Copyright (©) 2024  Frosty515

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

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long spinlock_t;

#define spinlock_init(lock) (*(lock) = 0)
#define spinlock_new(name) spinlock_t name = 0

#ifdef __APPLE__

void spinlock_acquire(spinlock_t* lock);
void spinlock_release(spinlock_t* lock);

#else /* __APPLE__ */

#define spinlock_acquire(lock) _spinlock_acquire(lock)
#define spinlock_release(lock) _spinlock_release(lock)

void _spinlock_acquire(spinlock_t* lock);
void _spinlock_release(spinlock_t* lock);

#endif /* __APPLE__ */

#ifdef __cplusplus
}
#endif

#endif /* _SPINLOCK_H */