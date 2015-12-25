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

#include "libraw_in.h"
#include <gpac/avparse.h>

enum
{
	LIBRAW_JPEG = 1,
	LIBRAW_PNG,
	LIBRAW_BMP,
	LIBRAW_PNGD,
	LIBRAW_PNGDS,
	LIBRAW_PNGS,
};

typedef struct
{
	GF_ClientService *service;
	/*service*/
	u32 srv_type;

	FILE *stream;
	u32 LIBRAW_type;

	u32 pad_bytes;
	Bool done;
	LPNETCHANNEL ch;

	Bool is_inline;
	char *data;
	u32 data_size;

	GF_SLHeader sl_hdr;

	/*file downloader*/
	GF_DownloadSession * dnload;
} LIBRAWLoader;


GF_ESD *LIBRAW_GetESD(LIBRAWLoader *read)
{
	GF_ESD *esd = gf_odf_desc_esd_new(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = GF_STREAM_VISUAL;
	esd->ESID = 1;

	if (read->LIBRAW_type == LIBRAW_BMP)
		esd->decoderConfig->objectTypeIndication = GPAC_BMP_OTI;
	else {
		u8 OTI=0;
		GF_BitStream *bs = gf_bs_from_file(read->stream, GF_BITSTREAM_READ);
/*#ifndef GPAC_DISABLE_AV_PARSERS
		u32 mtype, w, h;
		gf_LIBRAW_parse(bs, &OTI, &mtype, &w, &h, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
#endif*/
		gf_bs_del(bs);

		if (!OTI) {
			GF_LOG(GF_LOG_WARNING, GF_LOG_CODEC, ("[LIBRAW_IN] Unable to guess format image - assigning from extension\n"));
			if (read->LIBRAW_type==LIBRAW_JPEG) OTI = GPAC_OTI_IMAGE_JPEG;
			else if (read->LIBRAW_type==LIBRAW_PNG) OTI = GPAC_OTI_IMAGE_PNG;
		}
		esd->decoderConfig->objectTypeIndication = OTI;

		if (read->LIBRAW_type == LIBRAW_PNGD) {
			GF_Descriptor *d = gf_odf_desc_new(GF_ODF_AUX_VIDEO_DATA);
			((GF_AuxVideoDescriptor*)d)->aux_video_type = 1;
			gf_list_add(esd->extensionDescriptors, d);
		}
		else if (read->LIBRAW_type == LIBRAW_PNGDS) {
			GF_Descriptor *d = gf_odf_desc_new(GF_ODF_AUX_VIDEO_DATA);
			((GF_AuxVideoDescriptor*)d)->aux_video_type = 2;
			gf_list_add(esd->extensionDescriptors, d);
		}
		else if (read->LIBRAW_type == LIBRAW_PNGS) {
			GF_Descriptor *d = gf_odf_desc_new(GF_ODF_AUX_VIDEO_DATA);
			((GF_AuxVideoDescriptor*)d)->aux_video_type = 3;
			gf_list_add(esd->extensionDescriptors, d);
		}
	}
	return esd;
}

const char * LIBRAW_MIME_TYPES[] = {
	"image/jpeg", "jpeg jpg", "JPEG Images",
	"image/jp2", "jp2", "JPEG2000 Images",
	"image/png", "png", "PNG Images",
	"image/bmp", "bmp", "MS Bitmap Images",
	"image/x-png+depth", "pngd", "PNG+Depth Images",
	"image/x-png+depth+mask", "pngds", "PNG+Depth+Mask Images",
	"image/x-png+stereo", "pngs", "Stereo PNG Images",
	NULL
};

static u32 LIBRAW_RegisterMimeTypes(const GF_InputService *plug) {
	u32 i;
	if (!plug) {
		GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("LIBRAW_RegisterMimeTypes : plug is NULL !!\n"));
	}
	for (i = 0 ; LIBRAW_MIME_TYPES[i]; i+=3)
		gf_service_register_mime(plug, LIBRAW_MIME_TYPES[i], LIBRAW_MIME_TYPES[i+1], LIBRAW_MIME_TYPES[i+2]);
	return i/3;
}


static Bool LIBRAW_CanHandleURL(GF_InputService *plug, const char *url)
{
	char *sExt;
	u32 i;
	GF_LOG(GF_LOG_MEDIA, GF_LOG_INFO, ("LIBRAW_CanHandleURL(%s)\n", url));
	if (!plug || !url)
		return GF_FALSE;
	sExt = strrchr(url, '.');
	for (i = 0 ; LIBRAW_MIME_TYPES[i]; i+=3) {
		if (gf_service_check_mime_register(plug, LIBRAW_MIME_TYPES[i], LIBRAW_MIME_TYPES[i+1], LIBRAW_MIME_TYPES[i+2], sExt))
			return GF_TRUE;
	}
	return GF_FALSE;
}

static Bool jp_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return GF_TRUE;
	if (strstr(url, "://")) return GF_FALSE;
	return GF_TRUE;
}


static void LIBRAW_SetupObject(LIBRAWLoader *read)
{
	if (!read->ch) {
		GF_ObjectDescriptor *od = (GF_ObjectDescriptor *) gf_odf_desc_new(GF_ODF_OD_TAG);
		GF_ESD *esd = LIBRAW_GetESD(read);
		od->objectDescriptorID = 1;
		gf_list_add(od->ESDescriptors, esd);
		gf_service_declare_media(read->service, (GF_Descriptor *)od, GF_FALSE);
	}
}


void LIBRAW_NetIO(void *cbk, GF_NETIO_Parameter *param)
{
	GF_Err e;
	const char *szCache;
	LIBRAWLoader *read = (LIBRAWLoader *) cbk;
	if (!read->dnload) return;

	/*handle service message*/
	gf_service_download_update_stats(read->dnload);

	e = param->error;
	/*wait to get the whole file*/
	if (!e && (param->msg_type!=GF_NETIO_DATA_TRANSFERED)) return;
	if ((e==GF_EOS) && (param->msg_type==GF_NETIO_DATA_EXCHANGE)) return;

	if (param->msg_type==GF_NETIO_DATA_TRANSFERED) {
		szCache = gf_dm_sess_get_cache_name(read->dnload);
		if (!szCache) e = GF_IO_ERR;
		else {
			read->stream = gf_fopen((char *) szCache, "rb");
			if (!read->stream) e = GF_SERVICE_ERROR;
			else {
				e = GF_OK;
				gf_fseek(read->stream, 0, SEEK_END);
				read->data_size = (u32) gf_ftell(read->stream);
				gf_fseek(read->stream, 0, SEEK_SET);
			}
		}
	}
	/*OK confirm*/
	gf_service_connect_ack(read->service, NULL, e);
	if (!e) LIBRAW_SetupObject(read);
}

void jp_download_file(GF_InputService *plug, const char *url)
{
	LIBRAWLoader *read = (LIBRAWLoader *) plug->priv;

	read->dnload = gf_service_download_new(read->service, url, 0, LIBRAW_NetIO, read);
	if (!read->dnload) {
		gf_service_connect_ack(read->service, NULL, GF_NOT_SUPPORTED);
	} else {
		/*start our download (threaded)*/
		gf_dm_sess_process(read->dnload);
	}
	/*service confirm is done once fetched*/
}

static GF_Err LIBRAW_ConnectService(GF_InputService *plug, GF_ClientService *serv, const char *url)
{
	char *sExt;
	LIBRAWLoader *read = (LIBRAWLoader *)plug->priv;

	read->service = serv;
	if (!url)
		return GF_BAD_PARAM;
	sExt = strrchr(url, '.');
	if (!stricmp(sExt, ".jpeg") || !stricmp(sExt, ".jpg")) read->LIBRAW_type = LIBRAW_JPEG;
	else if (!stricmp(sExt, ".png")) read->LIBRAW_type = LIBRAW_PNG;
	else if (!stricmp(sExt, ".pngd")) read->LIBRAW_type = LIBRAW_PNGD;
	else if (!stricmp(sExt, ".pngds")) read->LIBRAW_type = LIBRAW_PNGDS;
	else if (!stricmp(sExt, ".pngs")) read->LIBRAW_type = LIBRAW_PNGS;
	else if (!stricmp(sExt, ".bmp")) read->LIBRAW_type = LIBRAW_BMP;

	if (read->dnload) gf_service_download_del(read->dnload);
	read->dnload = NULL;

	/*remote fetch*/
	if (!jp_is_local(url)) {
		jp_download_file(plug, url);
		return GF_OK;
	}

	read->stream = gf_fopen(url, "rb");
	if (read->stream) {
		gf_fseek(read->stream, 0, SEEK_END);
		read->data_size = (u32) gf_ftell(read->stream);
		gf_fseek(read->stream, 0, SEEK_SET);
	}
	gf_service_connect_ack(serv, NULL, read->stream ? GF_OK : GF_URL_ERROR);
	if (read->stream && read->is_inline) LIBRAW_SetupObject(read);
	return GF_OK;
}

static GF_Err LIBRAW_CloseService(GF_InputService *plug)
{
	LIBRAWLoader *read;
	if (!plug)
		return GF_BAD_PARAM;
	read = (LIBRAWLoader *)plug->priv;
	if (!read)
		return GF_BAD_PARAM;
	if (read->stream) gf_fclose(read->stream);
	read->stream = NULL;
	if (read->dnload) gf_service_download_del(read->dnload);
	read->dnload = NULL;
	if (read->service)
		gf_service_disconnect_ack(read->service, NULL, GF_OK);
	return GF_OK;
}

static GF_Descriptor *LIBRAW_GetServiceDesc(GF_InputService *plug, u32 expect_type, const char *sub_url)
{
	GF_ESD *esd;
	LIBRAWLoader *read = (LIBRAWLoader *)plug->priv;

	/*override default*/
	if (expect_type==GF_MEDIA_OBJECT_UNDEF) expect_type=GF_MEDIA_OBJECT_VIDEO;
	read->srv_type = expect_type;

	/*visual object*/
	if (expect_type==GF_MEDIA_OBJECT_VIDEO) {
		GF_ObjectDescriptor *od = (GF_ObjectDescriptor *) gf_odf_desc_new(GF_ODF_OD_TAG);
		od->objectDescriptorID = 1;
		esd = LIBRAW_GetESD(read);
		gf_list_add(od->ESDescriptors, esd);
		return (GF_Descriptor *) od;
	}
	read->is_inline = GF_TRUE;
	return NULL;
}

static GF_Err LIBRAW_ConnectChannel(GF_InputService *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID=0;
	GF_Err e;
	LIBRAWLoader *read;
	if (!plug)
		return GF_OK;
	read = (LIBRAWLoader *)plug->priv;

	e = GF_SERVICE_ERROR;
	if (read->ch==channel) goto exit;
	if (!url)
		goto exit;
	e = GF_STREAM_NOT_FOUND;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%ud", &ES_ID);
	}
	/*URL setup*/
	else if (!read->ch && LIBRAW_CanHandleURL(plug, url)) ES_ID = 1;

	if (ES_ID==1) {
		read->ch = channel;
		e = GF_OK;
	}

exit:
	gf_service_connect_ack(read->service, channel, e);
	return e;
}

static GF_Err LIBRAW_DisconnectChannel(GF_InputService *plug, LPNETCHANNEL channel)
{
	GF_Err e = GF_STREAM_NOT_FOUND;
	LIBRAWLoader *read = (LIBRAWLoader *)plug->priv;

	if (read->ch == channel) {
		read->ch = NULL;
		e = GF_OK;
	}
	gf_service_disconnect_ack(read->service, channel, e);
	return GF_OK;
}

static GF_Err LIBRAW_ServiceCommand(GF_InputService *plug, GF_NetworkCommand *com)
{
	LIBRAWLoader *read = (LIBRAWLoader *)plug->priv;

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
			read->done = GF_FALSE;
		}
		return GF_OK;
	case GF_NET_CHAN_STOP:
		return GF_OK;
	default:
		return GF_OK;
	}
}


static GF_Err LIBRAW_ChannelGetSLP(GF_InputService *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, GF_SLHeader *out_sl_hdr, Bool *sl_compressed, GF_Err *out_reception_status, Bool *is_new_data)
{
	LIBRAWLoader *read = (LIBRAWLoader *)plug->priv;

	*out_reception_status = GF_OK;
	*sl_compressed = GF_FALSE;
	*is_new_data = GF_FALSE;

	memset(&read->sl_hdr, 0, sizeof(GF_SLHeader));
	read->sl_hdr.randomAccessPointFlag = 1;
	read->sl_hdr.compositionTimeStampFlag = 1;
	*out_sl_hdr = read->sl_hdr;

	/*fetching es data*/
	if (read->ch == channel) {
		if (read->done) {
			*out_reception_status = GF_EOS;
			return GF_OK;
		}
		if (!read->data) {
			if (!read->stream) {
				*out_data_ptr = NULL;
				*out_data_size = 0;
				return GF_OK;
			}
			*is_new_data = GF_TRUE;
			gf_fseek(read->stream, 0, SEEK_SET);
			read->data = (char*) gf_malloc(sizeof(char) * (read->data_size + read->pad_bytes));
			read->data_size = (u32) fread(read->data, sizeof(char), read->data_size, read->stream);
			gf_fseek(read->stream, 0, SEEK_SET);
			if (read->pad_bytes) memset(read->data + read->data_size, 0, sizeof(char) * read->pad_bytes);

		}
		*out_data_ptr = read->data;
		*out_data_size = read->data_size;
		return GF_OK;
	}
	return GF_STREAM_NOT_FOUND;
}

static GF_Err LIBRAW_ChannelReleaseSLP(GF_InputService *plug, LPNETCHANNEL channel)
{
	LIBRAWLoader *read = (LIBRAWLoader *)plug->priv;

	if (read->ch == channel) {
		if (!read->data) return GF_BAD_PARAM;
		gf_free(read->data);
		read->data = NULL;
		read->done = GF_TRUE;
		return GF_OK;
	}
	return GF_OK;
}


void *NewLoaderInterface()
{
	LIBRAWLoader *priv;
	GF_InputService *plug;
	GF_SAFEALLOC(plug, GF_InputService);
	GF_REGISTER_MODULE_INTERFACE(plug, GF_NET_CLIENT_INTERFACE, "GPAC Image Reader", "gpac distribution")

	plug->RegisterMimeTypes = LIBRAW_RegisterMimeTypes;
	plug->CanHandleURL = LIBRAW_CanHandleURL;
	plug->CanHandleURLInService = NULL;
	plug->ConnectService = LIBRAW_ConnectService;
	plug->CloseService = LIBRAW_CloseService;
	plug->GetServiceDescriptor = LIBRAW_GetServiceDesc;
	plug->ConnectChannel = LIBRAW_ConnectChannel;
	plug->DisconnectChannel = LIBRAW_DisconnectChannel;
	plug->ChannelGetSLP = LIBRAW_ChannelGetSLP;
	plug->ChannelReleaseSLP = LIBRAW_ChannelReleaseSLP;
	plug->ServiceCommand = LIBRAW_ServiceCommand;

	GF_SAFEALLOC(priv, LIBRAWLoader);
	plug->priv = priv;
	return plug;
}

void DeleteLoaderInterface(void *ifce)
{
	LIBRAWLoader *read;
	GF_InputService *plug = (GF_InputService *) ifce;
	GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("DeleteLoaderInterface : 1\n"));
	if (!plug)
		return;
	read = (LIBRAWLoader *)plug->priv;
	if (read)
		gf_free(read);
	plug->priv = NULL;
	gf_free(plug);
	GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("DeleteLoaderInterface : 2\n"));
}
