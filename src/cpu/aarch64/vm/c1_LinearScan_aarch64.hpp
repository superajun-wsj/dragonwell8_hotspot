/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CPU_X86_VM_C1_LINEARSCAN_X86_HPP
#define CPU_X86_VM_C1_LINEARSCAN_X86_HPP

inline bool LinearScan::is_processed_reg_num(int reg_num) {
  return reg_num <= FrameMap::last_cpu_reg() || reg_num >= pd_nof_cpu_regs_frame_map;
}

inline int LinearScan::num_physical_regs(BasicType type) {
  return 1;
}


inline bool LinearScan::requires_adjacent_regs(BasicType type) {
  return false;
}

inline bool LinearScan::is_caller_save(int assigned_reg) {
  Unimplemented();
  return true; // no callee-saved registers on Intel

}


inline void LinearScan::pd_add_temps(LIR_Op* op) {
  // FIXME ??
}


// Implementation of LinearScanWalker

inline bool LinearScanWalker::pd_init_regs_for_alloc(Interval* cur) {
  if (allocator()->gen()->is_vreg_flag_set(cur->reg_num(), LIRGenerator::callee_saved)) {
    assert(cur->type() != T_FLOAT && cur->type() != T_DOUBLE, "cpu regs only");
    _first_reg = pd_first_callee_saved_reg;
    _last_reg = pd_last_callee_saved_reg;
    return true;
  } else if (cur->type() == T_INT || cur->type() == T_LONG || cur->type() == T_OBJECT || cur->type() == T_ADDRESS || cur->type() == T_METADATA) {
    _first_reg = pd_first_cpu_reg;
    _last_reg = pd_last_allocatable_cpu_reg;
    return true;
  }
  return false;
}



class FpuStackAllocator VALUE_OBJ_CLASS_SPEC {
 private:
  Compilation* _compilation;
  LinearScan* _allocator;

  LIR_OpVisitState visitor;

  LIR_List* _lir;
  int _pos;
  FpuStackSim _sim;
  FpuStackSim _temp_sim;

  bool _debug_information_computed;

  LinearScan*   allocator()                      { return _allocator; }
  Compilation*  compilation() const              { return _compilation; }

  // unified bailout support
  void          bailout(const char* msg) const   { compilation()->bailout(msg); }
  bool          bailed_out() const               { return compilation()->bailed_out(); }

  int pos() { return _pos; }
  void set_pos(int pos) { _pos = pos; }
  LIR_Op* cur_op() { Unimplemented(); return lir()->instructions_list()->at(pos()); }
  LIR_List* lir() { return _lir; }
  void set_lir(LIR_List* lir) { _lir = lir; }
  FpuStackSim* sim() { return &_sim; }
  FpuStackSim* temp_sim() { return &_temp_sim; }

  int fpu_num(LIR_Opr opr);
  int tos_offset(LIR_Opr opr);
  LIR_Opr to_fpu_stack_top(LIR_Opr opr, bool dont_check_offset = false);

  // Helper functions for handling operations
  void insert_op(LIR_Op* op);
  void insert_exchange(int offset);
  void insert_exchange(LIR_Opr opr);
  void insert_free(int offset);
  void insert_free_if_dead(LIR_Opr opr);
  void insert_free_if_dead(LIR_Opr opr, LIR_Opr ignore);
  void insert_copy(LIR_Opr from, LIR_Opr to);
  void do_rename(LIR_Opr from, LIR_Opr to);
  void do_push(LIR_Opr opr);
  void pop_if_last_use(LIR_Op* op, LIR_Opr opr);
  void pop_always(LIR_Op* op, LIR_Opr opr);
  void clear_fpu_stack(LIR_Opr preserve);
  void handle_op1(LIR_Op1* op1);
  void handle_op2(LIR_Op2* op2);
  void handle_opCall(LIR_OpCall* opCall);
  void compute_debug_information(LIR_Op* op);
  void allocate_exception_handler(XHandler* xhandler);
  void allocate_block(BlockBegin* block);

#ifndef PRODUCT
  void check_invalid_lir_op(LIR_Op* op);
#endif

  // Helper functions for merging of fpu stacks
  void merge_insert_add(LIR_List* instrs, FpuStackSim* cur_sim, int reg);
  void merge_insert_xchg(LIR_List* instrs, FpuStackSim* cur_sim, int slot);
  void merge_insert_pop(LIR_List* instrs, FpuStackSim* cur_sim);
  bool merge_rename(FpuStackSim* cur_sim, FpuStackSim* sux_sim, int start_slot, int change_slot);
  void merge_fpu_stack(LIR_List* instrs, FpuStackSim* cur_sim, FpuStackSim* sux_sim);
  void merge_cleanup_fpu_stack(LIR_List* instrs, FpuStackSim* cur_sim, BitMap& live_fpu_regs);
  bool merge_fpu_stack_with_successors(BlockBegin* block);

 public:
  LIR_Opr to_fpu_stack(LIR_Opr opr); // used by LinearScan for creation of debug information

  FpuStackAllocator(Compilation* compilation, LinearScan* allocator);
  void allocate();
};

#endif // CPU_X86_VM_C1_LINEARSCAN_X86_HPP
