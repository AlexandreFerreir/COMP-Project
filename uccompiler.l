/*Joao Tinoco-2021223708, Alexandre Ferreira-2021236702 */
%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "y.tab.h"
#include "ast.h"
#include "semantics.h"
#include "codegen.h"
char array[100];
#define YY_USER_ACTION { yylloc.first_line =syn_line = l ; yylloc.first_column =syn_column = c;}
#define PASS_TOKEN() yylval.token = strdup(yytext);
#define PASS_TOKEN_CHAR() yylval.token = strdup(array);

int yyparse(void);
void yyerror(char *);
int syn_line = 1, syn_column = 1;
int errors = 0;

struct table *table;
extern struct node *program;
extern int semantic_errors;


int l = 1,c=1,indice=0;
int finished_com=1;
int last_com=0;
int last_com_l=0;
int escape_seq=0;
int char_num_seq=0;
int indice_erro=0;
int count_char;

int check_barra_n=0;
int flag_l=0;
int flagt=0;
int flags=0;

void toUpper(char * string){
    for(int i=0;i<strlen(string);i++){
        string[i]=toupper(string[i]);
    }
}
void calcArray(int indice,char * array, char * text){
    int i;
    for(i=0;i<strlen(text);i++){
        array[indice+i]=text[i];
    }
}
void reformChar(char * string){
    char new[strlen(string)+3];
    new[0]='\'';
    for(int i=1;i<strlen(string)+1;i++){
        new[i]=string[i-1];
    }
    new[strlen(string)+1]='\'';
    new[strlen(string)+2]='\0';
    strcpy(string,new);
}

extern struct node *node;

%}

%option noyywrap

digit   [0-9]
alpha   [a-zA-Z_]
%X COMMENT
%X COMMENT2BARRA
%X STRING
%X CHARLIT
%%
"/*" { BEGIN(COMMENT); last_com=c;last_com_l=l;c+=yyleng;finished_com=0;}
<COMMENT>. { c+=yyleng; }
<COMMENT>\n { l+=1;c=1; }
<COMMENT>"*/" { c+=yyleng;finished_com=1; BEGIN(INITIAL);}

"//" { BEGIN(COMMENT2BARRA); c+=yyleng;}
<COMMENT2BARRA>. { c+=yyleng; }
<COMMENT2BARRA>\n { l+=1;c=1;BEGIN(INITIAL); }

\' { BEGIN(CHARLIT); c+=yyleng;indice=0;escape_seq=0;char_num_seq=-1;check_barra_n=1;}
<CHARLIT>\' {c+=yyleng;array[indice]='\0';
        if(char_num_seq==strlen(array) || (strlen(array)==1 || (strlen(array)==2 && escape_seq))){reformChar(array);PASS_TOKEN_CHAR();if(flag_l){printf("CHRLIT(\'%s\')\n",array);}BEGIN(INITIAL);return CHRLIT;}
        else{printf("Line %d, column %d: invalid char constant (\'%s\')\n",l,c-indice-check_barra_n,array);}BEGIN(INITIAL);}
<CHARLIT>\\[0-7][0-7]?[0-7]?  {calcArray(indice,array,yytext);indice+=yyleng;c+=yyleng;char_num_seq=yyleng;}        
<CHARLIT>\\['nt\"\\] {array[indice]=yytext[0];indice++;array[indice]=yytext[1];indice++;c+=yyleng;escape_seq=1;}
<CHARLIT>\\. {array[indice]=yytext[0];indice++;array[indice]=yytext[1];indice++;c+=yyleng;}
<CHARLIT>\n {printf("Line %d, column %d: unterminated char constant\n",l,c-indice-check_barra_n);l++;c=1;check_barra_n=0;BEGIN(INITIAL);}
<CHARLIT>. {array[indice]=yytext[0];indice++;c+=yyleng;}

 
auto|"["|"]"|"++"|"--"|break|case|const|continue|default|do|long|register|signed|sizeof|static|struct|switch|typedef|union|unsigned|volatile|enum|extern|for|float|goto|restrict|_Bool|_Complex|_Imaginary|inline   {c+=yyleng;if(flag_l){printf("RESERVED(%s)\n", yytext);}else{return RESERVED;}}
void       { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return VOID;}
return     { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return RETURN;}
short      { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return SHORT;}
int        { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return INT;}
while      { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return WHILE;}
char       { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return CHAR; }
else       { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return ELSE;}
if         { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return IF;}
double     { if(flag_l){toUpper(yytext);printf("%s\n",yytext);}c+=yyleng; return DOUBLE;}
"&"        { if(flag_l){printf("BITWISEAND\n");}c+=yyleng; return BITWISEAND; }
"|"        { if(flag_l){printf("BITWISEOR\n");}c+=yyleng;return BITWISEOR; }
"^"        { if(flag_l){printf("BITWISEXOR\n");} c+=yyleng;return BITWISEXOR;}
"&&"       { if(flag_l){printf("AND\n");}c+=yyleng;return AND; }
"="        { if(flag_l){printf("ASSIGN\n");} c+=yyleng;return ASSIGN;}
"*"        { if(flag_l){printf("MUL\n");} c+=yyleng;return MUL;}
","        { if(flag_l){printf("COMMA\n");} c+=yyleng;return COMMA;}
"/"        { if(flag_l){printf("DIV\n");}c+=yyleng;return DIV; }
"=="       { if(flag_l){printf("EQ\n");}c+=yyleng;return EQ; }
">="       { if(flag_l){printf("GE\n");} c+=yyleng;return GE;}
">"        { if(flag_l){printf("GT\n");} c+=yyleng;return GT;}
"{"        { if(flag_l){printf("LBRACE\n");} c+=yyleng;return LBRACE;}
"<="       { if(flag_l){printf("LE\n");} c+=yyleng;return LE;}
"("        { if(flag_l){printf("LPAR\n");} c+=yyleng;return LPAR;}
"<"        { if(flag_l){printf("LT\n");}c+=yyleng;return LT; }
"-"        { if(flag_l){printf("MINUS\n");} c+=yyleng;return MINUS;}
"%"        { if(flag_l){printf("MOD\n");}c+=yyleng;return MOD; }
"!="       { if(flag_l){printf("NE\n");}c+=yyleng;return NE; }
"!"        { if(flag_l){printf("NOT\n");} c+=yyleng;return NOT;}
"||"       { if(flag_l){printf("OR\n");} c+=yyleng;return OR;}
"+"        { if(flag_l){printf("PLUS\n");} c+=yyleng;return PLUS;}
"}"        { if(flag_l){printf("RBRACE\n");}c+=yyleng;return RBRACE; }
")"        { if(flag_l){printf("RPAR\n");} c+=yyleng;return RPAR;}
";"        { if(flag_l){printf("SEMI\n");} c+=yyleng;return SEMI;}

{digit}*"."{digit}+     { if(flag_l){printf("DECIMAL(%s)\n",yytext);}c+=yyleng; PASS_TOKEN(); return DECIMAL;}
{digit}+"."     { if(flag_l){printf("DECIMAL(%s)\n",yytext);}c+=yyleng; PASS_TOKEN(); return DECIMAL;}
{digit}*"."{digit}+([eE][+-]?{digit}+)?    { if(flag_l){printf("DECIMAL(%s)\n",yytext);}c+=yyleng; PASS_TOKEN(); return DECIMAL;}
{digit}+"."([eE][+-]?{digit}+)?    { if(flag_l){printf("DECIMAL(%s)\n",yytext);}c+=yyleng; PASS_TOKEN(); return DECIMAL;}
{digit}+([eE][+-]?{digit}+)    { if(flag_l){printf("DECIMAL(%s)\n",yytext);}c+=yyleng; PASS_TOKEN(); return DECIMAL;}
{digit}+                { if(flag_l){printf("NATURAL(%s)\n",yytext);} c+=yyleng; PASS_TOKEN(); return NATURAL;}
{alpha}+({digit}|{alpha})*       { if(flag_l){printf("IDENTIFIER(%s)\n",yytext);}c+=yyleng; PASS_TOKEN(); return IDENTIFIER;}
" "                       {c+=1;syn_line = l ; syn_column = c;}
"\t"                       {c+=yyleng;}
"\n"                      {l+=1;c=1;check_barra_n=0;syn_line = l ; syn_column = c;}
"\r\n"                      {l+=1;c=1;check_barra_n=0;syn_line = l ; syn_column = c;}
"\r"                      {l+=1;c=1;check_barra_n=0;}
.                       { printf("Line %d, column %d: unrecognized character (%s)\n",l,c,yytext);c+=yyleng; }
<<EOF>>                     { return 0; }



%%
//extern int yylex();

void yyerror(char *error) {
    errors++;
    printf("Line %d, column %d: %s: %s\n",syn_line, syn_column, error, yytext );
    
}

void locate_node(struct node * node){
    node->line=syn_line;
    node->column=syn_column;
}

int main(int argc, char * argv[]) {
    if (argc > 1 && strcmp(argv[1], "-l") == 0){
        flag_l=1;
    }else if (argc > 1 && strcmp(argv[1], "-t") == 0){
        flagt=1;
    }else if (argc > 1 && strcmp(argv[1], "-s") == 0){
        flags=1;
    }

    yyparse();    /* run the lexical analysis automaton */

    table=(struct table *)malloc(sizeof(struct table));
    table->identifier="";
    table->next=NULL;
    table->symbols=(struct symbol *)malloc(sizeof(struct symbol));
    table->symbols->next=NULL;
    struct symbol * putchar=insert_symbol(table,"putchar",Int,0,NULL);
    struct symbol * getchar=insert_symbol(table,"getchar",Int,0,NULL);
    struct node * paramList_getchar=newnode(ParamList,NULL);
    struct node * paramList_putchar=newnode(ParamList,NULL);
    struct node * declaration_getchar=newnode(ParamList,NULL);
    struct node * declaration_putchar=newnode(ParamList,NULL);  
    addchild(declaration_getchar,newnode(Void,NULL));
    addchild(declaration_putchar,newnode(Int,NULL));
    addchild(paramList_getchar,declaration_getchar);
    addchild(paramList_putchar,declaration_putchar);
    putchar->paramList=paramList_putchar;
    getchar->paramList=paramList_getchar;
    check_program(program,table,table);


    if(flags){
        print_symbol_table(table);
        print_tree_with_annotations(program,0);
    }
    if(flagt && errors==0){
        print_tree(program,0);
    }
    
    yylex();
    if(!finished_com){
        printf("Line %d, column %d: unterminated comment\n",last_com_l,last_com);c+=yyleng;
    }

    //if(argc <= 1 && errors == 0) {
    if(argc <= 1 && semantic_errors+errors==0) {
        //errors += check_program(program);
        //if(errors == 0)
        codegen_program(program);
    }else{
        //printf("virus dos jogos:%d\n",semantic_errors);
    }
    free_table(table);
    deallocate(program);
    yylex_destroy();
    return 0;
}
