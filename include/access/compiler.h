
#ifndef _gf_compiler_H_
#define _gf_compiler_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/module.h>
 *	\brief plugable module functions.
 */

/*!
 *	\addtogroup mods_grp plugable modules
 *	\ingroup utils_grp
 *	\brief Plugable Module functions
 *
 *This section documents the plugable module functions of the GPAC framework.
 *A module is a dynamic/shared library providing one or several interfaces to the GPAC framework.
 *A module cannot provide several interfaces of the same type. Each module must export the following functions:
 \code
 *	u32 *QueryInterfaces(u32 interface_type);
 \endcode
 *	This function is used to query supported interfaces. It returns a zero-terminated array of supported interface types.\n
 \code
 	GF_BaseAccInterface *LoadInterface(u32 interface_type);
 \endcode
 *	This function is used to load an interface. It returns the interface object, NULL if error.\n
 \code
 	void ShutdownInterface(GF_BaseAccInterface *interface);
 \endcode
 *This function is used to destroy an interface.\n\n
 *Each interface must begin with the interface macro in order to be type-casted to the base interface structure.
 \code
	struct {
		GF_DECL_MODULE_INTERFACE
		extensions;
	};
 \endcode
 *	@{
 */
#include "gpac/tools.h"


	GF_Err gf_init_compiler();
/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_gf_accessor_H_*/
