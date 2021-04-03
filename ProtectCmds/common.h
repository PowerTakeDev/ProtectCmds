#ifndef COMMON_H
#define COMMON_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define Bits2Bytes(b) ((b + 7) >> 3)

#endif