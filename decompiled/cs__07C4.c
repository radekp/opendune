/* $Id$ */

#include "types.h"
#include "libemu.h"
#include "decompiled.h"

/**
 * Decompiled function f__07C4_001A_0045_DCB4()
 *
 * @name f__07C4_001A_0045_DCB4
 * @implements 07C4:001A:0045:DCB4 ()
 *
 * Called From: 0F3F:0193:0008:77D3
 */
void f__07C4_001A_0045_DCB4()
{
	emu_push(emu_bp);
	emu_movw(&emu_bp, emu_sp);
	emu_push(emu_bx);
	emu_push(emu_cx);
	emu_push(emu_dx);
	emu_xorw(&emu_bx, emu_bx);
	emu_movw(&emu_dx, emu_get_memory16(emu_ss, emu_bp,  0x8));
	emu_movw(&emu_cx, emu_get_memory16(emu_ss, emu_bp,  0xC));
	emu_subw(&emu_dx, emu_cx);
	if (!emu_flags.sf) { f__07C4_002F_0030_5F7C(); return; }
	emu_incw(&emu_bx);
	emu_negw(&emu_dx, emu_dx);
	f__07C4_002F_0030_5F7C(); return;
}

/**
 * Decompiled function f__07C4_002F_0030_5F7C()
 *
 * @name f__07C4_002F_0030_5F7C
 * @implements 07C4:002F:0030:5F7C ()
 *
 * Called From: 07C4:002A:0045:DCB4
 */
void f__07C4_002F_0030_5F7C()
{
	emu_shlw(&emu_bx, 0x1);
	emu_movw(&emu_ax, emu_get_memory16(emu_ss, emu_bp,  0x6));
	emu_movw(&emu_cx, emu_get_memory16(emu_ss, emu_bp,  0xA));
	emu_subw(&emu_cx, emu_ax);
	if (!emu_flags.sf) { f__07C4_003E_0021_782D(); return; }
	emu_incw(&emu_bx);
	emu_negw(&emu_cx, emu_cx);
	f__07C4_003E_0021_782D(); return;
}

/**
 * Decompiled function f__07C4_003E_0021_782D()
 *
 * @name f__07C4_003E_0021_782D
 * @implements 07C4:003E:0021:782D ()
 *
 * Called From: 07C4:0039:0045:DCB4
 * Called From: 07C4:0039:0030:5F7C
 */
void f__07C4_003E_0021_782D()
{
	emu_cmpw(&emu_cx, emu_dx);
	if (emu_flags.cf) { f__07C4_0044_001B_B5CB(); return; }
	emu_xchgw(&emu_dx, &emu_cx);
	f__07C4_0044_001B_B5CB(); return;
}

/**
 * Decompiled function f__07C4_0044_001B_B5CB()
 *
 * @name f__07C4_0044_001B_B5CB
 * @implements 07C4:0044:001B:B5CB ()
 *
 * Called From: 07C4:0040:0021:782D
 * Called From: 07C4:0040:0045:DCB4
 * Called From: 07C4:0040:0030:5F7C
 */
void f__07C4_0044_001B_B5CB()
{
	emu_rclw(&emu_bx, 0x1);
	emu_movw(&emu_ax, emu_dx);
	emu_incw(&emu_ax);
	emu_shrw(&emu_ax, 0x1);
	emu_cmpw(&emu_cx, emu_ax);
	emu_rclw(&emu_bx, 0x1);
	emu_xorw(&emu_ax, emu_ax);
	emu_movb(&emu_al, emu_get_memory8(emu_cs, emu_bx, 0xA));
	emu_movb(&emu_cl, 0x5);
	emu_shlw(&emu_ax, emu_cl);
	emu_pop(&emu_dx);
	emu_pop(&emu_cx);
	emu_pop(&emu_bx);
	emu_pop(&emu_bp);

	/* Return from this function */
	emu_pop(&emu_ip);
	emu_pop(&emu_cs);
	return;
}
