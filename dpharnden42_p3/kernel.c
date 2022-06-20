#include <stdio.h>
#include <stdint.h>

// Program 1 Function Declarations
void k_print(char *string, int string_length, int row, int col);
void k_clearscr();
void print_border(int start_row, int start_col, int end_row, int end_col);
void setup_PIC();
void outportb(uint16_t port, uint8_t value);
// ASM Function Declarations
extern void dispatcher();
extern void go();
extern void init_timer_dev(int time);



// PCB Struct -----------------------------------------------------------------------------------------------------------------
struct pcb
{
    int* esp;
    int pid;
}__attribute__((packed));
typedef struct pcb pcb_t;

// PCB Global Variavles
pcb_t* Running;
uint32_t stack[5][1024];
int numOfProcesses = 0;
int tempPID = 0;
int currStack = 0;



// Queue Struct ---------------------------------------------------------------------------------------------------------------
struct pcbq
{
    pcb_t* pcb_queue[10];
    uint8_t i;
    uint8_t start;
    uint8_t end;
}__attribute__((packed));
typedef struct pcbq pcbq_t;

pcbq_t queue;

void enqueue(pcbq_t* q, pcb_t* pcb)
{
    q->pcb_queue[q->end] = pcb;
    q->end++;
    q->end = q->end%10;
    q->i++;
}
pcb_t* dequeue(pcbq_t* q)
{
    pcb_t* tempProc = q->pcb_queue[q->start];
    q->start++;
    q->start = q->start%10;
    q->i--;
    return tempProc;
}



// IDT Struct -----------------------------------------------------------------------------------------------------------------
struct idt_entry
{
    uint16_t base_low16;
    uint16_t selector;
    uint8_t  always0;
    uint8_t  access;
    uint16_t base_hi16;
} __attribute__((packed));    
typedef struct idt_entry idt_entry_t;

idt_entry_t idt[256];

// Used for interrupt descriptor table
void init_idt_entry(idt_entry_t *entry, uint32_t base, uint16_t selector, uint8_t access)
{
    entry->base_low16 = (uint16_t)(base & 0xFFFF);
    entry->base_hi16 = (uint16_t)((base >> 16) & 0xFFFF);
    
    entry->selector = selector;
    entry->always0 = 0;
    entry->access = access;
}

// Prints the default error message
void defaultHandler()
{
    char errorMessage[30] = "Default Handler Error";
    k_print(errorMessage, 30, 0, 0);
    while(1){}
}

struct idtr
{
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));
typedef struct idtr idtr_t;

idtr_t idtr;

extern void lidtr(idtr_t* idtr);

void init_idt()
{
    int entries = 0;
    uint8_t access = 0x10;
    uint16_t selector = 0x8e;
    
    // Set 0 through 31st entries to the default error handler
    for (entries = 0; entries < 32; entries++)
    {
        init_idt_entry(&idt[entries], (uint32_t)&defaultHandler, access, selector);
    }

    // Set 32nd entry to the dispatcher
    init_idt_entry(&idt[entries], (uint32_t)&dispatcher, access, selector);

    // Set rest of the entries to 0
    for (entries = 33; entries < 256; entries++)
    {
        init_idt_entry(&idt[entries], 0, access, selector);
    }

    // Initialize idtr and call lidtr
    idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtr.base = idt;
    lidtr(&idtr);

    return;
}



// Allocation Functions -------------------------------------------------------------------------------------------------------
uint32_t* allocateStack()
{
    return stack[currStack++];
}

pcb_t allocatePCBArray[10];

pcb_t* allocatePCB()
{
    return &allocatePCBArray[tempPID++];
}



// Create Process Function ----------------------------------------------------------------------------------------------------
int create_process(uint32_t processEntry)
{
    // Check if no more PCBs are available (max 10) or if no stack can be allocated (max 5)
    if (numOfProcesses >= 10 || currStack >= 5)
    {
        return 1; 
    }
    
    // Set stack pointer
    uint32_t* stackPointer = allocateStack();
    uint32_t* st = stackPointer + 1024;
    
    // Set up stack
    st--;
    *st = 0x200;
    st--;
    *st = 16;
    st--;
    *st = processEntry;

    for (int i = 0; i < 8; i++)
    {
        st--;
        *st = 0; 
    }

    for (int i = 0; i < 4; i++)
    {
        st--;
        *st = 8; 
    }

    // Set up PCB variables, increate numOfProcesses, enqueue
    pcb_t* tempProcess = allocatePCB();
    tempProcess->esp = st;
    numOfProcesses++;
    tempProcess->pid = tempPID;
    enqueue(&queue, tempProcess);
}



// Converts an int to a C style string
int intToCString(uint32_t inputNum, char inputBuffer[]) 
{
    if (inputNum == 0)
    {
        inputBuffer[0] = '0';
        inputBuffer[1] = '\0';
        return 0;
    } else {
        int tempNum = intToCString((inputNum / 10), inputBuffer);
        inputBuffer[tempNum] = inputNum % 10 + '0';
        inputBuffer[tempNum + 1] = '\0';
        return tempNum + 1;
    }
}


// These variables are used in all process functions and thought it would be more efficient to have them use 1 instead of initialize 2 variables per function
// Also I couldn't figure out how to append an int to a C string so I printed it after the process message instead
char intBuffer[3] = {'0'};
char* msg;

void p1()
{
    int i = 0;
    
    while(1)
    {
        msg = "Process P1: ";
        k_print(msg, 12, 5, 0);

        intToCString(i, intBuffer);
        k_print(intBuffer, 3, 5, 13);

        i = ((i+1)%500);
    }
}

void p2()
{
    int i = 0;
    
    while(1)
    {
        msg = "Process P2: ";
        k_print(msg, 12, 6, 0);

        intToCString(i, intBuffer);
        k_print(intBuffer, 3, 6, 13);

        i = ((i+1)%500);
    }
}

void p3()
{
    int i = 0;
    
    while(1)
    {
        msg = "Process P3: ";
        k_print(msg, 12, 7, 0);

        intToCString(i, intBuffer);
        k_print(intBuffer, 3, 7, 13);

        i = ((i+1)%500);
    }
}

void p4()
{
    int i = 0;
    
    while(1)
    {
        msg = "Process P4: ";
        k_print(msg, 12, 8, 0);

        intToCString(i, intBuffer);
        k_print(intBuffer, 3, 8, 13);

        i = ((i+1)%500);
    }
}

void p5()
{
    int i = 0;
    
    while(1)
    {
        msg = "Process P5: ";
        k_print(msg, 12, 9, 0);

        intToCString(i, intBuffer);
        k_print(intBuffer, 3, 9, 13);

        i = ((i+1)%500);
    }
}



// Main -----------------------------------------------------------------------------------------------------------------------
int main()
{
    //Two steps done in assignment 1
    k_clearscr();
    char beginMessage[17] = "Running processes";
    k_print(beginMessage, 17, 0, 0);

    //Initialize IDT
    init_idt();

    //Initialize queue
    queue.start = 0;
    queue.end = 0;

    //Initialize timer device
    init_timer_dev(50);

    //Set up the PIC
    setup_PIC();

    //Create and error check processes
    int retval = create_process(&p1); if (retval == 0)
    {
        while (1)
        {
            char abc[60] = "An error has occurred (Process 1 has failed to be created).";
            k_print(abc, 60, 0, 0);
        }
    }
    retval = create_process(&p2); if (retval == 0)
    {
        while (1)
        {
            char abc[60] = "An error has occurred (Process 2 has failed to be created).";
            k_print(abc, 60, 0, 0);
        }
    }
    retval = create_process(&p3); if (retval == 0)
    {
        while (1)
        {
            char abc[60] = "An error has occurred (Process 3 has failed to be created).";
            k_print(abc, 60, 0, 0);
        }
    }
    retval = create_process(&p4); if (retval == 0)
    {
        while (1)
        {
            char abc[60] = "An error has occurred (Process 4 has failed to be created).";
            k_print(abc, 60, 0, 0);
        }
    }
    retval = create_process(&p5); if (retval == 0)
    {
        while (1)
        {
            char abc[60] = "An error has occurred (Process 5 has failed to be created).";
            k_print(abc, 60, 0, 0);
        }
    }

    //Begin first process
    go();

    while(1){}
}




// Given setup_PIC function
void setup_PIC()
{
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);

    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);

    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);

    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);

    outportb(0x21, 0xfe);
    outportb(0xA1, 0xff);
}











// Program 1 Functions --------------------------------------------------------------------------------------------------------
// Loops through the rows and columns to replace everything on the screen with blank strings
void k_clearscr()
{
    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            k_print("", 1, i, j);
        }
    }
    return;
}

// Prints a border around the screen
void print_border(int start_row, int start_col, int end_row, int end_col)
{
    int i = 0;

    k_print("+", 1, 0, 0);

    for (i = 1; i < 79; i++)
    {
        k_print("-", 1, 0, i);
    }

    k_print("+", 1, 0, 79);

    for (i = 1; i < 24; i++)
    {
       k_print("|", 1, i, 0); 
       k_print("|", 1, i, 79); 
    }

    k_print("+", 1, 24, 0);

    for (i = 1; i < 79; i++)
    {
        k_print("-", 1, 24, i);
    }

    k_print("+", 1, 24, 79);
    return;
}