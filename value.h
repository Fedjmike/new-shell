#pragma once

#include <vector.h>

#include "common.h"
#include "forward.h"

/*Opaque, use interface below*/
typedef struct value value;

typedef struct valueIter {
    const value* iterable;

    /*Information depends on the iterable implementations kind*/
    union {
        /*Vector*/
        int n;
    };
} valueIter;

/*The value creators allocate objects with a garbage collector!

  These objects are only kept alive by references stored in:
    - The stack and data segments (variables and parameters)
    - Other GC allocated objects

  Therefore if you want a value to be owned by a manually managed object,
  allocate the owner with GC_MALLOC_UNCOLLECTABLE and free with GC_FREE.
  This is just a normal allocation that gets scanned for GC object
  references.*/

typedef value* (*simpleClosureFn)(const void* env, const value* arg);

value* valueCreateUnit (void);
value* valueCreateInt (int integer);
value* valueCreateStr (char* str);
value* valueCreateFn (value* (*fnptr)(const value*));
/*Takes ownership of the environment, which must be GC allocated*/
value* valueCreateSimpleClosure (const void* env, simpleClosureFn fnptr);
value* valueCreateFile (const char* filename);
/*Takes ownership of the vector and its elements. Therefore, they must
  have been allocated using the garbage collector.*/
value* valueCreateVector (vector(value*) elements);
value* valueCreateInvalid (void);

/*==== (Kind generic) Operations ====*/

bool valueIsInvalid (const value* v);

/*Both of these return the width of the string representation of a value.
  valuePrint actually prints it.*/
int valueGetWidthOfStr (const value* v);
int valuePrint (const value* v);

int64_t valueGetInt (const value* num);

value* valueCall (const value* fn, const value* arg);

const char* valueGetFilename (const value* file);

bool valueGetIterator (const value* iterable, valueIter* iter_out);
int valueGuessIterLength (valueIter iterator);
value* valueIterRead (valueIter* iterator);

/*Convert an iterable to a vector*/
vector(const value*) valueGetVector (const value* iterable);
