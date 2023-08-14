#include <stdint.h>
#include <inttypes.h>
#include <linux/auxvec.h>

#include "printf.h"

typedef struct {
    uint64_t tag;
    uint64_t value;
} pctx_aux_entry_t;

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
        case AT_ENTRY:
            printf("\t\tentry_addr = %"PRIx64"\n");
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
}
