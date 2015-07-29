#include "ast.h"

#include <assert.h>
#include <stdlib.h>

#include "common.h"

static ast* astCreate (astKind kind, ast init) {
    assert(kind != astKindNo);

    ast* node = malloci(sizeof(*node), &init);
    node->kind = kind;
    return node;
}

void astDestroy (ast* node) {
    if (node->l)
        astDestroy(node->l);

    if (node->r)
        astDestroy(node->r);

    vectorFreeObjs(&node->children, (vectorDtor) astDestroy);

    if (node->kind == astStrLit || node->kind == astFileLit)
        free(node->literal.str);

    free(node);
}

ast* astCreateBOP (ast* l, ast* r, opKind op) {
    return astCreate(astBOP, (ast) {
        .l = l,
        .r = r,
        .op = op
    });
}

ast* astCreateFnApp (vector(ast*) args, ast* fn) {
    return astCreate(astFnApp, (ast) {
        .r = fn,
        .children = args
    });
}

ast* astCreateSymbol (sym* symbol) {
    return astCreate(astSymbol, (ast) {
        .symbol = symbol,
    });
}

ast* astCreateStrLit (const char* str) {
    return astCreate(astStrLit, (ast) {
        .literal.str = strdup(str),
    });
}

ast* astCreateFileLit (const char* str) {
    return astCreate(astFileLit, (ast) {
        .literal.str = strdup(str),
    });
}

ast* astCreateListLit (vector(ast*) elements) {
    return astCreate(astListLit, (ast) {
        .children = elements,
    });
}

ast* astCreateInvalid (void) {
    return astCreate(astInvalid, (ast) {});
}

/*==== ====*/

const char* astKindGetStr (astKind kind) {
    switch (kind) {
    case astBOP: return "BOP";
    case astFnApp: return "FnApp";
    case astSymbol: return "Symbol";
    case astStrLit: return "StrLit";
    case astFileLit: return "FileLit";
    case astListLit: return "ListLit";
    case astInvalid: return "Invalid";
    case astKindNo: return "<KindNo; not real>";
    default: return "<unhandled AST kind>";
    }
}
