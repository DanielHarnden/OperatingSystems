BITS 32

global k_print, dispatcher, go, lidtr, init_timer_dev, outportb
extern enqueue_priority, dequeue, Running, queue

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



; PROGRAM 2 ----------------------------------------------------------------------------------------------------------------
go:
    ;dequeue PCB, make it running process
    push queue
    call dequeue
    mov [Running], eax

    ;set esp value
    mov esp, [eax]

    ;restore and return
    pop gs
    pop fs 
    pop es 
    pop ds 
    popad
    iret

dispatcher:
    ;save everything important
    pushad
    push ds
    push es 
    push fs 
    push gs
    mov edi, [Running]
    mov [edi], esp
    
    ;set up parameters for enqueue
    push edi
    push queue
    call enqueue_priority

    ;set up parameter for dequeue
    push queue
    call dequeue

    ;set current process and esp value
    mov [Running], eax
    mov esp, [eax]

    ;restore and return
    pop gs
    pop fs 
    pop es 
    pop ds 
    popad

    ;send EOI to PIC
    push eax
    mov al, 0x200
    out 0x200, al
    pop eax

    iret


lidtr:
    ;preserve stack information
    push ebp
    mov ebp, esp
    push ebx

    ;lidt the function input
    mov eax, [ebp+8]
    lidt [eax]

    ;pop stack information
    pop ebx
    pop ebp
    ret



; interrupt timer
init_timer_dev:
    ;step 1
    push ebp
    mov ebp, esp
    pushf
    push edx
    push eax

    ;step 2
    mov edx, [ebp + 8]

    ;step 3
    mov ax, 1193
    imul dx, ax

    ;steps 4 and 5
    mov al, 0b00110110
    out 0x43, al
    mov ax, dx
    out 0x40, al
    xchg ah, al
    out 0x40, al

    ;step 6
    pop eax
    pop edx
    popf
    pop ebp
    ret



outportb:
    ;preserve stack
    push ebp
    mov ebp, esp
    pushf
    push eax
    push edx

    ;moves the correct input into the correct registers, then preforms the out function
    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, al

    ;pop stack
    pop edx
    pop eax
    popf
    pop ebp
    ret