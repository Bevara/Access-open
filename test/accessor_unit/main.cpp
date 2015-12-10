#include "gtest/gtest.h"

#include "gpac/module.h"
#include "gpac/modules/codec.h"
#include "gpac/modules/service.h"
#include "gpac/internal/terminal_dev.h"

using namespace std;

string signals_fld;
string modules_fld;

GF_ModuleManager *modules;
GF_Config *config;
GF_MediaDecoder* ifce_acc;

static void term_on_connect(GF_ClientService *service, LPNETCHANNEL netch, GF_Err err)
{
	service->ifce->GetServiceDescriptor(service->ifce, 1, "");
}

static void term_on_disconnect(GF_ClientService *service, LPNETCHANNEL netch, GF_Err response)
{

}

static void term_on_command(GF_ClientService *service, GF_NetworkCommand *com, GF_Err response)
{

}

static void term_on_data_packet(GF_ClientService *service, LPNETCHANNEL netch, char *data, u32 data_size, GF_SLHeader *hdr, GF_Err reception_status){

}

static Bool is_same_od(GF_ObjectDescriptor *od1, GF_ObjectDescriptor *od2)
{
	return GF_TRUE;
}

static void term_on_media_add(GF_ClientService *service, GF_Descriptor *media_desc, Bool no_scene_check)
{

	if (media_desc) {
		GF_ESD *esd;
		u32 e2;
		GF_ObjectDescriptor *odm;

		odm = (GF_ObjectDescriptor *)media_desc;
		esd = (GF_ESD *)gf_list_get(odm->ESDescriptors, 0);
		ASSERT_EQ(ifce_acc->CanHandleStream((GF_BaseDecoder*)ifce_acc, 0, esd, 0), GF_CODEC_SUPPORTED);
		ifce_acc->AttachStream((GF_BaseDecoder*)ifce_acc, esd);
	}

}

void loadAccessor(const char* url){
	GF_ClientService* net_service;
	GF_Descriptor *desc;
	GF_Err e;

	u32 od_type = 0;
	char *ext, *redirect_url;
	char *sub_url=NULL;

	ifce_acc = (GF_MediaDecoder*)gf_modules_load_interface_by_name(modules, "Accessor dec", GF_MEDIA_DECODER_INTERFACE);
	GF_InputService* ifce_isom = (GF_InputService*)gf_modules_load_interface_by_name(modules, "GPAC IsoMedia Reader", GF_NET_CLIENT_INTERFACE);
	
	GF_SAFEALLOC(net_service, GF_ClientService);
	net_service->fn_connect_ack = term_on_connect;
	net_service->fn_disconnect_ack = term_on_disconnect;
	net_service->fn_command = term_on_command;
	net_service->fn_data_packet = term_on_data_packet;
	net_service->fn_add_media = term_on_media_add;
	net_service->ifce = ifce_isom;

	/* Connecting */
	e = ifce_isom->ConnectService(ifce_isom, net_service, url);
	
	/* Decoding */
	ifce_isom->ChannelGetSLP(ifce_isom, 0, 0, 0, 0, 0, 0, 0);
	e = ifce_acc->ProcessData(ifce_acc, NULL, 0, 0, 0, 0, 0, 0, 0);

	gf_modules_close_interface((GF_BaseInterface *)ifce_acc);
	gf_modules_close_interface((GF_BaseInterface *)ifce_isom);
}

TEST(File, JPG) {
//void test_jpg() {
	string file = signals_fld;
	file.append("Freedom.jpg.bvr");
	loadAccessor(file.c_str());
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	for (int i = 1; i < argc; i++) {
		if (i + 1 != argc){
			if (strcmp("-signals", argv[i]) == 0) {
				signals_fld = argv[++i];
				std::cout << "Test signal folder set to " << signals_fld << endl;
			}
			else if (strcmp("-modules", argv[i]) == 0) {
				modules_fld = argv[++i];
				std::cout << "Test modules folder set to " << modules_fld << endl;
			}
		}
	}
	gf_sys_init(GF_FALSE);
	config = gf_cfg_init(NULL, NULL);
	modules = gf_modules_new(NULL, config);

	//test_jpg();
	return RUN_ALL_TESTS();
}