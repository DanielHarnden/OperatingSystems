BITS 32

global k_print

section .text

k_print:

    ; preserves the stack pointer
    push ebp
    mov ebp, esp
    push ebx

    ; moves 0 into eax, used for the offset
    mov eax, 0

    ; moves the value of the row into ecx
    mov ecx, [ebp + 16]
    cmp ecx, 0
    jle rowLoopExit

rowLoop:
    ; for each row, add 80 to the offset
    add eax, 80
    loop rowLoop

rowLoopExit:
    ; moves the value of the column into ecx
    mov ecx, [ebp + 20]
    cmp ecx, 0
    jle colLoopExit

colLoop:
    ; for each column, add 1 to the offset
    inc eax
    loop colLoop

colLoopExit:
    ; doubles the offset value, then adds the offset to 0xb8000
    add eax, eax
    mov edx, 0xb8000
    add edx, eax

    ; moves the value of the string and its length into eax and ecx respectively
    mov eax, [ebp + 8]
    mov ecx, [ebp + 12]

; loops for the length of the string
printLoop:

    ; checks if the offset exceeds the screen bounds. if so, exit
    mov ebx, 0xb8000
    add ebx, 3998
    cmp edx, ebx
    jg printLoopExit

    mov ebx, 0

    ; moves the first byte of the string into bl, then moves it into the virtaul memory
    mov bl, byte [eax]
    mov [edx], bl
    ; increases the virtual memory and string value
    inc eax
    inc edx
    ; moves white text blue background into the second byte of virtual memory 
    mov byte [edx], 31
    ; increases the virtual memory
    inc edx
    loop printLoop

; exits the print loop
printLoopExit:
    pop ebx
    pop ebp
    ret