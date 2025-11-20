#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "Calls.h"

Tokenize_Arrays arrs;
int token_cout = 0;
int line_cout = 0;
Token current_token;
Token next_token;
char* Code;
long int Code_size = 0;


/*
        Uzywamy podejscia porównywania w Get Next token korzystając z tablic zamiast wielu case w switchu.
        pozwala to na mniej kodu mniejszą złozonosc itp  
*/

void init_tokenize_arrays(){
    static const char * toks[NUM_TOKENS] = {
        "Write",
        "_INT",
        "<+>",
        "<->",
        "<*>",
        "</>",
        ";",
        ":",
        "\n",
        "\r",
        "\"",
        NULL,
        NULL,
        "Rel",
        "**",
        "%",
        "All_Load_Memory",
        "_Logic",
        "True",
        "False",
        "_BOOL",
        "_CHAR",
        NULL,
        "[",
        "]",
        "_Array",
        "If",
        "==",
        "{",
        "}",
        "(",
        ")",
        "Loop",
        "Break",
        "Function",
        "_VOID",
        "Return",
        "Turn",
        ",",
        "!=",
        "_PTR",
        "_FLOAT",
        "Read",
        NULL,
        "//",
        "*",
        "/",
        "Vir",
        "Else",
        "Elfi",
        "Extern",
        "@",
        "*",
        ">>",
        "<<",
        ">",
        "<",
        "char",
        "int",
        "float",
        "$ptr",
        "\\",
        "_SS",
        "Sextern",
        "?",
        "=",
        "Sset",
        "Sback",
        "_STACK",
        "&&",
        "||",
        "&",
        "|",
        "^",
        "~",
        "++",
        "--",
        "+",
        "-",
        "-=",
        "+=",
        "save"
    };

    static Token_Name names[NUM_TOKENS] = {
        WRITE,
        _INT,
        VECTORE_ADD,
        VECTORE_SUB,
        VECTORE_MUL, 
        VECTORE_DIV,
        SEMICOLON,
        COLON,
        NEW_LINE,
        END,
        STRING,
        IDENTIFIER,
        NUMBER,
        REL,
        POW,
        MOD,
        ALM,
        _LOGIC,
        TRUE,
        FALSE,
        _BOOL,
        _CHAR,
        CHAR,
        LBRACKET,
        RBRACKET,
        _ARRAY,
        IF,
        EQUAL,
        CLBRACKET,
        CRBRACKET,
        LPAREN,
        RPAREN,
        LOOP,
        BREAK,
        FUNCTION,
        _VOID,
        RETURN,
        TURN,
        COMMA,
        NEQUAL,
        _PTR,
        _FLOAT,
        READ,
        FLOAT,
        DEPOW,
        MUL,
        DIV,
        VIR,
        ELSE,
        ELFI,
        EXTERN,
        ADR,
        IADR,
        SHR,
        SHL,
        JG,
        JL,
        M_CHAR,
        M_INT,
        M_FLOAT,
        M_PTR,
        NDIV,
        _SS,
        SEXTERN,
        PE,
        EQL,
        SSET,
        SBACK,
        _STACK,
        AND,
        OR,
        BAND,
        BOR,
        XOR,
        BNOT,
        INC,
        DEC,
        ADD,
        SUB,
        MSSUB,
        MSADD,
        SAVE
    };

    for(int i = 0; i < NUM_TOKENS; i++){
        arrs.Tokens[i] = toks[i];
        arrs.names[i]  = names[i];
    }
}


Token New_Token_init(int id_name, char *text, int value){
    Token Temp_Token;
    Temp_Token.name = arrs.names[id_name];
    Temp_Token.Text = text;
    Temp_Token.value = value;

    return Temp_Token;
}


Token Get_Next_Token(){
    Token New_Token;
    char current_char = Code[token_cout];
    // printf("Current token_cout: %d, char: '%c' (0x%x)\n", token_cout, current_char, current_char);

    if (current_char == '\0') {
        return New_Token_init(END, NULL, 0);
    }

    while (isspace(current_char)) {
        if (current_char == '\n') {
            New_Token = New_Token_init(4, NULL, 0);
            line_cout++;
        }
        token_cout++;
        current_char = Code[token_cout];
    }

    if(current_char == '?' && Code[token_cout+1] == '?'){
        token_cout += 3;
        while(Code[token_cout-2] != '?' && Code[token_cout-1] != '?'){
            token_cout++;
        }
    }

    if(current_char == '?'){
        do {
            token_cout++;
            current_char = Code[token_cout];
        } while(current_char != '\n' && current_char != '\0');

        if(current_char == '\n'){
            token_cout++;
            line_cout++;

            return Get_Next_Token();
        }

        token_cout -= 2;

        return Get_Next_Token();
    }


    for(int i=0; i<NUM_TOKENS; i++){
        if(arrs.Tokens[i] == NULL) continue;
        size_t len = strlen(arrs.Tokens[i]);
        if(strncmp(&Code[token_cout], arrs.Tokens[i], len) == 0){
            New_Token = New_Token_init(i, NULL, 0);
            token_cout += len;
            // printf("Token %d name %d\n", i, New_Token.name);
            if(New_Token.name == TRUE || New_Token.name == FALSE) New_Token.value = (New_Token.name == TRUE) ? 1 : 0;
            // printf("Token value %d\n", New_Token.value);
            // printf("TOKEN NAME %d\n", New_Token.name);
            return New_Token;
        }
    }

    if (isdigit(current_char)) {
        int start_pos = token_cout;
        int has_dot = 0;

        while (isdigit(Code[token_cout]) || (Code[token_cout] == '.' && !has_dot)) {
            if (Code[token_cout] == '.') {
                has_dot = 1;
            }
            token_cout++;
        }

        int len = token_cout - start_pos;
        char *num_str = malloc(len + 1);
        strncpy(num_str, &Code[start_pos], len);
        num_str[len] = '\0';

        Token New_Token;
        if (has_dot) {
            float val = strtof(num_str, NULL);
            New_Token = New_Token_init(43, NULL, 0);
            New_Token.value_float = val;
        } else {
            // int
            int val = atoi(num_str);
            New_Token = New_Token_init(12, NULL, val);
        }
        
        free(num_str);
        return New_Token;
    }

    if(current_char == '"'){
        char *string = malloc(sizeof(char) * 256);
        token_cout++;
        int i = 0;
        while(Code[token_cout] != '"'){
            string[i] = Code[token_cout];
            i++;
            token_cout++;
        }
        New_Token = New_Token_init(7, string, 0);
        token_cout++;
        return New_Token;
    }

    if(current_char == '\''){
        char *ch = malloc(sizeof(char) * 1);
        token_cout++;
        int i = 0;
        while(Code[token_cout] != '\''){
            ch[i] = Code[token_cout];
            i++;
            token_cout++;
        }
        New_Token = New_Token_init(22, ch, 0);
        token_cout++;
        return New_Token;
    }

    if(isalpha(Code[token_cout]) || Code[token_cout]=='_'){
        char buf[256];
        int i = 0;
        while(isalnum(Code[token_cout]) || Code[token_cout]=='_'){
            buf[i++] = Code[token_cout++];
        }
        buf[i] = '\0';
        // printf("token name %d", arrs.names[11]);
        return New_Token_init(11, strdup(buf), 0);
    }
    
    token_cout++;
    return New_Token;
}

void Set_Next_Token(){
    next_token = Get_Next_Token();
}
