#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#include <linux/auxvec.h>
#include <linux/elf.h>

#include "printf.h"

#ifndef NDEBUG
#   define DEBUG_LOG(...) \
    printf(__VA_ARGS__)
#else
#   define DEBUG_LOG(...)
#endif

__attribute((noreturn))
extern void exit(int exit_code);

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
typedef struct {
    Elf64_Dyn const* dynamic;
} dso_t;

inline static void parse_dso(dso_t* dso, pctx_data_t const* pctx_data) {
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
    DEBUG_LOG("m1stld started successfully!\n");

    pctx_data_t pctx_data;
    pcxt_parse(pctx, &pctx_data);

    dso_t dso;
    parse_dso(&dso, &pctx_data);

    if(pctx_data.std_auxv[AT_ENTRY].type == AT_ENTRY) {
        pctx_data.std_auxv[AT_ENTRY].func();
    }
}
