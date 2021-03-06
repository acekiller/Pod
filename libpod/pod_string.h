#ifndef INCLUDE_POD_STRING_H
#define INCLUDE_POD_STRING_H

/******************************************************************** 
 *  pod_string.h
 *  Copyright (c) 2011-2012, James A. Banks
 *  All rights reserved.
 *  See file LICENSE for details.
 ********************************************************************/

#include <stddef.h>
#include "pod_object.h"
#include "pod_char.h"



// Strings are basically what they traditionally are: arrays of characters.  In
// this case, pod_char_t.  However, these strings are not null terminated.
// They have a maximum size (size) and a length in use (used).  In this sense,
// they are more like Pascal strings than C strings.

// DONE (I'm thinking about this...) Keys must be strings, so
// I stuck a value member in every string because I don't see the point of
// another struct (that would be the key-value pair).

// TODO The size element of the string is of type size_t, but the size of a
// string is really in units of sizeof(pod_char_t).  On OS X x86, size_t is 32
// bits.  (probably, I haven't verified this) intended for addresses at a byte
// level.  Is size_t good to use?

// TODO Other functions that might be useful: pod_string_append,
// pod_string_dup, pod_string_copy_to_wstring, pod_string_copy_from_wstring

// DONE pod_copy_string and friends should be pod_string_copy and such.  In
// this way namespace conflicts/confusions can be minimized.  Or the function
// can be more easily be associated with pod_string.

// DONE If malloc fails, it always returns NULL with errno set to ENOMEM, so
// pod_string_create doesn't need the error parameter.  Get rid of it. 


    // Type value is 0x61, POD_OBJECT_TYPE + 1

#define POD_STRING_TYPE 0x61


    // Flags are:
    // POD_INIT_ZERO - Initialize the buffer to zero during construction
    // POD_DESTROY_ZERO - Overwrite buffer with zeros before freeing memory
    // POD_COPY_ZERO - Write zeros to remaining space after buffer has been
    //  copied to.

enum pod_string_flags {
    POD_INIT_ZERO = 0x01,
    POD_DESTROY_ZERO = 0x02,
    POD_COPY_ZERO = 0x04
};


    // Allocation size for a pod_string is sizeof(struct pod_string) +
    // (n * sizeof(pod_char_t)).

struct pod_string;
typedef struct pod_string pod_string;

struct pod_string {
    pod_object o;
    size_t size;
    size_t used;
    int flags;
//    struct pod_object *value;   // only used if this is a key in a KV pair
    pod_char_t buffer[];
};


    // Constructor and destructor

extern pod_string *pod_string_create(size_t size, int flags);
extern void pod_string_destroy(void *string);


    // Other pod_string-related functions

extern void pod_string_append_char(pod_string *string, pod_char_t c);
extern void pod_string_clear(pod_string *string);
extern int pod_string_compare(pod_string *a, pod_string *b);
extern int pod_string_compare_to_cstring(pod_string *ps, char *cs);
extern void pod_string_copy(pod_string *to, pod_string *from);
extern void pod_string_copy_from_cstring(pod_string *to, char *from);
extern void pod_string_copy_to_cstring(char *to, pod_string *from);
extern pod_string *pod_string_dup(pod_string *string);
extern pod_string *pod_string_dup_text(pod_string *string);

static inline int pod_string_is_empty(pod_string *string)
{
    return (string->used == 0);
}

#endif /* INCLUDE_POD_STRING_H */
