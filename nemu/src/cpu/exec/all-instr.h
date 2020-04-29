#include "cpu/exec.h"

make_EHelper(operand_size);

// special
make_EHelper(inv);
make_EHelper(nemu_trap);
make_EHelper(nop);

// control
make_EHelper(call);
make_EHelper(ret);
make_EHelper(jmp);      // todo: add to opcode_table
make_EHelper(jcc);      // todo: add to opcode_table
make_EHelper(jmp_rm);   // todo: add to opcode_table
make_EHelper(call_rm);  // todo: add to opcode_table

// arith
make_EHelper(add);      // todo: add to opcode_table
make_EHelper(sub);      // todo: add to opcode_table
make_EHelper(cmp);      // todo: add to opcode_table
make_EHelper(inc);      // todo: add to opcode_table
make_EHelper(dec);      // todo: add to opcode_table
make_EHelper(neg);      // todo: add to opcode_table
make_EHelper(adc);      // todo: add to opcode_table
make_EHelper(sbb);      // todo: add to opcode_table
make_EHelper(mul);      // todo: add to opcode_table
make_EHelper(imul1);    // todo: add to opcode_table
make_EHelper(imul2);    // todo: add to opcode_table
make_EHelper(imul3);    // todo: add to opcode_table
make_EHelper(div);      // todo: add to opcode_table
make_EHelper(idiv);     // todo: add to opcode_table

// data-mov
make_EHelper(mov);      // todo: add to opcode_table
make_EHelper(push);     // todo: add to opcode_table
make_EHelper(pop);      // todo: add to opcode_table
make_EHelper(pusha);    // todo: add to opcode_table
make_EHelper(popa);     // todo: add to opcode_table
make_EHelper(leave);    // todo: add to opcode_table
make_EHelper(cltd);     // todo: add to opcode_table
make_EHelper(cwtl);     // todo: add to opcode_table
make_EHelper(movsx);    // todo: add to opcode_table
make_EHelper(movzx);    // todo: add to opcode_table
make_EHelper(lea);      // todo: add to opcode_table

// logic
make_EHelper(test);     // todo: add to opcode_table
make_EHelper(and);      // todo: add to opcode_table
make_EHelper(xor);      // todo: add to opcode_table
make_EHelper(or);       // todo: add to opcode_table
make_EHelper(sar);      // todo: add to opcode_table
make_EHelper(shl);      // todo: add to opcode_table
make_EHelper(shr);      // todo: add to opcode_table
make_EHelper(setcc);    // todo: add to opcode_table
make_EHelper(not);      // todo: add to opcode_table

// system
make_EHelper(int);
make_EHelper(iret);
make_EHelper(lidt);
make_EHelper(mov_r2cr);
make_EHelper(mov_cr2r);
make_EHelper(in);
make_EHelper(out);