#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <gc/gc.h>

#include "common.h"
#include "type.h"
#include "sym.h"
#include "ast.h"

#include "builtins.h"

#include "lexer.h"
#include "parser.h"
#include "analyzer.h"

#include "ast-printer.h"

#include "value.h"
#include "runner.h"

/*==== Compiler ====*/

typedef struct compilerCtx {
    typeSys ts;
    sym* global;
} compilerCtx;

ast* compile (compilerCtx* ctx, const char* str, int* errors) {
    /*Turn the string into an AST*/
    ast* tree; {
        lexerCtx lexer = lexerInit(str);
        parserResult result = parse(ctx->global, &lexer);
        lexerDestroy(&lexer);

        tree = result.tree;
        *errors += result.errors;
    }

    /*Add types and other semantic information*/
    {
        analyzerResult result = analyze(&ctx->ts, tree);
        *errors += result.errors;
    }

    printAST(tree);

    return tree;
}

compilerCtx compilerInit (void) {
    return (compilerCtx) {
        .ts = typesInit(),
        .global = symInit()
    };
}

compilerCtx* compilerFree (compilerCtx* ctx) {
    symEnd(ctx->global);
    typesFree(&ctx->ts);
    return ctx;
}

/*==== Gosh ====*/

void gosh (compilerCtx* ctx, const char* str) {
    int errors = 0;
    ast* tree = compile(ctx, str, &errors);

    if (errors == 0) {
        /*Run the AST*/
        envCtx env = {};
        value* result = run(&env, tree);

        /*Print the value and type*/
        valuePrint(result);
        printf(" :: %s\n", typeGetStr(tree->dt));
    }

    astDestroy(tree);
}

/*==== REPL ====*/

void repl (compilerCtx* compiler) {
    while (true) {
        char* input = readline(" $ ");
        add_history(input);

        gosh(compiler, input);
    }
}

/*==== ====*/

const char* const samples[] = {
    "sh.c size",
    "[sh.c, parser.c]",
    "[sh.c, parser.c] | size"
};

int main (int argc, char** argv) {
    GC_INIT();

    compilerCtx compiler = compilerInit();
    addBuiltins(&compiler.ts, compiler.global);

    if (argc == 1) {
        puts(samples[2]);
        gosh(&compiler, samples[2]);

    } else if (argc == 2) {
        if (!strcmp(argv[1], "-i"))
            repl(&compiler);

        else
            gosh(&compiler, argv[1]);

    } else
        printf("Unknown arguments.\n");

    compilerFree(&compiler);
}
