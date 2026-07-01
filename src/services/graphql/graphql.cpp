// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file graphql.cpp
 * @brief GraphQL query subset - parser + executor (implementation).
 *
 * Recursive-descent parse into fixed node/arg pools, then a recursive emit that
 * mirrors the selection set into a JSON `data` object, calling the resolver for
 * leaf fields with the arguments collected along the path. No heap; all state is
 * file-static (single-accessor, like the other services).
 */

#include "services/graphql/graphql.h"

#if DETWS_ENABLE_GRAPHQL

#include <stdio.h>
#include <string.h>

struct DetwsGqlArgs
{
    const int *idx; // indices into g_args that are in scope
    int count;
};

namespace
{
struct Node
{
    char name[DETWS_GQL_NAME_MAX];
    int first_arg, n_args;
    int first_child; // -1 if leaf
    int next_sib;    // -1 if last
};
struct Arg
{
    char name[DETWS_GQL_NAME_MAX];
    DetwsGqlValue val;
};

Node g_nodes[DETWS_GQL_MAX_NODES];
Arg g_args[DETWS_GQL_MAX_ARGS];
char g_strbuf[DETWS_GQL_STRBUF];
int g_nnodes, g_nargs, g_strlen, g_root, g_err;

int new_node()
{
    if (g_nnodes >= DETWS_GQL_MAX_NODES)
    {
        g_err = DETWS_GQL_ERR_LIMIT;
        return -1;
    }
    Node *n = &g_nodes[g_nnodes];
    n->name[0] = '\0';
    n->first_arg = -1;
    n->n_args = 0;
    n->first_child = -1;
    n->next_sib = -1;
    return g_nnodes++;
}

// ---- lexer helpers --------------------------------------------------------
struct Lex
{
    const char *p, *e;
};

void skipws(Lex &L)
{
    while (L.p < L.e)
    {
        char c = *L.p;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',')
            L.p++;
        else if (c == '#')
        {
            while (L.p < L.e && *L.p != '\n')
                L.p++;
        }
        else
            break;
    }
}

char peek(Lex &L)
{
    skipws(L);
    return L.p < L.e ? *L.p : '\0';
}

bool is_name_start(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}
bool is_name(char c)
{
    return is_name_start(c) || (c >= '0' && c <= '9');
}

bool parse_name(Lex &L, char *out)
{
    skipws(L);
    if (L.p >= L.e || !is_name_start(*L.p))
        return false;
    int i = 0;
    while (L.p < L.e && is_name(*L.p))
    {
        if (i >= DETWS_GQL_NAME_MAX - 1)
        {
            g_err = DETWS_GQL_ERR_LIMIT;
            return false;
        }
        out[i++] = *L.p++;
    }
    out[i] = '\0';
    return true;
}

// Copy a decoded string into the strbuf pool; returns pointer or nullptr.
const char *intern(const char *s, int len)
{
    if (g_strlen + len + 1 > DETWS_GQL_STRBUF)
    {
        g_err = DETWS_GQL_ERR_LIMIT;
        return nullptr;
    }
    char *dst = g_strbuf + g_strlen;
    memcpy(dst, s, len);
    dst[len] = '\0';
    g_strlen += len + 1;
    return dst;
}

bool parse_value(Lex &L, DetwsGqlValue *v)
{
    char c = peek(L);
    if (c == '"')
    {
        L.p++; // opening quote
        char tmp[DETWS_GQL_STRBUF];
        int n = 0;
        while (L.p < L.e && *L.p != '"')
        {
            char ch = *L.p++;
            if (ch == '\\' && L.p < L.e)
            {
                char esc = *L.p++;
                switch (esc)
                {
                case 'n':
                    ch = '\n';
                    break;
                case 't':
                    ch = '\t';
                    break;
                case 'r':
                    ch = '\r';
                    break;
                case '"':
                    ch = '"';
                    break;
                case '\\':
                    ch = '\\';
                    break;
                case '/':
                    ch = '/';
                    break;
                default:
                    ch = esc;
                    break;
                }
            }
            if (n >= (int)sizeof(tmp) - 1)
            {
                g_err = DETWS_GQL_ERR_LIMIT;
                return false;
            }
            tmp[n++] = ch;
        }
        if (L.p >= L.e)
        {
            g_err = DETWS_GQL_ERR_PARSE;
            return false;
        }
        L.p++; // closing quote
        const char *s = intern(tmp, n);
        if (!s)
            return false;
        v->type = DETWS_GQL_STR;
        v->s = s;
        return true;
    }
    if (c == '-' || (c >= '0' && c <= '9'))
    {
        // Manual number parse (no stdlib): integer, optional fraction, optional
        // exponent. Builds an int64 for plain integers and a double otherwise.
        bool neg = false;
        if (*L.p == '-')
        {
            neg = true;
            L.p++;
        }
        bool any = false, is_float = false;
        long long ipart = 0;
        double fval = 0.0;
        while (L.p < L.e && *L.p >= '0' && *L.p <= '9')
        {
            ipart = ipart * 10 + (*L.p - '0');
            L.p++;
            any = true;
        }
        fval = (double)ipart;
        if (L.p < L.e && *L.p == '.')
        {
            is_float = true;
            L.p++;
            double scale = 1.0;
            while (L.p < L.e && *L.p >= '0' && *L.p <= '9')
            {
                scale *= 10.0;
                fval += (double)(*L.p - '0') / scale;
                L.p++;
                any = true;
            }
        }
        if (L.p < L.e && (*L.p == 'e' || *L.p == 'E'))
        {
            is_float = true;
            L.p++;
            bool eneg = false;
            if (L.p < L.e && (*L.p == '+' || *L.p == '-'))
                eneg = (*L.p++ == '-');
            int ex = 0;
            while (L.p < L.e && *L.p >= '0' && *L.p <= '9')
                ex = ex * 10 + (*L.p++ - '0');
            double m = 1.0;
            for (int k = 0; k < ex; k++)
                m *= 10.0;
            fval = eneg ? fval / m : fval * m;
        }
        if (!any)
        {
            g_err = DETWS_GQL_ERR_PARSE;
            return false;
        }
        if (is_float)
        {
            v->type = DETWS_GQL_FLOAT;
            v->f = neg ? -fval : fval;
        }
        else
        {
            v->type = DETWS_GQL_INT;
            v->i = neg ? -ipart : ipart;
        }
        return true;
    }
    // keyword: true / false / null
    char kw[8];
    if (parse_name(L, kw))
    {
        if (strcmp(kw, "true") == 0)
        {
            v->type = DETWS_GQL_BOOL;
            v->b = true;
            return true;
        }
        if (strcmp(kw, "false") == 0)
        {
            v->type = DETWS_GQL_BOOL;
            v->b = false;
            return true;
        }
        if (strcmp(kw, "null") == 0)
        {
            v->type = DETWS_GQL_NULL;
            return true;
        }
    }
    g_err = DETWS_GQL_ERR_PARSE;
    return false;
}

int parse_selection(Lex &L, int depth);

int parse_field(Lex &L, int depth)
{
    int idx = new_node();
    if (idx < 0)
        return -1;
    if (!parse_name(L, g_nodes[idx].name))
    {
        if (!g_err)
            g_err = DETWS_GQL_ERR_PARSE;
        return -1;
    }
    // arguments
    if (peek(L) == '(')
    {
        L.p++; // '('
        int first = -1, count = 0;
        while (peek(L) != ')')
        {
            if (g_nargs >= DETWS_GQL_MAX_ARGS)
            {
                g_err = DETWS_GQL_ERR_LIMIT;
                return -1;
            }
            Arg *a = &g_args[g_nargs];
            if (!parse_name(L, a->name))
            {
                if (!g_err)
                    g_err = DETWS_GQL_ERR_PARSE;
                return -1;
            }
            if (peek(L) != ':')
            {
                g_err = DETWS_GQL_ERR_PARSE;
                return -1;
            }
            L.p++; // ':'
            if (!parse_value(L, &a->val))
                return -1;
            if (first < 0)
                first = g_nargs;
            count++;
            g_nargs++;
        }
        L.p++; // ')'
        g_nodes[idx].first_arg = first;
        g_nodes[idx].n_args = count;
    }
    // sub-selection
    if (peek(L) == '{')
        g_nodes[idx].first_child = parse_selection(L, depth + 1);
    return g_err ? -1 : idx;
}

int parse_selection(Lex &L, int depth)
{
    if (depth > DETWS_GQL_MAX_DEPTH)
    {
        g_err = DETWS_GQL_ERR_LIMIT;
        return -1;
    }
    if (peek(L) != '{')
    {
        g_err = DETWS_GQL_ERR_PARSE;
        return -1;
    }
    L.p++; // '{'
    int first = -1, prev = -1;
    while (peek(L) != '}')
    {
        if (L.p >= L.e)
        {
            g_err = DETWS_GQL_ERR_PARSE;
            return -1;
        }
        int f = parse_field(L, depth);
        if (f < 0)
            return -1;
        if (first < 0)
            first = f;
        else
            g_nodes[prev].next_sib = f;
        prev = f;
    }
    L.p++; // '}'
    return first;
}

bool parse_document(Lex &L)
{
    char c = peek(L);
    if (c != '{')
    {
        char kw[DETWS_GQL_NAME_MAX];
        if (!parse_name(L, kw) || strcmp(kw, "query") != 0)
        {
            g_err = DETWS_GQL_ERR_PARSE; // only anonymous or `query` operations
            return false;
        }
        if (peek(L) != '{') // optional operation name
        {
            char opname[DETWS_GQL_NAME_MAX];
            if (!parse_name(L, opname))
            {
                if (!g_err)
                    g_err = DETWS_GQL_ERR_PARSE;
                return false;
            }
        }
    }
    g_root = parse_selection(L, 1);
    if (g_err)
        return false;
    if (peek(L) != '\0') // trailing junk after the operation
    {
        g_err = DETWS_GQL_ERR_PARSE;
        return false;
    }
    return true;
}

// ---- writer + executor ----------------------------------------------------
struct Writer
{
    char *o;
    size_t cap, n;
    bool ovf;
};
void w_raw(Writer &w, const char *s, size_t len)
{
    if (w.ovf)
        return;
    if (w.n + len > w.cap)
    {
        w.ovf = true;
        return;
    }
    memcpy(w.o + w.n, s, len);
    w.n += len;
}
void w_str(Writer &w, const char *s)
{
    w_raw(w, s, strlen(s));
}
void w_json_str(Writer &w, const char *s)
{
    w_raw(w, "\"", 1);
    for (const char *p = s; *p; p++)
    {
        unsigned char ch = (unsigned char)*p;
        if (ch == '"')
            w_raw(w, "\\\"", 2);
        else if (ch == '\\')
            w_raw(w, "\\\\", 2);
        else if (ch == '\n')
            w_raw(w, "\\n", 2);
        else if (ch == '\r')
            w_raw(w, "\\r", 2);
        else if (ch == '\t')
            w_raw(w, "\\t", 2);
        else if (ch < 0x20)
        {
            char u[7];
            snprintf(u, sizeof(u), "\\u%04x", ch);
            w_raw(w, u, 6);
        }
        else
            w_raw(w, (const char *)&ch, 1);
    }
    w_raw(w, "\"", 1);
}
void w_scalar(Writer &w, const DetwsGqlValue *v)
{
    char b[40];
    switch (v->type)
    {
    case DETWS_GQL_INT:
        snprintf(b, sizeof(b), "%lld", v->i);
        w_str(w, b);
        break;
    case DETWS_GQL_FLOAT:
        snprintf(b, sizeof(b), "%g", v->f);
        w_str(w, b);
        break;
    case DETWS_GQL_BOOL:
        w_str(w, v->b ? "true" : "false");
        break;
    case DETWS_GQL_STR:
        w_json_str(w, v->s ? v->s : "");
        break;
    default:
        w_str(w, "null");
        break;
    }
}

// scope stack of in-scope arg indices
int g_scope[DETWS_GQL_MAX_ARGS];
int g_scope_n;
detws_gql_resolver_fn g_resolver;
char g_path[DETWS_GQL_PATH_MAX];

void emit_field(Writer &w, int idx, int path_len)
{
    Node *node = &g_nodes[idx];

    // extend the dotted path: [parent].name
    int plen = path_len;
    if (plen > 0)
    {
        if (plen + 1 >= DETWS_GQL_PATH_MAX)
        {
            w.ovf = true;
            return;
        }
        g_path[plen++] = '.';
    }
    int nl = (int)strlen(node->name);
    if (plen + nl >= DETWS_GQL_PATH_MAX)
    {
        w.ovf = true;
        return;
    }
    memcpy(g_path + plen, node->name, nl);
    plen += nl;
    g_path[plen] = '\0';

    // push this field's args into scope
    int pushed = 0;
    for (int a = 0; a < node->n_args; a++)
        if (g_scope_n < DETWS_GQL_MAX_ARGS)
            g_scope[g_scope_n++] = node->first_arg + a, pushed++;

    w_json_str(w, node->name);
    w_raw(w, ":", 1);

    if (node->first_child >= 0)
    {
        w_raw(w, "{", 1);
        bool first = true;
        for (int c = node->first_child; c >= 0; c = g_nodes[c].next_sib)
        {
            if (!first)
                w_raw(w, ",", 1);
            first = false;
            emit_field(w, c, plen);
        }
        w_raw(w, "}", 1);
    }
    else
    {
        DetwsGqlValue v;
        v.type = DETWS_GQL_NULL;
        DetwsGqlArgs view = {g_scope, g_scope_n};
        if (g_resolver && g_resolver(g_path, &view, &v))
            w_scalar(w, &v);
        else
            w_str(w, "null");
    }

    g_scope_n -= pushed; // pop
    g_path[path_len] = '\0';
}
} // namespace

bool detws_gql_arg_int(const DetwsGqlArgs *args, const char *name, long long *out)
{
    if (!args)
        return false;
    for (int k = 0; k < args->count; k++)
    {
        Arg *a = &g_args[args->idx[k]];
        if (strcmp(a->name, name) == 0 && a->val.type == DETWS_GQL_INT)
        {
            *out = a->val.i;
            return true;
        }
    }
    return false;
}
bool detws_gql_arg_str(const DetwsGqlArgs *args, const char *name, const char **out)
{
    if (!args)
        return false;
    for (int k = 0; k < args->count; k++)
    {
        Arg *a = &g_args[args->idx[k]];
        if (strcmp(a->name, name) == 0 && a->val.type == DETWS_GQL_STR)
        {
            *out = a->val.s;
            return true;
        }
    }
    return false;
}
bool detws_gql_arg_bool(const DetwsGqlArgs *args, const char *name, bool *out)
{
    if (!args)
        return false;
    for (int k = 0; k < args->count; k++)
    {
        Arg *a = &g_args[args->idx[k]];
        if (strcmp(a->name, name) == 0 && a->val.type == DETWS_GQL_BOOL)
        {
            *out = a->val.b;
            return true;
        }
    }
    return false;
}

int detws_graphql_execute(const char *query, size_t len, detws_gql_resolver_fn resolver, char *out, size_t cap)
{
    g_nnodes = g_nargs = g_strlen = g_scope_n = 0;
    g_root = -1;
    g_err = 0;
    g_resolver = resolver;
    g_path[0] = '\0';

    Lex L = {query, query + (query ? len : 0)};
    if (!query || !out || cap == 0)
        return DETWS_GQL_ERR_PARSE;

    if (!parse_document(L))
    {
        const char *msg = (g_err == DETWS_GQL_ERR_LIMIT) ? "query exceeds a configured limit" : "syntax error";
        Writer w = {out, cap, 0, false};
        w_str(w, "{\"errors\":[{\"message\":");
        w_json_str(w, msg);
        w_str(w, "}]}");
        if (!w.ovf && w.n < cap)
            out[w.n] = '\0';
        return g_err ? g_err : DETWS_GQL_ERR_PARSE;
    }

    Writer w = {out, cap, 0, false};
    w_str(w, "{\"data\":{");
    bool first = true;
    for (int c = g_root; c >= 0; c = g_nodes[c].next_sib)
    {
        if (!first)
            w_raw(w, ",", 1);
        first = false;
        emit_field(w, c, 0);
    }
    w_str(w, "}}");
    if (w.ovf || w.n >= cap)
        return DETWS_GQL_ERR_OVERFLOW;
    out[w.n] = '\0';
    return DETWS_GQL_OK;
}

#endif // DETWS_ENABLE_GRAPHQL
