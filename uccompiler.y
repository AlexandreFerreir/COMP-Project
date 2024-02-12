/* START definitions section -- C code delimited by %{ ... %} and token declarations */

%{

#include "ast.h"
#include "semantics.h"
#include <stddef.h>

int yylex(void);
void yyerror(char *);

struct node *program;
struct node_list *temp;
enum category categoria;

extern void locate_node(struct node * node);

%}

%token INTEGER DOUBLE IF THEN ELSE VOID RETURN SHORT INT WHILE CHAR
%token<token> RESERVED IDENTIFIER NATURAL DECIMAL BITWISEAND  BITWISEXOR AND ASSIGN MUL COMMA DIV EQ GE GT LBRACE LE LPAR LT MINUS MOD NE NOT OR PLUS RBRACE RPAR SEMI BITWISEOR CHRLIT
%type<node>StatementList expressionCall StatementComErro Program expressionAux StatementAux Statement Declarator TypeSpec DeclarationAux Declaration ParameterDeclaration ParameterListAux ParameterList FunctionDeclaration DeclarationsAndStatements FunctionsAndDeclarations FunctionDefinition FunctionBody expression

%right ELSE
%left COMMA
%right ASSIGN
%left OR
%left AND
%left BITWISEOR
%left BITWISEXOR
%left BITWISEAND
%left EQ NE
%left LT LE GE GT 
%left PLUS MINUS
%left MUL DIV MOD
%right NOT
%left LPAR RPAR


%union{
    char *token;
    struct node *node;
    //struct node_list *node_list;
}

%locations

/* START grammar rules section -- BNF grammar */

%%
Program: FunctionsAndDeclarations             {$$=program=$1;$$->category=Program;}
        ;

FunctionsAndDeclarations:FunctionDefinition   {$$=newnode(Unknown,NULL);addchild($$,$1);}
                        | FunctionDeclaration  {$$=newnode(Unknown,NULL);addchild($$,$1);}
                        | Declaration  {$$=newnode(Unknown,NULL);if($1->category==Unknown){
                                assignBrothers($$,$1->children->next);
                        }else{
                                addchild($$,$1);}}
                        | FunctionDefinition  FunctionsAndDeclarations  {$$=newnode(Unknown,NULL);addchild($$,$1);if($2->category==Unknown){
                                assignBrothers($$,$2->children->next);
                        }else{
                                addchild($$,$2);}}
                        | FunctionDeclaration  FunctionsAndDeclarations {$$=newnode(Unknown,NULL);addchild($$,$1);if($2->category==Unknown){
                                assignBrothers($$,$2->children->next);
                        }else{
                                addchild($$,$2);}}
                        | Declaration  FunctionsAndDeclarations {$$=newnode(Unknown,NULL);if($1->category==Unknown){
                                assignBrothers($$,$1->children->next);
                        }else{
                                addchild($$,$1);}
                                if($2->category==Unknown){
                                assignBrothers($$,$2->children->next);
                        }else{
                                addchild($$,$2);}}
                        ;




FunctionDefinition: TypeSpec IDENTIFIER LPAR ParameterList RPAR FunctionBody {$$=newnode(FuncDefinition,NULL);addchild($$,$1);struct node * temp =newnode(Identifier,$2);temp->line=@2.first_line;temp->column=@2.first_column;addchild($$,temp);addchild($$,$4);addchild($$,$6);}
                    ;

FunctionBody: LBRACE DeclarationsAndStatements RBRACE  {if($2->children->next!=NULL && $2->children->next->node->category!=Null){$$=$2;$$->category=FuncBody;}else{$$=newnode(FuncBody,NULL);}}
            | LBRACE RBRACE                            {$$=newnode(FuncBody,NULL);}
            ;

DeclarationsAndStatements: Statement DeclarationsAndStatements   {$$=newnode(Unknown,NULL);
if($1!=NULL){
if($1->category==Unknown){
        assignBrothers($$,$1->children->next);}else{
        addchild($$,$1);}}
        if($2!=NULL){
        if($2->category==Unknown){
        assignBrothers($$,$2->children->next);}else{
        addchild($$,$2);}}}//addbro
                        | Declaration DeclarationsAndStatements  {$$=newnode(Unknown,NULL);if($1!=NULL){
                        if($1->category==Unknown){
                                assignBrothers($$,$1->children->next);
                        }else{
                                addchild($$,$1);}}if($2!=NULL){
        if($2->category==Unknown){
        assignBrothers($$,$2->children->next);}else{
        addchild($$,$2);}}}//addbro
                        |Statement                               {$$=newnode(Unknown,NULL);
                        if($1!=NULL){
                        if($1->category==Unknown){
                                assignBrothers($$,$1->children->next);
                        }else{
                                addchild($$,$1);}}}
                        |Declaration                             {
                                $$=newnode(Unknown,NULL);if($1!=NULL){
                        if($1->category==Unknown){
                                assignBrothers($$,$1->children->next);
                        }else{
                                addchild($$,$1);}}}
                        ;

FunctionDeclaration: TypeSpec IDENTIFIER LPAR ParameterList RPAR SEMI             {$$=newnode(FuncDeclaration,NULL);addchild($$,$1);struct node * temp =newnode(Identifier,$2);addchild($$,temp);addchild($$,$4);temp->line=@2.first_line;temp->column=@2.first_column;}
                    ;

/* FunctionDeclarator: IDENTIFIER LPAR ParameterList RPAR             {$$=newnode(Unknown,NULL);addchild($$,newnode(Identifier,$1));addchild($$,$3);}//nao conta
                  ; */

ParameterList: ParameterDeclaration                              {$$=newnode(ParamList,NULL);addchild($$,$1);}
             | ParameterDeclaration ParameterListAux            {$$=newnode(ParamList,NULL);addchild($$,$1);if($2->category==Unknown){
                                assignBrothers($$,$2->children->next);
                        }else{
                                addchild($$,$2);}}
             ;

ParameterListAux: COMMA ParameterDeclaration                       {$$=$2;}
                | ParameterListAux COMMA ParameterDeclaration      {$$=newnode(Unknown,NULL);addchild($$,$1);addchild($$,$3);}//Possivel erro
                ;

ParameterDeclaration:TypeSpec                                   {$$=newnode(ParamDeclaration,NULL);addchild($$,$1);$1->line=@1.first_line;$1->column=@1.first_column;}
                    |TypeSpec IDENTIFIER                        {$$=newnode(ParamDeclaration,NULL);addchild($$,$1);struct node * temp=newnode(Identifier,$2);addchild($$,temp);$1->line=@1.first_line;$1->column=@1.first_column;temp->line=@2.first_line;temp->column=@2.first_column;}
                    ;
Declaration:TypeSpec Declarator SEMI                             {$$=$2;$2->children->next->node->category=$1->category;}
            | TypeSpec Declarator DeclarationAux SEMI             {$$=newnode(Unknown,NULL);addchild($$,$2);$2->children->next->node->category=$1->category;if($3->category==Unknown){
                                assignBrothers($$,$3->children->next);
                        }else{
                                addchild($$,$3);}assignTypeSpec($$,$1->category);}
            |error SEMI                                           {$$=newnode(Error,NULL);}
            ;

Declarator: IDENTIFIER                      {$$=newnode(Declaration,NULL);addchild($$,newnode(Unknown,NULL));struct node * temp =newnode(Identifier,$1);addchild($$,temp);temp->line=@1.first_line;temp->column=@1.first_column;}
          | IDENTIFIER ASSIGN expression       {$$=newnode(Declaration,NULL);addchild($$,newnode(Unknown,NULL));struct node * temp =newnode(Identifier,$1);addchild($$,temp);addchild($$,$3);temp->line=@1.first_line;temp->column=@1.first_column;}
          ;            



DeclarationAux: COMMA Declarator                                   {$$=$2;}
              |DeclarationAux COMMA Declarator                     {$$=newnode(Unknown,NULL);addchild($$,$1);addchild($$,$3);}//Possivel erro
              ;

TypeSpec:CHAR                            {$$=newnode(Char,NULL);}
        |INT                            {$$=newnode(Int,NULL);}
        |VOID                            {$$=newnode(Void,NULL);}
        |SHORT                            {$$=newnode(Short,NULL);}
        |DOUBLE                            {$$=newnode(Double,NULL);} 
        ;



Statement: SEMI                              {$$=NULL;}
          | expressionAux SEMI                  {$$=$1;}
          | LBRACE  RBRACE                        {$$=NULL;}
          | LBRACE StatementAux RBRACE               {$$=$2;}//falta cÃ³digo
          |IF LPAR expressionAux RPAR StatementComErro  %prec ELSE  {$$=newnode(If,NULL);addchild($$,$3);if($5!=NULL){addchild($$,$5);}else{
          addchild($$,newnode(Null,NULL));}addchild($$,newnode(Null,NULL));}
          |IF LPAR expressionAux RPAR StatementComErro  ELSE StatementComErro     {
                $$=newnode(If,NULL);addchild($$,$3);
                if($5!=NULL){addchild($$,$5);}
                else{addchild($$,newnode(Null,NULL));}
                if($7!=NULL){addchild($$,$7);}else{addchild($$,newnode(Null,NULL));}}
          |WHILE LPAR expressionAux RPAR StatementComErro               {$$=newnode(While,NULL);addchild($$,$3);if($5!=NULL){addchild($$,$5);}else{addchild($$,newnode(Null,NULL));}}
          | RETURN SEMI                                      {$$=newnode(Return,NULL);addchild($$,newnode(Null,NULL));$$->line=@1.first_line;$$->column=@1.first_column;}
          | RETURN expressionAux SEMI                            {$$=newnode(Return,NULL);addchild($$,$2);}
          | LBRACE error RBRACE                                 {$$=newnode(Error,NULL);}
          ;
          
StatementComErro: Statement     {$$=$1;}
                |error SEMI   {$$=newnode(Error,NULL);}
                ;

StatementAux:StatementComErro                      {$$=$1;}
            | StatementList StatementComErro        {if($1!=NULL){$$=newnode(Unknown,NULL);if($1!=NULL){if($1->category==Unknown){
                                assignBrothers($$,$1->children->next);
                        }else{
                                addchild($$,$1);}}
                                if($$!=NULL){
                                if(checkChildren($$)==0 && $2==NULL){
                                        $$->category=Null;}
                                        else if(checkChildren($$)==0 && $2!=NULL){
                                                $$=$2;}
                                                else if(checkChildren($$)==1 && $2==NULL){
                                                        $$=returnCheckChildren($$);
                                                }
                                                else {$$->category=StatList;if($2!=NULL){addchild($$,$2);}}}
                                else{if($2!=NULL){
                                        $$=$2;
                                }else{
                                        $$=newnode(Null,NULL);
                                }}}else{$$=$2;}}//DÃšVIDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
            ;

StatementList:StatementComErro                      {$$=$1;}
            | StatementList StatementComErro        {$$=newnode(Unknown,NULL);if($1!=NULL){addchild($$,$1);}if($2!=NULL){addchild($$,$2);}}//DÃšVIDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
            ;

expression:  expression ASSIGN expression          {$$=newnode(Store,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression PLUS expression         {$$=newnode(Add,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression MINUS expression         {$$=newnode(Sub,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression MUL expression         {$$=newnode(Mul,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression DIV expression         {$$=newnode(Div,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression MOD expression         {$$=newnode(Mod,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression BITWISEAND expression         {$$=newnode(BitWiseAnd,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression OR expression         {$$=newnode(Or,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression AND expression         {$$=newnode(And,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression BITWISEXOR expression         {$$=newnode(BitWiseXor,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression BITWISEOR expression        {$$=newnode(BitWiseOr,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression EQ expression         {$$=newnode(Eq,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression NE expression         {$$=newnode(Ne,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression LE expression         {$$=newnode(Le,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression GE expression         {$$=newnode(Ge,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression LT expression         {$$=newnode(Lt,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            | expression GT expression         {$$=newnode(Gt,NULL); addchild($$,$1); addchild($$,$3);$$->do_annotation=1;$$->line=@2.first_line;$$->column=@2.first_column;}
            |PLUS expression  %prec NOT                 {$$=newnode(Plus,NULL);addchild($$,$2);$$->do_annotation=1;$$->line=@1.first_line;$$->column=@1.first_column;}
            |MINUS expression   %prec NOT                {$$=newnode(Minus,NULL);addchild($$,$2);$$->do_annotation=1;$$->line=@1.first_line;$$->column=@1.first_column;}
            |NOT expression                    {$$=newnode(Not,NULL);addchild($$,$2);$$->do_annotation=1;$$->line=@1.first_line;$$->column=@1.first_column;}
            |IDENTIFIER LPAR RPAR              {$$=newnode(Call,NULL);$$->line=@1.first_line;$$->column=@1.first_column;struct node * temp =newnode(Identifier,$1);temp->line=@1.first_line;temp->column=@1.first_column; addchild($$,temp);$$->do_annotation=1;temp->do_annotation=1;}
            |IDENTIFIER LPAR expressionCall RPAR   {$$=newnode(Call,NULL);$$->line=@1.first_line;$$->column=@1.first_column;struct node * temp =newnode(Identifier,$1);temp->line=@1.first_line;temp->column=@1.first_column;addchild($$,temp);$$->do_annotation=1;temp->do_annotation=1;
            if($3->category==Unknown){
        assignBrothers($$,$3->children->next);
        }else{
        addchild($$,$3);}
            }
            |IDENTIFIER                            {$$=newnode(Identifier,$1);$$->do_annotation=1;$$->line=@1.first_line;$$->column=@1.first_column;}
            |NATURAL                            {$$=newnode(Natural,$1);$$->annotation=Int;$$->line=@1.first_line;$$->column=@1.first_column;}
            |CHRLIT                            {$$=newnode(ChrLit,$1);$$->annotation=Int;$$->line=@1.first_line;$$->column=@1.first_column;}
            |DECIMAL                            {$$=newnode(Decimal,$1);$$->annotation=Double;$$->line=@1.first_line;$$->column=@1.first_column;}
            |LPAR expressionAux RPAR                {$$ = $2;}
            |IDENTIFIER LPAR error RPAR          {$$=newnode(Error,NULL);}
            |LPAR error RPAR                     {$$=newnode(Error,NULL);}
            ;


expressionAux: expression                                       {$$=$1;}
              |expressionAux COMMA expression   {$$=newnode(Comma,NULL);$$->line=@2.first_line;$$->column=@2.first_column;addchild($$,$1);addchild($$,$3);}//Possivel erro
              ;

expressionCall: expression                                       {$$=$1;}
              |expressionCall COMMA expression   {$$=newnode(Unknown,NULL);addchild($$,$1);addchild($$,$3);}//DÃšVIDAAAAAAAAAAAAAAAAAAAAAAAAAAAA
              ;






%%



/* START subroutines section */


// all needed functions are collected in the .l and ast.* files