
#ifndef r_utils_r_macro_h_
#define r_utils_r_macro_h_

#define R_MACRO_BEGIN do {
#define R_MACRO_END }while(0)

#define R_MACRO_END_LOOP_FOREVER }while(1)

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#define FULL_MEM_BARRIER __sync_synchronize

#endif
