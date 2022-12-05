/*
 * This contains a lot of definitions that are used if this is compiled with a
 * compiler missing some default definitions (MSVC).
 */

#ifndef DEFS_H
#define DEFS_H

#include<common.h>

#ifndef __UINT8_MAX__
#define __UINT8_MAX__ ((u8)-1)
#endif

#ifndef __UINT16_MAX__
#define __UINT16_MAX__ ((u16)-1)
#endif

#ifndef __UINT32_MAX__
#define __UINT32_MAX__ ((u32)-1)
#endif

#ifndef __UINT64_MAX__
#define __UINT64_MAX__ ((u64)-1)
#endif

#ifndef __UINTPTR_MAX__
#define __UINTPTR_MAX__ ((void*)-1)
#endif

#ifndef __INT8_MAX__
#define __INT8_MAX__ ((i8)(__UINT8_MAX__ >> 1))
#endif

#ifndef __INT16_MAX__
#define __INT16_MAX__ ((i16)(__UINT16_MAX__ >> 1))
#endif

#ifndef __INT32_MAX__
#define __INT32_MAX__ ((i32)(__UINT32_MAX__ >> 1))
#endif

#ifndef __INT64_MAX__
#define __INT64_MAX__ ((i64)(__UINT64_MAX__ >> 1))
#endif

#endif