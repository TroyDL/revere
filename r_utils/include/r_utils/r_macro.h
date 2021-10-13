
#ifndef r_utils_r_macro_h_
#define r_utils_r_macro_h_

#ifdef IS_WINDOWS
#define _WINSOCKAPI_
#include <windows.h>
#endif

#define R_MACRO_BEGIN do {
#define R_MACRO_END }while(0)

#define R_MACRO_END_LOOP_FOREVER }while(1)

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#ifdef IS_WINDOWS
#define FULL_MEM_BARRIER MemoryBarrier
#else
#ifdef IS_LINUX
#define FULL_MEM_BARRIER __sync_synchronize
#endif
#endif

#endif
