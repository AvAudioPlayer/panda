//
// Copyright 2013, Roberto Paleari <roberto@greyhats.it>
//

#pragma once
#include "panda/plugin.h"

namespace qtrace {

/* Allocation of CPU registers */
void notify_taint_regalloc(target_ulong reg, const char *name);

/* End of translation block */
void notify_taint_endtb(void);

/* Set taint label */
void notify_taint_memory(target_ulong addr, unsigned int size, int label);
void notify_taint_register(bool istmp, unsigned char regno, int label);

/* Check taint status */
bool notify_taint_check_memory(target_ulong addr, unsigned int size);

/* Data movement */
void notify_taint_moveM2R(target_ulong addr, bool is_addr_tmp,
                          target_ulong addr_reg, int size, bool istmp,
                          target_ulong reg);
void notify_taint_moveR2M(bool istmp, target_ulong reg, target_ulong addr,
                          int size);
void notify_taint_moveR2R(bool srctmp, target_ulong src, bool dsttmp,
                          target_ulong dst);

/* Micro ops */

void notify_taint_micro_ld(target_ulong reg, target_ulong addr, uint32_t size);
void notify_taint_micro_st(target_ulong reg, target_ulong addr, uint32_t size);

/* Move a source (sub)register into destination (sub)register */
void notify_taint_moveR2R_offset(bool srctmp, target_ulong src,
                                 unsigned int srcoff, bool dsttmp,
                                 target_ulong dst, unsigned int dstoff,
                                 int size);

/* Data combination */
void notify_taint_combineR2R(bool srctmp, target_ulong src, bool dsttmp,
                             target_ulong dst);

/* Clear taint status */
void notify_taint_clearR(bool istmp, target_ulong reg);
void notify_taint_clearM(target_ulong addr, int size);

/* Assertions on registry taintedness, just for debugging */
void notify_taint_assert(target_ulong reg, bool istrue);

/* Notify a CPL switch */
// void notify_taint_cpl(target_ulong cr3, target_ulong newcpl);

/* Switch and query taint-tracker state */
// void notify_taint_set_state(bool state);
// bool notify_taint_get_state(void);

} // namespace qtrace
