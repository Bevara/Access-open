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

static void term_on_connect(GF_ClientService *service, LPNETCHANNEL netch, GF_Err err)
{

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
	GF_ESD *esd1, *esd2;
	if (gf_list_count(od1->ESDescriptors) != gf_list_count(od2->ESDescriptors)) return GF_FALSE;
	esd1 = (GF_ESD *)gf_list_get(od1->ESDescriptors, 0);
	if (!esd1) return GF_FALSE;
	esd2 = (GF_ESD *)gf_list_get(od2->ESDescriptors, 0);
	if (!esd2) return GF_FALSE;
	if (esd1->ESID != esd2->ESID) return GF_FALSE;
	if (esd1->decoderConfig->streamType != esd2->decoderConfig->streamType) return GF_FALSE;
	if (esd1->decoderConfig->objectTypeIndication != esd2->decoderConfig->objectTypeIndication) return GF_FALSE;
	return GF_TRUE;
}

static void term_on_media_add(GF_ClientService *service, GF_Descriptor *media_desc, Bool no_scene_check)
{
	u32 i, min_od_id;
	GF_MediaObject *the_mo;
	GF_Scene *scene;
	GF_ObjectManager *odm, *root;
	GF_ObjectDescriptor *od;
	GF_Terminal *term = service->term;

	root = service->owner;
	if (!root) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_MEDIA, ("[Service %s] has not root, aborting !\n", service->url));
		return;
	}
	if (root->flags & GF_ODM_DESTROYED) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_MEDIA, ("[Service %s] root has been scheduled for destruction - aborting !\n", service->url));
		return;
	}
	scene = root->subscene ? root->subscene : root->parentscene;
	if (scene->root_od->addon && (scene->root_od->addon->addon_type == GF_ADDON_TYPE_MAIN)) {
		no_scene_check = GF_TRUE;
		scene->root_od->flags |= GF_ODM_REGENERATE_SCENE;
	}

	GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[Service %s] %s\n", service->url, media_desc ? "Adding new media object" : "Regenerating scene graph"));
	if (!media_desc) {
		if (!no_scene_check)
			gf_scene_regenerate(scene);
		return;
	}

	switch (media_desc->tag) {
	case GF_ODF_OD_TAG:
	case GF_ODF_IOD_TAG:
		if (root && (root->net_service == service)) {
			od = (GF_ObjectDescriptor *)media_desc;
			break;
		}
	default:
		gf_odf_desc_del(media_desc);
		return;
	}

	gf_term_lock_net(term, GF_TRUE);
	/*object declared this way are not part of an OD stream and are considered as dynamic*/
	/*	od->objectDescriptorID = GF_MEDIA_EXTERNAL_ID; */

	/*check if we have a mediaObject in the scene not attached and matching this object*/
	the_mo = NULL;
	odm = NULL;
	min_od_id = 0;
	for (i = 0; i<gf_list_count(scene->scene_objects); i++) {
		char *frag, *ext;
		GF_ESD *esd;
		char *url;
		u32 match_esid = 0;
		GF_MediaObject *mo = (GF_MediaObject*)gf_list_get(scene->scene_objects, i);

		if ((mo->OD_ID != GF_MEDIA_EXTERNAL_ID) && (min_od_id<mo->OD_ID))
			min_od_id = mo->OD_ID;

		if (!mo->odm) continue;
		/*if object is attached to a service, don't bother looking in a different one*/
		if (mo->odm->net_service && (mo->odm->net_service != service)) continue;

		/*already assigned object - this may happen since the compositor has no control on when objects are declared by the service,
		therefore opening file#video and file#audio may result in the objects being declared twice if the service doesn't
		keep track of declared objects*/
		if (mo->odm->OD) {
			if (od->objectDescriptorID && is_same_od(mo->odm->OD, od)) {
				/*reassign OD ID*/
				if (mo->OD_ID != GF_MEDIA_EXTERNAL_ID) {
					od->objectDescriptorID = mo->OD_ID;
				}
				else {
					mo->OD_ID = od->objectDescriptorID;
				}
				gf_odf_desc_del(media_desc);
				gf_term_lock_net(term, GF_FALSE);
				return;
			}
			continue;
		}
		if (mo->OD_ID != GF_MEDIA_EXTERNAL_ID) {
			if (mo->OD_ID == od->objectDescriptorID) {
				the_mo = mo;
				odm = mo->odm;
				break;
			}
			continue;
		}
		if (!mo->URLs.count || !mo->URLs.vals[0].url) continue;

		frag = NULL;
		ext = strrchr(mo->URLs.vals[0].url, '#');
		if (ext) {
			frag = strchr(ext, '=');
			ext[0] = 0;
		}
		url = mo->URLs.vals[0].url;
		if (!strnicmp(url, "file://localhost", 16)) url += 16;
		else if (!strnicmp(url, "file://", 7)) url += 7;
		else if (!strnicmp(url, "gpac://", 7)) url += 7;
		else if (!strnicmp(url, "pid://", 6)) match_esid = atoi(url + 6);

		if (!match_esid && !strstr(service->url, url)) {
			if (ext) ext[0] = '#';
			continue;
		}
		if (ext) ext[0] = '#';

		esd = (GF_ESD*)gf_list_get(od->ESDescriptors, 0);
		if (match_esid && (esd->ESID != match_esid))
			continue;
		/*match type*/
		switch (esd->decoderConfig->streamType) {
		case GF_STREAM_VISUAL:
			if (mo->type != GF_MEDIA_OBJECT_VIDEO) continue;
			break;
		case GF_STREAM_AUDIO:
			if (mo->type != GF_MEDIA_OBJECT_AUDIO) continue;
			break;
		case GF_STREAM_PRIVATE_MEDIA:
			if ((mo->type != GF_MEDIA_OBJECT_AUDIO) && (mo->type != GF_MEDIA_OBJECT_VIDEO)) continue;
			break;
		case GF_STREAM_SCENE:
			if (mo->type != GF_MEDIA_OBJECT_UPDATES) continue;
			break;
		default:
			continue;
		}
		if (frag) {
			u32 frag_id = 0;
			u32 ID = od->objectDescriptorID;
			if (ID == GF_MEDIA_EXTERNAL_ID) ID = esd->ESID;
			frag++;
			frag_id = atoi(frag);
			if (ID != frag_id) continue;
		}
		the_mo = mo;
		odm = mo->odm;
		break;
	}

	/*add a pass on scene->resource to check for min_od_id,
	otherwise we may have another modules declaring an object with ID 0 from
	another thread, which will assert (only one object with a givne OD ID)*/
	for (i = 0; i<gf_list_count(scene->resources); i++) {
		GF_ObjectManager *an_odm = (GF_ObjectManager *)gf_list_get(scene->resources, i);

		if (an_odm->OD && (an_odm->OD->objectDescriptorID != GF_MEDIA_EXTERNAL_ID) && (min_od_id < an_odm->OD->objectDescriptorID))
			min_od_id = an_odm->OD->objectDescriptorID;
	}

	if (!odm) {
		odm = gf_odm_new();
		odm->term = term;
		odm->parentscene = scene;
		gf_mx_p(scene->mx_resources);
		gf_list_add(scene->resources, odm);
		gf_mx_v(scene->mx_resources);
	}
	odm->flags |= GF_ODM_NOT_SETUP;
	odm->OD = od;
	odm->mo = the_mo;
	odm->flags |= GF_ODM_NOT_IN_OD_STREAM;
	if (!od->objectDescriptorID) {
		od->objectDescriptorID = min_od_id + 1;
	}

	if (the_mo) the_mo->OD_ID = od->objectDescriptorID;
	if (!scene->selected_service_id)
		scene->selected_service_id = od->ServiceID;


	/*net is unlocked before seting up the object as this might trigger events going into JS and deadlocks
	with the compositor*/
	gf_term_lock_net(term, GF_FALSE);

	GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[ODM%d] setup object - MO %08x\n", odm->OD->objectDescriptorID, odm->mo));
	gf_odm_setup_object(odm, service);

	/*OD inserted by service: resetup scene*/
	if (!no_scene_check && scene->is_dynamic_scene) gf_scene_regenerate(scene);
}

void loadAccessor(const char* url){
	GF_ClientService* net_service;

	GF_BaseDecoder* ifce_acc = (GF_BaseDecoder*)gf_modules_load_interface_by_name(modules, "Accessor dec", GF_MEDIA_DECODER_INTERFACE);
	GF_InputService* ifce_isom = (GF_InputService*)gf_modules_load_interface_by_name(modules, "GPAC IsoMedia Reader", GF_NET_CLIENT_INTERFACE);
	
	GF_SAFEALLOC(net_service, GF_ClientService);
	net_service->fn_connect_ack = term_on_connect;
	net_service->fn_disconnect_ack = term_on_disconnect;
	net_service->fn_command = term_on_command;
	net_service->fn_data_packet = term_on_data_packet;
	net_service->fn_add_media = term_on_media_add;


	ifce_isom->ConnectService(ifce_isom, net_service, url);
	//ASSERT_EQ(ifce_acc->CanHandleStream(ifce_acc, 0, NULL, 0), GF_CODEC_SUPPORTED);
	ifce_acc->CanHandleStream(ifce_acc, 0, NULL, 0);


	gf_modules_close_interface((GF_BaseInterface *)ifce_acc);
	gf_modules_close_interface((GF_BaseInterface *)ifce_isom);
}

//TEST(File, JPG) {
void test_jpg() {
	string file = signals_fld;
	file.append("Freedom.jpg.bvr");
	loadAccessor(file.c_str());
}
//}

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

	test_jpg();
	return RUN_ALL_TESTS();
}