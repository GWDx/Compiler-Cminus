%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
    yylval.node = new_syntax_tree_node(text);
}

#define print printf(">> %s\n", yytext);

#define DealPosPass pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext);

/*****************声明和选项设置  end*****************/

%}


%%
 /* to do for students */
 /* two cases for you, pass_node will send flex's token to bison */

 // 注释
\/\*([^\*]|\*[^\/])*\*+\/   {
    for(int i=0;yytext[i];i++){
        if(yytext[i]=='\n' || yytext[i]=='\r'){pos_end=0; lines++;}
        else pos_end++;
    }
}

[a-zA-Z]+ {
    DealPosPass;
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
    DealPosPass;
    return FLOATPOINT;
}

[0-9]+  {
    DealPosPass;
    return INTEGER;
}

"+" {DealPosPass; return ADD;}
"-" {DealPosPass; return SUBTRACT;}
"*" {DealPosPass; return MULTIPLY;}
"/" {DealPosPass; return DIVIDE;}

"(" {DealPosPass; return LPAREN;}
")" {DealPosPass; return RPAREN;}
"[" {DealPosPass; return LBRACKET;}
"]" {DealPosPass; return RBRACKET;}
"{" {DealPosPass; return LBRACE;}
"}" {DealPosPass; return RBRACE;}

">=" {DealPosPass; return GREATEREQUAL;}
"<=" {DealPosPass; return LESSEQUAL;}
">"  {DealPosPass; return GREATER;}
"<"  {DealPosPass; return LESS;}
"==" {DealPosPass; return EQUAL;}
"!=" {DealPosPass; return UNEQUAL;}

"=" {DealPosPass; return ASSIGN;}
";" {DealPosPass; return SEMICOLON;}
"," {DealPosPass; return COMMA;}

" "|\t {pos_end++;}
\n|\r {pos_end=0; lines++;}

.   {/*print;*/ DealPosPass; return ERROR;}

 /****请在此补全所有flex的模式与动作  end******/
%%
