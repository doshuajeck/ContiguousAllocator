/****************************************
	* Author: Joshua Deck
	* Assignment Number: 4
	* Date of Submission: 9/15/21
	* Name of this file: ContiguousAllocator.c
	* Description of the program: 
	* 	A contiguous memory allocator
 ****************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


// LinkedList structure, instances, and associated functions
struct Node
{
    int available;
    int start;
    int end;
    struct Node *next;
    char pid[100];
};

// Instances of Node struct used to implement LL
struct Node *head; 
struct Node *memorySpace; 
struct Node *temp;
struct Node *temp_alt; // alt temp needed for list updating
struct Node *replacement;

// Function declarations
int size();
void firstFit(char[], int);
void bestFit(char[], int);
void worstFit(char[], int);
void requestMemory(char[], int, char[]);
void releaseMemory(char[]);
void flushData();
void compact();
void status();





// Returns the size of the list
int size()
{
    temp = head;
    int count = 0;

    if (temp -> next == NULL)
    {
        return count;
    }

    while (temp -> next != NULL)
    {
        temp = temp -> next;
        count++;
    }

    return count;
}

// Necessary globals
char request[3];
char process[3];
char algorithm[2];
int lastAddress;
int space;





// Routes input from the CLI to a particular algorithm
void requestMemory(char pid[3], int space, char algo[2])
{
    if (strcmp("W", algo) == 0)
        worstFit(pid, space);
    else if (strcmp("B", algo) == 0)
        bestFit(pid, space);
    else if (strcmp("F", algo) == 0)
        firstFit(pid, space);
}





// Releases the memory associated with a particular PID
void releaseMemory(char in_pid[3])
{
    temp = head;
    int space;

    // Edge case handling for no used processes
    if (size() <= 1)
    {
        printf("Cannot remove unused data. Release request cancelled.\n");
        return;
    }

    // Edge case handling for one used process
    if (size() == 2)
    {
        space = head -> next -> available;
        head -> next = head -> next -> next;
        head -> next -> start = head -> next -> start - (space + 1);
        head -> next -> next = NULL;
        printf("Memory released from process %s.\n", in_pid);
        return;
    }

    // Cycle to the correct segment
    while (temp -> next != NULL && strcmp(temp -> next -> pid, in_pid) != 0)
    {
        temp = temp -> next;
    }

    // If the segment we found is the last segment in the list, just get rid of it
    if (temp -> next -> next == NULL) // Short list with only one used process
    {
        // Update previous node to point to NULL
        head -> next= NULL;

        // Add the space back to the Unused segment of memory
        space = temp -> next -> available; 
        head -> next -> available = head -> available + space;
    }
    else if (temp -> next -> next -> next == NULL)
    {
        // Update previous node to point to NULL
        temp -> next -> next == NULL;
        head -> next = temp;

        // Add the space back to the Unused segment of memory
        space = temp -> next -> next -> available; 
        head -> next -> available = head -> available + space;
    }
    else // Otherwise, we need to set the previous node's next to the current node's next
    {
        // Set the previous node's next to this one's next
        temp -> next = temp -> next -> next;

        // Update space
        space = temp -> next -> next -> available;
        head -> next -> available = head -> available + space;
    }
    printf("Memory released from process %s.\n", in_pid);
}





// Used in compaction; ensures special cases are handled
void flushData()
{
    temp = head -> next;

    // Traverse the list to check for special cases
    while (temp -> next != NULL)
    {
        temp = temp -> next;
        if (temp -> start > temp -> end) // If start value somehow exceeds end value
        {
            releaseMemory(temp -> pid);
        }
    }

}



// Removes all holes in segments and adds them to Unused segment
void compact()
{
    int total = 0;
    temp = head;

    // Calculate open space
    while (temp -> next != NULL)
    {
        temp = temp -> next;        
        total += temp -> available;
        temp -> end = temp -> end - temp -> available;      
    }

    // Add open space to the Unused segment
    temp = head;
    while (temp -> next != NULL)
    {
        temp = temp -> next;
        if (strcmp(temp -> pid, "Unused") == 0)
        {
            printf("Total: %i", total);
            temp -> end = total;
        }
    }

    flushData();
}





// Prints information about each process
void status()
{
    temp = head;
    while (temp -> next != NULL)
    {
        printf("Addresses [%d : %d] Process %s\n", temp -> next -> start, temp -> next -> end, temp -> next -> pid);
        temp = temp -> next;
    }
}




/*
 Allocation algorithms 
   -firstFit
   -bestFit
   -worstFit
*/
void firstFit(char pid[3], int space)
{
    temp = head;
    int remainingSpace = 0;

    while (temp -> next != NULL)
    {
        if (strcmp(temp -> next -> pid, "Unused") == 0 && temp -> next -> available >= space)
        {
            head -> available = head -> available - space;

            strcpy(temp -> next -> pid, pid);
            temp -> next -> end = temp -> next -> start + space;


            remainingSpace = temp -> next -> available - space;
            if (remainingSpace > 0)
            {
                struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
                strcpy(newNode -> pid, "Unused");
                newNode -> available = remainingSpace;
                temp -> next -> available = space;
                newNode -> start = temp -> next -> end + 1; 
                newNode -> end = newNode -> start + remainingSpace;
                if (newNode -> end > lastAddress) 
                    newNode -> end = lastAddress;
                newNode -> next = temp -> next -> next;
                temp -> next -> next = newNode;
            }

            return; 
        }
        else 
            temp = temp -> next;
    }

    printf("There is no space to place process %s, of %dkb\n", pid, space);
}





void bestFit(char pid[3], int space)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    int smallest = 1215752191; // Initialize to max value so it gets quickly overwritten
    int remainingSpace = 0;
    temp = head;

    if (size() != 1)
    {
        while (temp != NULL)
        {
            if (temp -> available >= space)
            {
                // Check if the current Node is smaller than the current smallest
                
                if (temp -> available <= smallest)
                    smallest = temp -> available;
            }
            temp = temp -> next;
        }

        temp = head -> next;       
        while (temp -> next != NULL)
        {
            // If this is the smallest space we found earlier, we found the target
            if (temp -> next -> available == smallest)
            {            
                head -> available = head -> available - space;
                strcpy(temp -> next -> pid, pid);
                temp -> next -> end = temp -> next -> start + space;

                // Make a node using the remaining space
                remainingSpace = temp -> next -> available - space;                
                if (remainingSpace > 0)
                {                
                    // Assign the proper info the Node and attach it to the list
                    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));     
                                     
                    temp -> next -> next = newNode;               
                    strcpy(newNode -> pid, pid);          
                    newNode -> available = remainingSpace;                
                    temp -> available = space;                
                    newNode -> start = temp -> next -> end; 
                    newNode -> end = newNode -> start + remainingSpace;
                    if (newNode -> end > lastAddress) 
                        newNode -> end = lastAddress;
                    
                    
                    temp -> next -> next = newNode;
                }
                return; // No more is left, so we return
            }
            temp = temp -> next;            
        }
    }
    else 
    {
        temp = head -> next;
        smallest = temp -> available;

        // Assign the proper info the Node and attach it to the list
        struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));     
                        
        strcpy(newNode -> pid, pid);          
        newNode -> available = space;
        temp -> start = temp -> start + space;
        temp -> next = newNode;
        newNode -> start = 0; 
        newNode -> end = space;
        if (newNode -> end > lastAddress) 
            newNode -> end = lastAddress;
        
        
        temp -> next = newNode;
        return;
    }

    

    // If this is reached, then not enough space is left
    printf("Not enough remaining space for process %s.\n", pid); 
}





void worstFit(char pid[3], int space)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    int largest = -1015752191; // Initialize to min value so it gets quickly overwritten
    int remainingSpace = 0;
    temp = head;

    while (temp -> next != NULL)
    {
        if (temp -> available >= space)
        {
            // Update max space if needed
            if (temp -> available >= largest)
                largest = temp -> next -> available;
        }
        temp = temp -> next;
    }

    temp = head;
    while (temp -> next != NULL)
    {
        // Checks to see if this is the largest open space
        if (largest - temp -> available <= 500) // Slight range to allow for statistical error
        {
            // Update remaining space
            head -> available = head -> available - space;

            // If there is still space, make a new node with that remaining size
            remainingSpace = temp -> available - space;
            if (remainingSpace > 0)
            {
                // Configure new node
                struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
                strcpy(newNode -> pid, pid);
                newNode -> available = space;
                newNode -> start = 0;
                newNode -> end = space;

                // Adjust segment being inserted into
                temp -> start = temp -> start + space;

                // If bigger than the initial space,
                if (newNode -> end > lastAddress) 
                    newNode -> end = lastAddress;
                newNode -> next = temp -> next -> next;
                temp -> next -> next = newNode;
            }

            return; // No more is left, so we return
        }
        else
            temp = temp -> next;
    }

    // If this is reached, then not enough space is left
    printf("Not enough remaining space for process %s.\n", pid); 
}





int main(int argc, char *argv[])
{

    int initial = atoi(argv[1]);
    char user_input[128];

    printf("allocator>");
    fflush(stdout);

    if (read(0, user_input, 128) < 0) // Just to confirm the initial input is fine :)
        printf("ERROR: An error occured during input parsing.");

    head = (struct Node *)malloc(sizeof(struct Node)); // Head node based on the Node structure outlined above
    memorySpace = (struct Node *)malloc(sizeof(struct Node));

    strcpy(head -> pid, "default");
    head -> start = -1;
    head -> end = -1;
    head -> available = initial;
    head -> next = memorySpace;

    strcpy(memorySpace -> pid, "Unused");
    memorySpace -> start = 0;
    memorySpace -> end = initial;
    memorySpace -> available = memorySpace -> end - memorySpace -> start;
    memorySpace -> next = NULL;

    lastAddress = initial;

    // Allocator functionality
    while (1 == 1)
    {
        if ((sscanf(user_input, "%s %s", request, process)) < 0)
            printf("ERROR: Invalid input.");

        // Handles command input
        if (strcmp("RQ", request) == 0) 
        {
            sscanf(user_input, "%s %s %d %s", request, process, &space, algorithm);
            requestMemory(process, space, algorithm);
        }
        else if (strcmp("RL", request) == 0) 
        {

            sscanf(user_input, "%s %s", request, process);
            releaseMemory(process);
        }
        else if (strcmp("C", request) == 0) 
        {
            compact();
        }
        else if (strcmp("STAT", request) == 0) 
        {
            status();
        }
        else if (strcmp("SIZE", request) == 0)
        {
            printf("%i\n", size());
        }
        // Set up the CLI impersonation
        printf("allocator>");
        fflush(stdout);

        if (read(0, user_input, 128) < 0)
            printf("ERROR: Invalid input.");
    }
}