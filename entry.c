#include <stdint.h>
#include <inttypes.h>

#include <linux/auxvec.h>
#include <linux/elf.h>

#include "printf.h"

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

inline static int pctx_auxv_parse(pctx_data_t* pctx_data) {
    for(pctx_aux_entry_t const* entry = pctx_data->auxv; 
            entry->type != AT_NULL; ++entry) {
        if(entry->type < PCTX_N_STD_AUXV_ENTRIES) {
            pctx_data->std_auxv[entry->type] = *entry;
        }
    }

    return 0;
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

// __attribute__((noreturn))
void entry(void* pctx) {
    printf("m1stld started successfully!\n");

    pctx_data_t pctx_data;
    pcxt_parse(pctx, &pctx_data);
#ifndef NDEBUG
    pcxt_data_dump(&pctx_data);
#endif /* NDEBUG */

    if(pctx_data.std_auxv[AT_ENTRY].type == AT_ENTRY) {
        pctx_data.std_auxv[AT_ENTRY].func();
    }
}
