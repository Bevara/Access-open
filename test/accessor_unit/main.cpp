#include "gtest/gtest.h"

#include "access/accessor.h"
#include "access/compiler.h"

#include "gpac/module.h"
#include "gpac/modules/codec.h"
#include "gpac/modules/service.h"
#include "gpac/internal/terminal_dev.h"

using namespace std;

string signals_fld;
string modules_fld;
string preserved_fld;
string accessors_fld;

GF_ModuleManager *modules;
GF_Config *config;
GF_MediaDecoder* ifce_acc;
GF_Channel *ch;
GF_NetworkCommand com;
GF_ClientService net_service;

#define CAP_CODES 29

static void img_on_connect(GF_ClientService *service, LPNETCHANNEL netch, GF_Err err)
{
	service->ifce->GetServiceDescriptor(service->ifce, 1, "");
}

static void img_on_disconnect(GF_ClientService *service, LPNETCHANNEL netch, GF_Err response)
{
	gf_es_del(ch);
}

static void img_on_command(GF_ClientService *service, GF_NetworkCommand *com, GF_Err response)
{

}

static void img_on_data_packet(GF_ClientService *service, LPNETCHANNEL netch, char *data, u32 data_size, GF_SLHeader *hdr, GF_Err reception_status){

}

static void img_on_media_add(GF_ClientService *service, GF_Descriptor *media_desc, Bool no_scene_check)
{

	if (media_desc) {
		GF_ESD *esd;
		u32 e2;
		GF_ObjectDescriptor *odm;

		odm = (GF_ObjectDescriptor *)media_desc;
		esd = (GF_ESD *)gf_list_get(odm->ESDescriptors, 0);
		ch = gf_es_new(esd);
		com.base.on_channel = ch;

		ASSERT_EQ(service->ifce->ConnectChannel(service->ifce, ch, "ES_ID=1", (Bool)esd->decoderConfig->upstream), GF_OK);
		ASSERT_EQ(service->ifce->ServiceCommand(service->ifce, &com), GF_OK);

		ASSERT_EQ(ifce_acc->CanHandleStream((GF_BaseDecoder*)ifce_acc, GF_STREAM_VISUAL, esd, 0), GF_CODEC_SUPPORTED);
		ASSERT_EQ(ifce_acc->AttachStream((GF_BaseDecoder*)ifce_acc, esd), GF_OK);
	}

}

void getImgOutput(const char* url, char* service, char* decoder, char** dataOut, u32* outDataLength, int* capcodes){
	GF_Descriptor *desc;
	GF_Err e, state;
	GF_SLHeader slh;
	u32 inDataLength = 0;
	Bool comp, is_new_data;
	char *dataIn = NULL;

	u32 od_type = 0;
	char *ext, *redirect_url;
	char *sub_url=NULL;

	*outDataLength = 0;
	*dataOut = 0;

	ifce_acc = (GF_MediaDecoder*)gf_modules_load_interface_by_name(modules, decoder, GF_MEDIA_DECODER_INTERFACE);
	ASSERT_TRUE(ifce_acc);
	GF_InputService* ifce_isom = (GF_InputService*)gf_modules_load_interface_by_name(modules, service, GF_NET_CLIENT_INTERFACE);
	ASSERT_TRUE(ifce_acc);


	net_service.fn_connect_ack = img_on_connect;
	net_service.fn_disconnect_ack = img_on_disconnect;
	net_service.fn_command = img_on_command;
	net_service.fn_data_packet = img_on_data_packet;
	net_service.fn_add_media = img_on_media_add;
	net_service.ifce = ifce_isom;

	/* Connecting */
	ASSERT_EQ(ifce_isom->ConnectService(ifce_isom, &net_service, url), GF_OK);
	
	/* Decoding */
	memset(&slh, 0, sizeof(GF_SLHeader));
	ASSERT_EQ(ifce_isom->ChannelGetSLP(ifce_isom, ch, (char **)&dataIn, &inDataLength, &slh, &comp, &state, &is_new_data), GF_OK);
	ASSERT_EQ(ifce_acc->ProcessData(ifce_acc, dataIn, inDataLength, 0, NULL, *dataOut, outDataLength, 0, 0), GF_BUFFER_TOO_SMALL);

	/* Decoding */
	*dataOut = (char*)malloc(*outDataLength);
	ASSERT_EQ(ifce_acc->ProcessData(ifce_acc, dataIn, inDataLength, 0, NULL, *dataOut, outDataLength, 0, 0), GF_OK);


	/* Get caps codes*/
	GF_CodecCapability caps;
	memset(&caps, 0, sizeof(GF_CodecCapability));
	for (caps.CapCode = 1; caps.CapCode  < CAP_CODES; caps.CapCode++, capcodes++) {
		ifce_acc->GetCapabilities((GF_BaseDecoder*)ifce_acc, &caps);
		*capcodes = caps.cap.valueInt;
	}

	gf_modules_close_interface((GF_BaseInterface *)ifce_acc);
	gf_modules_close_interface((GF_BaseInterface *)ifce_isom);
}

void getAccImgOutput(const char* file_url, const char* accessor_url, char** dataOut, u32* outDataLength, int* capcodes) {
	GF_Descriptor *desc;
	GF_Err e, state;
	GF_SLHeader slh;
	u32 inDataLength = 0;
	Bool comp, is_new_data;
	char *dataIn = NULL;
	char* service = "GPAC Image Reader";
	char* decoder = "Accessor dec";
	u32 od_type = 0;
	char *ext, *redirect_url;
	char *sub_url = NULL;

	*outDataLength = 0;
	*dataOut = 0;
	
	//ifce_acc = (GF_MediaDecoder*)gf_accessors_load_interface_by_name(modules, decoder, GF_MEDIA_DECODER_INTERFACE);
	ASSERT_TRUE(ifce_acc);
	GF_InputService* ifce_isom = (GF_InputService*)gf_modules_load_interface_by_name(modules, service, GF_NET_CLIENT_INTERFACE);
	ASSERT_TRUE(ifce_acc);
	

	net_service.fn_connect_ack = img_on_connect;
	net_service.fn_disconnect_ack = img_on_disconnect;
	net_service.fn_command = img_on_command;
	net_service.fn_data_packet = img_on_data_packet;
	net_service.fn_add_media = img_on_media_add;
	net_service.ifce = ifce_isom;

	/* Connecting */
	ASSERT_EQ(ifce_isom->ConnectService(ifce_isom, &net_service, file_url), GF_OK);

	/* Decoding */
	memset(&slh, 0, sizeof(GF_SLHeader));
	ASSERT_EQ(ifce_isom->ChannelGetSLP(ifce_isom, ch, (char **)&dataIn, &inDataLength, &slh, &comp, &state, &is_new_data), GF_OK);
	ASSERT_EQ(ifce_acc->ProcessData(ifce_acc, dataIn, inDataLength, 0, NULL, *dataOut, outDataLength, 0, 0), GF_BUFFER_TOO_SMALL);

	/* Decoding */
	*dataOut = (char*)malloc(*outDataLength);
	ASSERT_EQ(ifce_acc->ProcessData(ifce_acc, dataIn, inDataLength, 0, NULL, *dataOut, outDataLength, 0, 0), GF_OK);

	/* Get caps codes*/
	GF_CodecCapability caps;
	memset(&caps, 0, sizeof(GF_CodecCapability));
	for (caps.CapCode = 1; caps.CapCode < CAP_CODES; caps.CapCode++, capcodes++) {
		ifce_acc->GetCapabilities((GF_BaseDecoder*)ifce_acc, &caps);
		*capcodes = caps.cap.valueInt;
	}

	gf_modules_close_interface((GF_BaseInterface *)ifce_acc);
	gf_modules_close_interface((GF_BaseInterface *)ifce_isom);
}

TEST(File, DISABLED_JPG_PROGRESSIVE) {
	char *accData = NULL;
	char *jpegData = NULL;
	u32 accDataLength = 0;
	u32 jpegDataLength = 0;
	string inFile = signals_fld;
	string preservedFile = preserved_fld;
	int comp;
	int img_cap_code[CAP_CODES];
	int acc_cap_code[CAP_CODES];

	/* Set file in*/
	inFile.append("Freedom_progressive.jpg");

	/* Set preserved file */
	preservedFile.append("Freedom_progressive.jpg.bvr");

	getImgOutput(inFile.c_str(), "GPAC Image Reader", "GPAC Image Decoder", &jpegData, &jpegDataLength, &img_cap_code[0]);
	getImgOutput(inFile.c_str(), "GPAC IsoMedia Reader", "Accessor dec", &accData, &accDataLength, &acc_cap_code[0]);

	/* Compare result */
	ASSERT_EQ(jpegDataLength, accDataLength);
	comp = memcmp(jpegData, accData, accDataLength);
	printf("Value of decoder comparison is %d", comp);
}

TEST(File, DISABLED_DNG) {
	char *accData = NULL;
	char *jpegData = NULL;
	u32 accDataLength = 0;
	u32 jpegDataLength = 0;
	string inFile = signals_fld;
	string accessorFile = accessors_fld;
	int comp;
	int img_cap_code[CAP_CODES];
	int acc_cap_code[CAP_CODES];

	/* Set file in*/
	inFile.append("ATK_SFS_almost_no_knead_bread-32.dng");

	accessorFile.append("dng_entry_0.1.bc");

	getImgOutput(inFile.c_str(), "GPAC Image Reader", "GPAC Image Decoder", &jpegData, &jpegDataLength, &img_cap_code[0]);
	getAccImgOutput(inFile.c_str(), accessorFile.c_str(), &accData, &accDataLength, &acc_cap_code[0]);

	/* Compare caps */
	for (int i = 0; i < CAP_CODES-2; i++) {
		ASSERT_EQ(img_cap_code[i], acc_cap_code[i]);
	}

	/* Compare result */
	ASSERT_EQ(jpegDataLength, accDataLength);
	comp = memcmp(jpegData, accData, accDataLength);
	printf("Value of decoder comparison is %d", comp);
}

TEST(File, DISABLED_JPG) {
	char *accData = NULL;
	char *jpegData = NULL;
	u32 accDataLength = 0;
	u32 jpegDataLength = 0;
	string inFile = signals_fld;
	string accessorFile = accessors_fld;
	int comp;
	int img_cap_code[CAP_CODES];
	int acc_cap_code[CAP_CODES];

	/* Set file in*/
	inFile.append("Freedom.jpg");

	accessorFile.append("nanojpeg_entry_0.3.bc");

	getAccImgOutput(inFile.c_str(), accessorFile.c_str(), &accData, &accDataLength, &acc_cap_code[0]);

}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	for (int i = 1; i < argc; i++) {
		if (i + 1 != argc){
			if (strcmp("--signals", argv[i]) == 0) {
				signals_fld = argv[++i];
				signals_fld.append("\\");
				std::cout << "Test signal folder set to " << signals_fld << endl;
			}
			if (strcmp("--preserved", argv[i]) == 0) {
				preserved_fld = argv[++i];
				preserved_fld.append("\\");
				std::cout << "Preserved signal folder set to " << preserved_fld << endl;
			}
			else if (strcmp("--modules", argv[i]) == 0) {
				modules_fld = argv[++i];
				modules_fld.append("\\");
				std::cout << "Test modules folder set to " << modules_fld << endl;
			}
			else if (strcmp("--accessors", argv[i]) == 0) {
				accessors_fld = argv[++i];
				accessors_fld.append("\\");
				std::cout << "Accessors folder set to " << accessors_fld << endl;
			}
		}
	}
	gf_sys_init(GF_FALSE);
	config = gf_cfg_init(NULL, NULL);
	modules = gf_modules_new(NULL, config);
	
	gf_init_compiler();

	//Deactivate cache
	//gf_cfg_set_key(config, "Accessor", "Enable_cache", "no");

	//test_jpg();
	return RUN_ALL_TESTS();
}
