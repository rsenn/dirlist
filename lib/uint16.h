/* this header file comes from libowfat, http://www.fefe.de / libowfat/ */
#ifndef UINT16_H
#define UINT16_H

#if !defined(_MSC_VER) && !defined(__MSYS__)
#include <inttypes.h>
#endif /* !defined(_MSC_VER) */
#if !defined(_MSC_VER) && !defined(__MSYS__)
#include <stdint.h>
#endif

#ifdef __MSYS__
# ifndef __MS_types__
#  define __MS_types__
# endif
# include <sys/types.h>
# ifdef __BIT_TYPES_DEFINED__
#  define uint16_t u_int16_t
#  define int16_t short
# endif
#endif

#ifdef _MSC_VER
#include <windows.h>
#define uint16_t UINT16
#define int16_t INT16
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t uint16;
typedef int16_t int16;

#if(defined(__i386__) || defined(__x86_64__)) && !defined(NO_UINT16_MACROS)

static void uint16_pack(char* out, uint16 in) {
  *(uint16 *)out = in;
}

static void uint16_unpack(const char* in, uint16* out) {
  *out = *(uint16 *)in;
}

static uint16 uint16_get(const void* ptr) {
  const char* in = ptr;
  return *(uint16 *)in;
}

static uint16 uint16_read(const char* in) {
  return *(uint16 *)in;
}

void uint16_pack_big(char* out, uint16 in);
void uint16_unpack_big(const char* in, uint16* out);
uint16 uint16_read_big(const char* in);
#else

static uint16
uint16_get(const void* ptr) {
  const char* in = ptr;
  return (in[0] << 8) | (in[1]);
}

static uint16
uint16_read(const char* in) {
  return (in[0] << 8) | (in[1]);
}

void uint16_pack(char* out, uint16 in);
void uint16_pack_big(char* out, uint16 in);
void uint16_unpack(const char* in, uint16* out);
void uint16_unpack_big(const char* in, uint16* out);
uint16 uint16_read(const char* in);
uint16 uint16_read_big(const char* in);

#endif

#ifdef __cplusplus
}
#endif

#endif
