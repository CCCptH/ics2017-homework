#include "cpu/exec.h"

make_EHelper(add) {
  // TODO();
  /**
   * - Operation:
   *    DEST := DEST + SRC
   * - Description:
   *    The result of the addition is assigned
   *    to the first operand (DEST),and the 
   *    flags are set accordingly.
   * - Flag Affected:
   *    OF, SF, ZF, AF?, CF PF?
   * - Flag performance
   *    OF: Overflow flag -- et if result is too large a positive
   *    number or too small a negative number (excluding sign-bit)
   *    to fit in destination operand; cleared otherwise.
   *    SF: Sign Flag -- Set equal to high-order bit of result (0 is
   *        positive, 1 if negative).
   *    ZF: Zero Flag -- Set if result is zero; cleared otherwise.
   *    CF: Carry Flag -- Set on high-order bit carry or borrow;
   *        cleared otherwise.
   */

  // evaluate the result
  rtl_add(&t2, &id_dest->val, &id_src->val);

  // write the result
  operand_write(id_dest, &t2);

  // CF
  rtl_sltu(&t0, &t2, &id_dest->val);
  rtl_set_CF(&t0);

  // OF
  // ?Why
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  // update ZF ,SF
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(add);
}

make_EHelper(sub) {
  // TODO();
  /**
   * - Operation:
   *    IF SRC is a byte and DEST is a word or dword
   *    THEN DEST := DEST - SignExtend(SRC);
   *    ELSE DEST := DEST - SRC;
   *    FI;
   * - Description:
   *    SUB subtracts the second operand (SRC) from the first 
   *    operand (DEST). The first operand is assigned the result
   *    of the subtraction, and the flags are set accordingly.
   *    When an immediate byte value is subtracted from a word
   *    operand, the immediate value is first sign-extended to
   *    the size of the destination operand.
   * -Flag Affect:
   *    OF, SF, ZF, CF
   */
  // evaluate
  rtl_sub(&t2, &id_dest->val, &id_src2->val);

  // write
  operand_write(id_dest, &t2);

  // update ZF, SF
  rtl_update_ZFSF(&t2, id_dest->width);

  // CF
  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_set_CF(&t0);

  // OF
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sub);
}

make_EHelper(cmp) {
  // TODO();
  /**
   * - Operation:
   *    LeftSRC - SignExtend(RightSRC);
   *    (* CMP does not store a result;
   *    its purpose is to set the flags *)
   * - Description:
   *    CMP subtracts the second operand from the first but,
   *    unlike the SUB instruction, does not store the result;
   *    only the flags are changed. CMP is typically used in
   *    conjunction with conditional jumps and the SETcc
   *    instruction. (Refer to Appendix D for the list of
   *    signed and unsigned flag tests provided.) If an operand
   *    greater than one byte is compared to an immediate byte,
   *    the byte value is first sign-extended.
   * - Flags Affected
   *    OF, SF, ZF, CF
   */

  // evaluate
  rtl_sub(&t2, &id_dest->val, &id_src->val);

  // Flags
  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);
  print_asm_template2(cmp);
}

make_EHelper(inc) {
  // TODO();
  /**
   * - Operation
   *    DEST := DEST + 1;
   * - Description
   *    INC adds 1 to the operand. It does not change the carry
   *    flag. To affect the carry flag, use the ADD instruction
   *    with a second operand of 1.
   * - Flags Affected
   *    OF, SF, ZF, CF
   */

  // evaluate
  rtl_addi(&t2, &id_dest->val, &id_src->val);

  // write
  operand_write(id_dest, &t2);

  // Flags
  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &t2, &id_dest->val);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template1(inc);
}

make_EHelper(dec) {
  // TODO();
  /**
   * - Operation:
   *    DEST := DEST - 1;
   * - Description:
   *    DEC subtracts 1 from the operand. DEC does not change
   *    the carry flag. To affect the carry flag, use the SUB
   *    instruction with an immediate operand of 1.
   * - Flags Affected:
   *    ZF, SF, OF, CF
   */

  // evaluate
  rtl_subi(&t2, &id_dest->val, 1);

  // write
  operand_write(id_dest, &t2);

  // update ZF, SF
  rtl_update_ZFSF(&t2, id_dest->width);

  // CF
  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_set_CF(&t0);

  // OF
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template1(dec);
}

make_EHelper(neg) {
  TODO();

  print_asm_template1(neg);
}

make_EHelper(adc) {
  rtl_add(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &t2, &id_dest->val);
  rtl_get_CF(&t1);
  rtl_add(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &t2, &id_dest->val);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &id_dest->val, &t2);
  rtl_get_CF(&t1);
  rtl_sub(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}

// imul with one operand
make_EHelper(imul1) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_imul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_src2->val, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_src2->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_li(&t1, 0);
      rtl_lr_w(&t0, R_AX);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_li(&t1, 0);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_div(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  switch (id_dest->width) {
    case 1:
      rtl_lr_w(&t0, R_AX);
      rtl_sext(&t0, &t0, 2);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_idiv(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(idiv);
}
