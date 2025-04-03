#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

off_t getFileSize(FILE*);
char* getNLines(FILE* f, char* mem, const int top, int* endPtr, const int N);
int readIntoMem(FILE* f, char* mem, int* endPtr, const int N);



int main(int argc, char** argv){
    if(argc < 2){
        printf("Usage: %s <filenames> ...", argv[0]);
        argv[1] = "less.c"; //hardcoded override for debug mode
        //return 1;
    }

    FILE* f;
    
    int i = 1;
    
    //initialize a file
    f = fopen(argv[i], "r");
    if(f == NULL){
        printf("Error: Could not open file '%s'.", argv[i]);
        return 1;
    }

    //get file size
    long size = getFileSize(f);
    //pointer to our file in memory
    char* mem = malloc(size * sizeof(char));    //should be size bytes
    int top = 0; //used to point to the top of the screen.
    int end = 0; //points to the first unwritten byte in mem (where to save the next read bit)

    //initialize curses interface
    initscr();
    cbreak();   //use raw() to prevent ^C from sending interrupt signal.
    noecho();
    keypad(stdscr, TRUE);
    //loop
    int ch = '\0';
    while(ch != 'q'){
        //read an arbitrary length of bytes into mem
        //readIntoMem(f, mem, &end, 1024);
        //print y-1 lines of it to the screen
        int x, y;
        getmaxyx(stdscr, y, x);
        char* screen = getNLines(f, mem, top, &end, y-1);
        mvprintw(0, 0, "%s:", screen);
        //now that we've printed it, we can free screen
        refresh();
        free(screen);

        //handle input loop
        while((ch = getch() ) != 'q'){
            //go backwards a line
            if(ch == KEY_UP){
                //move top backwards until it hits zero, or until mem[top - 1] is a newline
                while(top > 0 && mem[--top - 1] != '\n');
                break;  //break so we can go do something with these updated values
            }
            //move forwards a line
            else if(ch == KEY_DOWN){
                //move forward until we run into the size of the file, or until mem[top - 1] is a newline (we post increment, so that's why it's -1)
                while(top < size && mem[++top - 1] != '\n');
                break;  //break so we can go do something with these updated values
            }
        }
    }
    free(mem);
    endwin();

    return 0;
}

/*
 * Allocates and returns a pointer to a string that contains N lines (or less if it hit EOF) from mem. 
 * If mem needed to be read further, the function will handle it itself.
 * NOTE: this will cap out at 20KiB of text per screen
*/
char* getNLines(FILE* f, char* mem, const int top, int* endPtr, const int N){
    const int blockSize = 1024;
    int lines = 0;

    int i = 0;
    char* screen = malloc(20 * 1024);   //20KiB
    //loop until: (we hit line count) or (we find an EOF) or (we exceed 20KiB)
    while(lines < N && i < 20 * 1024 && screen[i] != EOF){
        //if we hit the end of space, we need to read more bytes
        if(i >= *endPtr){
            readIntoMem(f, mem, endPtr, blockSize);
        }
        //read next byte from mem onto the screen
        screen[i] = mem[i + top];
        //if the current byte is a '\n', increment number of lines
        if(screen[i] == '\n'){
            ++lines;
        }
        //EOF also increments lines, but end the loop afterwards
        if(screen[i] == EOF){
            ++lines;
            break;
        }
        ++i;
    }
    return screen;
}

/*
 * reads N bytes from f into mem, and updates the index pointed to by endPtr. 
 * endPtr should be the first empty byte, and mem should have sufficient space.
 * readIntoMem() won't necessarily always write N bytes, for instance, if it reaches EOF first.
*/
int readIntoMem(FILE* f, char* mem, int* endPtr, const int N){
    int c, i = 0;
    char ch;
    //read bytes onto mem until EOF or we've read the specified number of bytes
    while((ch = c = getc(f) ) != EOF && i < N){
        //set the byte at end position of mem to c
        mem[*endPtr] = c;
        //increment the value pointed to by endPtr
        (*endPtr)++;
        //increment the number of bytes read so far
        ++i;
    }
}

//returns a pointer to a string which contains the specified length of chars from mem, using the indexes top and end. 
//returns NULL if mem needs to be read further first.
//sets the value pointed to by bytes to the number of bytes contained in the associated string.
char* OLD_getNLines(const int numLines, char* mem, long top, long end, long* bytes){
    long i = top;
    int lines = 0;
    int c;
    *bytes = 0;
    for(i = top; (lines < numLines && i < end); ++i){
        //if we exceed what has been read, we need to go back and read more into the buffer
        if(i >= end){
            return NULL;
        }
        //increment value pointed to by bytes for each new character read
        ++(*bytes);

        //increment line count whenever we hit a newline character
        if(mem[i] == '\n'){
            ++lines;
        }
        //if we hit an EOF, stop the count, don't return NULL
        if(mem[i] == '\n'){
            ++lines;
            break;
        }
    }
    //bytes total is just how many bytes were read.
    *bytes = i - top;
    
}

//returns the size in bytes of a file pointed to by f
off_t getFileSize(FILE* f){
    struct stat buf;
    int fd = fileno(f);
    fstat(fd, &buf);
    return buf.st_size;

}