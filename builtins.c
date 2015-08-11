#include <glob.h>

#include <gc.h>
#include <vector.h>
#include <nicestat.h>

#include "type.h"
#include "value.h"
#include "sym.h"

static value* builtinSize (const value* file) {
    const char* filename = valueGetFilename(file);

    if (!filename)
        return valueCreateInvalid();

    stat_t st;
    bool fail = nicestat(filename, &st);

    if (fail)
        return valueCreateInvalid();

    return valueCreateInt(st.size);
}

static value* builtinSum (const value* numbers) {
    int64_t total = 0;

    //todo adapt for Number, when it exists

    for_vector (value* number, valueGetVector(numbers), {
        total += valueGetInt(number);
    })

    return valueCreateInt(total);
}

static value* builtinZipf (const value* fn, const value* arg) {
    const value *first = valueCall(fn, arg),
                *second = arg;

    vector(const value*) vec = vectorInit(2, malloc);
    vectorPushFromArray(&vec, (void**) (const value*[]) {first, second}, 2, sizeof(value*));
    return valueCreateVector(vec);
}

static value* builtinZipfCurried (const value* fn) {
	/*Store the first parameter until we can do any computation*/
    return valueCreateSimpleClosure(fn, (simpleClosureFn) builtinZipf);
}

value* builtinExpandGlob (const char* pattern, value* arg) {
    (void) arg;

    vector(char*) results = {};

    glob_t matches = {};
    int error = glob(pattern, 0, 0, &matches);

    if (!error) {
        results = vectorInit(matches.gl_pathc, GC_malloc);

        /*Turn the matches into a vector*/
        for (unsigned int n = 0; n < matches.gl_pathc; n++)
            vectorPush(&results, valueCreateStr(matches.gl_pathv[n]));
    }

    globfree(&matches);

    return valueCreateVector(results);
}

static void addBuiltin (sym* global, const char* name, type* dt, value* val) {
    sym* symbol = symAdd(global, name);
    symbol->dt = dt;
    symbol->val = val;
}

void addBuiltins (typeSys* ts, sym* global) {
    addBuiltin(global, "size",
               typeFnChain(2, ts, type_File, type_Integer),
               valueCreateFn(builtinSize));

    addBuiltin(global, "sum",
               typeFn(ts, typeList(ts, typeInteger(ts)), typeInteger(ts)),
               valueCreateFn(builtinSum));

    addBuiltin(global, "zipf",
               /*(File -> Integer) -> File -> (Integer, File)*/
               typeFn(ts, typeFnChain(2, ts, type_File, type_Integer),
                          typeFn(ts, typeFile(ts),
                                     typeTupleChain(2, ts, typeInteger(ts),
                                                           typeFile(ts)))),
               valueCreateFn(builtinZipfCurried));
}
