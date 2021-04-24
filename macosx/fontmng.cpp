#include	"compiler.h"
#include	"fontmng.h"
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>

typedef struct _FNTMNG {
	int				fontsize;
	UINT			fonttype;
	int				fontwidth;
	int				fontheight;

	CGContextRef	gw;
	CGRect			rect;
	CGColorRef		bg;
	CGColorRef		fg;
	CTFontRef		fontid;
} _FNTMNG, *FNTMNG;


/**
 * Get charactor
 * @param[in,out] lppString Pointer
 * @return Charactor
 */
static UINT16 GetChar(const char** lppString)
{
	const char *lpString;
	UINT16 c;

	lpString = *lppString;
	if (lpString == NULL)
	{
		return 0;
	}

	c = 0;
	if ((lpString[0] & 0x80) == 0)
	{
		c = lpString[0] & 0x7f;
		lpString++;
	}
	else if (((lpString[0] & 0xe0) == 0xc0) && ((lpString[1] & 0xc0) == 0x80))
	{
		c = ((lpString[0] & 0x1f) << 6) | (lpString[1] & 0x3f);
		lpString += 2;
	}
	else if (((lpString[0] & 0xf0) == 0xe0) && ((lpString[1] & 0xc0) == 0x80) && ((lpString[2] & 0xc0) == 0x80))
	{
		c = ((lpString[0] & 0x0f) << 12) | ((lpString[1] & 0x3f) << 6) | (lpString[2] & 0x3f);
		lpString += 3;
	}

	*lppString = lpString;
	return c;
}


void *fontmng_create(int size, UINT type, const TCHAR *fontface) {

	void		*ret;
	_FNTMNG		fnt;
	int			fontalign;
	int			allocsize;

	ZeroMemory(&fnt, sizeof(fnt));

	if (size < 0) {
		size *= -1;
	}
	if (size < 6) {
		size = 6;
	}
	else if (size > 128) {
		size = 128;
	}
	fnt.fontsize = size;
	fnt.fonttype = type;
	fnt.fontwidth = size + 1;
	fnt.fontheight = size + 1;

	fontalign = sizeof(_FNTDAT) + (fnt.fontwidth * fnt.fontheight);

	allocsize = sizeof(fnt);
	allocsize += fontalign;
	CFStringRef fontName = CFSTR("Osaka-Mono");

	fnt.rect = CGRectMake(0, 0, fnt.fontwidth, fnt.fontheight);
	CGColorSpaceRef colorRef = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	
	fnt.gw = CGBitmapContextCreate(NULL, fnt.fontwidth, fnt.fontheight, 32, 4 * fnt.fontwidth, colorRef, kCGBitmapByteOrderDefault);
	CGColorSpaceRelease(colorRef);
	if (!fnt.gw) {
		return(NULL);
	}
	fnt.fg = (CGColorRef)CFRetain(CGColorGetConstantColor(kCGColorWhite));
	fnt.bg = (CGColorRef)CFRetain(CGColorGetConstantColor(kCGColorBlack));
	fnt.fontid = CTFontCreateWithName(fontName, size, NULL);
	ret = _MALLOC(allocsize, "font mng");
	if (ret) {
		CopyMemory(ret, &fnt, sizeof(fnt));
	} else {
		CGColorRelease(fnt.fg);
		CGColorRelease(fnt.bg);
		CGContextRelease(fnt.gw);
		CFRelease(fnt.fontid);
	}
	(void)fontface;
	return(ret);
}


void fontmng_destroy(void *hdl) {

	FNTMNG	fhdl;

	fhdl = (FNTMNG)hdl;
	if (fhdl) {
		CGColorRelease(fhdl->bg);
		CGColorRelease(fhdl->fg);
		CGContextRelease(fhdl->gw);
		CFRelease(fhdl->fontid);
		free(fhdl);
	}
}


/**
 * Sets font header (TTF)
 * @param[in] _this Instance
 * @param[out] fdat Data header
 * @param[in] s SDL_Surface
 */
static void TTFSetFontHeader(FNTMNG _this, FNTDAT fdat/*, const SDL_Surface *s*/)
{
	int width;
	int height;
	int pitch;

//	if (s)
//	{
//		width = MIN(s->w, _this->ptsize);
//		height = MIN(s->h, _this->ptsize);
//	}
//	else
//	{
		width = _this->fontsize;
		height = _this->fontsize;
//	}

	pitch = width;
	if (_this->fonttype & FDAT_ALIAS)
	{
		width = (width + 1) >> 1;
		pitch = width >> 1;
		height = (height + 1) >> 1;
	}
	fdat->width = width;
	fdat->pitch = pitch;
	fdat->height = height;
}

/**
 * Gets font length (TTF)
 * @param[in] _this Instance
 * @param[out] fdat Data
 * @param[in] c Charactor
 */
static void TTFGetLength1(FNTMNG _this, FNTDAT fdat, UINT16 c)
{
	UINT16 sString[2];

	sString[0] = c;
	sString[1] = 0;
	CFStringRef cfString = CFStringCreateWithCharacters(kCFAllocatorDefault, sString, 1);
//	s = NULL;
//	if (_this->ttf_font)
//	{
//		s = TTF_RenderUNICODE_Solid(_this->ttf_font, sString, s_white);
//	}
//	if (s)
//	{
//		TTFSetFontHeader(_this, fdat, s);
//		SDL_FreeSurface(s);
//	}
//	else
//	{
//		AnkGetLength1(_this, fdat, c);
//	}
	CFRelease(cfString);
}

BOOL fontmng_getsize(void *hdl, const char *lpString, POINT_T *pt)
{
	FNTMNG _this;
	int nWidth;
	UINT16 c;
	_FNTDAT fontData;

	_this = (FNTMNG)hdl;
	if ((_this == NULL) || (lpString == NULL))
	{
		return FAILURE;
	}

	nWidth = 0;
	while (1 /* EVER */)
	{
		c = GetChar(&lpString);
		if (c == 0)
		{
			break;
		}
		TTFGetLength1(_this, &fontData, c);
		nWidth += fontData.pitch;
	}

	if (pt)
	{
		pt->x = nWidth;
		pt->y = _this->fontsize;
	}
	return SUCCESS;
}


BOOL fontmng_getdrawsize(void *hdl, const char *lpString, POINT_T *pt)
{
	FNTMNG _this;
	int nWidth;
	int nPosX;
	UINT16 c;
	_FNTDAT fontData;

	_this = (FNTMNG)hdl;
	if (_this == NULL)
	{
		return FAILURE;
	}

	nWidth = 0;
	nPosX = 0;
	while (1 /* EVER */)
	{
		c = GetChar(&lpString);
		if (c == 0)
		{
			break;
		}
		TTFGetLength1(_this, &fontData, c);
		nWidth = nPosX + MAX(fontData.width, fontData.pitch);
		nPosX += fontData.pitch;
	}
	if (pt)
	{
		pt->x = nWidth;
		pt->y = _this->fontsize;
	}
	return SUCCESS;
}


static void fontmng_getchar(FNTMNG fhdl, FNTDAT fdat, const char *string) {

	GDHandle	hgd;
	GWorldPtr	gwp;
	BYTE		*p, *q;
	int			yalign;
	RGBColor	bak;
	int			x, y;
	FontInfo	info;
	int			leng;
	char		buf[4];
#if 0

	GetGWorld(&gwp, &hgd);
	LockPixels(fhdl->pm);
	SetGWorld(fhdl->gw, NULL);

	p = (BYTE *)GetPixBaseAddr(fhdl->pm);
	yalign = ((*fhdl->pm)->rowBytes) & 0x3fff;
	TextFont(fhdl->fontid);
	TextSize(fhdl->fontsize);
	GetFontInfo(&info);
	GetForeColor(&bak);
#if TARGET_API_MAC_CARBON
	// やり方解らないので 自前で削除します^^;
	{
		BYTE	*q;
		int		r;
		q = p;
		r = fhdl->fontheight;
		while(r--) {
			ZeroMemory(q, fhdl->fontwidth * 4);
			q += ((*fhdl->pm)->rowBytes) & 0x3fff;
		}
	}
#else
	FillRect(&fhdl->rect, &qd.black);
#endif

	leng = getsjis1(buf, string);
	if (leng) {
		RGBForeColor(&fhdl->fg);
#if TARGET_API_MAC_CARBON
		MoveTo(0, info.ascent - 1);
#else
		MoveTo(0, fhdl->fontsize - 1);
#endif
		DrawText(buf, 0, leng);
		getlength1(fhdl, fdat, buf, leng);
	}

	y = fdat->width;
	q = (BYTE *)(fdat + 1);
	while(y--) {
		BYTE *fnt;
		fnt = p;
		x = fdat->height;
		do {
			*q++ = fnt[1];
			fnt += 4;
		} while(--x);
		p += yalign;
	}
	RGBForeColor(&bak);
	UnlockPixels(fhdl->pm);
	SetGWorld(gwp, hgd);
#endif
}


FNTDAT fontmng_get(void *hdl, const char *string) {

	FNTMNG	fhdl;
	FNTDAT	fdat;

	if ((hdl == NULL) || (string == NULL)) {
		goto ftmggt_err;
	}
	fhdl = (FNTMNG)hdl;
	fdat = (FNTDAT)(fhdl + 1);
	fontmng_getchar(fhdl, fdat, string);
	return(fdat);

ftmggt_err:
	return(NULL);
}

