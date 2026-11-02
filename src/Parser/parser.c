#include <stdlib.h>
#include <windows.h>

#include "Globals/error.h"
#include "Globals/string.h"
#include "Globals/table.h"
#include "Parser/parser.h"
#include "TypeResolving/type_specifiers.h"
#include "Values/bytescript_funcs.h"

TValue user_types;


static Node* bin_op(ParseResult* res, ParseFunc func1, Token ops[], int opc, ParseFunc func2, char expected[]) {
    if (func2 == NULL) func2 = func1;

    Node* left = BOPTIONAL(func1(res));
    while (is_correct_operator(*res->tokens, ops, opc)) {
        Token op = *res->tokens; res->tokens++;
        Node* right = EXPECTED(func2(res), expected);

        Node* n_left = new_node(NT_BIN_OP);
        n_left->node.BinOpNode.left = left;
        n_left->node.BinOpNode.op = op;
        n_left->node.BinOpNode.right =  right;
        left = n_left;
    }

    return left;
}

static NodeArray separated(ParseResult* res, ParseFunc func, Token separator) {
    NodeArray nda = make_node_array();

    do {
        Node* elem = func(res);
        if (!elem) break;

        append_to_node_array(&nda, elem);
        if (!match_token(*res->tokens, SEPARATOR)) break;

        res->tokens++;
        clean_newlines(&res->tokens, &res->adv_count);
    } while (1);

    return nda;
}

static Node* call_like(Node* return_node, ParseResult* res, Node* num, NodeType nt) {
    clean_newlines(&res->tokens, &res->adv_count);
    ERR_CHECK(NodeArray args = separated(res, expr, SEPARATOR));
    
    ERR_CHECK(expect(res, TT_RPAR, "(RPAR : ')') EXPECTED", NULL, NULL))
    return_node->nt = nt;
    return_node->node.CallNode.args = args;
    return_node->node.CallNode.target = num;

    return return_node;
}

static Node* postfix_op(ParseResult* res, Node* num) {
    Node* return_node = new_node(NT_PLACEHOLDER);

    switch (res->tokens->tt) {
        case TT_COLON: {
            res->tokens++;

            ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &return_node->node.SelfCallnode.name, NULL))
            ERR_CHECK(expect(res, TT_LPAR, "(LPAR : '(') EXPECTED", NULL, NULL))

            return call_like(return_node, res, num, NT_SELFCALL);
        }

        case TT_LPAR: {
            res->tokens++; return call_like(return_node, res, num, NT_CALL);
        }

        case TT_LSBRAC: {
            res->tokens++;
            
            Node* index = EXPECTED(logical(res), "(LOGICAL-EXPRESSION) EXPECTED");
            return_node->nt = NT_INDEXING;
            return_node->node.IndexNode.target = num;
            return_node->node.IndexNode.index = index;

            ERR_CHECK(expect(res, TT_RSBRAC, "(RSBRAC : ']') EXPECTED", NULL, NULL))
            return return_node;
        }

        case TT_DOT: {
            res->tokens++; return_node->nt = NT_FIELDACCESS;
            return_node->node.FieldAccessNode.target = num;
            ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &return_node->node.FieldAccessNode.name, NULL))

            return return_node;
        }
    }

    free(return_node);
    return NULL;
}


Node* parse(Token* tokens, char filename[]) {
    init_operators();
    ParseResult res = new_res(tokens);

    Node* ast = main_parse(&res);
    if (res.err_details != NULL) call_error(new_syntax_error, res.err_details);
    return ast;
}

Node* alloc_expr(ParseResult* res) {
    res->tokens++;

    Node* target = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");
    ERR_CHECK(expect(res, TT_ARROW, "(ARROW) EXPECTED", NULL, NULL))

    clean_newlines(&res->tokens, &res->adv_count);
    NodeArray args =  separated(res, expr, SEPARATOR);

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    
    Node* return_node = new_node(NT_ALLOCATION);
    return_node->node.AllocNode.args = args;
    return_node->node.AllocNode.target = target;

    return return_node;
}

Node* if_expr(ParseResult* res) {
    NodeArray cases = make_node_array();
    NodeArray stments = make_node_array();

    do {
        res->tokens++;

        Node* condition = EXPECTED(logical(res), "(EXPRESSION) EXPECTED");
        ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'then') EXPECTED", NULL, "then"))
        Node* thenStatements = get_parsed(statement, res, ELSEIF_TOKEN);
        
        append_to_node_array(&cases, condition);
        append_to_node_array(&stments, thenStatements);
    } while( match_keyword(*res->tokens, "elseif") );

    if (match_keyword(*res->tokens, "else")) {
        res->tokens++;
        Node* thenStatements = get_parsed(statement, res, END_TOKEN);
        append_to_node_array(&stments, thenStatements);
    }

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))

    Node* ifNode = new_node(NT_IF);
    ifNode->node.IfNode.cases = cases;
    ifNode->node.IfNode.statements = stments;

    return ifNode;
}

Node* for_expr(ParseResult* res) {
    res->tokens++;
    Node* return_node;
    
    TValue i;
    TValue v = NIL;
    ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &i, NULL))
    
    switch (res->tokens->tt) {
        case TT_EQ: {
            res->tokens++;
            Node* baseValue = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");
            ERR_CHECK(expect(res, TT_SEPARATOR, "(SEPARATOR:',') EXPECTED", NULL, NULL))
            Node* endValue = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");

            Node* stepValue;
            if (res->tokens->tt == TT_SEPARATOR) { res->tokens++; stepValue = EXPECTED(expr(res), "(EXPRESSION) EXPECTED"); }
            else (stepValue = new_node(NT_NUMBER))->node.NumberNode.num = NUM_AS_TVALUE(1);

            ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'repeat') EXPECTED", NULL, "repeat"))
            Node* stments = get_parsed(statement, res, END_TOKEN);

            Node* nfor_node = new_node(NT_NFOR);
            nfor_node->node.nForNode.identifier = i;
            nfor_node->node.nForNode.values[0] = baseValue;
            nfor_node->node.nForNode.values[1] = endValue;
            nfor_node->node.nForNode.values[2] = stepValue;
            nfor_node->node.nForNode.statements = stments;

            return_node = nfor_node;
            break;
        }

        case TT_SEPARATOR:
        case TT_KWORD: {
            if (res->tokens->tt == TT_SEPARATOR) { res->tokens++; ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &v, NULL)) }
            ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'in') EXPECTED", NULL, "in"))

            Node* target = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");
            ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'repeat') EXPECTED", NULL, "repeat"))
            Node* stments = get_parsed(statement, res, END_TOKEN);

            Node* gfor_node = new_node(NT_GFOR);
            gfor_node->node.gForNode.identifiers[0] = i;
            gfor_node->node.gForNode.identifiers[1] = v;
            gfor_node->node.gForNode.target = target;
            gfor_node->node.gForNode.statements = stments;

            return_node = gfor_node;
            break;
        }

        default: call_error(new_syntax_error, "(EQ:'=' || SEPARATOR:',') EXPECTED");
    }

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    return return_node;
}

Node* while_expr(ParseResult* res) {
    res->tokens++;

    Node* condition = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");
    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'repeat') EXPECTED", NULL, "repeat"))
    Node* stments = get_parsed(statement, res, END_TOKEN);

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    Node* while_node = new_node(NT_WHILE);
    while_node->node.whileNode.condition = condition;
    while_node->node.whileNode.statements = stments;

    return while_node;
}

Node* repeat_expr(ParseResult* res) {
    res->tokens++;

    Node* stments = get_parsed(statement, res, WHILE_TOKEN);

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'while') EXPECTED", NULL, "while"))
    Node* condition = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");

    Node* repeat_node = new_node(NT_REPEAT);
    repeat_node->node.whileNode.condition = condition;
    repeat_node->node.whileNode.statements = stments;

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    return repeat_node;
}

Node* require_expr(ParseResult* res) {
    res->tokens++;

    NodeType nt;
    TValue module;
    TValue type = NIL;
    TValue identifier = NIL;
    ERR_CHECK(expect(res, TT_STR, "(STR) EXPECTED", &module, NULL))

    if (match_keyword(*res->tokens++, "as")) {
        type = type_specifier(res);
        if (IS_NIL(type)) type = varType; else res->tokens++;
        
        ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &identifier, NULL))
    }

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
#ifdef _WIN32

    nt = has_ext(module, ".dll") ? NT_CREQUIRE : NT_REQUIRE;
#else
    nt = has_ext(module, ".so") ? NT_CREQUIRE : NT_REQUIRE;

#endif

    Node* require_node = new_node(nt);
    require_node->node.importNode.module = module;
    require_node->node.importNode.type_specifier = type;
    require_node->node.importNode.identifier = identifier;
    if (nt == NT_REQUIRE) require_node->node.importNode.ast = simple_parse(AS_STRING(require_node->node.importNode.module)->str);
    else {
        HMODULE dll = LoadLibrary(AS_STRING(module)->str);
        if (!dll) {
            TValue error_message = make_fstring("Could not open shared library '%s'");
            error_message = format_string((TValue[2]){ error_message, module }, 2);

            call_error(new_failed_allocation_error, AS_STRING(error_message)->str);
        }

        void* filename_ptr = GetProcAddress(dll, "bytescript_module_map");
        if (!filename_ptr) {
            TValue error_message = make_fstring("Could not find a map for module '%s'. Make sure to store the path to your module's map in a variable of named : 'bytescript_module_map'");
            error_message = format_string((TValue[2]){ error_message, module }, 2);

            call_error(new_failed_allocation_error, AS_STRING(error_message)->str);
        }

        char** filename = (char**)filename_ptr;
        require_node->node.importNode.ast = simple_parse(*filename);
        require_node->node.cimportNode.dll = dll;
    }

    return require_node;
}

Node* from_import_expr(ParseResult* res) {
    res->tokens++;

    TValue module;
    TValue identifier = NIL;
    ERR_CHECK(expect(res, TT_STR, "(STR) EXPECTED", &module, NULL))
    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'import') EXPECTED", NULL, "import"))

#if _WIN32
    Node* import_node = has_ext(module, "dll") ? new_node(NT_CIMPORT) : new_node(NT_IMPORT);
#else
    Node* import_node = has_ext(module, ".so") ? new_node(NT_CIMPORT) : new_node(NT_IMPORT);
#endif
    import_node->node.importNode.identifier = NIL;
    import_node->node.importNode.module = module;
    if (import_node->nt == NT_IMPORT) import_node->node.importNode.ast = simple_parse(AS_STRING(module)->str);

    Node* from_import_node = new_node(NT_FROM_IMPORT);
    from_import_node->node.fromImportNode.import_node = import_node;

    from_import_node->node.fromImportNode.elements.next = NULL;
    from_import_node->node.fromImportNode.elements.value = NIL;
    from_import_node->node.fromImportNode.elem_types.next = NULL;
    from_import_node->node.fromImportNode.elem_types.value = NIL;
    from_import_node->node.fromImportNode.nicknames.next = NULL;
    from_import_node->node.fromImportNode.nicknames.value = NIL;

    TValue arg_specifier;
    if (!IS_NIL(arg_specifier = type_specifier(res)) || res->tokens->tt == TT_IDENTIFIER) {
        while (!IS_NIL(arg_specifier = type_specifier(res)) || res->tokens->tt == TT_IDENTIFIER) {
            if (!IS_NIL(arg_specifier)) res->tokens++;
            else arg_specifier = varType;

            struct TValueLinkedList* new = malloc(sizeof(struct TValueLinkedList));
            struct TValueLinkedList* new_type = malloc(sizeof(struct TValueLinkedList));
            struct TValueLinkedList* new_nickname = malloc(sizeof(struct TValueLinkedList));

            ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &new->value, NULL))
            new_nickname->value = NIL;
            new_type->value = arg_specifier;

            if (match_keyword(*res->tokens, "as")) { res->tokens++; ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &new_nickname->value, NULL)); }
            
            new->next = from_import_node->node.fromImportNode.elements.next;
            new_type->next = from_import_node->node.fromImportNode.elem_types.next;
            new_nickname->next = from_import_node->node.fromImportNode.nicknames.next;
            from_import_node->node.fromImportNode.elements.next = new;
            from_import_node->node.fromImportNode.elem_types.next = new_type;
            from_import_node->node.fromImportNode.nicknames.next = new_nickname;

            if (res->tokens->tt != TT_SEPARATOR) break; else res->tokens++;
        }

    } else ERR_CHECK(res->err_details = "Must specify import needs when using syntax 'from import'");

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    return from_import_node;
}

Node* import_expr(ParseResult* res) {
    res->tokens++;

    NodeType nt;
    TValue module;
    TValue identifier = NIL;

    ERR_CHECK(expect(res, TT_STR, "(STR) EXPECTED", &module, NULL))
    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'as') EXPECTED", NULL, "as"))
    ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &identifier, NULL));
    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
#ifdef _WIN32
    nt = has_ext(module, ".dll") ? NT_CIMPORT : NT_IMPORT;
#else
    nt = has_ext(module, ".so") ? NT_CIMPORT : NT_IMPORT;
#endif
    module = import_path_finder(module);

    Node* import_node = new_node(nt);
    import_node->node.importNode.module = module;
    import_node->node.importNode.type_specifier = NIL;
    import_node->node.importNode.identifier = identifier;
    if (nt == NT_IMPORT) import_node->node.importNode.ast = simple_parse(AS_STRING(import_node->node.importNode.module)->str);
    else {
        HMODULE dll = LoadLibrary(AS_STRING(module)->str);
        void* filename_ptr = GetProcAddress(dll, "bytescript_module_map");

        if (!filename_ptr) {
            TValue error_message = make_fstring("Could not find a map for module '%s'. Make sure to store the path to your module's map in a variable of named : 'bytescript_module_map'");
            error_message = format_string((TValue[2]){ error_message, module }, 2);

            call_error(new_failed_allocation_error, AS_STRING(error_message)->str);
        }

        char** filename = (char**)filename_ptr;
        import_node->node.importNode.ast = simple_parse(*filename);
        import_node->node.cimportNode.dll = dll;
    }

    return import_node;
}

Node* func_expr(ParseResult* res) {
    res->tokens++;

    TValue modifiers = get_modifiers(res);
    TValue func_type = type_specifier(res);
    if (IS_NIL(func_type)) func_type = varType; else res->tokens++;

    TValue func_name = NIL;
    if (res->tokens->tt == TT_IDENTIFIER) func_name = res->tokens++->value;

    ERR_CHECK(expect(res, TT_LPAR, "(LPAR : '(') EXPECTED", NULL, NULL))
    Node* func_node = new_node(NT_FUNCDEF);
    func_node->node.funcDefNode.name = func_name;
    func_node->node.funcDefNode.type_specifier = func_type;
    func_node->node.funcDefNode.modifiers = modifiers;
    func_node->node.funcDefNode.args.value = NIL;
    func_node->node.funcDefNode.args.next = NULL;
    func_node->node.funcDefNode.arg_types.value = NIL;
    func_node->node.funcDefNode.arg_types.next = NULL;

    TValue arg_specifier;
    if (!IS_NIL(arg_specifier = type_specifier(res)) || res->tokens->tt == TT_IDENTIFIER) {
        while (!IS_NIL(arg_specifier = type_specifier(res)) || res->tokens->tt == TT_IDENTIFIER) {
            if (!IS_NIL(arg_specifier)) res->tokens++;
            else arg_specifier = varType;

            struct TValueLinkedList* new = malloc(sizeof(struct TValueLinkedList));
            struct TValueLinkedList* new_type = malloc(sizeof(struct TValueLinkedList));

            ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &new->value, NULL))
            new_type->value = arg_specifier;
            
            new->next = func_node->node.funcDefNode.args.next;
            new_type->next = func_node->node.funcDefNode.arg_types.next;
            func_node->node.funcDefNode.args.next = new;
            func_node->node.funcDefNode.arg_types.next = new_type;

            if (res->tokens->tt != TT_SEPARATOR) break; else res->tokens++;
        }
    }

    ERR_CHECK(expect(res, TT_RPAR, "(LPAR : ')') EXPECTED", NULL, NULL))
    Node* stments = get_parsed(statement, res, END_TOKEN);
    func_node->node.funcDefNode.statements = stments;

    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    return func_node;
}

Node* class_expr(ParseResult* res) {
    res->tokens++;

    Node* class_node = new_node(NT_CLASSDEF);
    ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &class_node->node.classDefNode.name, NULL))

    class_node->node.classDefNode.inherited.next = NULL;
    class_node->node.classDefNode.inherited.value = NIL;
    if (res->tokens->tt == TT_ARROW) {
        res->tokens++;

        while (res->tokens->tt == TT_IDENTIFIER) {
            struct TValueLinkedList* new = malloc(sizeof(struct TValueLinkedList));

            new->value = res->tokens->value;
            new->next =  class_node->node.classDefNode.inherited.next;
            class_node->node.classDefNode.inherited.next = new;

            if ((++res->tokens)->tt != TT_SEPARATOR) break; else res->tokens++;
        }
    }

    add_value_to_hashtable(user_types, class_node->node.classDefNode.name, NUM_AS_TVALUE(1));

    class_node->node.classDefNode.statements = ERR_CHECK(main_parse(res));
    ERR_CHECK(expect(res, TT_KWORD, "(KEYWORD:'end') EXPECTED", NULL, "end"))
    return class_node;
}

Node* numerics(ParseResult* res) {
    Node* return_node = new_node(NT_PLACEHOLDER);

    switch (res->tokens->tt) {
        case TT_IDENTIFIER : {
            return_node->nt = NT_VAR_ACESS;
            return_node->node.VarAcess.name = res->tokens->value;
            break;
        }

        case TT_NUMBER : {
            return_node->nt = NT_NUMBER;
            return_node->node.NumberNode.num = res->tokens->value;
            break;
        }

        case TT_STR : {
            return_node->nt = NT_STRING;
            return_node->node.StringNode.str = res->tokens->value;
            break;
        }

        case TT_LPAR : {
            res->tokens++;
            Node* nexpr = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");
            ERR_CHECK(expect(res, TT_RPAR, "(RPAR : ')' ) EXPECTED", NULL, NULL))

            return_node->nt = nexpr->nt;
            return_node->node = nexpr->node;
            free(nexpr); res->tokens--;
            break;
        }

        case TT_LCBRAC : {
            res->tokens++;
            return_node->nt = NT_TABLE;
            return_node->node.TableNode.values = separated(res, logical, SEPARATOR);

            ERR_CHECK(expect(res, TT_RCBRAC, "(RCBRAC : '}') EXPECTED", NULL, NULL))
            res->tokens--; break;
        }

        case TT_KWORD : {
            if (match_keyword(*res->tokens, "func")) { free(return_node); return func_expr(res); }
        }

        default : {
            //res->err_details = "(INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED";
            free(return_node);
            return NULL;
        }
    }

    res->tokens++;
    return return_node;
}

Node* postfix(ParseResult* res) {
    Node* postfix_node = BOPTIONAL(numerics(res));

    for (;;) {
        ERR_CHECK(Node* op_node = postfix_op(res, postfix_node));
        if (!op_node) break; postfix_node = op_node;
    }

    return postfix_node;
}

Node* power(ParseResult* res) { return bin_op(res, postfix, POWER_OPERATORS, POWER_OPERATORS_SIZE, factor, "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED"); }

Node* factor(ParseResult* res) {
    if (is_correct_operator(*res->tokens, FACTOR_OPERATORS, FACTOR_OPERATORS_SIZE)) {
        Token op = *res->tokens++;
        Node* target = EXPECTED(factor(res), "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED");

        Node* return_node = new_node(NT_UNI_OP);
        return_node->node.UniOpNode.op = op;
        return_node->node.UniOpNode.target = target;

        return return_node;
    }

    return power(res);
}

Node* term(ParseResult* res) { return bin_op(res, factor, TERM_OPERATORS, TERM_OPERATORS_SIZE, NULL, "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED"); }

Node* arithmetic(ParseResult* res) { return bin_op(res, term, ARITHMETIC_OPERATORS, ARITHMETIC_OPERATORS_SIZE, NULL, "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED"); }

Node* comparaison(ParseResult* res) {
    if (match_tt(*res->tokens, TT_NOT)) {
        Token op = *res->tokens;
        Node* target = EXPECTED(comparaison(res), "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED");

        Node* return_node = new_node(NT_UNI_OP);
        return_node->node.UniOpNode.op = op;
        return_node->node.UniOpNode.target = target;

        return return_node;
    }

    return bin_op(res, arithmetic, COMP_OPERATORS, COMP_OPERATORS_SIZE, NULL, "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED");
}

Node* logical(ParseResult* res) { return bin_op(res, comparaison, LOGICAL_OPERATORS, LOGICAL_OPERATORS_SIZE, NULL, "(POSTFIX | INT | STR | FUNC | TABLE | IDENTIFIER) EXPECTED"); }

Node* assignement(ParseResult* res) {
    Node* log = BOPTIONAL(logical(res));
    if (res->tokens->tt == TT_EQ) {
        res->tokens++;
        Node* exp = expr(res);

        Node* new = new_node(NT_ASSIGNEMENT);
        new->node.AssignementNode.lvalue = log;
        new->node.AssignementNode.value = exp;
        log = new;
    }

    return log;
}

Node* expr(ParseResult* res) {
    TValue vartype;
    if (!IS_NIL((vartype = type_specifier(res)))) {
        Node* return_node = new_node(NT_VAR_DECLARE);
        TValue var_name; res->tokens++;

        if (match_token(*res->tokens, SELF_TOKEN)) {
            return_node->nt = NT_SELFDECLARE; res->tokens++;
            ERR_CHECK(expect(res, TT_DOT, "(DOT : '.') EXPECTED", NULL, NULL))
            ERR_CHECK(expect(res, TT_IDENTIFIER, "(IDENTIFIER) EXPECTED", &var_name, NULL))
        } else {
            if (res->tokens->tt == TT_IDENTIFIER) { var_name = res->tokens++->value; }
            else { res->tokens--; return assignement(res); }
        }


        return_node->node.VarDeclareNode.type_specifier = vartype;
        return_node->node.VarDeclareNode.var_name = var_name;
        if (res->tokens->tt == TT_NEWLINE) {
            return_node->node.VarDeclareNode.value = NULL;

            return return_node;
        }

        ERR_CHECK(expect(res, TT_EQ, "(EQ : '=') EXPECTED", NULL, NULL))
        Node* nexpr = EXPECTED(expr(res), "(EXPRESSION) EXPECTED");
        return_node->node.VarDeclareNode.value = nexpr;

        return return_node;
    } else if (match_keyword(*res->tokens, "require")) return require_expr(res);

    return assignement(res);
}

Node* statement(ParseResult* res) {
    if (match_keyword(*res->tokens, "return")) {
        res->tokens++; NodeArray values =  separated(res, expr, SEPARATOR);
        Node* return_node = new_node(NT_RETURN);
        return_node->node.ReturnNode = values;

        return return_node;

    } else if (match_keyword(*res->tokens, "break")) { res->tokens++; return new_node(NT_BREAK); }
    else if (match_keyword(*res->tokens, "continue")) { res->tokens++; return new_node(NT_CONTINUE); }
    else if (match_keyword(*res->tokens, "alloc")) return alloc_expr(res);
    else if (match_keyword(*res->tokens, "if")) return if_expr(res);
    else if (match_keyword(*res->tokens, "for")) return for_expr(res);
    else if (match_keyword(*res->tokens, "while")) return while_expr(res);
    else if (match_keyword(*res->tokens, "repeat")) return repeat_expr(res);
    else if (match_keyword(*res->tokens, "import")) return import_expr(res);
    else if (match_keyword(*res->tokens, "from")) return from_import_expr(res);

    return expr(res);
}

Node* statements(ParseResult* res) { return get_parsed(statement, res, EOF_TOKEN); }

Node* toplvl_statement(ParseResult* res) {
    if (match_keyword(*res->tokens, "class")) return class_expr(res);
    return statement(res);
}

Node* main_parse(ParseResult* res) { return get_parsed(toplvl_statement, res, EOF_TOKEN); }


Node* get_parsed(ParseFunc func, ParseResult* res, Token stopToken) {
    Node* node = new_node(NT_STATEMENTS);
    NodeArray statementNodes = make_node_array();

    clean_newlines(&res->tokens, &res->adv_count);
    while (res->tokens->tt != TT_EOF && !match_token(*res->tokens, stopToken)) {
        Node* stmnt = func(res);

        if (stmnt == NULL) break;
        append_to_node_array(&statementNodes, stmnt);
        clean_newlines(&res->tokens, &res->adv_count);
    }

    node->node.StatementsNode = statementNodes;
    return node;
}

static inline void clean_newlines(Token** tokens, size_t* adv_count) { while ( (**tokens).tt == TT_NEWLINE && ++*adv_count ) ++*tokens; }
static inline ParseResult new_res(Token* tokens) { return (ParseResult){ .err_details = NULL, .adv_count = 0, .tokens = tokens }; }


void clean_node_array(NodeArray* nda, size_t start_index) {
    for (int i = start_index; i < nda->cap; i++) nda->nodes[i] = NULL;
}

NodeArray make_node_array() {
    size_t cap = 8;
    Node** ndarray = malloc(sizeof(Node*) * cap);
    if (ndarray == NULL) call_error(new_failed_allocation_error, "Failed allocation while making node");

    NodeArray nda = { .len = 0, .cap = cap, .nodes = ndarray };
    clean_node_array(&nda, 0);
    return nda;
}

Node* append_to_node_array(NodeArray* nda, Node* node) {
    if (nda->len >= nda->cap) {
        nda->nodes = realloc(nda->nodes, sizeof(Node*) * (nda->cap *= 2));
        if (nda->nodes == NULL) call_error(new_failed_allocation_error, "Failed allocation while making node");

        clean_node_array(nda, nda->cap / 2);
    }

    nda->nodes[nda->len++] = node;
}


static inline bool is_correct_operator(Token op, Token ops[], int opc) {
    for (int i = 0; i < opc; i++) {
        if (match_token(op, ops[i])) return true;
    }

    return false;
}

static void expect(ParseResult* res, BTokenType tt, char* expected, TValue* value, char* kword) {
    if (tt == TT_IDENTIFIER && res->tokens->tt == TT_KWORD) res->tokens->tt = TT_IDENTIFIER;
    if (res->tokens->tt != tt || (kword && !match_keyword(*res->tokens, kword))) res->err_details = expected;
    if (value != NULL) *value = res->tokens->value;
    res->tokens++;
}

TValue type_specifier(ParseResult* res) {
    if (res->tokens->tt != TT_KWORD && res->tokens->tt != TT_IDENTIFIER) return NIL;
    if ((EQUAL(res->tokens->value, varType)) ||
        (res->tokens->tt == TT_IDENTIFIER && TRUTHY(get_value_from_hashtable(user_types, res->tokens->value)))) return res->tokens->value;

    return NIL;
}

TValue get_modifiers(ParseResult* res) {
    TValue modif = new_table(NULL, 0);

    while (res->tokens->tt == TT_KWORD) {
        if (match_keyword(*res->tokens, "static")) append_to_table(modif, res->tokens->value);
        else break;
        res->tokens++;
    }

    return modif;
}
