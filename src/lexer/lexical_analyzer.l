%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "lexical_analyzer.h"

int lines;
int pos_start;
int pos_end;

/*****************声明和选项设置  end*****************/

%}

letter [a-zA-Z]
digit [0-9]

%s COMMENT

%%

 /******************TODO*********************/
 /****请在此补全所有flex的模式与动作  start******/
 //STUDENT TO DO
<INITIAL>\+ {return ADD;}
<INITIAL>\- {return SUB;}
<INITIAL>\* {return MUL;}
<INITIAL>\/ {return DIV;}
<INITIAL>\< {return LT;}
<INITIAL>"<=" {return LTE;}
<INITIAL>\> {return GT;}
<INITIAL>">=" {return GTE;}
<INITIAL>"==" {return EQ;}
<INITIAL>"!=" {return NEQ;}
<INITIAL>"=" {return ASSIN;}
<INITIAL>";" {return SEMICOLON;}
<INITIAL>"," {return COMMA;}
<INITIAL>"(" {return LPARENTHESE;}
<INITIAL>")" {return RPARENTHESE;}
<INITIAL>"[" {return LBRACKET;}
<INITIAL>"]" {return RBRACKET;}
<INITIAL>"{" {return LBRACE;}
<INITIAL>"}" {return RBRACE;}
<INITIAL>"[]" {return ARRAY;}
<INITIAL>{letter}+ {
    if(!strcmp("else", yytext)) return ELSE;
    if(!strcmp("if", yytext)) return IF;
    if(!strcmp("int", yytext)) return INT;
    if(!strcmp("float", yytext)) return FLOAT;
    if(!strcmp("return", yytext)) return RETURN;
    if(!strcmp("void", yytext)) return VOID;
    if(!strcmp("while", yytext)) return WHILE;
    return IDENTIFIER;
    }
<INITIAL>{digit}+ {return INTEGER;}
<INITIAL>({digit}+\.{digit}*)|({digit}*\.{digit}+) {return FLOATPOINT;}
<INITIAL>"/*" {pos_start+=2;BEGIN COMMENT;}
<COMMENT>"*/" {pos_start+=2;BEGIN INITIAL;}
<COMMENT>\n {lines++;pos_start=1;pos_end=1;}
<COMMENT>. {pos_start++;}
<INITIAL>[\f\r\t\v ] {return BLANK;}
<INITIAL>\n {return EOL;}
<INITIAL>. {return ERROR;}
 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token;
    int index = 0;

    while(token = yylex()){
        switch(token){
            case COMMENT:
                break;
            case BLANK:
                pos_start++;
                break;
            case EOL:
                lines++;
                pos_start = 1;
                pos_end = 1;
                break;
            case ERROR:
                pos_end=pos_start+1;
                printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
            default :
                if (token == ERROR){
                    sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines, pos_start, pos_end);
                } else {
                    strcpy(token_stream[index].text, yytext);
                }
                token_stream[index].token = token;
                token_stream[index].lines = lines;
                token_stream[index].pos_start = pos_start;
                pos_end=pos_start+yyleng;
                token_stream[index].pos_end = pos_end;
                pos_start=pos_end;
                index++;
                if (index >= MAX_NUM_TOKEN_NODE){
                    printf("%s has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
                    exit(1);
                }
        }
    }
    printf("[END]: Analysis completed.\n");
    return;
}



/****************C代码 end*************/
