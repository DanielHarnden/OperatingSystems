#include <stdio.h>

void k_print(char *string, int string_length, int row, int col);
void k_clearscr();
void print_border(int start_row, int start_col, int end_row, int end_col);

// Calls the functions to print "OS 4100" and a border around the screen
int main()
{
    // Clears the screen
    k_clearscr();
    
    // Stores the message "OS 4100" and the length of the message, then calls k_print in kernel.asm
    char message[10] = "OS 4100";
    k_print(message, 7, 12, 36);
    
    // Calls print_border with the appropriate arguments
    print_border(0, 0, 25, 80);
    while(1){}
}

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