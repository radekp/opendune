/* $Id$ */

#ifndef SPRITES_H
#define SPRITES_H

extern csip32 *g_sprites;

extern void Sprites_Load(uint16 index, uint16 memory, csip32 *sprites);
extern csip32 Sprites_GetCSIP(csip32 buffer_csip, uint16 index);

extern void emu_Sprites_Load();
extern void emu_Sprites_GetCSIP();

#endif /* SPRITES_H */
