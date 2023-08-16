#include <stdint.h>
#include <inttypes.h>

#include <linux/auxvec.h>
#include <linux/elf.h>

#include "printf.h"

__attribute((noreturn))
extern void exit(int exit_code);

#define LD_ERROR_EXIT_CODE 2
#define LD_ERROR(condition, format, ...) \
    do { \
        if(!condition) { \
            printf(format, __VA_ARGS__); \
            exit(LD_ERROR_EXIT_CODE); \
        } \
    while(0);

typedef struct {
    int type;
    union {
        long val;
        void* ptr;
        void(*func)();
    };
} pctx_aux_entry_t;

typedef void(*entry_t)(void);

#define PCTX_N_STD_AUXV_ENTRIES 15
typedef struct {
    uint64_t argc;
    char const** argv;

    uint64_t envc;
    char const** envv;

    pctx_aux_entry_t std_auxv[PCTX_N_STD_AUXV_ENTRIES];
    pctx_aux_entry_t const* auxv;
} pctx_data_t;

#define ERR_UNSUPPORTED_FORMAT 1
#define PCTX_DATA_POISON UINT64_MAX

inline static void pctx_data_init(pctx_data_t* pctx_data) {
    pctx_data->argv = pctx_data->envv = NULL;
    pctx_data->auxv = NULL;

    pctx_data->argc = pctx_data->envc = 0;

    for(int i = 0; i < PCTX_N_STD_AUXV_ENTRIES; ++i) {
        pctx_data->std_auxv[i].type = 0;
    }
}

inline static void pctx_auxv_parse(pctx_data_t* pctx_data) {
    for(pctx_aux_entry_t const* entry = pctx_data->auxv; 
            entry->type != AT_NULL; ++entry) {
        if(entry->type < PCTX_N_STD_AUXV_ENTRIES) {
            pctx_data->std_auxv[entry->type] = *entry;
        }
    }
}

inline static void pctx_sysv_header_parse(void const* pctx, 
        pctx_data_t* pctx_data) {
    pctx_data->argc = *(uint64_t*)pctx;
    pctx_data->argv = (char const**)((uint64_t const*)pctx + 1);

    pctx_data->envv = (char const**)((uint64_t const*)pctx + pctx_data->argc + 2);

    pctx_data->envc = 0;
    for(char const** env = pctx_data->envv; *env; ++env) {
        ++pctx_data->envc;
    }

    pctx_data->auxv = (pctx_aux_entry_t const*)((uint64_t const*)pctx +
        pctx_data->argc + pctx_data->envc + 3);
}
inline static void pcxt_parse(void const* pctx, pctx_data_t* pctx_data) {
    pctx_data_init(pctx_data);
    pctx_sysv_header_parse(pctx, pctx_data);
    pctx_auxv_parse(pctx_data);
}

inline static void pcxt_data_dump(pctx_data_t const* pctx_data) {
    printf("process context data dump:\n");

    printf("\targc = %"PRId64"\n\targv contains\n", pctx_data->argc);
    for(char const** arg = pctx_data->argv; 
            arg < pctx_data->argv + pctx_data->argc; ++arg) {
        printf("\t\t\'%s\'\n", *arg);
    }

    printf("\tenvc = %"PRId64"\n\tenvv contains\n", pctx_data->envc);
    for(char const** arg = pctx_data->envv; 
            arg < pctx_data->envv + pctx_data->envc; ++arg) {
        printf("\t\t\'%s\'\n", *arg);
    }

    printf("\taux vector contains\n");
    for(pctx_aux_entry_t const* entry = pctx_data->auxv; 
            entry->type != AT_NULL; ++entry) {
        switch(entry->type) {
        case AT_IGNORE:
            printf("\t\tAT_IGNORE\n");
            break;
        case AT_ENTRY:
            printf("\t\tAT_ENTRY = %p\n", entry->func);
            break;
        case AT_EXECFD:
            printf("\t\tAT_EXECFD = %li\n", entry->val);
            break;
        case AT_PHDR:
            printf("\t\tAT_PHDR = %p\n", entry->ptr);
            break;
        case AT_PHENT:
            printf("\t\tAT_PHENT = %li\n", entry->val);
            break;
        case AT_PHNUM:
            printf("\t\tAT_PHNUM = %li\n", entry->val);
            break;
        case AT_PAGESZ:
            printf("\t\tAT_PAGESZ = %li\n", entry->val);
            break;
        case AT_BASE:
            printf("\t\tAT_BASE = %p\n", entry->ptr);
            break;
        case AT_FLAGS:
            printf("\t\tAT_FLAGS = %li\n", entry->val);
            break;
        case AT_NOTELF:
            printf("\t\tAT_NOTELF = %li\n", entry->val);
            break;
        case AT_UID:
            printf("\t\tAT_UID = %li\n", entry->val);
            break;
        case AT_EUID:
            printf("\t\tAT_EUID = %li\n", entry->val);
            break;
        case AT_GID:
            printf("\t\tAT_GID = %li\n", entry->val);
            break;
        case AT_EGID:
            printf("\t\tAT_EGID = %li\n", entry->val);
            break;
        default:
            printf("\t\tunknown entry\n");
            break;
        }
    }
}

inline static void dump_phdr_type(Elf64_Phdr const* phdr) {
    switch(phdr->p_type) {
    case PT_NULL:
        printf("\t\tp_type = PT_NULL\n");
        break;
    case PT_LOAD:
        printf("\t\tp_type = PT_LOAD\n");
        break;
    case PT_DYNAMIC:
        printf("\t\tp_type = PT_DYNAMIC\n");
        break;
    case PT_INTERP:
        printf("\t\tp_type = PT_INTERP\n");
        break;
    case PT_NOTE :
        printf("\t\tp_type = PT_NOTE\n");
        break;
    case PT_SHLIB:
        printf("\t\tp_type = PT_SHLIB\n");
        break;
    case PT_PHDR:
        printf("\t\tp_type = PT_PHDR\n");
        break;
    case PT_TLS:
        printf("\t\tp_type = PT_TLS\n");
        break;
    default:
        printf("\t\tp_type is unknown\n");
        break;
    }
}

inline static void dump_phdr(Elf64_Phdr const* phdr) {
    dump_phdr_type(phdr);
    printf("\t\tp_offset = %li\n", (long)phdr->p_offset);
    printf("\t\tp_vaddr = %li\n", (long)phdr->p_vaddr);
    printf("\t\tp_paddr = %li\n", (long)phdr->p_paddr);
    printf("\t\tp_memsz = %li\n", (long)phdr->p_memsz);
    printf("\t\tp_filesz = %li\n", (long)phdr->p_filesz);
    printf("\t\tp_align = %li\n", (long)phdr->p_align);
    printf("\t\tp_flags = %li\n", (long)phdr->p_flags);
}

inline static void dump_phdrs(pctx_data_t const* pctx_data) {
    if(pctx_data->std_auxv[AT_PHDR].type != AT_PHDR) {
        printf("PHDRs not present\n");
        return;
    }

    Elf64_Phdr const* phdrs = 
        (Elf64_Phdr const*)pctx_data->std_auxv[AT_PHDR].ptr;
    Elf64_Phdr const* phdrs_end = phdrs + pctx_data->std_auxv[AT_PHNUM].val;

    printf("PHDRs:\n");
    for(Elf64_Phdr const* phdr = phdrs; phdr < phdrs_end; ++phdr) {
        printf("\tPHDR\n");
        dump_phdr(phdr);
    }
}

typedef struct {
    Elf64_Dyn const* dynamic;
} dso_t;

void parse_dso(dso_t* dso, pctx_data_t const* pctx_data) {
    Elf64_Phdr const* dynamic_phdr = NULL;

    Elf64_Phdr const* phdrs = 
        (Elf64_Phdr const*)pctx_data->std_auxv[AT_PHDR].ptr;
    Elf64_Phdr const* phdrs_end = phdrs + pctx_data->std_auxv[AT_PHNUM].val;
    for(Elf64_Phdr const* phdr = phdrs; phdr < phdrs_end; ++phdr) {
        if(phdr->p_type == PT_DYNAMIC) {
            dynamic_phdr = phdr;
        }
    }

    if(dynamic_phdr == NULL) {
        return;
    }

    dso->dynamic = (Elf64_Dyn const*)(
        (uint8_t const*)pctx_data->std_auxv[AT_BASE].ptr + 
            dynamic_phdr->p_offset);
}

// __attribute__((noreturn))
void entry(void* pctx) {
    printf("m1stld started successfully!\n");

    pctx_data_t pctx_data;
    pcxt_parse(pctx, &pctx_data);
#ifndef NDEBUG
    pcxt_data_dump(&pctx_data);
#endif /* NDEBUG */

#ifndef NDEBUG
    dump_phdrs(&pctx_data);
#endif /* NDEBUG */

    dso_t dso;
    parse_dso(&dso, &pctx_data);

    if(pctx_data.std_auxv[AT_ENTRY].type == AT_ENTRY) {
        pctx_data.std_auxv[AT_ENTRY].func();
    }
}
