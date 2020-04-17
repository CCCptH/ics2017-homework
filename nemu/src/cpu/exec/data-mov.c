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
  /**
   * - Description
   *    POP replaces the previous contents of the memory, the register, or 
   *    the segment register operand with the word on the top of the 80386
   *    stack, addressed by SS:SP (address-size attribute of 16 bits) or 
   *    SS:ESP (addresssize attribute of 32 bits). The stack pointer SP is
   *    incremented by 2 for an operand-size of 16 bits or by 4 for an
   *    operand-size of 32 bits. It then points to the new top of stack.
   * 
   *    POP CS is not an 80386 instruction. Popping from the stack into the
   *    CS register is accomplished with a RET instruction.
   *    
   *    If the destination operand is a segment register (DS, ES, FS, GS, 
   *    or SS), the value popped must be a selector. In protected mode, 
   *    loading the selector initiates automatic loading of the descriptor
   *    information associated with that selector into the hidden part of
   *    the segment register; loading also initiates validation of both the
   *    selector and the descriptor information.
   * 
   *    A null value (0000-0003) may be popped into the DS, ES, FS, or GS 
   *    register without causing a protection exception. An attempt to 
   *    reference a segment whose corresponding segment register is loaded 
   *    with a null value causes a #GP(0) exception. No memory reference
   *    occurs. The saved value of the segment register is null.
   * 
   *    A POP SS instruction inhibits all interrupts, including NMI, until
   *    after execution of the next instruction. This allows sequential
   *    execution of POP SS and POP eSP instructions without danger of
   *    having an invalid stack during an interrupt. However, use of the
   *    LSS instruction is the preferred method of loading the SS and eSP
   *    registers.
   *    
   *    Loading a segment register while in protected mode results in special
   *    checks and actions, as described in the following listing:
   *    
   *    IF SS is loaded:
   *       IF selector is null THEN #GP(0);
   *       Selector index must be within its descriptor table limits ELSE
   *          #GP(selector);
   *       Selector's RPL must equal CPL ELSE #GP(selector);
   *       AR byte must indicate a writable data segment ELSE #GP(selector);
   *       DPL in the AR byte must equal CPL ELSE #GP(selector);
   *       Segment must be marked present ELSE #SS(selector);
   *       Load SS register with selector;
   *       Load SS register with descriptor;
   *    
   *    IF DS, ES, FS or GS is loaded with non-null selector:
   *       AR byte must indicate data or readable code segment ELSE
   *          #GP(selector);
   *       IF data or nonconforming code
   *       THEN both the RPL and the CPL must be less than or equal to DPL in
   *          AR byte
   *       ELSE #GP(selector);
   *       FI;
   *       Segment must be marked present ELSE #NP(selector);
   *       Load segment register with selector;
   *       Load segment register with descriptor;
   *    
   *    IF DS, ES, FS, or GS is loaded with a null selector:
   *       Load segment register with selector
   *       Clear valid bit in invisible portion of register
   * - Flags Affected
   *    None
  */
  rtl_pop(&id_dest->val);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  // TODO();
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
