%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
%}

letter [a-zA-Z]
digit [0-9]

%s COMMENT

%%
 /*TO STUDENTS: Copy your Lab1 here. Make adjustments if necessary.*/
<INITIAL>\+ {pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD;}
<INITIAL>\- {pos_start = pos_end; pos_end += 1; pass_node(yytext);return SUB;}
<INITIAL>\* {pos_start = pos_end; pos_end += 1; pass_node(yytext);return MUL;}
<INITIAL>\/ {pos_start = pos_end; pos_end += 1; pass_node(yytext);return DIV;}
<INITIAL>\< {pos_start = pos_end; pos_end += 1; pass_node(yytext);return LT;}
<INITIAL>"<=" {pos_start = pos_end; pos_end += 2; pass_node(yytext);return LTE;}
<INITIAL>\> {pos_start = pos_end; pos_end += 1; pass_node(yytext);return GT;}
<INITIAL>">=" {pos_start = pos_end; pos_end += 2; pass_node(yytext);return GTE;}
<INITIAL>"==" {pos_start = pos_end; pos_end += 2; pass_node(yytext);return EQ;}
<INITIAL>"!=" {pos_start = pos_end; pos_end += 2; pass_node(yytext);return NEQ;}
<INITIAL>"=" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return ASSIN;}
<INITIAL>";" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return SEMICOLON;}
<INITIAL>"," {pos_start = pos_end; pos_end += 1; pass_node(yytext);return COMMA;}
<INITIAL>"(" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return LPARENTHESE;}
<INITIAL>")" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return RPARENTHESE;}
<INITIAL>"[" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return LBRACKET;}
<INITIAL>"]" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return RBRACKET;}
<INITIAL>"{" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return LBRACE;}
<INITIAL>"}" {pos_start = pos_end; pos_end += 1; pass_node(yytext);return RBRACE;}
<INITIAL>"[]" {pos_start = pos_end; pos_end += 2; pass_node(yytext);return ARRAY;}
<INITIAL>{letter}+ {
    if(!strcmp("else", yytext)) 
    {
         pos_start = pos_end; pos_end += 4; 
         pass_node(yytext);
         return ELSE;
    }
    if(!strcmp("if", yytext)) 
     {
          pos_start = pos_end; pos_end += 2; 
          pass_node(yytext);
          return IF;
     }
    if(!strcmp("int", yytext)) 
    {
         pos_start = pos_end; pos_end += 3; 
         pass_node(yytext);
         return INT;
    }
    if(!strcmp("float", yytext)) 
    {
         pos_start = pos_end; pos_end += 5; 
         pass_node(yytext);
         return FLOAT;
    }
    if(!strcmp("return", yytext)) 
    {
         pos_start = pos_end; pos_end += 6; 
         pass_node(yytext);
         return RETURN;
    }
    if(!strcmp("void", yytext)) 
    {
         pos_start = pos_end; pos_end += 4; 
         pass_node(yytext);
         return VOID;
    }
    if(!strcmp("while", yytext)) 
    {
         pos_start = pos_end; pos_end += 5; 
         pass_node(yytext);
         return WHILE;
    }
    if(!strcmp("main", yytext)) 
    {
         pos_start = pos_end; pos_end += 4; 
         pass_node(yytext);
         return MAIN;
    }
    pos_start = pos_end; pos_end += yyleng; 
    pass_node(yytext);
    return IDENTIFIER;
}
<INITIAL>{digit}+ {pos_start = pos_end; pos_end += yyleng; pass_node(yytext);return INTEGER;}
<INITIAL>({digit}+\.{digit}*)|({digit}*\.{digit}+) {pos_start = pos_end; pos_end += yyleng; pass_node(yytext);return FLOATPOINT;}
<INITIAL>"/*" {pos_start = pos_end; pos_end += 2; BEGIN COMMENT;}
<COMMENT>"*/" {pos_start = pos_end; pos_end += 2; BEGIN INITIAL;}
<COMMENT>\n {lines++;pos_start=1;pos_end=1;}
<COMMENT>. {pos_start = pos_end; pos_end += 1;}
<INITIAL>[\f\r\t\v ] {pos_start = pos_end; pos_end += yyleng;}
<INITIAL>\n {lines++;pos_start=1;pos_end=1;}
<INITIAL>. {pos_start = pos_end; pos_end += 1;return 0;}
 /*Note: don't modify the prologue unless you know what you are doing.*/
%%
