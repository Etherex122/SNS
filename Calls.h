#ifndef CALLS_H
#define CALLS_H

#include "Types.h"
#include <stdio.h>


void init_tokenize_arrays();
Token Get_Next_Token();
void Set_Next_Token();
void consume(Token_Name name);
NODE* expression();
NODE* Parser();
void Transpilate(NODE *tree, char *segment);
void flush_codegen(FILE *out);
void init_codegen(void);
void finalize_codegen(FILE *out);
NODE* specyficators_evaluation(Token_Name specyficator);
void print_ast(NODE* node, int depth);
void Precompile();
void LoadFile(const char *filename, long int* ssize, char **code);
void syntax_arrays_init();
void syntax_tokens_array_init();

#endif