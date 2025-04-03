#include <stdio.h>

int main(int argc, char** argv){
    //no file given
    if (argc < 2){
        //print character by character, quitting before printing EOF
        int c;
        while((c = getchar()) != EOF){
            putchar(c);
        }
        return 0;
    }

    //file(s) provided

    //iterate through each file
    for(int i = 1; i < argc; ++i){
        //initialize file pointer for reading
        FILE* f = fopen(argv[i], "r");
        
        //can't open file error
        if(f == NULL){
            fprintf(stderr, "%s: couldn't open file '%s'.\n", argv[0], argv[i]);
            return 1;
        }
        
        //print character by character, quitting before printing EOF
        int c;
        while((c = fgetc(f)) != EOF){
            putchar(c);
        }
    }
}