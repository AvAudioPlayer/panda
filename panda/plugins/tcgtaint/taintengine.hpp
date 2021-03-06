//
// Copyright 2013, Roberto Paleari <roberto@greyhats.it>
//

#ifndef SRC_QTRACE_TAINT_TAINTENGINE_H_
#define SRC_QTRACE_TAINT_TAINTENGINE_H_

#include <bitset>
#include <cassert>
#include <memory>
#include <set>

#include "shadow.hpp"

const int NUM_CPU_REGS = 16;  // CPU_NB_REGS in cpu.h
const int NUM_TMP_REGS = 512; // TCG_MAX_TEMPS in tcg.h

enum RegisterKind : int { global = 0, temporary = 1 };

//
// The TaintEngine class implements the logic of the taint engine.
//
// The TaintEngine basically wraps a ShadowMemory object and multiple
// ShadowRegister instances, exposing APIs to set, clear and propagate taint
// information.
//
class TaintEngine {
  public:
    explicit TaintEngine(){
        //WARNING TODO(vigliag): THIS IS A VERY BAD HACK, we are telling it to always
        //consider esp and ebp untainted
        //also: these are hardcoded for x86
        //also: the register cache may create problems
        cpuregs_[1].ignore(); //no idea what they are
        cpuregs_[2].ignore(); //no idea what they are
        cpuregs_[11].ignore(); //esp
        cpuregs_[12].ignore(); //ebp
        for(int i=13; i<=15; i++){ //TODO better continue till 28? we'll find out
            cpuregs_[i].ignore(); //segments bases
        }
    }

    // Enable/disable the taint propagation engine
    // void setEnabled(bool status);

    // Set the "global" status of the taint propagation engine. This is
    // different from setEnabled(): the former is used to selectively
    // enable/disable taint-tracking during execution (e.g., enable only in
    // ring-0), while the latter lets the user to permanently disable the
    // taint-tracking module void setUserEnabled(bool status);

    // Retrieve the current status of the taint propagation engine, as set by
    // the user bool isUserEnabled();

    // Registers names
    void setRegisterName(target_ulong reg, const char *name);
    const char *getRegisterName(target_ulong regno);
    bool getRegisterIdByName(const char *name, target_ulong &regno) const;

    // Add taint labels
    void setTaintedMemory(int label, target_ulong addr, unsigned int size);
    void setTaintedRegister(int label, RegisterKind istmp, target_ulong reg);

    // Copy taint labels to the provided set
    void copyMemoryLabels(std::set<int> &labels, target_ulong addr,
                          unsigned int size = 1) const;

    // Check taintedness
    bool isTaintedMemory(target_ulong addr, unsigned int size = 1) const;
    bool isTaintedRegister(RegisterKind tmp, target_ulong regno,
                           unsigned int offset, int size = -1);

    inline bool isTaintedRegister(RegisterKind istmp,
                                  target_ulong regno) const {
        switch (istmp) {
        case RegisterKind::temporary:
            return regcache_tmp_[regno];
        case RegisterKind::global:
            //TODO(vigliag) REALLY BAD HACK FOR IGNORING ESP and similars
            if(cpuregs_[regno].isIgnored()){return false;}
            return regcache_cpu_[regno];
        }
        return 0;
    }

    inline bool hasRegisterLabel(RegisterKind tmp, target_ulong reg,
                                 int label) {
        return getRegister(RegisterKind(tmp), reg)->hasLabel(label);
    }

    inline bool hasMemoryLabel(target_ulong addr, int label) {
        return mem_.hasLabel(addr, label);
    }

    // Clear temporary registers (e.g., at the end of a BB)
    void clearTempRegisters();

    // Untaint register (e.g., for immediate assignment operations)
    void clearRegister(RegisterKind tmp, target_ulong reg);

    // Clear a memory region
    void clearMemory(target_ulong addr, int size = 1);

    // Register and memory assignment (move) operations
    void moveR2R(RegisterKind srctmp, target_ulong src, RegisterKind dsttmp,
                 target_ulong dst);
    void moveR2M(RegisterKind regtmp, target_ulong reg, target_ulong addr,
                 unsigned size);
    void moveM2R(target_ulong addr, RegisterKind addr_reg_kind, target_ulong addr_reg, unsigned size, RegisterKind regkind,
                 target_ulong reg);

    void moveR2MicroM(RegisterKind regtmp, target_ulong reg, target_ulong addr,
                      unsigned size);
    void moveMicroM2R(target_ulong addr, unsigned size, RegisterKind regkind,
                      target_ulong reg);

    // Register assignment, with offset (e.g., mov dl, ah)
    void moveR2R(RegisterKind srctmp, target_ulong src, unsigned int srcoff,
                 RegisterKind dsttmp, target_ulong dst, unsigned dstoff,
                 int size);

    // Register and memory combinations (OR) operations
    void combineR2R(RegisterKind srctmp, target_ulong src, RegisterKind dsttmp,
                    target_ulong dst);
    void combineR2M(RegisterKind regtmp, target_ulong reg, target_ulong addr,
                    unsigned size);
    void combineM2R(target_ulong addr, unsigned size, RegisterKind regtmp,
                    target_ulong reg);

    // convenience function added to allow a c-compatible api
    const std::set<int> *getMemoryLabels(target_ulong addr) const;
    void set_taint_dereference(bool enabled){
        taint_dereference_enabled_ = enabled;
    }
  private:
    // Caches used to efficiently check if a register is tainted
    std::bitset<NUM_CPU_REGS> regcache_cpu_;
    std::bitset<NUM_TMP_REGS> regcache_tmp_;

    bool taint_dereference_enabled_ = false;

    // Shadow registers and memory
    ShadowRegister cpuregs_[NUM_CPU_REGS];
    ShadowRegister tmpregs_[NUM_TMP_REGS];
    ShadowMemory mem_;
    ShadowMemory cpuarchstate_;

    ShadowRegister *getRegister(RegisterKind regkind, target_ulong reg);

    inline void _updateRegisterCache(bool istmp, target_ulong regno,
                                     bool tainted) {
        if (istmp) {
            regcache_tmp_[regno] = tainted;
        } else {
            regcache_cpu_[regno] = tainted;
        }
    }
};

#endif // SRC_QTRACE_TAINT_TAINTENGINE_H_
