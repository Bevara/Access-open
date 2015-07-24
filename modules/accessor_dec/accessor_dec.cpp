#include <gpac/modules/service.h>
#include <gpac/modules/codec.h>
#include <gpac/avparse.h>
#include <gpac/constants.h>



#include "accessor_comp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	u16 ES_ID;

	struct Acc_Comp_s* comp;

	GF_Err (*init)(char* data, unsigned int dataLength);
	GF_Err (*entry)(char *inBuffer, u32 inBufferLength, char *outBuffer, u32 *outBufferLength, u8 PaddingBits, u32 mmlevel);
	GF_Err (*shutdown)();
	GF_Err (*set_caps)(GF_CodecCapability capability);
	GF_Err (*caps)(GF_CodecCapability *capability);
} Acc_dec;

static GF_Err BV_AttachStream(GF_BaseDecoder *ifcg, GF_ESD *esd)
{
	GF_Err err = GF_OK;
	char *llvm_code = NULL; 
	u32 llvm_length;
	char* data = NULL;
	u32 dataLength = 0;
	Acc_dec *ctx = NULL;
	const char* cachedir = NULL;

	printf("[Accessor decoder] Trying to use specific bevara decoder... \n");
	ctx = (Acc_dec*) ifcg->privateStack;

	/*not supported for a first version (and may be never be)*/
	if (esd->dependsOnESID){
		err = GF_NOT_SUPPORTED;
		goto error;
	}
	llvm_length = esd->decoderConfig->rvc_config->dataLength;
	if (!llvm_length){
		err = GF_NOT_SUPPORTED;
		goto error;
	}
	
	printf("[Accessor decoder] Bevara decoder can be used! Starting decoder compilation... \n");

	ctx->ES_ID = esd->ESID;
	llvm_code = esd->decoderConfig->rvc_config->data;
	llvm_code[llvm_length] = '\0'; //Todo : Copy data and then add the code end


	cachedir = gf_modules_get_option((GF_BaseInterface *)ifcg, "General", "CacheDirectory");

	// Init LLVM compilation
	err = (GF_Err)compile(&ctx->comp, llvm_code, llvm_length, cachedir);
	if (err) goto error;
	printf("[Accessor decoder] Compilation succeed! Start parsing useful metadata... \n");

	ctx->entry = (GF_Err (*)(char *, u32, char *, u32*, u8, u32))
		    		  getFn(ctx->comp, "ENTRY");
	if (!ctx->entry){
		err = GF_CORRUPTED_DATA;
		goto error;
	}
	ctx->init = (GF_Err (*)(char *, unsigned int))
			getFn(ctx->comp, "INIT");
	ctx->shutdown = (GF_Err (*)())
			getFn(ctx->comp, "CLOSE");
	ctx->caps = (GF_Err (*)(GF_CodecCapability *))
			getFn(ctx->comp, "CAPABILITIES");
	ctx->set_caps = (GF_Err (*)(GF_CodecCapability))
			getFn(ctx->comp, "SET_CAPABILITIES");

	if (esd->decoderConfig->decoderSpecificInfo){
		data = esd->decoderConfig->decoderSpecificInfo->data;
		dataLength = esd->decoderConfig->decoderSpecificInfo->dataLength;
	}


	/* Initialize dec */
	if (ctx->init){
		printf("[Accessor decoder] Start initializing decoder..\n");
		err = ctx->init(data, dataLength);

		if (err) goto error;
		printf("[Accessor decoder] Initializing succeed!\n");
	}
	else{
		printf("[Accessor decoder] Decoder doesn't need initialization.\n");
	}

	//gf_free(llvm_code);
	esd->decoderConfig->rvc_config->data = NULL;
	esd->decoderConfig->rvc_config->dataLength = 0;

	printf("[Accessor decoder] Accessor is ready to be used!\n");
	return err;

error:
	fprintf(stderr,"[Accessor decoder] Unrecoverable error! Accessor can't be used for the following reason : %s", gf_error_to_string(err));
	return err;
}

static GF_Err BV_DetachStream(GF_BaseDecoder *ifcg, u16 ES_ID)
{
	Acc_dec *ctx = (Acc_dec*) ifcg->privateStack;
	if (ctx->shutdown)  ctx->shutdown();
	return GF_OK;
}

static GF_Err BV_GetCapabilities(GF_BaseDecoder *ifcg, GF_CodecCapability *capability)
{
	GF_Err err;
	Acc_dec *ctx = (Acc_dec*) ifcg->privateStack;
	if (!ctx->caps) return GF_NOT_SUPPORTED;
	err = ctx->caps(capability);
	 return err;
}

static GF_Err BV_SetCapabilities(GF_BaseDecoder *ifcg, GF_CodecCapability capability)
{
	Acc_dec *ctx = (Acc_dec*) ifcg->privateStack;
	if (!ctx->set_caps) return GF_NOT_SUPPORTED;
	return ctx->set_caps(capability);
}


int bookmark = 0;

static GF_Err BV_ProcessData(GF_MediaDecoder *ifcg, 
		char *inBuffer, u32 inBufferLength,
		u16 ES_ID, u32 *CTS,
		char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	Acc_dec *ctx = (Acc_dec*) ifcg->privateStack;
	return ctx->entry(inBuffer, inBufferLength, outBuffer, outBufferLength, PaddingBits, mmlevel);
}

static u32 BV_CanHandleStream(GF_BaseDecoder *dec, u32 StreamType, GF_ESD *esd, u8 PL)
{
	if (esd && esd->decoderConfig && esd->decoderConfig->rvc_config) return GF_CODEC_SUPPORTED;
	return GF_CODEC_NOT_SUPPORTED;
}

static const char *BV_GetCodecName(GF_BaseDecoder *dec)
{
	return "Accessor Decoder";
}


void Delete_Acc_dec(GF_BaseDecoder *ifcg)
{
	Acc_dec *ctx = (Acc_dec*) ifcg->privateStack;
	gf_free(ctx);
	gf_free(ifcg);
}

GF_BaseDecoder *New_BV_Decoder()
{
	GF_MediaDecoder *ifcd;
	Acc_dec *dec;
	
	GF_SAFEALLOC(ifcd, GF_MediaDecoder);
	GF_SAFEALLOC(dec, Acc_dec);
	GF_REGISTER_MODULE_INTERFACE(ifcd, GF_MEDIA_DECODER_INTERFACE, "Bevara Decoder", "Bevara Technologies")

	ifcd->privateStack = dec;

	/*setup our own interface*/	
	ifcd->AttachStream = BV_AttachStream;
	ifcd->DetachStream = BV_DetachStream;
	ifcd->GetCapabilities = BV_GetCapabilities;
	ifcd->SetCapabilities = BV_SetCapabilities;
	ifcd->GetName = BV_GetCodecName;
	ifcd->CanHandleStream = BV_CanHandleStream;
	ifcd->ProcessData = BV_ProcessData;
	return (GF_BaseDecoder *) ifcd;
}



GPAC_MODULE_EXPORT
const u32 *QueryInterfaces() 
{
	static u32 si [] = {
		GF_MEDIA_DECODER_INTERFACE,
		0
	};
	return si; 
}

GPAC_MODULE_EXPORT
GF_BaseInterface *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == GF_MEDIA_DECODER_INTERFACE) return (GF_BaseInterface *)New_BV_Decoder();
	return NULL;
}

GPAC_MODULE_EXPORT
void ShutdownInterface(GF_BaseInterface *ifce)
{
	switch (ifce->InterfaceType) {
#ifndef GPAC_DISABLE_AV_PARSERS
	case GF_MEDIA_DECODER_INTERFACE: 
		Delete_Acc_dec((GF_BaseDecoder*)ifce);
		break;
#endif
	}
}


GPAC_MODULE_STATIC_DECLARATION( Acc_dec )

#ifdef __cplusplus
}
#endif