/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean Le Feuvre
 *			Copyright (c) Telecom ParisTech 2000-2012
 *					All rights reserved
 *
 *  This file is part of GPAC
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
 **  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*this file is only used with Win32&MSVC to export the symbols from libgpac(static) to libgpac(dynamic) based on the GPAC configuration*/
#include <gpac/setup.h>

#if defined(_WIN32_WCE) || defined(_WIN64)
#define EXPORT_SYMBOL(a) "/export:"#a
#else
#define EXPORT_SYMBOL(a) "/export:_"#a
#endif

#ifdef _WIN32_WCE
#pragma comment (linker, EXPORT_SYMBOL(CE_Assert) )
#pragma comment (linker, EXPORT_SYMBOL(CE_CharToWide) )
#pragma comment (linker, EXPORT_SYMBOL(CE_WideToChar) )
#endif

#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_new) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_del) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_refresh) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_get_count) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_get_module_directories) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_get_file_name) )
#pragma comment (linker, EXPORT_SYMBOL(gf_module_get_file_name) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_load_interface) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_load_interface_by_name) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_close_interface) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_get_option) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_set_option) )
#pragma comment (linker, EXPORT_SYMBOL(gf_accessors_get_config) )

#pragma comment (linker, EXPORT_SYMBOL(gf_init_compiler) )


