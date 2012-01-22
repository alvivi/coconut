/*
 * core - Minimalist games engine
 * SDL_rotozoom LGPL (c) A. Schiffler
 * Copyright (C) 2006 Álvaro Vilanova Vidal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/**
 * @file gfx.h
 * Implmentación de las funciones de tratamiento de gráficos. Más información
 * en el archivo de cabecera.
 **/


#include <stdlib.h>
#include <string.h>

#include "core.h"

#define VALUE_LIMIT	0.001


/*
 * CRE_GfxAlphaBlit
 * Hace un blit entre dos superficies teniendo en cuenta el canal alpha indicado
 * como parámetro incluso cuando las superficies son de 32bits, caso en el que
 * hay que aplicar el cambio del canal alpha pixel a pixel
 */
int CRE_GfxAlphaBlit(SDL_Surface * Src, SDL_Surface * Trg, SDL_Rect * Rect,
    Uint8 Alpha)
{
    /* Lienzo temporal usado en caso de que el blit sea de 32 bits */
    SDL_Surface * Canvas = NULL;
    /* Bufers del lienzo y el origen (32 bits) */
    Uint32 * cBuffer, * sBuffer;
    /* Contadores para recorrer los gráficos (32 bits) */
    Uint32 max, a;
    /*
     * Variable temporal que almace la diferencia entre el canala alpha de un
     * pixel y el establecido (32 bits)
     */
    Sint32 Tmp;

    /* Comprobamos que los datos son correctos */
    if(Trg == NULL || Src == NULL || Rect == NULL)
        return -1;

    /* Si no hay transparencia en el blit, aplicamos uno normalmente */
    if(Alpha == 255) {
        SDL_BlitSurface(Src, NULL, Trg, Rect);

    }
    /* Si hay un cierto nivel de transparencia */
    else {
        /*
         * Si el gráfico no es de 32 bits, aplicamos el canal alpha a toda la
         * superficie del gráfico
         */
        if(Src->format->BitsPerPixel != 32) {
            /* Aplicamos el canal alpha */
            SDL_SetAlpha(Src, SDL_SRCALPHA, Alpha);
            /* Hacemos el blit */
            SDL_BlitSurface(Src, NULL, Trg, Rect);
        }
        /*
         * Si el gráfico es de 32 bits, debemos aplicar a la diferencia de los
         * canales alphas a cada pixel. Para ello usamos un gráfico temporal.
         */
        else {
            /* Obtemeos la diferencia (complemento de alpha, 255 - Alpha) */
            Alpha = ~Alpha;
            /* Creamos el gráfico temporal a imagen del origen */
            Canvas = SDL_CreateRGBSurface(SDL_HWSURFACE, Src->w, Src->h,
                Src->format->BitsPerPixel, Src->format->Rmask,
                Src->format->Gmask, Src->format->Bmask, Src->format->Amask);
            /* Obtenemos las direcciones de los buffers de datos */
            cBuffer = (Uint32 *) Canvas->pixels;
            sBuffer = (Uint32 *) Src->pixels;
            /* Recorremos el gráfico de 32 bits pixel a pixel */
            for(max = Src->w * Src->h; max > 0; max--, cBuffer++, sBuffer++) {
                /* Obtenemos el canal alpha del pixel actual */
                a = ((*sBuffer) & (Src->format->Amask)) >> Src->format->Ashift;
                /* Calculamso la diferencia entre los canales */
                Tmp = a - Alpha;
                /* Si la diferencia es negativa, lo ajutamos a 0 (Transpa.) */
                a = (Tmp < 0) ? 0 : Tmp;
                /* LLevamos al valor obtenido a su posición */
                a = a << Src->format->Ashift;
                /* Unimos el canal alpha nuevo con el RGB y hacemo el blit */
                *cBuffer = a | ((*sBuffer) & (~Src->format->Amask));

            }
            /* Copiamos el Lienzo temporal a la imagen destino */
            SDL_BlitSurface(Canvas, NULL, Trg, Rect);
        }
    }

    /* Si hemos usado un lienzo temporal, lo eliminamos */
    if(Canvas != NULL)
        SDL_FreeSurface(Canvas);

    return 0;
}


/*
 * CRE_GfxZSurfaceRGBA
 * Hace un zoom de la superfecie de 32 bits y da el resultado en dst.
 * By  A. Schiffler.
 */
int CRE_GfxZSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx,
    int flipy, int smooth)
{
    int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep;
    creGfxColorRGBA *c00, *c01, *c10, *c11;
    creGfxColorRGBA *sp, *csp, *dp;
    int dgap;

    /*
     * Variable setup
     */
    if (smooth) {
	/*
	 * For interpolation: assume source dimension is one pixel
	 */
	/*
	 * smaller to avoid overflow on right and bottom edge.
	 */
	sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w);
	sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h);
    } else {
	sx = (int) (65536.0 * (float) src->w / (float) dst->w);
	sy = (int) (65536.0 * (float) src->h / (float) dst->h);
    }

    /*
     * Allocate memory for row increments
     */
    if ((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
	return (-1);
    }
    if ((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
	free(sax);
	return (-1);
    }

    /*
     * Precalculate row increments
     */
    sp = csp = (creGfxColorRGBA *) src->pixels;
    dp = (creGfxColorRGBA *) dst->pixels;

    if (flipx) csp += (src->w-1);
    if (flipy) csp  = (creGfxColorRGBA*)( (Uint8*)csp + src->pitch*(src->h-1) );

    csx = 0;
    csax = sax;
    for (x = 0; x <= dst->w; x++) {
	*csax = csx;
	csax++;
	csx &= 0xffff;
	csx += sx;
    }
    csy = 0;
    csay = say;
    for (y = 0; y <= dst->h; y++) {
	*csay = csy;
	csay++;
	csy &= 0xffff;
	csy += sy;
    }

    dgap = dst->pitch - dst->w * 4;

    /*
     * Switch between interpolating and non-interpolating code
     */
    if (smooth) {

	/*
	 * Interpolating Zoom
	 */

	/*
	 * Scan destination
	 */
	csay = say;
	for (y = 0; y < dst->h; y++) {
	    /*
	     * Setup color source pointers
	     */
	    c00 = csp;
	    c01 = csp;
	    c01++;
	    c10 = (creGfxColorRGBA *) ((Uint8 *) csp + src->pitch);
	    c11 = c10;
	    c11++;
	    csax = sax;
	    for (x = 0; x < dst->w; x++) {

		/*
		 * Interpolate colors
		 */
		ex = (*csax & 0xffff);
		ey = (*csay & 0xffff);
		t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
		t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
		dp->r = (((t2 - t1) * ey) >> 16) + t1;
		t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
		t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
		dp->g = (((t2 - t1) * ey) >> 16) + t1;
		t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
		t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
		dp->b = (((t2 - t1) * ey) >> 16) + t1;
		t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
		t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
		dp->a = (((t2 - t1) * ey) >> 16) + t1;

		/*
		 * Advance source pointers
		 */
		csax++;
		sstep = (*csax >> 16);
		c00 += sstep;
		c01 += sstep;
		c10 += sstep;
		c11 += sstep;
		/*
		 * Advance destination pointer
		 */
		dp++;
	    }
	    /*
	     * Advance source pointer
	     */
	    csay++;
	    csp = (creGfxColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
	    /*
	     * Advance destination pointers
	     */
	    dp = (creGfxColorRGBA *) ((Uint8 *) dp + dgap);
	}

    } else {

	/*
	 * Non-Interpolating Zoom
	 */

	csay = say;
	for (y = 0; y < dst->h; y++) {
	    sp = csp;
	    csax = sax;
	    for (x = 0; x < dst->w; x++) {
		/*
		 * Draw
		 */
		*dp = *sp;
		/*
		 * Advance source pointers
		 */
		csax++;
		sstep = (*csax >> 16);
		if (flipx) sstep = -sstep;
		sp += sstep;
		/*
		 * Advance destination pointer
		 */
		dp++;
	    }
	    /*
	     * Advance source pointer
	     */
	    csay++;
	    sstep = (*csay >> 16) * src->pitch;
	    if (flipy) sstep = -sstep;
	    csp = (creGfxColorRGBA *) ((Uint8 *) csp + sstep);

	    /*
	     * Advance destination pointers
	     */
	    dp = (creGfxColorRGBA *) ((Uint8 *) dp + dgap);
	}

    }

    /*
     * Remove temp arrays
     */
    free(sax);
    free(say);

    return (0);
}


/*
 * CRE_GfxZSurfaceY
 * Hace un zoom de la superfecie de 8 bits y da el resultado en dst.
 * By  A. Schiffler.
 */
int CRE_GfxZSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy)
{
    Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
    Uint8 *sp, *dp, *csp;
    int dgap;

    /*
     * Variable setup
     */
    sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
    sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);

    /*
     * Allocate memory for row increments
     */
    if ((sax = (Uint32 *) malloc(dst->w * sizeof(Uint32))) == NULL) {
	return (-1);
    }
    if ((say = (Uint32 *) malloc(dst->h * sizeof(Uint32))) == NULL) {
	if (sax != NULL) {
	    free(sax);
	}
	return (-1);
    }

    /*
     * Precalculate row increments
     */
    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
	csx += sx;
	*csax = (csx >> 16);
	csx &= 0xffff;
	csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
	csy += sy;
	*csay = (csy >> 16);
	csy &= 0xffff;
	csay++;
    }

    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
	csx += (*csax);
	csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
	csy += (*csay);
	csay++;
    }

    /*
     * Pointer setup
     */
    sp = csp = (Uint8 *) src->pixels;
    dp = (Uint8 *) dst->pixels;
    dgap = dst->pitch - dst->w;

    /*
     * Draw
     */
    csay = say;
    for (y = 0; y < dst->h; y++) {
	csax = sax;
	sp = csp;
	for (x = 0; x < dst->w; x++) {
	    /*
	     * Draw
	     */
	    *dp = *sp;
	    /*
	     * Advance source pointers
	     */
	    sp += (*csax);
	    csax++;
	    /*
	     * Advance destination pointer
	     */
	    dp++;
	}
	/*
	 * Advance source pointer (for row)
	 */
	csp += ((*csay) * src->pitch);
	csay++;
	/*
	 * Advance destination pointers
	 */
	dp += dgap;
    }

    /*
     * Remove temp arrays
     */
    free(sax);
    free(say);

    return (0);
}


/*
 * CRE_GfxRSurfaceRGBA
 * Rota la imagen de 32 bits dada en src y el resultado lo da en dst.
 * By A. Schiffler
 */
void CRE_GfxRSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int cx, int cy,
    int isin, int icos, int flipx, int flipy, int smooth)
{
    int x, y, t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
    creGfxColorRGBA c00, c01, c10, c11;
    creGfxColorRGBA *pc, *sp;
    int gap;

    /*
     * Variable setup
     */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = dst->pixels;
    gap = dst->pitch - dst->w * 4;

    /*
     * Switch between interpolating and non-interpolating code
     */
    if (smooth) {
	for (y = 0; y < dst->h; y++) {
	    dy = cy - y;
	    sdx = (ax + (isin * dy)) + xd;
	    sdy = (ay - (icos * dy)) + yd;
	    for (x = 0; x < dst->w; x++) {
		dx = (sdx >> 16);
		dy = (sdy >> 16);
		if ((dx >= -1) && (dy >= -1) && (dx < src->w) && (dy < src->h)) {
		    if ((dx >= 0) && (dy >= 0) && (dx < sw) && (dy < sh)) {
			sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			sp += 1;
			c01 = *sp;
			sp = (creGfxColorRGBA *) ((Uint8 *) sp + src->pitch);
			sp -= 1;
			c10 = *sp;
			sp += 1;
			c11 = *sp;
		    } else if ((dx == sw) && (dy == sh)) {
			sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if ((dx == -1) && (dy == -1)) {
			sp = (creGfxColorRGBA *) (src->pixels);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if ((dx == -1) && (dy == sh)) {
			sp = (creGfxColorRGBA *) (src->pixels);
			sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if ((dx == sw) && (dy == -1)) {
			sp = (creGfxColorRGBA *) (src->pixels);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if (dx == -1) {
			sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			sp = (creGfxColorRGBA *) ((Uint8 *) sp + src->pitch);
			c11 = *sp;
		    } else if (dy == -1) {
			sp = (creGfxColorRGBA *) (src->pixels);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			sp += 1;
			c11 = *sp;
		    } else if (dx == sw) {
			sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			sp = (creGfxColorRGBA *) ((Uint8 *) sp + src->pitch);
			c10 = *sp;
			c11 = *sp;
		    } else if (dy == sh) {
			sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			sp += 1;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    }
		    /*
		     * Interpolate colors
		     */
		    ex = (sdx & 0xffff);
		    ey = (sdy & 0xffff);
		    t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
		    t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
		    pc->r = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
		    t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
		    pc->g = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
		    t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
		    pc->b = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
		    t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
		    pc->a = (((t2 - t1) * ey) >> 16) + t1;
		}
		sdx += icos;
		sdy += isin;
		pc++;
	    }
	    pc = (creGfxColorRGBA *) ((Uint8 *) pc + gap);
	}
    } else {
	for (y = 0; y < dst->h; y++) {
	    dy = cy - y;
	    sdx = (ax + (isin * dy)) + xd;
	    sdy = (ay - (icos * dy)) + yd;
	    for (x = 0; x < dst->w; x++) {
		dx = (short) (sdx >> 16);
		dy = (short) (sdy >> 16);
		if (flipx) dx = (src->w-1)-dx;
		if (flipy) dy = (src->h-1)-dy;
		if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
		    sp = (creGfxColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
		    sp += dx;
		    *pc = *sp;
		}
		sdx += icos;
		sdy += isin;
		pc++;
	    }
	    pc = (creGfxColorRGBA *) ((Uint8 *) pc + gap);
	}
    }
}


/*
 * CRE_GfxRSurfaceY
 * Rota la imagen de 8 bits dada en src y el resultado lo da en dst
 * By A. Schiffler
 */
void CRE_GfxRSurfaceY(SDL_Surface * src, SDL_Surface * dst, int cx, int cy,
    int isin, int icos)
{
    int x, y, dx, dy, xd, yd, sdx, sdy, ax, ay, sw, sh;
    creGfxColorY *pc, *sp;
    int gap;

    /*
     * Variable setup
     */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = dst->pixels;
    gap = dst->pitch - dst->w;
    /*
     * Clear surface to colorkey
     */
    memset(pc, (unsigned char) (src->format->colorkey & 0xff),
        dst->pitch * dst->h);
    /*
     * Iterate through destination surface
     */
    for (y = 0; y < dst->h; y++) {
	dy = cy - y;
	sdx = (ax + (isin * dy)) + xd;
	sdy = (ay - (icos * dy)) + yd;
	for (x = 0; x < dst->w; x++) {
	    dx = (short) (sdx >> 16);
	    dy = (short) (sdy >> 16);
	    if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
		sp = (creGfxColorY *) (src->pixels);
		sp += (src->pitch * dy + dx);
		*pc = *sp;
	    }
	    sdx += icos;
	    sdy += isin;
	    pc++;
	}
	pc += gap;
    }
}


/*
 * CRE_GfxRZSurfaceSizeTrig
 * Función que devuelve el tamaño trigonométrico de una imagen que se le aplica
 * un cierto zoom y una rotación.
 * By A. Schiffler
 */
void CRE_GfxRZSurfaceSizeTrig(int width, int height, double angle, double zoomx,
    double zoomy, int *dstwidth, int *dstheight, double *canglezoom,
    double *sanglezoom)
{
    double x, y, cx, cy, sx, sy;
    double radangle;
    int dstwidthhalf, dstheighthalf;

    /*
     * Determine destination width and height by rotating a centered source box
     */
    radangle = angle * (M_PI / 180.0);
    *sanglezoom = sin(radangle);
    *canglezoom = cos(radangle);
    *sanglezoom *= zoomx;
    *canglezoom *= zoomx;
    x = width / 2;
    y = height / 2;
    cx = *canglezoom * x;
    cy = *canglezoom * y;
    sx = *sanglezoom * x;
    sy = *sanglezoom * y;
    dstwidthhalf = MAX((int)
        ceil(MAX(MAX(MAX(fabs(cx + sy), fabs(cx - sy)), fabs(-cx + sy)),
        fabs(-cx - sy))), 1);
    dstheighthalf = MAX((int)
        ceil(MAX(MAX(MAX(fabs(sx + cy), fabs(sx - cy)), fabs(-sx + cy)),
        fabs(-sx - cy))), 1);
    *dstwidth = 2 * dstwidthhalf;
    *dstheight = 2 * dstheighthalf;
}


/*
 * CRE_GfxRZSurfaceSizeXY
 * Función que devuelve el tamaño trigonométrico de una imagen que se le aplica
 * un cierto zoom y una rotación. Basada en CRE_GfxRZSurfaceSizeTrig.
 * By A. Schiffler
 */
void CRE_GfxRZSurfaceSizeXY(int width, int height, double angle, double zoomx,
    double zoomy, int *dstwidth, int *dstheight)
{
    double dummy_sanglezoom, dummy_canglezoom;

    CRE_GfxRZSurfaceSizeTrig(width, height, angle, zoomx, zoomy, dstwidth,
    dstheight, &dummy_sanglezoom, &dummy_canglezoom);
}


/*
 * CRE_GfxRZSurfaceSize
 * Función que devuelve el tamaño trigonométrico de una imagen que se le aplica
 * un cierto zoom y una rotación. Basada en CRE_GfxRZSurfaceSizeTrig.
 * By A. Schiffler
 */
void CRE_GfxRZSurfaceSize(int width, int height, double angle, double zoom,
    int *dstwidth, int *dstheight)
{
    double dummy_sanglezoom, dummy_canglezoom;

    CRE_GfxRZSurfaceSizeTrig(width, height, angle, zoom, zoom, dstwidth,
    dstheight, &dummy_sanglezoom, &dummy_canglezoom);
}


/*
 * CRE_GfxRZSurface
 * Adaptación pública de CRE_GfxRZSurfaceXY
 * By A. Schiffler
 */
SDL_Surface * CRE_GfxRZSurface(SDL_Surface * src, double angle, double zoom,
    int smooth)
{
  return CRE_GfxRZSurfaceXY(src, angle, zoom, zoom, smooth);
}


/*
 * CRE_GfxRZSurface
 * Función que rota y aplica un zoom a una imagen dada, independientemente de
 * la profundidad de ésta.
 * By A. Schiffler
 */
SDL_Surface * CRE_GfxRZSurfaceXY(SDL_Surface * src, double angle, double zoomx,
    double zoomy, int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    double zoominv;
    double sanglezoom, canglezoom, sanglezoominv, canglezoominv;
    int dstwidthhalf, dstwidth, dstheighthalf, dstheight;
    int is32bit;
    int i, src_converted;
    int flipx,flipy;

    /*
     * Sanity check
     */
    if (src == NULL)
	return (NULL);

    /*
     * Determine if source surface is 32bit or 8bit
     */
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
	/*
	 * Use source surface 'as is'
	 */
	rz_src = src;
	src_converted = 0;
    } else {
	/*
	 * New source surface is 32bit with a defined RGBA ordering
	 */
	rz_src =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff,
	    0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_BlitSurface(src, NULL, rz_src, NULL);
	src_converted = 1;
	is32bit = 1;
    }

    /*
     * Sanity check zoom factor
     */
    flipx = (zoomx<0);
    if (flipx) zoomx=-zoomx;
    flipy = (zoomy<0);
    if (flipy) zoomy=-zoomy;
    if (zoomx < VALUE_LIMIT) zoomx = VALUE_LIMIT;
    if (zoomy < VALUE_LIMIT) zoomy = VALUE_LIMIT;
    zoominv = 65536.0 / (zoomx * zoomx);

    /*
     * Check if we have a rotozoom or just a zoom
     */
    if (fabs(angle) > VALUE_LIMIT) {

	/*
	 * Angle!=0: full rotozoom
	 */
	/*
	 * -----------------------
	 */

	/* Determine target size */
	CRE_GfxRZSurfaceSizeTrig(rz_src->w, rz_src->h, angle, zoomx, zoomy,
        &dstwidth, &dstheight, &canglezoom, &sanglezoom);

	/*
	 * Calculate target factors from sin/cos and zoom
	 */
	sanglezoominv = sanglezoom;
	canglezoominv = canglezoom;
	sanglezoominv *= zoominv;
	canglezoominv *= zoominv;

	/* Calculate half size */
	dstwidthhalf = dstwidth / 2;
	dstheighthalf = dstheight / 2;

	/*
	 * Alloc space to completely contain the rotated surface
	 */
	rz_dst = NULL;
	if (is32bit) {
	    /*
	     * Target surface is 32bit with source RGBA/ABGR ordering
	     */
	    rz_dst =
		SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
				     rz_src->format->Rmask, rz_src->format->Gmask,
				     rz_src->format->Bmask, rz_src->format->Amask);
	} else {
	    /*
	     * Target surface is 8bit
	     */
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0,
            0, 0, 0);
	}

	/*
	 * Lock source surface
	 */
	SDL_LockSurface(rz_src);
	/*
	 * Check which kind of surface we have
	 */
	if (is32bit) {
	    /*
	     * Call the 32bit transformation routine to do the rotation
	     * (using alpha)
	     */
	    CRE_GfxRSurfaceRGBA(rz_src, rz_dst, dstwidthhalf, dstheighthalf,
				 (int) (sanglezoominv), (int) (canglezoominv),
				 flipx, flipy,
				 smooth);
	    /*
	     * Turn on source-alpha support
	     */
	    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
	} else {
	    /*
	     * Copy palette and colorkey info
	     */
	    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
		rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	    }
	    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	    /*
	     * Call the 8bit transformation routine to do the rotation
	     */
	    CRE_GfxRSurfaceY(rz_src, rz_dst, dstwidthhalf, dstheighthalf,
			      (int) (sanglezoominv), (int) (canglezoominv));
	    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL,
            rz_src->format->colorkey);
	}
	/*
	 * Unlock source surface
	 */
	SDL_UnlockSurface(rz_src);

    } else {

	/*
	 * Angle=0: Just a zoom
	 */
	/*
	 * --------------------
	 */

	/*
	 * Calculate target size
	 */
	CRE_GfxZSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth,
        &dstheight);

	/*
	 * Alloc space to completely contain the zoomed surface
	 */
	rz_dst = NULL;
	if (is32bit) {
	    /*
	     * Target surface is 32bit with source RGBA/ABGR ordering
	     */
	    rz_dst =
		SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
				     rz_src->format->Rmask, rz_src->format->Gmask,
				     rz_src->format->Bmask, rz_src->format->Amask);
	} else {
	    /*
	     * Target surface is 8bit
	     */
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0,
            0, 0, 0);
	}

	/*
	 * Lock source surface
	 */
	SDL_LockSurface(rz_src);
	/*
	 * Check which kind of surface we have
	 */
	if (is32bit) {
	    /*
	     * Call the 32bit transformation routine to do the zooming (using alpha)
	     */
	    CRE_GfxZSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);
	    /*
	     * Turn on source-alpha support
	     */
	    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
	} else {
	    /*
	     * Copy palette and colorkey info
	     */
	    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
		rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	    }
	    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	    /*
	     * Call the 8bit transformation routine to do the zooming
	     */
	    CRE_GfxZSurfaceY(rz_src, rz_dst, flipx, flipy);
	    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL,
            rz_src->format->colorkey);
	}
	/*
	 * Unlock source surface
	 */
	SDL_UnlockSurface(rz_src);
    }

    /*
     * Cleanup temp surface
     */
    if (src_converted) {
	SDL_FreeSurface(rz_src);
    }

    /*
     * Return destination surface
     */
    return (rz_dst);
}


/*
 * CRE_GfxZSurfaceSize
 * Funcion que calcula el tamaño de una imagen ampliada
 * By A. Schiffler
 */
void CRE_GfxZSurfaceSize(int width, int height, double zoomx, double zoomy,
    int *dstwidth, int *dstheight)
{
    /*
     * Sanity check zoom factors
     */
    if (zoomx < VALUE_LIMIT) {
	zoomx = VALUE_LIMIT;
    }
    if (zoomy < VALUE_LIMIT) {
	zoomy = VALUE_LIMIT;
    }

    /*
     * Calculate target size
     */
    *dstwidth = (int) ((double) width * zoomx);
    *dstheight = (int) ((double) height * zoomy);
    if (*dstwidth < 1) {
	*dstwidth = 1;
    }
    if (*dstheight < 1) {
	*dstheight = 1;
    }
}


/*
 * CRE_GfxZSurface
 * Función que aplica el zoom a una imagen dada.
 * By A. Schiffler
 */
SDL_Surface *CRE_GfxZSurface(SDL_Surface * src, double zoomx, double zoomy,
    int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    int dstwidth, dstheight;
    int is32bit;
    int i, src_converted;
    int flipx, flipy;

    /*
     * Sanity check
     */
    if (src == NULL)
	return (NULL);

    /*
     * Determine if source surface is 32bit or 8bit
     */
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
	/*
	 * Use source surface 'as is'
	 */
	rz_src = src;
	src_converted = 0;
    } else {
	/*
	 * New source surface is 32bit with a defined RGBA ordering
	 */
	rz_src =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff,
	    0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_BlitSurface(src, NULL, rz_src, NULL);
	src_converted = 1;
	is32bit = 1;
    }

    flipx = (zoomx<0);
    if (flipx) zoomx = -zoomx;
    flipy = (zoomy<0);
    if (flipy) zoomy = -zoomy;

    /* Get size if target */
    CRE_GfxZSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth,
        &dstheight);

    /*
     * Alloc space to completely contain the zoomed surface
     */
    rz_dst = NULL;
    if (is32bit) {
	/*
	 * Target surface is 32bit with source RGBA/ABGR ordering
	 */
	rz_dst =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
				 rz_src->format->Rmask, rz_src->format->Gmask,
				 rz_src->format->Bmask, rz_src->format->Amask);
    } else {
	/*
	 * Target surface is 8bit
	 */
	rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0,
        0, 0);
    }

    /*
     * Lock source surface
     */
    SDL_LockSurface(rz_src);
    /*
     * Check which kind of surface we have
     */
    if (is32bit) {
	/*
	 * Call the 32bit transformation routine to do the zooming (using alpha)
	 */
	CRE_GfxZSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);
	/*
	 * Turn on source-alpha support
	 */
	SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
    } else {
	/*
	 * Copy palette and colorkey info
	 */
	for (i = 0; i < rz_src->format->palette->ncolors; i++) {
	    rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	}
	rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	/*
	 * Call the 8bit transformation routine to do the zooming
	 */
	CRE_GfxZSurfaceY(rz_src, rz_dst, flipx, flipy);
	SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL,
        rz_src->format->colorkey);
    }
    /*
     * Unlock source surface
     */
    SDL_UnlockSurface(rz_src);

    /*
     * Cleanup temp surface
     */
    if (src_converted) {
	SDL_FreeSurface(rz_src);
    }

    /*
     * Return destination surface
     */
    return (rz_dst);
}
