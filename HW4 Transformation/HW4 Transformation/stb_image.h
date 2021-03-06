/* stb_image - v2.22 - public domain image loader - http://nothings.org/stb
								  no warranty implied; use at your own risk

   Do this:
	  #define STB_IMAGE_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"

   You can #define STBI_ASSERT(x) before the #include to avoid using assert.h.
   And #define STBI_MALLOC, STBI_REALLOC, and STBI_FREE to avoid using malloc,realloc,free


   QUICK NOTES:
	  Primarily of interest to game developers and other people who can
		  avoid problematic images and only need the trivial interface

	  JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
	  PNG 1/2/4/8/16-bit-per-channel

	  TGA (not sure what subset, if a subset)
	  BMP non-1bpp, non-RLE
	  PSD (composited view only, no extra channels, 8/16 bit-per-channel)

	  GIF (*comp always reports as 4-channel)
	  HDR (radiance rgbE format)
	  PIC (Softimage PIC)
	  PNM (PPM and PGM binary only)

	  Animated GIF still needs a proper API, but here's one way to do it:
		  http://gist.github.com/urraka/685d9a6340b26b830d49

	  - decode from memory or through FILE (define STBI_NO_STDIO to remove code)
	  - decode from arbitrary I/O callbacks
	  - SIMD acceleration on x86/x64 (SSE2) and ARM (NEON)

   Full documentation under "DOCUMENTATION" below.


LICENSE

  See end of file for license information.

RECENT REVISION HISTORY:

	  2.22  (2019-03-04) gif fixes, fix warnings
	  2.21  (2019-02-25) fix typo in comment
	  2.20  (2019-02-07) support utf8 filenames in Windows; fix warnings and platform ifdefs
	  2.19  (2018-02-11) fix warning
	  2.18  (2018-01-30) fix warnings
	  2.17  (2018-01-29) bugfix, 1-bit BMP, 16-bitness query, fix warnings
	  2.16  (2017-07-23) all functions have 16-bit variants; optimizations; bugfixes
	  2.15  (2017-03-18) fix png-1,2,4; all Imagenet JPGs; no runtime SSE detection on GCC
	  2.14  (2017-03-03) remove deprecated STBI_JPEG_OLD; fixes for Imagenet JPGs
	  2.13  (2016-12-04) experimental 16-bit API, only for PNG so far; fixes
	  2.12  (2016-04-02) fix typo in 2.11 PSD fix that caused crashes
	  2.11  (2016-04-02) 16-bit PNGS; enable SSE2 in non-gcc x64
						 RGB-format JPEG; remove white matting in PSD;
						 allocate large structures on the stack;
						 correct channel count for PNG & BMP
	  2.10  (2016-01-22) avoid warning introduced in 2.09
	  2.09  (2016-01-16) 16-bit TGA; comments in PNM files; STBI_REALLOC_SIZED

   See end of file for full revision history.


 ============================    Contributors    =========================

 Image formats                          Extensions, features
	Sean Barrett (jpeg, png, bmp)          Jetro Lauha (stbi_info)
	Nicolas Schulz (hdr, psd)              Martin "SpartanJ" Golini (stbi_info)
	Jonathan Dummer (tga)                  James "moose2000" Brown (iPhone PNG)
	Jean-Marc Lienher (gif)                Ben "Disch" Wenger (io callbacks)
	Tom Seddon (pic)                       Omar Cornut (1/2/4-bit PNG)
	Thatcher Ulrich (psd)                  Nicolas Guillemot (vertical flip)
	Ken Miller (pgm, ppm)                  Richard Mitton (16-bit PSD)
	github:urraka (animated gif)           Junggon Kim (PNM comments)
	Christopher Forseth (animated gif)     Daniel Gibson (16-bit TGA)
										   socks-the-fox (16-bit PNG)
										   Jeremy Sawicki (handle all ImageNet JPGs)
 Optimizations & bugfixes                  Mikhail Morozov (1-bit BMP)
	Fabian "ryg" Giesen                    Anael Seghezzi (is-16-bit query)
	Arseny Kapoulkine
	John-Mark Allen
	Carmelo J Fdez-Aguera

 Bug & warning fixes
	Marc LeBlanc            David Woo          Guillaume George   Martins Mozeiko
	Christpher Lloyd        Jerry Jansson      Joseph Thomson     Phil Jordan
	Dave Moore              Roy Eltham         Hayaki Saito       Nathan Reed
	Won Chun                Luke Graham        Johan Duparc       Nick Verigakis
	the Horde3D community   Thomas Ruf         Ronny Chevalier    github:rlyeh
	Janez Zemva             John Bartholomew   Michal Cichon      github:romigrou
	Jonathan Blow           Ken Hamada         Tero Hanninen      github:svdijk
	Laurent Gomila          Cort Stratton      Sergio Gonzalez    github:snagar
	Aruelien Pocheville     Thibault Reuille   Cass Everitt       github:Zelex
	Ryamond Barbiero        Paul Du Bois       Engin Manap        github:grim210
	Aldo Culquicondor       Philipp Wiesemann  Dale Weiler        github:sammyhw
	Oriol Ferrer Mesia      Josh Tobin         Matthew Gregan     github:phprus
	Julian Raschke          Gregory Mullen     Baldur Karlsson    github:poppolopoppo
	Christian Floisand      Kevin Schmidt      JR Smith           github:darealshinji
	Blazej Dariusz Roszkowski                                     github:Michaelangel007
*/

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

// DOCUMENTATION
//
// Limitations:
//    - no 12-bit-per-channel JPEG
//    - no JPEGs with arithmetic coding
//    - GIF always returns *comp=4
//
// Basic usage (see HDR discussion below for HDR usage):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x                 -- outputs image width in pixels
//    int *y                 -- outputs image height in pixels
//    int *channels_in_file  -- outputs # of image components in image file
//    int desired_channels   -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data, or NULL on an allocation failure or if the image is
// corrupt or invalid. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'desired_channels' if desired_channels is non-zero, or
// *channels_in_file otherwise. If desired_channels is non-zero,
// *channels_in_file has the number of components that _would_ have been
// output otherwise. E.g. if you set desired_channels to 4, you will always
// get RGBA output, but you can check *channels_in_file to see if it's trivially
// opaque because e.g. there were only 3 channels in the source image.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *channels_in_file will be unchanged. The function
// stbi_failure_reason() can be queried for an extremely brief, end-user
// unfriendly explanation of why the load failed. Define STBI_NO_FAILURE_STRINGS
// to avoid compiling these strings at all, and STBI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// Paletted PNG, BMP, GIF, and PIC images are automatically depalettized.
//
// ===========================================================================
//
// UNICODE:
//
//   If compiling for Windows and you wish to use Unicode filenames, compile
//   with
//       #define STBI_WINDOWS_UTF8
//   and pass utf8-encoded filenames. Call stbi_convert_wchar_to_utf8 to convert
//   Windows wchar_t filenames to utf8.
//
// ===========================================================================
//
// Philosophy
//
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Sometimes I let "good performance" creep up in priority over "easy to maintain",
// and for best performance I may provide less-easy-to-use APIs that give higher
// performance, in addition to the easy-to-use ones. Nevertheless, it's important
// to keep in mind that from the standpoint of you, a client of this library,
// all you care about is #1 and #3, and stb libraries DO NOT emphasize #3 above all.
//
// Some secondary priorities arise directly from the first two, some of which
// provide more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small source code footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//
// ===========================================================================
//
// I/O callbacks
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead.
//
// The three functions you must define are "read" (reads some bytes of data),
// "skip" (skips some bytes of data), "eof" (reports if the stream is at the end).
//
// ===========================================================================
//
// SIMD support
//
// The JPEG decoder will try to automatically use SIMD kernels on x86 when
// supported by the compiler. For ARM Neon support, you must explicitly
// request it.
//
// (The old do-it-yourself SIMD API is no longer supported in the current
// code.)
//
// On x86, SSE2 will automatically be used when available based on a run-time
// test; if not, the generic C versions are used as a fall-back. On ARM targets,
// the typical path is to have separate builds for NEON and non-NEON devices
// (at least this is true for iOS and Android). Therefore, the NEON support is
// toggled by a build flag: define STBI_NEON to get NEON loops.
//
// If for some reason you do not want to use any of SIMD code, or if
// you have issues compiling it, you can disable it entirely by
// defining STBI_NO_SIMD.
//
// ===========================================================================
//
// HDR image support   (disable by defining STBI_NO_HDR)
//
// stb_image supports loading HDR images in general, and currently the Radiance
// .HDR file format specifically. You can still load any file through the existing
// interface; if you attempt to load an HDR file, it will be automatically remapped
// to LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
// both of these constants can be reconfigured through this interface:
//
//     stbi_hdr_to_ldr_gamma(2.2f);
//     stbi_hdr_to_ldr_scale(1.0f);
//
// (note, do not use _inverse_ constants; stbi_image will invert them
// appropriately).
//
// Additionally, there is a new, parallel interface for loading files as
// (linear) floats to preserve the full dynamic range:
//
//    float *data = stbi_loadf(filename, &x, &y, &n, 0);
//
// If you load LDR images through this interface, those images will
// be promoted to floating point values, run through the inverse of
// constants corresponding to the above:
//
//     stbi_ldr_to_hdr_scale(1.0f);
//     stbi_ldr_to_hdr_gamma(2.2f);
//
// Finally, given a filename (or an open file or memory block--see header
// file for details) containing image data, you can query for the "most
// appropriate" interface to use (that is, whether the image is HDR or
// not), using:
//
//     stbi_is_hdr(char *filename);
//
// ===========================================================================
//
// iPhone PNG support:
//
// By default we convert iphone-formatted PNGs back to RGB, even though
// they are internally encoded differently. You can disable this conversion
// by calling stbi_convert_iphone_png_to_rgb(0), in which case
// you will always just get the native iphone "format" through (which
// is BGR stored in RGB).
//
// Call stbi_set_unpremultiply_on_load(1) as well to force a divide per
// pixel to remove any premultiplied alpha *only* if the image file explicitly
// says there's premultiplied data (currently only happens in iPhone images,
// and only if iPhone convert-to-rgb processing is on).
//
// ===========================================================================
//
// ADDITIONAL CONFIGURATION
//
//  - You can suppress implementation of any of the decoders to reduce
//    your code footprint by #defining one or more of the following
//    symbols before creating the implementation.
//
//        STBI_NO_JPEG
//        STBI_NO_PNG
//        STBI_NO_BMP
//        STBI_NO_PSD
//        STBI_NO_TGA
//        STBI_NO_GIF
//        STBI_NO_HDR
//        STBI_NO_PIC
//        STBI_NO_PNM   (.ppm and .pgm)
//
//  - You can request *only* certain decoders and suppress all other ones
//    (this will be more forward-compatible, as addition of new decoders
//    doesn't require you to disable them explicitly):
//
//        STBI_ONLY_JPEG
//        STBI_ONLY_PNG
//        STBI_ONLY_BMP
//        STBI_ONLY_PSD
//        STBI_ONLY_TGA
//        STBI_ONLY_GIF
//        STBI_ONLY_HDR
//        STBI_ONLY_PIC
//        STBI_ONLY_PNM   (.ppm and .pgm)
//
//   - If you use STBI_NO_PNG (or _ONLY_ without PNG), and you still
//     want the zlib decoder to be available, #define STBI_SUPPORT_ZLIB
//


#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif // STBI_NO_STDIO

#define STBI_VERSION 1

enum
{
	STBI_default = 0, // only used for desired_channels

	STBI_grey = 1,
	STBI_grey_alpha = 2,
	STBI_rgb = 3,
	STBI_rgb_alpha = 4
};

#include <stdlib.h>
typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STBIDEF
#ifdef STB_IMAGE_STATIC
#define STBIDEF static
#else
#define STBIDEF extern
#endif
#endif

	//////////////////////////////////////////////////////////////////////////////
	//
	// PRIMARY API - works on images of any type
	//

	//
	// load image by filename, open file, or memory buffer
	//

	typedef struct
	{
		int(*read)  (void *user, char *data, int size);   // fill 'data' with 'size' bytes.  return number of bytes actually read
		void(*skip)  (void *user, int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
		int(*eof)   (void *user);                       // returns nonzero if we are at end of file/data
	} stbi_io_callbacks;

	////////////////////////////////////
	//
	// 8-bits-per-channel interface
	//

	STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc           const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
	STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#ifndef STBI_NO_STDIO
	STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
	STBIDEF stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
	// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

#ifndef STBI_NO_GIF
	STBIDEF stbi_uc *stbi_load_gif_from_memory(stbi_uc const *buffer, int len, int **delays, int *x, int *y, int *z, int *comp, int req_comp);
#endif

#ifdef STBI_WINDOWS_UTF8
	STBIDEF int stbi_convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t* input);
#endif

	////////////////////////////////////
	//
	// 16-bits-per-channel interface
	//

	STBIDEF stbi_us *stbi_load_16_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
	STBIDEF stbi_us *stbi_load_16_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#ifndef STBI_NO_STDIO
	STBIDEF stbi_us *stbi_load_16(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
	STBIDEF stbi_us *stbi_load_from_file_16(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
#endif

	////////////////////////////////////
	//
	// float-per-channel interface
	//
#ifndef STBI_NO_LINEAR
	STBIDEF float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
	STBIDEF float *stbi_loadf_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#ifndef STBI_NO_STDIO
	STBIDEF float *stbi_loadf(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
	STBIDEF float *stbi_loadf_from_file(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
#endif
#endif

#ifndef STBI_NO_HDR
	STBIDEF void   stbi_hdr_to_ldr_gamma(float gamma);
	STBIDEF void   stbi_hdr_to_ldr_scale(float scale);
#endif // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
	STBIDEF void   stbi_ldr_to_hdr_gamma(float gamma);
	STBIDEF void   stbi_ldr_to_hdr_scale(float scale);
#endif // STBI_NO_LINEAR

	// stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
	STBIDEF int    stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
	STBIDEF int    stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
#ifndef STBI_NO_STDIO
	STBIDEF int      stbi_is_hdr(char const *filename);
	STBIDEF int      stbi_is_hdr_from_file(FILE *f);
#endif // STBI_NO_STDIO


	// get a VERY brief reason for failure
	// NOT THREADSAFE
	STBIDEF const char *stbi_failure_reason(void);

	// free the loaded image -- this is just free()
	STBIDEF void     stbi_image_free(void *retval_from_stbi_load);

	// get image dimensions & components without fully decoding
	STBIDEF int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
	STBIDEF int      stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);
	STBIDEF int      stbi_is_16_bit_from_memory(stbi_uc const *buffer, int len);
	STBIDEF int      stbi_is_16_bit_from_callbacks(stbi_io_callbacks const *clbk, void *user);

#ifndef STBI_NO_STDIO
	STBIDEF int      stbi_info(char const *filename, int *x, int *y, int *comp);
	STBIDEF int      stbi_info_from_file(FILE *f, int *x, int *y, int *comp);
	STBIDEF int      stbi_is_16_bit(char const *filename);
	STBIDEF int      stbi_is_16_bit_from_file(FILE *f);
#endif



	// for image formats that explicitly notate that they have premultiplied alpha,
	// we just return the colors as stored in the file. set this flag to force
	// unpremultiplication. results are undefined if the unpremultiply overflow.
	STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

	// indicate whether we should process iphone images back to canonical format,
	// or just pass them through "as-is"
	STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);

	// flip the image vertically, so the first pixel in the output array is the bottom left
	STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

	// ZLIB client - used by PNG, available for other purposes

	STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
	STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
	STBIDEF char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
	STBIDEF int   stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

	STBIDEF char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
	STBIDEF int   stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);


#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBI_INCLUDE_STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#if defined(STBI_ONLY_JPEG) || defined(STBI_ONLY_PNG) || defined(STBI_ONLY_BMP) \
  || defined(STBI_ONLY_TGA) || defined(STBI_ONLY_GIF) || defined(STBI_ONLY_PSD) \
  || defined(STBI_ONLY_HDR) || defined(STBI_ONLY_PIC) || defined(STBI_ONLY_PNM) \
  || defined(STBI_ONLY_ZLIB)
#ifndef STBI_ONLY_JPEG
#define STBI_NO_JPEG
#endif
#ifndef STBI_ONLY_PNG
#define STBI_NO_PNG
#endif
#ifndef STBI_ONLY_BMP
#define STBI_NO_BMP
#endif
#ifndef STBI_ONLY_PSD
#define STBI_NO_PSD
#endif
#ifndef STBI_ONLY_TGA
#define STBI_NO_TGA
#endif
#ifndef STBI_ONLY_GIF
#define STBI_NO_GIF
#endif
#ifndef STBI_ONLY_HDR
#define STBI_NO_HDR
#endif
#ifndef STBI_ONLY_PIC
#define STBI_NO_PIC
#endif
#ifndef STBI_ONLY_PNM
#define STBI_NO_PNM
#endif
#endif

#if defined(STBI_NO_PNG) && !defined(STBI_SUPPORT_ZLIB) && !defined(STBI_NO_ZLIB)
#define STBI_NO_ZLIB
#endif


#include <stdarg.h>
#include <stddef.h> // ptrdiff_t on osx
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR)
#include <math.h>  // ldexp, pow
#endif

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT(x) assert(x)
#endif

#ifdef __cplusplus
#define STBI_EXTERN extern "C"
#else
#define STBI_EXTERN extern
#endif


#ifndef _MSC_VER
#ifdef __cplusplus
#define stbi_inline inline
#else
#define stbi_inline
#endif
#else
#define stbi_inline __forceinline
#endif


#ifdef _MSC_VER
typedef unsigned short stbi__uint16;
typedef   signed short stbi__int16;
typedef unsigned int   stbi__uint32;
typedef   signed int   stbi__int32;
#else
#include <stdint.h>
typedef uint16_t stbi__uint16;
typedef int16_t  stbi__int16;
typedef uint32_t stbi__uint32;
typedef int32_t  stbi__int32;
#endif

// should produce compiler error if size is wrong
typedef unsigned char validate_uint32[sizeof(stbi__uint32) == 4 ? 1 : -1];

#ifdef _MSC_VER
#define STBI_NOTUSED(v)  (void)(v)
#else
#define STBI_NOTUSED(v)  (void)sizeof(v)
#endif

#ifdef _MSC_VER
#define STBI_HAS_LROTL
#endif

#ifdef STBI_HAS_LROTL
#define stbi_lrot(x,y)  _lrotl(x,y)
#else
#define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif

#if defined(STBI_MALLOC) && defined(STBI_FREE) && (defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED))
// ok
#elif !defined(STBI_MALLOC) && !defined(STBI_FREE) && !defined(STBI_REALLOC) && !defined(STBI_REALLOC_SIZED)
// ok
#else
#error "Must define all or none of STBI_MALLOC, STBI_FREE, and STBI_REALLOC (or STBI_REALLOC_SIZED)."
#endif

#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
#define STBI_FREE(p)              free(p)
#endif

#ifndef STBI_REALLOC_SIZED
#define STBI_REALLOC_SIZED(p,oldsz,newsz) STBI_REALLOC(p,newsz)
#endif

// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
#define STBI__X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
#define STBI__X86_TARGET
#endif

#if defined(__GNUC__) && defined(STBI__X86_TARGET) && !defined(__SSE2__) && !defined(STBI_NO_SIMD)
// gcc doesn't support sse2 intrinsics unless you compile with -msse2,
// which in turn means it gets to use SSE2 everywhere. This is unfortunate,
// but previous attempts to provide the SSE2 functions with runtime
// detection caused numerous issues. The way architecture extensions are
// exposed in GCC/Clang is, sadly, not really suited for one-file libs.
// New behavior: if compiled with -msse2, we use SSE2 without any
// detection; if not, we don't use it at all.
#define STBI_NO_SIMD
#endif

#if defined(__MINGW32__) && defined(STBI__X86_TARGET) && !defined(STBI_MINGW_ENABLE_SSE2) && !defined(STBI_NO_SIMD)
// Note that __MINGW32__ doesn't actually mean 32-bit, so we have to avoid STBI__X64_TARGET
//
// 32-bit MinGW wants ESP to be 16-byte aligned, but this is not in the
// Windows ABI and VC++ as well as Windows DLLs don't maintain that invariant.
// As a result, enabling SSE2 on 32-bit MinGW is dangerous when not
// simultaneously enabling "-mstackrealign".
//
// See https://github.com/nothings/stb/issues/81 for more information.
//
// So default to no SSE2 on 32-bit MinGW. If you've read this far and added
// -mstackrealign to your build settings, feel free to #define STBI_MINGW_ENABLE_SSE2.
#define STBI_NO_SIMD
#endif

#if !defined(STBI_NO_SIMD) && (defined(STBI__X86_TARGET) || defined(STBI__X64_TARGET))
#define STBI_SSE2
#include <emmintrin.h>

#ifdef _MSC_VER

#if _MSC_VER >= 1400  // not VC6
#include <intrin.h> // __cpuid
static int stbi__cpuid3(void)
{
	int info[4];
	__cpuid(info, 1);
	return info[3];
}
#else
static int stbi__cpuid3(void)
{
	int res;
	__asm {
		mov  eax, 1
		cpuid
		mov  res, edx
	}
	return res;
}
#endif

#define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name

#if !defined(STBI_NO_JPEG) && defined(STBI_SSE2)
static int stbi__sse2_available(void)
{
	int info3 = stbi__cpuid3();
	return ((info3 >> 26) & 1) != 0;
}
#endif

#else // assume GCC-style if not VC++
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))

#if !defined(STBI_NO_JPEG) && defined(STBI_SSE2)
static int stbi__sse2_available(void)
{
	// If we're even attempting to compile this on GCC/Clang, that means
	// -msse2 is on, which means the compiler is allowed to use SSE2
	// instructions at will, and so are we.
	return 1;
}
#endif

#endif
#endif

// ARM NEON
#if defined(STBI_NO_SIMD) && defined(STBI_NEON)
#undef STBI_NEON
#endif

#ifdef STBI_NEON
#include <arm_neon.h>
// assume GCC or Clang on ARM targets
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
#endif

#ifndef STBI_SIMD_ALIGN
#define STBI_SIMD_ALIGN(type, name) type name
#endif

///////////////////////////////////////////////
//
//  stbi__context struct and start_xxx functions

// stbi__context structure is our basic context used by all images, so it
// contains all the IO context, plus some basic image information
typedef struct
{
	stbi__uint32 img_x, img_y;
	int img_n, img_out_n;

	stbi_io_callbacks io;
	void *io_user_data;

	int read_from_callbacks;
	int buflen;
	stbi_uc buffer_start[128];

	stbi_uc *img_buffer, *img_buffer_end;
	stbi_uc *img_buffer_original, *img_buffer_original_end;
} stbi__context;


static void stbi__refill_buffer(stbi__context *s);

// initialize a memory-decode context
static void stbi__start_mem(stbi__context *s, stbi_uc const *buffer, int len)
{
	s->io.read = NULL;
	s->read_from_callbacks = 0;
	s->img_buffer = s->img_buffer_original = (stbi_uc *)buffer;
	s->img_buffer_end = s->img_buffer_original_end = (stbi_uc *)buffer + len;
}

// initialize a callback-based context
static void stbi__start_callbacks(stbi__context *s, stbi_io_callbacks *c, void *user)
{
	s->io = *c;
	s->io_user_data = user;
	s->buflen = sizeof(s->buffer_start);
	s->read_from_callbacks = 1;
	s->img_buffer_original = s->buffer_start;
	stbi__refill_buffer(s);
	s->img_buffer_original_end = s->img_buffer_end;
}

#ifndef STBI_NO_STDIO

static int stbi__stdio_read(void *user, char *data, int size)
{
	return (int)fread(data, 1, size, (FILE*)user);
}

static void stbi__stdio_skip(void *user, int n)
{
	fseek((FILE*)user, n, SEEK_CUR);
}

static int stbi__stdio_eof(void *user)
{
	return feof((FILE*)user);
}

static stbi_io_callbacks stbi__stdio_callbacks =
{
   stbi__stdio_read,
   stbi__stdio_skip,
   stbi__stdio_eof,
};

static void stbi__start_file(stbi__context *s, FILE *f)
{
	stbi__start_callbacks(s, &stbi__stdio_callbacks, (void *)f);
}

//static void stop_file(stbi__context *s) { }

#endif // !STBI_NO_STDIO

static void stbi__rewind(stbi__context *s)
{
	// conceptually rewind SHOULD rewind to the beginning of the stream,
	// but we just rewind to the beginning of the initial buffer, because
	// we only use it after doing 'test', which only ever looks at at most 92 bytes
	s->img_buffer = s->img_buffer_original;
	s->img_buffer_end = s->img_buffer_original_end;
}

enum
{
	STBI_ORDER_RGB,
	STBI_ORDER_BGR
};

typedef struct
{
	int bits_per_channel;
	int num_channels;
	int channel_order;
} stbi__result_info;

#ifndef STBI_NO_JPEG
static int      stbi__jpeg_test(stbi__context *s);
static void    *stbi__jpeg_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__jpeg_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PNG
static int      stbi__png_test(stbi__context *s);
static void    *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__png_info(stbi__context *s, int *x, int *y, int *comp);
static int      stbi__png_is16(stbi__context *s);
#endif

#ifndef STBI_NO_BMP
static int      stbi__bmp_test(stbi__context *s);
static void    *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__bmp_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_TGA
static int      stbi__tga_test(stbi__context *s);
static void    *stbi__tga_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__tga_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PSD
static int      stbi__psd_test(stbi__context *s);
static void    *stbi__psd_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc);
static int      stbi__psd_info(stbi__context *s, int *x, int *y, int *comp);
static int      stbi__psd_is16(stbi__context *s);
#endif

#ifndef STBI_NO_HDR
static int      stbi__hdr_test(stbi__context *s);
static float   *stbi__hdr_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__hdr_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PIC
static int      stbi__pic_test(stbi__context *s);
static void    *stbi__pic_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__pic_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_GIF
static int      stbi__gif_test(stbi__context *s);
static void    *stbi__gif_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static void    *stbi__load_gif_main(stbi__context *s, int **delays, int *x, int *y, int *z, int *comp, int req_comp);
static int      stbi__gif_info(stbi__context *s, int *x, int *y, int *comp);
#endif

#ifndef STBI_NO_PNM
static int      stbi__pnm_test(stbi__context *s);
static void    *stbi__pnm_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int      stbi__pnm_info(stbi__context *s, int *x, int *y, int *comp);
#endif

// this is not threadsafe
static const char *stbi__g_failure_reason;

STBIDEF const char *stbi_failure_reason(void)
{
	return stbi__g_failure_reason;
}

static int stbi__err(const char *str)
{
	stbi__g_failure_reason = str;
	return 0;
}

static void *stbi__malloc(size_t size)
{
	return STBI_MALLOC(size);
}

// stb_image uses ints pervasively, including for offset calculations.
// therefore the largest decoded image size we can support with the
// current code, even on 64-bit targets, is INT_MAX. this is not a
// significant limitation for the intended use case.
//
// we do, however, need to make sure our size calculations don't
// overflow. hence a few helper functions for size calculations that
// multiply integers together, making sure that they're non-negative
// and no overflow occurs.

// return 1 if the sum is valid, 0 on overflow.
// negative terms are considered invalid.
static int stbi__addsizes_valid(int a, int b)
{
	if (b < 0) return 0;
	// now 0 <= b <= INT_MAX, hence also
	// 0 <= INT_MAX - b <= INTMAX.
	// And "a + b <= INT_MAX" (which might overflow) is the
	// same as a <= INT_MAX - b (no overflow)
	return a <= INT_MAX - b;
}

// returns 1 if the product is valid, 0 on overflow.
// negative factors are considered invalid.
static int stbi__mul2sizes_valid(int a, int b)
{
	if (a < 0 || b < 0) return 0;
	if (b == 0) return 1; // mul-by-0 is always safe
	// portable way to check for no overflows in a*b
	return a <= INT_MAX / b;
}

// returns 1 if "a*b + add" has no negative terms/factors and doesn't overflow
static int stbi__mad2sizes_valid(int a, int b, int add)
{
	return stbi__mul2sizes_valid(a, b) && stbi__addsizes_valid(a*b, add);
}

// returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
static int stbi__mad3sizes_valid(int a, int b, int c, int add)
{
	return stbi__mul2sizes_valid(a, b) && stbi__mul2sizes_valid(a*b, c) &&
		stbi__addsizes_valid(a*b*c, add);
}

// returns 1 if "a*b*c*d + add" has no negative terms/factors and doesn't overflow
#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR)
static int stbi__mad4sizes_valid(int a, int b, int c, int d, int add)
{
	return stbi__mul2sizes_valid(a, b) && stbi__mul2sizes_valid(a*b, c) &&
		stbi__mul2sizes_valid(a*b*c, d) && stbi__addsizes_valid(a*b*c*d, add);
}
#endif

// mallocs with size overflow checking
static void *stbi__malloc_mad2(int a, int b, int add)
{
	if (!stbi__mad2sizes_valid(a, b, add)) return NULL;
	return stbi__malloc(a*b + add);
}

static void *stbi__malloc_mad3(int a, int b, int c, int add)
{
	if (!stbi__mad3sizes_valid(a, b, c, add)) return NULL;
	return stbi__malloc(a*b*c + add);
}

#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR)
static void *stbi__malloc_mad4(int a, int b, int c, int d, int add)
{
	if (!stbi__mad4sizes_valid(a, b, c, d, add)) return NULL;
	return stbi__malloc(a*b*c*d + add);
}
#endif

// stbi__err - error
// stbi__errpf - error returning pointer to float
// stbi__errpuc - error returning pointer to unsigned char

#ifdef STBI_NO_FAILURE_STRINGS
#define stbi__err(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
#define stbi__err(x,y)  stbi__err(y)
#else
#define stbi__err(x,y)  stbi__err(x)
#endif

#define stbi__errpf(x,y)   ((float *)(size_t) (stbi__err(x,y)?NULL:NULL))
#define stbi__errpuc(x,y)  ((unsigned char *)(size_t) (stbi__err(x,y)?NULL:NULL))

STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
	STBI_FREE(retval_from_stbi_load);
}

#ifndef STBI_NO_LINEAR
static float   *stbi__ldr_to_hdr(stbi_uc *data, int x, int y, int comp);
#endif

#ifndef STBI_NO_HDR
static stbi_uc *stbi__hdr_to_ldr(float   *data, int x, int y, int comp);
#endif

static int stbi__vertically_flip_on_load = 0;

STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip)
{
	stbi__vertically_flip_on_load = flag_true_if_should_flip;
}

static void *stbi__load_main(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc)
{
	memset(ri, 0, sizeof(*ri)); // make sure it's initialized if we add new fields
	ri->bits_per_channel = 8; // default is 8 so most paths don't have to be changed
	ri->channel_order = STBI_ORDER_RGB; // all current input & output are this, but this is here so we can add BGR order
	ri->num_channels = 0;

#ifndef STBI_NO_JPEG
	if (stbi__jpeg_test(s)) return stbi__jpeg_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_PNG
	if (stbi__png_test(s))  return stbi__png_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_BMP
	if (stbi__bmp_test(s))  return stbi__bmp_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_GIF
	if (stbi__gif_test(s))  return stbi__gif_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_PSD
	if (stbi__psd_test(s))  return stbi__psd_load(s, x, y, comp, req_comp, ri, bpc);
#endif
#ifndef STBI_NO_PIC
	if (stbi__pic_test(s))  return stbi__pic_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_PNM
	if (stbi__pnm_test(s))  return stbi__pnm_load(s, x, y, comp, req_comp, ri);
#endif

#ifndef STBI_NO_HDR
	if (stbi__hdr_test(s)) {
		float *hdr = stbi__hdr_load(s, x, y, comp, req_comp, ri);
		return stbi__hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
	}
#endif

#ifndef STBI_NO_TGA
	// test tga last because it's a crappy test!
	if (stbi__tga_test(s))
		return stbi__tga_load(s, x, y, comp, req_comp, ri);
#endif

	return stbi__errpuc("unknown image type", "Image not of any known type, or corrupt");
}

static stbi_uc *stbi__convert_16_to_8(stbi__uint16 *orig, int w, int h, int channels)
{
	int i;
	int img_len = w * h * channels;
	stbi_uc *reduced;

	reduced = (stbi_uc *)stbi__malloc(img_len);
	if (reduced == NULL) return stbi__errpuc("outofmem", "Out of memory");

	for (i = 0; i < img_len; ++i)
		reduced[i] = (stbi_uc)((orig[i] >> 8) & 0xFF); // top half of each byte is sufficient approx of 16->8 bit scaling

	STBI_FREE(orig);
	return reduced;
}

static stbi__uint16 *stbi__convert_8_to_16(stbi_uc *orig, int w, int h, int channels)
{
	int i;
	int img_len = w * h * channels;
	stbi__uint16 *enlarged;

	enlarged = (stbi__uint16 *)stbi__malloc(img_len * 2);
	if (enlarged == NULL) return (stbi__uint16 *)stbi__errpuc("outofmem", "Out of memory");

	for (i = 0; i < img_len; ++i)
		enlarged[i] = (stbi__uint16)((orig[i] << 8) + orig[i]); // replicate to high and low byte, maps 0->0, 255->0xffff

	STBI_FREE(orig);
	return enlarged;
}

static void stbi__vertical_flip(void *image, int w, int h, int bytes_per_pixel)
{
	int row;
	size_t bytes_per_row = (size_t)w * bytes_per_pixel;
	stbi_uc temp[2048];
	stbi_uc *bytes = (stbi_uc *)image;

	for (row = 0; row < (h >> 1); row++) {
		stbi_uc *row0 = bytes + row * bytes_per_row;
		stbi_uc *row1 = bytes + (h - row - 1)*bytes_per_row;
		// swap row0 with row1
		size_t bytes_left = bytes_per_row;
		while (bytes_left) {
			size_t bytes_copy = (bytes_left < sizeof(temp)) ? bytes_left : sizeof(temp);
			memcpy(temp, row0, bytes_copy);
			memcpy(row0, row1, bytes_copy);
			memcpy(row1, temp, bytes_copy);
			row0 += bytes_copy;
			row1 += bytes_copy;
			bytes_left -= bytes_copy;
		}
	}
}

#ifndef STBI_NO_GIF
static void stbi__vertical_flip_slices(void *image, int w, int h, int z, int bytes_per_pixel)
{
	int slice;
	int slice_size = w * h * bytes_per_pixel;

	stbi_uc *bytes = (stbi_uc *)image;
	for (slice = 0; slice < z; ++slice) {
		stbi__vertical_flip(bytes, w, h, bytes_per_pixel);
		bytes += slice_size;
	}
}
#endif

static unsigned char *stbi__load_and_postprocess_8bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	stbi__result_info ri;
	void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 8);

	if (result == NULL)
		return NULL;

	if (ri.bits_per_channel != 8) {
		STBI_ASSERT(ri.bits_per_channel == 16);
		result = stbi__convert_16_to_8((stbi__uint16 *)result, *x, *y, req_comp == 0 ? *comp : req_comp);
		ri.bits_per_channel = 8;
	}

	// @TODO: move stbi__convert_format to here

	if (stbi__vertically_flip_on_load) {
		int channels = req_comp ? req_comp : *comp;
		stbi__vertical_flip(result, *x, *y, channels * sizeof(stbi_uc));
	}

	return (unsigned char *)result;
}

static stbi__uint16 *stbi__load_and_postprocess_16bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	stbi__result_info ri;
	void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 16);

	if (result == NULL)
		return NULL;

	if (ri.bits_per_channel != 16) {
		STBI_ASSERT(ri.bits_per_channel == 8);
		result = stbi__convert_8_to_16((stbi_uc *)result, *x, *y, req_comp == 0 ? *comp : req_comp);
		ri.bits_per_channel = 16;
	}

	// @TODO: move stbi__convert_format16 to here
	// @TODO: special case RGB-to-Y (and RGBA-to-YA) for 8-bit-to-16-bit case to keep more precision

	if (stbi__vertically_flip_on_load) {
		int channels = req_comp ? req_comp : *comp;
		stbi__vertical_flip(result, *x, *y, channels * sizeof(stbi__uint16));
	}

	return (stbi__uint16 *)result;
}

#if !defined(STBI_NO_HDR) && !defined(STBI_NO_LINEAR)
static void stbi__float_postprocess(float *result, int *x, int *y, int *comp, int req_comp)
{
	if (stbi__vertically_flip_on_load && result != NULL) {
		int channels = req_comp ? req_comp : *comp;
		stbi__vertical_flip(result, *x, *y, channels * sizeof(float));
	}
}
#endif

#ifndef STBI_NO_STDIO

#if defined(_MSC_VER) && defined(STBI_WINDOWS_UTF8)
STBI_EXTERN __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int cp, unsigned long flags, const char *str, int cbmb, wchar_t *widestr, int cchwide);
STBI_EXTERN __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, const wchar_t *widestr, int cchwide, char *str, int cbmb, const char *defchar, int *used_default);
#endif

#if defined(_MSC_VER) && defined(STBI_WINDOWS_UTF8)
STBIDEF int stbi_convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t* input)
{
	return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#endif

static FILE *stbi__fopen(char const *filename, char const *mode)
{
	FILE *f;
#if defined(_MSC_VER) && defined(STBI_WINDOWS_UTF8)
	wchar_t wMode[64];
	wchar_t wFilename[1024];
	if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, filename, -1, wFilename, sizeof(wFilename)))
		return 0;

	if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, mode, -1, wMode, sizeof(wMode)))
		return 0;

#if _MSC_VER >= 1400
	if (0 != _wfopen_s(&f, wFilename, wMode))
		f = 0;
#else
	f = _wfopen(wFilename, wMode);
#endif

#elif defined(_MSC_VER) && _MSC_VER >= 1400
	if (0 != fopen_s(&f, filename, mode))
		f = 0;
#else
	f = fopen(filename, mode);
#endif
	return f;
}


STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	FILE *f = stbi__fopen(filename, "rb");
	unsigned char *result;
	if (!f) return stbi__errpuc("can't fopen", "Unable to open file");
	result = stbi_load_from_file(f, x, y, comp, req_comp);
	fclose(f);
	return result;
}

STBIDEF stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
	unsigned char *result;
	stbi__context s;
	stbi__start_file(&s, f);
	result = stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
	if (result) {
		// need to 'unget' all the characters in the IO buffer
		fseek(f, -(int)(s.img_buffer_end - s.img_buffer), SEEK_CUR);
	}
	return result;
}

STBIDEF stbi__uint16 *stbi_load_from_file_16(FILE *f, int *x, int *y, int *comp, int req_comp)
{
	stbi__uint16 *result;
	stbi__context s;
	stbi__start_file(&s, f);
	result = stbi__load_and_postprocess_16bit(&s, x, y, comp, req_comp);
	if (result) {
		// need to 'unget' all the characters in the IO buffer
		fseek(f, -(int)(s.img_buffer_end - s.img_buffer), SEEK_CUR);
	}
	return result;
}

STBIDEF stbi_us *stbi_load_16(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	FILE *f = stbi__fopen(filename, "rb");
	stbi__uint16 *result;
	if (!f) return (stbi_us *)stbi__errpuc("can't fopen", "Unable to open file");
	result = stbi_load_from_file_16(f, x, y, comp, req_comp);
	fclose(f);
	return result;
}


#endif //!STBI_NO_STDIO

STBIDEF stbi_us *stbi_load_16_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
	stbi__context s;
	stbi__start_mem(&s, buffer, len);
	return stbi__load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
}

STBIDEF stbi_us *stbi_load_16_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *)clbk, user);
	return stbi__load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
}

STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_mem(&s, buffer, len);
	return stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
}

STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *)clbk, user);
	return stbi__load_and_postprocess_8bit(&s, x, y, comp, req_comp);
}

#ifndef STBI_NO_GIF
STBIDEF stbi_uc *stbi_load_gif_from_memory(stbi_uc const *buffer, int len, int **delays, int *x, int *y, int *z, int *comp, int req_comp)
{
	unsigned char *result;
	stbi__context s;
	stbi__start_mem(&s, buffer, len);

	result = (unsigned char*)stbi__load_gif_main(&s, delays, x, y, z, comp, req_comp);
	if (stbi__vertically_flip_on_load) {
		stbi__vertical_flip_slices(result, *x, *y, *z, *comp);
	}

	return result;
}
#endif

#ifndef STBI_NO_LINEAR
static float *stbi__loadf_main(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	unsigned char *data;
#ifndef STBI_NO_HDR
	if (stbi__hdr_test(s)) {
		stbi__result_info ri;
		float *hdr_data = stbi__hdr_load(s, x, y, comp, req_comp, &ri);
		if (hdr_data)
			stbi__float_postprocess(hdr_data, x, y, comp, req_comp);
		return hdr_data;
	}
#endif
	data = stbi__load_and_postprocess_8bit(s, x, y, comp, req_comp);
	if (data)
		return stbi__ldr_to_hdr(data, *x, *y, req_comp ? req_comp : *comp);
	return stbi__errpf("unknown image type", "Image not of any known type, or corrupt");
}

STBIDEF float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_mem(&s, buffer, len);
	return stbi__loadf_main(&s, x, y, comp, req_comp);
}

STBIDEF float *stbi_loadf_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *)clbk, user);
	return stbi__loadf_main(&s, x, y, comp, req_comp);
}

#ifndef STBI_NO_STDIO
STBIDEF float *stbi_loadf(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	float *result;
	FILE *f = stbi__fopen(filename, "rb");
	if (!f) return stbi__errpf("can't fopen", "Unable to open file");
	result = stbi_loadf_from_file(f, x, y, comp, req_comp);
	fclose(f);
	return result;
}

STBIDEF float *stbi_loadf_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_file(&s, f);
	return stbi__loadf_main(&s, x, y, comp, req_comp);
}
#endif // !STBI_NO_STDIO

#endif // !STBI_NO_LINEAR

// these is-hdr-or-not is defined independent of whether STBI_NO_LINEAR is
// defined, for API simplicity; if STBI_NO_LINEAR is defined, it always
// reports false!

STBIDEF int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len)
{
#ifndef STBI_NO_HDR
	stbi__context s;
	stbi__start_mem(&s, buffer, len);
	return stbi__hdr_test(&s);
#else
	STBI_NOTUSED(buffer);
	STBI_NOTUSED(len);
	return 0;
#endif
}

#ifndef STBI_NO_STDIO
STBIDEF int      stbi_is_hdr(char const *filename)
{
	FILE *f = stbi__fopen(filename, "rb");
	int result = 0;
	if (f) {
		result = stbi_is_hdr_from_file(f);
		fclose(f);
	}
	return result;
}

STBIDEF int stbi_is_hdr_from_file(FILE *f)
{
#ifndef STBI_NO_HDR
	long pos = ftell(f);
	int res;
	stbi__context s;
	stbi__start_file(&s, f);
	res = stbi__hdr_test(&s);
	fseek(f, pos, SEEK_SET);
	return res;
#else
	STBI_NOTUSED(f);
	return 0;
#endif
}
#endif // !STBI_NO_STDIO

STBIDEF int      stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user)
{
#ifndef STBI_NO_HDR
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *)clbk, user);
	return stbi__hdr_test(&s);
#else
	STBI_NOTUSED(clbk);
	STBI_NOTUSED(user);
	return 0;
#endif
}

#ifndef STBI_NO_LINEAR
static float stbi__l2h_gamma = 2.2f, stbi__l2h_scale = 1.0f;

STBIDEF void   stbi_ldr_to_hdr_gamma(float gamma) { stbi__l2h_gamma = gamma; }
STBIDEF void   stbi_ldr_to_hdr_scale(float scale) { stbi__l2h_scale = scale; }
#endif

static float stbi__h2l_gamma_i = 1.0f / 2.2f, stbi__h2l_scale_i = 1.0f;

STBIDEF void   stbi_hdr_to_ldr_gamma(float gamma) { stbi__h2l_gamma_i = 1 / gamma; }
STBIDEF void   stbi_hdr_to_ldr_scale(float scale) { stbi__h2l_scale_i = 1 / scale; }


//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
	STBI__SCAN_load = 0,
	STBI__SCAN_type,
	STBI__SCAN_header
};

static void stbi__refill_buffer(stbi__context *s)
{
	int n = (s->io.read)(s->io_user_data, (char*)s->buffer_start, s->buflen);
	if (n == 0) {
		// at end of file, treat same as if from memory, but need to handle case
		// where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
		s->read_from_callbacks = 0;
		s->img_buffer = s->buffer_start;
		s->img_buffer_end = s->buffer_start + 1;
		*s->img_buffer = 0;
	}
	else {
		s->img_buffer = s->buffer_start;
		s->img_buffer_end = s->buffer_start + n;
	}
}

stbi_inline static stbi_uc stbi__get8(stbi__context *s)
{
	if (s->img_buffer < s->img_buffer_end)
		return *s->img_buffer++;
	if (s->read_from_callbacks) {
		stbi__refill_buffer(s);
		return *s->img_buffer++;
	}
	return 0;
}

stbi_inline static int stbi__at_eof(stbi__context *s)
{
	if (s->io.read) {
		if (!(s->io.eof)(s->io_user_data)) return 0;
		// if feof() is true, check if buffer = end
		// special case: we've only got the special 0 character at the end
		if (s->read_from_callbacks == 0) return 1;
	}

	return s->img_buffer >= s->img_buffer_end;
}

static void stbi__skip(stbi__context *s, int n)
{
	if (n < 0) {
		s->img_buffer = s->img_buffer_end;
		return;
	}
	if (s->io.read) {
		int blen = (int)(s->img_buffer_end - s->img_buffer);
		if (blen < n) {
			s->img_buffer = s->img_buffer_end;
			(s->io.skip)(s->io_user_data, n - blen);
			return;
		}
	}
	s->img_buffer += n;
}

static int stbi__getn(stbi__context *s, stbi_uc *buffer, int n)
{
	if (s->io.read) {
		int blen = (int)(s->img_buffer_end - s->img_buffer);
		if (blen < n) {
			int res, count;

			memcpy(buffer, s->img_buffer, blen);

			count = (s->io.read)(s->io_user_data, (char*)buffer + blen, n - blen);
			res = (count == (n - blen));
			s->img_buffer = s->img_buffer_end;
			return res;
		}
	}

	if (s->img_buffer + n <= s->img_buffer_end) {
		memcpy(buffer, s->img_buffer, n);
		s->img_buffer += n;
		return 1;
	}
	else
		return 0;
}

static int stbi__get16be(stbi__context *s)
{
	int z = stbi__get8(s);
	return (z << 8) + stbi__get8(s);
}

static stbi__uint32 stbi__get32be(stbi__context *s)
{
	stbi__uint32 z = stbi__get16be(s);
	return (z << 16) + stbi__get16be(s);
}

#if defined(STBI_NO_BMP) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF)
// nothing
#else
static int stbi__get16le(stbi__context *s)
{
	int z = stbi__get8(s);
	return z + (stbi__get8(s) << 8);
}
#endif

#ifndef STBI_NO_BMP
static stbi__uint32 stbi__get32le(stbi__context *s)
{
	stbi__uint32 z = stbi__get16le(s);
	return z + (stbi__get16le(s) << 16);
}
#endif

#define STBI__BYTECAST(x)  ((stbi_uc) ((x) & 255))  // truncate int to byte without warnings


//////////////////////////////////////////////////////////////////////////////
//
//  generic converter from built-in img_n to req_comp
//    individual types do this automatically as much as possible (e.g. jpeg
//    does all cases internally since it needs to colorspace convert anyway,
//    and it never has alpha, so very few cases ). png can automatically
//    interleave an alpha=255 channel, but falls back to this for other cases
//
//  assume data buffer is malloced, so malloc a new one and free that one
//  only failure mode is malloc failing

static stbi_uc stbi__compute_y(int r, int g, int b)
{
	return (stbi_uc)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}

static unsigned char *stbi__convert_format(unsigned char *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
	int i, j;
	unsigned char *good;

	if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

	good = (unsigned char *)stbi__malloc_mad3(req_comp, x, y, 0);
	if (good == NULL) {
		STBI_FREE(data);
		return stbi__errpuc("outofmem", "Out of memory");
	}

	for (j = 0; j < (int)y; ++j) {
		unsigned char *src = data + j * x * img_n;
		unsigned char *dest = good + j * x * req_comp;

#define STBI__COMBO(a,b)  ((a)*8+(b))
#define STBI__CASE(a,b)   case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
		// convert source image with img_n components to one with req_comp components;
		// avoid switch per pixel, so use switch per scanline and massive macros
		switch (STBI__COMBO(img_n, req_comp)) {
			STBI__CASE(1, 2) { dest[0] = src[0]; dest[1] = 255; } break;
			STBI__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; } break;
			STBI__CASE(1, 4) { dest[0] = dest[1] = dest[2] = src[0]; dest[3] = 255; } break;
			STBI__CASE(2, 1) { dest[0] = src[0]; } break;
			STBI__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; } break;
			STBI__CASE(2, 4) { dest[0] = dest[1] = dest[2] = src[0]; dest[3] = src[1]; } break;
			STBI__CASE(3, 4) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = 255; } break;
			STBI__CASE(3, 1) { dest[0] = stbi__compute_y(src[0], src[1], src[2]); } break;
			STBI__CASE(3, 2) { dest[0] = stbi__compute_y(src[0], src[1], src[2]); dest[1] = 255; } break;
			STBI__CASE(4, 1) { dest[0] = stbi__compute_y(src[0], src[1], src[2]); } break;
			STBI__CASE(4, 2) { dest[0] = stbi__compute_y(src[0], src[1], src[2]); dest[1] = src[3]; } break;
			STBI__CASE(4, 3) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; } break;
		default: STBI_ASSERT(0);
		}
#undef STBI__CASE
	}

	STBI_FREE(data);
	return good;
}

static stbi__uint16 stbi__compute_y_16(int r, int g, int b)
{
	return (stbi__uint16)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}

static stbi__uint16 *stbi__convert_format16(stbi__uint16 *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
	int i, j;
	stbi__uint16 *good;

	if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

	good = (stbi__uint16 *)stbi__malloc(req_comp * x * y * 2);
	if (good == NULL) {
		STBI_FREE(data);
		return (stbi__uint16 *)stbi__errpuc("outofmem", "Out of memory");
	}

	for (j = 0; j < (int)y; ++j) {
		stbi__uint16 *src = data + j * x * img_n;
		stbi__uint16 *dest = good + j * x * req_comp;

#define STBI__COMBO(a,b)  ((a)*8+(b))
#define STBI__CASE(a,b)   case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
		// convert source image with img_n components to one with req_comp components;
		// avoid switch per pixel, so use switch per scanline and massive macros
		switch (STBI__COMBO(img_n, req_comp)) {
			STBI__CASE(1, 2) { dest[0] = src[0]; dest[1] = 0xffff; } break;
			STBI__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; } break;
			STBI__CASE(1, 4) { dest[0] = dest[1] = dest[2] = src[0]; dest[3] = 0xffff; } break;
			STBI__CASE(2, 1) { dest[0] = src[0]; } break;
			STBI__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; } break;
			STBI__CASE(2, 4) { dest[0] = dest[1] = dest[2] = src[0]; dest[3] = src[1]; } break;
			STBI__CASE(3, 4) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = 0xffff; } break;
			STBI__CASE(3, 1) { dest[0] = stbi__compute_y_16(src[0], src[1], src[2]); } break;
			STBI__CASE(3, 2) { dest[0] = stbi__compute_y_16(src[0], src[1], src[2]); dest[1] = 0xffff; } break;
			STBI__CASE(4, 1) { dest[0] = stbi__compute_y_16(src[0], src[1], src[2]); } break;
			STBI__CASE(4, 2) { dest[0] = stbi__compute_y_16(src[0], src[1], src[2]); dest[1] = src[3]; } break;
			STBI__CASE(4, 3) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; } break;
		default: STBI_ASSERT(0);
		}
#undef STBI__CASE
	}

	STBI_FREE(data);
	return good;
}

#ifndef STBI_NO_LINEAR
static float   *stbi__ldr_to_hdr(stbi_uc *data, int x, int y, int comp)
{
	int i, k, n;
	float *output;
	if (!data) return NULL;
	output = (float *)stbi__malloc_mad4(x, y, comp, sizeof(float), 0);
	if (output == NULL) { STBI_FREE(data); return stbi__errpf("outofmem", "Out of memory"); }
	// compute number of non-alpha components
	if (comp & 1) n = comp; else n = comp - 1;
	for (i = 0; i < x*y; ++i) {
		for (k = 0; k < n; ++k) {
			output[i*comp + k] = (float)(pow(data[i*comp + k] / 255.0f, stbi__l2h_gamma) * stbi__l2h_scale);
		}
	}
	if (n < comp) {
		for (i = 0; i < x*y; ++i) {
			output[i*comp + n] = data[i*comp + n] / 255.0f;
		}
	}
	STBI_FREE(data);
	return output;
}
#endif

#ifndef STBI_NO_HDR
#define stbi__float2int(x)   ((int) (x))
static stbi_uc *stbi__hdr_to_ldr(float   *data, int x, int y, int comp)
{
	int i, k, n;
	stbi_uc *output;
	if (!data) return NULL;
	output = (stbi_uc *)stbi__malloc_mad3(x, y, comp, 0);
	if (output == NULL) { STBI_FREE(data); return stbi__errpuc("outofmem", "Out of memory"); }
	// compute number of non-alpha components
	if (comp & 1) n = comp; else n = comp - 1;
	for (i = 0; i < x*y; ++i) {
		for (k = 0; k < n; ++k) {
			float z = (float)pow(data[i*comp + k] * stbi__h2l_scale_i, stbi__h2l_gamma_i) * 255 + 0.5f;
			if (z < 0) z = 0;
			if (z > 255) z = 255;
			output[i*comp + k] = (stbi_uc)stbi__float2int(z);
		}
		if (k < comp) {
			float z = data[i*comp + k] * 255 + 0.5f;
			if (z < 0) z = 0;
			if (z > 255) z = 255;
			output[i*comp + k] = (stbi_uc)stbi__float2int(z);
		}
	}
	STBI_FREE(data);
	return output;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//  "baseline" JPEG/JFIF decoder
//
//    simple implementation
//      - doesn't support delayed output of y-dimension
//      - simple interface (only one output format: 8-bit interleaved RGB)
//      - doesn't try to recover corrupt jpegs
//      - doesn't allow partial loading, loading multiple at once
//      - still fast on x86 (copying globals into locals doesn't help x86)
//      - allocates lots of intermediate memory (full size of all components)
//        - non-interleaved case requires this anyway
//        - allows good upsampling (see next)
//    high-quality
//      - upsampled channels are bilinearly interpolated, even across blocks
//      - quality integer IDCT derived from IJG's 'slow'
//    performance
//      - fast huffman; reasonable integer IDCT
//      - some SIMD kernels for common paths on targets with SSE2/NEON
//      - uses a lot of intermediate memory, could cache poorly

#ifndef STBI_NO_JPEG

// huffman decoding acceleration
#define FAST_BITS   9  // larger handles more cases; smaller stomps less cache

typedef struct
{
	stbi_uc  fast[1 << FAST_BITS];
	// weirdly, repacking this into AoS is a 10% speed loss, instead of a win
	stbi__uint16 code[256];
	stbi_uc  values[256];
	stbi_uc  size[257];
	unsigned int maxcode[18];
	int    delta[17];   // old 'firstsymbol' - old 'firstcode'
} stbi__huffman;

typedef struct
{
	stbi__context *s;
	stbi__huffman huff_dc[4];
	stbi__huffman huff_ac[4];
	stbi__uint16 dequant[4][64];
	stbi__int16 fast_ac[4][1 << FAST_BITS];

	// sizes for components, interleaved MCUs
	int img_h_max, img_v_max;
	int img_mcu_x, img_mcu_y;
	int img_mcu_w, img_mcu_h;

	// definition of jpeg image component
	struct
	{
		int id;
		int h, v;
		int tq;
		int hd, ha;
		int dc_pred;

		int x, y, w2, h2;
		stbi_uc *data;
		void *raw_data, *raw_coeff;
		stbi_uc *linebuf;
		short   *coeff;   // progressive only
		int      coeff_w, coeff_h; // number of 8x8 coefficient blocks
	} img_comp[4];

	stbi__uint32   code_buffer; // jpeg entropy-coded buffer
	int            code_bits;   // number of valid bits
	unsigned char  marker;      // marker seen while filling entropy buffer
	int            nomore;      // flag if we saw a marker so must stop

	int            progressive;
	int            spec_start;
	int            spec_end;
	int            succ_high;
	int            succ_low;
	int            eob_run;
	int            jfif;
	int            app14_color_transform; // Adobe APP14 tag
	int            rgb;

	int scan_n, order[4];
	int restart_interval, todo;

	// kernels
	void(*idct_block_kernel)(stbi_uc *out, int out_stride, short data[64]);
	void(*YCbCr_to_RGB_kernel)(stbi_uc *out, const stbi_uc *y, const stbi_uc *pcb, const stbi_uc *pcr, int count, int step);
	stbi_uc *(*resample_row_hv_2_kernel)(stbi_uc *out, stbi_uc *in_near, stbi_uc *in_far, int w, int hs);
} stbi__jpeg;

static int stbi__build_huffman(stbi__huffman *h, int *count)
{
	int i, j, k = 0;
	unsigned int code;
	// build size list for each symbol (from JPEG spec)
	for (i = 0; i < 16; ++i)
		for (j = 0; j < count[i]; ++j)
			h->size[k++] = (stbi_uc)(i + 1);
	h->size[k] = 0;

	// compute actual symbols (from jpeg spec)
	code = 0;
	k = 0;
	for (j = 1; j <= 16; ++j) {
		// compute delta to add to code to compute symbol id
		h->delta[j] = k - code;
		if (h->size[k] == j) {
			while (h->size[k] == j)
				h->code[k++] = (stbi__uint16)(code++);
			if (code - 1 >= (1u << j)) return stbi__err("bad code lengths", "Corrupt JPEG");
		}
		// compute largest code + 1 for this size, preshifted as needed later
		h->maxcode[j] = code << (16 - j);
		code <<= 1;
	}
	h->maxcode[j] = 0xffffffff;

	// build non-spec acceleration table; 255 is flag for not-accelerated
	memset(h->fast, 255, 1 << FAST_BITS);
	for (i = 0; i < k; ++i) {
		int s = h->size[i];
		if (s <= FAST_BITS) {
			int c = h->code[i] << (FAST_BITS - s);
			int m = 1 << (FAST_BITS - s);
			for (j = 0; j < m; ++j) {
				h->fast[c + j] = (stbi_uc)i;
			}
		}
	}
	return 1;
}

// build a table that decodes both magnitude and value of small ACs in
// one go.
static void stbi__build_fast_ac(stbi__int16 *fast_ac, stbi__huffman *h)
{
	int i;
	for (i = 0; i < (1 << FAST_BITS); ++i) {
		stbi_uc fast = h->fast[i];
		fast_ac[i] = 0;
		if (fast < 255) {
			int rs = h->values[fast];
			int run = (rs >> 4) & 15;
			int magbits = rs & 15;
			int len = h->size[fast];

			if (magbits && len + magbits <= FAST_BITS) {
				// magnitude code followed by receive_extend code
				int k = ((i << len) & ((1 << FAST_BITS) - 1)) >> (FAST_BITS - magbits);
				int m = 1 << (magbits - 1);
				if (k < m) k += (~0U << magbits) + 1;
				// if the result is small enough, we can fit it in fast_ac table
				if (k >= -128 && k <= 127)
					fast_ac[i] = (stbi__int16)((k * 256) + (run * 16) + (len + magbits));
			}
		}
	}
}

static void stbi__grow_buffer_unsafe(stbi__jpeg *j)
{
	do {
		unsigned int b = j->nomore ? 0 : stbi__get8(j->s);
		if (b == 0xff) {
			int c = stbi__get8(j->s);
			while (c == 0xff) c = stbi__get8(j->s); // consume fill bytes
			if (c != 0) {
				j->marker = (unsigned char)c;
				j->nomore = 1;
				return;
			}
		}
		j->code_buffer |= b << (24 - j->code_bits);
		j->code_bits += 8;
	} while (j->code_bits <= 24);
}

// (1 << n) - 1
static const stbi__uint32 stbi__bmask[17] = { 0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535 };

// decode a jpeg huffman value from the bitstream
stbi_inline static int stbi__jpeg_huff_decode(stbi__jpeg *j, stbi__huffman *h)
{
	unsigned int temp;
	int c, k;

	if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);

	// look at the top FAST_BITS and determine what symbol ID it is,
	// if the code is <= FAST_BITS
	c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
	k = h->fast[c];
	if (k < 255) {
		int s = h->size[k];
		if (s > j->code_bits)
			return -1;
		j->code_buffer <<= s;
		j->code_bits -= s;
		return h->values[k];
	}

	// naive test is to shift the code_buffer down so k bits are
	// valid, then test against maxcode. To speed this up, we've
	// preshifted maxcode left so that it has (16-k) 0s at the
	// end; in other words, regardless of the number of bits, it
	// wants to be compared against something shifted to have 16;
	// that way we don't need to shift inside the loop.
	temp = j->code_buffer >> 16;
	for (k = FAST_BITS + 1; ; ++k)
		if (temp < h->maxcode[k])
			break;
	if (k == 17) {
		// error! code not found
		j->code_bits -= 16;
		return -1;
	}

	if (k > j->code_bits)
		return -1;

	// convert the huffman code to the symbol id
	c = ((j->code_buffer >> (32 - k)) & stbi__bmask[k]) + h->delta[k];
	STBI_ASSERT((((j->code_buffer) >> (32 - h->size[c])) & stbi__bmask[h->size[c]]) == h->code[c]);

	// convert the id to a symbol
	j->code_bits -= k;
	j->code_buffer <<= k;
	return h->values[c];
}

// bias[n] = (-1<<n) + 1
static const int stbi__jbias[16] = { 0,-1,-3,-7,-15,-31,-63,-127,-255,-511,-1023,-2047,-4095,-8191,-16383,-32767 };

// combined JPEG 'receive' and JPEG 'extend', since baseline
// always extends everything it receives.
stbi_inline static int stbi__extend_receive(stbi__jpeg *j, int n)
{
	unsigned int k;
	int sgn;
	if (j->code_bits < n) stbi__grow_buffer_unsafe(j);

	sgn = (stbi__int32)j->code_buffer >> 31; // sign bit is always in MSB
	k = stbi_lrot(j->code_buffer, n);
	STBI_ASSERT(n >= 0 && n < (int)(sizeof(stbi__bmask) / sizeof(*stbi__bmask)));
	j->code_buffer = k & ~stbi__bmask[n];
	k &= stbi__bmask[n];
	j->code_bits -= n;
	return k + (stbi__jbias[n] & ~sgn);
}

// get some unsigned bits
stbi_inline static int stbi__jpeg_get_bits(stbi__jpeg *j, int n)
{
	unsigned int k;
	if (j->code_bits < n) stbi__grow_buffer_unsafe(j);
	k = stbi_lrot(j->code_buffer, n);
	j->code_buffer = k & ~stbi__bmask[n];
	k &= stbi__bmask[n];
	j->code_bits -= n;
	return k;
}

stbi_inline static int stbi__jpeg_get_bit(stbi__jpeg *j)
{
	unsigned int k;
	if (j->code_bits < 1) stbi__grow_buffer_unsafe(j);
	k = j->code_buffer;
	j->code_buffer <<= 1;
	--j->code_bits;
	return k & 0x80000000;
}

// given a value that's at position X in the zigzag stream,
// where does it appear in the 8x8 matrix coded as row-major?
static const stbi_uc stbi__jpeg_dezigzag[64 + 15] =
{
	0,  1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46,
   53, 60, 61, 54, 47, 55, 62, 63,
   // let corrupt input sample past end
   63, 63, 63, 63, 63, 63, 63, 63,
   63, 63, 63, 63, 63, 63, 63
};

// decode one 64-entry block--
static int stbi__jpeg_decode_block(stbi__jpeg *j, short data[64], stbi__huffman *hdc, stbi__huffman *hac, stbi__int16 *fac, int b, stbi__uint16 *dequant)
{
	int diff, dc, k;
	int t;

	if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
	t = stbi__jpeg_huff_decode(j, hdc);
	if (t < 0) return stbi__err("bad huffman code", "Corrupt JPEG");

	// 0 all the ac values now so we can do it 32-bits at a time
	memset(data, 0, 64 * sizeof(data[0]));

	diff = t ? stbi__extend_receive(j, t) : 0;
	dc = j->img_comp[b].dc_pred + diff;
	j->img_comp[b].dc_pred = dc;
	data[0] = (short)(dc * dequant[0]);

	// decode AC components, see JPEG spec
	k = 1;
	do {
		unsigned int zig;
		int c, r, s;
		if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
		c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
		r = fac[c];
		if (r) { // fast-AC path
			k += (r >> 4) & 15; // run
			s = r & 15; // combined length
			j->code_buffer <<= s;
			j->code_bits -= s;
			// decode into unzigzag'd location
			zig = stbi__jpeg_dezigzag[k++];
			data[zig] = (short)((r >> 8) * dequant[zig]);
		}
		else {
			int rs = stbi__jpeg_huff_decode(j, hac);
			if (rs < 0) return stbi__err("bad huffman code", "Corrupt JPEG");
			s = rs & 15;
			r = rs >> 4;
			if (s == 0) {
				if (rs != 0xf0) break; // end block
				k += 16;
			}
			else {
				k += r;
				// decode into unzigzag'd location
				zig = stbi__jpeg_dezigzag[k++];
				data[zig] = (short)(stbi__extend_receive(j, s) * dequant[zig]);
			}
		}
	} while (k < 64);
	return 1;
}

static int stbi__jpeg_decode_block_prog_dc(stbi__jpeg *j, short data[64], stbi__huffman *hdc, int b)
{
	int diff, dc;
	int t;
	if (j->spec_end != 0) return stbi__err("can't merge dc and ac", "Corrupt JPEG");

	if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);

	if (j->succ_high == 0) {
		// first scan for DC coefficient, must be first
		memset(data, 0, 64 * sizeof(data[0])); // 0 all the ac values now
		t = stbi__jpeg_huff_decode(j, hdc);
		diff = t ? stbi__extend_receive(j, t) : 0;

		dc = j->img_comp[b].dc_pred + diff;
		j->img_comp[b].dc_pred = dc;
		data[0] = (short)(dc << j->succ_low);
	}
	else {
		// refinement scan for DC coefficient
		if (stbi__jpeg_get_bit(j))
			data[0] += (short)(1 << j->succ_low);
	}
	return 1;
}

// @OPTIMIZE: store non-zigzagged during the decode passes,
// and only de-zigzag when dequantizing
static int stbi__jpeg_decode_block_prog_ac(stbi__jpeg *j, short data[64], stbi__huffman *hac, stbi__int16 *fac)
{
	int k;
	if (j->spec_start == 0) return stbi__err("can't merge dc and ac", "Corrupt JPEG");

	if (j->succ_high == 0) {
		int shift = j->succ_low;

		if (j->eob_run) {
			--j->eob_run;
			return 1;
		}

		k = j->spec_start;
		do {
			unsigned int zig;
			int c, r, s;
			if (j->code_bits < 16) stbi__grow_buffer_unsafe(j);
			c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
			r = fac[c];
			if (r) { // fast-AC path
				k += (r >> 4) & 15; // run
				s = r & 15; // combined length
				j->code_buffer <<= s;
				j->code_bits -= s;
				zig = stbi__jpeg_dezigzag[k++];
				data[zig] = (short)((r >> 8) << shift);
			}
			else {
				int rs = stbi__jpeg_huff_decode(j, hac);
				if (rs < 0) return stbi__err("bad huffman code", "Corrupt JPEG");
				s = rs & 15;
				r = rs >> 4;
				if (s == 0) {
					if (r < 15) {
						j->eob_run = (1 << r);
						if (r)
							j->eob_run += stbi__jpeg_get_bits(j, r);
						--j->eob_run;
						break;
					}
					k += 16;
				}
				else {
					k += r;
					zig = stbi__jpeg_dezigzag[k++];
					data[zig] = (short)(stbi__extend_receive(j, s) << shift);
				}
			}
		} while (k <= j->spec_end);
	}
	else {
		// refinement scan for these AC coefficients

		short bit = (short)(1 << j->succ_low);

		if (j->eob_run) {
			--j->eob_run;
			for (k = j->spec_start; k <= j->spec_end; ++k) {
				short *p = &data[stbi__jpeg_dezigzag[k]];
				if (*p != 0)
					if (stbi__jpeg_get_bit(j))
						if ((*p & bit) == 0) {
							if (*p > 0)
								*p += bit;
							else
								*p -= bit;
						}
			}
		}
		else {
			k = j->spec_start;
			do {
				int r, s;
				int rs = stbi__jpeg_huff_decode(j, hac); // @OPTIMIZE see if we can use the fast path here, advance-by-r is so slow, eh
				if (rs < 0) return stbi__err("bad huffman code", "Corrupt JPEG");
				s = rs & 15;
				r = rs >> 4;
				if (s == 0) {
					if (r < 15) {
						j->eob_run = (1 << r) - 1;
						if (r)
							j->eob_run += stbi__jpeg_get_bits(j, r);
						r = 64; // force end of block
					}
					else {
						// r=15 s=0 should write 16 0s, so we just do
						// a run of 15 0s and then write s (which is 0),
						// so we don't have to do anything special here
					}
				}
				else {
					if (s != 1) return stbi__err("bad huffman code", "Corrupt JPEG");
					// sign bit
					if (stbi__jpeg_get_bit(j))
						s = bit;
					else
						s = -bit;
				}

				// advance by r
				while (k <= j->spec_end) {
					short *p = &data[stbi__jpeg_dezigzag[k++]];
					if (*p != 0) {
						if (stbi__jpeg_get_bit(j))
							if ((*p & bit) == 0) {
								if (*p > 0)
									*p += bit;
								else
									*p -= bit;
							}
					}
					else {
						if (r == 0) {
							*p = (short)s;
							break;
						}
						--r;
					}
				}
			} while (k <= j->spec_end);
		}
	}
	return 1;
}

// take a -128..127 value and stbi__clamp it and convert to 0..255
stbi_inline static stbi_uc stbi__clamp(int x)
{
	// trick to use a single test to catch both cases
	if ((unsigned int)x > 255) {
		if (x < 0) return 0;
		if (x > 255) return 255;
	}
	return (stbi_uc)x;
}

#define stbi__f2f(x)  ((int) (((x) * 4096 + 0.5)))
#define stbi__fsh(x)  ((x) * 4096)

// derived from jidctint -- DCT_ISLOW
#define STBI__IDCT_1D(s0,s1,s2,s3,s4,s5,s6,s7) \
   int t0,t1,t2,t3,p1,p2,p3,p4,p5,x0,x1,x2,x3; \
   p2 = s2;                                    \
   p3 = s6;                                    \
   p1 = (p2+p3) * stbi__f2f(0.5411961f);       \
   t2 = p1 + p3*stbi__f2f(-1.847759065f);      \
   t3 = p1 + p2*stbi__f2f( 0.765366865f);      \
   p2 = s0;                                    \
   p3 = s4;                                    \
   t0 = stbi__fsh(p2+p3);                      \
   t1 = stbi__fsh(p2-p3);                      \
   x0 = t0+t3;                                 \
   x3 = t0-t3;                                 \
   x1 = t1+t2;                                 \
   x2 = t1-t2;                                 \
   t0 = s7;                                    \
   t1 = s5;                                    \
   t2 = s3;                                    \
   t3 = s1;                                    \
   p3 = t0+t2;                                 \
   p4 = t1+t3;                                 \
   p1 = t0+t3;                                 \
   p2 = t1+t2;                                 \
   p5 = (p3+p4)*stbi__f2f( 1.175875602f);      \
   t0 = t0*stbi__f2f( 0.298631336f);           \
   t1 = t1*stbi__f2f( 2.053119869f);           \
   t2 = t2*stbi__f2f( 3.072711026f);           \
   t3 = t3*stbi__f2f( 1.501321110f);           \
   p1 = p5 + p1*stbi__f2f(-0.899976223f);      \
   p2 = p5 + p2*stbi__f2f(-2.562915447f);      \
   p3 = p3*stbi__f2f(-1.961570560f);           \
   p4 = p4*stbi__f2f(-0.390180644f);           \
   t3 += p1+p4;                                \
   t2 += p2+p3;                                \
   t1 += p2+p4;                                \
   t0 += p1+p3;

static void stbi__idct_block(stbi_uc *out, int out_stride, short data[64])
{
	int i, val[64], *v = val;
	stbi_uc *o;
	short *d = data;

	// columns
	for (i = 0; i < 8; ++i, ++d, ++v) {
		// if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
		if (d[8] == 0 && d[16] == 0 && d[24] == 0 && d[32] == 0
			&& d[40] == 0 && d[48] == 0 && d[56] == 0) {
			//    no shortcut                 0     seconds
			//    (1|2|3|4|5|6|7)==0          0     seconds
			//    all separate               -0.047 seconds
			//    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
			int dcterm = d[0] * 4;
			v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
		}
		else {
			STBI__IDCT_1D(d[0], d[8], d[16], d[24], d[32], d[40], d[48], d[56])
				// constants scaled things up by 1<<12; let's bring them back
				// down, but keep 2 extra bits of precision
				x0 += 512; x1 += 512; x2 += 512; x3 += 512;
			v[0] = (x0 + t3) >> 10;
			v[56] = (x0 - t3) >> 10;
			v[8] = (x1 + t2) >> 10;
			v[48] = (x1 - t2) >> 10;
			v[16] = (x2 + t1) >> 10;
			v[40] = (x2 - t1) >> 10;
			v[24] = (x3 + t0) >> 10;
			v[32] = (x3 - t0) >> 10;
		}
	}

	for (i = 0, v = val, o = out; i < 8; ++i, v += 8, o += out_stride) {
		// no fast case since the first 1D IDCT spread components out
		STBI__IDCT_1D(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7])
			// constants scaled things up by 1<<12, plus we had 1<<2 from first
			// loop, plus horizontal and vertical each scale by sqrt(8) so together
			// we've got an extra 1<<3, so 1<<17 total we need to remove.
			// so we want to round that, which means adding 0.5 * 1<<17,
			// aka 65536. Also, we'll end up with -128 to 127 that we want
			// to encode as 0..255 by adding 128, so we'll add that before the shift
			x0 += 65536 + (128 << 17);
		x1 += 65536 + (128 << 17);
		x2 += 65536 + (128 << 17);
		x3 += 65536 + (128 << 17);
		// tried computing the shifts into temps, or'ing the temps to see
		// if any were out of range, but that was slower
		o[0] = stbi__clamp((x0 + t3) >> 17);
		o[7] = stbi__clamp((x0 - t3) >> 17);
		o[1] = stbi__clamp((x1 + t2) >> 17);
		o[6] = stbi__clamp((x1 - t2) >> 17);
		o[2] = stbi__clamp((x2 + t1) >> 17);
		o[5] = stbi__clamp((x2 - t1) >> 17);
		o[3] = stbi__clamp((x3 + t0) >> 17);
		o[4] = stbi__clamp((x3 - t0) >> 17);
	}
}

#ifdef STBI_SSE2
// sse2 integer IDCT. not the fastest possible implementation but it
// produces bit-identical results to the generic C version so it's
// fully "transparent".
static void stbi__idct_simd(stbi_uc *out, int out_stride, short data[64])
{
	// This is constructed to match our regular (generic) integer IDCT exactly.
	__m128i row0, row1, row2, row3, row4, row5, row6, row7;
	__m128i tmp;

	// dot product constant: even elems=x, odd elems=y
#define dct_const(x,y)  _mm_setr_epi16((x),(y),(x),(y),(x),(y),(x),(y))

// out(0) = c0[even]*x + c0[odd]*y   (c0, x, y 16-bit, out 32-bit)
// out(1) = c1[even]*x + c1[odd]*y
#define dct_rot(out0,out1, x,y,c0,c1) \
      __m128i c0##lo = _mm_unpacklo_epi16((x),(y)); \
      __m128i c0##hi = _mm_unpackhi_epi16((x),(y)); \
      __m128i out0##_l = _mm_madd_epi16(c0##lo, c0); \
      __m128i out0##_h = _mm_madd_epi16(c0##hi, c0); \
      __m128i out1##_l = _mm_madd_epi16(c0##lo, c1); \
      __m128i out1##_h = _mm_madd_epi16(c0##hi, c1)

   // out = in << 12  (in 16-bit, out 32-bit)
#define dct_widen(out, in) \
      __m128i out##_l = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), (in)), 4); \
      __m128i out##_h = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), (in)), 4)

   // wide add
#define dct_wadd(out, a, b) \
      __m128i out##_l = _mm_add_epi32(a##_l, b##_l); \
      __m128i out##_h = _mm_add_epi32(a##_h, b##_h)

   // wide sub
#define dct_wsub(out, a, b) \
      __m128i out##_l = _mm_sub_epi32(a##_l, b##_l); \
      __m128i out##_h = _mm_sub_epi32(a##_h, b##_h)

   // butterfly a/b, add bias, then shift by "s" and pack
#define dct_bfly32o(out0, out1, a,b,bias,s) \
      { \
         __m128i abiased_l = _mm_add_epi32(a##_l, bias); \
         __m128i abiased_h = _mm_add_epi32(a##_h, bias); \
         dct_wadd(sum, abiased, b); \
         dct_wsub(dif, abiased, b); \
         out0 = _mm_packs_epi32(_mm_srai_epi32(sum_l, s), _mm_srai_epi32(sum_h, s)); \
         out1 = _mm_packs_epi32(_mm_srai_epi32(dif_l, s), _mm_srai_epi32(dif_h, s)); \
      }

   // 8-bit interleave step (for transposes)
#define dct_interleave8(a, b) \
      tmp = a; \
      a = _mm_unpacklo_epi8(a, b); \
      b = _mm_unpackhi_epi8(tmp, b)

   // 16-bit interleave step (for transposes)
#define dct_interleave16(a, b) \
      tmp = a; \
      a = _mm_unpacklo_epi16(a, b); \
      b = _mm_unpackhi_epi16(tmp, b)

#define dct_pass(bias,shift) \
      { \
         /* even part */ \
         dct_rot(t2e,t3e, row2,row6, rot0_0,rot0_1); \
         __m128i sum04 = _mm_add_epi16(row0, row4); \
         __m128i dif04 = _mm_sub_epi16(row0, row4); \
         dct_widen(t0e, sum04); \
         dct_widen(t1e, dif04); \
         dct_wadd(x0, t0e, t3e); \
         dct_wsub(x3, t0e, t3e); \
         dct_wadd(x1, t1e, t2e); \
         dct_wsub(x2, t1e, t2e); \
         /* odd part */ \
         dct_rot(y0o,y2o, row7,row3, rot2_0,rot2_1); \
         dct_rot(y1o,y3o, row5,row1, rot3_0,rot3_1); \
         __m128i sum17 = _mm_add_epi16(row1, row7); \
         __m128i sum35 = _mm_add_epi16(row3, row5); \
         dct_rot(y4o,y5o, sum17,sum35, rot1_0,rot1_1); \
         dct_wadd(x4, y0o, y4o); \
         dct_wadd(x5, y1o, y5o); \
         dct_wadd(x6, y2o, y5o); \
         dct_wadd(x7, y3o, y4o); \
         dct_bfly32o(row0,row7, x0,x7,bias,shift); \
         dct_bfly32o(row1,row6, x1,x6,bias,shift); \
         dct_bfly32o(row2,row5, x2,x5,bias,shift); \
         dct_bfly32o(row3,row4, x3,x4,bias,shift); \
      }

	__m128i rot0_0 = dct_const(stbi__f2f(0.5411961f), stbi__f2f(0.5411961f) + stbi__f2f(-1.847759065f));
	__m128i rot0_1 = dct_const(stbi__f2f(0.5411961f) + stbi__f2f(0.765366865f), stbi__f2f(0.5411961f));
	__m128i rot1_0 = dct_const(stbi__f2f(1.175875602f) + stbi__f2f(-0.899976223f), stbi__f2f(1.175875602f));
	__m128i rot1_1 = dct_const(stbi__f2f(1.175875602f), stbi__f2f(1.175875602f) + stbi__f2f(-2.562915447f));
	__m128i rot2_0 = dct_const(stbi__f2f(-1.961570560f) + stbi__f2f(0.298631336f), stbi__f2f(-1.961570560f));
	__m128i rot2_1 = dct_const(stbi__f2f(-1.961570560f), stbi__f2f(-1.961570560f) + stbi__f2f(3.072711026f));
	__m128i rot3_0 = dct_const(stbi__f2f(-0.390180644f) + stbi__f2f(2.053119869f), stbi__f2f(-0.390180644f));
	__m128i rot3_1 = dct_const(stbi__f2f(-0.390180644f), stbi__f2f(-0.390180644f) + stbi__f2f(1.501321110f));

	// rounding biases in column/row passes, see stbi__idct_block for explanation.
	__m128i bias_0 = _mm_set1_epi32(512);
	__m128i bias_1 = _mm_set1_epi32(65536 + (128 << 17));

	// load
	row0 = _mm_load_si128((const __m128i *) (data + 0 * 8));
	row1 = _mm_load_si128((const __m128i *) (data + 1 * 8));
	row2 = _mm_load_si128((const __m128i *) (data + 2 * 8));
	row3 = _mm_load_si128((const __m128i *) (data + 3 * 8));
	row4 = _mm_load_si128((const __m128i *) (data + 4 * 8));
	row5 = _mm_load_si128((const __m128i *) (data + 5 * 8));
	row6 = _mm_load_si128((const __m128i *) (data + 6 * 8));
	row7 = _mm_load_si128((const __m128i *) (data + 7 * 8));

	// column pass
	dct_pass(bias_0, 10);

	{
		// 16bit 8x8 transpose pass 1
		dct_interleave16(row0, row4);
		dct_interleave16(row1, row5);
		dct_interleave16(row2, row6);
		dct_interleave16(row3, row7);

		// transpose pass 2
		dct_interleave16(row0, row2);
		dct_interleave16(row1, row3);
		dct_interleave16(row4, row6);
		dct_interleave16(row5, row7);

		// transpose pass 3
		dct_interleave16(row0, row1);
		dct_interleave16(row2, row3);
		dct_interleave16(row4, row5);
		dct_interleave16(row6, row7);
	}

	// row pass
	dct_pass(bias_1, 17);

	{
		// pack
		__m128i p0 = _mm_packus_epi16(row0, row1); // a0a1a2a3...a7b0b1b2b3...b7
		__m128i p1 = _mm_packus_epi16(row2, row3);
		__m128i p2 = _mm_packus_epi16(row4, row5);
		__m128i p3 = _mm_packus_epi16(row6, row7);

		// 8bit 8x8 transpose pass 1
		dct_interleave8(p0, p2); // a0e0a1e1...
		dct_interleave8(p1, p3); // c0g0c1g1...

		// transpose pass 2
		dct_interleave8(p0, p1); // a0c0e0g0...
		dct_interleave8(p2, p3); // b0d0f0h0...

		// transpose pass 3
		dct_interleave8(p0, p2); // a0b0c0d0...
		dct_interleave8(p1, p3); // a4b4c4d4...

		// store
		_mm_storel_epi64((__m128i *) out, p0); out += out_stride;
		_mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p0, 0x4e)); out += out_stride;
		_mm_storel_epi64((__m128i *) out, p2); out += out_stride;
		_mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p2, 0x4e)); out += out_stride;
		_mm_storel_epi64((__m128i *) out, p1); out += out_stride;
		_mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p1, 0x4e)); out += out_stride;
		_mm_storel_epi64((__m128i *) out, p3); out += out_stride;
		_mm_storel_epi64((__m128i *) out, _mm_shuffle_epi32(p3, 0x4e));
	}

#undef dct_const
#undef dct_rot
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_interleave8
#undef dct_interleave16
#undef dct_pass
}

#endif // STBI_SSE2

#ifdef STBI_NEON

// NEON integer IDCT. should produce bit-identical
// results to the generic C version.
static void stbi__idct_simd(stbi_uc *out, int out_stride, short data[64])
{
	int16x8_t row0, row1, row2, row3, row4, row5, row6, row7;

	int16x4_t rot0_0 = vdup_n_s16(stbi__f2f(0.5411961f));
	int16x4_t rot0_1 = vdup_n_s16(stbi__f2f(-1.847759065f));
	int16x4_t rot0_2 = vdup_n_s16(stbi__f2f(0.765366865f));
	int16x4_t rot1_0 = vdup_n_s16(stbi__f2f(1.175875602f));
	int16x4_t rot1_1 = vdup_n_s16(stbi__f2f(-0.899976223f));
	int16x4_t rot1_2 = vdup_n_s16(stbi__f2f(-2.562915447f));
	int16x4_t rot2_0 = vdup_n_s16(stbi__f2f(-1.961570560f));
	int16x4_t rot2_1 = vdup_n_s16(stbi__f2f(-0.390180644f));
	int16x4_t rot3_0 = vdup_n_s16(stbi__f2f(0.298631336f));
	int16x4_t rot3_1 = vdup_n_s16(stbi__f2f(2.053119869f));
	int16x4_t rot3_2 = vdup_n_s16(stbi__f2f(3.072711026f));
	int16x4_t rot3_3 = vdup_n_s16(stbi__f2f(1.501321110f));

#define dct_long_mul(out, inq, coeff) \
   int32x4_t out##_l = vmull_s16(vget_low_s16(inq), coeff); \
   int32x4_t out##_h = vmull_s16(vget_high_s16(inq), coeff)

#define dct_long_mac(out, acc, inq, coeff) \
   int32x4_t out##_l = vmlal_s16(acc##_l, vget_low_s16(inq), coeff); \
   int32x4_t out##_h = vmlal_s16(acc##_h, vget_high_s16(inq), coeff)

#define dct_widen(out, inq) \
   int32x4_t out##_l = vshll_n_s16(vget_low_s16(inq), 12); \
   int32x4_t out##_h = vshll_n_s16(vget_high_s16(inq), 12)

	// wide add
#define dct_wadd(out, a, b) \
   int32x4_t out##_l = vaddq_s32(a##_l, b##_l); \
   int32x4_t out##_h = vaddq_s32(a##_h, b##_h)

// wide sub
#define dct_wsub(out, a, b) \
   int32x4_t out##_l = vsubq_s32(a##_l, b##_l); \
   int32x4_t out##_h = vsubq_s32(a##_h, b##_h)

// butterfly a/b, then shift using "shiftop" by "s" and pack
#define dct_bfly32o(out0,out1, a,b,shiftop,s) \
   { \
      dct_wadd(sum, a, b); \
      dct_wsub(dif, a, b); \
      out0 = vcombine_s16(shiftop(sum_l, s), shiftop(sum_h, s)); \
      out1 = vcombine_s16(shiftop(dif_l, s), shiftop(dif_h, s)); \
   }

#define dct_pass(shiftop, shift) \
   { \
      /* even part */ \
      int16x8_t sum26 = vaddq_s16(row2, row6); \
      dct_long_mul(p1e, sum26, rot0_0); \
      dct_long_mac(t2e, p1e, row6, rot0_1); \
      dct_long_mac(t3e, p1e, row2, rot0_2); \
      int16x8_t sum04 = vaddq_s16(row0, row4); \
      int16x8_t dif04 = vsubq_s16(row0, row4); \
      dct_widen(t0e, sum04); \
      dct_widen(t1e, dif04); \
      dct_wadd(x0, t0e, t3e); \
      dct_wsub(x3, t0e, t3e); \
      dct_wadd(x1, t1e, t2e); \
      dct_wsub(x2, t1e, t2e); \
      /* odd part */ \
      int16x8_t sum15 = vaddq_s16(row1, row5); \
      int16x8_t sum17 = vaddq_s16(row1, row7); \
      int16x8_t sum35 = vaddq_s16(row3, row5); \
      int16x8_t sum37 = vaddq_s16(row3, row7); \
      int16x8_t sumodd = vaddq_s16(sum17, sum35); \
      dct_long_mul(p5o, sumodd, rot1_0); \
      dct_long_mac(p1o, p5o, sum17, rot1_1); \
      dct_long_mac(p2o, p5o, sum35, rot1_2); \
      dct_long_mul(p3o, sum37, rot2_0); \
      dct_long_mul(p4o, sum15, rot2_1); \
      dct_wadd(sump13o, p1o, p3o); \
      dct_wadd(sump24o, p2o, p4o); \
      dct_wadd(sump23o, p2o, p3o); \
      dct_wadd(sump14o, p1o, p4o); \
      dct_long_mac(x4, sump13o, row7, rot3_0); \
      dct_long_mac(x5, sump24o, row5, rot3_1); \
      dct_long_mac(x6, sump23o, row3, rot3_2); \
      dct_long_mac(x7, sump14o, row1, rot3_3); \
      dct_bfly32o(row0,row7, x0,x7,shiftop,shift); \
      dct_bfly32o(row1,row6, x1,x6,shiftop,shift); \
      dct_bfly32o(row2,row5, x2,x5,shiftop,shift); \
      dct_bfly32o(row3,row4, x3,x4,shiftop,shift); \
   }

   // load
	row0 = vld1q_s16(data + 0 * 8);
	row1 = vld1q_s16(data + 1 * 8);
	row2 = vld1q_s16(data + 2 * 8);
	row3 = vld1q_s16(data + 3 * 8);
	row4 = vld1q_s16(data + 4 * 8);
	row5 = vld1q_s16(data + 5 * 8);
	row6 = vld1q_s16(data + 6 * 8);
	row7 = vld1q_s16(data + 7 * 8);

	// add DC bias
	row0 = vaddq_s16(row0, vsetq_lane_s16(1024, vdupq_n_s16(0), 0));

	// column pass
	dct_pass(vrshrn_n_s32, 10);

	// 16bit 8x8 transpose
	{
		// these three map to a single VTRN.16, VTRN.32, and VSWP, respectively.
		// whether compilers actually get this is another story, sadly.
#define dct_trn16(x, y) { int16x8x2_t t = vtrnq_s16(x, y); x = t.val[0]; y = t.val[1]; }
#define dct_trn32(x, y) { int32x4x2_t t = vtrnq_s32(vreinterpretq_s32_s16(x), vreinterpretq_s32_s16(y)); x = vreinterpretq_s16_s32(t.val[0]); y = vreinterpretq_s16_s32(t.val[1]); }
#define dct_trn64(x, y) { int16x8_t x0 = x; int16x8_t y0 = y; x = vcombine_s16(vget_low_s16(x0), vget_low_s16(y0)); y = vcombine_s16(vget_high_s16(x0), vget_high_s16(y0)); }

	  // pass 1
		dct_trn16(row0, row1); // a0b0a2b2a4b4a6b6
		dct_trn16(row2, row3);
		dct_trn16(row4, row5);
		dct_trn16(row6, row7);

		// pass 2
		dct_trn32(row0, row2); // a0b0c0d0a4b4c4d4
		dct_trn32(row1, row3);
		dct_trn32(row4, row6);
		dct_trn32(row5, row7);

		// pass 3
		dct_trn64(row0, row4); // a0b0c0d0e0f0g0h0
		dct_trn64(row1, row5);
		dct_trn64(row2, row6);
		dct_trn64(row3, row7);

#undef dct_trn16
#undef dct_trn32
#undef dct_trn64
	}

	// row pass
	// vrshrn_n_s32 only supports shifts up to 16, we need
	// 17. so do a non-rounding shift of 16 first then follow
	// up with a rounding shift by 1.
	dct_pass(vshrn_n_s32, 16);

	{
		// pack and round
		uint8x8_t p0 = vqrshrun_n_s16(row0, 1);
		uint8x8_t p1 = vqrshrun_n_s16(row1, 1);
		uint8x8_t p2 = vqrshrun_n_s16(row2, 1);
		uint8x8_t p3 = vqrshrun_n_s16(row3, 1);
		uint8x8_t p4 = vqrshrun_n_s16(row4, 1);
		uint8x8_t p5 = vqrshrun_n_s16(row5, 1);
		uint8x8_t p6 = vqrshrun_n_s16(row6, 1);
		uint8x8_t p7 = vqrshrun_n_s16(row7, 1);

		// again, these can translate into one instruction, but often don't.
#define dct_trn8_8(x, y) { uint8x8x2_t t = vtrn_u8(x, y); x = t.val[0]; y = t.val[1]; }
#define dct_trn8_16(x, y) { uint16x4x2_t t = vtrn_u16(vreinterpret_u16_u8(x), vreinterpret_u16_u8(y)); x = vreinterpret_u8_u16(t.val[0]); y = vreinterpret_u8_u16(t.val[1]); }
#define dct_trn8_32(x, y) { uint32x2x2_t t = vtrn_u32(vreinterpret_u32_u8(x), vreinterpret_u32_u8(y)); x = vreinterpret_u8_u32(t.val[0]); y = vreinterpret_u8_u32(t.val[1]); }

	  // sadly can't use interleaved stores here since we only write
	  // 8 bytes to each scan line!

	  // 8x8 8-bit transpose pass 1
		dct_trn8_8(p0, p1);
		dct_trn8_8(p2, p3);
		dct_trn8_8(p4, p5);
		dct_trn8_8(p6, p7);

		// pass 2
		dct_trn8_16(p0, p2);
		dct_trn8_16(p1, p3);
		dct_trn8_16(p4, p6);
		dct_trn8_16(p5, p7);

		// pass 3
		dct_trn8_32(p0, p4);
		dct_trn8_32(p1, p5);
		dct_trn8_32(p2, p6);
		dct_trn8_32(p3, p7);

		// store
		vst1_u8(out, p0); out += out_stride;
		vst1_u8(out, p1); out += out_stride;
		vst1_u8(out, p2); out += out_stride;
		vst1_u8(out, p3); out += out_stride;
		vst1_u8(out, p4); out += out_stride;
		vst1_u8(out, p5); out += out_stride;
		vst1_u8(out, p6); out += out_stride;
		vst1_u8(out, p7);

#undef dct_trn8_8
#undef dct_trn8_16
#undef dct_trn8_32
	}

#undef dct_long_mul
#undef dct_long_mac
#undef dct_widen
#undef dct_wadd
#undef dct_wsub
#undef dct_bfly32o
#undef dct_pass
}

#endif // STBI_NEON

#define STBI__MARKER_none  0xff
// if there's a pending marker from the entropy stream, return that
// otherwise, fetch from the stream and get a marker. if there's no
// marker, return 0xff, which is never a valid marker value
static stbi_uc stbi__get_marker(stbi__jpeg *j)
{
	stbi_uc x;
	if (j->marker != STBI__MARKER_none) { x = j->marker; j->marker = STBI__MARKER_none; return x; }
	x = stbi__get8(j->s);
	if (x != 0xff) return STBI__MARKER_none;
	while (x == 0xff)
		x = stbi__get8(j->s); // consume repeated 0xff fill bytes
	return x;
}

// in each scan, we'll have scan_n components, and the order
// of the components is specified by order[]
#define STBI__RESTART(x)     ((x) >= 0xd0 && (x) <= 0xd7)

// after a restart interval, stbi__jpeg_reset the entropy decoder and
// the dc prediction
static void stbi__jpeg_reset(stbi__jpeg *j)
{
	j->code_bits = 0;
	j->code_buffer = 0;
	j->nomore = 0;
	j->img_comp[0].dc_pred = j->img_comp[1].dc_pred = j->img_comp[2].dc_pred = j->img_comp[3].dc_pred = 0;
	j->marker = STBI__MARKER_none;
	j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
	j->eob_run = 0;
	// no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
	// since we don't even allow 1<<30 pixels
}

static int stbi__parse_entropy_coded_data(stbi__jpeg *z)
{
	stbi__jpeg_reset(z);
	if (!z->progressive) {
		if (z->scan_n == 1) {
			int i, j;
			STBI_SIMD_ALIGN(short, data[64]);
			int n = z->order[0];
			// non-interleaved data, we just need to process one block at a time,
			// in trivial scanline order
			// number of blocks to do just depends on how many actual "pixels" this
			// component has, independent of interleaved MCU blocking and such
			int w = (z->img_comp[n].x + 7) >> 3;
			int h = (z->img_comp[n].y + 7) >> 3;
			for (j = 0; j < h; ++j) {
				for (i = 0; i < w; ++i) {
					int ha = z->img_comp[n].ha;
					if (!stbi__jpeg_decode_block(z, data, z->huff_dc + z->img_comp[n].hd, z->huff_ac + ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq])) return 0;
					z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2*j * 8 + i * 8, z->img_comp[n].w2, data);
					// every data block is an MCU, so countdown the restart interval
					if (--z->todo <= 0) {
						if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
						// if it's NOT a restart, then just bail, so we get corrupt data
						// rather than no data
						if (!STBI__RESTART(z->marker)) return 1;
						stbi__jpeg_reset(z);
					}
				}
			}
			return 1;
		}
		else { // interleaved
			int i, j, k, x, y;
			STBI_SIMD_ALIGN(short, data[64]);
			for (j = 0; j < z->img_mcu_y; ++j) {
				for (i = 0; i < z->img_mcu_x; ++i) {
					// scan an interleaved mcu... process scan_n components in order
					for (k = 0; k < z->scan_n; ++k) {
						int n = z->order[k];
						// scan out an mcu's worth of this component; that's just determined
						// by the basic H and V specified for the component
						for (y = 0; y < z->img_comp[n].v; ++y) {
							for (x = 0; x < z->img_comp[n].h; ++x) {
								int x2 = (i*z->img_comp[n].h + x) * 8;
								int y2 = (j*z->img_comp[n].v + y) * 8;
								int ha = z->img_comp[n].ha;
								if (!stbi__jpeg_decode_block(z, data, z->huff_dc + z->img_comp[n].hd, z->huff_ac + ha, z->fast_ac[ha], n, z->dequant[z->img_comp[n].tq])) return 0;
								z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2*y2 + x2, z->img_comp[n].w2, data);
							}
						}
					}
					// after all interleaved components, that's an interleaved MCU,
					// so now count down the restart interval
					if (--z->todo <= 0) {
						if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
						if (!STBI__RESTART(z->marker)) return 1;
						stbi__jpeg_reset(z);
					}
				}
			}
			return 1;
		}
	}
	else {
		if (z->scan_n == 1) {
			int i, j;
			int n = z->order[0];
			// non-interleaved data, we just need to process one block at a time,
			// in trivial scanline order
			// number of blocks to do just depends on how many actual "pixels" this
			// component has, independent of interleaved MCU blocking and such
			int w = (z->img_comp[n].x + 7) >> 3;
			int h = (z->img_comp[n].y + 7) >> 3;
			for (j = 0; j < h; ++j) {
				for (i = 0; i < w; ++i) {
					short *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
					if (z->spec_start == 0) {
						if (!stbi__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
							return 0;
					}
					else {
						int ha = z->img_comp[n].ha;
						if (!stbi__jpeg_decode_block_prog_ac(z, data, &z->huff_ac[ha], z->fast_ac[ha]))
							return 0;
					}
					// every data block is an MCU, so countdown the restart interval
					if (--z->todo <= 0) {
						if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
						if (!STBI__RESTART(z->marker)) return 1;
						stbi__jpeg_reset(z);
					}
				}
			}
			return 1;
		}
		else { // interleaved
			int i, j, k, x, y;
			for (j = 0; j < z->img_mcu_y; ++j) {
				for (i = 0; i < z->img_mcu_x; ++i) {
					// scan an interleaved mcu... process scan_n components in order
					for (k = 0; k < z->scan_n; ++k) {
						int n = z->order[k];
						// scan out an mcu's worth of this component; that's just determined
						// by the basic H and V specified for the component
						for (y = 0; y < z->img_comp[n].v; ++y) {
							for (x = 0; x < z->img_comp[n].h; ++x) {
								int x2 = (i*z->img_comp[n].h + x);
								int y2 = (j*z->img_comp[n].v + y);
								short *data = z->img_comp[n].coeff + 64 * (x2 + y2 * z->img_comp[n].coeff_w);
								if (!stbi__jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd], n))
									return 0;
							}
						}
					}
					// after all interleaved components, that's an interleaved MCU,
					// so now count down the restart interval
					if (--z->todo <= 0) {
						if (z->code_bits < 24) stbi__grow_buffer_unsafe(z);
						if (!STBI__RESTART(z->marker)) return 1;
						stbi__jpeg_reset(z);
					}
				}
			}
			return 1;
		}
	}
}

static void stbi__jpeg_dequantize(short *data, stbi__uint16 *dequant)
{
	int i;
	for (i = 0; i < 64; ++i)
		data[i] *= dequant[i];
}

static void stbi__jpeg_finish(stbi__jpeg *z)
{
	if (z->progressive) {
		// dequantize and idct the data
		int i, j, n;
		for (n = 0; n < z->s->img_n; ++n) {
			int w = (z->img_comp[n].x + 7) >> 3;
			int h = (z->img_comp[n].y + 7) >> 3;
			for (j = 0; j < h; ++j) {
				for (i = 0; i < w; ++i) {
					short *data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
					stbi__jpeg_dequantize(data, z->dequant[z->img_comp[n].tq]);
					z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2*j * 8 + i * 8, z->img_comp[n].w2, data);
				}
			}
		}
	}
}

static int stbi__process_marker(stbi__jpeg *z, int m)
{
	int L;
	switch (m) {
	case STBI__MARKER_none: // no marker found
		return stbi__err("expected marker", "Corrupt JPEG");

	case 0xDD: // DRI - specify restart interval
		if (stbi__get16be(z->s) != 4) return stbi__err("bad DRI len", "Corrupt JPEG");
		z->restart_interval = stbi__get16be(z->s);
		return 1;

	case 0xDB: // DQT - define quantization table
		L = stbi__get16be(z->s) - 2;
		while (L > 0) {
			int q = stbi__get8(z->s);
			int p = q >> 4, sixteen = (p != 0);
			int t = q & 15, i;
			if (p != 0 && p != 1) return stbi__err("bad DQT type", "Corrupt JPEG");
			if (t > 3) return stbi__err("bad DQT table", "Corrupt JPEG");

			for (i = 0; i < 64; ++i)
				z->dequant[t][stbi__jpeg_dezigzag[i]] = (stbi__uint16)(sixteen ? stbi__get16be(z->s) : stbi__get8(z->s));
			L -= (sixteen ? 129 : 65);
		}
		return L == 0;

	case 0xC4: // DHT - define huffman table
		L = stbi__get16be(z->s) - 2;
		while (L > 0) {
			stbi_uc *v;
			int sizes[16], i, n = 0;
			int q = stbi__get8(z->s);
			int tc = q >> 4;
			int th = q & 15;
			if (tc > 1 || th > 3) return stbi__err("bad DHT header", "Corrupt JPEG");
			for (i = 0; i < 16; ++i) {
				sizes[i] = stbi__get8(z->s);
				n += sizes[i];
			}
			L -= 17;
			if (tc == 0) {
				if (!stbi__build_huffman(z->huff_dc + th, sizes)) return 0;
				v = z->huff_dc[th].values;
			}
			else {
				if (!stbi__build_huffman(z->huff_ac + th, sizes)) return 0;
				v = z->huff_ac[th].values;
			}
			for (i = 0; i < n; ++i)
				v[i] = stbi__get8(z->s);
			if (tc != 0)
				stbi__build_fast_ac(z->fast_ac[th], z->huff_ac + th);
			L -= n;
		}
		return L == 0;
	}

	// check for comment block or APP blocks
	if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
		L = stbi__get16be(z->s);
		if (L < 2) {
			if (m == 0xFE)
				return stbi__err("bad COM len", "Corrupt JPEG");
			else
				return stbi__err("bad APP len", "Corrupt JPEG");
		}
		L -= 2;

		if (m == 0xE0 && L >= 5) { // JFIF APP0 segment
			static const unsigned char tag[5] = { 'J','F','I','F','\0' };
			int ok = 1;
			int i;
			for (i = 0; i < 5; ++i)
				if (stbi__get8(z->s) != tag[i])
					ok = 0;
			L -= 5;
			if (ok)
				z->jfif = 1;
		}
		else if (m == 0xEE && L >= 12) { // Adobe APP14 segment
			static const unsigned char tag[6] = { 'A','d','o','b','e','\0' };
			int ok = 1;
			int i;
			for (i = 0; i < 6; ++i)
				if (stbi__get8(z->s) != tag[i])
					ok = 0;
			L -= 6;
			if (ok) {
				stbi__get8(z->s); // version
				stbi__get16be(z->s); // flags0
				stbi__get16be(z->s); // flags1
				z->app14_color_transform = stbi__get8(z->s); // color transform
				L -= 6;
			}
		}

		stbi__skip(z->s, L);
		return 1;
	}

	return stbi__err("unknown marker", "Corrupt JPEG");
}

// after we see SOS
static int stbi__process_scan_header(stbi__jpeg *z)
{
	int i;
	int Ls = stbi__get16be(z->s);
	z->scan_n = stbi__get8(z->s);
	if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int)z->s->img_n) return stbi__err("bad SOS component count", "Corrupt JPEG");
	if (Ls != 6 + 2 * z->scan_n) return stbi__err("bad SOS 