
#include <gpac/modules/codec.h>
#include <gpac/constants.h>

#include "libraw/libraw.h"

#define RAWCTX()	RAWDec *ctx = (RAWDec *) ifcg->privateStack

typedef struct
{
	u16 ES_ID;
	u32 width, height, out_size, pixel_format;
	u8 BPP;
} RAWDec;

static u32 RAW_CanHandleStream(GF_BaseDecoder *dec, u32 StreamType, GF_ESD *esd, u8 PL)
{
	if (StreamType != GF_STREAM_VISUAL) return GF_CODEC_NOT_SUPPORTED;
	/*media type query*/
	if (!esd) return GF_CODEC_STREAM_TYPE_SUPPORTED;

	if (esd->decoderConfig->objectTypeIndication == GPAC_OTI_IMAGE_RAW
		&& esd->decoderConfig->decoderSpecificInfo
		&& esd->decoderConfig->decoderSpecificInfo->dataLength)
		return GF_CODEC_SUPPORTED;

	return GF_CODEC_NOT_SUPPORTED;
}

static GF_Err RAW_AttachStream(GF_BaseDecoder *ifcg, GF_ESD *esd)
{
	GF_BitStream *bs;
	RAWCTX();

	if (ctx->ES_ID && ctx->ES_ID != esd->ESID) return GF_NOT_SUPPORTED;

	if (!esd->decoderConfig->decoderSpecificInfo || !esd->decoderConfig->decoderSpecificInfo->dataLength) return GF_NON_COMPLIANT_BITSTREAM;

	bs = gf_bs_new(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, GF_BITSTREAM_READ);
	ctx->height = gf_bs_read_u32(bs);
	ctx->width = gf_bs_read_u32(bs);
	ctx->pixel_format = gf_bs_read_u32(bs);
	ctx->BPP = gf_bs_read_u8(bs);
	gf_bs_del(bs);

	ctx->ES_ID = esd->ESID;
	return GF_OK;
}
static GF_Err RAW_DetachStream(GF_BaseDecoder *ifcg, u16 ES_ID)
{
	RAWCTX();
	if (ctx->ES_ID != ES_ID) return GF_BAD_PARAM;
	ctx->ES_ID = ES_ID;
	return GF_OK;
}
static GF_Err RAW_GetCapabilities(GF_BaseDecoder *ifcg, GF_CodecCapability *capability)
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
		capability->cap.valueInt = ctx->width * ctx->BPP;
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
		capability->cap.valueInt = 1;
		break;
	case GF_CODEC_PADDING_BYTES:
		capability->cap.valueInt = 4;
		break;
	case GF_CODEC_PAR:
		capability->cap.valueInt = 0;
		break;
	case GF_CODEC_REORDER:
		capability->cap.valueInt = 0;
		break;
	default:
		capability->cap.valueInt = 0;
		return GF_NOT_SUPPORTED;
	}
	return GF_OK;
}
static GF_Err RAW_SetCapabilities(GF_BaseDecoder *ifcg, GF_CodecCapability capability)
{
	/*return unsupported to avoid confusion by the player (like color space changing ...) */
	return GF_NOT_SUPPORTED;
}

static const char *RAW_GetCodecName(GF_BaseDecoder *dec)
{
	return "Raw decoder";
}

static GF_Err RAW_ProcessData(GF_MediaDecoder *ifcg,
	char *inBuffer, u32 inBufferLength,
	u16 ES_ID, u32 *CTS,
	char *outBuffer, u32 *outBufferLength,
	u8 PaddingBits, u32 mmlevel)
{
	RAWCTX();

	if (*outBufferLength < inBufferLength) {
		ctx->out_size = inBufferLength;
		*outBufferLength = inBufferLength;
		return GF_BUFFER_TOO_SMALL;
	}

	memcpy(outBuffer, inBuffer, ctx->out_size);
	return GF_OK;
}

GF_BaseDecoder *NewRAWDec()
{
	GF_MediaDecoder *ifce;
	RAWDec *dec;

	GF_SAFEALLOC(ifce, GF_MediaDecoder);
	GF_SAFEALLOC(dec, RAWDec);

	/*setup our own interface*/
	ifce->AttachStream = RAW_AttachStream;
	ifce->DetachStream = RAW_DetachStream;
	ifce->CanHandleStream = RAW_CanHandleStream;
	ifce->GetCapabilities = RAW_GetCapabilities;
	ifce->SetCapabilities = RAW_SetCapabilities;
	ifce->GetName = RAW_GetCodecName;
	ifce->ProcessData = RAW_ProcessData;
	ifce->privateStack = dec;

	return (GF_BaseDecoder*)ifce;
}

void DeleteRAWDec(GF_BaseDecoder *ifcg)
{
	RAWCTX();
	gf_free(ctx);
}
