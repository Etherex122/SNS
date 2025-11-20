#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
 
#define INSTRUCTIONS_NUMBER 21
#define SYNTAX_TOKENS_NUMBER 9
#define NUM_TOKENS 83

typedef enum{
    NUMBER, ADD, SUB, DIV, MUL, NEW_LINE, POW, END, STRING, WRITE, FLOAT, SEMICOLON, RPAREN, LPAREN,
    _INT, IDENTIFIER, prdata, expr, COLON, type, REL, READ, MOD, ALM, _LOGIC, _BOOL, TRUE, FALSE, value, _CHAR, CHAR, _FLOAT,
    LBRACKET, RBRACKET, ex_type, _ARRAY, _CONST, CRBRACKET, CLBRACKET, IF, code, EQUAL, LOOP, BREAK, FUNCTION, _VOID, RETURN, TURN, COMMA, args,
    NEQUAL, _PTR, VECTORE_ADD, VECTORE_SUB, VECTORE_MUL, VECTORE_DIV, argse, DEPOW, VIR, ELSE, ELFI, EXTERN, ADR, IADR, JL, JG, M_CHAR, M_INT, M_FLOAT, M_PTR,
    NDIV, _SS, SEXTERN, PE, EQL, SSET, SBACK, _STACK, SHL, SHR, BAND, BOR, AND, OR, XOR, BNOT, NEG, INC, DEC, MSADD, MSSUB, SAVE
} Token_Name;

typedef struct{
    Token_Name name;
    int value;
    float value_float;
    char *Text;
} Token;

typedef struct{
    const char *Tokens[NUM_TOKENS];
    Token_Name  names[NUM_TOKENS];
} Tokenize_Arrays;


typedef struct NODE{
    Token_Name type;
    int value;
    float value_float;
    char *Text;
    struct NODE *left;
    struct NODE *right;
} NODE;

typedef struct{
    Token_Name *Syntax_array;
    int elements_per_array;
} Syntax_arrays;

typedef struct{
    Token_Name tokens[SYNTAX_TOKENS_NUMBER];
} Syntax_tokens_array;

typedef enum{
    DINT, COUNTER, LONG_INT, DCONST_INT, DCHAR, DCONST_CHAR, DBOOL, DARRAY_INT, DARRAY_BOOL, DARRAY_CHAR,
    DPTR, DFLOAT
} DTYPE;

typedef enum{
    EAX, EBX, EDX, ESI, EDI, ECX, R8D, R9D, R10D, R11D, R12D, R13D, R14D, R15D
} REGISTER;

typedef enum{
    AVX2, SSE2
} VecOpsM;

typedef struct REG_NODE{
    REGISTER reg;
    struct REG_NODE *left;
    struct REG_NODE *right;
} REG_NODE;

extern REGISTER Registers[14];
extern int token_cout;
extern int line_cout;
extern Token current_token;
extern Token next_token;
extern char *Code;
extern long int Code_size;
extern VecOpsM VecOps_mode;
extern bool Portable;

#endif