%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines=1;
int pos_start;
int pos_end=1;

void pass_node(char *text){
    yylval.node = new_syntax_tree_node(text);
}

#define print printf(">> %s\n", yytext);

#define DealPos pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext);

/*****************声明和选项设置  end*****************/

%}


%%
 /* to do for students */
 /* two cases for you, pass_node will send flex's token to bison */

 // 注释
\/\*([^\*]|\*[^\/])*\*+\/   {/*print; pos_end=strlen(strrchr(yytext, '\n'));*/ DealPos;}

[a-zA-Z]+ {
    DealPos;
    if(strcmp(yytext, "int")==0) return INT;
    if(strcmp(yytext, "float")==0) return FLOAT;
    if(strcmp(yytext, "void")==0) return VOID;

    if(strcmp(yytext, "if")==0) return IF;
    if(strcmp(yytext, "else")==0) return ELSE;
    if(strcmp(yytext, "while")==0) return WHILE;
    if(strcmp(yytext, "return")==0) return RETURN;
    return ID;
}

[0-9]+\.|[0-9]*\.[0-9]+  {
    DealPos;
    return FLOATPOINT;
}

[0-9]+  {
    DealPos;
    return INTEGER;
}

"+" {DealPos; return ADD;}
"-" {DealPos; return SUBTRACT;}
"*" {DealPos; return MULTIPLY;}
"/" {DealPos; return DIVIDE;}

"(" {DealPos; return LPAREN;}
")" {DealPos; return RPAREN;}
"[" {DealPos; return LBRACKET;}
"]" {DealPos; return RBRACKET;}
"{" {DealPos; return LBRACE;}
"}" {DealPos; return RBRACE;}

">=" {DealPos; return GREATEREQUAL;}
">"  {DealPos; return GREATER;}
"<"  {DealPos; return LESS;}
"<=" {DealPos; return LESSEQUAL;}
"==" {DealPos; return EQUAL;}
"!=" {DealPos; return UNEQUAL;}

"=" {DealPos; return ASSIGN;}
";" {DealPos; return SEMICOLON;}
"," {DealPos; return COMMA;}

" "|\t {DealPos;}
\n|\r {pos_end=1; lines++;}

.   {/*print;*/ DealPos; return ERROR;}

 /****请在此补全所有flex的模式与动作  end******/
%%
