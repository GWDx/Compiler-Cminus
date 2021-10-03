%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();
extern int yyparse();
extern int yyrestart();
extern FILE * yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char * yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
syntax_tree_node *node(const char *node_name, int children_num, ...);
%}

/* TODO: Complete this definition.
   Hint: See pass_node(), node(), and syntax_tree.h.
         Use forward declaring. */
%union {
    struct _syntax_tree_node *node;     // syntax_tree_node * 会报错

}

/* TODO: Your tokens here. */

%token <node> INT FLOAT VOID
%token <node> IF ELSE WHILE RETURN
%token <node> ID
%token <node> FLOATPOINT INTEGER
%token <node> ADD SUBTRACT MULTIPLY DIVIDE
%token <node> LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token <node> GREATEREQUAL GREATER LESS LESSEQUAL EQUAL UNEQUAL
%token <node> ASSIGN SEMICOLON COMMA
%token <node> ERROR


%type <node> program declaration_list declaration var_declaration type_specifier
%type <node> fun_declaration params param_list param compound_stmt
%type <node> local_declarations statement_list statement expression_stmt selection_stmt
%type <node> iteration_stmt return_stmt expression var simple_expression
%type <node> relop additive_expression addop term mulop
%type <node> factor integer float call args
%type <node> arg_list


%start program


%%
/* TODO: Your rules here. */

// 1
program : declaration_list {$$ = node("program", 1, $1); gt->root = $$;};

declaration_list : declaration_list declaration {$$ = node("declaration-list", 2, $1, $2);}
    | declaration {$$ = node("declaration-list", 1, $1);};

declaration : var_declaration {$$ = node("declaration", 1, $1);}
    | fun_declaration {$$ = node("declaration", 1, $1);};

var_declaration : type_specifier ID SEMICOLON {$$ = node("var-declaration", 3, $1, $2, $3);}
    | type_specifier ID LBRACKET INTEGER RBRACKET SEMICOLON {$$ = node("var-declaration", 6, $1, $2, $3, $4, $5, $6);};

type_specifier : INT {$$ = node("type-specifier", 1, $1);}
    | FLOAT {$$ = node("type-specifier", 1, $1);}
    | VOID {$$ = node("type-specifier", 1, $1);};

// 6
fun_declaration : type_specifier ID LPAREN params RPAREN compound_stmt {$$ = node("fun-declaration", 6, $1, $2, $3, $4, $5, $6);};

params : param_list {$$ = node("params", 1, $1);}
    | VOID {$$ = node("params", 1, $1);};

param_list : param_list COMMA param {$$ = node("param-list", 3, $1, $2, $3);}
    | param {$$ = node("param-list", 1, $1);};

param : type_specifier ID {$$ = node("param", 2, $1, $2);}
    | type_specifier ID LBRACKET RBRACKET {$$ = node("param", 4, $1, $2, $3, $4);};

compound_stmt : LBRACE local_declarations statement_list RBRACE {$$ = node("compound-stmt", 4, $1, $2, $3, $4);};

// 11
local_declarations : local_declarations var_declaration {$$ = node("local-declarations", 2, $1, $2);}
    |  {$$ = node("local-declarations", 0);};

statement_list : statement_list statement {$$ = node("statement-list", 2, $1, $2);}
    |  {$$ = node("statement-list", 0);};

statement : expression_stmt {$$ = node("statement", 1, $1);}
    | compound_stmt {$$ = node("statement", 1, $1);}
    | selection_stmt {$$ = node("statement", 1, $1);}
    | iteration_stmt {$$ = node("statement", 1, $1);}
    | return_stmt {$$ = node("statement", 1, $1);};

expression_stmt : expression SEMICOLON {$$ = node("expression-stmt", 2, $1, $2);}
    | SEMICOLON {$$ = node("expression-stmt", 1, $1);};

selection_stmt : IF LPAREN expression RPAREN statement {$$ = node("selection-stmt", 5, $1, $2, $3, $4, $5);}
    | IF LPAREN expression RPAREN statement ELSE statement {$$ = node("selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7);};

// 16
iteration_stmt : WHILE LPAREN expression RPAREN statement {$$ = node("iteration-stmt", 5, $1, $2, $3, $4, $5);};

return_stmt : RETURN SEMICOLON {$$ = node("return-stmt", 2, $1, $2);}
    | RETURN expression SEMICOLON {$$ = node("return-stmt", 3, $1, $2, $3);};

expression : var ASSIGN expression {$$ = node("expression", 3, $1, $2, $3);}
    | simple_expression {$$ = node("expression", 1, $1);};

var : ID {$$ = node("var", 1, $1);}
    | ID LBRACKET expression RBRACKET {$$ = node("var", 4, $1, $2, $3, $4);};

simple_expression : additive_expression relop additive_expression {$$ = node("simple-expression", 3, $1, $2, $3);}
    | additive_expression {$$ = node("simple-expression", 1, $1);};

// 21
relop : GREATEREQUAL {$$ = node("relop", 1, $1);}
    | GREATER {$$ = node("relop", 1, $1);}
    | LESS {$$ = node("relop", 1, $1);}
    | LESSEQUAL {$$ = node("relop", 1, $1);}
    | EQUAL {$$ = node("relop", 1, $1);}
    | UNEQUAL {$$ = node("relop", 1, $1);};

additive_expression : additive_expression addop term {$$ = node("additive-expression", 3, $1, $2, $3);}
    | term {$$ = node("additive-expression", 1, $1);};

addop : ADD {$$ = node("addop", 1, $1);}
    | SUBTRACT {$$ = node("addop", 1, $1);};

term : term mulop factor {$$ = node("term", 3, $1, $2, $3);}
    | factor {$$ = node("term", 1, $1);};

mulop : MULTIPLY {$$ = node("mulop", 1, $1);}
    | DIVIDE {$$ = node("mulop", 1, $1);};

// 26

factor : LPAREN expression RPAREN {$$ = node("factor", 3, $1, $2, $3);}
    | var {$$ = node("factor", 1, $1);}
    | call {$$ = node("factor", 1, $1);}
    | integer {$$ = node("factor", 1, $1);}
    | float {$$ = node("factor", 1, $1);};

integer : INTEGER {$$ = node("integer", 1, $1);};

float : FLOATPOINT {$$ = node("float", 1, $1);};

call : ID LPAREN args RPAREN {$$ = node("call", 4, $1, $2, $3, $4);};

args : arg_list {$$ = node("args", 1, $1);}
    |  {$$ = node("args", 0);};

// 31
arg_list : arg_list COMMA expression {$$ = node("arg-list", 3, $1, $2, $3);}
    | expression {$$ = node("arg-list", 1, $1);};

%%

/// The error reporting function.
void yyerror(const char * s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    fprintf(stderr, "error at line %d column %d: %s\n", lines, pos_start, s);
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL) {
        if (!(yyin = fopen(input_path, "r"))) {
            fprintf(stderr, "[ERR] Open input file %s failed.\n", input_path);
            exit(1);
        }
    } else {
        yyin = stdin;
    }

    lines = pos_start = pos_end = 1;
    gt = new_syntax_tree();
    yyrestart(yyin);
    yyparse();
    return gt;
}

/// A helper function to quickly construct a tree node.
///
/// e.g. $$ = node("program", 1, $1);
syntax_tree_node *node(const char *name, int children_num, ...)
{
    syntax_tree_node *p = new_syntax_tree_node(name);
    syntax_tree_node *child;
    if (children_num == 0) {
        child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child);
    } else {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i) {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
