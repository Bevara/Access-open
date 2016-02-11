
#include "img_in.h"

#include "libraw/libraw.h"

#define RAWCTX()	RAWDec *ctx = (RAWDec *) ((IMGDec *)ifcg->privateStack)->opaque

typedef struct
{
	u16 ES_ID;
	u32 width, height, out_size, pixel_format;
	u8 BPP;
} RAWDec;


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
		capability->cap.valueInt = IMG_CM_SIZE;
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

Bool NewRAWDec(GF_BaseDecoder *ifcd)
{
	IMGDec *wrap = (IMGDec *)ifcd->privateStack;
	RAWDec *dec = (RAWDec *)gf_malloc(sizeof(RAWDec));
	memset(dec, 0, sizeof(RAWDec));
	wrap->opaque = dec;
	wrap->type = DEC_RAW;

	/*setup our own interface*/
	ifcd->AttachStream = RAW_AttachStream;
	ifcd->DetachStream = RAW_DetachStream;
	ifcd->GetCapabilities = RAW_GetCapabilities;
	ifcd->SetCapabilities = RAW_SetCapabilities;
	ifcd->GetName = RAW_GetCodecName;
	((GF_MediaDecoder *)ifcd)->ProcessData = RAW_ProcessData;
	return GF_TRUE;
}

void DeleteRAWDec(GF_BaseDecoder *ifcg)
{
	RAWCTX();
	gf_free(ctx);
}
