/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "libemu.h"
#include "../global.h"
#include "font.h"
#include "gui.h"
#include "../gfx.h"
#include "../os/strings.h"
#include "../os/sleep.h"
#include "../unknown/unknown.h"
#include "../house.h"
#include "../interrupt.h"
#include "../load.h"
#include "../map.h"
#include "../structure.h"
#include "../unit.h"
#include "../os/math.h"
#include "../sprites.h"
#include "../codec/format80.h"
#include "../pool/pool.h"
#include "../pool/house.h"
#include "../pool/structure.h"
#include "../pool/unit.h"
#include "../string.h"
#include "../tools.h"
#include "widget.h"
#include "mentat.h"
#include "../mouse.h"
#include "../wsa.h"
#include "../file.h"
#include "../opendune.h"
#include "../ini.h"

extern void emu_Tools_Malloc();
extern void emu_Tools_Free();
extern void emu_Tools_GetFreeMemory();
extern void f__29E8_07FA_0020_177A();
extern void emu_Input_HandleInput();
extern void emu_Input_History_Clear();
extern void emu_Input_Keyboard_NextKey();

MSVC_PACKED_BEGIN
typedef struct struct_B4E9 {
	/* 0000(4)   */ PACK csip32 variable_00;                /*!< ?? */
	/* 0004(2)   */ PACK uint16 variable_04;                /*!< ?? */
	/* 0006(2)   */ PACK uint16 variable_06;                /*!< ?? */
	/* 0008(2)   */ PACK uint16 variable_08;                /*!< ?? */
	/* 000A(2)   */ PACK uint16 variable_0A;                /*!< ?? */
} GCC_PACKED struct_B4E9;
MSVC_PACKED_END
assert_compile(sizeof(struct_B4E9) == 0x0C);

MSVC_PACKED_BEGIN
typedef struct ClippingArea {
	/* 0000(2)   */ PACK uint16 left;                       /*!< ?? */
	/* 0002(2)   */ PACK uint16 top;                        /*!< ?? */
	/* 0004(2)   */ PACK uint16 right;                      /*!< ?? */
	/* 0006(2)   */ PACK uint16 bottom;                     /*!< ?? */
} GCC_PACKED ClippingArea;
MSVC_PACKED_END
assert_compile(sizeof(ClippingArea) == 0x08);

MSVC_PACKED_BEGIN
typedef struct StrategicMapData {
	/* 0000(2)   */ PACK int16 index;      /*!< ?? */
	/* 0002(2)   */ PACK int16 arrow;      /*!< ?? */
	/* 0004(2)   */ PACK int16 offsetX;    /*!< ?? */
	/* 0006(2)   */ PACK int16 offsetY;    /*!< ?? */
} GCC_PACKED StrategicMapData;
MSVC_PACKED_END
assert_compile(sizeof(StrategicMapData) == 0x8);

static uint8 g_colours[16];
static ClippingArea g_clipping = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };

/**
 * Draw a wired rectangle.
 * @param left The left position of the rectangle.
 * @param top The top position of the rectangle.
 * @param right The right position of the rectangle.
 * @param bottom The bottom position of the rectangle.
 * @param colour The colour of the rectangle.
 */
void GUI_DrawWiredRectangle(uint16 left, uint16 top, uint16 right, uint16 bottom, uint8 colour)
{
	GUI_DrawLine(left, top, right, top, colour);
	GUI_DrawLine(left, bottom, right, bottom, colour);
	GUI_DrawLine(left, top, left, bottom, colour);
	GUI_DrawLine(right, top, right, bottom, colour);
}

/**
 * Draw a filled rectangle.
 * @param left The left position of the rectangle.
 * @param top The top position of the rectangle.
 * @param right The right position of the rectangle.
 * @param bottom The bottom position of the rectangle.
 * @param colour The colour of the rectangle.
 */
void GUI_DrawFilledRectangle(int16 left, int16 top, int16 right, int16 bottom, uint8 colour)
{
	uint16 x;
	uint16 y;
	uint16 height;
	uint16 width;

	uint8 *screen = &emu_get_memory8(GFX_Screen_GetSegmentActive(), 0x0, 0x0);

	if (left >= SCREEN_WIDTH) return;
	if (left < 0) left = 0;

	if (top >= SCREEN_HEIGHT) return;
	if (top < 0) top = 0;

	if (right >= SCREEN_WIDTH) right = SCREEN_WIDTH - 1;
	if (right < 0) right = 0;

	if (bottom >= SCREEN_HEIGHT) bottom = SCREEN_HEIGHT - 1;
	if (bottom < 0) bottom = 0;

	if (left > right) return;
	if (top > bottom) return;

	screen += left + top * SCREEN_WIDTH;
	width = right - left + 1;
	height = bottom - top + 1;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*screen++ = colour;
		}
		screen += SCREEN_WIDTH - width;
	}
}

/**
 * Display a text.
 * @param str The text to display.
 * @param arg0A ??.
 * @param ... The args for the text.
 */
void GUI_DisplayText(const char *str, uint16 arg0A, ...)
{
	char buffer[80];
	buffer[0] = '\0';

	if (str != NULL) {
		va_list ap;

		va_start(ap, arg0A);
		vsnprintf(buffer, sizeof(buffer), str, ap);
		va_end(ap);
	}

	if (arg0A == 0xFFFF) {
		g_global->variable_3734 = 0xFFFF;
		g_global->variable_3736 = 0xFFFF;
		g_global->variable_3738 = 0xFFFF;

		g_global->variable_3644[0] = '\0';
		g_global->variable_3694[0] = '\0';
		g_global->variable_36E4[0] = '\0';

		g_global->variable_373A = 0;
		g_global->variable_373C = 0;
		return;
	}

	if (arg0A == 0xFFFE) {
		if (g_global->variable_373A == 0) {
			g_global->variable_3736 = 0xFFFF;
			g_global->variable_3694[0] = '\0';
		}
		g_global->variable_3738 = 0xFFFF;
		g_global->variable_36E4[0] = '\0';
	}

	if (g_global->variable_373A != 0) {
		uint16 oldValue_07AE_0000;
		uint16 loc06;

		if (buffer[0] != '\0') {
			if (strcasecmp(buffer, g_global->variable_3694) != 0 && (int16)arg0A >= (int16)g_global->variable_3738) {
				strcpy(g_global->variable_36E4, buffer);
				g_global->variable_3738 = arg0A;
			}
		}
		if ((int32)g_global->variable_373C > (int32)g_global->variable_76AC) return;

		oldValue_07AE_0000 = Unknown_07AE_0000(7);

		if (g_global->variable_38C4 != 0) {
			uint16 oldScreenID = GUI_Screen_SetActive(2);

			assert(g_global->variable_6668.csip == 0x22A60D31);

			GUI_DrawFilledRectangle(0, 0, SCREEN_WIDTH - 1, 23, (uint8)g_global->variable_6D59);

			GUI_DrawText_Wrapper(
				g_global->variable_3694,
				g_global->variable_992D << 3,
				2,
				g_global->variable_8ADA & 0xFF,
				0,
				0x012
			);

			GUI_DrawText_Wrapper(
				g_global->variable_3644,
				g_global->variable_992D << 3,
				13,
				g_global->variable_8AD8 & 0xFF,
				0,
				0x012
			);

			g_global->variable_38C4 = 0;

			GUI_Screen_SetActive(oldScreenID);
		}

		GUI_Mouse_Hide_InWidget(7);

		if (g_global->variable_3740 + g_global->variable_9931 > 24) {
			loc06 = 24 - g_global->variable_3740;
		} else {
			loc06 = g_global->variable_9931;
		}

		GUI_Screen_Copy(g_global->variable_992D, g_global->variable_3740, g_global->variable_992D, g_global->variable_992B, g_global->variable_992F, loc06, 2, 0);
		GUI_Mouse_Show_InWidget();

		Unknown_07AE_0000(oldValue_07AE_0000);

		if (g_global->variable_3740 != 0) {
			if ((int16)g_global->variable_3738 <= (int16)g_global->variable_3736) {
				g_global->variable_373C = g_global->variable_76AC + 1;
			}
			g_global->variable_3740--;
			return;
		}

		strcpy(g_global->variable_3644, g_global->variable_3694);

		g_global->variable_8AD8 = g_global->variable_8ADA;
		g_global->variable_3734 = (g_global->variable_3736 != 0) ? g_global->variable_3736 - 1 : 0;

		strcpy(g_global->variable_3694, g_global->variable_36E4);

		g_global->variable_3736 = g_global->variable_3738;
		g_global->variable_8ADA = g_global->variable_8ADC;
		g_global->variable_36E4[0] = '\0';
		g_global->variable_3738 = 0xFFFF;
		g_global->variable_38C4 = 1;
		g_global->variable_373C = g_global->variable_76AC + ((int16)g_global->variable_3736 <= (int16)g_global->variable_3734 ? 900 : 1);
		g_global->variable_373A = 0;
		return;
	}

	if (buffer[0] != '\0') {
		if (strcasecmp(buffer, g_global->variable_3644) != 0 && strcasecmp(buffer, g_global->variable_3694) != 0 && strcasecmp(buffer, g_global->variable_36E4) != 0) {
			if ((int16)arg0A >= (int16)g_global->variable_3736) {
				strcpy(g_global->variable_36E4, g_global->variable_3694);

				g_global->variable_8ADC = g_global->variable_8ADA;
				g_global->variable_3738 = g_global->variable_3736;

				strcpy(g_global->variable_3694, buffer);

				g_global->variable_8ADA = 12;
				g_global->variable_3736 = arg0A;
			} else if ((int16)arg0A >= (int16)g_global->variable_3738) {
				strcpy(g_global->variable_36E4, buffer);
				g_global->variable_3738 = arg0A;
				g_global->variable_8ADC = 12;
			}
		}
	} else {
		if (g_global->variable_3644[0] == '\0' && g_global->variable_3694[0] == '\0') return;
	}

	if ((int16)g_global->variable_3736 <= (int16)g_global->variable_3734 && (int32)g_global->variable_373C >= (int32)g_global->variable_76AC) return;

	g_global->variable_373A = 1;
	g_global->variable_3740 = 10;
	g_global->variable_373C = 0;
}

/**
 * Draw a char on the screen.
 *
 * @param c The char to draw.
 * @param x The most left position where to draw the string.
 * @param y The most top position where to draw the string.
 */
static void GUI_DrawChar(char c, uint16 x, uint16 y)
{
	csip32 font_csip = emu_get_csip32(0x22A6, 0x00, 0x80);
	uint8 *font      = emu_get_memorycsip(font_csip);
	uint8 *screen    = &emu_get_memory8(GFX_Screen_GetSegmentActive(), 0x0, 0x0);

	uint16 offset;
	uint16 remainingWidth;
	uint8  charWidth;
	uint8  charHeight;
	uint8  emptyLines;
	uint8  usedLines;
	uint8 i;

	if (font == NULL) return;

	offset     = ((uint16 *)font)[emu_get_memory16(0x22A6, 0x00, 0x74) / 2 + c];
	charWidth  = font[emu_get_memory16(0x22A6, 0x00, 0x76) + c];
	charHeight = font[emu_get_memory16(0x22A6, 0x00, 0x72) + 4];

	if (offset == 0) return;
	if (x >= SCREEN_WIDTH || (x + charWidth) > SCREEN_WIDTH) return;
	if (y >= SCREEN_HEIGHT || (y + charHeight) > SCREEN_HEIGHT) return;

	emptyLines  = font[emu_get_memory16(0x22A6, 0x00, 0x7A) + c * 2];
	usedLines   = font[emu_get_memory16(0x22A6, 0x00, 0x7A) + c * 2 + 1];
	charHeight -= emptyLines + usedLines;

	font += offset;

	x += y * SCREEN_WIDTH;
	remainingWidth = SCREEN_WIDTH - charWidth;

	if (emptyLines != 0) {
		if (g_colours[0] != 0) {
			while (emptyLines-- != 0) {
				for (i = 0; i < charWidth; i++) screen[x++] = g_colours[0];
				x += remainingWidth;
			}
		} else {
			x += emptyLines * SCREEN_WIDTH;
		}
	}

	if (usedLines == 0) return;

	while (usedLines-- != 0) {
		for (i = 0; i < charWidth; i++) {
			uint8 data = *font++;

			if (g_colours[data & 0xF] != 0) screen[x] = g_colours[data & 0xF];
			x++;

			if (++i == charWidth) break;

			if (g_colours[(data >> 4) & 0xF] != 0) screen[x] = g_colours[(data >> 4) & 0xF];
			x++;
		}
		x += remainingWidth;
	}

	if (charHeight <= 0) return;

	if (g_colours[0] == 0) return;

	while (charHeight-- != 0) {
		for (i = 0; i < charWidth; i++) screen[x++] = g_colours[0];
		x += remainingWidth;
	}
}

/**
 * Draw a string to the screen.
 *
 * @param string The string to draw.
 * @param left The most left position where to draw the string.
 * @param top The most top position where to draw the string.
 * @param fgColour The foreground colour of the text.
 * @param bgColour The background colour of the text.
 */
void GUI_DrawText(char *string, int16 left, int16 top, uint8 fgColour, uint8 bgColour)
{
	uint8 colours[2];
	uint8 *data;
	uint16 height;
	uint16 heightOffset;
	uint16 widthOffset;
	uint16 x;
	uint16 y;
	char *s;

	data = (uint8 *)emu_get_memorycsip(g_global->variable_99F3);
	if (data == NULL) return;

	heightOffset = ((uint16 *)data)[2];
	widthOffset  = ((uint16 *)data)[4];
	height = data[heightOffset + 4];

	if (left < 0) left = 0;
	if (top  < 0) top  = 0;
	if (left > SCREEN_WIDTH) return;
	if (top  > SCREEN_HEIGHT) return;

	colours[0] = bgColour;
	colours[1] = fgColour;

	GUI_InitColors(colours, 0, 1);

	s = string;
	x = left;
	y = top;
	while (*s != '\0') {
		uint16 width;

		if (*s == '\n' || *s == '\r') {
			x = left;
			y += height + g_global->variable_6C6E;

			while (*s == '\n' || *s == '\r') s++;
		}

		width = data[widthOffset + *s] + g_global->variable_6C6C;

		if (x + width > SCREEN_WIDTH) {
			x = left;
			y += height + g_global->variable_6C6E;
		}
		if (y > SCREEN_HEIGHT) break;

		GUI_DrawChar(*s, x, y);

		x += width;
		s++;
	}
}

/**
 * Draw a string to the screen, and so some magic.
 *
 * @param string The string to draw.
 * @param left The most left position where to draw the string.
 * @param top The most top position where to draw the string.
 * @param fgColour The foreground colour of the text.
 * @param bgColour The background colour of the text.
 * @param flags The flags of the string.
 */
void GUI_DrawText_Wrapper(char *string, int16 left, int16 top, uint8 fgColour, uint8 bgColour, uint16 flags, ...)
{
	uint8 arg12low = flags & 0xF;
	uint8 arg2mid  = flags & 0xF0;

	if ((arg12low != g_global->variable_376C && arg12low != 0) || string == NULL) {
		switch (arg12low) {
			case 1:  Font_Select(g_global->new6pFnt);  break;
			case 2:  Font_Select(g_global->new8pFnt2); break;
			default: Font_Select(g_global->new8pFnt);  break;
		}

		g_global->variable_376C = arg12low;
	}

	if ((arg2mid != g_global->variable_376E && arg2mid != 0) || string == NULL) {
		memset(g_global->variable_8ADE, 0, 16);

		switch (arg2mid) {
			case 0x0010:
				g_global->variable_8ADE[2] = 0;
				g_global->variable_8ADE[3] = 0;
				g_global->variable_6C6C = -2;
				break;

			case 0x0020:
				g_global->variable_8ADE[2] = 12;
				g_global->variable_8ADE[3] = 0;
				g_global->variable_6C6C = -1;
				break;

			case 0x0030:
				g_global->variable_8ADE[2] = 12;
				g_global->variable_8ADE[3] = 12;
				g_global->variable_6C6C = -1;
				break;

			case 0x0040:
				g_global->variable_8ADE[2] = 232;
				g_global->variable_8ADE[3] = 0;
				g_global->variable_6C6C = -1;
				break;
		}

		g_global->variable_8ADE[0] = bgColour;
		g_global->variable_8ADE[1] = fgColour;
		g_global->variable_8ADE[4] = 6;

		GUI_InitColors(g_global->variable_8ADE, 0, 15);

		g_global->variable_376E = arg2mid;
	}

	if (string == NULL) return;

	if (string != g_global->variable_8AEE) {
		char buf[256];
		va_list ap;

		strncpy(buf, string, sizeof(buf));

		va_start(ap, flags);
		vsnprintf(g_global->variable_8AEE, sizeof(g_global->variable_8AEE), buf, ap);
		va_end(ap);
	}

	switch (flags & 0x0F00) {
		case 0x100:
			left -= Font_GetStringWidth(g_global->variable_8AEE) / 2;
			break;

		case 0x200:
			left -= Font_GetStringWidth(g_global->variable_8AEE);
			break;
	}

	GUI_DrawText(g_global->variable_8AEE, left, top, fgColour, bgColour);
}

/**
 * Do something on the given colour in the given palette.
 *
 * @param palette The palette to work on.
 * @param colour The colour to modify.
 * @param reference The colour to use as reference.
 */
static bool GUI_Palette_2BA5_00A2(uint8 *palette, uint16 colour, uint16 reference)
{
	bool ret = false;
	uint16 i;

	colour *= 3;
	reference *= 3;

	for (i = 0; i < 3; i++) {
		if (palette[reference] != palette[colour]) {
			ret = true;
			palette[colour] += (palette[colour] > palette[reference]) ? -1 : 1;
		}
		colour++;
		reference++;
	}

	return ret;
}

/**
 * Animate the palette. Only works for some colours or something
 */
void GUI_PaletteAnimate()
{
	uint8 *palette = emu_get_memorycsip(g_global->variable_3C32);

	if (g_global->variable_31CE < g_global->variable_76AC) {
		uint16 colour;
		if (g_global->variable_37B2 != 0) {
			colour = 15;
		} else {
			colour = (g_global->variable_31D2 == 0) ? 15 : 6;
		}

		memcpy(palette + 3 * 239, palette + 3 * colour, 3);

		GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

		g_global->variable_31D2 = (g_global->variable_31D2 == 0) ? 1 : 0;
		g_global->variable_31CE = g_global->variable_76AC + 60;
	}

	if (g_global->variable_31CA < g_global->variable_76AC && g_global->selectionType != 0) {
		GUI_Palette_2BA5_00A2(palette, 255, g_global->variable_31D4);
		GUI_Palette_2BA5_00A2(palette, 255, g_global->variable_31D4);
		GUI_Palette_2BA5_00A2(palette, 255, g_global->variable_31D4);

		if (!GUI_Palette_2BA5_00A2(palette, 255, g_global->variable_31D4)) {
			if (g_global->variable_31D4 == 13) {
				g_global->variable_31D4 = 15;

				if (g_global->selectionType == 2) {
					if (g_global->variable_38EC != 0) {
						g_global->variable_31D4 = ((g_global->variable_38EC & 0x8000) != 0) ? 5 : 15;
					} else {
						g_global->variable_31D4 = 6;
					}
				}
			} else {
				g_global->variable_31D4 = 13;
			}
		}

		GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

		g_global->variable_31CA = g_global->variable_76AC + 3;
	}

	if (g_global->variable_31C6 < g_global->variable_76AC) {
		GUI_Palette_2BA5_00A2(palette, 223, g_global->variable_31D6);

		if (!GUI_Palette_2BA5_00A2(palette, 223, g_global->variable_31D6)) {
			if (g_global->variable_31D6 == 12) {
				g_global->variable_31D6 = 10;
			} else {
				g_global->variable_31D6 = 12;
			}
		}

		GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

		g_global->variable_31C6 = g_global->variable_76AC + 5;
	}

	Unknown_B483_0470();
}

/**
 * Sets productionStringID to the correct string for the active structure.
 */
void GUI_UpdateProductionStringID()
{
	Structure *s = NULL;

	s = Structure_Get_ByPackedTile(g_global->selectionPosition);

	g_global->productionStringID = 0;

	if (s == NULL) return;

	if (g_structureInfo[s->o.type].o.flags.s.factory) {
		if (s->o.flags.s.upgrading) {
			g_global->productionStringID = 0x90; /* "Upgrading|%d%% done" */
		} else {
			if (s->o.linkedID != 0xFF) {
				if (s->o.flags.s.onHold) {
					g_global->productionStringID = 0x28; /* "On hold" */
				} else {
					if (s->countDown != 0) {
						g_global->productionStringID = 0x2E; /* "%d%% done" */
					} else {
						if (s->o.type == STRUCTURE_CONSTRUCTION_YARD) {
							g_global->productionStringID = 0x26; /* "Place it" */
						} else {
							g_global->productionStringID = 0x27; /* "Completed" */
						}
					}
				}
			} else {
				g_global->productionStringID = 0x29; /* "Build it" */
			}
		}
	} else {
		if (s->o.type == STRUCTURE_PALACE) g_global->productionStringID = g_houseInfo[s->o.houseID].specialWeapon + 0x29;
	}
}

static void GUI_2599_000B(uint16 index, uint16 arg08, uint16 arg0A, uint16 arg0C, uint16 arg0E)
{
	g_global->variable_4062[index][0] = arg08;
	g_global->variable_4062[index][1] = arg0A;
	g_global->variable_4062[index][2] = arg0C;
	g_global->variable_4062[index][3] = arg0E;

	if (g_global->variable_6D5D == index) Unknown_07AE_0000(index);
}

/**
 * Displays a message and waits for a user action.
 * @param str The text to display.
 * @param spriteID The sprite to draw (0xFFFF for none).
 * @param ... The args for the text.
 * @return ??
 */
uint16 GUI_DisplayModalMessage(char *str, uint16 spriteID, ...)
{
	va_list ap;
	uint16 oldValue_07AE_0000;
	uint16 ret;
	uint16 oldScreenID;
	uint16 size = 0;

	va_start(ap, spriteID);
	vsnprintf(g_global->variable_87D8, sizeof(g_global->variable_87D8), str, ap);
	va_end(ap);

	GUI_Mouse_Hide_Safe();

	oldScreenID = GUI_Screen_SetActive(0);

	GUI_DrawText_Wrapper(NULL, 0, 0, 0, 0, 0x22);

	oldValue_07AE_0000 = Unknown_07AE_0000(1);

	g_global->variable_4062[1][3] = g_global->variable_6C71 * max(GUI_SplitText(g_global->variable_87D8, ((g_global->variable_992F - ((spriteID == 0xFFFF) ? 2 : 7)) << 3) - 6, '\r'), 3) + 18;

	Unknown_07AE_0000(1);

	if (g_global->variable_3600.csip == 0x0) {
		size = GFX_GetSize(g_global->variable_992F, g_global->variable_9931);

		emu_push(emu_cs); emu_push(0x0351); emu_cs = 0x23E1; emu_Tools_GetFreeMemory();

		if (((int32)((emu_dx << 16) | emu_ax)) >= size) {
			emu_push(0);
			emu_push(0); emu_push(size);
			emu_push(emu_cs); emu_push(0x0367); emu_cs = 0x23E1; emu_Tools_Malloc();
			emu_sp += 6;

			g_global->variable_3600.s.cs = emu_dx;
			g_global->variable_3600.s.ip = emu_ax;
		} else {
			size = 0;
		}
	}

	if (g_global->variable_3600.csip != 0x0) {
		GFX_CopyToBuffer(g_global->variable_992D, g_global->variable_992B, g_global->variable_992F, g_global->variable_9931, emu_get_memorycsip(g_global->variable_3600));
	}

	GUI_Widget_DrawBorder(1, 1, 1);

	if (spriteID != 0xFFFF) {
		GUI_DrawSprite(g_global->screenActiveID, g_sprites[spriteID], 7, 8, 1, 0x4000);
		GUI_2599_000B(1, g_global->variable_992D + 5, g_global->variable_992B + 8, g_global->variable_992F - 7, g_global->variable_9931 - 16);
	} else {
		GUI_2599_000B(1, g_global->variable_992D + 1, g_global->variable_992B + 8, g_global->variable_992F - 2, g_global->variable_9931 - 16);
	}

	g_global->variable_6D59 = 0;
	g_global->variable_9933 = 0;
	g_global->variable_9935 = 0;

	GUI_DrawText(g_global->variable_87D8, g_global->variable_992D << 3, g_global->variable_992B, g_global->variable_6D5B & 0xFF, g_global->variable_6D59 & 0xFF);

	GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

	GUI_Mouse_Show_Safe();

	g_global->variable_76B4 = 30;

	while (g_global->variable_76B4 != 0) GUI_PaletteAnimate();

	emu_push(emu_cs); emu_push(0x0496); emu_cs = 0x29E8; emu_Input_History_Clear();

	do {
		GUI_PaletteAnimate();

		emu_push(emu_cs); emu_push(0x04A0); emu_cs = 0x29E8; f__29E8_07FA_0020_177A();
		ret = emu_ax;
	} while (ret == 0 || (ret & 0x800) != 0);

	emu_push(0x841);
	emu_push(emu_cs); emu_push(0x04B9); emu_cs = 0x29E8; emu_Input_HandleInput();
	emu_sp += 2;

	GUI_Mouse_Hide_Safe();

	if (spriteID != 0xFFFF) {
		GUI_2599_000B(1, g_global->variable_992D - 5, g_global->variable_992B - 8, g_global->variable_992F + 7, g_global->variable_9931 + 16);
	} else {
		GUI_2599_000B(1, g_global->variable_992D - 1, g_global->variable_992B - 8, g_global->variable_992F + 2, g_global->variable_9931 + 16);
	}

	if (g_global->variable_3600.csip != 0x0) {
		GFX_CopyFromBuffer(g_global->variable_992D, g_global->variable_992B, g_global->variable_992F, g_global->variable_9931, emu_get_memorycsip(g_global->variable_3600));
	}

	Unknown_07AE_0000(oldValue_07AE_0000);

	if (size != 0) {
		emu_push(g_global->variable_3600.s.cs); emu_push(g_global->variable_3600.s.ip);
		emu_push(emu_cs); emu_push(0x0550); emu_cs = 0x23E1; emu_Tools_Free();
		emu_sp += 4;

		g_global->variable_3600.csip = 0;
	} else {
		g_global->variable_3A12 = 1;
	}

	GUI_Screen_SetActive(oldScreenID);

	GUI_Mouse_Show_Safe();

	return ret;
}

/**
 * Splits the given text in lines of maxwidth width using the given delimiter.
 * @param str The text to split.
 * @param maxwidth The maximum width the text will have.
 * @param delimiter The char used as delimiter.
 * @return The number of lines.
 */
uint16 GUI_SplitText(char *str, uint16 maxwidth, char delimiter)
{
	uint16 lines = 0;

	if (str == NULL) return 0;

	while (*str != '\0') {
		uint16 width = 0;

		lines++;

		while (width < maxwidth && *str != delimiter && *str != '\r' && *str != '\0') width += Font_GetCharWidth(*str++);

		if (width >= maxwidth) {
			while (*str != 0x20 && *str != delimiter && *str != '\r' && *str != '\0') width -= Font_GetCharWidth(*str--);
		}

		if (*str != '\0') *str++ = delimiter;
	}

	return lines;
}

/**
 * Draws a sprite.
 * @param screenID On which screen to draw the sprite.
 * @param sprite_csip The CS:IP of the sprite to draw.
 * @param posX ??.
 * @param posY ??.
 * @param windowID The ID of the window where the drawing is done.
 * @param flags The flags.
 * @param ... The extra args, flags dependant.
 */
void GUI_DrawSprite(uint16 screenID, csip32 sprite_csip, int16 posX, int16 posY, uint16 windowID, uint16 flags, ...)
{
	static uint16 s_variable_0E[8]  = {0x050E, 0x0545, 0x050E, 0x0545, 0x07A7, 0x0857, 0x07A7, 0x0857};
	static uint16 s_variable_1E[8]  = {0x050E, 0x0545, 0x050E, 0x0545, 0x07ED, 0x089D, 0x07ED, 0x089D};
	static uint16 s_variable_2E[8]  = {0x0530, 0x0569, 0x0530, 0x0569, 0x0812, 0x08C5, 0x0812, 0x08C5};
	static uint16 s_variable_3E[16] = {0x0580, 0x0584, 0x0599, 0x05C8, 0x05E0, 0x061F, 0x05EA, 0x05C8, 0x0634, 0x0653, 0x0683, 0x06C6, 0x06F9, 0x0777, 0x071E, 0x06C6};
	static uint16 s_variable_5E     = 0;
	static uint16 s_variable_60[8]  = {1, 3, 2, 5, 4, 3, 2, 1};
	static uint16 s_variable_70     = 1;
	static uint16 s_variable_72     = 0x8B55;
	static uint16 s_variable_74     = 0x51EC;

	va_list ap;

	int16  top;
	int16  bottom;
	uint16 width;
	uint16 loc10;
	int16  loc12;
	int16  loc14;
	int16  loc16;
	uint8  loc18;
	int16  loc1A;
	int16  loc1C;
	int16  loc1E;
	int16  loc20;
	uint16 loc22;
	uint8 *loc26 = NULL;
	int16  loc28 = 0;
	int16  loc2A;
	uint16 loc30 = 0;
	uint16 loc32;
	uint16 loc34;
	uint8 *loc38 = NULL;
	int16  loc3A;
	uint8 *loc3E = NULL;
	uint16 loc42;
	uint16 loc44;
	uint16 locbx;

	csip32 memBlock;
	uint8 *buf = NULL;
	uint8 *sprite = NULL;
	uint8 *b = NULL;
	int16  count;

	if (sprite_csip.csip == 0x0) return;

	sprite = emu_get_memorycsip(sprite_csip);

	if ((*sprite & 0x1) != 0) flags |= 0x400;

	va_start(ap, flags);

	if ((flags & 0x2000) != 0) loc3E = va_arg(ap, uint8*);

	if ((flags & 0x100) != 0) {
		loc26 = va_arg(ap, uint8*);
		loc28 = (int16)va_arg(ap, int);
		if (loc28 == 0) flags &= 0xFEFF;
	}

	if ((flags & 0x200) != 0) {
		s_variable_5E = (s_variable_5E + 1) % 8;
		s_variable_70 = s_variable_60[s_variable_5E];
		s_variable_74 = 0x0;
		s_variable_72 = 0x100;
	}

	if ((flags & 0x1000) != 0) s_variable_72 = (uint16)va_arg(ap, int);

	if ((flags & 0x800) != 0) {
		loc18 = (uint8)va_arg(ap, int);
	}

	if ((flags & 0x4) != 0) {
		loc30 = (uint16)va_arg(ap, int);
		loc32 = (uint16)va_arg(ap, int);
	} else {
		loc32 = 0x100;
	}

	va_end(ap);

	loc34 = 0;

	memBlock = Screen_GetSegment_ByIndex_2(screenID);
	memBlock.s.ip = g_global->variable_4062[windowID][0] << 3;

	buf = emu_get_memorycsip(memBlock);

	if ((flags & 0x4000) == 0) posX -= g_global->variable_4062[windowID][0] << 3;

	width = g_global->variable_4062[windowID][2] << 3;
	top = g_global->variable_4062[windowID][1];

	if ((flags & 0x4000) != 0) posY += g_global->variable_4062[windowID][1];

	bottom = g_global->variable_4062[windowID][1] + g_global->variable_4062[windowID][3];

	loc10 = *(uint16 *)sprite;
	sprite += 2;

	loc12 = *sprite++;

	if ((flags & 0x4) != 0) {
		loc12 *= loc32;
		loc12 >>= 8;
		if (loc12 == 0) return;
	}

	if ((flags & 0x8000) != 0) posY -= loc12 / 2;

	loc1A = *(uint16 *)sprite;
	sprite += 2;

	loc14 = loc1A;

	if ((flags & 0x4) != 0) {
		loc14 += loc30;
		loc14 >>= 8;
		if (loc14 == 0) return;
	}

	if ((flags & 0x8000) != 0) posX -= loc14 / 2;

	loc16 = loc14;

	sprite += 3;

	locbx = *(uint16 *)sprite;
	sprite += 2;

	if ((loc10 & 0x1) != 0 && (flags & 0x2000) == 0) loc3E = sprite;

	if ((flags & 0x400) != 0) {
		sprite += 16;
	}

	if ((loc10 & 0x2) == 0) {
		Format80_Decode(emu_get_memorycsip(g_global->variable_6F18), sprite, locbx);

		sprite = emu_get_memorycsip(g_global->variable_6F18);
	}

	if ((flags & 0x2) == 0) {
		loc2A = posY - top;
	} else {
		loc2A = bottom - posY - loc12;
	}

	if (loc2A < 0) {
		loc12 += loc2A;
		if (loc12 <= 0) return;

		loc2A = -loc2A;

		while (loc2A > 0) {
			loc42 = 0;
			loc38 = sprite;
			count = loc1A;
			loc1C = loc1A;

			/* Call based on memory/register values */
			switch (s_variable_1E[flags & 0xFF]) {
				case 0x050E:
				case 0x0545:
					if (count == 0) break;

					while (count > 0) {
						while (count != 0) {
							count--;
							if (*sprite++ == 0) break;
						}
						if (sprite[-1] != 0 && count == 0) break;

						count -= *sprite++ - 1;
					}

					buf += count * ((s_variable_1E[flags & 0xFF] == 0x050E) ? -1 : 1);
					break;

				default:
					/* In case we don't know the call point yet, call the dynamic call */
					emu_last_cs = 0x2903; emu_last_ip = 0x0384; emu_last_length = 0x0016; emu_last_crc = 0x7D40;
					emu_call();
					return;
			}

			loc34 += loc32;
			if ((loc34 & 0xFF00) == 0) continue;

			loc2A -= loc34 >> 8;
			loc34 &= 0xFF;
		}

		if (loc2A < 0) {
			sprite = loc38;

			loc2A = -loc2A;
			loc34 += loc2A << 8;
		}

		if ((flags & 0x2) == 0) posY = top;
	}

	if ((flags & 0x2) == 0) {
		loc1E = bottom - posY;
	} else {
		loc1E = posY + loc12 - top;
	}

	if (loc1E <= 0) return;

	if (loc1E < loc12) {
		loc12 = loc1E;
		if ((flags & 0x2) != 0) posY = top;
	}

	loc1E = 0;
	if (posX < 0) {
		loc14 += posX;
		loc1E = -posX;
		if (loc1E >= loc16) return;
		posX = 0;
	}

	loc20 = 0;
	loc3A = width - posX;
	if (loc3A <= 0) return;

	if (loc3A < loc14) {
		loc14 = loc3A;
		loc20 = loc16 - loc1E - loc14;
	}

	loc3A = SCREEN_WIDTH;
	loc22 = posY;

	if ((flags & 0x2) != 0) {
		loc3A = - loc3A;
		loc22 += loc12 - 1;
	}

	loc22 *= 2;
	loc22 = emu_get_memory16(g_global->variable_66EC.s.cs, g_global->variable_66EC.s.ip, loc22) + posX;
	buf += loc22;

	if ((flags & 0x1) != 0) {
		uint16 tmp = loc1E;
		loc1E = loc20;
		loc20 = tmp;
		buf += loc14 - 1;
	}

	b = buf;

	if ((flags & 0x4) != 0) {
		loc20 = 0;
		loc44 = loc1E;
		loc1E = (loc44 << 8) / loc30;
		loc42 = -((loc44 << 8) % loc30);
	}

	if ((loc34 & 0xFF00) == 0) {
	l__04A4:
		while (true) {
			loc34 += loc32;

			if ((loc34 & 0xFF00) != 0) break;
			count = loc1A;
			loc1C = loc1A;

			/* Call based on memory/register values */
			switch (s_variable_1E[flags & 0xFF]) {
				case 0x050E:
				case 0x0545:
					if (count == 0) break;

					while (count > 0) {
						while (count != 0) {
							count--;
							if (*sprite++ == 0) break;
						}
						if (sprite[-1] != 0 && count == 0) break;

						count -= *sprite++ - 1;
					}

					buf += count * ((s_variable_1E[flags & 0xFF] == 0x050E) ? -1 : 1);
					break;

				default:
					/* In case we don't know the call point yet, call the dynamic call */
					emu_last_cs = 0x2903; emu_last_ip = 0x04B9; emu_last_length = 0x0018; emu_last_crc = 0x7352;
					emu_call();
					return;
			}
		}
		loc38 = sprite;
	}

	while (true) {
		loc1C = loc1A;
		count = loc1E;

		/* Call based on memory/register values */
		switch (s_variable_0E[flags & 0xFF]) {
			case 0x050E:
			case 0x0545:
				if (count == 0) break;

				while (count > 0) {
					while (count != 0) {
						count--;
						if (*sprite++ == 0) break;
					}
					if (sprite[-1] != 0 && count == 0) break;

					count -= *sprite++ - 1;
				}

				buf += count * ((s_variable_0E[flags & 0xFF] == 0x050E) ? -1 : 1);
				break;

			default:
				/* In case we don't know the call point yet, call the dynamic call */
				emu_last_cs = 0x2903; emu_last_ip = 0x04D1; emu_last_length = 0x0010; emu_last_crc = 0x92CB;
				emu_call();
				return;
		}

		if (loc1C != 0) {
			count += loc14;
			if (count > 0) {
				uint8 v;

				/* Call based on memory/register values */
				switch (s_variable_2E[flags & 0xFF]) {
					case 0x0530:
					case 0x0569:
						while (count > 0) {
							v = *sprite++;
							if (v == 0) {
								buf += *sprite * ((s_variable_2E[flags & 0xFF] == 0x0530) ? 1 : -1);
								count -= *sprite++;
								continue;
							}

							/* Call based on memory/register values */
							switch (s_variable_3E[(flags >> 8) & 0xF]) {
								case 0x0580:
									*buf = v;
									break;

								case 0x0584: {
									int16 i;

									for(i = 0; i < loc28; i++) v = loc26[v];

									*buf = v;

									break;
								}

								case 0x0599:
									s_variable_74 += s_variable_72;

									if ((s_variable_74 & 0xFF00) == 0) {
										*buf = v;
									} else {
										s_variable_74 &= 0xFF;
										*buf = buf[s_variable_70];
									}
									break;

								case 0x05C8: {
									int16 i;

									v = *buf;

									for(i = 0; i < loc28; i++) v = loc26[v];

									*buf = v;

									break;
								}

								case 0x05E0:
									*buf = loc3E[v];
									break;

								case 0x05EA:
									s_variable_74 += s_variable_72;

									if ((s_variable_74 & 0xFF00) == 0) {
										*buf = loc3E[v];
									} else {
										s_variable_74 &= 0xFF;
										*buf = buf[s_variable_70];
									}
									break;

								case 0x061F: {
									int16 i;

									v = loc3E[v];

									for(i = 0; i < loc28; i++) v = loc26[v];

									*buf = v;

									break;
								}

								default:
									/* In case we don't know the call point yet, call the dynamic call */
									emu_last_cs = 0x2903; emu_last_ip = 0x0535; emu_last_length = 0x0008; emu_last_crc = 0xCF42;
									emu_call();
									return;
							}

							buf += ((s_variable_2E[flags & 0xFF] == 0x0530) ? 1 : -1);
							count--;
						}
						break;

					default:
						/* In case we don't know the call point yet, call the dynamic call */
						emu_last_cs = 0x2903; emu_last_ip = 0x04DF; emu_last_length = 0x000E; emu_last_crc = 0x82DD;
						emu_call();
						return;
				}
			}

			count += loc20;
			if (count != 0) {
				/* Call based on memory/register values */
				switch (s_variable_1E[flags & 0xFF]) {
					case 0x050E:
					case 0x0545:
						if (count == 0) break;

						while (count > 0) {
							while (count != 0) {
								count--;
								if (*sprite++ == 0) break;
							}
							if (sprite[-1] != 0 && count == 0) break;

							count -= *sprite++ - 1;
						}

						buf += count * ((s_variable_1E[flags & 0xFF] == 0x050E) ? -1 : 1);
						break;

					default:
						/* In case we don't know the call point yet, call the dynamic call */
						emu_last_cs = 0x2903; emu_last_ip = 0x04E7; emu_last_length = 0x0008; emu_last_crc = 0x5672;
						emu_call();
						return;
				}
			}
		}

		b += loc3A;
		buf = b;

		if (--loc12 == 0) return;

		loc34 -= 0x100;
		if ((loc34 & 0xFF00) == 0) goto l__04A4;
		sprite = loc38;
	}
}

/**
 * Draw the sprite of variable_8002 at (variable_8010, variable_8011)
 * @param screenID The screen to draw at.
 */
void GUI_DrawSprite_8002(uint16 screenID)
{
	GUI_DrawSprite(screenID, g_global->variable_8002, g_global->variable_8010, g_global->variable_8011, 0, 0);
}

/**
 * Updates the score.
 * @param score The base score.
 * @param harvestedAllied Pointer to the total amount of spice harvested by allies.
 * @param harvestedEnemy Pointer to the total amount of spice harvested by enemies.
 * @param houseID The houseID of the player.
 */
static uint16 Update_Score(int16 score, uint16 *harvestedAllied, uint16 *harvestedEnemy, uint16 houseID)
{
	PoolFindStruct find;
	uint16 locdi = 0;
	int16  loc0A;
	uint16 loc0C = 0;
	uint32 tmp;

	if (score < 0) score = 0;

	find.houseID = houseID;
	find.type    = 0xFFFF;
	find.index   = 0xFFFF;

	while (true) {
		Structure *s;

		s = Structure_Find(&find);
		if (s == NULL) break;

		score += g_structureInfo[s->o.type].o.buildCredits / 100;
	}

	g_global->variable_38BC++;

	find.houseID = 0xFFFF;
	find.type    = UNIT_HARVESTER;
	find.index   = 0xFFFF;

	while (true) {
		Unit *u;

		u = Unit_Find(&find);
		if (u == NULL) break;

		if (House_AreAllied(Unit_GetHouseID(u), (uint8)g_global->playerHouseID)) {
			locdi += u->amount * 7;
		} else {
			loc0C += u->amount * 7;
		}
	}

	g_global->variable_38BC--;

	tmp = *harvestedEnemy + loc0C;
	*harvestedEnemy = (tmp > 65000) ? 65000 : (tmp & 0xFFFF);

	tmp = *harvestedAllied + locdi;
	*harvestedAllied = (tmp > 65000) ? 65000 : (tmp & 0xFFFF);

	score += House_Get_ByIndex((uint8)houseID)->credits / 100;

	if (score < 0) score = 0;

	loc0A = g_global->campaignID * 45;

	if ((int16)g_global->variable_81EB < loc0A) {
		score += loc0A - g_global->variable_81EB;
	}

	return score;
}

/**
 * Draws a string on a filled rectangle.
 * @param string The string to draw.
 * @param top The most top position where to draw the string.
 */
static void GUI_DrawTextOnFilledRectangle(char *string, uint16 top)
{
	uint16 halfWidth;

	GUI_DrawText_Wrapper(NULL, 0, 0, 0, 0, 0x121);

	halfWidth = (Font_GetStringWidth(string) / 2) + 4;

	GUI_DrawFilledRectangle(SCREEN_WIDTH / 2 - halfWidth, top, SCREEN_WIDTH / 2 + halfWidth, top + 6, 116);
	GUI_DrawText_Wrapper(string, SCREEN_WIDTH / 2, top, 0xF, 0, 0x121);
}

static uint16 GUI_HallOfFame_GetRank(uint16 score)
{
	uint8 i;

	for (i = 0; i < 12; i++) {
		if (g_global->variable_37C0[i][1] > score) break;
	}

	return min(i, 11);
}

static void GUI_HallOfFame_DrawRank(uint16 score, bool fadeIn)
{
	GUI_DrawText_Wrapper(String_Get_ByIndex(g_global->variable_37C0[GUI_HallOfFame_GetRank(score)][0]), SCREEN_WIDTH / 2, 49, 6, 0, 0x122);

	if (!fadeIn) return;

	GUI_Screen_FadeIn(10, 49, 10, 49, 20, 12, 2, 0);
}

static void GUI_HallOfFame_DrawBackground(uint16 score, bool hallOfFame)
{
	uint16 oldScreenID;
	uint16 xSrc;
	uint16 colour;
	uint16 offset;

	oldScreenID = GUI_Screen_SetActive(2);;

	Sprites_LoadImage("FAME.CPS", 3, 3, emu_get_memorycsip(g_global->variable_998A), 1);

	xSrc = 1;
	if ((int16)g_global->playerHouseID >= HOUSE_HARKONNEN && (int16)g_global->playerHouseID <= HOUSE_ORDOS) {
		xSrc = (g_global->playerHouseID * 56 + 8) / 8;
	}

	GUI_Screen_Copy(xSrc, 136, 0, 8, 7, 56, 2, 2);

	if ((int16)g_global->playerHouseID < HOUSE_HARKONNEN || (int16)g_global->playerHouseID > HOUSE_ORDOS) {
		xSrc += 7;
	}

	GUI_Screen_Copy(xSrc, 136, 33, 8, 7, 56, 2, 2);

	GUI_DrawFilledRectangle(8, 136, 175, 191, 116);

	if (hallOfFame) {
		GUI_DrawFilledRectangle(8, 80, 311, 191, 116);
		if (score != 0xFFFF) GUI_HallOfFame_DrawRank(score, false);
	} else {
		GFX_Screen_Copy2(8, 80, 8, 116, 304, 36, 2, 2, false);
		if (g_global->scenarioID != 1) GFX_Screen_Copy2(8, 80, 8, 152, 304, 36, 2, 2, false);
	}

	if (score != 0xFFFF) {
		/* "Time: %dh %dm" */
		sprintf((char *)g_global->variable_9939, String_Get_ByIndex(0x16), g_global->variable_81EB / 60, g_global->variable_81EB % 60);

		if (g_global->variable_81EB < 60) {
			char *hours = strchr((char *)g_global->variable_9939, '0');
			while (*hours != ' ') strcpy(hours, hours + 1);
		}

		/* "Score: %d" */
		GUI_DrawText_Wrapper(String_Get_ByIndex(0x15), 72, 15, 15, 0, 0x22, score);
		GUI_DrawText_Wrapper((char *)g_global->variable_9939, 248, 15, 15, 0, 0x222);
		/* "You have attained the rank of" */
		GUI_DrawText_Wrapper(String_Get_ByIndex(0x17), SCREEN_WIDTH / 2, 38, 15, 0, 0x122);
	} else {
		/* "Hall of Fame" */
		GUI_DrawText_Wrapper(String_Get_ByIndex(0x150), SCREEN_WIDTH / 2, 15, 15, 0, 0x122);
	}

	switch (g_global->playerHouseID) {
		case HOUSE_HARKONNEN:
			colour = 149;
			offset = 0;
			break;

		default:
			colour = 165;
			offset = 2;
			break;

		case HOUSE_ORDOS:
			colour = 181;
			offset = 1;
			break;
	}

	g_global->variable_81ED = g_global->variable_3C32;
	g_global->variable_81ED.s.ip += 255 * 3;

	memcpy(emu_get_memorycsip(g_global->variable_81ED), emu_get_memorycsip(g_global->variable_3C32) + colour * 3, 3);

	g_global->variable_81ED.s.ip += offset;

	if (!hallOfFame) GUI_HallOfFame_Tick();

	GUI_Screen_SetActive(oldScreenID);
}

static void GUI_EndStats_Sleep(uint16 delay)
{
	g_global->variable_76B4 = delay;

	while (g_global->variable_76B4 != 0) GUI_HallOfFame_Tick();
}

/**
 * Shows the stats at end of scenario.
 * @param killedAllied The amount of destroyed allied units.
 * @param killedEnemy The amount of destroyed enemy units.
 * @param destroyedAllied The amount of destroyed allied structures.
 * @param destroyedEnemy The amount of destroyed enemy structures.
 * @param harvestedAllied The amount of spice harvested by allies.
 * @param harvestedEnemy The amount of spice harvested by enemies.
 * @param score The base score.
 * @param houseID The houseID of the player.
 */
void GUI_EndStats_Show(uint16 killedAllied, uint16 killedEnemy, uint16 destroyedAllied, uint16 destroyedEnemy, uint16 harvestedAllied, uint16 harvestedEnemy, int16 score, uint16 houseID)
{
	uint16 loc06;
	uint16 oldScreenID;
	uint16 loc16;
	uint16 loc18;
	uint16 loc1A;
	uint16 loc32[3][2][2];
	uint16 i;

	g_global->variable_81EB = ((g_global->tickGlobal - g_global->tickScenarioStart) / 3600) + 1;

	score = Update_Score(score, &harvestedAllied, &harvestedEnemy, houseID);

	loc16 = (g_global->scenarioID == 1) ? 2 : 3;

	GUI_Mouse_Hide_Safe();

	GUI_ChangeSelectionType(0);

	oldScreenID = GUI_Screen_SetActive(2);

	GUI_HallOfFame_DrawBackground(score, false);

	/* "Spice Harvested By" */
	GUI_DrawTextOnFilledRectangle(String_Get_ByIndex(0x1A), 83);

	/* "Units Destroyed By" */
	GUI_DrawTextOnFilledRectangle(String_Get_ByIndex(0x18), 119);

	if (g_global->scenarioID != 1) {
		/* "Buildings Destroyed By" */
		GUI_DrawTextOnFilledRectangle(String_Get_ByIndex(0x19), 155);
	}

	/* "You:", "Enemy:" */
	loc06 = max(Font_GetStringWidth(String_Get_ByIndex(0x149)), Font_GetStringWidth(String_Get_ByIndex(0x14A)));

	loc18 = loc06 + 19;
	loc1A = 261 - loc18;

	for (i = 0; i < loc16; i++) {
		/* "You:" */
		GUI_DrawText_Wrapper(String_Get_ByIndex(0x149), loc18 - 4,  92 + (i * 36), 0xF, 0, 0x221);

		/* "Enemy:" */
		GUI_DrawText_Wrapper(String_Get_ByIndex(0x14A), loc18 - 4, 101 + (i * 36), 0xF, 0, 0x221);
	}

	Music_Play(17 + Tools_RandomRange(0, 5));

	GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 2, 0);

	emu_push(emu_cs); emu_push(0x01E2); emu_cs = 0x29E8; emu_Input_History_Clear();

	loc32[0][0][0] = harvestedAllied;
	loc32[0][1][0] = harvestedEnemy;
	loc32[1][0][0] = killedEnemy;
	loc32[1][1][0] = killedAllied;
	loc32[2][0][0] = destroyedEnemy;
	loc32[2][1][0] = destroyedAllied;

	for (i = 0; i < loc16; i++) {
		uint16 loc08 = loc32[i][0][0];
		uint16 loc0A = loc32[i][1][0];

		if (loc08 >= 65000) loc08 = 65000;
		if (loc0A >= 65000) loc0A = 65000;

		loc32[i][0][0] = loc08;
		loc32[i][1][0] = loc0A;

		loc06 = max(loc08, loc0A);

		loc32[i][0][1] = 1 + ((loc06 > loc1A) ? (loc06 / loc1A) : 0);
		loc32[i][1][1] = 1 + ((loc06 > loc1A) ? (loc06 / loc1A) : 0);
	}

	GUI_EndStats_Sleep(45);
	GUI_HallOfFame_DrawRank(score, true);
	GUI_EndStats_Sleep(45);

	for (i = 0; i < loc16; i++) {
		uint16 loc02;

		GUI_HallOfFame_Tick();

		for (loc02 = 0; loc02 < 2; loc02++) {
			uint8 colour;
			uint16 loc04;
			uint16 locdi;
			uint16 loc0E;
			uint16 loc10;
			uint16 loc0C;

			GUI_HallOfFame_Tick();

			colour = (loc02 == 0) ? 255 : 209;
			loc04 = loc18;

			locdi = 93 + (i * 36) + (loc02 * 9);

			loc0E = loc32[i][loc02][0];
			loc10 = loc32[i][loc02][1];

			for (loc0C = 0; loc0C < loc0E; loc0C += loc10) {
				GUI_DrawFilledRectangle(271, locdi, 303, locdi + 5, 226);

				GUI_DrawText_Wrapper("%u", 287, locdi - 1, 0x14, 0, 0x121, loc0C);

				GUI_HallOfFame_Tick();

				g_global->variable_76B4 = 1;

				GUI_DrawLine(loc04, locdi, loc04, locdi + 5, colour);

				loc04++;

				GUI_DrawLine(loc04, locdi + 1, loc04, locdi + 6, 12);

				GFX_Screen_Copy2(loc18, locdi, loc18, locdi, 304, 7, 2, 0, false);

				Driver_Sound_Play(52, 0xFF);

				GUI_EndStats_Sleep(g_global->variable_76B4);
			}

			GUI_DrawFilledRectangle(271, locdi, 303, locdi + 5, 226);

			GUI_DrawText_Wrapper("%u", 287, locdi - 1, 0xF, 0, 0x121, loc0E);

			GFX_Screen_Copy2(loc18, locdi, loc18, locdi, 304, 7, 2, 0, false);

			Driver_Sound_Play(38, 0xFF);

			GUI_EndStats_Sleep(12);
		}

		GUI_EndStats_Sleep(60);
	}

	GUI_Mouse_Show_Safe();

	emu_push(emu_cs); emu_push(0x050C); emu_cs = 0x29E8; emu_Input_History_Clear();

	while (true) {
		GUI_HallOfFame_Tick();

		emu_push(emu_cs); emu_push(0x0516); emu_cs = 0x29E8; emu_Input_Keyboard_NextKey();
		if (emu_ax != 0) break;
	}

	emu_push(emu_cs); emu_push(0x051F); emu_cs = 0x29E8; emu_Input_History_Clear();

	GUI_HallOfFame_Show(score);

	memcpy(emu_get_memorycsip(g_global->variable_3C32) + 0x2FD, g_global->variable_81E8, 3);

	GUI_Screen_SetActive(oldScreenID);

	Driver_Music_FadeOut();
}

/**
 * Show pick house screen.
 */
uint16 GUI_PickHouse()
{
	uint16 oldScreenID;
	Widget *w = NULL;
	csip32 wcsip;
	uint8 *loc314; /* array of 768 bytes, probably a palette */
	csip32 csip314;
	uint16 i;
	uint16 ret;

	emu_push(emu_bp);
	emu_bp = emu_sp;
	emu_subw(&emu_sp, 0x314);

	ret = 0x5;
	wcsip.csip = 0x0;
	csip314.s.cs = emu_ss;
	csip314.s.ip = emu_bp - 0x314;
	loc314 = emu_get_memorycsip(csip314);

	memset(loc314, 0, 768);

	{
		csip32 nullcsip;
		nullcsip.csip = 0x0;
		Driver_Voice_Play(NULL, nullcsip, 0xFF, 0xFF);
	}

	Voice_LoadVoices(5);

	Sprites_Load(1, 7, g_sprites);

	while (true) {
		uint16 yes_no;

		for (i = 0; i < 3; i++) {
			Widget *w2;
			csip32 w2csip;

			w2 = GUI_Widget_Allocate(i + 1, g_global->variable_2BAC[i][2], g_global->variable_2BAC[i][0], g_global->variable_2BAC[i][1], 0xFFFF, 0, 0, &w2csip);

			w2->flags.all = 0x0;
			w2->flags.s.loseSelect = true;
			w2->flags.s.variable_0080 = true;
			w2->flags.s.buttonFilterLeft = 1;
			w2->flags.s.buttonFilterRight = 1;
			w2->width  = 96;
			w2->height = 104;

			w = GUI_Widget_Link(w, w2);
			wcsip = emu_Global_GetCSIP(w);
		}

		Sprites_LoadImage(String_GenerateFilename("HERALD"), 3, 3, NULL, 1);

		GFX_Screen_Copy3(3, 2);

		GUI_Mouse_Hide_Safe();
		GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 2, 0);
		Unknown_259E_0006(g_global->variable_3C32, 15);
		GUI_Mouse_Show_Safe();

		ret = 0xFFFE;

		while (ret == 0xFFFE) {
			uint16 key = GUI_Widget_HandleEvents(w);

			GUI_PaletteAnimate();

			if ((key & 0x800) != 0) key = 0;

			switch (key) {
				case 0x8001: ret = 1; break;
				case 0x8002: ret = 2; break;
				case 0x8003: ret = 0; break;
				default: break;
			}
		}

		GUI_Mouse_Hide_Safe();

		if (ret == 0xFFFF) {
			Unknown_259E_0006(csip314, 15);
			break;
		}

		if (g_global->variable_6D8F != 0) {
			Unknown_B483_0363(ret + 62);

			while (Unknown_B483_0470()) sleep(0);
		}

		while (w != NULL) {
			csip32 next = w->next;

			emu_push(wcsip.s.cs); emu_push(wcsip.s.ip);
			emu_push(emu_cs); emu_push(0x11D6); emu_cs = 0x23E1; emu_Tools_Free();
			emu_sp += 4;

			wcsip = next;
			w = (Widget *)emu_get_memorycsip(wcsip);
		}

		Unknown_259E_0006(csip314, 15);

		if (g_global->debugSkipDialogs != 0 || g_global->debugScenario != 0) break;

		w = GUI_Widget_Link(w, GUI_Widget_Allocate(1, GUI_Widget_GetShortcut(String_Get_ByIndex(107)[0]), 168, 168, 0, 0, 0, NULL)); /* "Yes" */
		w = GUI_Widget_Link(w, GUI_Widget_Allocate(2, GUI_Widget_GetShortcut(String_Get_ByIndex(108)[0]), 240, 168, 2, 0, 0, NULL)); /* "No" */
		wcsip = emu_Global_GetCSIP(w);

		sprintf((char *)g_global->variable_9939, "TEXT%c", *emu_get_memorycsip(g_houseInfo[ret].name));

		String_LoadFile(String_GenerateFilename((char *)g_global->variable_9939), 0, (char *)emu_get_memorycsip(g_global->readBuffer), g_global->readBufferSize);
		String_TranslateSpecial((char *)emu_get_memorycsip(g_global->readBuffer), (char *)emu_get_memorycsip(g_global->readBuffer));

		g_global->playerHouseID = HOUSE_MERCENARY;

		oldScreenID = GUI_Screen_SetActive(0);

		GUI_Mouse_Show_Safe();

		GUI_Mentat_Show(g_global->readBuffer, g_global->variable_2BBE[ret], NULL, false);

		Sprites_LoadImage(String_GenerateFilename("MISC"), 3, 3, emu_get_memorycsip(g_global->variable_3C32), 1);

		GUI_Mouse_Hide_Safe();

		GUI_Screen_Copy(0, 0, 0, 0, 26, 24, 2, 0);

		GUI_Screen_Copy(0, 24 * (ret + 1), 26, 0, 13, 24, 2, 0);

		GUI_Widget_DrawAll(w);

		GUI_Mouse_Show_Safe();

		while (true) {
			yes_no = GUI_Mentat_Loop((char *)emu_get_memorycsip(g_global->variable_2BBE[ret]), NULL, NULL, true, w);

			if ((yes_no & 0x8000) != 0) break;
		}

		if (yes_no == 0x8001) {
			Driver_Music_FadeOut();
		} else {
			Unknown_259E_0006(csip314, 15);
		}

		while (w != NULL) {
			csip32 next = w->next;

			emu_push(wcsip.s.cs); emu_push(wcsip.s.ip);
			emu_push(emu_cs); emu_push(0x146C); emu_cs = 0x23E1; emu_Tools_Free();
			emu_sp += 4;

			wcsip = next;
			w = (Widget *)emu_get_memorycsip(wcsip);
		}

		Load_Palette_Mercenaries();
		Sprites_LoadTiles();

		GUI_Screen_SetActive(oldScreenID);

		while (Driver_Voice_IsPlaying());

		if (yes_no == 0x8001) break;
	}

	Music_Play(0);

	Unknown_B4B8_110D((uint8)ret);

	Sprites_Load(0, 7, g_sprites);

	emu_push(emu_cs); emu_push(0x14D8); emu_cs = 0x29E8; emu_Input_History_Clear();

	GUI_Mouse_Show_Safe();

	Unknown_259E_0006(csip314, 15);

	emu_sp = emu_bp;
	emu_pop(&emu_bp);

	return ret;
}

/**
 * Creates a palette mapping: colour -> colour + reference * intensity.
 *
 * @param palette The palette to create the mapping for.
 * @param colours The resulting mapping.
 * @param reference The colour to use as reference.
 * @param intensity The intensity to use.
 */
void GUI_Palette_CreateMapping(uint8 *palette, uint8 *colours, uint8 reference, uint8 intensity)
{
	uint16 index;

	if (palette == NULL || colours == NULL) return;

	colours[0] = 0;

	for (index = 1; index < 256; index++) {
		uint16 i;
		uint8 red   = palette[3 * index + 0] - (((palette[3 * index + 0] - palette[3 * reference + 0]) * (intensity / 2)) >> 7);
		uint8 blue  = palette[3 * index + 1] - (((palette[3 * index + 1] - palette[3 * reference + 1]) * (intensity / 2)) >> 7);
		uint8 green = palette[3 * index + 2] - (((palette[3 * index + 2] - palette[3 * reference + 2]) * (intensity / 2)) >> 7);
		uint8 colour = reference;
		uint16 sumMin = 0xFFFF;

		for (i = 1; i < 256; i++) {
			uint16 sum = 0;

			sum += (palette[3 * i + 0] - red)   * (palette[3 * i + 0] - red);
			sum += (palette[3 * i + 1] - blue)  * (palette[3 * i + 1] - blue);
			sum += (palette[3 * i + 2] - green) * (palette[3 * i + 2] - green);

			if (sum > sumMin) continue;
			if ((i != reference) && (i == index)) continue;

			sumMin = sum;
			colour = i & 0xFF;
		}

		colours[index] = colour;
	}
}

/**
 * Draw a border.
 *
 * @param left Left position of the border.
 * @param top Top position of the border.
 * @param width Width of the border.
 * @param height Height of the border.
 * @param colourSchemaIndex Index of the colourSchema used.
 * @param fill True if you want the border to be filled.
 */
void GUI_DrawBorder(uint16 left, uint16 top, uint16 width, uint16 height, uint16 colourSchemaIndex, bool fill)
{
	uint16 *colourSchema;

	width  -= 1;
	height -= 1;

	colourSchema = g_global->colourBorderSchema[colourSchemaIndex];

	if (fill) GUI_DrawFilledRectangle(left, top, left + width, top + height, colourSchema[0] & 0xFF);

	GUI_DrawLine(left, top + height, left + width, top + height, colourSchema[1] & 0xFF);
	GUI_DrawLine(left + width, top, left + width, top + height, colourSchema[1] & 0xFF);
	GUI_DrawLine(left, top, left + width, top, colourSchema[2] & 0xFF);
	GUI_DrawLine(left, top, left, top + height, colourSchema[2] & 0xFF);

	GFX_PutPixel(left, top + height, colourSchema[3] & 0xFF);
	GFX_PutPixel(left + width, top, colourSchema[3] & 0xFF);
}

/**
 * Display a hint to the user. Only show each hint exactly once.
 *
 * @param stringID The string of the hint to show.
 * @param spriteID The sprite to show with the hint.
 * @return Zero or the return value of GUI_DisplayModalMessage.
 */
uint16 GUI_DisplayHint(uint16 stringID, uint16 spriteID)
{
	uint32 *hintsShown;
	uint32 mask;

	assert(stringID < 64);

	if (g_global->debugGame) return 0;
	if (stringID == 0) return 0;
	if (!g_global->gameConfig.hints) return 0;
	if (g_global->selectionType == 0) return 0;

	if (stringID < 32) {
		mask = (1 << stringID);
		hintsShown = &g_global->hintsShown1;
	} else {
		mask = (1 << (stringID - 32));
		hintsShown = &g_global->hintsShown2;
	}

	if ((*hintsShown & mask) != 0) return 0;
	*hintsShown |= mask;

	return GUI_DisplayModalMessage(String_GetFromBuffer_ByIndex((char *)emu_get_memorycsip(g_global->variable_38C6), stringID), spriteID);
}

void GUI_DrawProgressbar(uint16 current, uint16 max)
{
	uint16 *info;
	uint16 width;
	uint16 height;
	uint16 colour;

	info = g_global->progressbarInfo;

	info[7] = max;
	info[6] = current;

	if (current > max) current = max;
	if (max < 1) max = 1;

	width  = info[2];
	height = info[3];

	/* 0 = Horizontal, 1 = Vertial */
	if (info[5] == 0) {
		width = current * width / max;
		if (width < 1) width = 1;
	} else {
		height = current * height / max;
		if (height < 1) height = 1;
	}

	colour = info[8];
	if (current <= max / 2) colour = info[9];
	if (current <= max / 4) colour = info[10];

	if (current != 0 && width  == 0) width = 1;
	if (current != 0 && height == 0) height = 1;

	if (height != 0) {
		GUI_DrawBorder(info[0] - 1, info[1] - 1, info[2] + 2, info[3] + 2, 1, true);
	}

	if (width != 0) {
		GUI_DrawFilledRectangle(info[0], info[1] + info[3] - height, info[0] + width - 1, info[1] + info[3] - 1, (uint8)colour);
	}
}

/**
 * Draw the interface (borders etc etc) and radar on the screen.
 * @param screenID The screen to draw the radar on.
 */
void GUI_DrawInterfaceAndRadar(uint16 screenID)
{
	PoolFindStruct find;
	uint16 oldScreenID;
	Widget *w;

	oldScreenID = GUI_Screen_SetActive((screenID == 0) ? 2 : screenID);

	g_global->variable_3A12 = 1;

	Sprites_LoadImage(g_global->string_3777, 3, 3, NULL, 1);

	GUI_Palette_RemapScreen(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 2, g_global->variable_3C42);

	GUI_DrawSprite(2, g_sprites[11], 192, 0, 0, 0);

	g_global->variable_38C4 = 1;

	Unknown_07D4_159A(g_global->screenActiveID);

	Unknown_07D4_0000(g_global->screenActiveID);

	GUI_Widget_ActionPanel_Draw(true);

	w = GUI_Widget_Get_ByIndex((Widget *)emu_get_memorycsip(g_global->variable_3C26), 1);
	GUI_Widget_Draw(w);

	w = GUI_Widget_Get_ByIndex((Widget *)emu_get_memorycsip(g_global->variable_3C26), 2);
	GUI_Widget_Draw(w);

	find.houseID = 0xFFFF;
	find.index   = 0xFFFF;
	find.type    = 0xFFFF;

	while (true) {
		Structure *s;

		s = Structure_Find(&find);
		if (s == NULL) break;

		Structure_UpdateMap(s);
	}

	find.houseID = 0xFFFF;
	find.index   = 0xFFFF;
	find.type    = 0xFFFF;

	while (true) {
		Unit *u;

		u = Unit_Find(&find);
		if (u == NULL) break;

		Unit_B4CD_01BF(1, u);
	}

	if (screenID == 0) {
		GUI_Screen_SetActive(0);

		GUI_Mouse_Hide_Safe();

		GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 2 ,0);
		GUI_DrawCredits((uint8)g_global->playerHouseID, (g_global->playerCredits == 0xFFFF) ? 2 : 1);
		Unknown_259E_0006(g_global->variable_3C32, 15);

		GUI_Mouse_Show_Safe();
	}

	GUI_Screen_SetActive(oldScreenID);

	GUI_DrawCredits((uint8)g_global->playerHouseID, 2);

	emu_push(emu_cs); emu_push(0x228A); emu_cs = 0x29E8; emu_Input_History_Clear();
}

/**
 * Draw the credits on the screen, and animate it when the value is changing.
 * @param houseID The house to display the credits from.
 * @param mode The mode of displaying. 0 = animate, 1 = force draw, 2 = reset.
 */
void GUI_DrawCredits(uint8 houseID, uint16 mode)
{
	uint16 oldScreenID;
	uint16 oldValue_07AE_0000;
	House *h;
	char charCreditsOld[7];
	char charCreditsNew[7];
	int i;
	int16 creditsDiff;
	int16 creditsNew;
	int16 creditsOld;
	int16 offset;

	if (g_global->tickCreditsAnimation > g_global->variable_76AC && mode == 0) return;
	g_global->tickCreditsAnimation = g_global->variable_76AC + 1;

	h = House_Get_ByIndex(houseID);

	if (mode == 2) {
		g_global->playerCredits = h->credits;
		g_global->creditsAnimation = h->credits;
	}

	if (mode == 0 && h->credits == g_global->creditsAnimation && g_global->creditsAnimationOffset == 0) return;

	oldScreenID = GUI_Screen_SetActive(2);

	oldValue_07AE_0000 = Unknown_07AE_0000(4);

	creditsDiff = h->credits - g_global->creditsAnimation;
	if (creditsDiff != 0) {
		int16 diff = creditsDiff / 4;
		if (diff == 0)   diff = (creditsDiff < 0) ? -1 : 1;
		if (diff > 128)  diff = 128;
		if (diff < -128) diff = -128;
		g_global->creditsAnimationOffset += diff;
	} else {
		g_global->creditsAnimationOffset = 0;
	}

	if (creditsDiff != 0 && (g_global->creditsAnimationOffset < -7 || g_global->creditsAnimationOffset > 7)) {
		Driver_Sound_Play(creditsDiff > 0 ? 52 : 53, 0xFF);
	}

	if (g_global->creditsAnimationOffset < 0 && g_global->creditsAnimation == 0) g_global->creditsAnimationOffset = 0;

	g_global->creditsAnimation += g_global->creditsAnimationOffset / 8;

	if (g_global->creditsAnimationOffset > 0) g_global->creditsAnimationOffset &= 7;
	if (g_global->creditsAnimationOffset < 0) g_global->creditsAnimationOffset = -((-g_global->creditsAnimationOffset) & 7);

	creditsOld = g_global->creditsAnimation;
	creditsNew = g_global->creditsAnimation;
	offset = 1;

	if (g_global->creditsAnimationOffset < 0) {
		creditsOld -= 1;
		if (creditsOld < 0) creditsOld = 0;

		offset -= 8;
	}

	if (g_global->creditsAnimationOffset > 0) {
		creditsNew += 1;
	}

	GUI_DrawSprite(g_global->screenActiveID, g_sprites[12], 0, 0, 4, 0x4000);

	g_global->playerCredits = creditsOld;

	snprintf(charCreditsOld, sizeof(charCreditsOld), "%6d", creditsOld);
	snprintf(charCreditsNew, sizeof(charCreditsNew), "%6d", creditsNew);

	for (i = 0; i < 6; i++) {
		uint16 left = i * 10 + 4;
		uint16 spriteID;

		spriteID = (charCreditsOld[i] == ' ') ? 13 : charCreditsOld[i] - 34;

		if (charCreditsOld[i] != charCreditsNew[i]) {
			GUI_DrawSprite(g_global->screenActiveID, g_sprites[spriteID], left, offset - g_global->creditsAnimationOffset, 4, 0x4000);
			if (g_global->creditsAnimationOffset == 0) continue;

			spriteID = (charCreditsNew[i] == ' ') ? 13 : charCreditsNew[i] - 34;

			GUI_DrawSprite(g_global->screenActiveID, g_sprites[spriteID], left, offset + 8 - g_global->creditsAnimationOffset, 4, 0x4000);
		} else {
			GUI_DrawSprite(g_global->screenActiveID, g_sprites[spriteID], left, 1, 4, 0x4000);
		}
	}

	if (oldScreenID != g_global->screenActiveID) {
		GUI_Mouse_Hide_InWidget(5);
		GUI_Screen_Copy(g_global->variable_992D, g_global->variable_992B, g_global->variable_992D, g_global->variable_992B - 40, g_global->variable_992F, g_global->variable_9931, g_global->screenActiveID, oldScreenID);
		GUI_Mouse_Show_InWidget();
	}

	GUI_Screen_SetActive(oldScreenID);

	Unknown_07AE_0000(oldValue_07AE_0000);
}

/**
 * Change the selection type.
 * @param selectionType The new selection type.
 */
void GUI_ChangeSelectionType(uint16 selectionType)
{
	struct_B4E9 *info = (struct_B4E9 *)&emu_get_memory8(0x2E8A, 0, 0xE);
	uint16 oldScreenID;

	if (selectionType == 3 && g_global->selectionUnit.csip == 0x0) {
		selectionType = 4;
	}

	if (selectionType == 4 && g_global->selectionUnit.csip != 0x0) {
		g_global->selectionUnit.csip = 0x0;
	}

	oldScreenID = GUI_Screen_SetActive(2);

	if (g_global->selectionType != selectionType || info[selectionType].variable_0A != 0) {
		uint16 oldSelectionType = g_global->selectionType;

		Game_Timer_SetState(2, false);

		g_global->selectionType = selectionType;
		g_global->variable_3A10 = selectionType;
		g_global->variable_37B8 = 1;

		switch (oldSelectionType) {
			case 1:
			case 2:
				Map_SetSelection(g_global->activeStructurePosition);
				/* Fall-through */
			case 4:
				g_global->cursorDefaultSpriteID = 0;
				GUI_DisplayText(NULL, 0xFFFF);
				break;

			case 3:
				if (g_global->selectionUnit.csip != 0x0 && selectionType != 1 && selectionType != 3) {
					Unit *u = Unit_Get_ByMemory(g_global->selectionUnit);

					Unit_B4CD_01BF(2, u);

					g_global->selectionUnit.csip = 0x0;
				}
				break;

			default:
				break;
		}

		if (info[oldSelectionType].variable_04 != 0 && info[selectionType].variable_06 != 0) {
			g_global->variable_3A12 = 1;
			g_global->variable_3A14 = 1;

			GUI_DrawInterfaceAndRadar(0);
		}

		Unknown_07AE_0000(info[selectionType].variable_08);

		if (g_global->variable_6D5D != 0) {
			GUI_Widget_DrawBorder(g_global->variable_6D5D, 0, false);
		}

		if (selectionType != 0) {
			Widget *w = (Widget *)emu_get_memorycsip(g_global->variable_3C26);

			uint8 *loc08 = emu_get_memorycsip(info[selectionType].variable_00);

			while (w != NULL) {
				w->state.s.selected = false;
				w->flags.s.invisible = true;

				if (memchr(loc08 + 1, w->index, *loc08) != NULL) w->flags.s.invisible = false;

				GUI_Widget_Draw(w);
				w = GUI_Widget_GetNext(w);
			}

			GUI_Widget_DrawAll((Widget *)emu_get_memorycsip(g_global->variable_3C26));
			g_global->variable_38C4 = 1;
		}

		switch (g_global->selectionType) {
			case 0:
				if (oldSelectionType != 7) {
					g_global->cursorSpriteID = 0;

					Sprites_SetMouseSprite(0, 0, g_sprites[0]);
				}

				Unknown_07AE_0000(info[selectionType].variable_08);
				break;

			case 1:
				g_global->activeStructurePosition = g_global->selectionPosition;
				GUI_Widget_ActionPanel_Draw(true);

				g_global->cursorDefaultSpriteID = 5;

				Game_Timer_SetState(2, (g_global->variable_37AA != 0) ? true : false);
				break;

			case 2:
				Unit_Select(NULL);
				GUI_Widget_ActionPanel_Draw(true);

				Map_SetSelectionSize(g_structureInfo[g_global->activeStructureType].layout);

				Game_Timer_SetState(2, (g_global->variable_37AA != 0) ? true : false);
				break;

			case 3:
				Game_Timer_SetState(2, (g_global->variable_37AA != 0) ? true : false);

				GUI_Widget_ActionPanel_Draw(true);
				break;

			case 4:
				GUI_Widget_ActionPanel_Draw(true);

				Game_Timer_SetState(2, (g_global->variable_37AA != 0) ? true : false);
				break;

			default: break;
		}
	}

	GUI_Screen_SetActive(oldScreenID);
}

/**
 * Sets the colours to be used when drawing chars.
 * @param colours The colours to use.
 * @param min The index of the first colour to set.
 * @param max The index of the last colour to set.
 */
void GUI_InitColors(uint8 *colours, uint8 first, uint8 last)
{
	uint8 i;

	first &= 0xF;
	last &= 0xF;

	if (last < first || colours == NULL) return;

	for (i = first; i < last + 1; i++) g_colours[i] = *colours++;
}

/**
 * Get how the given point must be clipped.
 * @param x The X-coordinate of the point.
 * @param y The Y-coordinate of the point.
 * @return A bitset.
 */
static uint16 GetNeededClipping(int16 x, int16 y)
{
	uint16 flags = 0;

	if (y < g_clipping.top)    flags |= 0x1;
	if (y > g_clipping.bottom) flags |= 0x2;
	if (x < g_clipping.left)   flags |= 0x4;
	if (x > g_clipping.right)  flags |= 0x8;

	return flags;
}

/**
 * Applies top clipping to a line.
 * @param x1 Pointer to the X-coordinate of the begin of the line.
 * @param y1 Pointer to the Y-coordinate of the begin of the line.
 * @param x2 The X-coordinate of the end of the line.
 * @param y2 The Y-coordinate of the end of the line.
 */
static void ClipTop(int16 *x1, int16 *y1, int16 x2, int16 y2)
{
	*x1 += (x2 - *x1) * (g_clipping.top - *y1) / (y2 - *y1);
	*y1 = g_clipping.top;
}

/**
 * Applies bottom clipping to a line.
 * @param x1 Pointer to the X-coordinate of the begin of the line.
 * @param y1 Pointer to the Y-coordinate of the begin of the line.
 * @param x2 The X-coordinate of the end of the line.
 * @param y2 The Y-coordinate of the end of the line.
 */
static void ClipBottom(int16 *x1, int16 *y1, int16 x2, int16 y2)
{
	*x1 += (x2 - *x1) * (*y1 - g_clipping.bottom) / (*y1 - y2);
	*y1 = g_clipping.bottom;
}

/**
 * Applies left clipping to a line.
 * @param x1 Pointer to the X-coordinate of the begin of the line.
 * @param y1 Pointer to the Y-coordinate of the begin of the line.
 * @param x2 The X-coordinate of the end of the line.
 * @param y2 The Y-coordinate of the end of the line.
 */
static void ClipLeft(int16 *x1, int16 *y1, int16 x2, int16 y2)
{
	*y1 += (y2 - *y1) * (g_clipping.left - *x1) / (x2 - *x1);
	*x1 = g_clipping.left;
}

/**
 * Applies right clipping to a line.
 * @param x1 Pointer to the X-coordinate of the begin of the line.
 * @param y1 Pointer to the Y-coordinate of the begin of the line.
 * @param x2 The X-coordinate of the end of the line.
 * @param y2 The Y-coordinate of the end of the line.
 */
static void ClipRight(int16 *x1, int16 *y1, int16 x2, int16 y2)
{
	*y1 += (y2 - *y1) * (*x1 - g_clipping.right) / (*x1 - x2);
	*x1 = g_clipping.right;
}

/**
 * Draws a line from (x1, y1) to (x2, y2) using given colour.
 * @param x1 The X-coordinate of the begin of the line.
 * @param y1 The Y-coordinate of the begin of the line.
 * @param x2 The X-coordinate of the end of the line.
 * @param y2 The Y-coordinate of the end of the line.
 * @param colour The colour to use to draw the line.
 */
void GUI_DrawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint8 colour)
{
	uint8 *screen = &emu_get_memory8(GFX_Screen_GetSegmentActive(), 0x00, 0x00);
	int16 increment = 1;

	if (x1 < g_clipping.left || x1 > g_clipping.right || y1 < g_clipping.top || y1 > g_clipping.bottom || x2 < g_clipping.left || x2 > g_clipping.right || y2 < g_clipping.top || y2 > g_clipping.bottom) {
		while (true) {
			uint16 clip1 = GetNeededClipping(x1, y1);
			uint16 clip2 = GetNeededClipping(x2, y2);

			if (clip1 == 0 && clip2 == 0) break;
			if ((clip1 & clip2) != 0) return;

			switch (clip1) {
				case 1: case 9:  ClipTop(&x1, &y1, x2, y2); break;
				case 2: case 6:  ClipBottom(&x1, &y1, x2, y2); break;
				case 4: case 5:  ClipLeft(&x1, &y1, x2, y2); break;
				case 8: case 10: ClipRight(&x1, &y1, x2, y2); break;
				default:
					switch (clip2) {
						case 1: case 9:  ClipTop(&x2, &y2, x1, y1); break;
						case 2: case 6:  ClipBottom(&x2, &y2, x1, y1); break;
						case 4: case 5:  ClipLeft(&x2, &y2, x1, y1); break;
						case 8: case 10: ClipRight(&x2, &y2, x1, y1); break;
						default: break;
					}
			}
		}
	}

	y2 -= y1;

	if (y2 == 0) {
		if (x1 >= x2) {
			int16 x = x1;
			x1 = x2;
			x2 = x;
		}

		x2 -= x1 - 1;

		screen += y1 * SCREEN_WIDTH + x1;

		memset(screen, colour, x2);
		return;
	}

	if (y2 < 0) {
		int16 x = x1;
		x1 = x2;
		x2 = x;
		y2 = -y2;
		y1 -= y2;
	}

	screen += y1 * SCREEN_WIDTH;

	x2 -= x1;
	if (x2 == 0) {
		screen += x1;

		while (y2-- != 0) {
			*screen = colour;
			screen += SCREEN_WIDTH;
		}

		return;
	}

	if (x2 < 0) {
		x2 = -x2;
		increment = -1;
	}

	if (x2 < y2) {
		int16 full = y2;
		int16 half = y2 / 2;
		screen += x1;
		while (true) {
			*screen = colour;
			if (y2-- == 0) return;
			screen += SCREEN_WIDTH;
			half -= x2;
			if (half < 0) {
				half += full;
				screen += increment;
			}
		}
	} else {
		int16 full = x2;
		int16 half = x2 / 2;
		screen += x1;
		while (true) {
			*screen = colour;
			if (x2-- == 0) return;
			screen += increment;
			half -= y2;
			if (half < 0) {
				half += full;
				screen += SCREEN_WIDTH;
			}
		}
	}
}

/**
 * Sets the clipping area.
 * @param left The left clipping.
 * @param top The top clipping.
 * @param right The right clipping.
 * @param bottom The bottom clipping.
 */
void GUI_SetClippingArea(uint16 left, uint16 top, uint16 right, uint16 bottom)
{
	g_clipping.left   = left;
	g_clipping.top    = top;
	g_clipping.right  = right;
	g_clipping.bottom = bottom;
}

/**
 * Wrapper around GFX_Screen_Copy. Protects against wrong input values.
 * @param xSrc The X-coordinate on the source divided by 8.
 * @param ySrc The Y-coordinate on the source.
 * @param xDst The X-coordinate on the destination divided by 8.
 * @param yDst The Y-coordinate on the destination.
 * @param width The width divided by 8.
 * @param height The height.
 * @param screenSrc The ID of the source screen.
 * @param screenDst The ID of the destination screen.
 */
void GUI_Screen_Copy(int16 xSrc, int16 ySrc, int16 xDst, int16 yDst, int16 width, int16 height, int16 screenSrc, int16 screenDst)
{
	if (width  > SCREEN_WIDTH / 8) width  = SCREEN_WIDTH / 8;
	if (height > SCREEN_HEIGHT)    height = SCREEN_HEIGHT;

	if (xSrc < 0) {
		xDst -= xSrc;
		width += xSrc;
		xSrc = 0;
	}

	if (xSrc >= SCREEN_WIDTH / 8 || xDst >= SCREEN_WIDTH / 8) return;

	if (xDst < 0) {
		xSrc -= xDst;
		width += xDst;
		xDst = 0;
	}

	if (ySrc < 0) {
		yDst -= ySrc;
		height += ySrc;
		ySrc = 0;
	}

	if (yDst < 0) {
		ySrc -= yDst;
		height += yDst;
		yDst = 0;
	}

	GFX_Screen_Copy(xSrc, ySrc, xDst, yDst, width, height, screenSrc, screenDst);
}

static uint32 GUI_FactoryWindow_CreateWidgets()
{
	MSVC_PACKED_BEGIN
	typedef struct WidgetInfo {
		/* 0000(1)   */ PACK uint8  offsetX;      /*!< ?? */
		/* 0001(1)   */ PACK uint8  offsetY;      /*!< ?? */
		/* 0002(2)   */ PACK uint16 spriteID;     /*!< ?? */
		/* 0004(1)   */ PACK uint8  width;        /*!< ?? */
		/* 0005(1)   */ PACK uint8  height;       /*!< ?? */
		/* 0006(2)   */ PACK int16  shortcut;     /*!< ?? */
		/* 0008(2)   */ PACK uint16 flags;        /*!< ?? */
		/* 000A(4)   */ PACK csip32 clickProc;    /*!< ?? */
	} GCC_PACKED WidgetInfo;
	MSVC_PACKED_END
	assert_compile(sizeof(WidgetInfo) == 0xE);

	uint16 i;
	uint16 count = 0;
	WidgetInfo *wi = (WidgetInfo *)&emu_get_memory8(0x2C34, 0x00, 0x00);
	Widget *w = (Widget *)emu_get_memorycsip(g_global->factoryWindowWidgets);

	memset(w, 0, 13 * sizeof(Widget));

	Sprites_Load(2, 7, g_sprites);

	for (i = 0; i < 13; i++, wi++) {
		if ((i == 8 || i == 9 || i == 10 || i == 12) && g_global->factoryWindowStarport == 0) continue;
		if (i == 11 && g_global->factoryWindowStarport != 0) continue;
		if (i == 7 && g_global->factoryWindowUpgradeCost == 0) continue;

		count++;

		w->index     = i + 46;
		w->state.all = 0x0;
		w->offsetX   = wi->offsetX << 3;
		w->offsetY   = wi->offsetY;
		w->flags.all = wi->flags;
		w->shortcut  = (wi->shortcut < 0) ? abs(wi->shortcut) : GUI_Widget_GetShortcut(*String_Get_ByIndex(wi->shortcut));
		w->clickProc = wi->clickProc;
		w->width     = wi->width << 3;
		w->height    = wi->height;

		if (wi->spriteID == 0xFFFF) {
			w->drawModeNormal   = DRAW_MODE_NONE;
			w->drawModeSelected = DRAW_MODE_NONE;
			w->drawModeDown     = DRAW_MODE_NONE;
		} else {
			w->drawModeNormal   = DRAW_MODE_SPRITE;
			w->drawModeSelected = DRAW_MODE_SPRITE;
			w->drawModeDown     = DRAW_MODE_SPRITE;
			w->drawProcNormal   = g_sprites[wi->spriteID];
			w->drawProcSelected = g_sprites[wi->spriteID + 1];
			w->drawProcDown     = g_sprites[wi->spriteID + 1];
		}

		if (i != 0) {
			g_global->variable_7FA2 = emu_Global_GetCSIP(GUI_Widget_Link((Widget *)emu_get_memorycsip(g_global->variable_7FA2), w));
		} else {
			g_global->variable_7FA2 = emu_Global_GetCSIP(w);
		}

		w++;
	}

	GUI_Widget_DrawAll((Widget *)emu_get_memorycsip(g_global->variable_7FA2));

	return count * sizeof(Widget);
}

static uint32 GUI_FactoryWindow_LoadGraymapTbl()
{
	uint8 fileID;

	fileID = File_Open("GRAYRMAP.TBL", 1);
	File_Read(fileID, emu_get_memorycsip(g_global->factoryWindowGraymapTbl), 256);
	File_Close(fileID);

	return 256;
}

static uint16 GUI_FactoryWindow_CalculateStarportPrice(uint16 credits)
{
	credits = (credits / 10) * 4 + (credits / 10) * (Tools_RandomRange(0, 6) + Tools_RandomRange(0, 6));

	return min(credits, 999);
}

static int GUI_FactoryWindow_Sorter(const void *a, const void *b)
{
	const FactoryWindowItem *pa = a;
	const FactoryWindowItem *pb = b;

	return pb->sortPriority - pa->sortPriority;
}

static void GUI_FactoryWindow_InitItems()
{
	g_global->factoryWindowTotal = 0;
	g_global->factoryWindowSelected = 0;
	g_global->factoryWindowBase = 0;

	memset(g_global->factoryWindowItems, 0, 25 * sizeof(FactoryWindowItem));

	if (g_global->factoryWindowStarport != 0) {
		uint16 seconds = (g_global->tickGlobal - g_global->tickScenarioStart) / 60;
		uint16 seed = (seconds / 60) + g_global->scenarioID + g_global->playerHouseID;
		seed *= seed;

		srand(seed);

		sprintf((char *)g_global->variable_9939, "sec(%u) seed(%u) ", seconds, seed);
	}

	if (g_global->factoryWindowConstructionYard == 0) {
		uint16 i;

		for (i = 0; i < UNIT_MAX; i++) {
			ObjectInfo *oi = &g_unitInfo[i].o;

			if (oi->available == 0) continue;

			g_global->factoryWindowItems[g_global->factoryWindowTotal].objectInfo = emu_Global_GetCSIP(oi);
			g_global->factoryWindowItems[g_global->factoryWindowTotal].objectType = i;

			if (g_global->factoryWindowStarport != 0) {
				g_global->factoryWindowItems[g_global->factoryWindowTotal].credits = GUI_FactoryWindow_CalculateStarportPrice(oi->buildCredits);
			} else {
				g_global->factoryWindowItems[g_global->factoryWindowTotal].credits = oi->buildCredits;
			}

			g_global->factoryWindowItems[g_global->factoryWindowTotal].sortPriority = oi->sortPriority;

			g_global->factoryWindowTotal++;
		}
	} else {
		uint16 i;

		for (i = 0; i < STRUCTURE_MAX; i++) {
			ObjectInfo *oi = &g_structureInfo[i].o;

			if (oi->available == 0) continue;

			g_global->factoryWindowItems[g_global->factoryWindowTotal].objectInfo    = emu_Global_GetCSIP(oi);
			g_global->factoryWindowItems[g_global->factoryWindowTotal].objectType    = i;
			g_global->factoryWindowItems[g_global->factoryWindowTotal].credits       = oi->buildCredits;
			g_global->factoryWindowItems[g_global->factoryWindowTotal].sortPriority  = oi->sortPriority;

			if (i == 0 || i == 1) g_global->factoryWindowItems[g_global->factoryWindowTotal].sortPriority = 0x64;

			g_global->factoryWindowTotal++;
		}
	}

	if (g_global->factoryWindowTotal == 0) {
		GUI_DisplayModalMessage("ERROR: No items in construction list!", 0xFFFF);
		PrepareEnd();
		exit(0);
	}

	qsort(g_global->factoryWindowItems, g_global->factoryWindowTotal, sizeof(FactoryWindowItem), GUI_FactoryWindow_Sorter);
}

static void GUI_FactoryWindow_Init()
{
	uint16 oldScreenID;
	csip32 wsaBuffer;
	uint16 loc0A;
	uint16 locdi;
	uint32 size;
	int16 i;
	ObjectInfo *oi;

	oldScreenID = GUI_Screen_SetActive(2);

	Sprites_LoadImage("CHOAM.CPS", 3, 3, NULL, 1);

	GUI_Palette_RemapScreen(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 2, g_global->variable_3C42);

	GUI_DrawSprite(2, g_sprites[11], 192, 0, 0, 0);

	loc0A = emu_get_memory8(0x2C34, g_global->playerHouseID * 2, 0xB6);
	locdi = emu_get_memory8(0x2C34, g_global->playerHouseID * 2, 0xB7);

	GUI_Screen_Copy(loc0A, locdi, 0, 8, 7, 40, 2, 2);
	GUI_Screen_Copy(loc0A, locdi, 0, 152, 7, 40, 2, 2);

	g_global->variable_7FA6 = g_global->variable_6CD3[2][1];

	g_global->factoryWindowWidgets = Screen_GetSegment_ByIndex_1(5);

	size = GUI_FactoryWindow_CreateWidgets();

	g_global->variable_7FA6 -= size;

	g_global->factoryWindowGraymapTbl = g_global->factoryWindowWidgets;
	g_global->factoryWindowGraymapTbl.csip += size;

	size = GUI_FactoryWindow_LoadGraymapTbl();

	g_global->variable_7FAE = g_global->factoryWindowGraymapTbl;
	g_global->variable_7FAE.csip += size;

	g_global->variable_7FA6 -= size;

	GUI_FactoryWindow_InitItems();

	for (i = g_global->factoryWindowTotal; i < 4; i++) GUI_Widget_MakeInvisible(GUI_Widget_Get_ByIndex((Widget *)emu_get_memorycsip(g_global->variable_7FA2), i + 46));

	for (i = 0; i < 4; i++) {
		FactoryWindowItem *item = GUI_FactoryWindow_GetItem(i);

		if (item == NULL) continue;

		oi = (ObjectInfo *)emu_get_memorycsip(item->objectInfo);
		if (oi->available == -1) {
			GUI_DrawSprite(2, g_sprites[oi->spriteID], 72, 24 + i * 32, 0, 0x100, emu_get_memorycsip(g_global->factoryWindowGraymapTbl), 1);
		} else {
			GUI_DrawSprite(2, g_sprites[oi->spriteID], 72, 24 + i * 32, 0, 0);
		}
	}

	g_global->factoryWindowBase = 0;
	g_global->factoryWindowSelected = 0;

	oi = (ObjectInfo *)emu_get_memorycsip(g_global->factoryWindowItems[0].objectInfo);
	{
		csip32 nullcsip;
		nullcsip.csip = 0x0;
		wsaBuffer = WSA_LoadFile((char *)emu_get_memorycsip(oi->wsa), g_global->variable_7FAE, g_global->variable_7FA6, 0, nullcsip);
	}

	WSA_DisplayFrame(wsaBuffer, 0, 128, 48, 2);

	WSA_Unload(wsaBuffer);

	GUI_Mouse_Hide_Safe();
	GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 2, 0);
	GUI_Mouse_Show_Safe();

	GUI_DrawFilledRectangle(64, 0, 112, SCREEN_HEIGHT - 1, GFX_GetPixel(72, 23));

	GUI_FactoryWindow_PrepareScrollList();

	GUI_Screen_SetActive(0);

	GUI_FactoryWindow_DrawDetails();

	GUI_DrawCredits((uint8)g_global->playerHouseID, 1);

	GUI_Screen_SetActive(oldScreenID);
}

/**
 * Display the window where you can order/build stuff for a structure.
 * @param var06 Unknown.
 * @param isStarPort True if this is for a starport.
 * @param var0A Unknown.
 * @return Unknown value.
 */
FactoryResult GUI_DisplayFactoryWindow(bool isConstructionYard, bool isStarPort, uint16 upgradeCost)
{
	uint16 oldScreenID = GUI_Screen_SetActive(0);
	uint8 backup[3];

	memcpy(emu_get_memorycsip(g_global->variable_3C32) + 765, backup, 3);

	g_global->factoryWindowConstructionYard = isConstructionYard;
	g_global->factoryWindowStarport = isStarPort;
	g_global->factoryWindowUpgradeCost = upgradeCost;
	g_global->factoryWindowOrdered = 0;

	GUI_FactoryWindow_Init();

	GUI_FactoryWindow_UpdateSelection(true);

	g_global->factoryWindowResult = FACTORY_CONTINUE;
	while (g_global->factoryWindowResult == FACTORY_CONTINUE) {
		uint16 event;

		GUI_DrawCredits((uint8)g_global->playerHouseID, 0);

		GUI_FactoryWindow_UpdateSelection(false);

		event = GUI_Widget_HandleEvents((g_global->variable_7FA2.csip == 0x0) ? NULL : (Widget *)emu_get_memorycsip(g_global->variable_7FA2));

		if (event == 0x6E) GUI_Production_ResumeGame_Click(NULL);

		GUI_PaletteAnimate();
	}

	GUI_DrawCredits((uint8)g_global->playerHouseID, 1);

	GUI_Screen_SetActive(oldScreenID);

	GUI_FactoryWindow_B495_0F30();

	memcpy(backup, emu_get_memorycsip(g_global->variable_3C32) + 765, 3);

	GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

	/* Visible credits have to be reset, as it might not be the real value */
	g_global->playerCredits = 0xFFFF;

	return g_global->factoryWindowResult;
}

char *GUI_String_Get_ByIndex(uint16 stringID)
{
	switch (stringID) {
		case 0xFFFB: case 0xFFFC: case 0xFFFD: case 0xFFFE: case 0xFFFF: {
			char *s = g_global->savegameDesc[abs((int16)stringID + 1)];
			if (*s == '\0') return NULL;
			return s;
		}

		case 0xFFF6:
			stringID = (g_global->gameConfig.music != 0) ? 0x69 : 0x6A; /* "ON" : "OFF" */
			break;

		case 0xFFF5:
			stringID = (g_global->gameConfig.sounds != 0) ? 0x69 : 0x6A; /* "ON" : "OFF" */
			break;

		case 0xFFF4:
			stringID = g_global->variable_2AAD[g_global->gameSpeed];
			break;

		case 0xFFF3:
			stringID = (g_global->gameConfig.hints != 0) ? 0x69 : 0x6A; /* "ON" : "OFF" */
			break;

		case 0xFFF2:
			stringID = (g_global->gameConfig.autoScroll != 0) ? 0x69 : 0x6A; /* "ON" : "OFF" */
			break;

		default: break;
	}

	return String_Get_ByIndex(stringID);
}

static void GUI_StrategicMap_AnimateArrows()
{
	uint8 *palette;

	if (g_global->variable_81B6 >= g_global->variable_76AC) return;

	g_global->variable_81B6 = g_global->variable_76AC + 7;

	g_global->variable_2B10 = (g_global->variable_2B10 + 1) % 4;

	palette = emu_get_memorycsip(g_global->variable_3C32);

	memcpy(palette + 251 * 3, g_global->variable_81BA + g_global->variable_2B10 * 3, 4 * 3);

	GFX_SetPalette(palette);
}

static void GUI_StrategicMap_AnimateSelected(uint16 selected, StrategicMapData *data)
{
	char key[4];
	int16 x;
	int16 y;
	csip32 sprite;
	uint16 width;
	uint16 height;
	uint16 i;

	Unknown_B4B8_110D((uint8)g_global->playerHouseID);

	for (i = 0; i < 20; i++) {
		GUI_StrategicMap_AnimateArrows();

		if (data[i].index == 0 || data[i].index == selected) continue;

		GUI_Mouse_Hide_Safe();
		GFX_Screen_Copy2(i * 16, 0, data[i].offsetX, data[i].offsetY, 16, 16, 2, 0, false);
		GUI_Mouse_Show_Safe();
	}

	sprintf(key, "%d", selected);

	Ini_GetString("PIECES", key, NULL, (char *)g_global->variable_9939, 80, (char *)emu_get_memorycsip(g_global->REGION_INI));
	sscanf((char *)g_global->variable_9939, "%hd,%hd", &x, &y);

	sprite = Sprites_GetCSIP(g_global->PIECES_SHP, selected);
	width = Sprite_GetWidth(sprite);
	height = Sprite_GetHeight(sprite);

	x += 8;
	y += 24;

	GUI_Mouse_Hide_Safe();
	GFX_Screen_Copy2(x, y, 16, 16, width, height, 0, 2, false);
	GUI_Mouse_Show_Safe();

	GFX_Screen_Copy2(16, 16, 176, 16, width, height, 2, 2, false);

	GUI_DrawSprite(2, sprite, 16, 16, 0, 0x100, emu_get_memorycsip(g_global->variable_3C42), 1);

	for (i = 0; i < 20; i++) {
		GUI_StrategicMap_AnimateArrows();

		if (data[i].index != selected) continue;

		GUI_DrawSprite(2, Sprites_GetCSIP(g_global->ARROWS_SHP, data[i].arrow), data[i].offsetX + 16 - x, data[i].offsetY + 16 - y, 0, 0x100, emu_get_memorycsip(g_global->variable_3C42), 1);
	}

	for (i = 0; i < 4; i++) {
		GUI_Mouse_Hide_Safe();
		GFX_Screen_Copy2((i % 2 == 0) ? 16 : 176, 16, x, y, width, height, 2, 0, false);
		GUI_Mouse_Show_Safe();

		g_global->variable_76B4 = 20;
		while (g_global->variable_76B4 != 0) GUI_StrategicMap_AnimateArrows();
	}
}

static bool GUI_StrategicMap_Var2AF4_Get(uint16 region)
{
	return (g_global->variable_2AF4 & (1 << region)) != 0;
}

static void GUI_StrategicMap_Var2AF4_Set(uint16 region, bool set)
{
	if (set) {
		g_global->variable_2AF4 |= (1 << region);
	} else {
		g_global->variable_2AF4 &= ~(1 << region);
	}
}

static int16 GUI_StrategicMap_ClickedRegion()
{
	GUI_StrategicMap_AnimateArrows();

	emu_push(emu_cs); emu_push(0x089B); emu_cs = 0x29E8; emu_Input_Keyboard_NextKey();
	if (emu_ax == 0) return 0;

	emu_push(emu_cs); emu_push(0x08A8); emu_cs = 0x29E8; f__29E8_07FA_0020_177A();
	if (emu_ax != 0xC6 && emu_ax != 0xC7) return 0;

	return emu_get_memorycsip(g_global->RGNCLK_CPS)[(g_global->mouseClickY - 24) * 304 + g_global->mouseClickX - 8];
}

static bool GUI_StrategicMap_FastForwardToggleWithESC()
{
	emu_push(emu_cs); emu_push(0x13CA); emu_cs = 0x29E8; emu_Input_Keyboard_NextKey();
	if (emu_ax == 0) return g_global->strategicMapFastForward != 0;

	emu_push(emu_cs); emu_push(0x13D3); emu_cs = 0x29E8; f__29E8_07FA_0020_177A();
	if (emu_ax != 0x1B) return g_global->strategicMapFastForward != 0;

	g_global->strategicMapFastForward = (g_global->strategicMapFastForward == 0) ? 1 : 0;

	emu_push(emu_cs); emu_push(0x13E8); emu_cs = 0x29E8; emu_Input_History_Clear();

	return g_global->strategicMapFastForward != 0;
}

static void GUI_StrategicMap_DrawText(char *string)
{
	uint16 oldScreenID;
	uint16 y;

	oldScreenID = GUI_Screen_SetActive(2);

	GUI_Screen_Copy(8, 165, 8, 186, 24, 14, 0, 2);

	GUI_DrawFilledRectangle(64, 172, 255, 185, GFX_GetPixel(64, 186));

	GUI_DrawText_Wrapper(string, 64, 175, 12, 0, 0x12);

	while (g_global->variable_76AC + 90 < g_global->variable_2B0C) sleep(0);

	for (y = 185; y > 172; y--) {
		GUI_Screen_Copy(8, y, 8, 165, 24, 14, 2, 0);

		g_global->variable_76B4 = 3;

		while (g_global->variable_76B4 != 0) {
			if (GUI_StrategicMap_FastForwardToggleWithESC()) break;
		}
	}

	g_global->variable_2B0C = g_global->variable_76AC + 90;

	GUI_Screen_SetActive(oldScreenID);
}

static uint16 GUI_StrategicMap_ScenarioSelection(uint16 campaignID)
{
	uint16 count;
	char key[6];
	bool loop = true;
	bool loc12 = true;
	char category[16];
	StrategicMapData data[20];
	uint16 scenarioID;
	uint16 region;
	uint16 i;

	Unknown_B4B8_110D((uint8)g_global->playerHouseID);

	sprintf(category, "GROUP%d", campaignID);

	memset(data, 0, 20 * sizeof(StrategicMapData));

	for (i = 0; i < 20; i++) {
		sprintf(key, "REG%d", i + 1);

		if (Ini_GetString(category, key, NULL, (char *)g_global->variable_9939, 80, (char *)emu_get_memorycsip(g_global->REGION_INI)) == NULL) break;

		sscanf((char *)g_global->variable_9939, "%hd,%hd,%hd,%hd", &data[i].index, &data[i].arrow, &data[i].offsetX, &data[i].offsetY);

		if (!GUI_StrategicMap_Var2AF4_Get(data[i].index)) loc12 = false;

		GFX_Screen_Copy2(data[i].offsetX, data[i].offsetY, i * 16, 152, 16, 16, 2, 2, false);
		GFX_Screen_Copy2(data[i].offsetX, data[i].offsetY, i * 16, 0, 16, 16, 2, 2, false);
		GUI_DrawSprite(2, Sprites_GetCSIP(g_global->ARROWS_SHP, data[i].arrow), i * 16, 152, 0, 0x100, emu_get_memorycsip(g_global->variable_3C42), 1);
	}

	count = i;

	if (loc12) {
		for (i = 0; i < count; i++) {
			GUI_StrategicMap_Var2AF4_Set(data[i].index, false);
		}
	} else {
		for (i = 0; i < count; i++) {
			if (GUI_StrategicMap_Var2AF4_Get(data[i].index)) data[i].index = 0;
		}
	}

	GUI_Mouse_Hide_Safe();

	for (i = 0; i < count; i++) {
		if (data[i].index == 0) continue;

		GFX_Screen_Copy2(i * 16, 152, data[i].offsetX, data[i].offsetY, 16, 16, 2, 0, false);
	}

	GUI_Mouse_Show_Safe();

	emu_push(emu_cs); emu_push(0x07F2); emu_cs = 0x29E8; emu_Input_History_Clear();

	while (loop) {
		region = GUI_StrategicMap_ClickedRegion();

		if (region == 0) continue;

		for (i = 0; i < count; i++) {
			GUI_StrategicMap_AnimateArrows();

			if (data[i].index == region) {
				loop = false;
				scenarioID = i;
				break;
			}
		}
	}

	GUI_StrategicMap_Var2AF4_Set(region, true);

	GUI_StrategicMap_DrawText("");

	GUI_StrategicMap_AnimateSelected(region, data);

	scenarioID += (campaignID - 1) * 3 + 2;

	if (campaignID > 7) scenarioID--;
	if (campaignID > 8) scenarioID--;

	return scenarioID;
}

static void GUI_StrategicMap_ReadHouseRegions(uint8 houseID, uint16 campaignID)
{
	char key[4];
	char buffer[100];
	char *s = buffer;
	uint16 *regions = (uint16 *)emu_get_memorycsip(g_global->regions);

	strncpy(key, (char *)emu_get_memorycsip(g_houseInfo[houseID].name), 3);
	key[3] = '\0';

	sprintf((char *)g_global->variable_9939, "GROUP%d", campaignID);

	if (Ini_GetString((char *)g_global->variable_9939, key, NULL, buffer, 99, (char *)emu_get_memorycsip(g_global->REGION_INI)) == NULL) return;

	while (*s != '\0') {
		uint16 region = atoi(s);

		if (region != 0) regions[region] = houseID;

		while (*s != '\0') {
			if (*s++ == ',') break;
		}
	}
}

static void GUI_StrategicMap_DrawRegion(uint8 houseId, uint16 region, bool progressive)
{
	char key[4];
	int16 x;
	int16 y;
	csip32 sprite;

	Unknown_B4B8_110D(houseId);

	sprintf(key, "%d", region);

	Ini_GetString("PIECES", key, NULL, (char *)g_global->variable_9939, 80, (char *)emu_get_memorycsip(g_global->REGION_INI));
	sscanf((char *)g_global->variable_9939, "%hd,%hd", &x, &y);

	sprite = Sprites_GetCSIP(g_global->PIECES_SHP, region);

	GUI_DrawSprite(3, sprite, x + 8, y + 24, 0, 0x100, emu_get_memorycsip(g_global->variable_3C42), 1);

	if (!progressive) return;

	GUI_Screen_FadeIn2(x + 8, y + 24, Sprite_GetWidth(sprite), Sprite_GetHeight(sprite), 2, 0, GUI_StrategicMap_FastForwardToggleWithESC() ? 0 : 1, false);
}

static void GUI_StrategicMap_PrepareRegions(uint16 campaignID)
{
	uint16 i;
	uint16 *regions = (uint16 *)emu_get_memorycsip(g_global->regions);

	for (i = 0; i < campaignID; i++) {
		GUI_StrategicMap_ReadHouseRegions(HOUSE_HARKONNEN, i + 1);
		GUI_StrategicMap_ReadHouseRegions(HOUSE_ATREIDES, i + 1);
		GUI_StrategicMap_ReadHouseRegions(HOUSE_ORDOS, i + 1);
		GUI_StrategicMap_ReadHouseRegions(HOUSE_SARDAUKAR, i + 1);
	}

	for (i = 0; i < regions[0]; i++) {
		if (regions[i + 1] == 0xFFFF) continue;

		GUI_StrategicMap_DrawRegion((uint8)regions[i + 1], i + 1, false);
	}
}

static void GUI_StrategicMap_ShowProgression(uint16 campaignID)
{
	char key[10];
	char category[10];
	char buffer[100];
	uint16 i;

	sprintf(category, "GROUP%d", campaignID);

	for (i = 0; i < 6; i++) {
		uint8 houseID = (g_global->playerHouseID + i) % 6;
		char *s = buffer;

		strncpy(key, (char *)emu_get_memorycsip(g_houseInfo[houseID].name), 3);
		key[3] = '\0';

		if (Ini_GetString(category, key, NULL, buffer, 99, (char *)emu_get_memorycsip(g_global->REGION_INI)) == NULL) continue;

		while (*s != '\0') {
			uint16 region = atoi(s);

			if (region != 0) {
				sprintf(key, "%sTXT%d", g_global->string_2AF8[g_global->language], region);

				if (Ini_GetString(category, key, NULL, (char *)g_global->variable_9939, 80, (char *)emu_get_memorycsip(g_global->REGION_INI)) != NULL) {
					GUI_StrategicMap_DrawText((char *)g_global->variable_9939);
				}

				GUI_StrategicMap_DrawRegion(houseID, region, true);
			}

			while (*s != '\0') {
				if (*s++ == ',') break;
			}
		}
	}

	GUI_StrategicMap_DrawText("");
}

uint16 GUI_StrategicMap_Show(uint16 campaignID, bool win)
{
	uint16 scenarioID;
	uint16 previousCampaignID;
	uint16 x;
	uint16 y;
	uint16 oldScreenID;
	uint8 *loc30A;
	uint8 loc316[12];
	csip32 csip30A;

	if (campaignID == 0) return 1;

	emu_sp -= 0x300;
	csip30A.s.cs = emu_ss;
	csip30A.s.ip = emu_sp;
	loc30A = emu_get_memorycsip(csip30A);

	Tools_Sleep(10);
	Music_Play(0x1D);

	memset(loc30A, 0, 0x300);

	previousCampaignID = campaignID - (win ? 1 : 0);
	oldScreenID = GUI_Screen_SetActive(4);

	Unknown_259E_0006(csip30A, 15);

	Mouse_SetRegion(8, 24, 311, 143);

	GUI_Mouse_SetPosition(160, 84);

	Sprites_LoadImage("MAPMACH.CPS", 5, 5, emu_get_memorycsip(g_global->variable_998A), 1);

	GUI_Palette_RemapScreen(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5, g_global->variable_3C42);

	GFX_Screen_Copy3(5, 4);

	x = 0;
	y = 0;

	switch (g_global->playerHouseID) {
		case HOUSE_HARKONNEN:
			x = 0;
			y = 152;
			break;

		default:
			x = 33;
			y = 152;
			break;

		case HOUSE_ORDOS:
			x = 1;
			y = 24;
			break;
	}

	memcpy(loc316, emu_get_memorycsip(g_global->variable_3C32) + (251 * 3), 12);
	memcpy(g_global->variable_81BA, emu_get_memorycsip(g_global->variable_3C32) + (144 + (g_global->playerHouseID * 16)) * 3, 12);
	memcpy(g_global->variable_81C6, g_global->variable_81BA, 12);

	GUI_Screen_Copy(x, y, 0, 152, 7, 40, 4, 4);
	GUI_Screen_Copy(x, y, 33, 152, 7, 40, 4, 4);

	switch (g_global->language) {
		case LANGUAGE_GERMAN:
			GUI_Screen_Copy(1, 120, 1, 0, 38, 24, 4, 4);
			break;

		case LANGUAGE_FRENCH:
			GUI_Screen_Copy(1, 96, 1, 0, 38, 24, 4, 4);
			break;

		default: break;
	}

	GUI_DrawFilledRectangle(8, 24, 311, 143, 12);

	GUI_Mouse_Hide_Safe();
	GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 4, 0);
	Unknown_259E_0006(g_global->variable_3C32, 15);
	GUI_Mouse_Show_Safe();

	g_global->strategicMapFastForward = 0;

	if (win && campaignID == 1) {
		Sprites_LoadImage("PLANET.CPS", 3, 3, emu_get_memorycsip(g_global->variable_998A), 1);

		GFX_Screen_Copy3(3, 2);

		/* "Three Houses have come to Dune." */
		GUI_StrategicMap_DrawText(String_Get_ByIndex(0x11B));

		GUI_Screen_FadeIn2(8, 24, 304, 120, 2, 0, 0, false);

		emu_push(emu_cs); emu_push(0x0316); emu_cs = 0x29E8; emu_Input_History_Clear();

		g_global->variable_76B4 = 120;

		Sprites_CPS_LoadRegionClick();

		while (g_global->variable_76B4 != 0) {
			if (GUI_StrategicMap_FastForwardToggleWithESC()) break;
		}

		Sprites_LoadImage("DUNEMAP.CPS", 3 , 3, emu_get_memorycsip(g_global->variable_998A), 1);

		GFX_Screen_Copy3(3, 2);

		/* "To take control of the land." */
		GUI_StrategicMap_DrawText(String_Get_ByIndex(0x11C));

		GUI_Screen_FadeIn2(8, 24, 304, 120, 2, 0, GUI_StrategicMap_FastForwardToggleWithESC() ? 0 : 1, false);

		g_global->variable_76B4 = 60;

		while (g_global->variable_76B4 != 0) {
			if (GUI_StrategicMap_FastForwardToggleWithESC()) break;
		}

		/* "That has become divided." */
		GUI_StrategicMap_DrawText(String_Get_ByIndex(0x11D));
	} else {
		Sprites_CPS_LoadRegionClick();
	}

	Sprites_LoadImage("DUNERGN.CPS", 3, 3, emu_get_memorycsip(g_global->variable_998A), 1);

	GFX_Screen_Copy3(3, 2);

	GUI_Screen_SetActive(2);

	GUI_StrategicMap_PrepareRegions(previousCampaignID);

	if (GUI_StrategicMap_FastForwardToggleWithESC()) {
		GUI_Screen_Copy(1, 24, 1, 24, 38, 120, 2, 0);
	} else {
		GUI_Screen_FadeIn2(8, 24, 304, 120, 2, 0, 0, false);
	}

	GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 0, 2);

	if (campaignID != previousCampaignID) GUI_StrategicMap_ShowProgression(campaignID);

	GUI_Mouse_Show_Safe();

	if (*(uint16 *)emu_get_memorycsip(g_global->regions) >= campaignID) {
		/* "Select your next region" */
		GUI_StrategicMap_DrawText(String_Get_ByIndex(0x11E));

		scenarioID = GUI_StrategicMap_ScenarioSelection(campaignID);
	} else {
		scenarioID = 0;
	}

	Driver_Music_FadeOut();

	GUI_Screen_SetActive(oldScreenID);

	Mouse_SetRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

	emu_push(emu_cs); emu_push(0x0529); emu_cs = 0x29E8; emu_Input_History_Clear();

	memcpy(emu_get_memorycsip(g_global->variable_3C32) + (251 * 3), loc316, 12);

	Unknown_259E_0006(csip30A, 15);

	GUI_Mouse_Hide_Safe();
	GUI_ClearScreen(0);
	GUI_Mouse_Show_Safe();

	GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

	emu_sp += 0x300;

	return scenarioID;
}

/**
 * Clear the screen.
 * @param screenID Which screen to clear.
 */
void GUI_ClearScreen(uint16 screenID)
{
	uint16 oldScreenID = GUI_Screen_SetActive(screenID);

	GFX_ClearScreen();

	GUI_Screen_SetActive(oldScreenID);
}

/**
 * Draw a string to the screen using a fixed width for each char.
 *
 * @param string The string to draw.
 * @param left The most left position where to draw the string.
 * @param top The most top position where to draw the string.
 * @param fgColour The foreground colour of the text.
 * @param bgColour The background colour of the text.
 * @param charWidth The width of a char.
 */
void GUI_DrawText_Monospace(char *string, uint16 left, uint16 top, uint8 fgColour, uint8 bgColour, uint16 charWidth)
{
	char s[2] = " ";

	while (*string != '\0') {
		*s = *string++;
		GUI_DrawText(s, left, top, fgColour, bgColour);
		left += charWidth;
	}
}

void GUI_FactoryWindow_B495_0F30()
{
	GUI_Mouse_Hide_Safe();
	GFX_Screen_Copy2(69, ((g_global->factoryWindowSelected + 1) * 32) + 5, 69, (g_global->factoryWindowSelected * 32) + 21, 38, 30, 2, 0, false);
	GUI_Mouse_Show_Safe();
}

FactoryWindowItem *GUI_FactoryWindow_GetItem(int16 offset)
{
	offset += g_global->factoryWindowBase;

	if (offset < 0 || offset >= g_global->factoryWindowTotal) return NULL;

	return &g_global->factoryWindowItems[offset];
}

void GUI_FactoryWindow_DrawDetails()
{
	uint16 oldScreenID;
	FactoryWindowItem *item = GUI_FactoryWindow_GetItem(g_global->factoryWindowSelected);
	ObjectInfo *oi = (ObjectInfo *)emu_get_memorycsip(item->objectInfo);
	csip32 wsa;

	oldScreenID = GUI_Screen_SetActive(2);

	{
		csip32 nullcsip;
		nullcsip.csip = 0x0;
		wsa = WSA_LoadFile((char *)emu_get_memorycsip(oi->wsa), g_global->variable_7FAE, g_global->variable_7FA6, 0, nullcsip);
	}

	WSA_DisplayFrame(wsa, 0, 128, 48, 2);
	WSA_Unload(wsa);

	if (g_global->factoryWindowConstructionYard != 0) {
		int16 x = 288;
		int16 y = 136;
		csip32 sprite;
		uint16 width;
		StructureInfo *si;
		uint16 i;
		uint16 j;

		GUI_DrawSprite(g_global->screenActiveID, g_sprites[64], x, y, 0, 0);
		x++;
		y++;

		sprite = g_sprites[24];
		width = Sprite_GetWidth(sprite) + 1;
		si = &g_structureInfo[item->objectType];

		for (j = 0; j < g_global->layoutSize[si->layout][1]; j++) {
			for (i = 0; i < g_global->layoutSize[si->layout][0]; i++) {
				GUI_DrawSprite(g_global->screenActiveID, sprite, x + i * width, y + j * width, 0, 0);
			}
		}
	}

	if (oi->available == -1) {
		GUI_Palette_RemapScreen(128, 48, 184, 112, 2, g_global->factoryWindowGraymapTbl);

		if (g_global->factoryWindowStarport != 0) {
			/* "OUT OF STOCK" */
			GUI_DrawText_Wrapper(String_Get_ByIndex(0xB9), 220, 99, 6, 0, 0x132);
		} else {
			/* "NEED STRUCTURE UPGRADE" */
			GUI_DrawText_Wrapper(String_Get_ByIndex(0xBA), 220, 94, 6, 0, 0x132);

			if (g_global->factoryWindowUpgradeCost != 0) {
				/* "Upgrade Cost : %d" */
				GUI_DrawText_Wrapper(String_Get_ByIndex(0xBB), 220, 104, 6, 0, 0x132, g_global->factoryWindowUpgradeCost);
			} else {
				/* "Repair structure first" */
				GUI_DrawText_Wrapper(String_Get_ByIndex(0x14D), 220, 104, 6, 0, 0x132);
			}
		}
	} else {
		if (g_global->factoryWindowStarport != 0) {
			GUI_Screen_Copy(16, 99, 16, 160, 23, 9, 2, 2);
			GUI_Screen_Copy(16, 99, 16, 169, 23, 9, 2, 2);
			/* "OUT OF STOCK" */
			GUI_DrawText_Wrapper(String_Get_ByIndex(0xB9), 220, 169, 6, 0, 0x132);

			GUI_FactoryWindow_UpdateDetails();
		}
	}

	GUI_Mouse_Hide_Safe();
	GUI_Screen_Copy(16, 48, 16, 48, 23, 112, 2, oldScreenID);
	GUI_Mouse_Show_Safe();

	GUI_Screen_SetActive(oldScreenID);

	GUI_FactoryWindow_DrawCaption(NULL);
}

void GUI_FactoryWindow_DrawCaption(char *caption)
{
	uint16 oldScreenID;

	oldScreenID = GUI_Screen_SetActive(2);

	GUI_DrawFilledRectangle(128, 21, 310, 35, 116);

	if (caption != NULL && *caption != '\0') {
		GUI_DrawText_Wrapper(caption, 128, 23, 12, 0, 0x12);
	} else {
		FactoryWindowItem *item = GUI_FactoryWindow_GetItem(g_global->factoryWindowSelected);
		ObjectInfo *oi = (ObjectInfo *)emu_get_memorycsip(item->objectInfo);
		uint16 width;

		GUI_DrawText_Wrapper(String_Get_ByIndex(oi->stringID_full), 128, 23, 12, 0, 0x12);

		/* "Cost: 999" */
		width = Font_GetStringWidth(String_Get_ByIndex(0xB2));

		/* "Cost: %3d" */
		GUI_DrawText_Wrapper(String_Get_ByIndex(0xB1), 310 - width, 23, 12, 0, 0x12, item->credits);

		if (g_global->factoryWindowStarport != 0) {
			/* "Qty: 99" */
			width += Font_GetStringWidth(String_Get_ByIndex(0xB4)) + 2;

			/* "Qty: %2d" */
			GUI_DrawText_Wrapper(String_Get_ByIndex(0xB3), 310 - width, 23, 12, 0, 0x12, item->amount);
		}
	}

	GUI_Mouse_Hide_Safe();
	if (oldScreenID == 0) GFX_Screen_Copy2(128, 21, 128, 21, 182, 14, 3, oldScreenID, false);
	GUI_Mouse_Show_Safe();

	GUI_Screen_SetActive(oldScreenID);
}

void GUI_FactoryWindow_UpdateDetails()
{
	FactoryWindowItem *item = GUI_FactoryWindow_GetItem(g_global->factoryWindowSelected);
	ObjectInfo *oi = (ObjectInfo *)emu_get_memorycsip(item->objectInfo);

	if (oi->available == -1) return;

	GUI_Mouse_Hide_Safe();
	GUI_Screen_Copy(16, (oi->available == item->amount) ? 169 : 160, 16, 99, 23, 9, 2, g_global->screenActiveID);
	GUI_Mouse_Show_Safe();
}

void GUI_FactoryWindow_UpdateSelection(bool selectionChanged)
{
	uint8 *palette = emu_get_memorycsip(g_global->variable_3C32);

	if (selectionChanged) {
		uint16 y;

		memset(palette + 255 * 3, 0x3F, 3);

		GFX_SetPalette(palette);

		g_global->variable_7F9C = 0;
		g_global->variable_7FA0 = 0;
		g_global->variable_7FA1 = 8;

		y = g_global->factoryWindowSelected * 32 + 24;

		GUI_Mouse_Hide_Safe();
		GUI_DrawWiredRectangle(71, y - 1, 104, y + 24, 255);
		GUI_DrawWiredRectangle(72, y, 103, y + 23, 255);
		GUI_Mouse_Show_Safe();
	} else {
		if (g_global->variable_7F9C > g_global->variable_76AC) return;
	}

	g_global->variable_7F9C = g_global->variable_76AC + 3;
	g_global->variable_7FA0 += g_global->variable_7FA1;

	if ((int8)g_global->variable_7FA0 < 0 || g_global->variable_7FA0 > 63) {
		g_global->variable_7FA1 = -g_global->variable_7FA1;
		g_global->variable_7FA0 += g_global->variable_7FA1;
	}

	switch (g_global->playerHouseID) {
		case HOUSE_HARKONNEN:
			*(palette + 255 * 3 + 1) = g_global->variable_7FA0;
			*(palette + 255 * 3 + 2) = g_global->variable_7FA0;
			break;

		case HOUSE_ATREIDES:
			*(palette + 255 * 3 + 0) = g_global->variable_7FA0;
			*(palette + 255 * 3 + 1) = g_global->variable_7FA0;
			break;

		case HOUSE_ORDOS:
			*(palette + 255 * 3 + 0) = g_global->variable_7FA0;
			*(palette + 255 * 3 + 2) = g_global->variable_7FA0;
			break;

		default: break;
	}

	GFX_SetPalette(palette);
}

/**
 * Fade in parts of the screen from one screenbuffer to the other screenbuffer.
 * @param xSrc The X-position to start in the source screenbuffer divided by 8.
 * @param ySrc The Y-position to start in the source screenbuffer.
 * @param xDst The X-position to start in the destination screenbuffer divided by 8.
 * @param yDst The Y-position to start in the destination screenbuffer.
 * @param width The width of the screen to copy divided by 8.
 * @param height The height of the screen to copy.
 * @param screenSrc The ID of the source screen.
 * @param screenDst The ID of the destination screen.
 */
void GUI_Screen_FadeIn(uint16 xSrc, uint16 ySrc, uint16 xDst, uint16 yDst, uint16 width, uint16 height, uint16 screenSrc, uint16 screenDst)
{
	uint16 offsetsY[100];
	uint16 offsetsX[40];
	int x, y;

	if (screenDst == 0) {
		GUI_Mouse_Hide_InRegion(xDst << 3, yDst, (xDst + width) << 3, yDst + height);
	}

	height /= 2;

	for (x = 0; x < width;  x++) offsetsX[x] = x;
	for (y = 0; y < height; y++) offsetsY[y] = y;

	for (x = 0; x < width; x++) {
		uint16 index;
		uint16 temp;

		index = Tools_RandomRange(0, width - 1);

		temp = offsetsX[index];
		offsetsX[index] = offsetsX[x];
		offsetsX[x] = temp;
	}

	for (y = 0; y < height; y++) {
		uint16 index;
		uint16 temp;

		index = Tools_RandomRange(0, height - 1);

		temp = offsetsY[index];
		offsetsY[index] = offsetsY[y];
		offsetsY[y] = temp;
	}

	for (y = 0; y < height; y++) {
		uint16 y2 = y;
		for (x = 0; x < width; x++) {
			uint16 offsetX, offsetY;

			offsetX = offsetsX[x];
			offsetY = offsetsY[y2];

			GUI_Screen_Copy(xSrc + offsetX, ySrc + offsetY * 2, xDst + offsetX, yDst + offsetY * 2, 1, 2, screenSrc, screenDst);

			y2++;
			if (y2 == height) y2 = 0;

			/* XXX -- This delays the system so you can in fact see the animation */
			if ((x % 10) != 0) continue;
			do { emu_inb(&emu_al, 0x3DA); } while ((emu_al & 0x8) == 0);
			do { emu_inb(&emu_al, 0x3DA); } while ((emu_al & 0x8) != 0);
		}
	}

	if (screenDst == 0) {
		GUI_Mouse_Show_InRegion();
	}
}

/**
 * Change the current active screen to the new value.
 * @param screenID The new screen to get active.
 * @return Old screenID that was currently active.
 */
uint16 GUI_Screen_SetActive(uint16 screenID)
{
	uint16 oldScreen = g_global->screenActiveID;
	g_global->screenActiveID = screenID;
	return oldScreen;
}

void GUI_FactoryWindow_PrepareScrollList()
{
	FactoryWindowItem *item;

	GUI_Mouse_Hide_Safe();
	GUI_Screen_Copy(9, 24, 9, 40, 4, 128, 0, 2);
	GUI_Mouse_Show_Safe();

	item = GUI_FactoryWindow_GetItem(-1);

	if (item != NULL) {
		ObjectInfo *oi = (ObjectInfo *)emu_get_memorycsip(item->objectInfo);

		if (oi->available == -1) {
			GUI_DrawSprite(2, g_sprites[oi->spriteID], 72, 8, 0, 0x100, emu_get_memorycsip(g_global->factoryWindowGraymapTbl), 1);
		} else {
			GUI_DrawSprite(2, g_sprites[oi->spriteID], 72, 8, 0, 0);
		}
	} else {
		GUI_Screen_Copy(9, 32, 9, 24, 4, 8, 2, 2);
	}

	item = GUI_FactoryWindow_GetItem(4);

	if (item != NULL) {
		ObjectInfo *oi = (ObjectInfo *)emu_get_memorycsip(item->objectInfo);

		if (oi->available == -1) {
			GUI_DrawSprite(2, g_sprites[oi->spriteID], 72, 168, 0, 0x100, emu_get_memorycsip(g_global->factoryWindowGraymapTbl), 1);
		} else {
			GUI_DrawSprite(2, g_sprites[oi->spriteID], 72, 168, 0, 0);
		}
	} else {
		GUI_Screen_Copy(9, 0, 9, 168, 4, 8, 2, 2);
	}
}

/**
 * Fade in parts of the screen from one screenbuffer to the other screenbuffer.
 * @param x The X-position in the source and destination screenbuffers.
 * @param y The Y-position in the source and destination screenbuffer.
 * @param width The width of the screen to copy.
 * @param height The height of the screen to copy.
 * @param screenSrc The ID of the source screen.
 * @param screenDst The ID of the destination screen.
 * @param delay The delay.
 * @param skipNull Wether to copy pixels with colour 0.
 */
void GUI_Screen_FadeIn2(int16 x, int16 y, int16 width, int16 height, uint16 screenSrc, uint16 screenDst, uint16 delay, bool skipNull)
{
	uint16 oldScreenID;
	uint16 i;
	uint16 j;

	if (screenDst == 0) {
		GUI_Mouse_Hide_InRegion(x, y, x + width, y + height);
	}

	for (i = 0; i < width; i++)  g_global->variable_7B8C[i] = i;
	for (i = 0; i < height; i++) g_global->variable_7E0C[i] = i;

	for (i = 0; i < width; i++) {
		uint16 tmp;

		j = Tools_RandomRange(0, width - 1);

		tmp = g_global->variable_7B8C[j];
		g_global->variable_7B8C[j] = g_global->variable_7B8C[i];
		g_global->variable_7B8C[i] = tmp;
	}

	for (i = 0; i < height; i++) {
		uint16 tmp;

		j = Tools_RandomRange(0, height - 1);

		tmp = g_global->variable_7E0C[j];
		g_global->variable_7E0C[j] = g_global->variable_7E0C[i];
		g_global->variable_7E0C[i] = tmp;
	}

	oldScreenID = GUI_Screen_SetActive(screenDst);

	for (j = 0; j < height; j++) {
		uint16 j2 = j;
		uint32 tick;

		for (i = 0; i < width; i++) {
			uint8 colour;
			uint16 curX = x + g_global->variable_7B8C[i];
			uint16 curY = y + g_global->variable_7E0C[j2];

			if (++j2 >= height) j2 = 0;

			GUI_Screen_SetActive(screenSrc);

			colour = GFX_GetPixel(curX, curY);

			GUI_Screen_SetActive(screenDst);

			if (skipNull && colour == 0) continue;

			GFX_PutPixel(curX, curY, colour);
		}

		tick = g_global->variable_76A8 + delay;

		while (g_global->variable_76A8 < tick) sleep(0);
	}

	if (screenDst == 0) {
		GUI_Mouse_Show_InRegion();
	}

	GUI_Screen_SetActive(oldScreenID);
}

/**
 * Show the mouse on the screen. Copy the screen behind the mouse in a safe
 *  buffer.
 */
void GUI_Mouse_Show()
{
	int left, top;

	if (g_global->variable_7097) return;
	if (g_global->mouseHiddenDepth == 0 || --g_global->mouseHiddenDepth != 0) return;

	left = g_global->mouseX - g_global->mouseSpriteHotspotX;
	top = g_global->mouseY - g_global->mouseSpriteHotspotY;

	g_global->mouseSpriteLeft = (left < 0) ? 0 : (left >> 3);
	g_global->mouseSpriteTop = (top < 0) ? 0 : top;

	g_global->mouseSpriteWidth = g_global->mouseWidth;
	if ((left >> 3) + g_global->mouseWidth >= SCREEN_WIDTH / 8) g_global->mouseSpriteWidth -= (left >> 3) + g_global->mouseWidth - SCREEN_WIDTH / 8;

	g_global->mouseSpriteHeight = g_global->mouseHeight;
	if (top + g_global->mouseHeight >= SCREEN_HEIGHT) g_global->mouseSpriteHeight -= top + g_global->mouseHeight - SCREEN_HEIGHT;

	if (g_global->mouseSpriteBuffer.csip != 0) {
		GFX_CopyToBuffer(g_global->mouseSpriteLeft, g_global->mouseSpriteTop, g_global->mouseSpriteWidth, g_global->mouseSpriteHeight, emu_get_memorycsip(g_global->mouseSpriteBuffer));
	}

	GUI_DrawSprite(0, g_global->mouseSprite, left, top, 0, 0);
}

/**
 * Hide the mouse from the screen. Do this by copying the mouse buffer back to
 *  the screen.
 */
void GUI_Mouse_Hide()
{
	if (g_global->variable_7097) return;

	if (g_global->mouseHiddenDepth == 0 && g_global->mouseSpriteWidth != 0) {
		if (g_global->mouseSpriteBuffer.csip != 0) {
			GFX_CopyFromBuffer(g_global->mouseSpriteLeft, g_global->mouseSpriteTop, g_global->mouseSpriteWidth, g_global->mouseSpriteHeight, emu_get_memorycsip(g_global->mouseSpriteBuffer));
		}

		g_global->mouseSpriteWidth = 0;
	}

	g_global->mouseHiddenDepth++;
}

/**
 * The safe version of GUI_Mouse_Hide(). It waits for a mouselock before doing
 *  anything.
 */
void GUI_Mouse_Hide_Safe()
{
	while (g_global->mouseLock != 0) sleep(0);
	g_global->mouseLock++;

	if (g_global->variable_7097) {
		g_global->mouseLock--;
		return;
	}

	GUI_Mouse_Hide();

	g_global->mouseLock--;
}

/**
 * The safe version of GUI_Mouse_Show(). It waits for a mouselock before doing
 *  anything.
 */
void GUI_Mouse_Show_Safe()
{
	while (g_global->mouseLock != 0) sleep(0);
	g_global->mouseLock++;

	if (g_global->variable_7097) {
		g_global->mouseLock--;
		return;
	}

	GUI_Mouse_Show();

	g_global->mouseLock--;
}

/**
 * Show the mouse if needed. Should be used in combination with
 *  GUI_Mouse_Hide_InRegion().
 */
void GUI_Mouse_Show_InRegion()
{
	uint8 counter;

	while (g_global->mouseLock != 0) sleep(0);
	g_global->mouseLock++;

	counter = g_global->regionFlags & 0xFF;
	if (counter == 0 || --counter != 0) {
		g_global->regionFlags = (g_global->regionFlags & 0xFF00) | (counter & 0xFF);
		g_global->mouseLock--;
		return;
	}

	if ((g_global->regionFlags & 0x4000) != 0) {
		GUI_Mouse_Show();
	}

	g_global->regionFlags = 0;
	g_global->mouseLock--;
}

/**
 * Hide the mouse when it is inside the specified region. Works with
 *  GUI_Mouse_Show_InRegion(), which only calls GUI_Mouse_Show() when
 *  mouse was really hidden.
 */
void GUI_Mouse_Hide_InRegion(uint16 left, uint16 top, uint16 right, uint16 bottom)
{
	int minx, miny;
	int maxx, maxy;

	minx = left - ((g_global->mouseWidth - 1) << 3) + g_global->mouseSpriteHotspotX;
	if (minx < 0) minx = 0;

	miny = top - g_global->mouseHeight + g_global->mouseSpriteHotspotY;
	if (miny < 0) miny = 0;

	maxx = right + g_global->mouseSpriteHotspotX;
	if (maxx > SCREEN_WIDTH - 1) maxx = SCREEN_WIDTH - 1;

	maxy = bottom + g_global->mouseSpriteHotspotY;
	if (maxy > SCREEN_HEIGHT - 1) maxy = SCREEN_HEIGHT - 1;

	while (g_global->mouseLock != 0) sleep(0);
	g_global->mouseLock++;

	if (g_global->regionFlags == 0) {
		g_global->regionMinX = minx;
		g_global->regionMinY = miny;
		g_global->regionMaxX = maxx;
		g_global->regionMaxY = maxy;
	}

	if (minx > g_global->regionMinX) g_global->regionMinX = minx;
	if (miny > g_global->regionMinY) g_global->regionMinY = miny;
	if (maxx < g_global->regionMaxX) g_global->regionMaxX = maxx;
	if (maxy < g_global->regionMaxY) g_global->regionMaxY = maxy;

	if ((g_global->regionFlags & 0x4000) == 0 &&
	     g_global->mouseX >= g_global->regionMinX &&
	     g_global->mouseX <= g_global->regionMaxX &&
	     g_global->mouseY >= g_global->regionMinY &&
	     g_global->mouseY <= g_global->regionMaxY) {
		GUI_Mouse_Hide();

		g_global->regionFlags |= 0x4000;
	}

	g_global->regionFlags |= 0x8000;
	g_global->regionFlags = (g_global->regionFlags & 0xFF00) | (((g_global->regionFlags & 0x00FF) + 1) & 0xFF);

	g_global->mouseLock--;
}

/**
 * Show the mouse if needed. Should be used in combination with
 *  GUI_Mouse_Hide_InWidget().
 */
void GUI_Mouse_Show_InWidget()
{
	GUI_Mouse_Show_InRegion();
}

/**
 * Hide the mouse when it is inside the specified widget. Works with
 *  GUI_Mouse_Show_InWidget(), which only calls GUI_Mouse_Show() when
 *  mouse was really hidden.
 * @param widgetIndex The index of the widget to check on.
 */
void GUI_Mouse_Hide_InWidget(uint16 widgetIndex)
{
	uint16 left, top;
	uint16 width, height;

	left   = g_global->variable_4062[widgetIndex][0] << 3;
	top    = g_global->variable_4062[widgetIndex][1];
	width  = g_global->variable_4062[widgetIndex][2] << 3;
	height = g_global->variable_4062[widgetIndex][3];

	GUI_Mouse_Hide_InRegion(left, top, left + width - 1, top + height - 1);
}

/**
 * Draws a chess-pattern filled rectangle.
 * @param left The X-position of the rectangle.
 * @param top The Y-position of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param colour The colour of the rectangle.
 */
void GUI_DrawBlockedRectangle(int16 left, int16 top, int16 width, int16 height, uint8 colour)
{
	uint8 *screen;

	if (g_global->variable_6C76 != 3) return;

	if (width <= 0) return;
	if (height <= 0) return;
	if (left >= SCREEN_WIDTH) return;
	if (top >= SCREEN_HEIGHT) return;

	if (left < 0) {
		if (left + width <= 0) return;
		width += left;
		left = 0;
	}
	if (top < 0) {
		if (top + height <= 0) return;
		height += top;
		top = 0;
	}

	if (left + width >= SCREEN_WIDTH) {
		width = SCREEN_WIDTH - left;
	}
	if (top + height >= SCREEN_HEIGHT) {
		height = SCREEN_HEIGHT - top;
	}

	screen = &emu_get_memory8(g_global->variable_6C93[g_global->screenActiveID >> 1][0], 0, 0);
	screen += top * SCREEN_WIDTH + left;
	for (; height > 0; height--) {
		int i = width;

		if ((height & 1) != (width & 1)) {
			screen++;
			i--;
		}

		for (; i > 0; i -= 2) {
			*screen = colour;
			screen += 2;
		}

		screen += SCREEN_WIDTH - width - (height & 1);
	}
}

/**
 * Set the mouse to the given position on the screen.
 *
 * @param x The new X-position of the mouse.
 * @param y The new Y-position of the mouse.
 */
void GUI_Mouse_SetPosition(uint16 x, uint16 y)
{
	emu_cx = x;
	emu_dx = y;

	while (g_global->mouseLock != 0) sleep(0);
	g_global->mouseLock++;

	if (x < g_global->mouseRegionLeft)   x = g_global->mouseRegionLeft;
	if (x > g_global->mouseRegionRight)  x = g_global->mouseRegionRight;
	if (y < g_global->mouseRegionTop)    y = g_global->mouseRegionTop;
	if (y > g_global->mouseRegionBottom) y = g_global->mouseRegionBottom;

	g_global->mouseX = x;
	g_global->mouseY = y;

	if (g_global->mouseInstalled) {
		if (g_global->doubleWidth) x *= 2;

		emu_ax = 4;
		emu_pushf(); emu_flags.inf = 0; emu_push(emu_cs); emu_cs = 0x0070; emu_push(0x004B); Interrupt_Mouse();
	}

	if (g_global->mouseX == g_global->mousePrevX && g_global->mouseY == g_global->mousePrevY) {
		g_global->mouseLock--;
		return;
	}

	GUI_Mouse_Hide();
	GUI_Mouse_Show();

	g_global->mouseLock--;
}

/**
 * Remap all the colours in the region with the ones indicated by the remap palette.
 * @param left The left of the region to remap.
 * @param top The top of the region to remap.
 * @param width The width of the region to remap.
 * @param height The height of the region to remap.
 * @param screenID The screen to do the remapping on.
 * @param remapcsip The pointer to the remap palette.
 */
void GUI_Palette_RemapScreen(uint16 left, uint16 top, uint16 width, uint16 height, uint16 screenID, csip32 remapcsip)
{
	uint8 *screen = &emu_get_memory8(g_global->variable_6C93[screenID >> 1][1], 0, 0);
	uint8 *remap = emu_get_memorycsip(remapcsip);

	screen += top * SCREEN_WIDTH + left;
	for (; height > 0; height--) {
		int i;
		for (i = width; i > 0; i--) {
			uint8 pixel = *screen;
			*screen++ = remap[pixel];
		}
		screen += SCREEN_WIDTH - width;
	}
}

uint16 GUI_HallOfFame_Tick()
{
	uint8 *var81ED = emu_get_memorycsip(g_global->variable_81ED);

	if (g_global->variable_2C3A >= g_global->variable_76AC) return 0;

	g_global->variable_2C3A = g_global->variable_76AC + 2;

	if (*var81ED >= 63) {
		g_global->variable_2C38 = 0xFFFF;
	} else if (*var81ED <= 35) {
		g_global->variable_2C38 = 1;
	}

	*var81ED += (int16)g_global->variable_2C38;

	GFX_SetPalette(emu_get_memorycsip(g_global->variable_3C32));

	return 0;
}

static Widget *GUI_HallOfFame_CreateButtons(HallOfFameData *data)
{
	char *resumeString;
	char *clearString;
	Widget *wClear;
	Widget *wResume;
	uint16 width;

	memcpy(g_global->variable_81F1, g_global->colourBorderSchema, 40);
	memcpy(g_global->colourBorderSchema, g_global->variable_2C10, 40);

	resumeString = String_Get_ByIndex(0x146); /* "Resume Game" */
	clearString  = String_Get_ByIndex(0x147); /* "Clear List" */

	width = max(Font_GetStringWidth(resumeString), Font_GetStringWidth(clearString)) + 6;

	/* "Clear List" */
	wClear = GUI_Widget_Allocate(100, *clearString, 160 - width - 18, 180, 0xFFFE, 0x147, 0, NULL);
	wClear->width          = width;
	wClear->height         = 10;
	wClear->clickProc.csip = 0x35180034;
	wClear->flags.all      = 0x44C5;
	wClear->scrollbar      = emu_Global_GetCSIP(data);

	/* "Resume Game" */
	wResume = GUI_Widget_Allocate(101, *resumeString, 178, 180, 0xFFFE, 0x146, 0, NULL);
	wResume->width          = width;
	wResume->height         = 10;
	wResume->clickProc.csip = 0x35180039;
	wResume->flags.all      = 0x44C5;
	wResume->scrollbar      = emu_Global_GetCSIP(data);

	return GUI_Widget_Insert(wClear, wResume);
}

static void GUI_HallOfFame_DeleteButtons(Widget *w)
{
	while (w != NULL) {
		csip32 wcsip = emu_Global_GetCSIP(w);
		w = GUI_Widget_GetNext(w);

		emu_push(wcsip.s.cs); emu_push(wcsip.s.ip);
		emu_push(emu_cs); emu_push(0x0A6D); emu_cs = 0x23E1; emu_Tools_Free();
		emu_sp += 4;
	}

	memcpy(g_global->colourBorderSchema, g_global->variable_81F1, 40);
}

static void GUI_HallOfFame_Encode(HallOfFameData *data)
{
	uint8 i;
	uint8 *d;

	for (d = (uint8 *)data, i = 0; i < 128; i++, d++) *d = (*d + i) ^ 0xA7;
}

static void GUI_HallOfFame_Decode(HallOfFameData *data)
{
	uint8 i;
	uint8 *d;

	for (d = (uint8 *)data, i = 0; i < 128; i++, d++) *d = (*d ^ 0xA7) - i;
}

static uint16 GUI_HallOfFame_InsertScore(HallOfFameData *data, uint16 score)
{
	uint16 i;
	for (i = 0; i < 8; i++, data++) {
		if (data->score >= score) continue;

		memmove(data + 1, data, 128);
		memset(data->name, 0, 6);
		data->score = score;
		data->houseID = g_global->playerHouseID;
		data->rank = GUI_HallOfFame_GetRank(score);
		data->campaignID = g_global->campaignID;
		data->variable_E = 0;

		return i + 1;
	}

	return 0;
}

void GUI_HallOfFame_Show(uint16 score)
{
	uint16 width;
	csip32 data_csip;
	uint16 editLine;
	Widget *w;
	uint8 fileID;
	HallOfFameData *data;

	GUI_Mouse_Hide_Safe();

	if (score == 0xFFFF) {
		if (!File_Exists("SAVEFAME.DAT")) {
			GUI_Mouse_Show_Safe();
			return;
		}
		g_global->variable_81EB = 0;
	}

	data_csip = Screen_GetSegment_ByIndex_1(5);
	data = (HallOfFameData *)emu_get_memorycsip(data_csip);

	if (!File_Exists("SAVEFAME.DAT")) {
		uint16 written;

		memset(data, 0, 128);

		GUI_HallOfFame_Encode(data);

		fileID = File_Open("SAVEFAME.DAT", 2);
		written = File_Write(fileID, data, 128);
		File_Close(fileID);

		if (written != 128) return;
	}

	File_ReadBlockFile("SAVEFAME.DAT", data, 128);

	GUI_HallOfFame_Decode(data);

	GUI_HallOfFame_DrawBackground(score, true);

	if (score == 0xFFFF) {
		editLine = 0;
	} else {
		editLine = GUI_HallOfFame_InsertScore(data, score);
	}

	width = GUI_HallOfFame_DrawData(data, false);

	GUI_Screen_Copy(0, 0, 0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT, 2, 0);

	if (editLine != 0) {
		uint16 backup_4062[16];
		char *name;

		name = data[editLine - 1].name;

		memcpy(backup_4062, g_global->variable_4062[19], 16);

		g_global->variable_4062[19][0] = 4;
		g_global->variable_4062[19][1] = (editLine - 1) * 11 + 90;
		g_global->variable_4062[19][2] = width / 8;
		g_global->variable_4062[19][3] = 11;
		g_global->variable_4062[19][4] = 6;
		g_global->variable_4062[19][5] = 116;

		GUI_DrawText_Wrapper(NULL, 0, 0, 0, 0, 0x22);

		while (*name == '\0') {
			char *nameEnd;
			uint16 oldScreenID;

			oldScreenID = GUI_Screen_SetActive(0);
			Unknown_07AE_00E4(19);
			GUI_Screen_SetActive(oldScreenID);

			{
				csip32 null;
				csip32 tick;
				null.csip = 0x0;
				tick.csip = 0x35180066;
				GUI_EditBox(emu_Global_GetCSIP(name), 5, 19, null, tick, 0);
			}

			if (*name == '\0') continue;

			nameEnd = name + strlen(name) - 1;

			while (*nameEnd <= ' ' && nameEnd >= name) *nameEnd-- = '\0';
		}

		memcpy(g_global->variable_4062[19], backup_4062, 16);

		GUI_HallOfFame_DrawData(data, true);

		GUI_HallOfFame_Encode(data);

		fileID = File_Open("SAVEFAME.DAT", 2);
		File_Write(fileID, data, 128);
		File_Close(fileID);
	}

	GUI_Mouse_Show_Safe();

	w = GUI_HallOfFame_CreateButtons(data);

	emu_push(emu_cs); emu_push(0x0840); emu_cs = 0x29E8; emu_Input_History_Clear();

	GUI_Screen_SetActive(0);

	g_global->variable_81E6 = 0;

	while (g_global->variable_81E6 == 0) GUI_Widget_HandleEvents(w);

	GUI_HallOfFame_DeleteButtons(w);

	emu_push(emu_cs); emu_push(0x0875); emu_cs = 0x29E8; emu_Input_History_Clear();

	if (score == 0xFFFF) return;

	memcpy(emu_get_memorycsip(g_global->variable_3C32) + 255 * 3, g_global->variable_81E8, 3);
}

uint16 GUI_HallOfFame_DrawData(HallOfFameData *data, bool show)
{
	uint16 oldScreenID;
	char *scoreString;
	char *battleString;
	uint16 width = 0;
	uint16 offsetY;
	uint16 scoreX;
	uint16 battleX;
	uint8 i;

	oldScreenID = GUI_Screen_SetActive(2);
	GUI_DrawFilledRectangle(8, 80, 311, 178, 116);
	GUI_DrawText_Wrapper(NULL, 0, 0, 0, 0, 0x22);

	battleString = String_Get_ByIndex(0x144); /* "Battle" */
	scoreString = String_Get_ByIndex(0x145); /* "Score" */

	scoreX = 320 - Font_GetStringWidth(scoreString) / 2 - 12;
	battleX = scoreX - Font_GetStringWidth(scoreString) / 2 - 8 - Font_GetStringWidth(battleString) / 2;
	offsetY = 80;

	/* "Name and Rank" */
	GUI_DrawText_Wrapper(String_Get_ByIndex(0x143), 32, offsetY, 8, 0, 0x22);

	/* "Battle" */
	GUI_DrawText_Wrapper(battleString, battleX, offsetY, 8, 0, 0x122);

	/* "Score" */
	GUI_DrawText_Wrapper(scoreString, scoreX, offsetY, 8, 0, 0x122);

	offsetY = 90;
	for (i = 0; i < 8; i++, offsetY += 11) {
		if (data[i].score == 0) break;

		strcpy((char *)g_global->variable_9939, data[i].name);
		strcat((char *)g_global->variable_9939, ", ");
		if (g_global->language == LANGUAGE_FRENCH) {
			strcat((char *)g_global->variable_9939, String_Get_ByIndex(g_global->variable_37C0[data[i].rank][0]));
			strcat((char *)g_global->variable_9939, " ");
			strcat((char *)g_global->variable_9939, (char *)emu_get_memorycsip(g_houseInfo[data[i].houseID].name));
		} else {
			strcat((char *)g_global->variable_9939, (char *)emu_get_memorycsip(g_houseInfo[data[i].houseID].name));
			strcat((char *)g_global->variable_9939, " ");
			strcat((char *)g_global->variable_9939, String_Get_ByIndex(g_global->variable_37C0[data[i].rank][0]));
		}

		if (*data[i].name == '\0') {
			width = battleX - 36 - Font_GetStringWidth((char *)g_global->variable_9939);
		} else {
			GUI_DrawText_Wrapper((char *)g_global->variable_9939, 32, offsetY, 15, 0, 0x22);
		}

		GUI_DrawText_Wrapper("%u.", 24, offsetY, 15, 0, 0x222, i + 1);
		GUI_DrawText_Wrapper("%u", battleX, offsetY, 15, 0, 0x122, data[i].campaignID);
		GUI_DrawText_Wrapper("%u", scoreX, offsetY, 15, 0, 0x122, data[i].score);
	}

	if (show) {
		GUI_Mouse_Hide_Safe();
		GUI_Screen_Copy(1, 80, 1, 80, 38, 100, 2, 0);
		GUI_Mouse_Show_Safe();
	}

	GUI_Screen_SetActive(oldScreenID);

	return width;
}

/**
 * Draw a filled rectangle using xor.
 * @param left The left position of the rectangle.
 * @param top The top position of the rectangle.
 * @param right The right position of the rectangle.
 * @param bottom The bottom position of the rectangle.
 * @param colour The colour of the rectangle.
 */
void GUI_DrawXorFilledRectangle(int16 left, int16 top, int16 right, int16 bottom, uint8 colour)
{
	uint16 x;
	uint16 y;
	uint16 height;
	uint16 width;

	uint8 *screen = &emu_get_memory8(GFX_Screen_GetSegmentActive(), 0x0, 0x0);

	if (left >= SCREEN_WIDTH) return;
	if (left < 0) left = 0;

	if (top >= SCREEN_HEIGHT) return;
	if (top < 0) top = 0;

	if (right >= SCREEN_WIDTH) right = SCREEN_WIDTH - 1;
	if (right < 0) right = 0;

	if (bottom >= SCREEN_HEIGHT) bottom = SCREEN_HEIGHT - 1;
	if (bottom < 0) bottom = 0;

	if (left > right) return;
	if (top > bottom) return;

	screen += left + top * SCREEN_WIDTH;
	width = right - left + 1;
	height = bottom - top + 1;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*screen++ ^= colour;
		}
		screen += SCREEN_WIDTH - width;
	}
}
