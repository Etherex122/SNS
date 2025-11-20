#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Calls.h"


void print_ast(NODE *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) printf("  ");
    printf("Node type: %d", node->type);
    if (node->Text) printf(", text: %s", node->Text);
    if (node->type == NUMBER) printf(", value: %d", node->value);
    if (node->type == FLOAT) printf(", value: %f", node->value_float);
    printf("\n");
    print_ast(node->left, depth + 1);
    print_ast(node->right, depth + 1);
} 

void free_node(NODE *node){
    if(node == NULL){
        return;
    }
    free_node(node->left);
    free_node(node->right);
    free(node->Text);
    free(node->left);
    free(node->right);
}

void LoadFile(const char *filename, long int* ssize, char **code) {
    FILE *file = fopen(filename, "rb");
    if (!file) return;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    *code = malloc(size + 1);
    if (!*code) {
        fclose(file);
        return;
    }

    fread(*code, 1, size, file);  
    (*code)[size] = '\0';        

    *ssize = size;  

    fclose(file);
}

VecOpsM VecOps_mode = SSE2;
bool Portable = false;

void Optimalization_Flag(int argc, char *argv[]){
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-VecOps:AVX-2", 14) == 0) {
            VecOps_mode = AVX2;
        }
        if (strncmp(argv[i], "-Portable", 9) == 0) {
            Portable = true;
        }
    }
}   

int main(int argc, char *argv[]){
    LoadFile(argv[1], &Code_size, &Code);

    Optimalization_Flag(argc, argv);

    Precompile();
    // printf("%s", Code);
    init_tokenize_arrays();
    Set_Next_Token();
    current_token = next_token;

    syntax_arrays_init();
    syntax_tokens_array_init();

    init_codegen();

    NODE *node = NULL;

    do {
        while(current_token.name == CRBRACKET){
            // printf("\n\nN token? %d %d\n\n", current_token.name, next_token.name);
            consume(current_token.name);
        }
        node = Parser();
        // print_ast(node, 1);
        Transpilate(node, "main");
        free_node(node);
    } while (node != NULL);

    FILE *f = fopen(argv[2], "w");
    if (!f) {
        perror("Nie można otworzyć pliku do zapisu");
        return 1;
    }
    finalize_codegen(f);
    fclose(f);

    return 0;
}