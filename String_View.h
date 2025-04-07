//
// String_View.h - better strings
//
// Fletcher M - 08/02/2025
//


#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include "ints.h"

typedef struct SV {
    u64 size;
    char *data;
} SV;

#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).size, (sv).data 
// Example: printf("my_sv = "SV_Fmt"\n", SV_Arg(my_sv));

typedef struct SV_Array {
    SV *items;
    u64 count;
    u64 capacity;
} SV_Array;


// functions on String views

// takes a C_Str return a SV, dose not allocate
SV SV_From_C_Str(const char *str);

// SV equality check
bool32 SV_Eq(SV s1, SV s2);


// TODO more functions

#endif // STRING_VIEW_H_

#ifdef STRING_VIEW_IMPLEMENTATION

#ifndef STRING_VIEW_IMPLEMENTATION_GUARD_
#define STRING_VIEW_IMPLEMENTATION_GUARD_


// TODO remove? make own functions?
#include "string.h"


SV SV_From_C_Str(const char *str) {
    SV result = {
        .data = (char *) str,
        .size = strlen(str),
    };
    return result;
}


bool32 SV_Eq(SV s1, SV s2) {
    if (s1.size != s2.size) return False;
    for (u64 n = 0; n < s1.size; n++) {
        if (s1.data[n] != s2.data[n]) return False;
    }
    return True;
}


#endif // STRING_VIEW_IMPLEMENTATION_GUARD_
#endif // STRING_VIEW_IMPLEMENTATION
