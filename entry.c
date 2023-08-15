#include <stdint.h>
#include <inttypes.h>

#include <linux/auxvec.h>
#include <linux/elf.h>

#include "printf.h"

typedef struct {
    uint64_t tag;
    uint64_t value;
} pctx_aux_entry_t;

typedef void(*entry_t)(void);

typedef struct {
    uint64_t argc;
    char const** argv;

    uint64_t envc;
    char const** envv;

    pctx_aux_entry_t const* auxv;
} pctx_data_t;

inline static void pcxt_parse(void const* pctx, pctx_data_t* pctx_data) {
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
            entry->tag != AT_NULL; ++entry) {
        switch(entry->tag) {
        case AT_IGNORE:
            printf("\t\tAT_IGNORE\n");
            break;
        case AT_ENTRY:
            printf("\t\tAT_ENTRY = %"PRIx64"\n", entry->value);
            break;
        case AT_EXECFD:
            printf("\t\tAT_EXECFD = %"PRId64"\n", entry->value);
            break;
        case AT_PHDR:
            printf("\t\tAT_PHDR = %"PRIx64"\n", entry->value);
            break;
        case AT_PHENT:
            printf("\t\tAT_PHENT = %"PRId64"\n", entry->value);
            break;
        case AT_PHNUM:
            printf("\t\tAT_PHNUM = %"PRId64"\n", entry->value);
            break;
        case AT_PAGESZ:
            printf("\t\tAT_PAGESZ = %"PRId64"\n", entry->value);
            break;
        case AT_BASE:
            printf("\t\tAT_BASE = %"PRIx64"\n", entry->value);
            break;
        case AT_FLAGS:
            printf("\t\tAT_FLAGS = %"PRIx64"\n", entry->value);
            break;
        case AT_NOTELF:
            printf("\t\tAT_NOTELF\n");
            break;
        case AT_UID:
            printf("\t\tAT_UID = %"PRId64"\n", entry->value);
            break;
        case AT_EUID:
            printf("\t\tAT_EUID = %"PRId64"\n", entry->value);
            break;
        case AT_GID:
            printf("\t\tAT_GID = %"PRId64"\n", entry->value);
            break;
        case AT_EGID:
            printf("\t\tAT_EGID = %"PRId64"\n", entry->value);
            break;
        case AT_PLATFORM:
            printf("\t\tAT_PLATFORM = \'%s\'\n", (char const*)entry->value);
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

    pcxt_data_dump(&pctx_data);

    for(pctx_aux_entry_t const* entry = pctx_data.auxv; 
            entry->tag != AT_NULL; ++entry) {
        if(entry->tag == AT_ENTRY) {
            printf("passing control to the client process...\n");
            ((entry_t)entry->value)();
        }
    }
}
