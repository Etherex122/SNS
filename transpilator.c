// transpilator.c
#include "Calls.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


bool Error_flag = false;

bool pow_use = false;
bool depow_use = false;

bool write_int = false;

bool cdwsse = false;
bool fswsse8 = false;


REGISTER Registers[14] = {EAX, EBX, EDX, ESI, EDI, ECX, R8D, R9D, R10D, R11D, R12D, R13D, R14D, R15D};

REG_NODE* regs_tree = NULL;
bool long_ = false;
bool float_r = false;
bool char_r = false;
REG_NODE* regs_node = NULL;

char *get_reg_string(REGISTER reg){
  if(long_){
    switch(reg){
      case EAX:
        return "rax";
      case EBX:
        return "rbx";
      case EDX:
        return "rdx";
      case ESI:
        return "rsi";
      case EDI:
        return "rdi";
      case ECX:
        return "rcx";
      case R8D:
        return "r8";
      case R9D:
        return "r9";
      case R10D:
        return "r10";
      case R11D:
        return "r11";
      case R12D:
        return "r12";
      case R13D:
        return "r13";
      case R14D:
        return "r14";
      case R15D:
        return "r15";
    }
  }
  if(char_r){
    switch(reg){
      case EAX:
        return "al";
      case EBX:
        return "bl";
      case EDX:
        return "dl";
      case ESI:
        return "sil";
      case EDI:
        return "dil";
      case ECX:
        return "cl";
      case R8D:
        return "r8b";
      case R9D:
        return "r9b";
      case R10D:
        return "r10b";
      case R11D:
        return "r11b";
      case R12D:
        return "r12b";
      case R13D:
        return "r13b";
      case R14D:
        return "r14b";
      case R15D:
        return "r15b";
    }
  }
  if(float_r){
    switch(reg){
      case EAX:
        return "xmm0";
      case EBX:
        return "xmm1";
      case EDX:
        return "xmm2";
      case ESI:
        return "xmm3";
      case EDI:
        return "xmm4";
      case ECX:
        return "xmm5";
      case R8D:
        return "xmm6";
      case R9D:
        return "xmm7";
    }
  }
  else{
    switch(reg){
      case EAX:
        return "eax";
      case EBX:
        return "ebx";
      case EDX:
        return "edx";
      case ESI:
        return "esi";
      case EDI:
        return "edi";
      case ECX:
        return "ecx";
      case R8D:
        return "r8d";
      case R9D:
        return "r9d";
      case R10D:
        return "r10d";
      case R11D:
        return "r11d";
      case R12D:
        return "r12d";
      case R13D:
        return "r13d";
      case R14D:
        return "r14d";
      case R15D:
        return "r15d";
    }
  }

  return "NULL";
}

struct REG_NODE* new_reg_node(REGISTER reg, struct REG_NODE *left, struct REG_NODE *right){
  struct REG_NODE* node = malloc(sizeof(REG_NODE));
  node->reg = reg;
  node->left = left;
  node->right = right;

  return node;
}

/*

5 * 5 + 5 * 5 - 5 ** 5

5 ** 5:
eax ebx = eax

5 * 5:
ebx edx = ebx

5 * 5:
edx esi = edx

5 + 5:
ebx edx = ebx

5 - 5:
eax ebx  = eax 
*/

struct REG_NODE *building_regs_tree(NODE *ast, int iregs){
  struct REG_NODE *node = NULL;

  if(ast != NULL){
    node = new_reg_node(Registers[iregs], building_regs_tree(ast->right, iregs + 1), building_regs_tree(ast->left, iregs));
  }

  return node;
}


void print_regs_tree(struct REG_NODE *tree, int depth){
  if(tree != NULL){
    for (int i = 0; i < depth; i++) printf("  ");
    printf("%s\n", get_reg_string(tree->reg));
    print_regs_tree(tree->right, depth + 1);
    print_regs_tree(tree->left, depth + 1);
  }
}

#define INITIAL_BUFFER_SIZE 8192

typedef struct {
    char   *name;  
    char   *buffer;
    size_t  size; 
    size_t  pos; 
    unsigned long long offset;
    unsigned long long stack_N;
    unsigned long long freebytes_number;
    bool *free_bytes;
} CodeSegment;

static CodeSegment *segments       = NULL;
static size_t        segment_count = 0;
static size_t        segment_capacity = 0;

static CodeSegment *find_segment(char *name) {
    for (size_t i=0; i<segment_count; i++)
        if (strcmp(segments[i].name, name)==0)
            return &segments[i];
    return NULL;
}

void create_segment(char *name, int offset) {
    if (find_segment(name)) return;
    if (segment_count == segment_capacity) {
        segment_capacity = segment_capacity ? segment_capacity*2 : 4;
        segments = realloc(segments, segment_capacity * sizeof *segments);
        if (!segments) { perror("realloc segments"); exit(1); }
    }
    CodeSegment *seg = &segments[segment_count++];
    seg->name   = strdup(name);
    seg->size   = INITIAL_BUFFER_SIZE;
    seg->buffer = malloc(seg->size);
    seg->pos    = 0;
    seg->offset = offset;
    seg->stack_N = 0;
    seg->freebytes_number = 1000;
    seg->free_bytes = malloc(1000 * sizeof(bool));
    for(int i = 0; i < seg->freebytes_number; i++){
      seg->free_bytes[i] = true;
    }
    if (!seg->name || !seg->buffer) {
        perror("alloc segment");
        exit(1);
    }
}

void append_code(char *segment_name, const char *fmt, ...) {
    CodeSegment *seg = find_segment(segment_name);
    if (!seg) {
        fprintf(stderr, "append_code: nieznany segment '%s'\n", segment_name);
        exit(1);
    }
    va_list args;
    while (1) {
        va_start(args, fmt);
        int wr = vsnprintf(seg->buffer + seg->pos, seg->size - seg->pos, fmt, args);
        va_end(args);
        if (wr < 0) { perror("vsnprintf"); exit(1); }
        if ((size_t)wr >= seg->size - seg->pos) {
            seg->size *= 2;
            seg->buffer = realloc(seg->buffer, seg->size);
            if (!seg->buffer) { perror("realloc buffer"); exit(1); }
            continue;
        }
        seg->pos += wr;
        break;
    }
}

static const char *fallback_seg(const char *name) {
    if (name[0] != '.') return NULL;
    for (size_t i = 0; i < segment_count; i++) {
        if (strcmp(segments[i].name, name) == 0) {
            for (ssize_t j = (ssize_t)i - 1; j >= 0; j--) {
                if (segments[j].name[0] != '.') {
                    return segments[j].name;
                }
            }
        }
    }
    return NULL;
}

void flush_codegen(FILE *out) {
    size_t i = 0;
    while (i < segment_count) {
      fwrite(segments[i].buffer, 1, segments[i].pos, out);
      free(segments[i].buffer);
      free(segments[i].name);
      i++;
    }
    free(segments);
    segments = NULL;
    segment_count = segment_capacity = 0;
}

void init_offsets();

void init_codegen(void) {
    create_segment("data", 0);
    create_segment("bss", 0);
    create_segment("main", 0);

    append_code("data",
        "section .data\n"
        "    buffer db 256 dup(0)\n"
        "    newline db 10\n"
    );
    append_code("bss",
        "section .bss\n"
    );
    append_code("main",
        "section .text\n"
        "global _start\n\n"
    );
}

void finalize_codegen(FILE *out) {
    // epilog
    create_segment("_start", 0);
    append_code("_start",
        "\n_start:\n"
        "    and rsp, -16\n"
        "    sub rsp, 8\n"
    );
    if(Portable){
      append_code("_start",
        "\n    xor rcx, rcx\n    mov eax, 1\n    cpuid \n"
        "    bt edx, 26\n"
        "    setc al\n"
        "    shl rax, 0\n"
        "    or rcx, rax\n"
        "    mov eax, 7\n    mov ecx, 0\n    cpuid\n"
        "    bt ebx, 5\n"
        "    setc al\n"
        "    shl rax, 1\n"
        "    or rcx, rax\n"
        "    bt ebx, 16\n"
        "    setc al\n"
        "    shl rax, 2\n"
        "    or rcx, rax\n"
        "    push rcx\n"
      );
    }
    append_code("_start",
      "\n    call _main\n\n"
    );

    if(Portable) append_code("_start", "    pop rcx\n\n");

    append_code("_start",
      "    mov rax, 60\n"
      "    xor rdi, rdi\n"
      "    syscall\n\n"
    );

    if (pow_use) {
        create_segment("pow", 0);
        append_code("pow",
            "pow_loop:\n"
            "    imul eax, ebx\n"
            "    dec ecx\n"
            "    jnz pow_loop\n"
            "    ret\n\n"
            "pow_end:\n\n"
        );
    }

    if (depow_use) {
        create_segment("depow", 0);
        append_code("depow",
            "depow:\n"
            "    idiv ebx\n"
            "    dec ecx\n"
            "    jnz depow\n"
            "    ret\n\n"
        );
    }
    if(cdwsse){
      create_segment("cdwsse8", 0);
      append_code("cdwsse8",
          "cdwsse8:\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "\n"
          "    sub rdi, 128\n"
          "\n"
          "    movups [rsi - 16], xmm0\n"
          "    movups [rsi - 32], xmm1\n"
          "    movups [rsi - 48], xmm2\n"
          "    movups [rsi - 64], xmm3\n"
          "    movups [rsi - 80], xmm4\n"
          "    movups [rsi - 96], xmm5\n"
          "    movups [rsi - 112], xmm6\n"
          "    movups [rsi - 128], xmm7\n"
          "\n"
          "    lea rsi, [rsi - 128]\n"
          "\n"
          "    ret\n"
      );
      append_code("cdwsse8",
          "cdwsse_lum64:\n"
          "    cmp rcx, 0\n"
          "    je .LL0\n"
          "    call cdwsse64\n"
          ".LL0:\n"
          "    ret\n"
      );
      append_code("cdwsse8",
          "cdwsse64:\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "    movups xmm8, [rdi - 144]\n"
          "    movups xmm9, [rdi - 160]\n"
          "    movups xmm10, [rdi - 176]\n"
          "    movups xmm11, [rdi - 192]\n"
          "    movups xmm12, [rdi - 208]\n"
          "    movups xmm13, [rdi - 224]\n"
          "    movups xmm14, [rdi - 240]\n"
          "    movups xmm15, [rdi - 256]\n"
          
          "\n"
          "    sub rdi, 256\n"
          "\n"
          "\n"
          "    movups [rsi - 16], xmm0\n"
          "    movups [rsi - 32], xmm1\n"
          "    movups [rsi - 48], xmm2\n"
          "    movups [rsi - 64], xmm3\n"
          "    movups [rsi - 80], xmm4\n"
          "    movups [rsi - 96], xmm5\n"
          "    movups [rsi - 112], xmm6\n"
          "    movups [rsi - 128], xmm7\n"
          "    movups [rsi - 144], xmm8\n"
          "    movups [rsi - 160], xmm9\n"
          "    movups [rsi - 176], xmm10\n"
          "    movups [rsi - 192], xmm11\n"
          "    movups [rsi - 208], xmm12\n"
          "    movups [rsi - 224], xmm13\n"
          "    movups [rsi - 240], xmm14\n"
          "    movups [rsi - 256], xmm15\n"
          "\n"
          "    lea rsi, [rsi - 256]\n"
          "\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "    movups xmm8, [rdi - 144]\n"
          "    movups xmm9, [rdi - 160]\n"
          "    movups xmm10, [rdi - 176]\n"
          "    movups xmm11, [rdi - 192]\n"
          "    movups xmm12, [rdi - 208]\n"
          "    movups xmm13, [rdi - 224]\n"
          "    movups xmm14, [rdi - 240]\n"
          "    movups xmm15, [rdi - 256]\n"
          
          "\n"
          "    sub rdi, 256\n"
          "\n"
          "\n"
          "    movups [rsi - 16], xmm0\n"
          "    movups [rsi - 32], xmm1\n"
          "    movups [rsi - 48], xmm2\n"
          "    movups [rsi - 64], xmm3\n"
          "    movups [rsi - 80], xmm4\n"
          "    movups [rsi - 96], xmm5\n"
          "    movups [rsi - 112], xmm6\n"
          "    movups [rsi - 128], xmm7\n"
          "    movups [rsi - 144], xmm8\n"
          "    movups [rsi - 160], xmm9\n"
          "    movups [rsi - 176], xmm10\n"
          "    movups [rsi - 192], xmm11\n"
          "    movups [rsi - 208], xmm12\n"
          "    movups [rsi - 224], xmm13\n"
          "    movups [rsi - 240], xmm14\n"
          "    movups [rsi - 256], xmm15\n"
          "\n"
          "    lea rsi, [rsi - 256]\n"
          "\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "    movups xmm8, [rdi - 144]\n"
          "    movups xmm9, [rdi - 160]\n"
          "    movups xmm10, [rdi - 176]\n"
          "    movups xmm11, [rdi - 192]\n"
          "    movups xmm12, [rdi - 208]\n"
          "    movups xmm13, [rdi - 224]\n"
          "    movups xmm14, [rdi - 240]\n"
          "    movups xmm15, [rdi - 256]\n"
          
          "\n"
          "    sub rdi, 256\n"
          "\n"
          "\n"
          "    movups [rsi - 16], xmm0\n"
          "    movups [rsi - 32], xmm1\n"
          "    movups [rsi - 48], xmm2\n"
          "    movups [rsi - 64], xmm3\n"
          "    movups [rsi - 80], xmm4\n"
          "    movups [rsi - 96], xmm5\n"
          "    movups [rsi - 112], xmm6\n"
          "    movups [rsi - 128], xmm7\n"
          "    movups [rsi - 144], xmm8\n"
          "    movups [rsi - 160], xmm9\n"
          "    movups [rsi - 176], xmm10\n"
          "    movups [rsi - 192], xmm11\n"
          "    movups [rsi - 208], xmm12\n"
          "    movups [rsi - 224], xmm13\n"
          "    movups [rsi - 240], xmm14\n"
          "    movups [rsi - 256], xmm15\n"
          "\n"
          "    lea rsi, [rsi - 256]\n"
          "\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "    movups xmm8, [rdi - 144]\n"
          "    movups xmm9, [rdi - 160]\n"
          "    movups xmm10, [rdi - 176]\n"
          "    movups xmm11, [rdi - 192]\n"
          "    movups xmm12, [rdi - 208]\n"
          "    movups xmm13, [rdi - 224]\n"
          "    movups xmm14, [rdi - 240]\n"
          "    movups xmm15, [rdi - 256]\n"
          
          "\n"
          "    sub rdi, 256\n"
          "\n"
          "\n"
          "    movups [rsi - 16], xmm0\n"
          "    movups [rsi - 32], xmm1\n"
          "    movups [rsi - 48], xmm2\n"
          "    movups [rsi - 64], xmm3\n"
          "    movups [rsi - 80], xmm4\n"
          "    movups [rsi - 96], xmm5\n"
          "    movups [rsi - 112], xmm6\n"
          "    movups [rsi - 128], xmm7\n"
          "    movups [rsi - 144], xmm8\n"
          "    movups [rsi - 160], xmm9\n"
          "    movups [rsi - 176], xmm10\n"
          "    movups [rsi - 192], xmm11\n"
          "    movups [rsi - 208], xmm12\n"
          "    movups [rsi - 224], xmm13\n"
          "    movups [rsi - 240], xmm14\n"
          "    movups [rsi - 256], xmm15\n"
          "\n"
          "    lea rsi, [rsi - 256]\n"
          "\n"
          "    ret\n"
      );
    }
    if(fswsse8){
      create_segment("fswsse8", 0);
      append_code("fswsse8",
          "afswsse8:\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "\n"
          "    movups xmm8, [rdx - 16]\n"
          "    movups xmm9, [rdx - 32]\n"
          "    movups xmm10, [rdx - 48]\n"
          "    movups xmm11, [rdx - 64]\n"
          "    movups xmm12, [rdx - 80]\n"
          "    movups xmm13, [rdx - 96]\n"
          "    movups xmm14, [rdx - 112]\n"
          "    movups xmm15, [rdx - 128]\n"
          "\n"
          "    addps xmm8, xmm0\n"
          "    addps xmm9, xmm1\n"
          "    addps xmm10, xmm2\n"
          "    addps xmm11, xmm3\n"
          "    addps xmm12, xmm4\n"
          "    addps xmm13, xmm5\n"
          "    addps xmm14, xmm6\n"
          "    addps xmm15, xmm7\n"
          "\n"
          "    movups [rsi - 16], xmm8\n"
          "    movups [rsi - 32], xmm9\n"
          "    movups [rsi - 48], xmm10\n"
          "    movups [rsi - 64], xmm11\n"
          "    movups [rsi - 80], xmm12\n"
          "    movups [rsi - 96], xmm13\n"
          "    movups [rsi - 112], xmm14\n"
          "    movups [rsi - 128], xmm15\n"
          "\n"
          "    sub rdi, 128\n"
          "    lea rdx, [rdx - 128]\n"
          "    sub rsi, 128\n"
          "\n"
          "    ret\n"
      );

      append_code("fswsse8",
          "sfswsse8:\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "\n"
          "    movups xmm8, [rdx - 16]\n"
          "    movups xmm9, [rdx - 32]\n"
          "    movups xmm10, [rdx - 48]\n"
          "    movups xmm11, [rdx - 64]\n"
          "    movups xmm12, [rdx - 80]\n"
          "    movups xmm13, [rdx - 96]\n"
          "    movups xmm14, [rdx - 112]\n"
          "    movups xmm15, [rdx - 128]\n"
          "\n"
          "    subps xmm8, xmm0\n"
          "    subps xmm9, xmm1\n"
          "    subps xmm10, xmm2\n"
          "    subps xmm11, xmm3\n"
          "    subps xmm12, xmm4\n"
          "    subps xmm13, xmm5\n"
          "    subps xmm14, xmm6\n"
          "    subps xmm15, xmm7\n"
          "\n"
          "    movups [rsi - 16], xmm8\n"
          "    movups [rsi - 32], xmm9\n"
          "    movups [rsi - 48], xmm10\n"
          "    movups [rsi - 64], xmm11\n"
          "    movups [rsi - 80], xmm12\n"
          "    movups [rsi - 96], xmm13\n"
          "    movups [rsi - 112], xmm14\n"
          "    movups [rsi - 128], xmm15\n"
          "\n"
          "    sub rdi, 128\n"
          "    lea rdx, [rdx - 128]\n"
          "    sub rsi, 128\n"
          "\n"
          "    ret\n"
      );

      append_code("fswsse8",
          "mfswsse8:\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "\n"
          "    movups xmm8, [rdx - 16]\n"
          "    movups xmm9, [rdx - 32]\n"
          "    movups xmm10, [rdx - 48]\n"
          "    movups xmm11, [rdx - 64]\n"
          "    movups xmm12, [rdx - 80]\n"
          "    movups xmm13, [rdx - 96]\n"
          "    movups xmm14, [rdx - 112]\n"
          "    movups xmm15, [rdx - 128]\n"
          "\n"
          "    mulps xmm8, xmm0\n"
          "    mulps xmm9, xmm1\n"
          "    mulps xmm10, xmm2\n"
          "    mulps xmm11, xmm3\n"
          "    mulps xmm12, xmm4\n"
          "    mulps xmm13, xmm5\n"
          "    mulps xmm14, xmm6\n"
          "    mulps xmm15, xmm7\n"
          "\n"
          "    movups [rsi - 16], xmm8\n"
          "    movups [rsi - 32], xmm9\n"
          "    movups [rsi - 48], xmm10\n"
          "    movups [rsi - 64], xmm11\n"
          "    movups [rsi - 80], xmm12\n"
          "    movups [rsi - 96], xmm13\n"
          "    movups [rsi - 112], xmm14\n"
          "    movups [rsi - 128], xmm15\n"
          "\n"
          "    sub rdi, 128\n"
          "    lea rdx, [rdx - 128]\n"
          "    sub rsi, 128\n"
          "\n"
          "    ret\n"
      );

      append_code("fswsse8",
          "dfswsse8:\n"
          "    movups xmm0, [rdi - 16]\n"
          "    movups xmm1, [rdi - 32]\n"
          "    movups xmm2, [rdi - 48]\n"
          "    movups xmm3, [rdi - 64]\n"
          "    movups xmm4, [rdi - 80]\n"
          "    movups xmm5, [rdi - 96]\n"
          "    movups xmm6, [rdi - 112]\n"
          "    movups xmm7, [rdi - 128]\n"
          "\n"
          "    movups xmm8, [rdx - 16]\n"
          "    movups xmm9, [rdx - 32]\n"
          "    movups xmm10, [rdx - 48]\n"
          "    movups xmm11, [rdx - 64]\n"
          "    movups xmm12, [rdx - 80]\n"
          "    movups xmm13, [rdx - 96]\n"
          "    movups xmm14, [rdx - 112]\n"
          "    movups xmm15, [rdx - 128]\n"
          "\n"
          "    divps xmm8, xmm0\n"
          "    divps xmm9, xmm1\n"
          "    divps xmm10, xmm2\n"
          "    divps xmm11, xmm3\n"
          "    divps xmm12, xmm4\n"
          "    divps xmm13, xmm5\n"
          "    divps xmm14, xmm6\n"
          "    divps xmm15, xmm7\n"
          "\n"
          "    movups [rsi - 16], xmm8\n"
          "    movups [rsi - 32], xmm9\n"
          "    movups [rsi - 48], xmm10\n"
          "    movups [rsi - 64], xmm11\n"
          "    movups [rsi - 80], xmm12\n"
          "    movups [rsi - 96], xmm13\n"
          "    movups [rsi - 112], xmm14\n"
          "    movups [rsi - 128], xmm15\n"
          "\n"
          "    sub rdi, 128\n"
          "    lea rdx, [rdx - 128]\n"
          "    sub rsi, 128\n"
          "\n"
          "    ret\n"
      );

    }
    if (write_int) {
        create_segment("write_int", 0);
        append_code("write_int",
            "; --- write_int routines ---\n"
            "int_to_str:\n"
            "    mov rcx, 10\n"
            "    xor rbx, rbx\n"
            "    mov rdx, 0\n"
            "    cmp rdi, 0\n"
            "    jge .L1\n"
            "    neg rdi\n"
            "    mov byte [rsi], '-'\n"
            "    inc rsi\n"
            ".L1:\n"
            "    mov rax, rdi\n"
            ".L2:\n"
            "    xor rdx, rdx\n"
            "    div rcx\n"
            "    push rdx\n"
            "    inc rbx\n"
            "    test rax, rax\n"
            "    jnz .L2\n"
            ".L3:\n"
            "    pop rax\n"
            "    add al, '0'\n"
            "    mov [rsi], al\n"
            "    inc rsi\n"
            "    dec rbx\n"
            "    jnz .L3\n"
            "    mov byte [rsi], 0\n"
            "    ret\n\n"
            "strlen:\n"
            "    push rdi\n"
            "    xor rax, rax\n"
            ".L4:\n"
            "    cmp byte [rsi + rax], 0\n"
            "    je .L5\n"
            "    inc rax\n"
            "    jmp .L4\n"
            ".L5:\n"
            "    pop rdi\n"
            "    ret\n\n"
            "write_int:\n"
            "    push rsi\n"
            "    mov rsi, buffer\n"
            "    call int_to_str\n"
            "    mov rsi, buffer\n"
            "    call strlen\n"
            "    mov rdx, rax\n"
            "    mov rax, 1\n"
            "    mov rdi, 1\n"
            "    syscall\n"
            "    pop rsi\n"
            "    ret\n\n"
        );
    }


    flush_codegen(out);
}

int goft(DTYPE type);

bool fmuse = false;
bool ss = false;

#define MAX_VARS 1000

typedef struct { 
  char *name;
  int offset;
  int lenght;
  DTYPE type;
  char *owner_seg;
} Symbol;

static Symbol Symbol_Array[MAX_VARS];
char* Ssymbol;
static int symcount = 0; 
int fmuse_half = 0;
int last_fmuse_half = 0;

int Alofr(int roff, CodeSegment *seg, int goft){
    int Aoff = 0;

    for(int i = 0; i < seg->freebytes_number; i++){
      if(seg->free_bytes[i]){
        bool is = true;
        if(i + roff > seg->freebytes_number){
        seg->free_bytes = realloc(seg->free_bytes, (i + roff + 1) * sizeof(bool));
          seg->freebytes_number = (i + roff + 1);
          for(int j = i; j < seg->freebytes_number; j++){
            seg->free_bytes[j] = true;
          }
        }
        for(int j = i; j < i + roff; j++){
          if(!seg->free_bytes[j] || i % goft != 0) is = false;
        }
        if(is){
          Aoff = i + roff;

          for(int j = i; j < i + roff; j++){
            seg->free_bytes[j] = false;
          }

          last_fmuse_half = fmuse_half;

          break;
        }
      }
    }
    
    return -Aoff;
}

void dealloc_var(char *name, char *seg){
  int dtv = 0;

  for (int i = 0; i < symcount; i++)
    if (strcmp(Symbol_Array[i].name, name) == 0 && strcmp(Symbol_Array[i].owner_seg, seg) == 0)
        dtv = i; 

  if (strcmp(Symbol_Array[dtv].name, name) != 0 || strcmp(Symbol_Array[dtv].owner_seg, seg) != 0) return;

  for(int i = abs(Symbol_Array[dtv].offset); i > abs(Symbol_Array[dtv].offset) - Symbol_Array[dtv].lenght; i--){
    find_segment(seg)->free_bytes[i-1] = true;
  }

  Symbol_Array[dtv].name = " ";
  Symbol_Array[dtv].offset = 0;
  Symbol_Array[dtv].lenght = 0;
  Symbol_Array[dtv].owner_seg = " ";
}

int add_variable(char *name, int offs, char *seg, DTYPE type) {
    int aoff = Alofr(offs, find_segment(seg), goft(type));
    Symbol_Array[symcount] = (Symbol){ name, aoff, offs, type, seg};
    symcount++;
    if(abs(aoff) > find_segment(seg)->stack_N){
      // append_code(seg, "    sub rsp, %d\n",abs(aoff) - find_segment(seg)->stack_N);
      find_segment(seg)->stack_N = abs(aoff);
    }

    int aao = (Ssymbol == NULL) ? 0 : find_segment(Ssymbol)->stack_N;
    aao = -aao;
    
    return aoff + aao;
}

int get_variable_offset(const char *name, char *seg) {
    for (int i = 0; i < symcount; i++) {
        if (strcmp(Symbol_Array[i].name, name) == 0
         && strcmp(Symbol_Array[i].owner_seg, seg) == 0)
            return Symbol_Array[i].offset;
    }
    const char *fb = fallback_seg(seg);
    if (fb) {
        for (int i = 0; i < symcount; i++) {
            if (strcmp(Symbol_Array[i].name, name) == 0
             && strcmp(Symbol_Array[i].owner_seg, fb) == 0)
                return Symbol_Array[i].offset;
        }
    }
    if(ss){
      return get_variable_offset(name, Ssymbol);
    }

    return 0;
}

int get_variable_lenght(const char *name, char *seg) {
    for (int i = 0; i < symcount; i++) {
        if (strcmp(Symbol_Array[i].name, name) == 0
         && strcmp(Symbol_Array[i].owner_seg, seg) == 0)
            return Symbol_Array[i].lenght;
    }
    const char *fb = fallback_seg(seg);
    if (fb) {
        for (int i = 0; i < symcount; i++) {
            if (strcmp(Symbol_Array[i].name, name) == 0
             && strcmp(Symbol_Array[i].owner_seg, fb) == 0)
                return Symbol_Array[i].lenght;
        }
    }

    if(ss){
      return get_variable_lenght(name, Ssymbol);
    }

    return -1;
}

DTYPE get_variable_type(const char *name, char *seg) {
    for (int i = 0; i < symcount; i++) {
        if (strcmp(Symbol_Array[i].name, name) == 0
         && strcmp(Symbol_Array[i].owner_seg, seg) == 0)
            return Symbol_Array[i].type;
    }
    const char *fb = fallback_seg(seg);
    if (fb) {
        for (int i = 0; i < symcount; i++) {
            if (strcmp(Symbol_Array[i].name, name) == 0
             && strcmp(Symbol_Array[i].owner_seg, fb) == 0)
                return Symbol_Array[i].type;
        }
    }
    
    if(ss){
      return get_variable_type(name, Ssymbol);
    }

    return (DTYPE)-1;
}

void T_shift_chars(char *buffer, int S, int L) {
    int dc = strlen(bufor);
    int ddp;

    if (S < 0 || S >= dc) {
        return; 
    }

    if (L <= 0) {
        return; 
    }

    ddp = dc - S + 1;

    memmove(buffer + S + L, buffer + S, ddp);
}


void Saverss(char *I, char *seg, char *I2){
// dodaj wiesz save nadrzedny ten
}

void Rst(char *seg, char *ftext, int X){
    char cyfrs[32];
    int i = snprintf(cyfrs, sizeof(cyfrs), "%d", X);
    if (i <= 0) return;

    char *buf = find_segment(seg)->buffer;
    if (!buf) return;

    size_t buf_len = strlen(buf);
    size_t f_len = strlen(ftext);

    int S = -1;
    for (size_t s = 0; s + f_len <= buf_len; s++){
        if (memcmp(buf + s, ftext, f_len) == 0){ S = (int)s; break; }
    }
    if (S == -1) return;

    char *px = strchr(ftext, 'X');
    int idx_in_f = px ? (int)(px - ftext) : (int)f_len;
    int pos = S + idx_in_f;

    size_t old_tail = buf_len - (pos + 1) + 1;

    memmove(buf + pos + i, buf + pos + 1, old_tail);
    memcpy(buf + pos, cyfrs, (size_t)i);
}

int M = 0;
int S = 0;

void mcerm(int D){

  long long N = 4294967296;
  S = 32;

  long long q = (N - 1) / D; 
  long long r = (N - 1) % D; 
  
  long long dm1r = D - 1 - r;

  while (q < dm1r) {
      S++;
      
      q = (q * 2) + ((r * 2 + 1) / D);
      r = (r * 2 + 1) % D;
      
      dm1r = D - 1 - r;
  }
    

  M = q + 1;
}


int r_ret_asm_win(NODE *expr) {
    switch (expr->type) {
        case NUMBER: return expr->value;
        case CHAR: return *expr->Text;
        case ADD: return r_ret_asm_win(expr->left) + r_ret_asm_win(expr->right);
        case SUB: return r_ret_asm_win(expr->left) - r_ret_asm_win(expr->right);
        case MUL: return r_ret_asm_win(expr->left) * r_ret_asm_win(expr->right);
        case DIV: return r_ret_asm_win(expr->left) / r_ret_asm_win(expr->right);
        case BAND: return r_ret_asm_win(expr->left) & r_ret_asm_win(expr->right);
        case BOR: return r_ret_asm_win(expr->left) | r_ret_asm_win(expr->right);
        case XOR: return r_ret_asm_win(expr->left) ^ r_ret_asm_win(expr->right);
        case SHL: return r_ret_asm_win(expr->left) << r_ret_asm_win(expr->right);
        case SHR: return r_ret_asm_win(expr->left) >> r_ret_asm_win(expr->right);
        case POW: return pow(r_ret_asm_win(expr->left), r_ret_asm_win(expr->right));
        case MOD: return r_ret_asm_win(expr->left) % r_ret_asm_win(expr->right);
        case BNOT: return ~r_ret_asm_win(expr->right);
        case NEG: return -r_ret_asm_win(expr->right);
        default:
            Error_flag = true;
            return -1;
    }
}

int ret_asm_win(NODE *expr){
  Error_flag = false;

  return r_ret_asm_win(expr);
}

int L = 0;

Token_Name gtne(NODE *node){
  NODE *temp = node;

  while (temp && temp->type != NUMBER && temp->type != IDENTIFIER && temp->type != FLOAT && temp->type != CHAR) {
    temp = temp->right;
  }

  return temp->type;
}

int goft(DTYPE type){
  return (( type == DPTR) ? 8 : (type == DFLOAT || type == DINT) ? 4 : 1);
}

char* woffr(int off){
  return (off == 8) ? "QWORD" : (off == 4) ? "DWORD" : "BYTE";
}
bool pot(int n){
  return n > 0 && (n & (n - 1)) == 0;
}


int fld = 0;
int sld = 0;

bool float_ = false;
bool int_ = false;
bool VECTORE_ = false;

bool first_vectore = true;

bool conins = false;
int block_size = 0;
int vectore_operations = 0;

int RFJAIL = 0;

bool ilf = true;
bool ptriadrt = false;
bool _ptr = false;
bool char_ = false;
int IADR_N = 0;

int mvco = 0;

unsigned char slr = 0;

void transpile_expr_asm(NODE *node, char *segment, REG_NODE *regs_tnode) {
    if (!node) return;

    if(regs_tnode == NULL){
      regs_tnode = building_regs_tree(node, 0);
      regs_tree = regs_tnode;
    }
    else if(regs_tnode->right == NULL){
      regs_tnode->right = building_regs_tree(node->left, 0);
    }
    else if(regs_tnode->left == NULL){
      regs_tnode->left = building_regs_tree(node->right, 0);
    }
    
    int goft_win;
    int lenght;

    float_ = false;
    int_ = false;
    VECTORE_ = false;
    first_vectore = true;
    vectore_operations = 0;
    ptriadrt = false;
    char_ = false;
    IADR_N = 0;
    float_r = false;
    char_r = false;
    _ptr = false;
    int off;
    switch (node->type) {
      case NUMBER:
        //if(node->value < 256) char_r = true; // wylaczone 
        append_code(segment, "    mov %s, %d\n", get_reg_string(regs_tnode->reg), node->value);
        int_ = true;
        break;
      case CHAR:
        char_ = true;
        append_code(segment, "    mov %s, '%s'\n", get_reg_string(regs_tnode->reg), node->Text);
        break;
      case STRING:
        append_code("data", "    sld%d: db \"%s\", 0\n", sld, node->Text);
        sld++;
        append_code(segment, "    lea rax, [sld%d]\n", sld-1);
        break;
      case FLOAT:
        float_r = true;
        char *strs = malloc(256 * sizeof(char));
        snprintf(strs, 256, "    fld%d: dd %f\n", fld, node->value_float);
        append_code("data", strs);
        append_code(segment, "    movss %s, [fld%d]\n", get_reg_string(regs_tnode->reg), fld);
        fld++;
        float_ = true;
        break;
      case TRUE:
      case FALSE:
        append_code(segment, "    mov eax, %d\n", node->value);
        break;
      case ADR:
        off = get_variable_offset(node->right->Text, segment);
        if(off != 0) append_code(segment, "    lea rax, [rbp%+d]\n", off);
        else{
          append_code(segment, "    lea rax, %s\n", node->right->Text);
        }
        break;
      case IADR:
        ptriadrt = true;
        while(node->type == IADR){
          node = node->right;
          IADR_N++;
        }
      case IDENTIFIER:
        lenght = get_variable_lenght(node->Text, segment);
        off = get_variable_offset(node->Text, segment);
        goft_win = goft(get_variable_type(node->Text, segment));

        switch(goft_win){
          case 8: 
            long_ = true;
            _ptr = true;
            break;
          case 4: 
            int_ = true;
            break;
          case 1: 
            char_ = true;
            break;
          default:
            break;
        }

        if(!ptriadrt){
          if(goft_win == lenght){
            if(mvco != off){
              append_code(segment, "    mov %s, %s [rbp%+d]\n", get_reg_string(regs_tnode->reg), woffr(goft_win), off);
            }
            else{
              if(regs_tnode->reg != slr)append_code(segment, "    mov %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(slr));
            }
            slr = regs_tnode->reg;
            mvco = off;
          }
          else if(lenght > goft_win){
            if(node->right != NULL){
              long_ = true;
              char *_8reg = get_reg_string(regs_tnode->reg);
              long_ = false; 
              transpile_expr_asm(node->right, segment, regs_tnode);
              append_code(segment, "    mov %s, %s [rbp%+d+%s*%d]\n", get_reg_string(regs_tnode->reg), woffr(goft_win), off, _8reg, goft_win);
            }
            else if(lenght > (VecOps_mode == AVX2) ? 8 * 4 : 4 * 4){
              VECTORE_ = true;
              block_size = lenght;
              append_code(segment, "    lea rdi, [rbp%+d]\n", off);
            }
          }
          else{
            int R = 8;

            NODE* temp = node->right;
            while (temp && temp->type == argse && temp->left){
                Transpilate(temp->left, segment);
                append_code(segment, "    mov r%d, rax\n", R);
                R++;
                temp = temp->right;
            }

            append_code(segment, "    call %s\n", node->Text);
          }
        }
        else{
          append_code(segment, "    mov rdi, QWORD [rbp%+d]\n", off);
          while(IADR_N > 1){
            append_code(segment, "    mov rdi, QWORD [rdi]\n");
            IADR_N--;
          }
          long_ = false;
          append_code(segment, "    mov %s, %s [rdi]\n", (node->right->right->type == M_INT) ? get_reg_string(regs_tnode->reg) : (node->right->right->type == M_CHAR) ? "al" : "rax", woffr((node->right->right->type == M_INT) ? 4 : (node->right->right->type == M_CHAR) ? 1 : 8));
        }

        break;
      case AND:
        ilf = true;
        
        char strl[256];

        snprintf(strl, 256, ".L%d", L);

        create_segment(strl, find_segment(segment)->offset);
        L++;

        transpile_expr_asm(node->left, strl, regs_tnode);

        snprintf(strl, 256, ".L%d", L);

        create_segment(strl, find_segment(segment)->offset);
        L++;

        transpile_expr_asm(node->right, strl, regs_tnode->left);

        break;
      case EQUAL:
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        transpile_expr_asm(node->right, segment, regs_tnode->left);
        if(ilf)append_code(segment, "    push rsi\n");
        append_code(segment, "    cmp %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        if(ilf) append_code(segment, "    lea rsi, [rel $ + 13]\n");
        append_code(segment, "    je near .L%d\n", L);
        if(ilf)append_code(segment, "    pop rsi\n");
        break;
      case NEQUAL:
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        transpile_expr_asm(node->right, segment, regs_tnode->left);
        if(ilf)append_code(segment, "    push rsi\n");
        append_code(segment, "    cmp %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        if(ilf) append_code(segment, "    lea rsi, [rel $ + 13]\n");
        append_code(segment, "    jne near .L%d\n", L);
        if(ilf)append_code(segment, "    pop rsi\n");
        break;
      case JG:
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        transpile_expr_asm(node->right, segment, regs_tnode->left);
        if(ilf)append_code(segment, "    push rsi\n");
        append_code(segment, "    cmp %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        if(ilf) append_code(segment, "    lea rsi, [rel $ + 13]\n");
        append_code(segment, "    jg near .L%d\n", L);
        if(ilf)append_code(segment, "    pop rsi\n");
        break;
      case JL:
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        transpile_expr_asm(node->right, segment, regs_tnode->left);
        if(ilf)append_code(segment, "    push rsi\n");
        append_code(segment, "    cmp %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        if(ilf) append_code(segment, "    lea rsi, [rel $ + 13]\n");
        append_code(segment, "    jl near .L%d\n", L);
        if(ilf)append_code(segment, "    pop rsi\n");
        break;
      case BAND:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag){
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, "    and %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        }
        else{
           append_code(segment, "    and %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }
        break;
      case BOR:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag){
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, "    or %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        }
        else{
           append_code(segment, "    or %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }
        break;
      case XOR:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag){
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, "    xor %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        }
        else{
           append_code(segment, "    xor %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }
        break;
      case BNOT:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        regs_tnode->left->reg--;
        transpile_expr_asm(node->right, segment, regs_tnode->left);
        append_code(segment, "    not %s\n", get_reg_string(regs_tnode->reg));
        break;
      case NEG:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        regs_tnode->left->reg--;
        transpile_expr_asm(node->right, segment, regs_tnode->left);
        append_code(segment, "    neg %s\n", get_reg_string(regs_tnode->reg));
        break;
      case INC:
        append_code(segment, "    inc %s [rbp%+d]\n", woffr(get_variable_lenght(node->right->Text, segment)), get_variable_offset(node->right->Text, segment), get_reg_string(regs_tnode->reg));
        break;
      case DEC:
        append_code(segment, "    dec %s [rbp%+d]\n", woffr(get_variable_lenght(node->right->Text, segment)), get_variable_offset(node->right->Text, segment), get_reg_string(regs_tnode->reg));
        break;
      case SHR:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag) transpile_expr_asm(node->right, segment, new_reg_node(ECX, NULL, NULL));
        if(ret_asm_win(node->right) != 1947934 && Error_flag) append_code(segment, "    shr %s, %s\n", get_reg_string(regs_tnode->reg), "cl");
        else{
          append_code(segment, "    shr %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }
        break;
      case SHL:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag) transpile_expr_asm(node->right, segment, new_reg_node(ECX, NULL, NULL));
        if(ret_asm_win(node->right) != 1947934 && Error_flag) append_code(segment, "    shr %s, %s\n", get_reg_string(regs_tnode->reg), "cl");
        else{
          append_code(segment, "    shr %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }
        break;
      case ADD:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(gtne(node) == FLOAT || float_r){
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left); 
          append_code(segment, "    addss %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
          break;
        }

        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag){
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, "    add %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        }
        else{
           append_code(segment, "    add %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }

        break;
      case VECTORE_ADD:
        VECTORE_ = true;
        /*if(VecOps_mode == AVX2){
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    sub rsp, 32\n");
          append_code(segment, "    vmovups [rsp], ymm0\n");
          transpile_expr_asm(node->right, segment);
          append_code(segment, "    vmovups ymm1, [rsp]\n");
          append_code(segment, "    add rsp, 32\n");
          append_code(segment, "    vaddps ymm0, ymm1\n");
        }
        else{
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    push rdi\n");
          transpile_expr_asm(node->right, segment);
          if(first_vectore) append_code(segment, "    mov rdx, rdi\n");
          if(!first_vectore) append_code(segment, "    mov rdx, rcx\n    mov rsi, rcx\n");
          append_code(segment, "    pop rdi\n");
          append_code(segment, "    call afswsse8\n");

          fswsse8 = true;
        }*/
        first_vectore = false;

        break;
      case SUB:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(gtne(node) == FLOAT || float_r){
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left); 
          append_code(segment, "    subss %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
          break;
        }
        transpile_expr_asm(node->left, segment, regs_tnode->right);
        if(ret_asm_win(node->right) != 1947934 && Error_flag){
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, "    sub %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
        }
        else{
           append_code(segment, "    sub %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
        }
        break;
      case VECTORE_SUB:
        VECTORE_ = true;
        /*if(VecOps_mode == AVX2){
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    sub rsp, 32\n");
          append_code(segment, "    vmovups [rsp], ymm0\n");
          transpile_expr_asm(node->right, segment);
          append_code(segment, "    vmovups ymm1, [rsp]\n");
          append_code(segment, "    add rsp, 32\n");
          append_code(segment, "    vsubps ymm0, ymm1\n");
        }
        else{
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    push rdi\n");
          transpile_expr_asm(node->right, segment);
          if(first_vectore) append_code(segment, "    mov rdx, rdi\n");
          if(!first_vectore) append_code(segment, "    mov rdx, rcx\n    mov rsi, rcx\n");
          append_code(segment, "    pop rdi\n");
          append_code(segment, "    call sfswsse8\n");

          fswsse8 = true;
        }*/
        first_vectore = false;

        break;
      case MUL:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(gtne(node) == FLOAT || float_r){
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left); 
          append_code(segment, "    mulss %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
          break;
        }
        if(pot(ret_asm_win(node->right))){
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          append_code(segment, "    shl %s, %d\n", get_reg_string(regs_tnode->reg), log2(ret_asm_win(node->right)));
        }
        else{
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          if(ret_asm_win(node->right) != 1947934 && Error_flag){
            transpile_expr_asm(node->right, segment, regs_tnode->left);
            append_code(segment, "    imul %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
          }
          else{
           append_code(segment, "    imul %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right));
          }
        }
        break;
      case VECTORE_MUL:
        VECTORE_ = true;
        /*if(VecOps_mode == AVX2){
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    sub rsp, 32\n");
          append_code(segment, "    vmovups [rsp], ymm0\n");
          transpile_expr_asm(node->right, segment);
          append_code(segment, "    vmovups ymm1, [rsp]\n");
          append_code(segment, "    add rsp, 32\n");
          append_code(segment, "    vmulps ymm0, ymm1\n");
        }
        else{
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    push rdi\n");
          transpile_expr_asm(node->right, segment);
          if(first_vectore) append_code(segment, "    mov rdx, rdi\n");
          if(!first_vectore) append_code(segment, "    mov rdx, rcx\n    mov rsi, rcx\n");
          append_code(segment, "    pop rdi\n");
          append_code(segment, "    call mfswsse8\n");

          fswsse8 = true;
        }*/
        first_vectore = false;

        break;
      case DIV:
        if(ret_asm_win(node) != -1 && !Error_flag){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(gtne(node) == FLOAT || float_r){
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left); 
          append_code(segment, "    divss %s, %s\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg));
          break;
        }
        if(pot(ret_asm_win(node->right))){
          transpile_expr_asm(node->left, segment, regs_tnode->left);
          append_code(segment, "    shr %s, %d\n", get_reg_string(regs_tnode->reg), log2(ret_asm_win(node->right)));
        }
        else if(ret_asm_win(node->right) > 0 && !Error_flag){
          transpile_expr_asm(node->left, segment, regs_tnode->left);
          mcerm(ret_asm_win(node->right));
          long_ = true;
          append_code(segment, "    imul %s, %s, %d\n    shr %s, %d\n", get_reg_string(regs_tnode->reg), get_reg_string(regs_tnode->left->reg), M, get_reg_string(regs_tnode->reg), S);
        }
        else{
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, "    mov r15, %s\n    push rax\n    push rdx\n", get_reg_string(regs_tnode->reg));
          long_ = true;
          append_code(segment, "    cqo\n    idiv r15\n    mov %s, rax\n    pop rax\n     pop rdx\n", get_reg_string(regs_tnode->reg));
        }
        break;
      case VECTORE_DIV:
        VECTORE_ = true;
        /*if(VecOps_mode == AVX2){
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    sub rsp, 32\n");
          append_code(segment, "    vmovups [rsp], ymm0\n");
          transpile_expr_asm(node->right, segment);
          append_code(segment, "    vmovups ymm1, [rsp]\n");
          append_code(segment, "    add rsp, 32\n");
          append_code(segment, "    vdivps ymm0, ymm1\n");
        }
        else{
          transpile_expr_asm(node->left, segment);
          append_code(segment, "    push rdi\n");
          transpile_expr_asm(node->right, segment);
          if(first_vectore) append_code(segment, "    mov rdx, rdi\n");
          if(!first_vectore) append_code(segment, "    mov rdx, rcx\n    mov rsi, rcx\n");
          append_code(segment, "    pop rdi\n");
          append_code(segment, "    call dfswsse8\n");

          fswsse8 = true;
        }*/
        first_vectore = false;

        break;
      case POW:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(pot(ret_asm_win(node->left))){
          append_code(segment, "    mov %s, 1\n", get_reg_string(regs_tnode->left->reg));
          append_code(segment, "    mov cl, %d\n", (int)(ret_asm_win(node->right) * log2((double)ret_asm_win(node->left))));
          append_code(segment, "    shl %s, cl\n", get_reg_string(regs_tnode->reg));
        }
        else{
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, 
            "    mov ecx, eax            \n"
            "    mov eax, 1              \n"
            "    cmp ecx, 0\n"
            "    je pow_end\n"
            "    call pow_loop\n"
          );
          pow_use = true;
        }
        break;
      case DEPOW:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(pot(ret_asm_win(node->left))){
          append_code(segment, "    mov %s, 1\n", get_reg_string(regs_tnode->left->reg));
          append_code(segment, "    mov cl, %d\n", (int)(ret_asm_win(node->right) * log2((double)ret_asm_win(node->left))));
          append_code(segment, "    shr %s, cl\n", get_reg_string(regs_tnode->right->reg));
        }
        else{
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          append_code(segment, 
            "    mov ecx, eax             \n"
            "    mov eax, 1              \n"
            "    cmp ecx, 0\n"
            "    cdq\n"
            "    call depow\n"
          );
          depow_use = true;
        }
        break;
      case MOD:
        if(ret_asm_win(node) != -1 && Error_flag == false){
          append_code(segment, "    mov %s, %d\n",  get_reg_string(regs_tnode->reg), ret_asm_win(node));
          break;
        }
        if(pot(ret_asm_win(node->right))){
          transpile_expr_asm(node->left, segment, regs_tnode->left);
          append_code(segment, "    and %s, %d\n", get_reg_string(regs_tnode->reg), ret_asm_win(node->right) - 1);
        }
        else{
          transpile_expr_asm(node->left, segment, regs_tnode->right);
          transpile_expr_asm(node->right, segment, regs_tnode->left);
          
          append_code(segment, 
            "    mov rbx, rax\n"
            "    cqo            \n"
            "    idiv rbx       \n"
            "    mov rax, rdx   \n"
          );
        }
        break;
      default:
        append_code(segment, "    ; unsupported node %d\n", node->type);
    }
}

int d = 0;
bool ptriadr = false;

void Transpilate(NODE *tree, char *segment) {
    if (!tree) return;

    Error_flag = false;
    ilf = false;
    ptriadr = false;
    _ptr = false;
    long_ = false;
    IADR_N = 0;
    mvco = 0;
    int lenght;
    
    switch (tree->type) {
      case _INT: {
        NODE *idn = tree->right;
        NODE *val = idn? idn->right: NULL;
        int off = add_variable(idn->Text, 4, segment, DINT);

        if(val != NULL){
          if(ret_asm_win(val) != -1 && Error_flag == false) append_code(segment, "    mov DWORD [rbp%+d], %d\n", off, ret_asm_win(val));
          else{
            transpile_expr_asm(val, segment, regs_tree);
            append_code(segment, "    mov DWORD [rbp%+d], eax\n", off);
          }
        }

        break;
      }
      case WRITE: {
        NODE *arg = tree->right;

        append_code(segment, "    push rsi\n");

        if (arg->type == STRING) {
          char *strs = malloc(256 * sizeof(char));
          snprintf(strs, 256, "    sld%d: db \"%s\", 0\n", sld, arg->Text);
          append_code("data", strs);
          append_code(segment, 
            "    mov rax, 1\n"
            "    mov rdi, 1\n"
            "    mov rsi, [sld%d]\n"
            "    mov rdx, %d\n"
            "    syscall\n", sld, strlen(arg->Text + 1)
          );
          sld++;
        }
          if(arg->type == CHAR || arg->type == IDENTIFIER && get_variable_type(arg->Text, segment) == DCHAR){
            if(arg->type == IDENTIFIER){
              // char *str2 = malloc(256 * sizeof(char));
              int off;
              off = get_variable_offset(arg->Text, segment);
              append_code(segment, 
                "    mov rax, 1\n"
                "    mov rdi, 1\n"
                "    lea rsi, [rbp%+d]\n"
                "    mov rdx, 1\n"
                "    syscall\n", off);
            }
            else if(arg->type == CHAR){
              int off = add_variable("ACHAR", 1, segment, DCHAR);
              append_code(segment, "    mov al, '%s'\n", arg->Text);
              append_code(segment, "    sub rsp, 1\n");
              find_segment(segment)->stack_N += 1;
              append_code(segment, "    mov BYTE [rbp%+d], al\n", off);
              append_code(segment, 
              "    mov rax, 1\n"
              "    mov rdi, 1\n"
              "    lea rsi, [rbp%+d]\n"
              "    mov rdx, 1\n"
              "    syscall\n", off);
            }
        } else{
          transpile_expr_asm(arg, segment, regs_tree);
          append_code(segment, "    movsxd rdi, eax\n    call write_int\n");
          write_int = true;
        }

        append_code(segment, "    pop rsi\n");

        break;
      }
      case _CHAR:
        int off = add_variable(tree->right->Text, 1, segment, DCHAR);
        transpile_expr_asm(tree->right->right, segment, regs_tree);
        append_code(segment, "    mov BYTE [rbp%+d], al\n", off);
        break;
      case _BOOL:
        int off1 = add_variable(tree->right->Text, 1, segment, DBOOL);
        append_code(segment, "    mov BYTE [rbp%+d], %d\n", off1, tree->right->right->value);
        break;
      case _PTR:
        _ptr = true;
        int off4 = add_variable(tree->right->Text, 8, segment, DPTR);
        transpile_expr_asm(tree->right->right, segment, regs_tree);
        append_code(segment, "    mov QWORD [rbp%+d], rax\n", off4);

        break;
      case _FLOAT:
        int off5 = add_variable(tree->right->Text, 4, segment, DFLOAT);
        transpile_expr_asm(tree->right->right, segment, regs_tree);
        append_code(segment, "    movss DWORD [rbp%+d], xmm0\n", off5);
        break;
      case IADR:
       ptriadr = true;
        while(tree->type == IADR){
          tree = tree->right;
          IADR_N++;
        }
      case IDENTIFIER:
        off = get_variable_offset(tree->Text, segment);
        int goft_win = goft(get_variable_type(tree->Text, segment));
        lenght = get_variable_lenght(tree->Text, segment);
        bool vla = true;

        if(lenght > goft_win && ret_asm_win(tree->right) != -1 && !Error_flag){
          int index = ret_asm_win(tree->right);

          if(lenght < index) printf("ERROR:  index wiekszy niz wielkosc tablicy %d %d\n", lenght, index);

          off = off + index * goft_win; 
          vla = false;           
        }

        int offl = get_variable_offset(tree->Text, segment);

        if(lenght > goft_win) append_code(segment, "    push rsi\n    lea rsi, [rbp%+d]\n    mov rcx, rsi\n", offl);

        regs_tree = building_regs_tree(tree->left, 0);

        transpile_expr_asm(tree->right, segment, regs_tree);
        transpile_expr_asm(tree->right->right, segment, regs_tree->left);

        long_ = true;
        char *_8reg = get_reg_string(regs_tree->reg);
        long_ = false;

        if(!ptriadr){
          if(int_ && !vla) append_code(segment, "    mov DWORD [rbp%+d], %s\n", off, get_reg_string(regs_tree->left->reg));
          if(float_ && !vla){
            float_r = true;
            append_code(segment, "    movss DWORD [rbp%+d], %s\n", off, get_reg_string(regs_tree->left->reg));
          }
          if(char_ && !vla){
            char_r = true;
            append_code(segment, "    mov BYTE [rbp%+d], %s\n", off, get_reg_string(regs_tree->left->reg));
          }
          if(int_ && vla) append_code(segment, "    mov %s [rbp%+d+%s*%d], %s\n", woffr(goft_win), off, _8reg, goft_win, get_reg_string(regs_tree->left->reg));
          if(float_ && vla){
            float_r = true;
            append_code(segment, "    movss %s [rbp%+d+%s*%d], %s\n", woffr(goft_win), off, _8reg, goft_win, get_reg_string(regs_tree->reg));
          }
          if(char_ && vla){
            char_r = true;
            append_code(segment, "    mov %s [rbp%+d+%s*%d], %s\n", woffr(goft_win), off, _8reg, goft_win, get_reg_string(regs_tree->left->reg));
          }
          if(_ptr && !vla){
            long_ = true;
            append_code(segment, "    mov QWORD [rbp%+d], %s\n", off, get_reg_string(regs_tree->left->reg));
          }
          if(_ptr && vla){
            long_ = true;
            append_code(segment, "    mov %s [rbp%+d+%s*%d], %s\n", woffr(goft_win), off, _8reg, goft_win, get_reg_string(regs_tree->left->reg));
          }
          if(VECTORE_){
            // tu tez troche ten no tego hehe..
            if(!first_vectore) append_code(segment, "    mov rsi, rcx\n");;

            int i = 0;
            while(offl != 0){
              if(abs(offl) >= 4096 * 32){
                append_code(segment, "    mov rcx, %d\n    call cdwsse_lum64\n", abs(lenght + offl));
                offl -= offl;
              }
              else{
                while(abs(offl) >= 128){
                    if(abs(offl) >= 2048){
                      append_code(segment, "    call cdwsse64\n");

                      offl += 2048;
                      i++;
                    }
                    else if(abs(offl) >= 128){
                      append_code(segment, "    call cdwsse8\n");

                      offl += 128;
                      i++;
                  }
                }
              }
              if(abs(offl) < 128) offl = 0;
            }
            cdwsse = true;
          }
        } 
        else if(ptriadr){
          append_code(segment, "    mov rdi, QWORD [rbp%+d]\n", off);
          while(IADR_N > 1){
            append_code(segment, "    mov rdi, QWORD [rdi]\n");
            IADR_N--;
          }
          transpile_expr_asm(tree->right->right->right, segment, regs_tree);
          append_code(segment, "    mov %s [rdi], %s\n", woffr((tree->right->right->type == M_INT) ? 4 : (tree->right->right->type == M_CHAR) ? 1 : 8), (tree->right->right->type == M_INT) ? "eax" : (tree->right->right->type == M_CHAR) ? "al" : "rax");
        }
        if(lenght > goft_win) append_code(segment, "    pop rsi\n");
        
        break;
      case MSSUB:
        break;
      case MSADD:
        break;
      case _ARRAY:
        int array_off;
        transpile_expr_asm(tree->right->right->right, segment, regs_tree);

        array_off = add_variable(tree->right->right->Text, ((tree->right->type == _INT) ? 4 : (tree->right->type == _FLOAT) ? 4 : 1) * ret_asm_win(tree->right->right->right), segment, ((tree->right->type == _INT) ? DINT : (tree->right->type == _FLOAT) ? DFLOAT : DCHAR));

        if(!fmuse)find_segment(segment)->stack_N += abs(array_off);

        if(!fmuse) append_code(segment, "    sub rsp, %d\n", ((tree->right->type == _INT) ? 4 : (tree->right->type == _FLOAT) ? 4 : 1) * ret_asm_win(tree->right->right->right));
        break;
      case IF:
        append_code(segment, "    pushfq\n");

        ilf = true;
        transpile_expr_asm(tree->right, segment, regs_tree);

        append_code(segment, "    popfq\n");

        char strl[256];

        snprintf(strl, 256, ".L%d", L);

        create_segment(strl, find_segment(segment)->offset);
        L++;

        append_code(strl, "%s:\n", strl);

        NODE* temp = tree->right;
        while(temp != NULL){
          while(temp->type != code) temp = temp->right;
          Transpilate(temp->left, strl);
          temp = temp->right;
        }

        append_code(strl, "    jmp rsi\n");

        break;
      case LOOP:
        // złe 
        append_code(segment, "    pushfq\n");

        ilf = true;
        transpile_expr_asm(tree->right, segment, regs_tree);

        append_code(segment, "    popfq\n");
        
        char strl3[256];
        char strl1[256];
        char strl2[256];

        snprintf(strl3, 256, ".L%d", L);

        create_segment(strl3, find_segment(segment)->offset);
        L++;

        append_code(strl3, "%s:\n", strl3);

        snprintf(strl1, 256, ".L%d", L);

        create_segment(strl1, find_segment(segment)->offset);
        L++;

        append_code(strl1, "%s:\n", strl1);


        ilf = false;
        L--;
        transpile_expr_asm(tree->right, strl3, regs_tree);
        L++;

        temp = tree->right;
        while(temp != NULL){
          while(temp->type != code) temp = temp->right;
          Transpilate(temp->left, strl1);
          temp = temp->right;
        }
        append_code(strl1, "    jmp %s\n", strl3);

        snprintf(strl2, 256, ".L%d", L);

        create_segment(strl2, find_segment(segment)->offset);
        L++;

        append_code(strl3, "    jmp %s\n", strl2);

        append_code(strl2, "%s:\n    jmp rsi\n", strl2);
        break;
      case BREAK:
        // do implementacji
        break;
      case RETURN:
        transpile_expr_asm(tree->right, segment, regs_tree);
        if(ss && find_segment(segment)->stack_N != 0) append_code(segment, "    add rsp, %d\n", find_segment(segment)->stack_N);
        append_code(segment, "%s    ret\n ", ss ? "" : "    leave\n");
        break;
      case FUNCTION:
        create_segment(tree->right->right->Text, 0);

        if(tree->right->type == _SS) ss = true;

        append_code(tree->right->right->Text, "\n%s:\n", tree->right->right->Text);

        if(!ss) append_code(tree->right->right->Text,"    push rbp\n    mov rbp, rsp\n");

        append_code(tree->right->right->Text, "    sub rsp, X \n");

        int R = 8;

        NODE* temp2 = tree->right->right->right;
        while (temp2 && temp2->type == args && temp2->left){
            add_variable(temp2->left->left->Text, (temp2->left->type == _PTR) ? 8 : (temp2->left->type == _INT) ? 4 : 1, tree->right->right->Text, (temp2->left->type == _PTR) ? DPTR : (temp2->left->type == _INT) ? DINT : DCHAR);
            append_code(tree->right->right->Text, "    mov %s [rbp%+d], r%d%c\n", woffr(abs(get_variable_offset(temp2->left->left->Text, tree->right->right->Text))), get_variable_offset(temp2->left->left->Text, tree->right->right->Text), R, (get_variable_offset(temp2->left->left->Text, tree->right->right->Text) == -8) ? ' ' : (get_variable_offset(temp2->left->left->Text, tree->right->right->Text) == -4) ? 'd' : (get_variable_offset(temp2->left->left->Text, tree->right->right->Text) == -2) ? 'w' : 'b');
            // Dodaj argumsnty domyslne itp 
            R++;
            temp2 = temp2->right;
        }
        NODE *temp8 = temp2;
        while(temp2 != NULL){
          while(temp2->type != code) temp2 = temp2->right;
          temp8 = temp2;
          while(temp8 != NULL && temp8->left != NULL && temp8->left->type == ELSE || temp8->left->type == ELFI || temp8->left->type == IF){
            if(temp8->left->type == ELSE || temp8->left->type == ELFI) RFJAIL++;
            if(temp8->right == NULL) break;
            temp8 = temp8->right;
          }
          Transpilate(temp2->left, tree->right->right->Text);
          temp2 = temp2->right;
        }

        Rst(tree->right->right->Text, "    sub rsp, X", find_segment(tree->right->right->Text)->stack_N);

        ss = false;

        break;
      case TURN:
        R = 8;

        NODE* temp3 = tree->right->right;

        while (temp3 && temp3->type == argse && temp3->left){
            Transpilate(temp3->left, segment);
            append_code(segment, "    mov r%d, rax\n", R);
            R++;
            temp3 = temp3->right;
        }

        if(get_variable_type(tree->right->Text, segment) == DPTR){
          append_code(segment, "    mov rax, QWORD [rbp%+d]\n    call rax\n", get_variable_offset(tree->right->Text, segment));
        }
        else{
          append_code(segment, "    call %s\n", tree->right->Text);
        }
        break;
      case READ:
        append_code(segment, "    push rsi\n    mov rax, 0\n    mov rdi, 0\n    lea rsi, [rbp%+d]\n    mov rdx, 1\n    syscall\n    pop rsi\n", get_variable_offset(tree->right->Text, segment));
        break;
      case VIR:
        dealloc_var(tree->right->Text, segment);
        break;
      case EXTERN:
        append_code("main", "extern %s\n", tree->right->Text);
        break;
      case SEXTERN:
        Ssymbol = tree->right->Text;
        break;
      case _STACK:
        add_variable(tree->right->Text, 0, "bss", DPTR);
        append_code("bss", "    %s resb %d\n", tree->right->Text, ret_asm_win(tree->right->right));
        break;
      case SSET:
        append_code(segment, "    push rsp\n    push rbp\n    mov rsp, %s + 6000\n    mov rbp, rsp\n", tree->right->Text);
        break;
      case SBACK:
        append_code(segment, "    pop rsp\n    pop rbp\n"); // zmien by przywowywało normalne rsp np zapisywało gdzoie indziej tez pushy nie i popy nie
        break;
      case SAVE:
        Saverss(tree->right->Text, segment, tree->right->right->Text);
        break;
      default:
        transpile_expr_asm(tree, segment, regs_tree);
  }
}

