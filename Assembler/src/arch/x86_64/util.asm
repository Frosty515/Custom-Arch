; Copyright (©) 2023  Frosty515
; 
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

[bits 64]

global fast_memset
fast_memset:
    push rbp
    mov rbp, rsp

    push rdi

    mov r8b, dl ; save low byte of dl for later

    xor rcx, rcx
    and rdx, ~7
    jz .l2

    mov r9, rdx ; save rdx in r9 as it might be overridden by imul
    
    ; check if there is a point in creating a 64-bit wide version of sil
    test rsi, rsi
    jz .l
    mov rax, 0x0101010101010101
    imul rsi

; use fast 64-bit operations as much as possible
.l:
    mov QWORD [rdi+rcx], rax
    add rcx, 8
    cmp rcx, r9
    jl .l

    ; update address
    add rdi, rcx

; prepare to set the rest using 8-bit operations
.l2:
    and r8b, 7
    jz .end
    xor rcx, rcx

; do the setting
.l3:
    mov BYTE [rdi+rcx], al
    add cl, 1
    cmp cl, r8b
    jl .l3

; cleanup
.end:
    pop rax ; get return value
    mov rsp, rbp
    pop rbp
    ret

global fast_memcpy
fast_memcpy:
    push rbp
    mov rbp, rsp

    push rdi

    mov r8b, dl ; save low byte of dl for later

    xor rcx, rcx
    mov rax, ~7
    and rdx, rax
    jz .l2

; use fast 64-bit operations as much as possible
.l:
    mov r9, QWORD [rsi+rcx]
    mov QWORD [rdi+rcx], r9
    add rcx, 8
    cmp rcx, rdx
    jl .l

    ; update addresses
    add rdi, rcx
    add rsi, rcx

; prepare to copy the rest using 8-bit operations
.l2:
    and r8b, 7
    jz .end
    xor rcx, rcx

; do the copy
.l3:
    mov r9b, BYTE [rsi+rcx]
    mov BYTE [rdi+rcx], r9b
    add cl, 1
    cmp cl, r8b
    jl .l3

; cleanup
.end:
    pop rax ; get return value
    mov rsp, rbp
    pop rbp
    ret