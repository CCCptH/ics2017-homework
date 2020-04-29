#include "cpu/exec.h"

make_EHelper(test) {
  // TODO();
  // test不影响操作结果
  rtl_and(&t0, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(test);
}

make_EHelper(rol) {
	rtl_shl(&t0, &id_dest->val, &id_src->val);
	rtl_shri(&t1, &id_dest->val, id_dest->width * 8 - id_src->val);
	rtl_or(&t2, &t1, &t0);
	operand_write(id_dest, &t2);

	print_asm_template2(rol);
}

make_EHelper(and) {
  // TODO();
  // ????
  rtl_and(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(and);
}

make_EHelper(xor) {
  // TODO();
  /**
   * - Operation
   *    DEST := LeftSRC XOR RightSRC
   *    CF := 0
   *    OF := 0
   * - Description
   *    XOR computes the exclusive OR of the two operands. Each bit of
   *    the result is 1 if the corresponding bits of the operands are
   *    different; each bit is 0 if the corresponding bits are the same.
   *    The answer replaces the first operand.
   * - Flags Affected
   *    CF, OF, SF, ZF
   */
  // evaluate
  rtl_xor(&t2, &id_dest->val, &id_src->val);

  // write
  operand_write(id_dest, &t2);

  // flags
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or) {
  // TODO();
  // evaluate
  rtl_or(&t2, &id_dest->val, &id_src->val);

  // write
  operand_write(id_dest, &t2);

  // flags
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  // ???
  // !!!
  rtl_sar(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, 4);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
	rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
	rtl_shr(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
	rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  // TODO();
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);
  print_asm_template1(not);
}
