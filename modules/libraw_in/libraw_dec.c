/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean Le Feuvre
 *			Copyright (c) Telecom ParisTech 2000-2012
 *					All rights reserved
 *
 *  This file is part of GPAC / image format module
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

 /*all codecs are regular */
#include <gpac/modules/codec.h>
#include <gpac/modules/service.h>
#include <gpac/constants.h>

#include "libraw/libraw.h"

#define RAWCTX()	LIBRAWDec *ctx = (LIBRAWDec *) ifcg->privateStack

enum
{
	IMG_JPEG = 1,
	IMG_PNG,
	IMG_BMP,
	IMG_PNGD,
	IMG_PNGDS,
	IMG_PNGS,
};

typedef struct
{
	u32 type;
	u32 bpp, nb_comp, width, height, out_size, pixel_format, dsi_size;
	void *opaque;
	libraw_data_t *iprc;
} LIBRAWDec;


static u32 LIBRAW_CanHandleStream(GF_BaseDecoder *ifcg, u32 StreamType, GF_ESD *esd, u8 PL)
{
	if (StreamType != GF_STREAM_VISUAL) return GF_CODEC_NOT_SUPPORTED;
	/*media type query*/
	if (!esd) return GF_CODEC_STREAM_TYPE_SUPPORTED;
	RAWCTX();

	ctx->iprc = libraw_init(0);


/*	switch (esd->decoderConfig->objectTypeIndication) {
#ifdef GPAC_HAS_PNG
	case GPAC_OTI_IMAGE_PNG:
		if (NewPNGDec(dec)) return esd->decoderConfig->bvr_config? GF_CODEC_MAYBE_SUPPORTED : GF_CODEC_SUPPORTED;
		return GF_CODEC_NOT_SUPPORTED;
#endif
#ifdef GPAC_HAS_JPEG
	case GPAC_OTI_IMAGE_JPEG:
		if (NewJPEGDec(dec)) return esd->decoderConfig->bvr_config? GF_CODEC_MAYBE_SUPPORTED : GF_CODEC_SUPPORTED;
		return GF_CODEC_NOT_SUPPORTED;
#endif
#ifdef GPAC_HAS_JP2
	case GPAC_OTI_IMAGE_JPEG_2000:
		if (NewJP2Dec(dec)) return esd->decoderConfig->bvr_config? GF_CODEC_MAYBE_SUPPORTED : GF_CODEC_SUPPORTED;
		return GF_CODEC_NOT_SUPPORTED;
#endif
	case GPAC_BMP_OTI:
		if (NewBMPDec(dec)) return esd->decoderConfig->bvr_config? GF_CODEC_MAYBE_SUPPORTED : GF_CODEC_SUPPORTED;
		return GF_CODEC_NOT_SUPPORTED;

	default:
#ifdef GPAC_HAS_JP2
	{
		char *dsi = esd->decoderConfig->decoderSpecificInfo ? esd->decoderConfig->decoderSpecificInfo->data : NULL;
		if (dsi && (dsi[0]=='m') && (dsi[1]=='j') && (dsi[2]=='p') && (dsi[3]=='2'))
			if (NewJP2Dec(dec)) return esd->decoderConfig->bvr_config? GF_CODEC_MAYBE_SUPPORTED : GF_CODEC_SUPPORTED;
		return GF_CODEC_NOT_SUPPORTED;
	}
#endif
	return GF_CODEC_NOT_SUPPORTED;
	}*/
	return GF_CODEC_NOT_SUPPORTED;
}


static GF_Err LIBRAW_AttachStream(GF_BaseDecoder *ifcg, GF_ESD *esd)
{

	return GF_OK;
}
static GF_Err LIBRAW_DetachStream(GF_BaseDecoder *ifcg, u16 ES_ID)
{
	return GF_OK;
}
static GF_Err LIBRAW_GetCapabilities(GF_BaseDecoder *ifcg, GF_CodecCapability *capability)
{
	RAWCTX();
	switch (capability->CapCode) {
	case GF_CODEC_WIDTH:
		capability->cap.valueInt = ctx->width;
		break;
	case GF_CODEC_HEIGHT:
		capability->cap.valueInt = ctx->height;
		break;
	case GF_CODEC_STRIDE:
		if (ctx->pixel_format == GF_PIXEL_YV12) {
			capability->cap.valueInt = ctx->width;
		}
		else {
			capability->cap.valueInt = ctx->width * ctx->nb_comp;
		}
		break;
	case GF_CODEC_FPS:
		capability->cap.valueFloat = 0;
		break;
	case GF_CODEC_PIXEL_FORMAT:
		capability->cap.valueInt = ctx->pixel_format;
		break;
	case GF_CODEC_OUTPUT_SIZE:
		capability->cap.valueInt = ctx->out_size;
		break;
	case GF_CODEC_BUFFER_MIN:
		capability->cap.valueInt = 0;
		break;
	case GF_CODEC_BUFFER_MAX:
		capability->cap.valueInt = (ctx->pixel_format == GF_PIXEL_YV12) ? 4 : 1;
		break;
	case GF_CODEC_PADDING_BYTES:
		capability->cap.valueInt = 4;
		break;
	case GF_CODEC_PAR:
		capability->cap.valueInt = 0;
		break;
	default:
		capability->cap.valueInt = 0;
		return GF_NOT_SUPPORTED;
	}
	return GF_OK;
}
static GF_Err LIBRAW_SetCapabilities(GF_BaseDecoder *ifcg, GF_CodecCapability capability)
{
	/*return unsupported to avoid confusion by the player (like color space changing ...) */
	return GF_NOT_SUPPORTED;
}

static const char *LIBRAW_GetCodecName(GF_BaseDecoder *dec)
{
	return "LibRaw";
}

static GF_Err LIBRAW_ProcessData(GF_MediaDecoder *ifcg,
	char *inBuffer, u32 inBufferLength,
	u16 ES_ID, u32 *CTS,
	char *outBuffer, u32 *outBufferLength,
	u8 PaddingBits, u32 mmlevel)
{

}

GF_BaseDecoder *NewBaseDecoder()
{
	GF_MediaDecoder *ifce;
	LIBRAWDec *wrap;
	GF_SAFEALLOC(ifce, GF_MediaDecoder);
	if (!ifce) return NULL;
	GF_SAFEALLOC(wrap, LIBRAWDec);
	if (!wrap) {
		gf_free(ifce);
		return NULL;
	}
	ifce->privateStack = wrap;
	ifce->CanHandleStream = LIBRAW_CanHandleStream;
	ifce->AttachStream = LIBRAW_AttachStream;
	ifce->DetachStream = LIBRAW_DetachStream;
	ifce->GetCapabilities = LIBRAW_GetCapabilities;
	ifce->SetCapabilities = LIBRAW_SetCapabilities;
	ifce->GetName = LIBRAW_GetCodecName;
	ifce->ProcessData = LIBRAW_ProcessData;

	GF_REGISTER_MODULE_INTERFACE(ifce, GF_MEDIA_DECODER_INTERFACE, "GPAC Raw Image Decoder", "Bevara distribution")

	/*other interfaces will be setup at run time*/
	return (GF_BaseDecoder *)ifce;
}

void DeleteBaseDecoder(GF_BaseDecoder *ifcd)
{
	LIBRAWDec *wrap;
	if (!ifcd)
		return;
	wrap = (LIBRAWDec *)ifcd->privateStack;
	if (!wrap)
		return;
	switch (wrap->type) {
/*	case DEC_PNG:
		DeletePNGDec(ifcd);
		break;
	case DEC_JPEG:
		DeleteJPEGDec(ifcd);
		break;
#ifdef GPAC_HAS_JP2
	case DEC_JP2:
		DeleteJP2Dec(ifcd);
		break;
#endif
	case DEC_BMP:
		DeleteBMPDec(ifcd);
		break;*/
	default:
		break;
	}
	gf_free(wrap);
	ifcd->privateStack = NULL;
	gf_free(ifcd);
}

GPAC_MODULE_EXPORT
const u32 *QueryInterfaces()
{
	static u32 si [] = {
		GF_MEDIA_DECODER_INTERFACE,
		GF_NET_CLIENT_INTERFACE,
		0
	};
	return si;
}

GPAC_MODULE_EXPORT
GF_BaseInterface *LoadInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case GF_MEDIA_DECODER_INTERFACE:
		return (GF_BaseInterface *)NewBaseDecoder();
	case GF_NET_CLIENT_INTERFACE:
		return (GF_BaseInterface *)NewLoaderInterface();
	default:
		return NULL;
	}
}

GPAC_MODULE_EXPORT
void ShutdownInterface(GF_BaseInterface *ifce)
{
	switch (ifce->InterfaceType) {
	case GF_MEDIA_DECODER_INTERFACE:
		DeleteBaseDecoder((GF_BaseDecoder *)ifce);
		break;
	case GF_NET_CLIENT_INTERFACE:
		DeleteLoaderInterface(ifce);
		break;
	}
}

GPAC_MODULE_STATIC_DECLARATION( libraw_in )
