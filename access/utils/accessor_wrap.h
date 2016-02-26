
#include <gpac/module.h>
#include <gpac/list.h>
#include <gpac/thread.h>

#ifndef _GF_MODULE_WRAP_H_
#define _GF_MODULE_WRAP_H_

#define MAX_MODULE_DIRS 1024

/* interface api*/
typedef const u32 *(*QueryInterfaces) ();
typedef void * (*LoadInterface) (u32 InterfaceType);
typedef void (*ShutdownInterface) (void *interface_obj);


typedef struct
{
	struct __tag_mod_man *plugman;
	char *name;
	GF_List *interfaces;

	/*for static modules*/
	GF_InterfaceRegister *ifce_reg;

	/*library is loaded only when an interface is attached*/
	void *lib_handle;
	QueryInterfaces query_func;
	LoadInterface load_func;
	ShutdownInterface destroy_func;
	char* dir;
} AccessorInstance;


struct __tag_mod_man
{
	/*location of the modules*/
	const char* dirs[MAX_MODULE_DIRS];
	u32 num_dirs;
	GF_List *plug_list;
	GF_Config *cfg;
	Bool no_unload;
	/*the one and only ssl instance used throughout the client engine*/
	void *ssl_inst;

	/*all static modules store their InterfaceRegistry here*/
	GF_List *plugin_registry;

	/* Mutex to handle simultaneous calls to the load_interface function */
	GF_Mutex *mutex;
};

#ifdef __cplusplus
extern "C" {
#endif

/*returns 1 if a module with the same filename is already loaded*/
Bool gf_module_is_loaded(GF_ModuleManager *pm, char *filename);

/*these are OS specific*/
void gf_accessors_free_accessor(AccessorInstance *inst);
Bool gf_accessors_load_library(AccessorInstance *inst);
void gf_accessors_unload_library(AccessorInstance *inst);
u32 gf_accessors_refresh(GF_ModuleManager *pm);

#ifdef __cplusplus
}
#endif

#endif	/*_GF_MODULE_WRAP_H_*/

