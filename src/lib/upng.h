/*
uPNG -- derived from LodePNG version 20100808

Copyright (c) 2005-2010 Lode Vandevenne
Copyright (c) 2010 Sean Middleditch

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.

		2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		distribution.
*/


#ifndef UPNG_H
	#define UPNG_H
	#include <Arduino.h>
	#include <FS.h>
	#include "SPIFFS.h"

		struct upng_s_rgb16b{
			int r:5;
			int g:6;
			int b:5;
			int null : 16;
		};
		struct upng_s_rgb18b{
			int r:6;
			int g:6;
			int b:6;
			int null:14;
		};
		struct upng_s_rgb24b{
			int r:8;
			int g:8;
			int b:8;
			int null:8;
		};

		struct upng_s_rgba32b{
			upng_s_rgb24b rgb;
			byte alpha;
		};

		typedef enum upng_color_type {
			UPNG_CT_PALETTE	= 1,
			UPNG_CT_COLOR	= 2,
			UPNG_CT_ALPHA	= 4
		} upng_color_type;

		typedef enum upng_error {
			UPNG_EOK			= 0, /* success (no error) */
			UPNG_ENOMEM			= 1, /* memory allocation failed */
			UPNG_ENOTFOUND		= 2, /* resource not found (file missing) */
			UPNG_ENOTPNG		= 3, /* image data does not have a PNG header */
			UPNG_EMALFORMED		= 4, /* image data is not a valid PNG image */
			UPNG_EUNSUPPORTED	= 5, /* critical PNG chunk type is not supported */
			UPNG_EUNINTERLACED	= 6, /* image interlacing is not supported */
			UPNG_EUNFORMAT		= 7, /* image color format is not supported */
			UPNG_EPARAM			= 8  /* invalid parameter to method call */
		} upng_error;

		typedef enum upng_format {
			UPNG_BADFORMAT,
			UPNG_RGB8,
			UPNG_RGB16,
			UPNG_RGBA8,
			UPNG_RGBA16,
			UPNG_LUMINANCE1,
			UPNG_LUMINANCE2,
			UPNG_LUMINANCE4,
			UPNG_LUMINANCE8,
			UPNG_LUMINANCE_ALPHA1,
			UPNG_LUMINANCE_ALPHA2,
			UPNG_LUMINANCE_ALPHA4,
			UPNG_LUMINANCE_ALPHA8,
			UPNG_PALLETTE1,
			UPNG_PALLETTE2,
			UPNG_PALLETTE4,
			UPNG_PALLETTE8
		} upng_format;

		typedef struct upng_t upng_t;

		upng_t*		upng_new_from_bytes	(const unsigned char* buffer, unsigned long size);
		upng_t*		upng_new_from_file	(const char* path);
		void		upng_free			(upng_t* upng);

		upng_error	upng_header			(upng_t* upng);
		upng_error	upng_decode			(upng_t* upng);

		upng_error	upng_get_error		(const upng_t* upng);
		unsigned	upng_get_error_line	(const upng_t* upng);

		unsigned	upng_get_width		(const upng_t* upng);
		unsigned	upng_get_height		(const upng_t* upng);
		unsigned	upng_get_bpp		(const upng_t* upng);
		unsigned	upng_get_bitdepth	(const upng_t* upng);
		unsigned	upng_get_components	(const upng_t* upng);
		unsigned	upng_get_pixelsize	(const upng_t* upng);
		upng_format	upng_get_format		(const upng_t* upng);

		const unsigned char*	upng_get_buffer		(const upng_t* upng);
		unsigned				upng_get_size		(const upng_t* upng);

		void upng_GetPixel(void* pixel, const upng_t* upng, int x, int y); //Get pixel info from buffer

		upng_s_rgb16b* InitColorR5G6B5();
		upng_s_rgb18b* InitColorR6G6B6();
		upng_s_rgb24b* InitColorR8G8B8();
		void InitColor(upng_s_rgb16b **dst);
		void InitColor(upng_s_rgb18b **dst);
		void InitColor(upng_s_rgb24b **dst);
		void ResetColor(upng_s_rgb16b *dst);
		void ResetColor(upng_s_rgb18b *dst);
		void ResetColor(upng_s_rgb24b *dst);
		void upng_rgb24bto18b(upng_s_rgb18b *dst, upng_s_rgb24b *src); //Converts 24bit-color(r8,g8,b8) into 18bit-color(r6,g6,b6)
		void upng_rgb24bto16b(upng_s_rgb16b *dst, upng_s_rgb24b *src); //Converts 24bit-color(r8,g8,b8) into 16bit-color(r5,g6,b5)
		void upng_rgb18btouint32(uint32_t *dst, upng_s_rgb18b *src);
		void upng_rgb16btouint32(uint32_t *dst, upng_s_rgb16b *src);

//	#endif /*defined(UPNG_H)*/

#endif
