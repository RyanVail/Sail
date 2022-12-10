/* This contains some commonly used assembly structures and functions. */

#ifndef BACKEND_ASM_BIN_H
#define BACKEND_ASM_BIN_H

#include<common.h>
#include<backend/intermediate/intermediate.h>

/* struct reg - This holds the status of a single register
 * @size: The size of this register in bytes, only present if the target cpu has
 * registers with different sizes
 * @content: The intermediate currently inside of this register
 */
typedef struct reg {
    #if REGISTERS_SIZE_NOT_CONST
    u8 size;
    #endif
    intermediate content;
} reg;

/* struct label - This is a label into the "contents" of a "bin"
 * @location: The location of this label in the "contents"
 * @name: The name of this label
 * @local: If this is a local label
 */
typedef struct label {
    u32 location;
    char* name;
    bool local;
} label;

/* struct bin - This is a simple binary format
 * @contents: This is the machine code
 * @labels: This is a vector of "label"
 */
typedef struct bin {
    vector contents;
    vector labels;
} bin;

#endif