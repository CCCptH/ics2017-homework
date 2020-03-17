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
   *
   *    IF (PE = 0 OR (PE = 1 AND VM = 1))
   *    (* real mode or virtual 8086 mode *)
   *       AND instruction = far CALL
   *       (* i.e., operand type is m16:16, m16:32, ptr16:16, ptr16:32 *)
   *    IF r/m16 or r/m32 type of call
   *    THEN (* near absolute call *)
   *       IF OperandSize = 16
   *       THEN
   *          Push(IP);
   *          EIP := [r/m16] AND 0000FFFFH;
   *       ELSE (* OperandSize = 32 *)
   *          Push(EIP);
   *          EIP := [r/m32];
   *       FI;
   *    FI;THEN
   *    IF OperandSize = 16
   *    THEN
   *       Push(CS);
   *       Push(IP); (* address of next instruction; 16 bits *)
   *    ELSE
   *       Push(CS); (* padded with 16 high-order bits *)
   *       Push(EIP); (* address of next instruction; 32 bits *)
   *    FI;
   *    IF operand type is m16:16 or m16:32
   *    THEN (* indirect far call *)
   *       IF OperandSize = 16
   *       THEN
   *          CS:IP := [m16:16];
   *          EIP := EIP AND 0000FFFFH; (* clear upper 16 bits *)
   *       ELSE (* OperandSize = 32 *)
   *          CS:EIP := [m16:32];
   *       FI;
   *    FI;
   *    IF operand type is ptr16:16 or ptr16:32
   *    THEN (* direct far call *)
   *       IF OperandSize = 16
   *       THEN
   *          CS:IP := ptr16:16;
   *          EIP := EIP AND 0000FFFFH; (* clear upper 16 bits *)
   *       ELSE (* OperandSize = 32 *)
   *          CS:EIP := ptr16:32;
   *       FI;
   *    FI;
   * FI;
   * 
   *    The action of the different forms of the instruction are
   *    described below.
   * 
   *    Near calls are those with destinations of type r/m16,
   *    r/m32, rel16, rel32; changing or saving the segment 
   *    register value is not necessary. The CALL rel16 and
   *    CALL rel32 forms add a signed offset to the address
   *    of the instruction following CALL to determine the
   *    destination. The rel16 form is used when the
   *    instruction's operand-size attribute is 16 bits; rel32
   *    is used when the operand-size attribute is 32 bits. The
   *    result is stored in the 32-bit EIP register. With rel16,
   *    the upper 16 bits of EIP are cleared, resulting in an
   *    offset whose value does not exceed 16 bits. CALL r/m16
   *    and CALL r/m32 specify a register or memory location
   *    from which the absolute segment offset is fetched. The
   *    offset fetched from r/m is 32 bits for an operand-size
   *    attribute of 32 (r/m32), or 16 bits for an operand-size
   *    of 16 (r/m16). The offset of the instruction following
   *    CALL is pushed onto the stack. It will be popped by a near
   *    RET instruction within the procedure. The CS register is
   *    not changed by this form of CALL.
   * 
   *    The far calls, CALL ptr16:16 and CALL ptr16:32, use a
   *    four-byte or six-byte operand as a long pointer to the
   *    procedure called. The CALL m16:16 and m16:32 forms fetch
   *    the long pointer from the memory location specified
   *    (indirection). In Real Address Mode or Virtual 8086 Mode,
   *    the long pointer provides 16 bits for the CS register and
   *    16 or 32 bits for the EIP register (depending on the
   *    operand-size attribute). These forms of the instruction
   *    push both CS and IP or EIP as a return address.
   * 
   *    For more information on Protected Mode control transfers,
   *    refer to Chapter 6 and Chapter 7.
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
  TODO();

  print_asm("call *%s", id_dest->str);
}
