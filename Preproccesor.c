#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Calls.h"

void shift_chars(size_t size, char *buffer, size_t i){
    Code = realloc(Code, Code_size + size);
    if (!Code) return;

    memmove(Code + i + size, Code + i, Code_size - i);

    memcpy(Code + i, buffer, size);

    Code_size += size;
}

int pre_include(){
    // printf("\n\nINCLUDE\n\n");
    size_t i = 0;
    bool is_pre = false;

    while(i + 1 < Code_size && (Code[i] != '\\' || Code[i+1] != 'I')) i++;
    if(i + 1 >= Code_size || Code[i] != '\\' || Code[i+1] != 'I') {
        return -1;
    }
    is_pre = true;

    while(Code[i] != '<' && is_pre && i < Code_size){
        Code[i] = ' ';
        i++;
    }
    Code[i] = ' ';
    i++;
    
    char filename[256];
    int l = 0;

    while(Code[i] != '>' && l < 255 && i < Code_size && is_pre){
        filename[l] = Code[i];
        Code[i] = ' ';
        i++;
        l++;
    }
    
    filename[255] = '\0';

    if(is_pre){
        Code[i] = ' ';

        char *buffer;
        long int chars;

        LoadFile(filename, &chars, &buffer);

        shift_chars(chars, buffer, i);
    }
    else{
        return -1;
    }

    return 0;
}

void Precompile(){
    int end = 0;
    while(end != -1){
        end = pre_include();
    }
}