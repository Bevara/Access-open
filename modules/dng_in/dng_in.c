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

#include "dng_in.h"
#include <gpac/avparse.h>
#include "libraw/libraw.h"

enum
{
	IMG_JPEG = 1,
	IMG_PNG,
	IMG_BMP,
	IMG_PNGD,
	IMG_PNGDS,
	IMG_PNGS,
	IMG_DNG,
};

typedef struct
{
	GF_ClientService *service;
	/*service*/
	u32 srv_type;

	FILE *stream;
	u32 img_type;

	u32 pad_bytes;

	LPNETCHANNEL ch;

	Bool is_inline;

	GF_SLHeader sl_hdr;

	/* dng handler*/
	libraw_data_t *iprc;

	/* raw data */
	LPNETCHANNEL ch_raw;
	char *raw_data;
	u32 raw_data_size;
	Bool raw_done;

	/* thumb data */
	LPNETCHANNEL ch_thumb;
	char *thumb_data;
	u32 thumb_data_size;
	Bool thumb_done;

} IMGLoader;


const char * DNG_MIME_TYPES[] = {
	"image/dng", "dng", "DNG Images",
	NULL
};

static u32 DNG_RegisterMimeTypes(const GF_InputService *plug) {
	u32 i;
	if (!plug) {
		GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("IMG_RegisterMimeTypes : plug is NULL !!\n"));
	}
	for (i = 0 ; DNG_MIME_TYPES[i]; i+=3)
		gf_service_register_mime(plug, DNG_MIME_TYPES[i], DNG_MIME_TYPES[i+1], DNG_MIME_TYPES[i+2]);
	return i/3;
}


static Bool DNG_CanHandleURL(GF_InputService *plug, const char *url)
{
	char *sExt;
	u32 i;
	GF_LOG(GF_LOG_MEDIA, GF_LOG_INFO, ("IMG_CanHandleURL(%s)\n", url));
	if (!plug || !url)
		return GF_FALSE;
	sExt = strrchr(url, '.');
	for (i = 0 ; DNG_MIME_TYPES[i]; i+=3) {
		if (gf_service_check_mime_register(plug, DNG_MIME_TYPES[i], DNG_MIME_TYPES[i+1], DNG_MIME_TYPES[i+2], sExt))
			return GF_TRUE;
	}
	return GF_FALSE;
}

static GF_ESD*  DNG_SetupRAW(libraw_data_t *iprc)
{
	GF_ESD *esd;

	esd = gf_odf_desc_esd_new(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = GF_STREAM_VISUAL;
	esd->ESID = 1;	
	esd->decoderConfig->objectTypeIndication = GPAC_OTI_IMAGE_DNG;

	return esd;
}

static GF_ESD* DNG_GetThumbESD(libraw_thumbnail_t *thumb)
{
	GF_ESD *esd = gf_odf_desc_esd_new(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = GF_STREAM_THUMBNAIL;
	esd->ESID = 2;

	switch (thumb->tformat)
	{
	case LIBRAW_THUMBNAIL_JPEG:
		esd->decoderConfig->objectTypeIndication = GPAC_OTI_IMAGE_JPEG;
		break;

	case LIBRAW_THUMBNAIL_BITMAP:
		esd->decoderConfig->objectTypeIndication = GPAC_BMP_OTI;
		break;

	default:
		break;
	}

	return esd;
}


static void DNG_SetupObject(IMGLoader *read)
{
	GF_ESD *esd;
	GF_ObjectDescriptor *od;
	u32 ret = 0;

	/* Setup raw image */
	ret = libraw_unpack(read->iprc);
	if (ret) {
		fprintf(stderr, "libraw : error decoding raw image (%s) \n", libraw_strerror(ret));
	}

	ret = libraw_dcraw_process(read->iprc);
	if (ret) {
		fprintf(stderr, "libraw : error decoding raw image (%s) \n", libraw_strerror(ret));
	}

	od = (GF_ObjectDescriptor *)gf_odf_desc_new(GF_ODF_OD_TAG);
	esd = DNG_SetupRAW(read->iprc);
	od->objectDescriptorID = esd->ESID;
	gf_list_add(od->ESDescriptors, esd);
	gf_service_declare_media(read->service, (GF_Descriptor*)od, GF_FALSE);

	/* Setup thumbnail */
	ret = libraw_unpack_thumb(read->iprc);
	if (ret) {
		fprintf(stderr, "libraw : error decoding thumbnail (%s) \n", libraw_strerror(ret));
	}
	od = (GF_ObjectDescriptor *)gf_odf_desc_new(GF_ODF_OD_TAG);
	esd = DNG_GetThumbESD(&read->iprc->thumbnail);
	od->objectDescriptorID = esd->ESID;
	gf_list_add(od->ESDescriptors, esd);
	gf_service_declare_media(read->service, (GF_Descriptor*)od, GF_TRUE);
}

static GF_Err DNG_ConnectService(GF_InputService *plug, GF_ClientService *serv, const char *url)
{
	IMGLoader *read = (IMGLoader *)plug->priv;
	int ret;
	GF_Err err = GF_OK;

	read->service = serv;
	if (!url)
		return GF_BAD_PARAM;

	ret = libraw_open_file(read->iprc, url);
	if (ret) {
		fprintf(stderr, "libraw  %s\n", libraw_strerror(ret));
		err = GF_NOT_SUPPORTED;
	}
	
	gf_service_connect_ack(serv, NULL, err);
	if (ret != 0)
		return err;

	DNG_SetupObject(read);
	return err;
}

static GF_Err IMG_CloseService(GF_InputService *plug)
{
	IMGLoader *read;
	if (!plug)
		return GF_BAD_PARAM;
	read = (IMGLoader *)plug->priv;
	if (!read)
		return GF_BAD_PARAM;
	if (read->stream) gf_fclose(read->stream);
	read->stream = NULL;
	if (read->service)
		gf_service_disconnect_ack(read->service, NULL, GF_OK);
	return GF_OK;
}

static GF_Descriptor *IMG_GetServiceDesc(GF_InputService *plug, u32 expect_type, const char *sub_url)
{
	GF_ESD *esd;
	IMGLoader *read = (IMGLoader *)plug->priv;

	/*override default*/
	if (expect_type==GF_MEDIA_OBJECT_UNDEF) expect_type=GF_MEDIA_OBJECT_VIDEO;
	read->srv_type = expect_type;

	/*visual object*/
	/*if (expect_type==GF_MEDIA_OBJECT_VIDEO) {
		GF_ObjectDescriptor *od = (GF_ObjectDescriptor *) gf_odf_desc_new(GF_ODF_OD_TAG);
		od->objectDescriptorID = 1;
		esd = IMG_GetESD(read);
		gf_list_add(od->ESDescriptors, esd);
		return (GF_Descriptor *) od;
	}*/
	read->is_inline = GF_TRUE;
	return NULL;
}

static GF_Err DNG_ConnectChannel(GF_InputService *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID=0;
	GF_Err e;
	IMGLoader *read;
	if (!plug)
		return GF_OK;
	read = (IMGLoader *)plug->priv;

	e = GF_SERVICE_ERROR;
	if (read->ch==channel) goto exit;
	if (!url)
		goto exit;
	e = GF_STREAM_NOT_FOUND;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%ud", &ES_ID);
	}
	/*URL setup*/
	else if (!read->ch && DNG_CanHandleURL(plug, url)) ES_ID = 1;

	if (ES_ID==1) {
		read->ch_raw = channel;
		e = GF_OK;
	}
	else if (ES_ID == 1) {
		read->ch_thumb = channel;
		e = GF_OK;
	}

exit:
	gf_service_connect_ack(read->service, channel, e);
	return e;
}

static GF_Err DNG_DisconnectChannel(GF_InputService *plug, LPNETCHANNEL channel)
{
	GF_Err e = GF_STREAM_NOT_FOUND;
	IMGLoader *read = (IMGLoader *)plug->priv;

	if (read->ch_raw == channel) {
		read->ch_raw = NULL;
		e = GF_OK;
	}

	if (read->ch_thumb == channel) {
		read->ch_thumb = NULL;
		e = GF_OK;
	}

	gf_service_disconnect_ack(read->service, channel, e);
	return GF_OK;
}

static GF_Err IMG_ServiceCommand(GF_InputService *plug, GF_NetworkCommand *com)
{
	IMGLoader *read = (IMGLoader *)plug->priv;

	if (com->command_type==GF_NET_SERVICE_HAS_AUDIO) return GF_NOT_SUPPORTED;

	if (!com->base.on_channel) return GF_NOT_SUPPORTED;
	switch (com->command_type) {
	case GF_NET_CHAN_SET_PADDING:
		read->pad_bytes = com->pad.padding_bytes;
		return GF_OK;
	case GF_NET_CHAN_DURATION:
		com->duration.duration = 0;
		return GF_OK;
	case GF_NET_CHAN_PLAY:
		/*note we don't handle range since we're only dealing with images*/
		if (read->ch == com->base.on_channel) {
			read->raw_done = GF_FALSE;
			read->thumb_done = GF_FALSE;
		}
		return GF_OK;
	case GF_NET_CHAN_STOP:
		return GF_OK;
	default:
		return GF_OK;
	}
}


static GF_Err IMG_ChannelGetSLP(GF_InputService *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, GF_SLHeader *out_sl_hdr, Bool *sl_compressed, GF_Err *out_reception_status, Bool *is_new_data)
{
	IMGLoader *read = (IMGLoader *)plug->priv;

	*out_reception_status = GF_OK;
	*sl_compressed = GF_FALSE;
	*is_new_data = GF_FALSE;

	memset(&read->sl_hdr, 0, sizeof(GF_SLHeader));
	read->sl_hdr.randomAccessPointFlag = 1;
	read->sl_hdr.compositionTimeStampFlag = 1;
	*out_sl_hdr = read->sl_hdr;


	/*fetching raw data*/
	if (read->ch_raw == channel) {
		libraw_data_t * iprc = read->iprc;

		if (read->raw_done) {
			*out_reception_status = GF_EOS;
			return GF_OK;
		}
		if (!read->raw_data) {
			if (!read->iprc) {
				*out_data_ptr = NULL;
				*out_data_size = 0;
				return GF_OK;
			}
			*is_new_data = GF_TRUE;
			
			//read->raw_data_size = thumb->tlength;
			//read->raw_data = (char*)gf_malloc(sizeof(char) * read->thumb_data_size);
			//memcpy(read->raw_data, thumb->thumb, read->raw_data_size);

		}
		*out_data_ptr = read->thumb_data;
		*out_data_size = read->thumb_data_size;
		return GF_OK;
	}

	/*fetching thumb data*/
	if (read->ch_thumb == channel) {
		libraw_data_t * iprc = read->iprc;
		libraw_thumbnail_t* thumb = &iprc->thumbnail;

		if (read->thumb_done) {
			*out_reception_status = GF_EOS;
			return GF_OK;
		}
		if (!read->thumb_data) {
			if (!read->iprc) {
				*out_data_ptr = NULL;
				*out_data_size = 0;
				return GF_OK;
			}
			*is_new_data = GF_TRUE;
			read->thumb_data_size = thumb->tlength;
			read->thumb_data = (char*) gf_malloc(sizeof(char) * read->thumb_data_size);
			memcpy(read->thumb_data, thumb->thumb, read->thumb_data_size);

		}
		*out_data_ptr = read->thumb_data;
		*out_data_size = read->thumb_data_size;
		return GF_OK;
	}
	return GF_STREAM_NOT_FOUND;
}

static GF_Err IMG_ChannelReleaseSLP(GF_InputService *plug, LPNETCHANNEL channel)
{
	IMGLoader *read = (IMGLoader *)plug->priv;

	if (read->ch_raw == channel) {
		if (!read->raw_data) return GF_BAD_PARAM;
		gf_free(read->raw_data);
		read->raw_data = NULL;
		read->raw_done = GF_TRUE;
		return GF_OK;
	}
	
	if (read->ch_thumb == channel) {
		if (!read->thumb_data) return GF_BAD_PARAM;
		gf_free(read->thumb_data);
		read->thumb_data = NULL;
		read->thumb_done = GF_TRUE;
		return GF_OK;

	}
	return GF_OK;
}


void *NewLoaderInterface()
{
	IMGLoader *priv;
	GF_InputService *plug;
	GF_SAFEALLOC(plug, GF_InputService);
	GF_REGISTER_MODULE_INTERFACE(plug, GF_NET_CLIENT_INTERFACE, "GPAC RAW Image Reader", "Bevara technologies")

	plug->RegisterMimeTypes = DNG_RegisterMimeTypes;
	plug->CanHandleURL = DNG_CanHandleURL;
	plug->CanHandleURLInService = NULL;
	plug->ConnectService = DNG_ConnectService;
	plug->CloseService = IMG_CloseService;
	plug->GetServiceDescriptor = IMG_GetServiceDesc;
	plug->ConnectChannel = DNG_ConnectChannel;
	plug->DisconnectChannel = DNG_DisconnectChannel;
	plug->ChannelGetSLP = IMG_ChannelGetSLP;
	plug->ChannelReleaseSLP = IMG_ChannelReleaseSLP;
	plug->ServiceCommand = IMG_ServiceCommand;

	GF_SAFEALLOC(priv, IMGLoader);
	priv->iprc = libraw_init(0);
	plug->priv = priv;
	return plug;
}

void DeleteLoaderInterface(void *ifce)
{
	IMGLoader *read;
	GF_InputService *plug = (GF_InputService *) ifce;
	GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("DeleteLoaderInterface : 1\n"));
	if (!plug)
		return;
	read = (IMGLoader *)plug->priv;
	if (read) {
		libraw_close(read->iprc);
		gf_free(read);
	}
		
	plug->priv = NULL;
	gf_free(plug);
	GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("DeleteLoaderInterface : 2\n"));
}


GPAC_MODULE_EXPORT
const u32 *QueryInterfaces()
{
	static u32 si[] = {
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
		DeleteRAWDec((GF_BaseDecoder *)ifce);
		break;
	case GF_NET_CLIENT_INTERFACE:
		DeleteLoaderInterface(ifce);
		break;
	}
}

GPAC_MODULE_STATIC_DECLARATION(dng_in)