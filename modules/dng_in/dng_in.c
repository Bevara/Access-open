#include <time.h>

#include <gpac/modules/service.h>
#include <gpac/modules/codec.h>
#include <gpac/constants.h>

/*****************************************************/
/** Bevara
*** The following code may utilize libraw structures or modified structures or code. See the
*** ORIGINAL directory for the unmodified libraw distribution. 
*** The license information is as follows:
*** LibRaw is free software; you can redistribute it and/or modify
*** it under the terms of the one of three licenses as you choose:
***
*** 1. GNU LESSER GENERAL PUBLIC LICENSE version 2.1
*** (See file LICENSE.LGPL provided in LibRaw distribution archive for details).
***
*** 2. COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL) Version 1.0
*** (See file LICENSE.CDDL provided in LibRaw distribution archive for details).
*** 
*** 3. LibRaw Software License 27032010
*** (See file LICENSE.LibRaw.pdf provided in LibRaw distribution archive for details).
***
***/


//#include "libraw/libraw.h"

#define GPAC_BMP_OTI	0x82

#define MAX_IFDS	10  // maximum number of IFDs handled in DNG parsing

/**
\struct 
\brief Libraw structures
*/

#if !defined(ushort)
#define ushort unsigned short
#endif
#if !defined(uchar)
#define uchar unsigned char
#endif
#if !defined(UINT32)
#define UINT32 unsigned int
#endif 


typedef struct
{
	char        make[64];
	char        model[64];
	char		software[64];
	unsigned    raw_count;
	unsigned    dng_version;
	unsigned    is_foveon;
	int         colors;
	unsigned    filters;
	char        xtrans[6][6];
	char        xtrans_abs[6][6];
	char        cdesc[5];
	unsigned    xmplen;
	char	    *xmpdata;

}libraw_iparams_t;

typedef struct
{
	unsigned    mix_green;
	unsigned    raw_color;
	unsigned    zero_is_bad;
	ushort      shrink;
	ushort      fuji_width;
} libraw_internal_output_params_t;

typedef struct
{
	ushort      raw_height,
				raw_width,
				height,
				width,
				top_margin,
				left_margin;
	ushort      iheight,
				iwidth;
	unsigned    raw_pitch;
	double      pixel_aspect;
	int         flip;
	int         mask[8][4];

} libraw_image_sizes_t;

typedef struct
{
	unsigned short illuminant;
	float calibration[4][4];
	float colormatrix[4][3];
} libraw_dng_color_t;


typedef struct
{
	int CanonColorDataVer;
	int CanonColorDataSubVer;
	int SpecularWhiteLevel;
	int AverageBlackLevel;
} libraw_canon_makernotes_t;



typedef struct
{
	ushort      curve[0x10000];
	unsigned    cblack[4102];
	unsigned    black;
	unsigned    data_maximum;
	unsigned    maximum;
	ushort      white[8][8];
	float       cam_mul[4];
	float       pre_mul[4];
	float       cmatrix[3][4];
	float       rgb_cam[3][4];
	float       cam_xyz[4][3];
	//struct ph1_t       phase_one_data;
	float       flash_used;
	float       canon_ev;
	char        model2[64];
	void        *profile;
	unsigned    profile_length;
	unsigned    black_stat[8];
	libraw_dng_color_t  dng_color[2];
	libraw_canon_makernotes_t canon_makernotes;
	float	      baseline_exposure;
	int			OlympusSensorCalibration[2];
	float       FujiExpoMidPointShift;
	int			digitalBack_color;
} libraw_colordata_t;


typedef struct
{
	/* the actual input DNG data*/
	unsigned char *inbuf;
	unsigned long inbuf_len;


	/* really allocated bitmap */
	void          *raw_alloc;
	/* alias to single_channel variant */
	ushort        *raw_image;
	/* alias to 4-channel variant */
	ushort(*color4_image)[4];
	/* alias to 3-color variand decoded by RawSpeed */
	ushort(*color3_image)[3];

	/* Phase One black level data; */
	short(*ph1_cblack)[2];
	short(*ph1_rblack)[2];

	/* save color and sizes here, too.... */
	libraw_iparams_t  iparams;
	libraw_image_sizes_t sizes;
	libraw_internal_output_params_t ioparams;
	libraw_colordata_t color;
} libraw_rawdata_t;


struct ph1_t
{
	int format, key_off, tag_21a;
	int t_black, split_col, black_col, split_row, black_row;
	float tag_210;
};



enum LibRaw_image_formats
{
	/*LIBRAW_IMAGE_JPEG = 1,
	LIBRAW_IMAGE_BITMAP = 2*/
	COMP_UNKNOWN = 0,
	UNCOMPRESSED = 1,
	PACKED_DNG = 2,
	LOSSLESS_JPEG = 3,
	BASELINE_JPEG =5,
	NON_BASELINE_JPEG = 6,
	ZIP_DEFLATE =7
};

typedef struct
{
	enum LibRaw_image_formats type;
	ushort      height,
		width,
		colors,
		bits;
	unsigned int  data_size;
	unsigned char data[1];
} libraw_processed_image_t;

enum LibRaw_thumbnail_formats
{
	LIBRAW_THUMBNAIL_UNKNOWN = 0,
	LIBRAW_THUMBNAIL_JPEG = 1,
	LIBRAW_THUMBNAIL_BITMAP = 2,
	LIBRAW_THUMBNAIL_LAYER = 4,
	LIBRAW_THUMBNAIL_ROLLEI = 5
};


typedef struct
{
	enum LibRaw_thumbnail_formats tformat;
	ushort      twidth,
				theight;
	unsigned    tlength;
	int         tcolors;

	char       *thumb;
}libraw_thumbnail_t;


typedef struct
{
	unsigned    greybox[4];     /* -A  x1 y1 x2 y2 */
	unsigned    cropbox[4];     /* -B x1 y1 x2 y2 */
	double      aber[4];        /* -C */
	double      gamm[6];        /* -g */
	float       user_mul[4];    /* -r mul0 mul1 mul2 mul3 */
	unsigned    shot_select;    /* -s */
	float       bright;         /* -b */
	float       threshold;      /*  -n */
	int         half_size;      /* -h */
	int         four_color_rgb; /* -f */
	int         highlight;      /* -H */
	int         use_auto_wb;    /* -a */
	int         use_camera_wb;  /* -w */
	int         use_camera_matrix; /* +M/-M */
	int         output_color;   /* -o */
	char        *output_profile; /* -o */
	char        *camera_profile; /* -p */
	char        *bad_pixels;    /* -P */
	char        *dark_frame;    /* -K */
	int         output_bps;     /* -4 */
	int         output_tiff;    /* -T */
	int         user_flip;      /* -t */
	int         user_qual;      /* -q */
	int         user_black;     /* -k */
	int			user_cblack[4];
	int         user_sat;       /* -S */

	int         med_passes;     /* -m */
	float       auto_bright_thr;
	float       adjust_maximum_thr;
	int         no_auto_bright; /* -W */
	int         use_fuji_rotate;/* -j */
	int         green_matching;
	/* DCB parameters */
	int         dcb_iterations;
	int         dcb_enhance_fl;
	int         fbdd_noiserd;
	/* VCD parameters */
	int         eeci_refine;
	int         es_med_passes;
	/* AMaZE*/
	int         ca_correc;
	float       cared;
	float	cablue;
	int cfaline;
	float linenoise;
	int cfa_clean;
	float lclean;
	float cclean;
	int cfa_green;
	float green_thresh;
	int exp_correc;
	float exp_shift;
	float exp_preser;
	/* WF debanding */
	int   wf_debanding;
	float wf_deband_treshold[4];
	/* Raw speed */
	int use_rawspeed;
	/* Disable Auto-scale */
	int no_auto_scale;
	/* Disable intepolation */
	int no_interpolation;
	/* Disable sRAW YCC to RGB conversion */
	int sraw_ycc;
	/* Force use x3f data decoding either if demosaic pack GPL2 enabled */
	int force_foveon_x3f;
	int x3f_flags;
	/* Sony ARW2 digging mode */
	int sony_arw2_options;
	int sony_arw2_posterization_thr;
	/* Nikon Coolscan */
	float coolscan_nef_gamma;
}libraw_output_params_t;

typedef struct
{
	float       iso_speed;
	float       shutter;
	float       aperture;
	float       focal_len;
	time_t      timestamp;
	unsigned    shot_order;
	unsigned    gpsdata[32];
	//libraw_gps_info_t parsed_gps;
	char        desc[512],
				artist[64];
} libraw_imgother_t;


typedef struct
{
	ushort(*image)[4];
	libraw_image_sizes_t        sizes;
	libraw_iparams_t            idata;
	//libraw_lensinfo_t			lens;
	libraw_output_params_t		params;
	unsigned int                progress_flags;
	unsigned int                process_warnings;
	libraw_colordata_t          color;
	libraw_imgother_t           other;
	libraw_thumbnail_t          thumbnail;
	// BEVARA: added multiple thumbnails to this structure
	libraw_thumbnail_t          thumbnail2;
	libraw_thumbnail_t          thumbnail3;
	libraw_thumbnail_t          thumbnail4;
	libraw_thumbnail_t          thumbnail5;

	// holds the original DNG data and data length
	libraw_rawdata_t            *rawdata;

	void						*parent_class;
} libraw_data_t;

//typedef struct
//{
//	unsigned long long LensID;
//	char	Lens[128];
//	ushort	LensFormat;    /* to characterize the image circle the lens covers */
//	ushort	LensMount;     /* 'male', lens itself */
//	unsigned long long  CamID;
//	ushort	CameraFormat;  /* some of the sensor formats */
//	ushort	CameraMount;   /* 'female', body throat */
//	char	body[64];
//	short	FocalType;       /* -1/0 is unknown; 1 is fixed focal; 2 is zoom */
//	char	LensFeatures_pre[16], LensFeatures_suf[16];
//	float	MinFocal, MaxFocal;
//	float	MaxAp4MinFocal, MaxAp4MaxFocal, MinAp4MinFocal, MinAp4MaxFocal;
//	float	MaxAp, MinAp;
//	float	CurFocal, CurAp;
//	float	MaxAp4CurFocal, MinAp4CurFocal;
//	float	LensFStops;
//	unsigned long long TeleconverterID;
//	char	Teleconverter[128];
//	unsigned long long AdapterID;
//	char	Adapter[128];
//	unsigned long long AttachmentID;
//	char	Attachment[128];
//	short CanonFocalUnits;
//	float	FocalLengthIn35mmFormat;
//} libraw_makernotes_lens_t;
//
//typedef struct
//{
//	float NikonEffectiveMaxAp;
//	uchar NikonLensIDNumber, NikonLensFStops, NikonMCUVersion, NikonLensType;
//} libraw_nikonlens_t;
//
//typedef struct
//{
//	float MinFocal, MaxFocal, MaxAp4MinFocal, MaxAp4MaxFocal;
//} libraw_dnglens_t;
//
//typedef struct
//{
//	float MinFocal, MaxFocal, MaxAp4MinFocal, MaxAp4MaxFocal, EXIF_MaxAp;
//	char LensMake[128], Lens[128];
//	ushort FocalLengthIn35mmFormat;
//	libraw_nikonlens_t nikon;
//	libraw_dnglens_t dng;
//	libraw_makernotes_lens_t makernotes;
//} libraw_lensinfo_t;


/* end libraw data structures */



/**
* \struct DNGLoader
* \brief Main structure of the muxer.
*
* This structure stored all the attribute of the muxer 
*/
typedef struct
{
	GF_ClientService *service;

	GF_SLHeader sl_hdr;

	/* dng handler*/
	libraw_data_t *iprc;
	unsigned long  DNG_data_length;

	/* raw data */
	LPNETCHANNEL ch_raw;
	libraw_processed_image_t *raw_data;
	Bool raw_done;

	/* thumb data */
	LPNETCHANNEL ch_thumb;
	char *thumb_data;
	u32 thumb_data_size;
	Bool thumb_done;

} DNGLoader;


/**** This section is out of acccessor scope ***/
static void DNG_SetupObject(DNGLoader *read);
static GF_ESD*  DNG_SetupRAW(libraw_processed_image_t *img);
static GF_ESD* DNG_GetThumbESD(libraw_thumbnail_t *thumb);
GF_BaseDecoder *NewRAWDec();
void DeleteRAWDec(GF_BaseDecoder *ifcg);


const char * DNG_MIME_TYPES[] = {
	"image/dng", "dng", "DNG Images",
	NULL
};

static u32 DNG_RegisterMimeTypes(const GF_InputService *plug) {
	u32 i;
	if (!plug) {
		GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("IMG_RegisterMimeTypes : plug is NULL !!\n"));
	}
	for (i = 0 ; DNG_MIME_TYPES[i]; i+=3)
		gf_service_register_mime(plug, DNG_MIME_TYPES[i], DNG_MIME_TYPES[i+1], DNG_MIME_TYPES[i+2]);
	return i/3;
}


static Bool DNG_CanHandleURL(GF_InputService *plug, const char *url)
{
	char *sExt;
	u32 i;
	GF_LOG(GF_LOG_MEDIA, GF_LOG_INFO, ("IMG_CanHandleURL(%s)\n", url));
	if (!plug || !url)
		return GF_FALSE;
	sExt = strrchr(url, '.');
	for (i = 0 ; DNG_MIME_TYPES[i]; i+=3) {
		if (gf_service_check_mime_register(plug, DNG_MIME_TYPES[i], DNG_MIME_TYPES[i+1], DNG_MIME_TYPES[i+2], sExt))
			return GF_TRUE;
	}
	return GF_FALSE;
}

static GF_Descriptor *IMG_GetServiceDesc(GF_InputService *plug, u32 expect_type, const char *sub_url)
{
	//	GF_ESD *esd;
	DNGLoader *read = (DNGLoader *)plug->priv;

	/*override default*/
	if (expect_type == GF_MEDIA_OBJECT_UNDEF) expect_type = GF_MEDIA_OBJECT_VIDEO;
	//read->srv_type = expect_type;

	/*visual object*/
	/*if (expect_type==GF_MEDIA_OBJECT_VIDEO) {
	GF_ObjectDescriptor *od = (GF_ObjectDescriptor *) gf_odf_desc_new(GF_ODF_OD_TAG);
	od->objectDescriptorID = 1;
	esd = IMG_GetESD(read);
	gf_list_add(od->ESDescriptors, esd);
	return (GF_Descriptor *) od;
	}*/

	return NULL;
}

static GF_Err IMG_ServiceCommand(GF_InputService *plug, GF_NetworkCommand *com)
{
	DNGLoader *read = (DNGLoader *)plug->priv;

	if (com->command_type == GF_NET_SERVICE_HAS_AUDIO) return GF_NOT_SUPPORTED;

	if (!com->base.on_channel) return GF_NOT_SUPPORTED;
	switch (com->command_type) {
	case GF_NET_CHAN_SET_PADDING:
		//read->pad_bytes = com->pad.padding_bytes;
		return GF_OK;
	case GF_NET_CHAN_DURATION:
		com->duration.duration = 0;
		return GF_OK;
	case GF_NET_CHAN_PLAY:
		/*note we don't handle range since we're only dealing with images*/
		/*if (read->ch == com->base.on_channel) {
		read->raw_done = GF_FALSE;
		read->thumb_done = GF_FALSE;
		}*/
		return GF_OK;
	case GF_NET_CHAN_STOP:
		return GF_OK;
	default:
		return GF_OK;
	}
}


/**** END : This section is out of acccessor scope ***/

/**** This section is in acccessor scope ***/

/**
* \fn static GF_Err DNG_ConnectService(GF_InputService *plug, GF_ClientService *serv, const char *url)
*
* \brief This is the first function called by the player to open the service
*
* \param plug This variable contains the attribute of the struct @DNGLoader
*
* \param serv This service of the player, it's usefull for pushing notification about how the demuxing of file goes
*
* \param url This is the url to open
*
* \return GF_OK is everything happend, otherwise a GF_Err.
*/


static GF_Err DNG_ConnectService(GF_InputService *plug, GF_ClientService *serv, const char *url)
{
	DNGLoader *read = (DNGLoader *)plug->priv;
	int ret;
	GF_Err err = GF_OK;

	read->service = serv;
	if (!url)
		return GF_BAD_PARAM;

	// TODO: here implement file passing from Access
	// populate the following with pointer to DNG data (iprc->rawdata->inbuf) and the length
	//	of the DNG data (iprc->rawdata->inbuf_len)
	//libraw_data_t *iprc;
	

	//ret = dng_open_file(read->iprc, url);
	/*if (ret) {
		printf("libraw  %s\n", libraw_strerror(ret));
		err = GF_NOT_SUPPORTED;
	}
*/


	gf_service_connect_ack(serv, NULL, err);
	if (ret != 0)
		return err;

	DNG_SetupObject(read);
	return err;
}

/**
*	DNG parsing structures and functions
*
*/

long DATALOC;
long DATALEN;
long XMPLOC;
long XMPLEN;

int parse_xmp(char *buf)
{
	int UTF = 0;
	int saveDATALOC = DATALOC;

	uchar tmpxmp;
	int ii;
	printf("XMP: ");
	for (ii = 0; ii<10; ++ii)
	{
		tmpxmp = *(buf + DATALOC + ii);
		printf("0x%02x ", tmpxmp);
	}

	/*determine UTF type 8,16,32 (UTF-8, UTF-16/UCS-2, UTF-32/UCS-4*/
	if ((*(buf + DATALOC) == 0x3C) && (*(buf + DATALOC + 1) == 0x3F))
	{
		UTF = 8;
	}
	else if ((*(buf + DATALOC) == 0x3C) && (*(buf + DATALOC + 1) == 0x00))
	{
		if (*(buf + DATALOC + 1) == 0x3F)
			UTF = 16;
		else if ((*(buf + DATALOC + 1) == 0x00) && (*(buf + DATALOC + 2) == 0x00) && (*(buf + DATALOC + 3) == 0x00))
			UTF = 32;
		else
		{
			return 0;
		}
	}
	else
	{
		printf("error parsing XMP\n");
		return 0;
	}

	/* printf("UTF = %d\n",UTF);  */

	if (UTF == 8)
	{
		while (!((*(buf + DATALOC) == 'e') && (*(buf + DATALOC + 1) == 'n') && (*(buf + DATALOC + 2) == 'd')))
		{
			/* printf("%c",*(buf+DATALOC)); */
			++DATALOC;
			if (DATALOC > DATALEN)
			{
				DATALOC = saveDATALOC;
				return 0;
			}
			++XMPLEN;
		}
		while ((*(buf + DATALOC) != '>'))
		{
			++DATALOC;
			if (DATALOC > DATALEN)
			{
				DATALOC = saveDATALOC;
				return 0;
			}
			++XMPLEN;
		}
	}
	else if ((UTF == 16) || (UTF == 32))
	{
		printf("UTF-16/32 XMP not yet parsed. Contact Bevara for support.\n");
		exit(0);
	}

	/* printf("XMP length = %ld\n",XMPLEN); */
	/* printf("XMP: (not printed) "); */
	/* for (ii=0;ii<XMPLEN+100;++ii) */
	/*   { */
	/*     tmpxmp = *(buf+saveDATALOC+ii); */
	/*     printf("%c",tmpxmp); */
	/*   } */

	return 1; /* success */
}

#define FORC(cnt) for (c=0; c < cnt; c++)
#define FORC3 FORC(3)
#define FORC4 FORC(4)
#define FORCC FORC(colors)
#define SQR(x) ((x)*(x))
#define SWAP(a,b) { a=a+b; b=a-b; a=a-b; }


short order;
char *meta_data, xtrans[6][6], xtrans_abs[6][6];
char cdesc[5], desc[512], make[64], model[64], model2[64], artist[64];
float flash_used, canon_ev, iso_speed, shutter, aperture, focal_len;
unsigned int  strip_offset, data_offset;
unsigned int thumb_offset, meta_offset, profile_offset;
unsigned shot_order, /* kodak_cbpp, */ exif_cfa, unique_id;
unsigned thumb_length, meta_length, profile_length;
unsigned int thumb_misc, *oprof, fuji_layout, shot_select = 0;
/* unsigned  multi_out=0; */
unsigned tiff_nifds, tiff_samples, tiff_bps, tiff_compress;
unsigned black, maximum, mix_green, raw_color/* , zero_is_bad */;
unsigned zero_after_ff, is_raw, dng_version, is_foveon, data_error;
unsigned tile_width, tile_length, gpsdata[32], load_flags;
unsigned flip, tiff_flip, filters, colors;
ushort raw_height, raw_width, height, width, top_margin, left_margin;
ushort shrink, iheight, iwidth, /* fuji_width, */ thumb_width, thumb_height;
ushort *raw_image, (*image)[4], cblack[4102];
ushort white[8][8], curve[0x10000], cr2_slice[3], sraw_mul[4];
double pixel_aspect, aber[4] = { 1,1,1,1 }, gamm[6] = { 0.45,4.5,0,0,0,0 };
float bright = 1, /* user_mul[4]={0,0,0,0}, */ threshold = 0;
int mask[8][4];
int half_size = 0, four_color_rgb = 0, /* document_mode=0, */ highlight = 0;
int load_raw, load_thumb;

int /* use_auto_wb=0, */ /* use_camera_wb=0, */ use_camera_matrix = 1;
int output_color = 1, output_bps = 8  /* , output_tiff=0 *//* , med_passes=0 */;
/* int no_auto_bright=0; */
/* unsigned greybox[4] = { 0, 0, UINT_MAX, UINT_MAX }; */
float cam_mul[4], pre_mul[4], cmatrix[3][4], rgb_cam[3][4];
const double xyz_rgb[3][3] = {			/* XYZ from RGB */
	{ 0.412453, 0.357580, 0.180423 },
	{ 0.212671, 0.715160, 0.072169 },
	{ 0.019334, 0.119193, 0.950227 } };
const float d65_white[3] = { 0.950456, 1, 1.088754 };
int histogram[4][0x2000];


/*****
BEVARA needs some local implementations
*********/

void *mymemset(void *s, int c, int n)
{
	const unsigned char uc = c;
	unsigned char *su;

	for (su = s; 0<n; ++su, --n)
		*su = uc;

	return (s);

}


/* handle the endianess */
const int endtest = 1;
#define is_bigendian() ( (*(char*)&endtest) == 0 )

void myswab(ushort *pixelsrc, ushort *pixeldst, int count) /*by def count is even, since fed in as even */
{
	if (count == 2)
	{
		*pixeldst = 0;
		*pixeldst = ((*pixelsrc >> 8) & 0x00FF) + ((*pixelsrc & 0x00FF) << 8);
	}
	else
		printf("problem with myswab() -- more than 16bits being swapped\n");

}

UINT32 myswabint(UINT32 a)
{
	return (((a >> 24) & 0x000000FF) + ((a >> 8 & 0x0000FF00)) + ((a & 0x000000FF) << 24) + ((a & 0x0000FF00) << 8));
}




/* string implementations from Plauger "The Standard C Library" */
int mystrlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc);

	return (sc - s);
}

int mystrncmp(const char *s1, const char *s2, int n)
{
	for (; 0<n; ++s1, ++s2, --n)
		if (*s1 != *s2)
			return ((*(unsigned char *)s1, *(unsigned char *)s2) ? -1 : +1);
		else if (*s1 == '\0')
			return (0);
	return (0);
}

int mystrcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; ++s1, ++s2)
	{
		if (*s1 == '\0')
		{
			return (0);
		}
	}
	return ((*(unsigned char*)s1 < *(unsigned char *)s2) ? -1 : +1);
}

char *mystrcpy(char *s1, const char *s2)
{
	char *s = s1;

	for (s = s1; (*s++ = *s2++) != '\0'; );
	return(s1);
}


char *my_memmem(char *haystack, size_t haystacklen,
	char *needle, size_t needlelen)
{
	char *c;
	for (c = haystack; c <= haystack + haystacklen - needlelen; c++)
		if (!memcmp(c, needle, needlelen))
			return c;
	return 0;
}
#define memmem my_memmem

#define strncasecmp my_strncasecmp

int my_strncasecmp(const char *s1, const char *s2, int n)
{
	if (n == 0)
		return 0;

	while (n-- != 0 && tolower(*s1) == tolower(*s2))
	{
		if (n == 0 || *s1 == '\0' || *s2 == '\0')
			break;
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}




char *my_strcasestr(char *haystack, const char *needle)
{
	char *c;
	for (c = haystack; *c; c++)
		if (!strncasecmp(c, needle, mystrlen(needle)))
			return c;
	return 0;
}
#define strcasestr my_strcasestr



struct tiff_ifd {
	int width, height, bps, comp, phint, offset, flip, samples, bytes, endoffset;
	int tile_width, tile_length;
	float shutter;
} tiff_ifd[MAX_IFDS];


struct jhead {
	int algo, bits, high, wide, clrs, sraw, psv, restart, vpred[6];
	ushort quant[64], idct[64], *huff[20], *free[20], *row;
};

char *myfgets(char *s, int num, char *buf)
{


	if (DATALOC >= DATALEN)
	{
		printf("error in data read...exiting...\n"); exit(0);
		/* return EOF; */
	}


	int i;
	for (i = 0; i<num; ++i)
	{
		*(s + i) = *(buf + DATALOC); ++DATALOC;
		if ((*(s + i) == '\n') || (DATALOC>DATALEN))
			return s;

	}

	return s;

}


int myfgetc(char *buf)
{
	if (DATALOC >= DATALEN)
		return EOF;
	char tmp;
	int rettmp = 255;  /* tmp gets forced to 0xffffffxx */
	tmp = *(buf + DATALOC);  ++DATALOC;
	return  rettmp & tmp;

}


ushort   sget2(uchar *s)
{
	if (order == 0x4949)		/* "II" means little-endian */
	{
		return s[0] | s[1] << 8;
	}
	else				/* "MM" means big-endian */
	{
		return s[0] << 8 | s[1];
	}
}
unsigned   sget4(uchar *s)
{
	if (order == 0x4949)
		return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
	else
		return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}
#define sget4(s) sget4((uchar *)s)


/* if data and system endian don't match, then switch */
ushort   myget2(char *inbuf)
{
	uchar str[2] = { 0xff,0xff };

	str[0] = (uchar)*(inbuf + DATALOC); ++DATALOC;
	str[1] = (uchar)*(inbuf + DATALOC); ++DATALOC;
	return sget2(str);
}



unsigned   myget4(char *inbuf)
{
	uchar str[4] = { 0xff,0xff,0xff,0xff };
	/* fread (str, 1, 4, ifp); */
	str[0] = *(inbuf + DATALOC); ++DATALOC;
	str[1] = *(inbuf + DATALOC); ++DATALOC;
	str[2] = *(inbuf + DATALOC); ++DATALOC;
	str[3] = *(inbuf + DATALOC); ++DATALOC;
	return sget4(str);
}


unsigned   getint(int type, char *buf)
{
	return type == 3 ? myget2(buf) : myget4(buf);
}

float   int_to_float(int i)
{
	union { int i; float f; } u;
	u.i = i;
	return u.f;
}

double   getreal(int type, char *buf)
{
	union { char c[8]; double d; } u;
	int i, rev;

	switch (type) {
	case 3: return (unsigned short)myget2(buf);
	case 4: return (unsigned int)myget4(buf);
	case 5:  u.d = (unsigned int)myget4(buf);
		return u.d / (unsigned int)myget4(buf);
	case 8: return (signed short)myget2(buf);
	case 9: return (signed int)myget4(buf);
	case 10: u.d = (signed int)myget4(buf);
		return u.d / (signed int)myget4(buf);
	case 11: return int_to_float(myget4(buf));
	case 12:
		/* BEVARA: local impl of byte-swapping */
		/* order 0x4949 is little-endian */
		/* ntohs(0x1234) == 0x1234) checks for big-endian */
		/* rev = 7 * ((order == 0x4949) == (ntohs(0x1234) == 0x1234)); */
		rev = 7 * ((order == 0x4949) == (is_bigendian()));
		for (i = 0; i < 8; i++)
			/* u.c[i ^ rev] = fgetc(ifp); */
			u.c[i ^ rev] = myfgetc(buf);
		return u.d;
	default:
		/* return fgetc(ifp); */
		return myfgetc(buf);
	}
}

void   read_shorts(ushort *pixel, int count, char *buf)
{
	int i;

	for (i = 0; i<count; ++i)
	{
		*(pixel + i) = (*(buf + DATALOC) << 8) && *(buf + DATALOC + 1);
		DATALOC = DATALOC + 2;
		if (DATALOC > DATALEN)
		{
			printf("reading a ushort, buffer exceeded...exiting....\n");
			exit(0);

		}
	}


	/* BEVARA: local implementation of byte-swapping, ntohs is not ANSI */
	/* if ((order == 0x4949) == (ntohs(0x1234) == 0x1234)) */
	/*   swab (pixel, pixel, count*2); */
	ushort tmppixel;
	tmppixel = *pixel;
	if ((order == 0x4949) && is_bigendian())
	{
		printf("pixel before myswab 0x%04x, ", tmppixel);
		myswab(&tmppixel, pixel, count * 2);
		printf("pixel after myswab 0x%04x, ", *pixel);
	}

	for (i = 0; i<count; ++i)
		printf("pixel after ushort= 0x%08x\n", *(pixel + i)); exit(0);
}


/*
Construct a decode tree according the specification in *source.
The first 16 bytes specify how many codes should be 1-bit, 2-bit
3-bit, etc.  Bytes after that are the leaf values.

For example, if the source is

{ 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

then the code is

00		0x04
010		0x03
011		0x05
100		0x06
101		0x02
1100		0x07
1101		0x01
11100		0x08
11101		0x09
11110		0x00
111110		0x0a
1111110		0x0b
1111111		0xff
*/
ushort *   make_decoder_ref(const uchar **source)
{
	int max, len, h, i, j;
	const uchar *count;
	ushort *huff;

	count = (*source += 16) - 17;
	for (max = 16; max && !count[max]; max--);
	huff = (ushort *)calloc(1 + (1 << max), sizeof *huff);
	if (!huff)
	{
		printf("error allocating memory for Huffman coded...exiting\n");
		exit(0);
		// TODO: return ????
	}
	huff[0] = max;
	for (h = len = 1; len <= max; len++)
		for (i = 0; i < count[len]; i++, ++*source)
			for (j = 0; j < 1 << (max - len); j++)
				if (h <= 1 << max)
					huff[h++] = len << 8 | **source;
	return huff;
}


void   tiff_get(unsigned int base,
	unsigned int *tag, unsigned int *type, unsigned int *len, unsigned int *save, char* buf)
{
	/* printf("in tiff-get current file loc = %d\n",DATALOC); */

	/* printf("buf[0] = 0x%02x, buf[1] = 0x%02x\n",*(buf+DATALOC),*(buf+1+DATALOC)); */

	*tag = myget2(buf);
	*type = myget2(buf);
	*len = myget4(buf);


	*save = /*ftell(ifp) + 4;*/ DATALOC + 4;


	/* printf("in tiff_get tag = 0x%08x, type = %d, len = %d, save = %d\n",*tag,*type,*len,*save); */

	if (*len * ("11124811248484"[*type < 14 ? *type : 0] - '0') > 4)
	{

		/* fseek (ifp, myget4(buf)+base, SEEK_SET); */
		DATALOC = myget4(buf) + base;
		/* printf("in len check set loc to %d\n",DATALOC); */
	}
}


void   pseudoinverse(double(*in)[3], double(*out)[3], int size)
{
	double work[3][6], num;
	int i, j, k;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 6; j++)
			work[i][j] = j == i + 3;
		for (j = 0; j < 3; j++)
			for (k = 0; k < size; k++)
				work[i][j] += in[k][i] * in[k][j];
	}
	for (i = 0; i < 3; i++) {
		num = work[i][i];
		for (j = 0; j < 6; j++)
			work[i][j] /= num;
		for (k = 0; k < 3; k++) {
			if (k == i) continue;
			num = work[k][i];
			for (j = 0; j < 6; j++)
				work[k][j] -= work[i][j] * num;
		}
	}
	for (i = 0; i < size; i++)
		for (j = 0; j < 3; j++)
			for (out[i][j] = k = 0; k < 3; k++)
				out[i][j] += work[j][k + 3] * in[i][k];
}

void   cam_xyz_coeff(float rgb_cam[3][4], double cam_xyz[4][3])
{
	double cam_rgb[4][3], inverse[4][3], num;
	int i, j, k;

	for (i = 0; i < colors; i++)		/* Multiply out XYZ colorspace */
		for (j = 0; j < 3; j++)
			for (cam_rgb[i][j] = k = 0; k < 3; k++)
				cam_rgb[i][j] += cam_xyz[i][k] * xyz_rgb[k][j];

	for (i = 0; i < colors; i++) {		/* Normalize cam_rgb so that */
		for (num = j = 0; j < 3; j++)		/* cam_rgb * (1,1,1) is (1,1,1,1) */
			num += cam_rgb[i][j];
		for (j = 0; j < 3; j++)
			cam_rgb[i][j] /= num;
		pre_mul[i] = 1 / num;
	}
	pseudoinverse(cam_rgb, inverse, colors);
	for (i = 0; i < 3; i++)
		for (j = 0; j < colors; j++)
			rgb_cam[i][j] = inverse[j][i];
}




void   parse_exif(int base, char *buf)
{
	unsigned kodak, entries, tag, type, len, save, c;
	double expo;


	kodak = !mystrncmp(make, "EASTMAN", 7) && tiff_nifds < 3;
	entries = myget2(buf);
	while (entries--) {
		tiff_get(base, &tag, &type, &len, &save, buf);
		switch (tag) {
		case 33434:  tiff_ifd[tiff_nifds - 1].shutter =
			shutter = getreal(type, buf);		break;
		case 33437:  aperture = getreal(type, buf);		break;
		case 34855:  iso_speed = myget2(buf);			break;
		case 36867:
		case 36868:  /* get_timestamp(0); */			break;
		case 37377:  if ((expo = -getreal(type, buf)) < 128)
			tiff_ifd[tiff_nifds - 1].shutter =
			shutter = pow(2, expo);		break;
		case 37378:  aperture = pow(2, getreal(type, buf) / 2);	break;
		case 37386:  focal_len = getreal(type, buf);		break;
		case 37500:  /*parse_makernote(base, 0, buf);*/
			printf("not handling makernote \n");
			exit(0);
			// TODO: return ????
			break;
		case 40962:  if (kodak) raw_width = myget4(buf);	break;
		case 40963:  if (kodak) raw_height = myget4(buf);	break;
		case 41730:
			if (myget4(buf) == 0x20002)
				for (exif_cfa = c = 0; c < 8; c += 2)
					
					exif_cfa |= myfgetc(buf) * 0x01010101 << c;
		}
		
		DATALOC = save;
	}
}

int   parse_tiff_ifd(int base, char *buf)
{
	unsigned entries, tag, type, len, plen = 16, save;
	int ifd, use_cm = 0, cfa, i, j, c; /* , ima_len=0; */
	char software[64], /* *cbuf, */ *cp;
	uchar cfa_pat[16], cfa_pc[] = { 0,1,2,3 }, tab[256];
	double cc[4][4], cm[4][3], cam_xyz[4][3], num;
	double ab[] = { 1,1,1,1 }, asn[] = { 0,0,0,0 }, xyz[] = { 1,1,1 };
	/* unsigned sony_curve[] = { 0,0,0,0,0,4095 }; */
	/* unsigned int *tmpbuf, sony_offset=0, sony_length=0, sony_key=0; */
	/* struct jhead jh; */
	/* FILE *sfp; */


	int hhh;

	printf("in parse tiff ifd...\n");

	if (tiff_nifds >= sizeof tiff_ifd / sizeof tiff_ifd[0])
		return 1;
	ifd = tiff_nifds++;
	printf("current ifd = %d\n", ifd);
	tiff_ifd[ifd].endoffset = 0;
	for (j = 0; j < 4; j++)
		for (i = 0; i < 4; i++)
			cc[j][i] = i == j;
	entries = myget2(buf);

	/* Bevara for checking IFDs */
	int totalentries = entries;
	printf("Number of entries in this IFD = %d\n", totalentries);


	if (entries > 512) return 1;

	while (entries--) {
		/* printf("going to tiff_get with entries = %d\n",entries); */
		tiff_get(base, &tag, &type, &len, &save, buf);
		/* printf("tag after tiff_get= %d\n",tag); */
		printf("tag in entry#%d of %d after tiff_get= %d\n", entries, totalentries, tag);


		switch (tag) {
		case 5:   width = myget2(buf);  break;
		case 6:   height = myget2(buf);  break;
		case 7:   width += myget2(buf);  break;
		case 9:   if ((i = myget2(buf))) filters = i;   break;
		case 17: case 18:
			if (type == 3 && len == 1)
				cam_mul[(tag - 17) * 2] = myget2(buf) / 256.0;
			break;
		case 23:
			if (type == 3) iso_speed = myget2(buf);
			break;
		case 28: case 29: case 30:
			cblack[tag - 28] = myget2(buf);
			cblack[3] = cblack[1];
			break;
		case 36: case 37: case 38:
			cam_mul[tag - 36] = myget2(buf);
			break;
		case 39:
			if (len < 50 || cam_mul[0]) break;
			/* fseek (ifp, 12, SEEK_CUR); */
			DATALOC = 12;
			FORC3 cam_mul[c] = myget2(buf);
			break;
		case 46:
			/* if (type != 7 || fgetc(ifp) != 0xff || fgetc(ifp) != 0xd8) break; */
			/* thumb_offset = ftell(ifp) - 2; */
			if (type != 7 || myfgetc(buf) != 0xff || myfgetc(buf) != 0xd8) break;
			thumb_offset = DATALOC - 2;
			thumb_length = len;
			printf("set thumb offset\n");
			break;
		case 61440:			/* Fuji HS10 table */
							/* fseek (ifp, myget4(buf)+base, SEEK_SET); */
			DATALOC = myget4(buf) + base;
			parse_tiff_ifd(base, buf);
			break;
		case 254: /* NewSubFileType */
				  /* printf("need to handle new subfile type\n"); */
				  /* mystrcpy(meta_table[TAGTBLNUM].tagname,"NewSubFileType"); */
				  /* meta_table[TAGTBLNUM].tagdata = malloc(20); */
				  /* int NSFType; */
				  /* NSFType = myget4(buf); */
			myget4(buf);
			/* if ( (NSFType & 0x000F) == 1) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Thumbnail Image"); */
			/* else if ( (NSFType & 0x000F) == 0) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Main Image"); */
			/* else if ( ((NSFType & 0x000F) == 4) || ((NSFType & 0x000F) == 5)) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Transparency Image"); */
			/* else if ( (NSFType & 0x001F) == 65537) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Alternate Image"); */
			/* else */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Unknown"); */
			/* ++TAGTBLNUM; */
			break;


		case 2: case 256: case 61441:	/* ImageWidth */
			tiff_ifd[ifd].width = getint(type, buf);
			printf("image width = %d\n", tiff_ifd[ifd].width);
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"ImageWidth"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(21); /\* from double max int size *\/ */
			/* myitoa(tiff_ifd[ifd].width,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */

			break;
		case 3: case 257: case 61442:	/* ImageHeight */
			tiff_ifd[ifd].height = getint(type, buf);
			printf("image height = %d\n", tiff_ifd[ifd].height);
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"ImageHeight"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(21); /\* from double max int size *\/ */
			/* myitoa(tiff_ifd[ifd].height,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */
			break;
		case 258:				/* BitsPerSample */
		case 61443:
			tiff_ifd[ifd].samples = len & 7;
			tiff_ifd[ifd].bps = getint(type, buf);
			/* printf("image samples=%d, bps = %d\n",tiff_ifd[ifd].samples,tiff_ifd[ifd].bps); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"BitsPerSample"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(21); /\* from double max int size *\/ */
			/* myitoa(tiff_ifd[ifd].bps,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */
			break;
		case 61446:
			printf("private tag 61446 not yet supported...contact Bevara\n"); exit(0);
			/* raw_height = 0; */
			/* if (tiff_ifd[ifd].bps > 12) break; */
			/* load_raw = &  packed_load_raw; */
			/* load_flags = myget4(buf) ? 24:80; */
			break;
		case 259:				/* Compression */
			tiff_ifd[ifd].comp = getint(type, buf);
			/* printf("image comp type = %d, read at dataloc = %ld\n",tiff_ifd[ifd].comp,DATALOC); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"Compression"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(32); */
			/* if ( tiff_ifd[ifd].comp == 1) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Uncompressed"); */
			/* else if ( tiff_ifd[ifd].comp == 7) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"TIFF/JPEG Baseline or Lossless"); */
			/* else if ( tiff_ifd[ifd].comp == 8) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"ZIP"); */
			/* else if ( tiff_ifd[ifd].comp == 34892) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Lossy JPEG"); */
			/* else */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Vendor Unique"); */
			/* ++TAGTBLNUM; */
			break;
		case 262:				/* PhotometricInterpretation */
			tiff_ifd[ifd].phint = myget2(buf);
			/* printf("image photometric = %d\n",tiff_ifd[ifd].phint); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"PhotometricInterpretation"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(14); */
			/* if ( tiff_ifd[ifd].phint == 1) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"BlackIsZero"); */
			/* else if ( tiff_ifd[ifd].phint == 2) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"RGB"); */
			/* else if ( tiff_ifd[ifd].phint == 6) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"YCbCr"); */
			/* else if ( tiff_ifd[ifd].phint == 32803) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"CFA"); */
			/* else */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Vendor Unique"); */
			/* ++TAGTBLNUM; */
			break;
		case 270:				/* ImageDescription */

			for (hhh = 0; hhh<512; ++hhh)
			{
				*(desc + hhh) = *(buf + DATALOC); ++DATALOC;
				if (DATALOC>DATALEN)
				{
					printf("buffer length exceeded...exiting...\n"); exit(0);
				}
			}

			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"ImageDescription"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(512); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagdata,desc); */
			/* ++TAGTBLNUM; */
			break;
		case 271:				/* Make */
			myfgets(make, 64, buf);
			/* printf("MAKE = %s\n",make); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"Make"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(64); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagdata,make); */
			/* ++TAGTBLNUM; */
			break;
		case 272:				/* Model */
			myfgets(model, 64, buf);
			/* printf("MODEL = %s\n",model); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"Model"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(64); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagdata,model); */
			/* ++TAGTBLNUM; */
			break;
		case 280:				/* Panasonic RW2 offset */
								/* if (type != 4) break; */
								/* load_raw = &  panasonic_load_raw; */
								/* load_flags = 0x2008; */
			printf("Panasonic not yet supported...contact Bevara\n"); exit(0);
		case 273:				/* StripOffset */
			tiff_ifd[ifd].offset = myget4(buf) + base;
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"StripOffset"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(64); */
			/* myitoa(tiff_ifd[ifd].offset,meta_table[TAGTBLNUM].tagdata);  */
			/* ++TAGTBLNUM; */
			break;
		case 513:				/* JpegIFOffset */
		case 61447:
			printf("JPEGIFOffset not yet supported..contact Bevara\n"); exit(0);
			/* tiff_ifd[ifd].offset = myget4(buf)+base; */
			/* if (!tiff_ifd[ifd].bps && tiff_ifd[ifd].offset > 0) { */
			/*   /\* fseek (ifp, tiff_ifd[ifd].offset, SEEK_SET); *\/ */
			/*   DATALOC = tiff_ifd[ifd].offset; */
			/*   if (ljpeg_start (&jh, 1, buf)) { */
			/*     tiff_ifd[ifd].comp    = 6; */
			/*     tiff_ifd[ifd].width   = jh.wide; */
			/*     tiff_ifd[ifd].height  = jh.high; */
			/*     tiff_ifd[ifd].bps     = jh.bits; */
			/*     tiff_ifd[ifd].samples = jh.clrs; */
			/*     if (!(jh.sraw || (jh.clrs & 1))) */
			/*       tiff_ifd[ifd].width *= jh.clrs; */
			/*     if ((tiff_ifd[ifd].width > 4*tiff_ifd[ifd].height) & ~jh.clrs) { */
			/*       tiff_ifd[ifd].width  /= 2; */
			/*       tiff_ifd[ifd].height *= 2; */
			/*     } */
			/*     i = order; */
			/*     parse_tiff (tiff_ifd[ifd].offset + 12,buf); */
			/*     order = i; */
			/*   } */
			/* } */
			break;
		case 274:				/* Orientation */
			tiff_ifd[ifd].flip = "50132467"[myget2(buf) & 7] - '0';
			printf("\t\t\tOrientation = %d\n", tiff_ifd[ifd].flip);
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"Orientation"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(14); */
			/* if ( tiff_ifd[ifd].flip == 1) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Visual Top"); */
			/* else if ( tiff_ifd[ifd].flip == 3) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Visual Bottom"); */
			/* else if ( tiff_ifd[ifd].flip == 6) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Visual RHS"); */
			/* else if ( tiff_ifd[ifd].flip == 8) */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Visual LHS"); */
			/* else */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Unknown"); */
			/* ++TAGTBLNUM; */
			break;
		case 277:				/* SamplesPerPixel */
			tiff_ifd[ifd].samples = getint(type, buf) & 7;
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"SamplesPerPixel"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(14); */
			/* if ( tiff_ifd[ifd].samples == 1) */
			/*   { */
			/*     if (tiff_ifd[ifd].phint == 1) */
			/*       mystrcpy(meta_table[TAGTBLNUM].tagdata,"grayscale"); */
			/*     else if (tiff_ifd[ifd].phint == 32803) */
			/*       mystrcpy(meta_table[TAGTBLNUM].tagdata,"CFA"); */
			/*     else */
			/*       mystrcpy(meta_table[TAGTBLNUM].tagdata,"Unknown"); */
			/*   } */
			/* else if ( tiff_ifd[ifd].samples == 3) */
			/*   { */
			/*     if (tiff_ifd[ifd].phint == 2) */
			/*       mystrcpy(meta_table[TAGTBLNUM].tagdata,"RGB"); */
			/*     else if (tiff_ifd[ifd].phint == 6) */
			/*       mystrcpy(meta_table[TAGTBLNUM].tagdata,"YCbCr"); */
			/*     else */
			/*       mystrcpy(meta_table[TAGTBLNUM].tagdata,"Unknown"); */
			/*   } */
			/* else */
			/*   mystrcpy(meta_table[TAGTBLNUM].tagdata,"Vendor Unique"); */
			/* ++TAGTBLNUM; */
			break;
		case 279:				/* StripByteCounts */
			tiff_ifd[ifd].bytes = myget4(buf);
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"StripByteCounts"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(22); */
			/* myitoa(tiff_ifd[ifd].bytes,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */
			break;
		case 514:  /*JPEG interchange format length */
		case 61448:
			printf("JPEG interchange format not yet supported...contact Bevara\n"); exit(0);
			/* tiff_ifd[ifd].bytes = myget4(buf); */
			break;
		case 61454:
			printf("private tags not yet supported...contact Bevara\n"); exit(0);
			FORC3 cam_mul[(4 - c) % 3] = getint(type, buf);
			break;
		case 305:  case 11:		/* Software */
			myfgets(software, 64, buf);
			if (!mystrncmp(software, "Adobe", 5) ||
				!mystrncmp(software, "dcraw", 5) ||
				!mystrncmp(software, "UFRaw", 5) ||
				!mystrncmp(software, "Bibble", 6) ||
				!mystrncmp(software, "Nikon Scan", 10) ||
				!mystrcmp(software, "Digital Photo Professional"))
				is_raw = 0;
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"Software"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(64); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagdata,software); */
			/* ++TAGTBLNUM; */
			break;
		case 306:				/* DateTime */
								/* mystrcpy(meta_table[TAGTBLNUM].tagname,"DateTime"); */
								/* meta_table[TAGTBLNUM].tagdata = malloc(20);  */
								/* int tt; */
								/* for (tt=0;tt<20;++tt) */
								/*   *(meta_table[TAGTBLNUM].tagdata + tt) = myfgetc(buf); */
								/* ++TAGTBLNUM; */

								/* get_timestamp(0); */

			break;
		case 315:				/* Artist */

			for (hhh = 0; hhh<64; ++hhh)
			{
				*(artist + hhh) = *(buf + DATALOC); ++DATALOC;
				if (DATALOC>DATALEN)
				{
					printf("buffer length exceeded...exiting...\n"); exit(0);
				}
			}

			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"Artist"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(64); */
			/* mystrcpy(meta_table[TAGTBLNUM].tagdata,artist); */
			/* ++TAGTBLNUM; */
			break;
		case 322:				/* TileWidth */
			tiff_ifd[ifd].tile_width = getint(type, buf);
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"TileWidth"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(22); */
			/* myitoa(tiff_ifd[ifd].tile_width,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */
			break;
		case 323:				/* TileLength */
			tiff_ifd[ifd].tile_length = getint(type, buf);
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"TileLength"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(22); */
			/* myitoa(tiff_ifd[ifd].tile_length,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */
			break;
		case 324:				/* TileOffsets */
								/* tiff_ifd[ifd].offset = len > 1 ? ftell(ifp) : myget4(buf); */
			tiff_ifd[ifd].offset = len > 1 ? DATALOC : myget4(buf);
			if (len == 1)
				tiff_ifd[ifd].tile_width = tiff_ifd[ifd].tile_length = 0;
			if (len == 4) {
				printf("Sinar not yet supported...exiting...\n"); exit(0);
				/* load_raw = &  sinar_4shot_load_raw; */
				/* is_raw = 5; */
			}
			/* mystrcpy(meta_table[TAGTBLNUM].tagname,"TileOffsets"); */
			/* meta_table[TAGTBLNUM].tagdata = malloc(22); */
			/* myitoa(tiff_ifd[ifd].offset,meta_table[TAGTBLNUM].tagdata); */
			/* ++TAGTBLNUM; */
			break;
		case 325:  /*TileByteCounts*/
			printf("Tile Bytes Counts\n");
			break;
		case 330:				/* SubIFDs */
			if (!mystrcmp(model, "DSLR-A100") && tiff_ifd[ifd].width == 3872) {
				/* load_raw = &  sony_arw_load_raw; */

				/* data_offset = myget4(buf)+base; */

				printf("Sony not yet supported...exiting...\n");
				exit(0);
				/* ifd++; */  break;
			}
			while (len--) {
				/* i = ftell(ifp); */
				/* fseek (ifp, myget4(buf)+base, SEEK_SET); */
				i = DATALOC;
				DATALOC = myget4(buf) + base;
				if (parse_tiff_ifd(base, buf)) break;
				/* fseek (ifp, i+4, SEEK_SET); */
				DATALOC = i + 4;
			}
			break;
		case 400:
			mystrcpy(make, "Sarnoff");
			maximum = 0xfff;
			break;
		case 700:  /* XMP Metadata*/
			printf("found XMP metadata..\n");
			XMPLEN = 0;
			XMPLOC = DATALOC;
			if (parse_xmp(buf) == 0)
			{
				printf("incorrect parse of XMP Metadata\n");
			}
			/* printf("new DATALOC after XMP = %ld\n",DATALOC); */
			break;
		case 33723:   /*IPTC Metadata */
			printf("IPTC metadata not yet supported...exiting\n"); exit(0);
			break;
		case 28688:
			/* FORC4 sony_curve[c+1] = myget2(buf) >> 2 & 0xfff; */
			/* for (i=0; i < 5; i++) */
			/*   for (j = sony_curve[i]+1; j <= sony_curve[i+1]; j++) */
			/*     curve[j] = curve[j-1] + (1 << i); */
			/* break; */
		case 29184:
			/* sony_offset = myget4(buf);   */
			/* break; */
		case 29185: /* sony_length = myget4(buf);  break; */
		case 29217: /* sony_key    = myget4(buf);  */
			printf("Sony not yet supported...exiting...\n"); exit(0);
			break;
		case 29264:
			printf("Minolta not yet supported...exiting...\n"); exit(0);
			/* parse_minolta (ftell(ifp),buf); */
			/* parse_minolta (DATALOC,buf); */
			/* raw_width = 0; */
			break;
		case 29443:
			FORC4 cam_mul[c ^ (c < 2)] = myget2(buf);
			break;
		case 29459:
			FORC4 cam_mul[c] = myget2(buf);
			i = (cam_mul[1] == 1024 && cam_mul[2] == 1024) << 1;
			SWAP(cam_mul[i], cam_mul[i + 1])
				break;
		case 33405:			/* Model2 */
							/* fgets (model2, 64, ifp); */
			myfgets(model2, 64, buf);
			printf("MODEL2 = %s\n", model2);
			break;
		case 33421:			/* CFARepeatPatternDim */
			if (myget2(buf) == 6 && myget2(buf) == 6)
			{
				filters = 9;

			}
			break;
		case 33422:			/* CFAPattern */
			if (filters == 9) {
				/* FORC(36) ((char *)xtrans)[c] = fgetc(ifp) & 3; */
				FORC(36) ((char *)xtrans)[c] = myfgetc(buf) & 3;
				break;
			}
		case 64777:			/* Kodak P-series */
			if ((plen = len) > 16) plen = 16;
			/* fread (cfa_pat, 1, plen, ifp); */
			int mm;
			for (mm = 0; mm<plen; ++mm)
			{
				*(cfa_pat + mm) = *(buf + DATALOC); ++DATALOC;
				if (DATALOC > DATALEN)
				{
					printf("buffer read error..exiting...\n"); exit(0);
				}
			}

			for (colors = cfa = i = 0; i < plen && colors < 4; i++) {
				colors += !(cfa & (1 << cfa_pat[i]));
				cfa |= 1 << cfa_pat[i];
			}
			if (cfa == 070) memcpy(cfa_pc, "\003\004\005", 3);	/* CMY */
			if (cfa == 072) memcpy(cfa_pc, "\005\003\004\001", 4);	/* GMCY */
			printf("have kodak P-series: going to guess cfa\n");
			goto guess_cfa_pc;
		case 33424:
		case 65024:
			printf("not handling KODAK...contact Bevara\n"); exit(0);
			/* DATALOC =  myget4(buf)+base; */
			/* parse_kodak_ifd (base, buf); */
			break;
		case 33434:			/* ExposureTime */
			tiff_ifd[ifd].shutter = shutter = getreal(type, buf);
			break;
		case 33437:			/* FNumber */
			aperture = getreal(type, buf);
			break;
		case 34306:			/* Leaf white balance */
			FORC4 cam_mul[c ^ 1] = 4096.0 / myget2(buf);
			break;
		case 34307:			/* Leaf CatchLight color matrix */
							/* for (hhh=0; hhh<7; ++hhh) */
							/*   { */
							/*     *(software+hhh) = *(buf+DATALOC); ++DATALOC; */
							/*     if (DATALOC>DATALEN)  */
							/*       { */
							/* 	printf("buffer length exceeded...exiting...\n"); exit(0); */
							/*       } */
							/*   } */

							/* if (mystrncmp(software,"MATRIX",6)) break; */
							/* colors = 4; */
							/* for (raw_color = i=0; i < 3; i++) { */
							/*   FORC4 fscanf (ifp, "%f", &rgb_cam[i][c^1]); */
							/*   if (!use_camera_wb) continue; */
							/*   num = 0; */
							/*   FORC4 num += rgb_cam[i][c]; */
							/*   FORC4 rgb_cam[i][c] /= num; */
							/* } */
							/* break; */
		case 34310:			/* Leaf metadata */
							/* parse_mos (ftell(ifp),buf); */

							/* parse_mos (DATALOC,buf); */
		case 34303:
			printf("Leaf not yet supported....exiting...\n"); exit(0);
			/* mystrcpy (make, "Leaf"); */
			break;
		case 34665:			/* EXIF tag */
							/* fseek (ifp, myget4(buf)+base, SEEK_SET); */
			printf("found EXIF data\n");
			DATALOC = myget4(buf) + base;
			parse_exif(base, buf);
			break;
			/* case 34853:			/\* GPSInfo tag *\/ */
			/* 	/\* fseek (ifp, myget4(buf)+base, SEEK_SET); *\/ */
			/* 	printf("found GPS data\n"); */
			/* 	DATALOC =  myget4(buf)+base;  */
			/* 	parse_gps (base,buf); */
			/* 	break; */
		case 34675:			/* InterColorProfile */
		case 50831:			/* AsShotICCProfile */
							/* profile_offset = ftell(ifp); */
			profile_offset = DATALOC;
			profile_length = len;
			break;
		case 37122:			/* CompressedBitsPerPixel */
			/* kodak_cbpp = */ myget4(buf);
			break;
		case 37386:			/* FocalLength */
			focal_len = getreal(type, buf);
			break;
		case 37393:			/* ImageNumber */
			shot_order = getint(type, buf);
			printf("shot order = %d\n", shot_order);
			break;
		case 37400:			/* old Kodak KDC tag */
			for (raw_color = i = 0; i < 3; i++) {
				getreal(type, buf);
				FORC3 rgb_cam[i][c] = getreal(type, buf);
			}
			break;
		case 40976:
			printf("Samsung not yet supported...exiting...\n"); exit(0);
			/* strip_offset = myget4(buf); */
			/* switch (tiff_ifd[ifd].comp) { */
			/*   case 32770: load_raw = &  samsung_load_raw;   break; */
			/*   case 32772: load_raw = &  samsung2_load_raw;  break; */
			/*   case 32773: load_raw = &  samsung3_load_raw;  break; */
			/* } */
			break;
		case 46275:			/* Imacon tags */
							/* mystrcpy (make, "Imacon"); */
							/* /\* data_offset = ftell(ifp); *\/ */
							/* data_offset = DATALOC; */
							/* printf("in IMACON data_offset = %d\n",(int)data_offset); */
							/* ima_len = len; */
							/* break; */
		case 46279:
			/* if (!ima_len) break; */
			/* /\* fseek (ifp, 38, SEEK_CUR); *\/ */
			/* DATALOC = 38; */
		case 46274:
			/* fseek (ifp, 40, SEEK_CUR); */
			/* DATALOC = 40; */
			/* raw_width  = myget4(buf); */
			/* raw_height = myget4(buf); */
			/* left_margin = myget4(buf) & 7; */
			/* width = raw_width - left_margin - (myget4(buf) & 7); */
			/* top_margin = myget4(buf) & 7; */
			/* height = raw_height - top_margin - (myget4(buf) & 7); */
			/* if (raw_width == 7262) { */
			/*   height = 5444; */
			/*   width  = 7244; */
			/*   left_margin = 7; */
			/* } */
			/* /\* fseek (ifp, 52, SEEK_CUR); *\/ */
			/* DATALOC = 52; */
			/* FORC3 cam_mul[c] = getreal(11,buf); */
			/* /\* fseek (ifp, 114, SEEK_CUR); *\/ */
			/* DATALOC = 114; */
			/* flip = (myget2(buf) >> 7) * 90; */
			/* if (width * height * 6 == ima_len) { */
			/*   if (flip % 180 == 90) SWAP(width,height); */
			/*   raw_width = width; */
			/*   raw_height = height; */
			/*   left_margin = top_margin = filters = flip = 0; */
			/* } */
			/* mystrcpy(model,"Ixpress"); */
			/* /\* sprintf (model, "Ixpress %d-Mp", height*width/1000000); *\/ */
			/* load_raw = &  imacon_full_load_raw; */
			/* if (filters) { */
			/*   if (left_margin & 1) filters = 0x61616161; */
			/*   load_raw = &  unpacked_load_raw; */
			/* } */
			/* maximum = 0xffff; */
			printf("Imacon not yet supported...exiting...\n"); exit(0);
			break;
		case 50454:			/* Sinar tag */
		case 50455:
			printf("Sinar not yet supported...exiting...\n"); exit(0);
			/* if (!(cbuf = (char *) malloc(len))) break; */

			/* for (hhh=0; hhh<len; ++hhh) */
			/*   { */
			/*     *(cbuf+hhh) = *(buf+DATALOC); ++DATALOC; */
			/*     if (DATALOC>DATALEN)  */
			/*       { */
			/* 	printf("buffer length exceeded...exiting...\n"); exit(0); */
			/*       } */
			/*   } */

			/* for (cp = cbuf-1; cp && cp < cbuf+len; cp = strchr(cp,'\n')) */
			/*   if (!mystrncmp (++cp,"Neutral ",8)) */
			/*     sscanf (cp+8, "%f %f %f", cam_mul, cam_mul+1, cam_mul+2); */
			/* free (cbuf); */
			break;
		case 50458:
			/* if (!make[0]) mystrcpy (make, "Hasselblad"); */
			/* break; */
		case 50459:			/* Hasselblad tag */
							/* i = order; */
							/* /\* j = ftell(ifp); *\/ */
							/* j=DATALOC; */
							/* c = tiff_nifds; */
							/* order = myget2(buf); */
							/* /\* fseek (ifp, j+(myget2(buf),myget4(buf)), SEEK_SET); *\/ */
							/* printf("DOUBLECHECK the mygets...exiting...\n"); exit(0); */
							/* DATALOC = j+(myget2(buf),myget4(buf)); */
							/* parse_tiff_ifd (j,buf); */
							/* maximum = 0xffff; */
							/* tiff_nifds = c; */
							/* order = i; */
			printf("Hasselblad not yet supported...exiting...\n"); exit(0);
			break;
		case 50706:			/* DNGVersion */
							/* FORC4 dng_version = (dng_version << 8) + fgetc(ifp); */
			FORC4 dng_version = (dng_version << 8) + myfgetc(buf);
			if (!make[0]) mystrcpy(make, "DNG");
			is_raw = 1;
			break;
		case 50707: /*DNGBackwardVersion */
			printf("UNUSED: DNG backward version\n");
			/* From DNG spec this is the oldest version of DNG for which */
			/* this file is compatible. Used in readers. */
			break;

		case 50708:			/* UniqueCameraModel */
			if (model[0]) break;
			/* fgets (make, 64, ifp); */
			myfgets(make, 64, buf);
			if ((cp = strchr(make, ' '))) {
				mystrcpy(model, cp + 1);
				*cp = 0;
			}
			break;

		case 50709: /*LocalizedCameraModel */
			printf("UNUSED: LocalizedCameraModel\n");
			break;

		case 50710:			/* CFAPlaneColor */
			printf("CFA plane color\n");
			if (filters == 9) break;
			if (len > 4) len = 4;
			colors = len;
			/* fread (cfa_pc, 1, colors, ifp); */
			/* myfread(cfa_pc, 1, colors, buf); */
			int ggg;
			for (ggg = 0; ggg<colors; ++ggg)
			{
				*(cfa_pc + ggg) = *(buf + DATALOC); ++DATALOC;
			}
			printf("GOT CFA PLANE COLOR:\n");
			for (ggg = 0; ggg<colors; ++ggg)
			{
				printf("\t 0x%02x\n", cfa_pc[ggg]);
			}


		guess_cfa_pc:

			FORCC tab[cfa_pc[c]] = c;
			cdesc[c] = 0;


			for (i = 16; i--; )
			{
				filters = filters << 2 | tab[cfa_pat[i % plen]];
			}

			filters -= !filters;

			break;
		case 50711:			/* CFALayout */
			if (myget2(buf) != 1)
			{
				printf("CFA layout is not rectangular...exiting...\n"); exit(0);
			}
			/* if (CFALayout == 2) fuji_width = 1; */

			break;
		case 291:
		case 50712:			/* LinearizationTable */
			printf("LinearizationTable not supported....Contact Bevara...\n");
			/* linear_table (len,buf); */
			break;
		case 50713:			/* BlackLevelRepeatDim */
			cblack[4] = myget2(buf);
			cblack[5] = myget2(buf);
			if (cblack[4] * cblack[5] > sizeof cblack / sizeof *cblack - 6)
				cblack[4] = cblack[5] = 1;
			break;
		case 61450:
			cblack[4] = cblack[5] = MIN(sqrt(len), 64);
		case 50714:			/* BlackLevel */
			if (!(cblack[4] * cblack[5]))
				cblack[4] = cblack[5] = 1;
			FORC(cblack[4] * cblack[5])
				cblack[6 + c] = getreal(type, buf);
			black = 0;
			break;
		case 50715:			/* BlackLevelDeltaH */
		case 50716:			/* BlackLevelDeltaV */
			for (num = i = 0; i < len; i++)
				num += getreal(type, buf);
			black += num / len + 0.5;
			break;
		case 50717:			/* WhiteLevel */
			maximum = getint(type, buf);
			break;
		case 50718:			/* DefaultScale */
			pixel_aspect = getreal(type, buf);
			pixel_aspect /= getreal(type, buf);
			break;

		case 50719: /*DefaultCropOrigin*/
			printf("UNUSED: DefaultCropOrigin\n");
			break;
		case 50720: /*DefaultCropSize*/
			printf("UNUSED: DefaultCropSize\n");
			break;

		case 50721:			/* ColorMatrix1 */
		case 50722:			/* ColorMatrix2 */
			FORCC for (j = 0; j < 3; j++)
				cm[c][j] = getreal(type, buf);
			use_cm = 1;
			break;
		case 50723:			/* CameraCalibration1 */
		case 50724:			/* CameraCalibration2 */
			for (i = 0; i < colors; i++)
				FORCC cc[i][c] = getreal(type, buf);
			break;
		case 50725: /*ReductionMatrix1*/
			printf("UNUSED: ReductionMatrix1\n");
			break;

		case 50726: /*ReductionMatrix2*/
			printf("UNUSED: ReductionMatrix2\n");
			break;
		case 50727:			/* AnalogBalance */
			FORCC ab[c] = getreal(type, buf);
			break;
		case 50728:			/* AsShotNeutral */
			FORCC asn[c] = getreal(type, buf);
			break;
		case 50729:			/* AsShotWhiteXY */
			xyz[0] = getreal(type, buf);
			xyz[1] = getreal(type, buf);
			xyz[2] = 1 - xyz[0] - xyz[1];
			FORC3 xyz[c] /= d65_white[c];
			break;

		case 50730: /* BaselineExposure*/
			printf("UNUSED: BaselineExposure\n");
			break;
		case 50731: /* BaselineNoise*/
			printf("UNUSED: BaselineNoise\n");
			break;
		case 50732: /* BaselineSharpness*/
			printf("UNUSED: BaselineSharpness\n");
			break;
		case 50733: /* BayerGreenSplit*/
			printf("UNUSED: BayerGreenSplit\n");
			break;
		case 50734: /* LinearResponseLimit*/
			printf("UNUSED: LinearResponseLimit\n");
			break;

		case 50735: /* CameraSerialNumber*/
			printf("UNUSED: CameraSerialNumber\n");
			break;
		case 50736: /* LensInfo*/
			printf("UNUSED: LensInfo\n");
			break;
		case 50737: /* ChromaBlurRadius*/
			printf("UNUSED: ChromaBlurRadius\n");
			break;
		case 50738: /* AntiAliasStrength*/
			printf("UNUSED: AntiAliasStrength\n");
			break;
		case 50739: /* ShadowScale*/
			printf("UNUSED: ShadowScale\n");
			break;

		case 50740:			/* DNGPrivateData */
			printf("Check if UNUSED: DNGPrivateData\n");
			if (dng_version) break;
			printf("Minolta not yet supported...contact Bevara\n"); exit(0);
			/* parse_minolta (j = myget4(buf)+base,buf); */
			/* fseek (ifp, j, SEEK_SET); */
			DATALOC = j;
			parse_tiff_ifd(base, buf);
			break;

		case 50741: /* MakerNoteSafety*/
			printf("UNUSED: MakerNoteSafety\n");
			break;



		case 50752:
			read_shorts(cr2_slice, 3, buf);
			break;
		case 50778: /*CalibrationIlluminant1*/
			printf("UNUSED: CalibrartionIlluminant1\n");
			break;
		case 50779: /*CalibrationIlluminant2*/
			printf("UNUSED: CalibrartionIlluminant2\n");
			break;
		case 50780: /*BestQualityScale*/
			printf("UNUSED: Best Quality Scale\n");
			break;
		case 50781: /*RawDataUniqueID*/
			printf("UNUSED: RawDataUniqueID\n");
			break;

		case 50827: /* OriginalRawFileName*/
			printf("UNUSED: OriginalRawFileName\n");
			break;
		case 50828: /* OriginalRawFileData*/
			printf("UNUSED: OriginalRawFileData\n");
			break;


		case 50829:			/* ActiveArea */
			top_margin = getint(type, buf);
			left_margin = getint(type, buf);
			height = getint(type, buf) - top_margin;
			width = getint(type, buf) - left_margin;
			break;
		case 50830:			/* MaskedAreas */
			for (i = 0; i < len && i < 32; i++)
				((int *)mask)[i] = getint(type, buf);
			black = 0;
			break;


		case 50832: /* AsShotPreProfileMatrix*/
			printf("UNUSED: AsShotPreProfileMatrix\n");
			break;

		case 50833: /* CurrentICCProfile*/
			printf("UNUSED: CurrentICCProfile\n");
			break;
		case 50834: /* CurrentPreProfileMatrix*/
			printf("UNUSED: CurrentPreProfileMatrix\n");
			break;

		case 50879: /* ColorimetricReference*/
			printf("UNUSED: ColorimetricReference\n");
			break;
		case 50931: /* CameraCalibrationSignature*/
			printf("UNUSED: CameraCalibrationSignature\n");
			break;
		case 50932: /* ProfileCalibrationSignature*/
			printf("UNUSED: ProfileCalibrationSignature\n");
			break;
		case 50933: /* ExtraCameraProfiles*/
			printf("UNUSED: ExtraCameraProfiles\n");
			break;
		case 50934: /* AsShotProfileName*/
			printf("UNUSED: AsShotProfileName\n");
			break;
		case 50935: /* NoiseReductionApplied*/
			printf("UNUSED: NoiseReductionApplied\n");
			break;
		case 50936: /* ProfileName*/
			printf("UNUSED: ProfileName\n");
			break;
		case 50937: /* ProfileHueSatMapDims*/
			printf("UNUSED: ProfileHueSatMapDims\n");
			break;

		case 50938: /* ProfileHueSatMapData1*/
			printf("UNUSED: ProfileHueSatMapData1\n");
			break;
		case 50939: /* ProfileHueSatMapData2*/
			printf("UNUSED: ProfileHueSatMapData2\n");
			break;
		case 50940: /* ProfileToneCurve*/
			printf("UNUSED: ProfileToneCurve\n");
			break;
		case 50941: /* ProfileEmbedPolicy*/
			printf("UNUSED: ProfileEmbedPolicy\n");
			break;
		case 50942: /* ProfileCopyright*/
			printf("UNUSED: ProfileCopyright\n");
			break;
		case 50964: /* ForwardMatrix1*/
			printf("UNUSED: ForwardMatrix1\n");
			break;
		case 50965: /* ForwardMatrix2*/
			printf("UNUSED: ForwardMatrix2\n");
			break;

		case 50966: /* PreviewApplicationName*/
			printf("UNUSED: PreviewApplicationName\n");
			break;

		case 50967: /* PreviewApplicationVersion*/
			printf("UNUSED: PreviewApplicationVersion\n");
			break;

		case 50968: /* PreviewSettingsName*/
			printf("UNUSED: PreviewSettingsName\n");
			break;

		case 50969: /* PreviewSettingsDigest*/
			printf("UNUSED: PreviewSettingsDigest\n");
			break;

		case 50970: /* PreviewColorSpace*/
			printf("UNUSED: PreviewColorSpace\n");
			break;

		case 50971: /* PreviewDateTime*/
			printf("UNUSED: PreviewDateTime\n");
			break;

		case 50972: /* RawImageDigest*/
			printf("UNUSED: RawImageDigest\n");
			break;

		case 50973: /* OriginalRawFileDigest*/
			printf("UNUSED: OriginalRawFileDigest\n");
			break;

		case 50974: /* SubTileBlockSize*/
			printf("UNUSED: SubTileBlockSize\n");
			break;


		case 50975: /* RowInterleaveFactor*/
			printf("UNUSED: RowInterleaveFactor\n");
			break;

		case 50981: /* ProfileLookTableDims*/
			printf("UNUSED: ProfileLookTableDims\n");
			break;

		case 50982: /* ProfileLookTableData*/
			printf("UNUSED: ProfileLookTableData\n");
			break;

		case 51008:			/* OpcodeList1 */
							/* meta_offset = ftell(ifp); */
			meta_offset = DATALOC;
			printf("UNUSED: OpcodeList1\n");
			break;

		case 51009:			/* OpcodeList2 */
							/* meta_offset = ftell(ifp); */
			meta_offset = DATALOC;
			printf("UNUSED: OpcodeList2\n");
			break;

		case 51022:			/* OpcodeList3 */
							/* meta_offset = ftell(ifp); */
			meta_offset = DATALOC;
			printf("UNUSED: OpcodeList3\n");
			break;

		case 51041: /* NoiseProfile*/
			printf("UNUSED: NoiseProfile\n");
			break;

			/* these are numbered in reverse order in the spec */
		case 51125: /* DefaultUserCrop*/
			printf("UNUSED: DefaultUserCrop\n");
			break;

		case 51110: /* DefaultBlackRender*/
			printf("UNUSED: DefaultBlackRender\n");
			break;


		case 51109: /* BaselineExposureOffset*/
			printf("UNUSED: BaselineExposureOffset\n");
			break;

		case 51108: /* ProfileLookTableEncoding*/
			printf("UNUSED: ProfileLookTableEncoding\n");
			break;

		case 51107: /* ProfileHueSatMapEncoding*/
			printf("UNUSED: ProfileHueDatMapEncoding\n");
			break;

		case 51089: /* OriginalDefaultFinalSize*/
			printf("UNUSED: OriginalDefaultFinalSize\n");
			break;

		case 51090: /* OriginalBestQualityFinalSize*/
			printf("UNUSED: OriginalBestQualityFinalSize\n");
			break;

		case 51091: /* OriginalDefaultCropSize*/
			printf("UNUSED: OriginalDefaultCropSize\n");
			break;



		case 51111: /* NewRawImageDigest*/
			printf("UNUSED: NewRawImageDigest\n");
			break;


		case 51112: /* RawToPreviewGain*/
			printf("UNUSED: RawToPreviewGain\n");
			break;




		case 64772:			/* Kodak P-series */

			printf("Kodak not yet supported....enxiting...\n"); exit(0);
			/* if (len < 13) break; */
			/* /\* fseek (ifp, 16, SEEK_CUR); *\/ */
			/* DATALOC = 16; */
			/* data_offset = myget4(buf); */
			/* /\* fseek (ifp, 28, SEEK_CUR); *\/ */
			/* DATALOC = 28; */
			/* data_offset += myget4(buf); */

			/* printf("data offset in Kodak = %d\n",(int)data_offset); */
			/* load_raw = &  packed_load_raw; */
			break;
		case 65026:
			/* if (type == 2) fgets (model2, 64, ifp); */
			if (type == 2) myfgets(model2, 64, buf);
		}





		DATALOC = save;

	}



	/* if (sony_length && (tmpbuf = (unsigned *) malloc(sony_length))) { */
	/*   /\* fseek (ifp, sony_offset, SEEK_SET); *\/ */
	/*   DATALOC = sony_offset; */
	/*   /\* fread (tmpbuf, sony_length, 1, ifp); *\/ */
	/*   /\* myfread (tmpbuf, sony_length, 1, buf); *\/ */


	/* 	for (hhh=0; hhh<sony_length; ++hhh) */
	/* 	  { */
	/* 	    *(tmpbuf+hhh) = *(buf+DATALOC); ++DATALOC; */
	/* 	    if (DATALOC>DATALEN)  */
	/* 	      { */
	/* 		printf("buffer length exceeded...exiting...\n"); exit(0); */
	/* 	      } */
	/* 	  } */
	/*   sony_decrypt (tmpbuf, sony_length/4, 1, sony_key); */
	/*   sfp = ifp; */
	/*   printf("need to implement SONY... exiting\n"); exit(0); */
	/* if ((ifp = tmpfile())) { */
	/*   fwrite (tmpbuf, sony_length, 1, ifp); */
	/*   fseek (ifp, 0, SEEK_SET); */
	/*   parse_tiff_ifd (-sony_offset,buf); */
	/*   fclose (ifp); */
	/* } */
	/* ifp = sfp; */
	/*   free (tmpbuf); */
	/* } */
	for (i = 0; i < colors; i++)
		FORCC cc[i][c] *= ab[i];
	if (use_cm) {
		FORCC for (i = 0; i < 3; i++)
			for (cam_xyz[c][i] = j = 0; j < colors; j++)
				cam_xyz[c][i] += cc[c][j] * cm[j][i] * xyz[i];
		cam_xyz_coeff(cmatrix, cam_xyz);
	}
	if (asn[0]) {
		cam_mul[3] = 0;
		FORCC cam_mul[c] = 1 / asn[c];
	}
	if (!use_cm)
		FORCC pre_mul[c] /= cc[c][c];

	return 0;
}

int   parse_tiff(int base, char *buf)
{
	int doff;
	int tmp;


	DATALOC = base;
	printf("in parse-tiff, start parsing at offset %d\n", (int)DATALOC);



	order = myget2(buf);

	if (order != 0x4949 && order != 0x4d4d) return 0;
	tmp = myget2(buf);


	while ((doff = myget4(buf))) {

		DATALOC = doff + base;
		if (parse_tiff_ifd(base, buf)) break;
	}
	return 1;
}

int   ljpeg_start(struct jhead *jh, int info_only, char *buf)
{
	ushort c, tag, len;
	uchar data[0xA00];
	const uchar *dp;
	int ii;

	mymemset(jh, 0, sizeof *jh);
	jh->restart = INT_MAX;

	if ((myfgetc(buf), myfgetc(buf)) != 0xd8)
	{
		printf("concat myfgetc checked, returning 0\n");
		return 0;
	}



	do {

		data[0] = *(buf + DATALOC); ++DATALOC;
		data[1] = *(buf + DATALOC); ++DATALOC;
		data[2] = *(buf + DATALOC); ++DATALOC;
		data[3] = *(buf + DATALOC); ++DATALOC;

		if (DATALOC > DATALEN)
		{
			printf("buffer read error\n");
			return 0;
		}
		tag = data[0] << 8 | data[1];
		/* printf("\t in ljpeg_start tag = 0x%04x ",tag); */
		len = (data[2] << 8 | data[3]) - 2;
		/* printf("\t len = 0x%04x\n",len); */
		if (tag <= 0xff00) return 0;

		for (ii = 0; ii < len; ++ii)
		{
			data[ii] = *(buf + DATALOC); ++DATALOC;
		}

		/* printf("tag in ljpeg_start = 0x%04x\n",tag); */

		switch (tag) {
		case 0xffc3:
			jh->sraw = ((data[7] >> 4) * (data[7] & 15) - 1) & 3;
		case 0xffc1:
		case 0xffc0:
			jh->algo = tag & 0xff;
			jh->bits = data[0];
			jh->high = data[1] << 8 | data[2];
			jh->wide = data[3] << 8 | data[4];
			jh->clrs = data[5] + jh->sraw;
			if (len == 9 && !dng_version) myfgetc(buf);
			break;
		case 0xffc4:
			if (info_only) break;
			/* printf("in c4....going to len = %d\n",len); */
			for (dp = data; dp < data + len && !((c = *dp++) & -20); )
				jh->free[c] = jh->huff[c] = make_decoder_ref(&dp);
			break;
		case 0xffda:
			jh->psv = data[1 + data[0] * 2];
			jh->bits -= data[3 + data[0] * 2] & 15;
			break;
		case 0xffdb:
			FORC(64) jh->quant[c] = data[c * 2 + 1] << 8 | data[c * 2 + 2];
			break;
		case 0xffdd:
			jh->restart = data[0] << 8 | data[1];
		}
	} while (tag != 0xffda);
	if (info_only) return 1;
	if (jh->clrs > 6 || !jh->huff[0]) return 0;
	FORC(19) if (!jh->huff[c + 1]) jh->huff[c + 1] = jh->huff[c];
	if (jh->sraw) {
		FORC(4)        jh->huff[2 + c] = jh->huff[1];
		FORC(jh->sraw) jh->huff[1 + c] = jh->huff[0];
	}
	jh->row = (ushort *)calloc(jh->wide*jh->clrs, 4);
	if (!jh->row)
	{
		printf("Out of memory in allocating JPEG row...exiting\n");
		exit(0);
		// TODO: return ????
	}
	
	return zero_after_ff = 1;
}



void   apply_tiff(char *buf)
{
	int max_samp = 0, ties = 0, os, ns, raw = -1, i;
	struct jhead jh;

	thumb_misc = 16;
	if (thumb_offset) {
		DATALOC = thumb_offset;
		if (ljpeg_start(&jh, 1, buf)) {
			thumb_misc = jh.bits;
			thumb_width = jh.wide;
			thumb_height = jh.high;
		}
	}
	for (i = tiff_nifds; i--; ) {
		if (tiff_ifd[i].shutter)
			shutter = tiff_ifd[i].shutter;
		tiff_ifd[i].shutter = shutter;
	}
	for (i = 0; i < tiff_nifds; i++) {
		if (max_samp < tiff_ifd[i].samples)
			max_samp = tiff_ifd[i].samples;
		if (max_samp > 3) max_samp = 3;
		os = raw_width*raw_height;
		ns = tiff_ifd[i].width*tiff_ifd[i].height;
		if ((tiff_ifd[i].comp != 6 || tiff_ifd[i].samples != 3) &&
			(tiff_ifd[i].width | tiff_ifd[i].height) < 0x10000 &&
			ns && ((ns > os && (ties = 1)) ||
				(ns == os && shot_select == ties++))) {
			raw_width = tiff_ifd[i].width;
			raw_height = tiff_ifd[i].height;
			tiff_bps = tiff_ifd[i].bps;
			tiff_compress = tiff_ifd[i].comp;
			data_offset = tiff_ifd[i].offset;
			tiff_flip = tiff_ifd[i].flip;
			tiff_samples = tiff_ifd[i].samples;
			tile_width = tiff_ifd[i].tile_width;
			tile_length = tiff_ifd[i].tile_length;
			shutter = tiff_ifd[i].shutter;
			raw = i;


		}
	}
	if (is_raw == 1 && ties) is_raw = ties;
	if (!tile_width) tile_width = INT_MAX;
	if (!tile_length) tile_length = INT_MAX;
	for (i = tiff_nifds; i--; )
		if (tiff_ifd[i].flip) tiff_flip = tiff_ifd[i].flip;
	if (raw >= 0 && !load_raw)
		printf("setting load raw\n");
	switch (tiff_compress) {
	case 0:
	case 1:
		load_raw = PACKED_DNG;
		break;  /* this case expected only for thumbnails, handled separately*/
	case 7:
		load_raw = LOSSLESS_JPEG;  /*main image load */
		break;
	case 8:
		break; /* this case not yet handled */
	case 34892: break;  /* lossy JPEG not yet handled */
	default: is_raw = 0;
	}


	if (!dng_version)
		/* if ( (tiff_samples == 3 && tiff_ifd[raw].bytes && tiff_bps != 14 && */
		/* 	  (tiff_compress & -16) != 32768) */
		/*   || (tiff_bps == 8 && !strcasestr(make,"Kodak") && */
		/* 	  !strstr(model2,"DEBUG RAW"))) */
		/*   is_raw = 0; */
	{
		printf("DNG version not found....exiting...\n");
		exit(0);
	}


	int thm;
	for (i = 0; i < tiff_nifds; i++)
	{
		if (i != raw && tiff_ifd[i].samples == max_samp &&
			tiff_ifd[i].width * tiff_ifd[i].height / (SQR(tiff_ifd[i].bps) + 1) >
			thumb_width *       thumb_height / (SQR(thumb_misc) + 1)
			/* && tiff_ifd[i].comp != 34892 */)
		{
			thumb_width = tiff_ifd[i].width;
			thumb_height = tiff_ifd[i].height;
			thumb_offset = tiff_ifd[i].offset;
			thumb_length = tiff_ifd[i].bytes;
			thumb_misc = tiff_ifd[i].bps;
			thm = i;
			/* printf("\tthumbnail found, ifd#= %d, thumb_offset = %ld\n",i,thumb_offset); */
			/* printf("\t\t thumb width and height = %d %d \n",thumb_width,thumb_height); */
			/* printf("\t\t thumb comp type = %d\n",tiff_ifd[i].comp); */
			/* printf("\t\t thumb bps = %d\n",tiff_ifd[i].bps); */

			/* BEVARA: choose which thumbnail to display, unncompressed or baseline
			JPEG. These are the only two options supported at the moment */
			if (tiff_ifd[i].comp == 1) /* we found a simple thumbnail, so stop */
			{

				load_thumb = UNCOMPRESSED; /* placeholder */
			}
			else if (tiff_ifd[i].comp == 7) /* we found a TIFF/JPEG, so stop */
			{
				load_thumb = LOSSLESS_JPEG; /* placeholder */
				
			}
			else if (tiff_ifd[i].comp == 34892) /* we found a lossy JPEG, so stop */
			{
				printf("found non-baseline JPEG thumbnail\n");
				load_thumb = NON_BASELINE_JPEG;
			}
			else
				load_thumb = COMP_UNKNOWN;

		}
	}
	//TODO: roll into each thumbnail
	if (thm >= 0) {
		thumb_misc |= tiff_ifd[thm].samples << 5;
		
	}
}




/*
Identify which camera created this file, and set global variables
accordingly.
*/
void   identify(unsigned char *buf,unsigned int insize)
{

	static const char *corp[] =
	{ "AgfaPhoto", "Canon", "Casio", "Epson", "Fujifilm",
		"Mamiya", "Minolta", "Motorola", "Kodak", "Konica", "Leica",
		"Nikon", "Nokia", "Olympus", "Pentax", "Phase One", "Ricoh",
		"Samsung", "Sigma", "Sinar", "Sony" };
	char head[32], *cp;
	int flen, fsize, i, c;
	
	tiff_flip = flip = filters = UINT_MAX;	/* unknown */
										
	raw_height = raw_width = /* fuji_width = */ fuji_layout = cr2_slice[0] = 0;
	maximum = height = width = top_margin = left_margin = 0;
	cdesc[0] = desc[0] = artist[0] = make[0] = model[0] = model2[0] = 0;
	iso_speed = shutter = aperture = focal_len = unique_id = 0;
	tiff_nifds = 0;


	mymemset(tiff_ifd, 0, sizeof tiff_ifd);
	mymemset(gpsdata, 0, sizeof gpsdata);
	mymemset(cblack, 0, sizeof cblack);
	mymemset(white, 0, sizeof white);
	mymemset(mask, 0, sizeof mask);


	thumb_offset = thumb_length = thumb_width = thumb_height = 0;

	load_raw = 0;

	data_offset = meta_offset = meta_length = tiff_bps = tiff_compress = 0;
	printf("init data offset\n");
	/* kodak_cbpp = */ zero_after_ff = dng_version = load_flags = 0;
	/* timestamp = */ shot_order = tiff_samples = black = is_foveon = 0;
	mix_green = profile_length = data_error = /* zero_is_bad = */ 0;
	pixel_aspect = is_raw = raw_color = 1;
	tile_width = tile_length = 0;



	for (i = 0; i < 4; i++) {
		cam_mul[i] = i == 1;
		pre_mul[i] = i < 3;
		FORC3 cmatrix[c][i] = 0;
		FORC3 rgb_cam[c][i] = c == i;
	}
	colors = 3;
	for (i = 0; i < 0x10000; i++) curve[i] = i;

	/* header bytes 0-1 for endianness */
	order = myget2(buf);
	DATALOC = 0;  /*reset data ptr to read entire header */

	/* This is a general header read, useful for raw as well as TIFF/DNG */
	int hloop;
	for (hloop = 0; hloop<32; ++hloop)
	{
		head[hloop] = *(buf + DATALOC); ++DATALOC;
	}

	flen = fsize = insize;

	if (order == 0x4949 || order == 0x4d4d)
	{

		printf("we found endian flag \n");

		if (!memcmp(head + 6, "HEAPCCDR", 8))
		{
			printf("not yet handling HEAPCCDR (Canon CRW)...contact Bevara for suppport...\n"); exit(0);
			// return ???BAD_PARAM?????

		}
		else if (parse_tiff(0, buf))
		{
			apply_tiff(buf);
		}
	}
	else
	{
		printf("Expected DNG header not found...exiting...\n");
		exit(0);
		// return ??????
	}


	if (make[0] == 0)

	{
		printf("Camera Make not identified\n"); exit(0);
		//return ??????;
	}

	for (i = 0; i < sizeof corp / sizeof *corp; i++)
		if (strcasestr(make, corp[i]))	/* Simplify company names */
		{
			mystrcpy(make, corp[i]);

		}
	if ((!mystrcmp(make, "Kodak") || !mystrcmp(make, "Leica")) &&
		((cp = strcasestr(model, " DIGITAL CAMERA")) ||
			(cp = strstr(model, "FILE VERSION"))))
		*cp = 0;
	if (!strncasecmp(model, "PENTAX", 6))
		mystrcpy(make, "Pentax");

	cp = make + mystrlen(make);		/* Remove trailing spaces */
	while (*--cp == ' ') *cp = 0;
	cp = model + mystrlen(model);
	while (*--cp == ' ') *cp = 0;
	i = mystrlen(make);			/* Remove make from model */
	if (!strncasecmp(model, make, i) && model[i++] == ' ')
		memmove(model, model + i, 64 - i);
	if (!mystrncmp(model, "FinePix ", 8))
		mystrcpy(model, model + 8);
	if (!mystrncmp(model, "Digital Camera ", 15))
		mystrcpy(model, model + 15);
	desc[511] = artist[63] = make[63] = model[63] = model2[63] = 0;
	if (!is_raw) goto notraw;



	if (!height) height = raw_height;
	if (!width)  width = raw_width;
	if (height == 2624 && width == 3936)	/* Pentax K10D and Samsung GX10 */
	{
		height = 2616;   width = 3896;
	}
	if (height == 3136 && width == 4864)  /* Pentax K20D and Samsung GX20 */
	{
		height = 3124;   width = 4688; filters = 0x16161616;
	}
	if (width == 4352 && (!mystrcmp(model, "K-r") || !mystrcmp(model, "K-x")))
	{
		width = 4309; filters = 0x16161616;
	}
	if (width >= 4960 && !mystrncmp(model, "K-5", 3))
	{
		left_margin = 10; width = 4950; filters = 0x16161616;
	}
	if (width == 4736 && !mystrcmp(model, "K-7"))
	{
		height = 3122;   width = 4684; filters = 0x16161616; top_margin = 2;
	}
	if (width == 6080 && !mystrcmp(model, "K-3"))
	{
		left_margin = 4;  width = 6040;
	}
	if (width == 7424 && !mystrcmp(model, "645D"))
	{
		height = 5502;   width = 7328; filters = 0x61616161; top_margin = 29;
		left_margin = 48;
	}
	if (height == 3014 && width == 4096)	/* Ricoh GX200 */
		width = 4014;
	if (dng_version) {
		if (filters == UINT_MAX) filters = 0;

		if (filters) is_raw *= tiff_samples;
		else	 colors = tiff_samples;
		switch (tiff_compress) {
		case 0:

			//TODO: Set the Accessor type here!
		case 1:    printf("\t setting decompression to packed dng load raw\n");  //load_raw = &packed_dng_load_raw;  break;
		case 7:    printf("\t setting decompression to lossless dng load raw\n"); //load_raw = &lossless_dng_load_raw;  break;
		case 34892:
		{
			/* load_raw = &  lossy_dng_load_raw;  */
			printf("not handling lossy DNGs. Contact Bevara for suppport.\n");
			exit(0);
			// TODO: return value
			break;

		}
		default:    load_raw = 0;
		}
		goto dng_skip;
	}


dng_skip:
	if ((use_camera_matrix & (/* use_camera_wb || */ dng_version))
		&& cmatrix[0][0] > 0.125) {
		memcpy(rgb_cam, cmatrix, sizeof cmatrix);
		raw_color = 0;
	}
	/* if (raw_color) adobe_coeff (make, model); */
	/* if (load_raw == &  kodak_radc_load_raw) */
	/*   { */
	/*     printf("not currently supporting kodak radc...exiting...\n"); */
	/*   /\* if (raw_color) adobe_coeff ("Apple","Quicktake"); *\/ */
	/*   } */
	/* if (fuji_width) { */
	/*   fuji_width = width >> !fuji_layout; */
	/*   filters = fuji_width & 1 ? 0x94949494 : 0x49494949; */
	/*   width = (height >> fuji_layout) + fuji_width; */
	/*   height = width - 1; */
	/*   pixel_aspect = 1; */
	/* } else { */
	if (raw_height < height) raw_height = height;
	if (raw_width  < width) raw_width = width;
	/* } */
	if (!tiff_bps) tiff_bps = 12;
	if (!maximum) maximum = (1 << tiff_bps) - 1;
	if (!load_raw || height < 22 || width < 22 ||
		tiff_bps > 16 || tiff_samples > 6 || colors > 4)
		is_raw = 0;


	if (!cdesc[0])
		mystrcpy(cdesc, colors == 3 ? "RGBG" : "GMCY");
	if (!raw_height) raw_height = height;
	if (!raw_width) raw_width = width;
	if (filters > 999 && colors == 3)
		filters |= ((filters >> 2 & 0x22222222) |
			(filters << 2 & 0x88888888)) & filters << 1;
notraw:
	if (flip == UINT_MAX) flip = tiff_flip;
	if (flip == UINT_MAX) flip = 0;
}






/**
*	\fn static GF_Err DNG_Parse()
*
*	\brief This function is called by DG_SetupObject to 
*	parse the DNG, extract TAGs into structures, and 
*	locate the main image, thumbnails, and XMP 
*/

static GF_Err DNG_Parse(libraw_rawdata_t *rawdata)
{

	u32 ret = 0;


	/* pointer to input data stream */
	/* for reading data */
	DATALOC = 0;	
	DATALEN = rawdata->inbuf_len;

	ret = (identify(rawdata->inbuf, DATALEN), !is_raw);


	/* TODO: copy over all params into libraw structs */



	/*printf("After identify the DNG found height and width to be: %d  %d %d\n", height, width, colors);
	if (*outbuf_sz < height*width*colors*output_bps / 8)
	{
		*outbuf_sz = height*width*colors*output_bps / 8;
		DATALOC = 0;
		printf("returning for more output buffer\n");
		return GF_BUFFER_TOO_SMALL;
	}

	out_size = height*width*colors*output_bps / 8;
	printf("height = %d, width = %d, colors = %d, output_bps = %d\n", height, width, colors, output_bps);
*/

	if (!is_raw)
	{
		printf("is not a raw file....exiting\n");
		return GF_NOT_SUPPORTED;
	}
	

	/*shrink = filters && (half_size || (!identify_only &&
		(threshold || aber[0] != 1 || aber[2] != 1)));
	iheight = (height + shrink) >> shrink;
	iwidth = (width + shrink) >> shrink;


	if (meta_length) {
		meta_data = (char *)malloc(meta_length);
		merror(meta_data, "main()");
	}
	if (filters || colors == 1) {
		raw_image = (ushort *)calloc((raw_height + 7), raw_width * 2);
		merror(raw_image, "main()");
	}
	else {
		image = (ushort(*)[4]) calloc(iheight, iwidth*sizeof *image);
		merror(image, "main()");
	}

	if (shot_select >= is_raw)
		printf(" \"-s %d\" requests a nonexistent image!\n", shot_select);

	DATALOC = data_offset;*/


}


/**
* \fn static void DNG_SetupObject(DNGLoader *read)
*
* \brief This function is called by DNG_ConnectService. 
* It aims at declaring the objects contained in the DNG files.
*
* \param read The muxer
*/
static void DNG_SetupObject(DNGLoader *read)
{
	GF_ESD *esd;
	GF_ObjectDescriptor *od;
	u32 ret = 0;

	/* Read the TIFF/DNG info to populate the DNG structures */
	DNG_Parse(read->iprc->rawdata);


	/* TODO: Setup raw image */
	// BEVARA -- we have options here:
	// 1. most DNGs will be single main image, to handle the various
	//	tile offsets, we just point to the entire file, with 
	//	the IFD offset for the start of the main image
	// 2. Could offset to the main image IFD; later references, e.g., to 
	//	the tiles would then have to be offset
	// For now, we go with option 1
	ret = libraw_unpack(read->iprc);
	if (ret) {
		printf("libraw : error decoding raw image \n");
	}

	/*ret = libraw_dcraw_process(read->iprc);
	if (ret) {
		printf("libraw : error decoding raw image \n");
	}
*/
	//TODO: this is just here for check decompress??
	/*read->raw_data = libraw_dcraw_make_mem_image(read->iprc, &ret);
	if (ret) {
		printf("libraw : error decompressing raw image \n");
	}*/

	od = (GF_ObjectDescriptor *)gf_odf_desc_new(GF_ODF_OD_TAG);
	esd = DNG_SetupRAW(read->raw_data);
	od->objectDescriptorID = esd->ESID;
	gf_list_add(od->ESDescriptors, esd);
	gf_service_declare_media(read->service, (GF_Descriptor*)od, GF_FALSE);

	/* TODO: Setup thumbnail(s) */
	/*ret = libraw_unpack_thumb(read->iprc);
	if (ret) {
		printf("libraw : error unpacking thumbnail \n");
	}
	od = (GF_ObjectDescriptor *)gf_odf_desc_new(GF_ODF_OD_TAG);
	esd = DNG_GetThumbESD(&read->iprc->thumbnail);
	od->objectDescriptorID = esd->ESID;
	gf_list_add(od->ESDescriptors, esd);
	gf_service_declare_media(read->service, (GF_Descriptor*)od, GF_TRUE);*/

}

/**
* \fn static GF_ESD*  DNG_SetupRAW(libraw_processed_image_t *img)
*
* \brief This function is called by DNG_SetupObject.
* It aims at declaring the descriptor of the raw image contained in the DNG files.
*
* \param read The raw image descriptor of LIBRAW
*
* \return The raw image descriptor for the player.
*/
static GF_ESD*  DNG_SetupRAW(libraw_processed_image_t *img)
{
	GF_ESD *esd;

	esd = gf_odf_desc_esd_new(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = GF_STREAM_VISUAL;
	esd->ESID = 1;	
	esd->decoderConfig->objectTypeIndication = GPAC_OTI_IMAGE_RAW;
	
	GF_BitStream *bs_dsi = gf_bs_new(NULL, 0, GF_BITSTREAM_WRITE);
	gf_bs_write_u32(bs_dsi, img->height);
	gf_bs_write_u32(bs_dsi, img->width);
	gf_bs_write_u32(bs_dsi, GF_PIXEL_RGB_24);
	gf_bs_write_u8(bs_dsi, 3);
	gf_bs_get_content(bs_dsi, &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
	gf_bs_del(bs_dsi);

	return esd;
}

/**
* \fn static GF_ESD* DNG_GetThumbESD(libraw_thumbnail_t *thumb)
*
* \brief This function is called by DNG_SetupObject.
* It aims at declaring the descriptor of the thumbnail image contained in the DNG files.
*
* \param thumb The thumb image descriptor of LIBRAW
*
* \return The thumb image descriptor for the player.
*/
static GF_ESD* DNG_GetThumbESD(libraw_thumbnail_t *thumb)
{
	GF_ESD *esd = gf_odf_desc_esd_new(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = GF_STREAM_THUMBNAIL;
	esd->ESID = 2;

	switch (thumb->tformat)
	{
	case LIBRAW_THUMBNAIL_JPEG:
		esd->decoderConfig->objectTypeIndication = GPAC_OTI_IMAGE_JPEG;
		break;

	case LIBRAW_THUMBNAIL_BITMAP:
		esd->decoderConfig->objectTypeIndication = GPAC_BMP_OTI;
		break;

	default:
		break;
	}

	return esd;
}


/**
* \fn static GF_Err IMG_CloseService(GF_InputService *plug)
*
* \brief This function is called when closing the muxer.
* It closes the files opend in libraw.
*
* \param plug The service of the player
*
* \return GF_OK is everything happend, otherwise a GF_Err.
*/
static GF_Err IMG_CloseService(GF_InputService *plug)
{
	DNGLoader *read;
	if (!plug)
		return GF_BAD_PARAM;
	read = (DNGLoader *)plug->priv;
	if (!read)
		return GF_BAD_PARAM;
	if (read->iprc) libraw_close(read->iprc);
	read->iprc = NULL;
	if (read->service)
		gf_service_disconnect_ack(read->service, NULL, GF_OK);
	return GF_OK;
}



/**
* \fn static GF_Err DNG_ConnectChannel(GF_InputService *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
*
* \brief This function is called by the player when all object in dng has been declared.
* It aims at affecting a communication channel to a declared object.
*
* \param plug The service of the player
*
* \param channel The communication channel
*
* \param url The identifier of the object
*
* \param upstream Not used in our case
*
* \return GF_OK is everything happend, otherwise a GF_Err.
*/
static GF_Err DNG_ConnectChannel(GF_InputService *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID=0;
	GF_Err e;
	DNGLoader *read;
	if (!plug)
		return GF_OK;
	read = (DNGLoader *)plug->priv;

	e = GF_SERVICE_ERROR;
	if (!url)
		goto exit;
	e = GF_STREAM_NOT_FOUND;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%ud", &ES_ID);
	}
	/*URL setup*/
	else if (!read->ch_raw && DNG_CanHandleURL(plug, url)) ES_ID = 1;

	if (ES_ID==1) {
		if (read->ch_raw == channel) goto exit;
		read->ch_raw = channel;
		e = GF_OK;
	}
	else if (ES_ID == 2) {
		if (read->ch_thumb == channel) goto exit;
		read->ch_thumb = channel;
		e = GF_OK;
	}

exit:
	gf_service_connect_ack(read->service, channel, e);
	return e;
}

/**
* \fn static GF_Err DNG_DisconnectChannel(GF_InputService *plug, LPNETCHANNEL channel)
*
* \brief This function is called by the player when all object in dng has to be destroy.
*
* \param plug The service of the player
*
* \param channel The communication channel to destroy
*
* \return GF_OK is everything happend, otherwise a GF_Err.
*/
static GF_Err DNG_DisconnectChannel(GF_InputService *plug, LPNETCHANNEL channel)
{
	GF_Err e = GF_STREAM_NOT_FOUND;
	DNGLoader *read = (DNGLoader *)plug->priv;

	if (read->ch_raw == channel) {
		if (read->raw_data) {
			libraw_dcraw_clear_mem(read->raw_data);
			read->raw_data = NULL;
		}
		read->ch_raw = NULL;
		e = GF_OK;
	}

	if (read->ch_thumb == channel) {
		read->ch_thumb = NULL;
		e = GF_OK;
	}

	gf_service_disconnect_ack(read->service, channel, e);
	return GF_OK;
}



/**
* \fn static GF_Err IMG_ChannelGetSLP(GF_InputService *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, GF_SLHeader *out_sl_hdr, Bool *sl_compressed, GF_Err *out_reception_status, Bool *is_new_data)
*
* \brief This function is called by the player to retrieve the raw data from an object in the dng.
*
* \param plug The service of the player
*
* \param channel The communication channel to get data from
*
* \param out_data_ptr The pointer to the raw data
*
* \param out_data_size The size of the raw data
*
* \param out_sl_hdr The description of the raw data
*
* \param sl_compressed Not used in our case, set to false
*
* \param out_reception_status The reception status (GF_EOS if end of stream, otherwise GF_OK)
*
* \param is_new_data Whether it's a new data or not
*
* \return GF_OK is everything happend, otherwise a GF_Err.
*/
static GF_Err IMG_ChannelGetSLP(GF_InputService *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, GF_SLHeader *out_sl_hdr, Bool *sl_compressed, GF_Err *out_reception_status, Bool *is_new_data)
{
	DNGLoader *read = (DNGLoader *)plug->priv;

	*out_reception_status = GF_OK;
	*sl_compressed = GF_FALSE;
	*is_new_data = GF_FALSE;

	memset(&read->sl_hdr, 0, sizeof(GF_SLHeader));
	read->sl_hdr.randomAccessPointFlag = 1;
	read->sl_hdr.compositionTimeStampFlag = 1;
	*out_sl_hdr = read->sl_hdr;


	/*fetching raw data*/
	if (read->ch_raw == channel) {
		libraw_data_t * iprc = read->iprc;

		if (read->raw_done) {
			*out_reception_status = GF_EOS;
			return GF_OK;
		}
		if (!read->raw_data) {
			if (!read->iprc) {
				*out_data_ptr = NULL;
				*out_data_size = 0;
				return GF_OK;
			}
			*is_new_data = GF_TRUE;

		}
		*out_data_ptr = read->raw_data->data;
		*out_data_size = read->raw_data->data_size;
		return GF_OK;
	}

	/*fetching thumb data*/
	if (read->ch_thumb == channel) {
		libraw_data_t * iprc = read->iprc;
		libraw_thumbnail_t* thumb = &iprc->thumbnail;

		if (read->thumb_done) {
			*out_reception_status = GF_EOS;
			return GF_OK;
		}
		if (!read->thumb_data) {
			if (!read->iprc) {
				*out_data_ptr = NULL;
				*out_data_size = 0;
				return GF_OK;
			}
			*is_new_data = GF_TRUE;
			read->thumb_data_size = thumb->tlength;
			read->thumb_data = (char*) gf_malloc(sizeof(char) * read->thumb_data_size);
			memcpy(read->thumb_data, thumb->thumb, read->thumb_data_size);

		}
		*out_data_ptr = read->thumb_data;
		*out_data_size = read->thumb_data_size;
		return GF_OK;
	}
	return GF_STREAM_NOT_FOUND;
}

/**
* \fn static GF_Err IMG_ChannelReleaseSLP(GF_InputService *plug, LPNETCHANNEL channel)
*
* \brief This function is called by the player when to delete retrieved raw data from an object in the dng.
*
* \param plug The service of the player
*
* \param channel The communication channel to get data from
*
* \return GF_OK is everything happend, otherwise a GF_Err.
*/
static GF_Err IMG_ChannelReleaseSLP(GF_InputService *plug, LPNETCHANNEL channel)
{
	DNGLoader *read = (DNGLoader *)plug->priv;

	if (read->ch_raw == channel) {
		if (!read->raw_data) return GF_BAD_PARAM;
		read->raw_done = GF_TRUE;
		return GF_OK;
	}
	
	if (read->ch_thumb == channel) {
		if (!read->thumb_data) return GF_BAD_PARAM;
		gf_free(read->thumb_data);
		read->thumb_data = NULL;
		read->thumb_done = GF_TRUE;
		return GF_OK;

	}
	return GF_OK;
}

/**** This section is out of acccessor scope ***/
void *NewLoaderInterface()
{
	DNGLoader *priv;
	GF_InputService *plug;
	GF_SAFEALLOC(plug, GF_InputService);
	GF_REGISTER_MODULE_INTERFACE(plug, GF_NET_CLIENT_INTERFACE, "GPAC DNG Image Reader", "Bevara technologies")

	plug->RegisterMimeTypes = DNG_RegisterMimeTypes;
	plug->CanHandleURL = DNG_CanHandleURL;
	plug->CanHandleURLInService = NULL;
	plug->ConnectService = DNG_ConnectService;
	plug->CloseService = IMG_CloseService;
	plug->GetServiceDescriptor = IMG_GetServiceDesc;
	plug->ConnectChannel = DNG_ConnectChannel;
	plug->DisconnectChannel = DNG_DisconnectChannel;
	plug->ChannelGetSLP = IMG_ChannelGetSLP;
	plug->ChannelReleaseSLP = IMG_ChannelReleaseSLP;
	plug->ServiceCommand = IMG_ServiceCommand;

	GF_SAFEALLOC(priv, DNGLoader);
	priv->iprc = libraw_init(0);
	plug->priv = priv;
	return plug;
}

void DeleteLoaderInterface(void *ifce)
{
	DNGLoader *read;
	GF_InputService *plug = (GF_InputService *) ifce;
	GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("DeleteLoaderInterface : 1\n"));
	if (!plug)
		return;
	read = (DNGLoader *)plug->priv;
	if (read) {
		libraw_close(read->iprc);
		gf_free(read);
	}
		
	plug->priv = NULL;
	gf_free(plug);
	GF_LOG(GF_LOG_MEDIA, GF_LOG_ERROR, ("DeleteLoaderInterface : 2\n"));
}


GPAC_MODULE_EXPORT
const u32 *QueryInterfaces()
{
	static u32 si[] = {
		GF_NET_CLIENT_INTERFACE,
		GF_MEDIA_DECODER_INTERFACE,
		0
	};
	return si;
}

GPAC_MODULE_EXPORT
GF_BaseInterface *LoadInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case GF_MEDIA_DECODER_INTERFACE:
		return (GF_BaseInterface *)NewRAWDec();
		break;
	case GF_NET_CLIENT_INTERFACE:
		return (GF_BaseInterface *)NewLoaderInterface();
	default:
		return NULL;
	}
}

GPAC_MODULE_EXPORT
void ShutdownInterface(GF_BaseInterface *ifce)
{
	switch (ifce->InterfaceType) {
	case GF_MEDIA_DECODER_INTERFACE:
		DeleteRAWDec((GF_BaseDecoder *)ifce);
		break;
	case GF_NET_CLIENT_INTERFACE:
		DeleteLoaderInterface(ifce);
		break;
	}
}

GPAC_MODULE_STATIC_DECLARATION(dng_in)
