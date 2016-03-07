
#include <access/accessor.h>
#include "accessor_wrap.h"
#include <gpac/config_file.h>
#include <gpac/tools.h>
#include <gpac/network.h>


GF_EXPORT
GF_Err gf_accessor_load_static(GF_ModuleManager *pm, GF_AccessorRegister *(*register_module)())
{
	GF_AccessorRegister *pr = register_module();
	GF_Err rc;

	if (!pr) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("Failed to statically load module\n"));
		return GF_NOT_SUPPORTED;
	}

	rc = gf_list_add(pm->plugin_registry, pr);
	if (rc != GF_OK) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("Failed to statically load module\n"));
		return rc;
	}
	return GF_OK;
}

GF_EXPORT
GF_ModuleManager *gf_accessors_new(const char *directory, GF_Config *config)
{
	GF_ModuleManager *tmp;
	u32 loadedModules;
	const char *opt;
	u32 num_dirs = 0;

	if (!config) return NULL;

	/* Try to resolve directory from config file */
	GF_SAFEALLOC(tmp, GF_ModuleManager);
	if (!tmp) return NULL;
	tmp->cfg = config;
	tmp->mutex = gf_mx_new("Module Manager");
	gf_accessors_get_module_directories(tmp, &num_dirs);

	/* Initialize module list */
	tmp->plug_list = gf_list_new();
	if (!tmp->plug_list) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("OUT OF MEMORY, cannot create list of modules !!!\n"));
		gf_free(tmp);
		return NULL;
	}
	tmp->plugin_registry = gf_list_new();
	if (!tmp->plugin_registry) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("OUT OF MEMORY, cannot create list of static module registers !!!\n"));
		gf_list_del(tmp->plug_list);
		gf_free(tmp);
		return NULL;
	}

	opt = gf_cfg_get_key(config, "Systems", "ModuleUnload");
	if (opt && !strcmp(opt, "no")) {
		tmp->no_unload = GF_TRUE;
	}

	loadedModules = gf_accessors_refresh(tmp);
	GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("Loaded %d modules from directory %s.\n", loadedModules, directory));

	return tmp;
}

GF_EXPORT
void gf_accessors_del(GF_ModuleManager *pm)
{
	u32 i;
	AccessorInstance *inst;
	if (!pm) return;
	/*unload all modules*/
	while (gf_list_count(pm->plug_list)) {
		inst = (AccessorInstance *) gf_list_get(pm->plug_list, 0);
		gf_accessors_free_accessor(inst);
		gf_list_rem(pm->plug_list, 0);
	}
    gf_list_del(pm->plug_list);

	/* Delete module directories*/
	for (i = 0; i < pm->num_dirs; i++) {
		gf_free((void*)pm->dirs[i]);
	}

    /*remove all static modules registry*/
    while (gf_list_count(pm->plugin_registry)) {
        GF_AccessorRegister *reg  = (GF_AccessorRegister *) gf_list_get(pm->plugin_registry, 0);
        gf_free(reg);
        gf_list_rem(pm->plugin_registry, 0);
    }

	if (pm->plugin_registry) gf_list_del(pm->plugin_registry);
	gf_mx_del(pm->mutex);
	gf_free(pm);
}

Bool gf_accessor_is_loaded(GF_ModuleManager *pm, char *filename)
{
	u32 i = 0;
	AccessorInstance *inst;
	while ( (inst = (AccessorInstance *) gf_list_enum(pm->plug_list, &i) ) ) {
		if (!strcmp(inst->name, filename)) return GF_TRUE;
	}
	return GF_FALSE;
}

GF_EXPORT
u32 gf_accessors_get_count(GF_ModuleManager *pm)
{
	if (!pm) return 0;
	return gf_list_count(pm->plug_list);
}

GF_EXPORT
const char **gf_accessors_get_module_directories(GF_ModuleManager *pm, u32* num_dirs)
{
	char* directories;
	char* tmp_dirs;
	char * pch;
	if (!pm) return NULL;
	if (pm->num_dirs > 0 ) {
		*num_dirs = pm->num_dirs;
		return pm->dirs;
	}
	if (!pm->cfg) return NULL;

	/* Get directory from config file */
	directories = (char*)gf_cfg_get_key(pm->cfg, "General", "ModulesDirectory");
	if (! directories) {
#ifndef GPAC_IPHONE
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("Module directory not found - check the configuration file exit and the \"ModulesDirectory\" key is set\n"));
#endif
		return NULL;
	}

//	tmp_dirs = gf_strdup(directories);
	tmp_dirs = directories;
	pch = strtok (tmp_dirs,";");

	while (pch != NULL)
	{
		if (pm->num_dirs == MAX_MODULE_DIRS) {
			GF_LOG(GF_LOG_WARNING, GF_LOG_CORE, ("Reach maximum number of module directories  - check the configuration file and the \"ModulesDirectory\" key.\n"));
			break;
		}

		pm->dirs[pm->num_dirs] = gf_strdup(pch);
		pm->num_dirs++;
		pch = strtok (NULL, ";");
	}
//	gf_free(tmp_dirs);
	*num_dirs = pm->num_dirs;
	return pm->dirs;
}

GF_EXPORT
GF_BaseAccInterface *gf_accessors_load_interface(GF_ModuleManager *pm, u32 whichplug, u32 InterfaceFamily)
{
	const char *opt;
	char szKey[32];
	AccessorInstance *inst;
	GF_BaseAccInterface *ifce;

	if (!pm) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] gf_accessors_load_interface() : No Module Manager set\n"));
		return NULL;
	}
	gf_mx_p(pm->mutex);
	inst = (AccessorInstance *) gf_list_get(pm->plug_list, whichplug);
	if (!inst) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] gf_accessors_load_interface() : no module %d exist.\n", whichplug));
		gf_mx_v(pm->mutex);
		return NULL;
	}
	GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] Load interface...%s\n", inst->name));
	/*look in cache*/
	if (!pm->cfg) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] No pm->cfg has been set !!!\n"));
		gf_mx_v(pm->mutex);
		return NULL;
	}
	opt = gf_cfg_get_key(pm->cfg, "PluginsCache", inst->name);
	if (opt) {
		const char * ifce_str = gf_4cc_to_str(InterfaceFamily);
		snprintf(szKey, 32, "%s:yes", ifce_str ? ifce_str : "(null)");
		if (!strstr(opt, szKey)) {
			gf_mx_v(pm->mutex);
			return NULL;
		}
	}
	if (!gf_accessors_load_library(inst)) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] Cannot load library %s\n", inst->name));
		gf_cfg_set_key(pm->cfg, "PluginsCache", inst->name, "Invalid Plugin");
		gf_mx_v(pm->mutex);
		return NULL;
	}
	if (!inst->query_func) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] Library %s missing GPAC export symbols\n", inst->name));
		gf_cfg_set_key(pm->cfg, "PluginsCache", inst->name, "Invalid Plugin");
		goto err_exit;
	}

	/*build cache*/
	if (!opt) {
		u32 i;
		Bool found = GF_FALSE;
		char *key;
		const u32 *si = inst->query_func();
		if (!si) {
			GF_LOG(GF_LOG_WARNING, GF_LOG_CORE, ("[Core] GPAC module %s has no supported interfaces - disabling\n", inst->name));
			gf_cfg_set_key(pm->cfg, "PluginsCache", inst->name, "Invalid Plugin");
			goto err_exit;
		}
		i=0;
		while (si[i]) i++;

		key = (char*)gf_malloc(sizeof(char) * 10 * i);
		key[0] = 0;
		i=0;
		while (si[i]) {
			snprintf(szKey, 32, "%s:yes ", gf_4cc_to_str(si[i]));
			strcat(key, szKey);
			if (InterfaceFamily==si[i]) found = GF_TRUE;
			i++;
		}
		gf_cfg_set_key(pm->cfg, "PluginsCache", inst->name, key);
		gf_free(key);
		if (!found) goto err_exit;
	}

	if (!inst->query_func || !inst->query_func(/*InterfaceFamily*/) ) goto err_exit;
	ifce = (GF_BaseAccInterface *) inst->load_func(InterfaceFamily);
	/*sanity check*/
	if (!ifce) goto err_exit;
	if (!ifce->module_name || (ifce->InterfaceType != InterfaceFamily)) {
		inst->destroy_func(ifce);
		goto err_exit;
	}
	gf_list_add(inst->interfaces, ifce);
	/*keep track of parent*/
	ifce->HPLUG = inst;
	GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] Load interface %s DONE.\n", inst->name));
	gf_mx_v(pm->mutex);
	return ifce;

err_exit:
	GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] Load interface %s exit label, freing library...\n", inst->name));
	gf_accessors_unload_library(inst);
	GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] Load interface %s EXIT.\n", inst->name));
	gf_mx_v(pm->mutex);
	return NULL;
}


GF_EXPORT
GF_BaseAccInterface *gf_accessors_load_interface_by_name(GF_ModuleManager *pm, const char *plug_name, u32 InterfaceFamily)
{
	const char *file_name;
	u32 i, count;
	GF_BaseAccInterface *ifce;
	if (!pm || !plug_name || !pm->plug_list || !pm->cfg) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] gf_accessors_load_interface_by_name has bad parameters pm=%p, plug_name=%s.\n", pm, plug_name));
		return NULL;
	}
	count = gf_list_count(pm->plug_list);
	/*look for cache entry*/
	file_name = gf_cfg_get_key(pm->cfg, "PluginsCache", plug_name);

	if (file_name) {

		for (i=0; i<count; i++) {
			AccessorInstance *inst = (AccessorInstance *) gf_list_get(pm->plug_list, i);
			if (!strcmp(inst->name,  file_name)) {
				ifce = gf_accessors_load_interface(pm, i, InterfaceFamily);
				if (ifce) return ifce;
			}
		}
	}
	GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("[Core] Plugin %s of type %d not found in cache, searching for it...\n", plug_name, InterfaceFamily));
	for (i=0; i<count; i++) {
		ifce = gf_accessors_load_interface(pm, i, InterfaceFamily);
		if (!ifce) continue;
		if (ifce->module_name && !strnicmp(ifce->module_name, plug_name, MIN(strlen(ifce->module_name), strlen(plug_name)) )) {
			/*update cache entry*/
			gf_cfg_set_key(pm->cfg, "PluginsCache", plug_name, ((AccessorInstance*)ifce->HPLUG)->name);
			GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] Added plugin cache %s for %s\n", plug_name, ((AccessorInstance*)ifce->HPLUG)->name));
			return ifce;
		}
		gf_accessors_close_interface(ifce);
	}
	GF_LOG(GF_LOG_WARNING, GF_LOG_CORE, ("[Core] Plugin %s not found in %d modules.\n", plug_name, count));
	return NULL;
}

GF_EXPORT
GF_Err gf_accessors_close_interface(GF_BaseAccInterface *ifce)
{
	AccessorInstance *par;
	s32 i;
	if (!ifce) return GF_BAD_PARAM;
	par = (AccessorInstance *) ifce->HPLUG;

	if (!par || !ifce->InterfaceType) return GF_BAD_PARAM;

	i = gf_list_find(par->plugman->plug_list, par);
	if (i<0) return GF_BAD_PARAM;

	i = gf_list_find(par->interfaces, ifce);
	if (i<0) return GF_BAD_PARAM;
	gf_list_rem(par->interfaces, (u32) i);
	par->destroy_func(ifce);
	gf_accessors_unload_library(par);
	//GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] interface %s unloaded\n", ifce->module_name));
	return GF_OK;
}

GF_EXPORT
const char *gf_accessors_get_option(GF_BaseAccInterface *ifce, const char *secName, const char *keyName)
{
	GF_Config *cfg;
	if (!ifce || !ifce->HPLUG) return NULL;
	cfg = ((AccessorInstance *)ifce->HPLUG)->plugman->cfg;
	if (!cfg) return NULL;
	return gf_cfg_get_ikey(cfg, secName, keyName);
}

GF_EXPORT
GF_Err gf_accessors_set_option(GF_BaseAccInterface *ifce, const char *secName, const char *keyName, const char *keyValue)
{
	GF_Config *cfg;
	if (!ifce || !ifce->HPLUG) return GF_BAD_PARAM;
	cfg = ((AccessorInstance *)ifce->HPLUG)->plugman->cfg;
	if (!cfg) return GF_NOT_SUPPORTED;
	return gf_cfg_set_key(cfg, secName, keyName, keyValue);
}

GF_EXPORT
GF_Config *gf_accessors_get_config(GF_BaseAccInterface *ifce)
{
	if (!ifce || !ifce->HPLUG) return NULL;
	return ((AccessorInstance *)ifce->HPLUG)->plugman->cfg;
}

GF_EXPORT
const char *gf_accessors_get_file_name(GF_ModuleManager *pm, u32 i)
{
	AccessorInstance *inst = (AccessorInstance *) gf_list_get(pm->plug_list, i);
	if (!inst) return NULL;
	return inst->name;
}

GF_EXPORT
const char *gf_accessor_get_file_name(GF_BaseAccInterface *ifce)
{
	AccessorInstance *inst = (AccessorInstance *) ifce->HPLUG;
	if (!inst) return NULL;
	return inst->name;
}



