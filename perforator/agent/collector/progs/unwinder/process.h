#pragma once

#include "binary.h"

#include <bpf/types.h>

enum unwind_type : u8 {
    UNWIND_TYPE_DISABLED = 0,
    UNWIND_TYPE_FP = 1,
    UNWIND_TYPE_DWARF = 2,
};

struct mapped_binary {
    binary_id id;
    u64 start_address;
};

static ALWAYS_INLINE bool is_mapped(struct mapped_binary binary) {
    return binary.start_address != 0;
}

struct process_info {
    enum unwind_type unwind_type;
    binary_id main_binary_id;

    struct mapped_binary pthread_binary;

    struct mapped_binary python_binary;
    struct mapped_binary php_binary;
};
