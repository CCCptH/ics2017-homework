#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  // TODO();
  // ! In pa2, implement call ret32
  /**
   * - Operation
   *    IF rel16 or rel32 type of call
   *    THEN (* near relative call *)
   *       IF OperandSize = 16
   *       THEN
   *          Push(IP);
   *          EIP := (EIP + rel16) AND 0000FFFFH;
   *       ELSE (* OperandSize = 32 *)
   *          Push(EIP);
   *          EIP := EIP + rel32;
   *       FI;
   *    FI;
   * - Flags Affected
   *    All flags are affected if a task switch occurs; no flags
   *    are affected if a task switch does not occur
   */
  rtl_push(&decoding.seq_eip);
  decoding.is_jmp = 1;
  
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  // TODO();
  /**
   * - Operation
   *    IF instruction = near RET
   *    THEN;
   *      IF OperandSize = 16
   *      THEN
   *          IP := Pop();
   *          EIP := EIP AND 0000FFFFH;
   *      ELSE (* OperandSize = 32 *)
   *          EIP := Pop();
   *      FI;
   *      IF instruction has immediate operand THEN eSP := eSP + imm16; FI;
   *    FI;
   * - Description
   *    RET transfers control to a return address located on the stack. The
   *    address is usually placed on the stack by a CALL instruction, and
   *    the return is made to the instruction that follows the CALL.
   * 
   *    The optional numeric parameter to RET gives the number of stack bytes
   *    (OperandMode=16) or words (OperandMode=32) to be released after the
   *    return address is popped. These items are typically used as input
   *    parameters to the procedure called.
   * - Flags Affected
   *    None
   */
  rtl_pop(&decoding.jmp_eip);
  decoding.is_jmp = 1;
  print_asm("ret");
}

make_EHelper(call_rm) {
  // TODO();
  decoding.jmp_eip = id_dest->val;
  rtl_push(&decoding.seq_eip);
  decoding.is_jmp = 1;
  print_asm("call *%s", id_dest->str);
}


make_EHelper(clc) {
  rtl_clc();
  print_asm("clc");
}