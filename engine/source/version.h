// Copyright (C) 2004-2011 Christoph Kubisch & Eike Decker
// This file is part of the "Luxinia Engine".
// For conditions of distribution and use, see luxinia.h


// autogenerated version info
//
#define VERXSTR(s) VERSTR(s)
#define VERSTR(s) #s


#define LUX_VERSION_MAJOR	1
#define LUX_VERSION_MINOR	500
#define LUX_VERSION_REV		0

#define LUX_VERSION_FULL	LUX_VERSION_MAJOR, LUX_VERSION_MINOR, 0, LUX_VERSION_REV
#define LUX_VERSION_FULLSTR	VERXSTR(LUX_VERSION_FULL)

#if defined(_DEBUG)
	#define LUX_VERSION_TYPESTR	"Luxinia dbg"
#elif defined(LUX_DEVBUILD)
	#define LUX_VERSION_TYPESTR	"Luxinia dev"
#else
	#define LUX_VERSION_TYPESTR	"Luxinia"
#endif

#define LUX_VERSION			(LUX_VERSION_TYPESTR " " VERXSTR(LUX_VERSION_MAJOR) "." VERXSTR(LUX_VERSION_MINOR))

