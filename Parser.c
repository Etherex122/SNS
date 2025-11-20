#include "Calls.h"
#include <stdio.h>
#include <stdlib.h>

Syntax_arrays arrays[INSTRUCTIONS_NUMBER];
Syntax_tokens_array Syntax_tokens;
int Instruction_agree_number = 0;

void consume(Token_Name name){
    if(current_token.name == name){
        Set_Next_Token();
        current_token = next_token;
    }
    else{
        // printf("nieoczekiwany token %d ", current_token.name);
    }
}


/*
        Parser Biegnący:
        Operuje na tablicach składniowych czyli porównywaniu tokenów i wykonywaniu specyfikatorów

*/

void syntax_arrays_init(){
    arrays[0].elements_per_array = 2;
    arrays[0].Syntax_array = malloc(sizeof(Token_Name) * arrays[0].elements_per_array);
    arrays[0].Syntax_array[0] = expr;
    arrays[0].Syntax_array[1] = SEMICOLON;

    arrays[1].elements_per_array = 3;
    arrays[1].Syntax_array = malloc(sizeof(Token_Name) * arrays[1].elements_per_array);
    arrays[1].Syntax_array[0] = type;
    arrays[1].Syntax_array[1] = IDENTIFIER;
    arrays[1].Syntax_array[2] = SEMICOLON;

    arrays[2].elements_per_array = 4;
    arrays[2].Syntax_array = malloc(sizeof(Token_Name) * arrays[2].elements_per_array);
    arrays[2].Syntax_array[0] = WRITE;
    arrays[2].Syntax_array[1] = COLON;
    arrays[2].Syntax_array[2] = prdata;
    arrays[2].Syntax_array[3] = SEMICOLON;

    arrays[3].elements_per_array = 5;
    arrays[3].Syntax_array = malloc(sizeof(Token_Name) * arrays[3].elements_per_array);
    arrays[3].Syntax_array[0] = type;
    arrays[3].Syntax_array[1] = IDENTIFIER;
    arrays[3].Syntax_array[2] = COLON;
    arrays[3].Syntax_array[3] = prdata;
    arrays[3].Syntax_array[4] = SEMICOLON;

    arrays[4].elements_per_array = 4;
    arrays[4].Syntax_array = malloc(sizeof(Token_Name) * arrays[4].elements_per_array);
    arrays[4].Syntax_array[0] = expr;
    arrays[4].Syntax_array[1] = COLON;
    arrays[4].Syntax_array[2] = prdata;
    arrays[4].Syntax_array[3] = SEMICOLON;

    arrays[5].elements_per_array = 7;
    arrays[5].Syntax_array = malloc(sizeof(Token_Name) * arrays[5].elements_per_array);
    arrays[5].Syntax_array[0] = ex_type;
    arrays[5].Syntax_array[1] = type;
    arrays[5].Syntax_array[2] = IDENTIFIER;
    arrays[5].Syntax_array[3] = LBRACKET;
    arrays[5].Syntax_array[4] = expr;
    arrays[5].Syntax_array[5] = RBRACKET;
    arrays[5].Syntax_array[6] = SEMICOLON;

    arrays[6].elements_per_array = 7;
    arrays[6].Syntax_array = malloc(sizeof(Token_Name) * arrays[6].elements_per_array);
    arrays[6].Syntax_array[0] = IDENTIFIER;
    arrays[6].Syntax_array[1] = LBRACKET;
    arrays[6].Syntax_array[2] = expr;
    arrays[6].Syntax_array[3] = RBRACKET;
    arrays[6].Syntax_array[4] = COLON;
    arrays[6].Syntax_array[5] = prdata;
    arrays[6].Syntax_array[6] = SEMICOLON;

    arrays[7].elements_per_array = 7;
    arrays[7].Syntax_array = malloc(sizeof(Token_Name) * arrays[7].elements_per_array);
    arrays[7].Syntax_array[0] = IF;
    arrays[7].Syntax_array[1] = LPAREN;
    arrays[7].Syntax_array[2] = expr;
    arrays[7].Syntax_array[3] = RPAREN;
    arrays[7].Syntax_array[4] = CLBRACKET;
    arrays[7].Syntax_array[5] = code;
    arrays[7].Syntax_array[6] = CRBRACKET;

    arrays[8].elements_per_array = 7;
    arrays[8].Syntax_array = malloc(sizeof(Token_Name) * arrays[8].elements_per_array);
    arrays[8].Syntax_array[0] = LOOP;
    arrays[8].Syntax_array[1] = LPAREN;
    arrays[8].Syntax_array[2] = expr;
    arrays[8].Syntax_array[3] = RPAREN;
    arrays[8].Syntax_array[4] = CLBRACKET;
    arrays[8].Syntax_array[5] = code;
    arrays[8].Syntax_array[6] = CRBRACKET;

    arrays[9].elements_per_array = 4;
    arrays[9].Syntax_array = malloc(sizeof(Token_Name) * arrays[9].elements_per_array);
    arrays[9].Syntax_array[0] = RETURN;
    arrays[9].Syntax_array[1] = COLON;
    arrays[9].Syntax_array[2] = prdata;
    arrays[9].Syntax_array[3] = SEMICOLON;

    arrays[10].elements_per_array = 9;
    arrays[10].Syntax_array = malloc(sizeof(Token_Name) * arrays[10].elements_per_array);
    arrays[10].Syntax_array[0] = FUNCTION;
    arrays[10].Syntax_array[1] = type;
    arrays[10].Syntax_array[2] = IDENTIFIER;
    arrays[10].Syntax_array[3] = LPAREN;
    arrays[10].Syntax_array[4] = args;
    arrays[10].Syntax_array[5] = RPAREN;
    arrays[10].Syntax_array[6] = CLBRACKET;
    arrays[10].Syntax_array[7] = code;
    arrays[10].Syntax_array[8] = CRBRACKET;

    arrays[11].elements_per_array = 7;
    arrays[11].Syntax_array = malloc(sizeof(Token_Name) * arrays[11].elements_per_array);
    arrays[11].Syntax_array[0] = TURN;
    arrays[11].Syntax_array[1] = COLON;
    arrays[11].Syntax_array[2] = IDENTIFIER;
    arrays[11].Syntax_array[3] = LPAREN;
    arrays[11].Syntax_array[4] = argse;
    arrays[11].Syntax_array[5] = RPAREN;
    arrays[11].Syntax_array[6] = SEMICOLON;

    arrays[12].elements_per_array = 4;
    arrays[12].Syntax_array = malloc(sizeof(Token_Name) * arrays[12].elements_per_array);
    arrays[12].Syntax_array[0] = READ;
    arrays[12].Syntax_array[1] = COLON;
    arrays[12].Syntax_array[2] = IDENTIFIER;
    arrays[12].Syntax_array[3] = SEMICOLON;

    arrays[13].elements_per_array = 4;
    arrays[13].Syntax_array = malloc(sizeof(Token_Name) * arrays[13].elements_per_array);
    arrays[13].Syntax_array[0] = VIR;
    arrays[13].Syntax_array[1] = COLON;
    arrays[13].Syntax_array[2] = IDENTIFIER;
    arrays[13].Syntax_array[3] = SEMICOLON;

    arrays[14].elements_per_array = 7;
    arrays[14].Syntax_array = malloc(sizeof(Token_Name) * arrays[14].elements_per_array);
    arrays[14].Syntax_array[0] = ELFI;
    arrays[14].Syntax_array[1] = LPAREN;
    arrays[14].Syntax_array[2] = expr;
    arrays[14].Syntax_array[3] = RPAREN;
    arrays[14].Syntax_array[4] = CLBRACKET;
    arrays[14].Syntax_array[5] = code;
    arrays[14].Syntax_array[6] = CRBRACKET;

    arrays[15].elements_per_array = 4;
    arrays[15].Syntax_array = malloc(sizeof(Token_Name) * arrays[15].elements_per_array);
    arrays[15].Syntax_array[0] = ELSE;
    arrays[15].Syntax_array[1] = CLBRACKET;
    arrays[15].Syntax_array[2] = code;
    arrays[15].Syntax_array[3] = CRBRACKET;

    arrays[16].elements_per_array = 4;
    arrays[16].Syntax_array = malloc(sizeof(Token_Name) * arrays[16].elements_per_array);
    arrays[16].Syntax_array[0] = EXTERN;
    arrays[16].Syntax_array[1] = COLON;
    arrays[16].Syntax_array[2] = IDENTIFIER;
    arrays[16].Syntax_array[3] = SEMICOLON;

    arrays[17].elements_per_array = 4;
    arrays[17].Syntax_array = malloc(sizeof(Token_Name) * arrays[17].elements_per_array);
    arrays[17].Syntax_array[0] = SEXTERN;
    arrays[17].Syntax_array[1] = COLON;
    arrays[17].Syntax_array[2] = IDENTIFIER;
    arrays[17].Syntax_array[3] = SEMICOLON;

    arrays[18].elements_per_array = 4;
    arrays[18].Syntax_array = malloc(sizeof(Token_Name) * arrays[18].elements_per_array);
    arrays[18].Syntax_array[0] = SSET;
    arrays[18].Syntax_array[1] = COLON;
    arrays[18].Syntax_array[2] = IDENTIFIER;
    arrays[18].Syntax_array[3] = SEMICOLON;

    arrays[19].elements_per_array = 2;
    arrays[19].Syntax_array = malloc(sizeof(Token_Name) * arrays[19].elements_per_array);
    arrays[19].Syntax_array[0] = SBACK;
    arrays[19].Syntax_array[1] = SEMICOLON;

    arrays[20].elements_per_array = 6;
    arrays[20].Syntax_array = malloc(sizeof(Token_Name) * arrays[20].elements_per_array);
    arrays[20].Syntax_array[0] = SAVE;
    arrays[20].Syntax_array[1] = COLON;
    arrays[20].Syntax_array[2] = IDENTIFIER;
    arrays[20].Syntax_array[3] = COMMA;
    arrays[20].Syntax_array[4] = IDENTIFIER;
    arrays[20].Syntax_array[5] = SEMICOLON;

}

void syntax_tokens_array_init(){
    Syntax_tokens.tokens[0] = SEMICOLON;
    Syntax_tokens.tokens[1] = COLON;
    Syntax_tokens.tokens[2] = LBRACKET;
    Syntax_tokens.tokens[3] = RBRACKET;
    Syntax_tokens.tokens[4] = CLBRACKET;
    Syntax_tokens.tokens[5] = CRBRACKET;
    Syntax_tokens.tokens[6] = LPAREN;
    Syntax_tokens.tokens[7] = RPAREN;
    Syntax_tokens.tokens[8] = COMMA;
}

bool is_specyficator(Token_Name specyficator){
    bool is = (specyficator == expr || specyficator == prdata || specyficator == type || specyficator == ex_type || specyficator == code || specyficator == args || specyficator == argse) ?  true : false;

    return is;
}

bool check_agree(){
    bool agree = false;
    int token_cout_save = token_cout;
    Token current_token_save = current_token;
    Token next_token_save = next_token;

    for(int i = INSTRUCTIONS_NUMBER - 1; i >= 0; i--){
        agree = true;
        // printf("INSTRUKCJA AKTU %d\n", i);

        for(int j = 0; j < arrays[i].elements_per_array; j++){

            if(current_token.name == arrays[i].Syntax_array[j]){
                // printf("CHECK AGREE true 1 if\n");
                consume(current_token.name);
                continue;
            }

            if(is_specyficator(arrays[i].Syntax_array[j])){
                // printf("CHECK AGREE true 2 if\n");
                specyficators_evaluation(arrays[i].Syntax_array[j]);
                continue;
            }
            agree = false;
            break;
        }

        token_cout = token_cout_save;
        current_token = current_token_save;
        next_token = next_token_save;
        if(agree){
            // printf("CHECK AGREE est agree instrukcja agree %d\n", i);
            Instruction_agree_number = i;
            return agree;
        }
    }

    return agree;
}

NODE* new_node(Token_Name type, int value, char *text, NODE* left, NODE* right){
    NODE* node = calloc(1, sizeof(NODE));
    node->type = type;
    node->value = value;
    node->Text = text;
    node->left = left;
    node->right = right;

    return node;
}

NODE* char_parse(){
    NODE* node = NULL;
    node = new_node(CHAR, 0, current_token.Text, NULL, NULL);
    consume(CHAR);
    // printf("char jest\n");
    return node;
}

NODE* logic();

NODE* factor() {
    NODE* node = NULL;

    // printf("TOKEN FACTOR  %d value %d\n", current_token.name, current_token.value);
    if(current_token.name == BNOT){
        node = new_node(current_token.name, current_token.value, current_token.Text, NULL, NULL);
        consume(current_token.name);
        node->right = logic();
    }
    if(current_token.name == SUB){
        node = new_node(NEG, current_token.value, current_token.Text, NULL, NULL);
        consume(current_token.name);
        node->right = logic();
    }
    if (current_token.name == NUMBER) {
        node = new_node(current_token.name, current_token.value, current_token.Text, NULL, NULL);
        consume(NUMBER); 
    }
    else if(current_token.name == TRUE || current_token.name == FALSE){
        // printf("BOOL");
        node = new_node(current_token.name, current_token.value, NULL, NULL, NULL);
        consume(current_token.name);
    }
    else if(current_token.name == FLOAT){
        node = new_node(FLOAT, current_token.value, current_token.Text, NULL, NULL);
        node->value_float = current_token.value_float;
        consume(FLOAT); 
    }
    else if(current_token.name == ADR || current_token.name == MUL || current_token.name == POW){
        node = new_node((current_token.name == MUL) ? IADR : (current_token.name == POW) ? IADR : current_token.name, current_token.value, current_token.Text, NULL, (current_token.name == POW) ? new_node(IADR, current_token.value, NULL, NULL, NULL) : NULL);
        NODE* temp = node;
        if(current_token.name == POW){
            temp = temp->right;
        }
        consume(current_token.name);
        while(current_token.name == MUL || current_token.name == POW){
            if(current_token.name == MUL) temp->right = new_node(IADR, current_token.value, current_token.Text, NULL, NULL);
            else if(current_token.name == POW){
                temp->right = new_node(IADR, current_token.value, current_token.Text, NULL, NULL);
                temp = temp->right;
                temp->right = new_node(IADR, current_token.value, current_token.Text, NULL, NULL);
            }
            temp = temp->right;
            consume(current_token.name);
        }
        temp->right = new_node(current_token.name, current_token.value, current_token.Text, NULL, NULL);
        consume(current_token.name);
        if(current_token.name == NDIV){
            temp = temp->right;
            temp->right = new_node(NDIV, current_token.value, current_token.Text, NULL, NULL);
            consume(current_token.name);
            temp = temp->right;
            temp->right = new_node(current_token.name, current_token.value, current_token.Text, NULL, NULL);
            consume(current_token.name);
        }
        if(current_token.name == COLON){
            consume(current_token.name);
            temp = temp->right;
            temp->right = logic();
        }
    }
    else if(current_token.name == IDENTIFIER){
        node = new_node(current_token.name, current_token.value, current_token.Text, NULL, NULL);
        consume(IDENTIFIER);
        if(current_token.name == LBRACKET){
            consume(LBRACKET);
            node->right = expression();
            consume(RBRACKET);
        }
        else if(current_token.name == LPAREN){
            consume(LPAREN);
            node->right = specyficators_evaluation(argse);
            consume(RPAREN);
        }
    }
    else if(current_token.name == CHAR){
        node = char_parse();
    }
    else if (current_token.name == LPAREN) {  
        consume(LPAREN);  
        node = expression();  

        if (current_token.name == RPAREN) {  
            consume(RPAREN);
        } else {
        }
    }
    else {
    }

    if(current_token.name == INC || current_token.name == DEC){
        node = new_node(current_token.name, 0, NULL, NULL, node);
        consume(current_token.name);
    }
    
    return node;
}

NODE* expo(){
    NODE* node = factor();
    // printf("TOKEN EXPO %d value %d\n", current_token.name, current_token.value);

    if(current_token.name == POW || current_token.name == DEPOW) {
        Token_Name op = current_token.name;
        consume(op);  

        NODE* right = factor();  
        node = new_node(op, 0, NULL, node, right);  
    }

    return node;
}

NODE* term() {
    NODE* node = expo(); 
    // printf("TOKEN TERM %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == MUL || current_token.name == DIV || current_token.name == MOD || current_token.name == VECTORE_MUL || current_token.name == VECTORE_DIV) {
        Token_Name op = current_token.name;
        consume(op);  

        NODE* right = expo();  
        node = new_node(op, 0, NULL, node, right);  
    }

    return node;
}

NODE* expression() {
    NODE* node = term();  
    //  printf("TOKEN EXPR %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == ADD || current_token.name == SUB || current_token.name == VECTORE_ADD || current_token.name == VECTORE_SUB) {
        Token_Name op = current_token.name;
        consume(op);  
                
        NODE* right = term();  
        node = new_node(op, 0, NULL, node, right);  
    }

    return node;
}

NODE* bex(){
    NODE* node = expression();  
    // printf("TOKEN LOGIC %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == SHR || current_token.name == SHL) {
        Token_Name op = current_token.name;
        consume(op);  
                
        NODE* right = expression();  
        node = new_node(op, 0, NULL, node, right);
    }

    return node;
}

NODE* logic2(){
    NODE* node = bex();  
    // printf("TOKEN LOGIC %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == JL || current_token.name == JG) {
        Token_Name op = current_token.name;
        consume(op);  
                
        NODE* right = bex();  
        node = new_node(op, 0, NULL, node, right);
    }

    return node;
}

NODE* logic1(){
    NODE* node = logic2();  
    // printf("TOKEN LOGIC %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == EQUAL || current_token.name == NEQUAL) {
        Token_Name op = current_token.name;
        consume(op);  
                
        NODE* right = logic2();  
        node = new_node(op, 0, NULL, node, right);
    }

    return node;
}

NODE* logic0(){
    NODE* node = logic1();  
    // printf("TOKEN LOGIC %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == BAND || current_token.name == BOR || current_token.name == XOR) {
        Token_Name op = current_token.name;
        consume(op);  
                
        NODE* right = logic1();  
        node = new_node(op, 0, NULL, node, right);
    }

    return node;
}

NODE* logic(){
    NODE* node = logic0();  
    // printf("TOKEN LOGIC %d value %d\n", current_token.name, current_token.value);

    while (current_token.name == AND || current_token.name == OR) {
        Token_Name op = current_token.name;
        consume(op);  
                
        NODE* right = logic0();  
        node = new_node(op, 0, NULL, node, right);
    }

    return node;
}


NODE* strings_parse(){
    NODE* node = NULL;
    node = new_node(STRING, 0, current_token.Text, NULL, NULL);
    consume(STRING);
    // printf("string jest\n");
    return node;
}

NODE* type_parse(){
    // printf("TYPE PARSE");
    switch(current_token.name){
        case _INT:
            // printf("INTCIK");
            consume(current_token.name);
            return new_node(_INT, 0, NULL, NULL, NULL);
        case _FLOAT:
            // printf("FLOACIK");
            consume(current_token.name);
            return new_node(_FLOAT, 0, NULL, NULL, NULL);
        case _BOOL:
            // printf("BOOLCIK");
            int valuen = current_token.value;
            consume(current_token.name);
            return new_node(_BOOL, valuen, NULL, NULL, NULL);
        case _CHAR:
            // printf("CHARCHIK");
            char *Text = current_token.Text;
            consume(current_token.name);
            return new_node(_CHAR, 0, Text, NULL, NULL);
        case _VOID:
            // printf("VOIDCIK");
            consume(current_token.name);
            return new_node(_VOID, 0, NULL, NULL, NULL);
        case _PTR:
            // printf("PTRCIK");
            consume(current_token.name);
            return new_node(_PTR, 0, NULL, NULL, NULL);
        case _SS:
            consume(current_token.name);
            return new_node(_SS, 0, NULL, NULL, NULL);
        case _STACK:
            consume(current_token.name);
            return new_node(_STACK, 0, NULL, NULL, NULL);
        default:
            break;
    }
    return NULL;
}

NODE* ex_type_parse(){
    // printf("EX_TYPE PARSE");
    switch(current_token.name){
        case _ARRAY:
            // printf("ARRAYCIK");
            consume(current_token.name);
            return new_node(_ARRAY, 0, NULL, NULL, NULL);
        case _CONST:
            // printf("CONSTEREK");
            consume(current_token.name);
            return new_node(_CONST, 0, NULL, NULL, NULL);
        default:
            break;
    }
}

int RSLPC = 0;
int RSLPCLP = 0;

NODE* specyficators_evaluation(Token_Name specyficator){
    // printf("evaluacja specyfikatorów\n");
    NODE* node = NULL;
    if(specyficator == type){
        // printf("TYPE");
        node = type_parse();
        return node;
    }
    if(specyficator == ex_type){
        // printf("EX_TYPE");
        node = ex_type_parse();
        return node;
    }
    if(specyficator == expr){
        // printf("EXPR");
        node = logic();
        return node;
    }
    if(specyficator == prdata){
        // printf("specyfikator prdata\n");

        if(current_token.name == STRING){
            // printf("wykryto string\n");
            node = strings_parse();
            // printf("string? %s \n", node->Text);
        }
        else{
            node = logic();
        }

        return node;
    }
    if(specyficator == code){
        // printf("specyficator CODE");
        NODE* node = new_node(code, 0, NULL, Parser(), NULL);
        NODE* temp = node;
        
        while(current_token.name != CRBRACKET){
            // printf("TOKEN %d CODE\n", current_token.name);
            temp->right = new_node(code, 0, NULL, Parser(), NULL);
            temp = temp->right;
            // printf("TOKEN %d CODE\n", current_token.name);
            // printf("\n\nRSLPC: %d\n\n", RSLPC);
            if(current_token.name == CRBRACKET && RSLPC == 1) RSLPC = 0;
            if(current_token.name == CRBRACKET && RSLPC > 1){
                RSLPC--;
                consume(CRBRACKET);
            }
        }
        
        // print_ast(node, 1);
        return node;
    }

    if(specyficator == args){
        // printf("specyficator args");
        NODE* node = new_node(args, 0, NULL, NULL, NULL);
        NODE* temp = node;
        NODE *tmp = temp;

        // printf("\n\n\n\n\n\nETRO %d %d\n\n\n\n\n\n\n", current_token.name, next_token.name);
        while(current_token.name != RPAREN){
            consume(COMMA);
            tmp = temp;
            temp->left = specyficators_evaluation(type);
            temp = temp->left;
            temp->left = logic();
            temp = tmp;
            temp->right = new_node(args, 0, NULL, NULL, NULL);
            temp = temp->right;
            // printf("\n\n\n\n\n\nETRO %d %d\n\n\n\n\n\n\n", current_token.name, next_token.name);
        }

        // print_ast(node, 1);
        return node;
    }
    
    if(specyficator == argse){
        // printf("specyficator argse");
        NODE* node = new_node(argse, 0, NULL, NULL, NULL);
        NODE* temp = node;

        // printf("\n\n\n\n\n\nITRO %d %d\n\n\n\n\n\n\n", current_token.name, next_token.name);

        while(current_token.name != RPAREN){
            consume(COMMA);
            temp->left = specyficators_evaluation(prdata);
            temp->right = new_node(argse, 0, NULL, NULL, NULL);
            temp = temp->right;
            // printf("\n\n\n\n\n\nITRO\n\n\n\n\n\n\n");
            if(current_token.name == RPAREN && RSLPCLP == 1) RSLPCLP = 0;
            if(current_token.name == RPAREN && RSLPCLP > 1){
                RSLPCLP--;
                consume(RPAREN);
            }
        }

        // print_ast(node, 1);
        return node;
    }
    
    return node;
}

bool is_token_syntax(Token_Name name, int index_) {
    bool agree = false;

    for (int i = 0; i < SYNTAX_TOKENS_NUMBER; i++) {
        if (current_token.name == Syntax_tokens.tokens[i] && 
            arrays[Instruction_agree_number].Syntax_array[index_] == current_token.name) {
            
            agree = true;
            consume(current_token.name); 
            break;
        }
        else if (i == SYNTAX_TOKENS_NUMBER - 1) {
        }
    }

    return agree;
}

void Error_(){

}

NODE* Parser(){
    NODE* node = NULL;
    NODE* current_node = NULL;
    
    if(check_agree()){
        for(int i = 0; i < arrays[Instruction_agree_number].elements_per_array; i++){
            // printf("%d %d\n", current_token.name, arrays[Instruction_agree_number].Syntax_array[i]);
            if(current_token.name == arrays[Instruction_agree_number].Syntax_array[i]){
                // printf("instrukcja token\n");

                if(current_token.name == CLBRACKET) RSLPC++;
                if(current_token.name == LPAREN) RSLPCLP++;
                if(current_token.name == RPAREN) RSLPCLP--;

                if(is_token_syntax(current_token.name, i)){
                    // printf("instruckja syntax token*\n");
                    continue;
                } 
                
                NODE* node_ = new_node(current_token.name, 0, current_token.Text, NULL, NULL);
                consume(current_token.name);

                if(node == NULL){
                    node = node_;
                    current_node = node;
                }
                else{
                    current_node->right = node_;
                    current_node = node_;
                }
                
            }

            else if(is_specyficator(arrays[Instruction_agree_number].Syntax_array[i])){
                // printf("instruckja specyfikator\n");
                NODE* node_ = specyficators_evaluation(arrays[Instruction_agree_number].Syntax_array[i]);

                if (node == NULL) {
                    node = node_;
                    current_node = node_;
                    while(current_node != NULL && current_node->right != NULL) {
                        current_node = current_node->right;
                    }
                } else {
                    current_node->right = node_;
                    while (current_node != NULL && current_node->right != NULL) {
                        current_node = current_node->right;
                    }
                }
                
            }
        }   
    }

    NODE* temp = node;
    while(temp != NULL){
        // printf("type %d text %s", temp->type, temp->Text);
        temp = temp->right;
    }

    return node;
}