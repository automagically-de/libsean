#include <stdlib.h>
#include <math.h>

#include <glib.h>

#define WIDTH_THR 0.45

static guint16 bar_digits[] = {
	3211, 2221, 2122, 1411, 1132,
	1231, 1114, 1312, 1213, 3112
};

static guint16 bar_digit13[] = {
	000000, 001011, 001101, 001110, 010011,
	011001, 011100, 010101, 010110, 011010
};

guint32 bar_get_digit(gint32 *digit_code, gboolean *is_even)
{
	guint32 i, even = 0, uneven = 0, me = 1, mu = 1000;

	for(i = 0; i < 4; i ++)
	{
		even += digit_code[i] * me;
		uneven += digit_code[i] * mu;
		me *= 10;
		mu /= 10;
	}

	for(i = 0; i < 10; i ++)
	{
		if(even == bar_digits[i])
		{
			*is_even = TRUE;
			return i;
		}
		if(uneven == bar_digits[i])
		{
			*is_even = FALSE;
			return i;
		}
	}

	return 0xFF;
}

guint32 bar_get_digit13(gboolean *even)
{
	gint32 i, sum = 0, m = 1;

	for(i = 5; i >= 0; i --)
	{
		sum += (even[i] ? m : 0);
		m *= 8;
	}

	for(i = 0; i < 10; i ++)
		if(bar_digit13[i] == sum)
			return i;

	return 0xFF;
}

gboolean bar_check_ean13_crc(guint32 *code)
{
	guint32 sum = 0, i, crc;

	for(i = 0; i < 12; i ++)
		sum += code[i] * ((i%2) ? 3 : 1);
	crc = 10 - sum % 10;
	crc %= 10;

#if DEBUG > 1
	if(crc != code[12])
		g_debug("CRC check failed: %d != %d\n", crc, code[12]);
#endif

	return (crc == code[12]);
}

gboolean bar_is_start(guint32 *bars, gfloat *basewidth)
{
	gfloat avg;
	guint8 i;

	avg = (gfloat)(bars[0] + bars[1] + bars[2]) / 3.0;
	if(avg < 3.0)
		return FALSE;

	for(i = 0; i < 3; i ++)
		if(abs((gfloat)bars[i] - avg) > (WIDTH_THR * avg))
			return FALSE;

	*basewidth = avg;
	return TRUE;
}

gboolean bar_check_divider(guint8 *pixels, guint32 width, guint32 *offset,
	gfloat *basewidth)
{
	guint32 i;
	guint32 bars[5], off, saveoff;
	guint8 col = 0xFF;
	gfloat avg;

	/* get 5 bars (white,black,white,black,white) */
	off = *offset;
	for(i = 0; i < 5; i ++)
	{
		saveoff = off;
		while(pixels[off * 3] == col)
		{
			if(off == (width - 1))
				return FALSE;
			off ++;
		}
		col = col ? 0x00 : 0xFF;
		bars[i] = off - saveoff;
	}
#if 0
	g_debug("divider: %u %u %u %u %u",
		bars[0], bars[1], bars[2], bars[3], bars[4]);
#endif

	/* check bar widths */
	avg = (gfloat)(bars[0] + bars[1] + bars[2] + bars[3] + bars[4]) / 5.0;
	if(avg < 2.0)
		return FALSE;

	for(i = 0; i < 5; i ++)
		if(abs((gfloat)bars[i] - avg) > (WIDTH_THR * avg))
			return FALSE;

	/* adjust base width */
	*basewidth = avg;
	*offset = off;
	return TRUE;
}

gboolean bar_find_start(guint8 *pixels, guint32 width, guint32 offset,
	guint32 *start, gfloat *basewidth)
{
	static guint32 bars[3];
	guint32 off, saveoff, nbar;
	guint8 col = 0x00;

	/* skip leading white space */
	off = offset;
	while(pixels[off * 3] == 0xFF)
	{
		if(off >= (width - 1))
			return FALSE;
		off ++;
	}

	/* find first two bars (black, white, black) */
	for(nbar = 0; nbar < 3; nbar ++)
	{
		saveoff = off;
		while(pixels[off * 3] == col)
		{
			if(off >= (width - 1))
				return FALSE;
			off ++;
		}
		col = col ? 0x00 : 0xFF;
		bars[nbar] = off - saveoff;
	}

	if(bar_is_start(bars, basewidth))
	{
		*start = off;
		/* g_debug("bar start @ %d (%.2f pixels)", off, *basewidth); */
		return TRUE;
	}

	return bar_find_start(pixels, width, saveoff, start, basewidth);
}

gboolean bar_read_half_code(guint8 *pixels, guint32 width, guint32 *offset,
	gfloat basewidth, guint32 *code, gboolean second)
{
	guint32 i, j, off, saveoff, ci;
	guint8 col;
	gint32 digit[4];
	gfloat val;
	gboolean even[6];

	off = *offset;

	col = second ? 0x00 : 0xFF;

	/* read 6 digits */
	for(i = 0; i < 6; i++)
	{
		/* read 4 bars */
		for(j = 0; j < 4; j ++)
		{
			saveoff = off;
			while(pixels[off * 3] == col)
			{
				if(off == (width - 1))
					return FALSE;
				off ++;
			}
			col = col ? 0x00 : 0xFF;
			val = (gfloat)(off - saveoff) / basewidth;
			digit[j] = val + 0.4999;
			if((digit[j] > 4) || (digit[j] < 1))
				return FALSE;
		}
		if((digit[0] + digit[1] + digit[2] + digit[3]) != 7)
			return FALSE;

		ci = second ? i + 7 : i + 1;
		code[ci] = bar_get_digit(digit, even + i);

#if 0
		if(second)
			g_debug("digit: %u%u%u%u (%u) %s",
				digit[0], digit[1], digit[2], digit[3],
				code[ci], even[i] ? "even" : "uneven");
#endif
	}

	if(!second)
		code[0] = bar_get_digit13(even);

	*offset = off;

	return TRUE;
}

gboolean bar_read_code(guint8 *pixels, guint32 width, guint32 *code)
{
	gfloat basewidth;
	guint32 offset;

	if(!bar_find_start(pixels, width, 0, &offset, &basewidth))
		return FALSE;
	if(!bar_read_half_code(pixels, width, &offset, basewidth, code, FALSE))
		return FALSE;
	if(!bar_check_divider(pixels, width, &offset, &basewidth))
		return FALSE;
	if(!bar_read_half_code(pixels, width, &offset, basewidth, code, TRUE))
		return FALSE;
	if(code[0] > 9)
		return FALSE;
	if(!bar_check_ean13_crc(code))
		return FALSE;

	code[13] = '\0';

	return TRUE;
}

