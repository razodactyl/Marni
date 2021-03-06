﻿#include "stdafx.h"
#include "MarniSurface.h"
#include "debug_new.h"
#include <assert.h>
#include "lodepng.h"

//////////////////////////////////////////
// SOFTWARE SURFACES					//
//////////////////////////////////////////
CMarniSurface2::CMarniSurface2()
{
	//memset(this, 0, sizeof(*this));
	ClassReset(this, pData);
}

CMarniSurface2::~CMarniSurface2()
{
	Release();
	//ClassReset(this, pData);
}

CMarniSurface2* CMarniSurface2::constructor()
{
	return this;
}

//////////////////////////////////////////
// virtual methods
int CMarniSurface2::ClearBg(int *adjust, int rgb, int use_image)
{
	if (!this->Is_open)
		return 0;

	CRect rect, dstrect;
	if (adjust)
	{
		SetRect(&rect, 0, 0, this->dwWidth - 1, this->dwHeight - 1);
		if (!Adjust_rect(&rect, adjust, &dstrect))
			return 0;
		dstrect.right++;
		dstrect.bottom++;
	}
	else SetRect(&dstrect, 0, 0, this->dwWidth, this->dwHeight);

	if (!this->Lock(NULL, NULL))
	{
		this->Is_open = 0;
		return 0;
	}

	if (rgb || use_image)
	{
		for (int y = 0, h = dstrect.Height(); y < h; y++)
			for (int x = 0, w = dstrect.Width(); x < w; x++)
				SetCurrentColor(x + dstrect.left, y + dstrect.top, rgb, use_image);
	}

	if (!this->Unlock())
		return 0;

	return 1;
}

int CMarniSurface2::Blt(RECT *dstrect, RECT *srcrect, CMarniSurface2 *src, int a5, float *a6)
{
	if (!this->Is_open || !src->Is_open)
		return 0;

	CRect rc0, rc1,
		trect, rect;
	if (srcrect)
	{
		// invalid area
		if (srcrect->top < 0 || srcrect->left < 0 || srcrect->right >= src->dwWidth || srcrect->bottom >= src->dwHeight)
			return 0;
		memcpy(&rc0, src, sizeof(rc0));
	}
	else SetRect(&rc0, 0, 0, src->dwWidth - 1, src->dwHeight - 1);

	if (dstrect)
	{
		memcpy(&rc1, dstrect, sizeof(rc1));
		SetRect(&trect, 0, 0, this->dwWidth - 1, this->dwHeight - 1);
		if (!Adjust_rect(&trect, (int*)&dstrect->left, &rect))
			return 0;
	}
	else
	{
		SetRect(&rect, 0, 0, this->dwWidth - 1, this->dwHeight - 1);
		rc1 = rect;
	}

	u8 *pp0, *pp1;
	if (!this->Lock(NULL, &pp0))
		return 0;
	if (!src->Lock(NULL, &pp1))
		return 0;

	int w = rect.Width() + 1,
		h = rect.Height() + 1;
	// used for stretched surfaces?
	float fw = (float)(rc0.Width() + 1) / (float)(rc1.Width() + 1),
		fh = (float)(rc0.Height() + 1) / (float)(rc1.Height() + 1);
	int ww = (rc0.Width()) * (rect.Width()) / (rc1.Width()),
		wh = (rc0.Height()) * (rect.Height()) / (rc1.Height());

	float fx = (float)rc0.left;
	int ix[3000];	// change this later to std::vector<int>
	for (int x = 0; x < w; x++)
		ix[x] = (int)((x + ww) * fw + fx);

	// determine type
	DWORD dwType = 0;
	if (!a5 && !a6)
	{
		dwType = (src->sdesc.dwRGBBitCount >> 3) |
			((u8)this->sdesc.bpp >> 3 << 16) | 2
			* ((this->sdesc.dwRGBBitCount | ((u8)(src->sdesc.bpp & 0xF8) << 16)) & ~7u);
		if (src->Has_palette)
			dwType |= 0x100;
		if (this->Has_palette)
			dwType |= 0x1000;
	}

	DWORD *p0, *p1;

	switch (dwType)
	{
	// paletted cases
	case 0x241111:
		break;
	case 0x221111:
		break;
	case 0x200140:
		break;
	case 0x200141:
		break;
	case 0x200121:
		break;
	// non paletted cases
	case 0x000044:
		break;
	case 0x000042:
		break;
	case 0x000022:
		break;
	case 0x000024:
		p0 = (DWORD*)this->CalcAddress(rect.left, rect.top);
		for (int y = 0; y < h; y++)
		{
			p1 = (DWORD*)src->CalcAddress(0, fh + (double)rc0.top);
			for (int x = 0; x < ix[wh]; x++)
			{

			}
		}
		break;
	// type mismatch
	default:
		Marni1Out("type mismatch, src: %d %d / dst:%d %d",
			src->sdesc.dwRGBBitCount,
			src->sdesc.bpp,
			this->sdesc.dwRGBBitCount,
			this->sdesc.bpp);
		if ((dwType & 0x11) == 0x11)
		{
			for (int i = 0, si = 1 << src->sdesc.dwRGBBitCount; i < si; ++i)
			{
				u32 p;
				src->GetPaletteColor(i, &p);
				this->SetPaletteColor(i, p, 0);
			}
		}
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				u32 p;
				if (this->Has_palette)
				{
					if (!src->GetColor(x, y, &p))
					{
						this->Unlock();
						src->Unlock();
						return 0;
					}
				}
				else
				{
					if (!src->GetCurrentColor(x, y, &p))
					{
						this->Unlock();
						src->Unlock();
						return 0;
					}
				}

				if (!(a5 & 1) || p & 0xffffff)
				{
					if (a6)
					{

					}

					int x0, y0;
					// mirrored
					if (a5 & BLTMODE_MIRROR) x0 = rect.left - x;
					else x0 = rect.left + x;
					// upside down
					if (a5 & BLTMODE_FLIP) y0 = rect.bottom - y;
					else y0 = rect.top + y;

					if (this->Has_palette)
						this->SetColor(x0, y0, p, a5);
					else this->SetCurrentColor(x0, y0, p, a5);
				}
			}
		}
		break;
	}

	this->Unlock();
	src->Unlock();

	return 1;
}

int CMarniSurface2::Null() { return 1; }
int CMarniSurface2::PalBlt(CMarniSurface2 *pSurfaceSrc, int reserved0, int colcnt, int reserved1)
{
	if (!this->Is_open)
		return 0;

	if (!this->Has_palette || !pSurfaceSrc->Has_palette)
		return 0;

	if (colcnt == 0)
	{
		int v8 = this->sdesc.dwRGBBitCount;
		int v9 = pSurfaceSrc->sdesc.dwRGBBitCount;
		if (v8 < v9)
			v9 = this->sdesc.dwRGBBitCount;
		colcnt = 1 << v9;
	}

	if (this->sdesc.dwRGBBitCount < colcnt)
		return 0;

	u32 color;
	for (int i = 0; i < colcnt; i++)
	{
		pSurfaceSrc->GetPaletteColor(i, &color);
		this->SetPaletteColor(i, color, 0);
	}

	return 1;
}

int CMarniSurface2::Lock(u8 **ppBitmap, u8 **ppPalette)
{
	if (this->Is_open)
	{
		// already locked
		if (this->field_C == 1)
			return 0;

		if (ppBitmap) *ppBitmap = this->pData;
		if (ppPalette) *ppPalette = this->pPalette;
		this->field_C = 1;	// lock
	}
	// can't open an unassigned surface
	else return 0;

	return 1;
}

int CMarniSurface2::Unlock()
{
	this->field_C = 0;
	return 1;
}

int CMarniSurface2::Release()
{
	if (this->field_C)
		Unlock();
	if (this->field_44 && this->field_4C)
	{
		delete[] this->pData;
		delete[] this->pPalette;
		this->pData = NULL;
		this->pPalette = NULL;
	}

	this->pPalette = 0;
	this->pData = 0;
	this->field_C = 0;
	memset(&this->sdesc, 0, sizeof(this->sdesc));
	this->sdesc.Pal_index = 0;
	this->sdesc.bpp = 0;
	this->sdesc.dwRGBBitCount = 0;
	this->lPitch = 0;
	this->dwHeight = 0;
	this->dwWidth = 0;
	this->field_50 = 0;
	this->Has_palette = 0;
	this->field_44 = 0;
	this->Is_open = 0;
	this->field_4C = 0;
	this->field_3C = 0;
	this->field_38 = 0;

	return 1;
}

//////////////////////////////////////////
// regular methods
int CMarniSurface2::WriteBitmap(LPCSTR lpFilename)
{
	if (this->Is_open)
	{
		u8 *pData = (u8*)calloc(this->dwWidth * this->dwHeight * 3, 1);

		CMarniSurface2 surf;
		surf.sdesc.dwRBitMask_setcnt = 16;
		surf.sdesc.field_4 = 8;
		surf.sdesc.dwBBitMask_setcnt = 0;
		surf.sdesc.field_2 = 255;
		surf.sdesc.dwRBitMask_cnt = 8;
		surf.sdesc.dwRGBBitCount = 24;
		surf.sdesc.bpp = 0;
		surf.sdesc.field_A = 8;
		surf.sdesc.field_8 = 255;
		surf.sdesc.field_10 = 8;
		surf.sdesc.field_E = 255;
		surf.dwWidth = this->dwWidth;
		surf.dwHeight = this->dwHeight;
		surf.lPitch = 3 * surf.dwWidth;
		surf.SetAddress(pData, NULL);
		surf.field_4C = 1;
		surf.Is_open = 1;
		surf.field_44 = 1;
		surf.Blt(NULL, NULL, this, 0/*BLTMODE_FLIP*/, 0);
		surf.field_44 = 0;

		// surfaces use BGR instead of RGB, convert bitmap
		u8 *p = pData;
		for (int y = 0; y < surf.dwHeight; y++)
		{
			for (int x = 0; x < surf.dwWidth; x++, p+=3)
			{
				u8 r = p[2], b = p[0];
				p[0] = r, p[2] = b;
			}
		}

		lodepng::encode(lpFilename, pData, surf.dwWidth, surf.dwHeight, LodePNGColorType::LCT_RGB);
		free(pData);
	}

	return 1;
}

int CMarniSurface2::SetAddress(u8* pData, u8 *pPalette)
{
	if (this->Is_open)
		return 0;
	if (this->field_44)
		return 0;

	this->pData = pData;
	this->pPalette = pPalette;
	this->field_44 = 1;

	return 1;
}

int CMarniSurface2::copy(CMarniSurface2 &srf)
{
	try
	{
		if (!srf.Is_open)
			throw 0;

		if (!this->field_44)
		{
			if (srf.field_4C)
			{
				this->pData = srf.pData;
				this->pPalette = srf.pPalette;
				srf.field_C = 0;
				memcpy(&this->sdesc, &srf.sdesc, sizeof(this->sdesc));
				this->dwWidth = srf.dwWidth;
				this->dwHeight = srf.dwHeight;
				this->lPitch = srf.lPitch;
				this->Has_palette = srf.Has_palette;
				this->field_50 = srf.field_50;
				this->field_44 = 0;
				this->field_C = 0;
				this->Is_open = 1;
				this->field_4C = 1;
				return 1;
			}
			throw 0;
		}

		if (!this->Is_open)
			throw 0;

		if (this->Has_palette)
		{
			if (srf.Has_palette)
			{
				if (srf.sdesc.dwRGBBitCount != this->sdesc.dwRGBBitCount)
					throw 1;
				this->PalBlt(&srf, 0, 0, 0);
			}
			else
			{
				if (!this->Lock(NULL, NULL))
					throw 1;
				for (int i = 0, si = 1 << this->sdesc.dwRGBBitCount; i < si; i++)
					this->SetPaletteColor(i, ((i & 0xE0) << 16) | ((i & 3 | 32 * (i & 0x1C)) << 6), 0);
				this->Unlock();
			}
		}

		// surfaces has identical attributes, a simple blt will be enough
		if (!memcmp(&this->sdesc, &srf.sdesc, sizeof(this->sdesc)) ||
			srf.dwWidth != this->dwWidth ||
			srf.dwHeight != this->dwHeight ||
			srf.Has_palette ||
			this->Has_palette ||
			srf.sdesc.dwRGBBitCount != this->sdesc.dwRGBBitCount)
		{
			RECT rc0, rc1;
			SetRect(&rc0, 0, 0, this->dwWidth - 1, this->dwHeight - 1);
			SetRect(&rc1, 0, 0, srf.dwWidth - 1, srf.dwHeight - 1);
			if (this->Blt(&rc0, &rc1, &srf, 0, 0))
				return 1;
			throw 1;
		}

		this->Lock(NULL, NULL);
		this->Lock(NULL, NULL);
		for (int y = 0; y < this->dwHeight; y++)
		{
			u8 *l0 = (u8*)this->CalcAddress(0, y);
			u8 *l1 = (u8*)srf.CalcAddress(0, y);

			switch (this->sdesc.dwRGBBitCount)
			{
			case 8:
				memcpy(l0, l1, this->dwWidth);
				break;
			case 16:
				memcpy(l0, l1, this->dwWidth * 2);
				break;
			case 32:
				memcpy(l0, l1, this->dwWidth * 4);
				break;
			default:
				throw 0;
			}
		}

		this->Unlock();
		srf.Unlock();
	}
	catch (int type)
	{
		switch (type)
		{
		case 1:	// unlock and close
			this->Unlock();
			srf.Unlock();
			this->Is_open = 0;
		case 0:	// regular return
			return 0;
		}
	}

	return 1;
}

int CMarniSurface2::CreateWork(int width, int height, int bmp_depth, int srf_depth)
{
	this->Release();

	this->dwWidth = width;
	this->dwHeight = height;
	this->sdesc.dwRGBBitCount = bmp_depth;

	assert(this->pData == NULL);

	switch (bmp_depth)
	{
	case 4:
		this->pData = new u8[width * height / 2];
		this->lPitch = width / 2;
		break;
	case 8:
		this->pData = new u8[width * height];
		this->lPitch = width;
		break;
	case 16:
		this->pData = new u8[2 * width * height];
		this->lPitch = 2 * width;
		break;
	case 32:
		this->pData = new u8[4 * width * height];
		this->lPitch = 4 * width;
		break;
	default:
		return 0;
	}

	if (!this->pData)
	{
		//MarniOut2(aGpbGnvKmxVOFs, aMarnibitsCreat);
		return 0;
	}

	this->sdesc.bpp = srf_depth;
	if (srf_depth)
	{
		size_t size;
		this->Has_palette = 1;
		switch (bmp_depth)
		{
		case 4:
			size = 2 * (srf_depth & ~7);
			break;
		case 8:
			size = 32 * (srf_depth & ~7);
			break;
		default:
			return 0;
		}

		assert(this->pPalette == NULL);
		this->pPalette = new u8[size];
		if(!this->pPalette)
		{
			delete[] this->pData;
			this->pData = NULL;
			return 0;
		}
	}
	
	switch (bmp_depth)
	{
	case 4:
	case 8:
	case 16:
		this->sdesc.dwRBitMask_setcnt = 11;
		this->sdesc.dwRGBAlphaBitMask_setcnt = 0;
		this->sdesc.field_16 = 0;
		this->sdesc.field_A = 6;
		this->sdesc.field_8 = 63;
		this->sdesc.field_14 = 0;
		this->sdesc.field_4 = 5;
		this->sdesc.field_2 = 31;
		this->sdesc.dwRBitMask_cnt = 5;
		this->sdesc.dwBBitMask_setcnt = 0;
		this->sdesc.field_10 = 5;
		this->sdesc.field_E = 31;
		break;
	case 32:
		this->sdesc.dwRGBAlphaBitMask_setcnt = 24;
		this->sdesc.dwRBitMask_setcnt = 16;
		this->sdesc.field_16 = 8;
		this->sdesc.field_14 = 255;
		this->sdesc.field_4 = 8;
		this->sdesc.dwBBitMask_setcnt = 0;
		this->sdesc.field_2 = 255;
		this->sdesc.dwRBitMask_cnt = 8;
		this->sdesc.field_A = 8;
		this->sdesc.field_8 = 255;
		this->sdesc.field_10 = 8;
		this->sdesc.field_E = 255;
		break;
	default:
		break;
	}

	this->Is_open = 1;
	this->field_44 = 1;
	this->field_4C = 1;

	return 1;
}

#define GETR(x)		(x & 0xff)
#define GETG(x)		((x & 0xff00) >> 8)
#define GETB(x)		((x & 0xff0000) >> 16)
#define GETA(x)		(x >> 24)

int CMarniSurface2::GetPaletteColor(int index, u32 *rgb)
{
	if (!this->Is_open)
		return 0;
	if (!this->field_C)
		return 0;
	if (!this->Has_palette)
		return 0;
	if (1 << this->sdesc.dwRGBBitCount <= index)
		return 0;

	u32 p;
	switch (this->sdesc.bpp)
	{
	case 16:
		p = ((u16*)this->pPalette)[index];
		break;
	case 32:
		p = ((u32*)this->pPalette)[index];
		break;
	default:
		return 0;
	}

	*rgb =    (((u16)this->sdesc.field_E & (p >> (char)(this->sdesc.dwBBitMask_setcnt))) << (8 - (char)(this->sdesc.field_10)))      & 0xFF |
		((u16)(((u16)this->sdesc.field_8 & (p >> (char)(this->sdesc.dwRBitMask_cnt)))    << (8 - (char)(this->sdesc.field_A))) << 8) & 0xFF00 |
		    (((((u16)this->sdesc.field_2 & (p >> (char)(this->sdesc.dwRBitMask_setcnt))) << (8 - (char)(this->sdesc.field_4)))       & 0xFF) << 16);
	return 1;
}

int CMarniSurface2::SetPaletteColor(int index, u32 rgb, int a3)
{
	if (!this->Is_open)
		return 0;

	if (a3)
	{
		//フラグに対応していません
		Marni2Out("flag is not supported", __FUNCTION__);
		return 0;
	}
	else if (this->field_C)
	{

	}
	else return 0;

	return 1;
}

int CMarniSurface2::SetCurrentColor(int x, int y, u32 rgb, u32 flag)
{
	if (this->Has_palette)
		return SetIndexColor(x, y, rgb, flag);

	u8* pP = (u8*)CalcAddress(x, y);
	if (!pP)
		return 0;

	u8 a = GETA(rgb),
		b = GETB(rgb),
		g = GETG(rgb),
		r = GETR(rgb);

	u32 p;
	// allows blending
	if (flag & 8)
	{
		GetCurrentColor(x, y, &p);
		// full alpha, get previous color
		if (a == 255)
		{
			r = GETR(p);
			g = GETG(p);
			b = GETB(p);
		}
		// alpha in the middle, blend
		else if (a)
		{
			// apply alpha weights
			b = ((b*(256 - a)) >> 8) + ((a * GETB(p)) >> 8);
			g = ((g*(256 - a)) >> 8) + ((a * GETG(p)) >> 8);
			r = ((r*(256 - a)) >> 8) + ((a * GETR(p)) >> 8);
		}
	}

	// converts pixel to whatever format the surface uses
	p = ((this->sdesc.field_14 & (a >> (8 - (char)(this->sdesc.field_16)))) << (char)(this->sdesc.dwRGBAlphaBitMask_setcnt)) |
		((this->sdesc.field_E  & (r >> (8 - (char)(this->sdesc.field_10)))) << (char)(this->sdesc.dwBBitMask_setcnt)) |
		((this->sdesc.field_8  & (g >> (8 - (char)(this->sdesc.field_A )))) << (char)(this->sdesc.dwRBitMask_cnt)) |
		((this->sdesc.field_2  & (b >> (8 - (char)(this->sdesc.field_4 )))) << (char)(this->sdesc.dwRBitMask_setcnt));

	switch (sdesc.dwRGBBitCount)
	{
	case 8:
		if (flag & 2) *pP &= p;
		else if (flag & 4) *pP |= p;
		else *pP = p;
		break;
	case 16:
		if (flag & 2) *(WORD*)pP &= p;
		else if (flag & 4) *(WORD*)pP |= p;
		else *(WORD*)pP = p;
		break;
	case 24:
		if (flag & 2)
		{
			*(WORD*)pP &= p;
			pP[2] &= (p >> 16);
		}
		else if (flag & 4)
		{
			*(WORD*)pP |= p;
			pP[2] |= (p >> 16);
		}
		else
		{
			*(WORD*)pP = p;
			pP[2] = (p >> 16);
		}
		break;
	case 32:
		if (flag & 2) *(DWORD *)pP &= p;
		else if (flag & 4) *(DWORD *)pP |= p; 
		else *(DWORD *)pP = p;
		break;
	default:
		return 0;
	}

	return 1;
}

void* CMarniSurface2::CalcAddress(int x, int y)
{
	if (!this->Is_open || !this->field_C)
		return 0;
	// out of boundaries
	if (this->dwWidth <= x || this->dwHeight <= y || x < 0 || y < 0)
	{
		//MarniOut1(aTheCoordinateY, x, y);
		return 0;
	}

	switch (this->sdesc.dwRGBBitCount)
	{
	case 4:
		return &this->pData[y * this->lPitch + x / 2];
	case 8:
		return &this->pData[y * this->lPitch + x];
	case 16:
		return &this->pData[y * this->lPitch + 2 * x];
	case 24:
		return &this->pData[y * this->lPitch + 3 * x];
	case 32:
		return &this->pData[y * this->lPitch + 4 * x];
	}

	return 0;
}

int CMarniSurface2::SetIndexColor(int x, int y, u32 rgb, u32 flag)
{
	return 1;
}

int CMarniSurface2::GetCurrentColor(int x, int y, u32 *pixel)
{
	if (this->Has_palette)
		return this->GetIndexColor(x, y, pixel);

	u8 *pB = (u8*)this->CalcAddress(x, y);
	if (!pB)
		return 0;

	u32 p;
	switch (this->sdesc.dwRGBBitCount)
	{
	case 4:
		p = *pB;	// get both nibbles
		if (!this->field_50)		// nibble endianness?
		{
			if (!(x & 1)) p >>= 4;	// upper nibble
			else p &= 0xf;			// lower nibble
		}
		else
		{
			if (x & 1) p >>= 4;		// upper nibble
			else p &= 0xf;			// lower nibble
		}
		break;
	case 8:
		p = *pB;
		break;
	case 16:
		p = *(u16*)pB;
		break;
	case 32:
		p = *(u32*)pB;
		break;
	}

	*pixel = ((this->sdesc.field_E & (p >> LOBYTE(this->sdesc.dwBBitMask_setcnt))) << (8 - LOBYTE(this->sdesc.field_10))) & 0xFF |
		((((this->sdesc.field_2 & (p >> LOBYTE(this->sdesc.dwRBitMask_setcnt))) << (8 - LOBYTE(this->sdesc.field_4))) & 0xFF) << 16) |
		(((u16)this->sdesc.field_14 & (p >> LOBYTE(this->sdesc.dwRGBAlphaBitMask_setcnt))) << (8 - LOBYTE(this->sdesc.field_16)) << 16 |
		(this->sdesc.field_8 & (p >> LOBYTE(this->sdesc.dwRBitMask_cnt))) << (8 - LOBYTE(this->sdesc.field_A))) << 8;

	return 1;
}

//////////////////////////////////////////
// HARDWARE SURFACES					//
//////////////////////////////////////////
CMarniSurface::CMarniSurface()
{
	CMarniSurface2::CMarniSurface2();
	this->DDpalette = NULL;
	this->DDsurface = NULL;
	this->Is_vmem = 0;
}

CMarniSurface::~CMarniSurface()
{
	Release();
}

//////////////////////////////////////////
// virtual methods
int CMarniSurface::ClearBg(int *adjust, int rgb, int use_image)
{
	if (use_image)
		return CMarniSurface2::ClearBg(adjust, rgb, use_image);

	if (!this->Is_open)
		return 0;

	RECT rect, dstrect;
	if (adjust)
	{
		SetRect(&rect, 0, 0, this->dwWidth - 1, this->dwHeight - 1);
		if (!Adjust_rect(&rect, adjust, &dstrect))
			return 0;
		dstrect.right++;
		dstrect.bottom++;
	}
	else SetRect(&dstrect, 0, 0, this->dwWidth, this->dwHeight);

	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(bltfx));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = rgb;
	if (this->DDsurface->Blt(&dstrect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &bltfx))
	{
		this->Is_open = 0;
		return 0;
	}

	return 1;
}

int CMarniSurface::Blt(RECT *dstrect, RECT *srcrect, CMarniSurface2 *src, int a5, float *a6)
{
	return CMarniSurface2::Blt(dstrect, srcrect, src, a5, a6);
}

int CMarniSurface::Null() { return 1; }

int CMarniSurface::Lock(u8 **ppBitmap, u8 **ppPalette)
{
	if (!this->Is_open)
		return 0;

	DDSURFACEDESC ddesc;
	memset(&ddesc, 0, sizeof(ddesc));
	ddesc.dwSize = sizeof(ddesc);

	if (this->DDsurface->Lock(NULL, &ddesc, 1, NULL))
		return 0;

	this->pData = (u8*)ddesc.lpSurface;
	this->dwWidth = ddesc.dwWidth;
	this->dwHeight = ddesc.dwHeight;
	this->lPitch = ddesc.lPitch;
	this->sdesc.dwRGBBitCount = (char) ddesc.ddpfPixelFormat.dwRGBBitCount;
	this->pPalette = 0;

	// is this even used?
	if (this->Has_palette)
	{
		PALETTEENTRY palette[256];
		size_t size = 1 << this->sdesc.dwRGBBitCount;
		assert(this->pPalette == NULL);
		this->pPalette = new u8[4 * size];
		if (this->DDpalette->GetEntries(0, 0, size, palette))
			return 0;
		// convert from hardware palette to software palette
		for (size_t i = 0; i < size; i++)
		{
			// TODO: check conversion values
		}
		this->sdesc.dwRGBAlphaBitMask_setcnt = 24;
		this->sdesc.dwRBitMask_setcnt = 16;
		this->sdesc.field_14 = 255;
		this->sdesc.field_16 = 8;
		this->sdesc.field_2 = 255;
		this->sdesc.dwBBitMask_setcnt = 0;
		this->sdesc.field_4 = 8;
		this->sdesc.dwRBitMask_cnt = 8;
		this->sdesc.field_8 = 255;
		this->sdesc.field_A = 8;
		this->sdesc.field_E = 255;
		this->sdesc.field_10 = 8;
	}

	if (ppBitmap) *ppBitmap = this->pData;
	if (ppPalette) *ppPalette = this->pPalette;

	this->field_C = 1;

	return 1;
}

int CMarniSurface::Unlock()
{
	if (!this->Is_open)
		return -1;

	this->DDsurface->Unlock(NULL);
	if (this->Has_palette)
	{
		// no palette assigned
		if (!this->pPalette)
			return 0;
		PALETTEENTRY pal[256];
		for (int i = 0, si = 1 << this->sdesc.dwRGBBitCount; i < si; i++)
		{
			pal[i].peRed;
			pal[i].peGreen;
			pal[i].peBlue;
			pal[i].peFlags = 0;
		}
		// set failed
		if (this->DDpalette->SetEntries(0, 0, 1 << this->sdesc.dwRGBBitCount, pal))
			return 0;
	}

	this->pData = NULL;
	this->pPalette = NULL;
	this->field_C = 0;

	return 0;
}

int CMarniSurface::Release()
{
	if (this->field_44)
	{
		if (this->DDsurface)
		{
			this->DDsurface->Release();
			this->DDsurface = NULL;
		}

		if (this->DDpalette)
		{
			this->DDpalette->Release();
			this->DDpalette = NULL;
		}
	}

	CMarniSurface2::Release();

	return 1;
}

//////////////////////////////////////////
// regular methods
int CMarniSurface::clearBG(int *adjust, u32 rgb, int use_image)
{
	return 1;
}

int CMarniSurface::CreateOffscreenSurface(IDirectDraw *pDD, int dwWidth, int dwHeight)
{
	DDSURFACEDESC ddesc;
	memset(&ddesc, 0, sizeof(ddesc));

	// override parameters
	ddesc.dwSize = sizeof(ddesc);
	ddesc.dwWidth = dwWidth;
	ddesc.dwHeight = dwHeight;
	ddesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	return DirectDrawSurface(pDD, &ddesc);
}

int CMarniSurface::CreateOffscreenSurface(IDirectDraw *pDD, DDSURFACEDESC *desc)
{
	DDSURFACEDESC ddesc;
	memset(&ddesc, 0, sizeof(ddesc));

	// override parameters
	ddesc.dwSize = sizeof(ddesc);
	ddesc.dwWidth = desc->dwWidth;
	ddesc.dwHeight = desc->dwHeight;
	ddesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	return DirectDrawSurface(pDD, &ddesc);
}

int CMarniSurface::Test(IDirectDraw *pDirectDraw, DDSURFACEDESC *pDesc)
{
	return 1;
}

int CMarniSurface::CreateCompatibleSurface(IDirectDraw *pDD, DDSURFACEDESC *desc)
{
	return DirectDrawSurface(pDD, desc);
}

int CMarniSurface::DirectDrawSurface(IDirectDraw *pDD, DDSURFACEDESC *pDesc)
{
	assert(this->DDsurface == NULL);
	DDSURFACEDESC dsdesc0, dsdesc1;

	// pad size to a power of 2
	DWORD wh = upper_power_of_two(pDesc->dwWidth, pDesc->dwHeight);
	memcpy(&dsdesc0, pDesc, sizeof(dsdesc0));
	dsdesc0.dwWidth = wh;
	dsdesc0.dwHeight = wh;

	// create a square surface
	if (pDD->CreateSurface(&dsdesc0, &this->DDsurface, NULL))
		return 0;

	GetSurfaceDesc(&dsdesc1, this->DDsurface);
	// determine if surface is software or hardware
#if 0
	this->Is_vmem = !(dsdesc1.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? 0 : 1;
#else
	this->Is_vmem = dsdesc1.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ? 0 : 1;
#endif

	DDCOLORKEY color_key;
	color_key.dwColorSpaceLowValue = 0;
	color_key.dwColorSpaceHighValue = 0;
	if (this->DDsurface->SetColorKey(8, &color_key))
	{
		this->Release();
		return 0;
	}

	DWORD dwFlags;
	// ---
	this->sdesc.dwRGBBitCount = (char)dsdesc1.ddpfPixelFormat.dwRGBBitCount;
	dwFlags = 0;
	if (dsdesc1.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
	{
		dwFlags = DDPCAPS_4BIT;
		this->sdesc.bpp = 32;
		this->Has_palette = 1;
	}
	if (dsdesc1.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
	{
		dwFlags = DDPCAPS_8BIT;
		this->sdesc.bpp = 32;
		this->Has_palette = 1;
	}

	// ---
	if (this->Has_palette)
	{
		PALETTEENTRY palette[256];
		if (pDD->CreatePalette(dwFlags, palette, &this->DDpalette, NULL))
			return 0;
		//if(pDD->QueryInterface->Release(pDD))
		// needs unspecified ddraw method here
		this->sdesc.dwRGBAlphaBitMask_setcnt = 24;
		this->sdesc.dwRBitMask_setcnt = 16;
		this->sdesc.field_14 = 255;
		this->sdesc.field_16 = 8;
		this->sdesc.field_2 = 255;
		this->sdesc.dwBBitMask_setcnt = 0;
		this->sdesc.field_4 = 8;
		this->sdesc.dwRBitMask_cnt = 8;
		this->sdesc.field_8 = 255;
		this->sdesc.field_A = 8;
		this->sdesc.field_E = 255;
		this->sdesc.field_10 = 8;
	}
	else CalcPixelFormat(&dsdesc1, &this->sdesc);

	this->dwWidth = dsdesc1.dwWidth;
	this->dwHeight = dsdesc1.dwHeight;
	this->lPitch = dsdesc1.lPitch;
	this->Is_open = 1;
	this->field_4C = 0;
	this->field_44 = 1;

	return 1;
}

int CMarniSurface::BltSurface(RECT *dstrect, RECT *srcrect, CMarniSurface *pSrcSurface, int a5, float *a6)
{
	if (!this->Is_open)
		return 0;

	if (a5 & 0xe)
		return this->Blt(dstrect, srcrect, pSrcSurface, a5, a6);

	if(a6 && (a6[0] != 1.f || a6[1] != 1.f || a6[2] != 1.f))
		return this->Blt(dstrect, srcrect, pSrcSurface, a5, a6);

	CRect rc0, rc1;
	if (srcrect)
	{
		if (srcrect->top < 0
			|| srcrect->left < 0
			|| srcrect->right >= pSrcSurface->dwWidth
			|| srcrect->bottom > pSrcSurface->dwHeight)
		{
			// needs warning code here
			return 0;
		}
		rc0 = srcrect;
	}
	else rc0.Set(0, 0, pSrcSurface->dwWidth - 1, pSrcSurface->dwHeight - 1);

	if (dstrect)
	{
		CRect trect;
		trect.Set(0, 0, this->dwWidth - 1, this->dwHeight - 1);
		if (!Adjust_rect(&trect, (int*)&dstrect->left, &rc1))
			return 0;
		rc1 = dstrect;
	}
	else rc1.Set(0, 0, this->dwWidth - 1, this->dwHeight - 1);

	int w0, h0, left0, left1, right1, top0, top1, bottom, widt0, heig0;
	w0 = rc0.right - rc0.left;
	h0 = rc0.bottom - rc0.top;
	//
	if (rc1.left >= 0)
	{
		left1 = rc1.left;
		left0 = rc0.left;
		widt0 = rc0.right - rc0.left;
	}
	else
	{
		left1 = 0;
		left0 = rc0.left + w0 * rc1.left / (rc1.left - rc1.right);
		widt0 = w0 - w0 * rc1.left / (rc1.left - rc1.right);
	}
	//
	if (rc1.top >= 0)
	{
		top0 = rc0.top;
		top1 = rc1.top;
		heig0 = rc0.bottom - rc0.top;
	}
	else
	{
		top1 = 0;
		top0 = h0 * rc1.top / (rc1.top - rc1.bottom) + rc0.top;
		heig0 = h0 - h0 * rc1.top / (rc1.top - rc1.bottom);
	}
	//
	if (rc1.right < this->dwWidth)
		right1 = rc1.right;

	else
	{
		right1 = this->dwWidth - 1;
		widt0 = w0 * (this->dwWidth - rc1.left) / (unsigned int)(rc1.right - rc1.left);
	}
	//
	if (rc1.bottom < this->dwHeight)
		bottom = rc1.bottom;
	else
	{
		bottom = this->dwHeight - 1;
		heig0 = h0 * (this->dwHeight - rc1.top) / (unsigned int)(rc1.bottom - rc1.top);
	}

	CRect dstRect, srcRect;
	DWORD dwFlags;
	DDBLTFX ddbltfx;

	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	if ((a5 & (BLTMODE_MIRROR | BLTMODE_FLIP)) == (BLTMODE_MIRROR | BLTMODE_FLIP))
	{
		ddbltfx.dwDDFX |= DDOVERFX_MIRRORLEFTRIGHT | DDOVERFX_MIRRORUPDOWN;
		SetRect(&srcRect,
			2 * rc0.left + w0 - widt0 - left0,
			2 * rc0.top  + h0 - heig0 - top0,
			2 * rc0.left + w0 - left0,
			2 * rc0.top + h0 - top0);
	}
	else if (a5 & BLTMODE_FLIP)
	{
		ddbltfx.dwDDFX |= DDOVERFX_MIRRORUPDOWN;
		SetRect(&srcRect,
			2 * rc0.left + w0 - widt0 - left0,
			top0,
			2 * rc0.left + w0 - left0,
			top0 + heig0);
	}
	else if (a5 & BLTMODE_MIRROR)
	{
		ddbltfx.dwDDFX |= DDOVERFX_MIRRORLEFTRIGHT;
		SetRect(&srcRect,
			left0,
			top0,
			left0 + widt0,
			2 * rc0.top + h0 - top0);
	}
	else SetRect(&srcRect,
		left0,
		top0,
		left0 + widt0,
		top0  + heig0);
	SetRect(&dstRect, left1, top1, right1, bottom);
	++srcRect.right;
	++srcRect.bottom;
	++dstRect.right;
	++dstRect.bottom;

	if (srcRect.top == srcRect.bottom)
		return 0;
	if (srcRect.right == srcRect.left)
		return 0;

	if (a5 & 1)
		dwFlags = DDBLT_WAIT | DDBLT_KEYSRC;
	else
	{
		if (srcRect.right == srcRect.left + (dstRect.right - dstRect.left) / 2
			&& this->sdesc.dwRGBBitCount == 16
			&& pSrcSurface->sdesc.dwRGBBitCount == 16)
		{
			int w = srcRect.Width(),
				h = srcRect.Height();
			u8 *pBmpDst,
				*pBmpSrc;
			if (!this->Lock(&pBmpDst, NULL) || !pSrcSurface->Lock(&pBmpSrc, NULL))
			{
				// add warning
				return 0;
			}

			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{

				}
			}

			this->Unlock();
			pSrcSurface->Unlock();
		}
		else
		{
			dwFlags = DDBLT_WAIT;
			if (this->DDsurface->Blt(&dstRect, pSrcSurface->DDsurface, &srcRect, dwFlags, &ddbltfx))
			{
				// warning message
				return 0;
			}
		}
	}

	return 1;
}

//////////////////////////////////////////
CMarniSurfaceEx::CMarniSurfaceEx()
{
	CMarniSurface::CMarniSurface();
	TextureHandle = NULL;
	DDtexture = NULL;
}

CMarniSurfaceEx::~CMarniSurfaceEx()
{
	Release();
}

int CMarniSurfaceEx::Release()
{
	if (this->DDtexture)
	{
		this->DDtexture->Release();
		this->DDtexture = NULL;
	}

	CMarniSurface::Release();

	return 1;
}

int CMarniSurfaceEx::CreateTextureObject()
{
	if (!this->Is_open)
		return 0;

	if (this->DDtexture)
	{
		this->DDtexture->Release();
		this->DDtexture = NULL;
	}

	if (this->DDsurface->QueryInterface(IID_IDirect3DTexture, (void**)&this->DDtexture))
	{
		Release();
		return 0;
	}

	return 1;
}

int CMarniSurfaceEx::GetTextureHandle(IDirect3DDevice *pD3DDevice)
{
	try
	{
		if (!this->Is_open)
			throw 0;

		if (!this->DDtexture)
			throw 0;

		if (this->DDtexture->GetHandle(pD3DDevice, &this->TextureHandle))
			throw 0;
	}
	catch (int)
	{
		this->Release();
		return 0;
	}
	return 1;
}

//////////////////////////////////////////

