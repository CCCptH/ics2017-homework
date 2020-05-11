#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  // TODO();
  // !in pa2, implement push r32 only
  /**
   * - Operation
   *    IF StackAddrSize = 16
   *    THEN
   *      IF OperandSize = 16 THEN
   *          SP := SP - 2;
   *          (SS:SP) := (SOURCE); (* word assignment *)
   *      ELSE
   *          SP := SP - 4;
   *          (SS:SP) := (SOURCE); (* dword assignment *)
   *      FI;
   *    ELSE (* StackAddrSize = 32 *)
   *      IF OperandSize = 16
   *      THEN
   *          ESP := ESP - 2;
   *          (SS:ESP) := (SOURCE); (* word assignment *)
   *      ELSE
   *          ESP := ESP - 4;
   *          (SS:ESP) := (SOURCE); (* dword assignment *)
   *      FI;
   *    FI;
   * - Description
   *    PUSH decrements the stack pointer by 2 if the operand-size attribute
   *    of the instruction is 16 bits; otherwise, it decrements the stack
   *    pointer by 4. PUSH then places the operand on the new top of stack,
   *    which is pointed to by the stack pointer.
   * 
   *    The 80386 PUSH eSP instruction pushes the value of eSP as it existed
   *    before the instruction. This differs from the 8086, where PUSH SP
   *    pushes the new value (decremented by 2).
   * 
   * - Flags Affected
   *    None
   */
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  // TODO();
  // !in pa2, only implement pop r32 only;
  // ???
  // rtl_pop(&id_dest->val);
  rtl_pop(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();
  //if (decoding.is_operand_size_16) {
  //  t0 = reg_w(4);
  //  rtl_push(&reg_w(0));
  //  rtl_push(&reg_w(1));
  //  rtl_push(&reg_w(2));
  //  rtl_push(&reg_w(3));
  //  rtl_push(&t0);
  //  rtl_push(&reg_w(5));
  //  rtl_push(&reg_w(6));
  //  rtl_push(&reg_w(7));
  //}
  //else {
    t0 = cpu.esp;
    rtl_push(&cpu.eax);
    rtl_push(&cpu.ecx);
    rtl_push(&cpu.edx);
    rtl_push(&cpu.ebx);
    rtl_push(&t0);
    rtl_push(&cpu.ebp);
    rtl_push(&cpu.esi);
    rtl_push(&cpu.edi);
  //}
  print_asm("pusha");
}

make_EHelper(popa) {
  // TODO();
  rtlreg_t throwaway;
  //if (decoding.is_operand_size_16) {
  //  rtl_pop(&reg_w(7));
  //  rtl_pop(&reg_w(6));
  //  rtl_pop(&reg_w(5));
  //  rtl_pop(&throwaway);
  //  rtl_pop(&reg_w(3));
  //  rtl_pop(&reg_w(2));
  //  rtl_pop(&reg_w(1));
  //  rtl_pop(&reg_w(0));
  //}
  //else {
    rtl_pop(&cpu.edi);
    rtl_pop(&cpu.esi);
    rtl_pop(&cpu.ebp);
    rtl_pop(&throwaway);
    rtl_pop(&cpu.ebx);
    rtl_pop(&cpu.edx);
    rtl_pop(&cpu.ecx);
    rtl_pop(&cpu.eax);
  //}
  print_asm("popa");
}

make_EHelper(leave) {
  // TODO();
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    // TODO();
    rtl_lr_w(&t0, R_AX);
    if((int32_t)(int16_t)(uint16_t)t0 < 0) {
      rtl_addi(&t1, &tzero, 0xffff);
      rtl_sr_w(R_DX, &t1);
    }
    else {
      rtl_sr_w(R_DX, &tzero);
    }
  }
  else {
    // TODO();
    rtl_lr_l(&t0, R_AX);
    if((int32_t)t0 < 0) {
      rtl_addi(&t1, &tzero,0xffffffff);
      rtl_sr_l(R_DX, &t1);
    }
    else {
      rtl_sr_l(R_DX, &tzero);
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    // TODO();
    rtl_lr_b(&t0, R_AL);
    t0 = (int16_t)(int8_t)(uint8_t)t0;
    rtl_sr_w(R_AX, &t0);
  }
  else {
    // TODO();
    rtl_sext(&cpu.eax, &cpu.eax, 2);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
