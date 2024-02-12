#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"
#include "codegen.h"

int codegen_expression(struct node *expression,struct table * func_table);

char * type_to_llvm(enum category type){
    switch (type){
        case Int:
            return "i32";
            break;

        case Double:
            return "double";
            break;

        case Char:
            return "i8";
            break;

        case Short:
            return "i16";
            break;
        case Void:
            return "void";
            break;

        default:
            return "UNKNOWN";
            break;
    }
}

int type_alignment(enum category type){
    switch (type){
        case Int:
            return 4;
            break;

        case Double:
            return 8;
            break;

        case Char:
            return 1;
            break;

        case Short:
            return 2;
            break;

        default:
            return -1;
            break;
    }
}

struct node * getchild(struct node * node, int son){
    struct node_list * temp=node->children->next;
    int counter=0;
    while(temp!=NULL){
        if(counter==son){
            return temp->node;
        }
        counter++;
        temp=temp->next;
    }
    return NULL;
}

char remove_plica(char * str){
    char * new=strdup(str);
    int i=0;
    new[i]='\\';
    for ( i = 0; i < strlen(new)-2; i++)
    {
        new[i+1]=str[i+1];

    }
    new[i+1]='\0';
    //printf("STRING:%s",new);
    char char_octal=(char)strtol(new + 2, NULL, 8);
    //sscanf(new, "\\%c", &char_octal);

    return char_octal;

}

char * initialized_value(struct node * node){
    char buffer[50];
    if(node==NULL){
        return "0";
    }
    switch(node->category){
        case Short:
        case Natural:
        case Int:
            //sprintf(buffer,"%d",atoi(node->token));
            return node->token;
            break;

        case ChrLit:

            if(node->token[1]=='\\'){
                sprintf(buffer,"%d",remove_plica(node->token));
            }else{
                sprintf(buffer,"%d",node->token[1]);
            }
            return strdup(buffer);
            break;

        case Decimal:
            // sprintf(buffer,"%lf",atof(node->token));
            return node->token;
            break;

        default:
            return NULL;
            break;
    }
}

int temporary;

enum category  sext_category;



int codegen_le(struct node * le){
    // codegen_expression(getchild(le, 0));
    // codegen_expression(getchild(le, 1));


    // printf(" %%%d = load %s, %%%s* %%%d, align %d\n",
    // type_to_llvm(getchild(le, 0)->annotation),initialized_value(getchild(store, 0)),type_to_llvm(getchild(store, 0)->annotation), -1, type_alignment(getchild(store, 0)->annotation));
    return 0;
}

char * get_globalvar(char * identifier){
    return search_symbol(table,identifier)->identifier;

}

int codegen_store(struct node *store,struct table * func_table){
    sext_category=getchild(store, 0)->annotation;
    int e1;
    // int e1=codegen_expression(getchild(store, 0),func_table);
    codegen_expression(getchild(store, 1),func_table);
    int flag1=0;
    char * global1;
    if(search_symbol(func_table,getchild(store, 0)->token)==NULL){
        global1=get_globalvar(getchild(store, 0)->token);
        flag1=1;
    }else{
        e1=search_symbol(func_table,getchild(store, 0)->token)->temporary;
    }
    // if(e2==-2){
    //     global2=get_globalvar(getchild(store, 1)->token);
    //     flag2=1;
    // }
    // if(flag2){
    //         printf("  %%%d = load %s, %s* @%s, align %d\n",temporary,type_to_llvm(getchild(store, 1)->annotation),
    //     type_to_llvm(getchild(store, 1)->annotation),global2,type_alignment(getchild(store, 1)->annotation));
    //     temporary++;
    // }else{
    //     printf("  %%%d = load %s, %s* %%%d, align %d\n",temporary,type_to_llvm(getchild(store, 1)->annotation),
    //     type_to_llvm(getchild(store, 1)->annotation),e2,type_alignment(getchild(store, 1)->annotation));
    //     temporary++;
    // }


    // if(getchild(store, 0)->annotation!=getchild(store, 1)->annotation){
    //     printf("  %%%d = sext %s %%%d to %s\n",temporary,type_to_llvm(getchild(store, 1)->annotation),
    //     temporary-1,type_to_llvm(getchild(store, 0)->annotation));
    //     temporary++;
    // }
    if(flag1){
        printf("  store %s %%%d, %s* @%s, align %d\n",
    type_to_llvm(getchild(store, 0)->annotation),temporary-1,type_to_llvm(getchild(store, 0)->annotation), global1, type_alignment(getchild(store, 0)->annotation));
    }else{
        printf("  store %s %%%d, %s* %%%d, align %d\n",
    type_to_llvm(getchild(store, 0)->annotation),temporary-1,type_to_llvm(getchild(store, 0)->annotation), e1, type_alignment(getchild(store, 0)->annotation));
    }
    return 0;
}



int codegen_add(struct node *add,struct table * func_table) {
    int e1 = codegen_expression(getchild(add, 0),func_table);
    int e2 = codegen_expression(getchild(add, 1),func_table);
    printf("  %%%d = add nsw %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}
int codegen_sub(struct node *sub,struct table * func_table) {
    int e1 = codegen_expression(getchild(sub, 0),func_table);
    int e2 = codegen_expression(getchild(sub, 1),func_table);
    printf("  %%%d = sub nsw %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category),e1,e2);
    //printf("  %%%d = fneg %s %%%d\n",temporary,type_to_llvm(minus->annotation),temporary-1);
    return temporary++;
}

int codegen_mul(struct node *mul,struct table * func_table) {
    int e1 = codegen_expression(getchild(mul, 0),func_table);
    int e2 = codegen_expression(getchild(mul, 1),func_table);
    printf("  %%%d = mul nsw %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}
int codegen_div(struct node *div,struct table * func_table) {
    int e1 = codegen_expression(getchild(div, 0),func_table);
    int e2 = codegen_expression(getchild(div, 1),func_table);
    printf("  %%%d = sdiv %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}

int codegen_bitwiseor(struct node *bitwiseor,struct table * func_table) {
    int e1 = codegen_expression(getchild(bitwiseor, 0),func_table);
    int e2 = codegen_expression(getchild(bitwiseor, 1),func_table);
    printf("  %%%d = or %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}

int codegen_bitwisexor(struct node *bitwisexor,struct table * func_table) {
    int e1 = codegen_expression(getchild(bitwisexor, 0),func_table);
    int e2 = codegen_expression(getchild(bitwisexor, 1),func_table);
    printf("  %%%d = xor %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}

int codegen_bitwiseand(struct node *bitwiseand,struct table * func_table) {
    int e1 = codegen_expression(getchild(bitwiseand, 0),func_table);
    int e2 = codegen_expression(getchild(bitwiseand, 1),func_table);
    printf("  %%%d = and %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}

int codegen_mod(struct node *mod,struct table * func_table) {
    int e1 = codegen_expression(getchild(mod, 0),func_table);
    int e2 = codegen_expression(getchild(mod, 1),func_table);
    printf("  %%%d = srem %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category), e1, e2);
    return temporary++;
}

int codegen_minus(struct node *minus,struct table * func_table) {
    //sext_category=getchild(minus, 0)->annotation;
    int e1 = codegen_expression(getchild(minus, 0),func_table);
    printf("  %%%d = sub nsw %s %d, %%%d\n", temporary,type_to_llvm(sext_category),0,e1);
    //printf("  %%%d = fneg %s %%%d\n",temporary,type_to_llvm(minus->annotation),temporary-1);
    return temporary++;
}

int codegen_plus(struct node *plus,struct table * func_table) {
    int e1 = codegen_expression(getchild(plus, 0),func_table);
    //printf("  %%%d = sub nsw %s %d, %%%d\n", temporary,type_to_llvm(sext_category),0,e1);
    //printf("  %%%d = fneg %s %%%d\n",temporary,type_to_llvm(minus->annotation),temporary-1);
    return e1;
}

int codegen_not(struct node *not,struct table * func_table) {
    int e1 = codegen_expression(getchild(not, 0),func_table);
    printf("  %%%d = icmp ne %s %%%d, %d\n", temporary,type_to_llvm(sext_category),e1,0);
    temporary++;
    printf("  %%%d = xor i1 %%%d, %s\n", temporary,temporary-1,"true");
    temporary++;
    printf("  %%%d = zext i1 %%%d to %s\n",temporary,temporary-1,type_to_llvm(sext_category));
    temporary++;
    return temporary;
}

int codegen_eq(struct node *eq,struct table * func_table) {
    int e1 = codegen_expression(getchild(eq, 0),func_table);
    int e2 = codegen_expression(getchild(eq, 1),func_table);
    printf("  %%%d = icmp eq %s %%%d, %%%d\n", temporary,type_to_llvm(sext_category),e1,e2);
    temporary++;
    printf("  %%%d = zext i1 %%%d to %s\n",temporary,temporary-1,type_to_llvm(sext_category));
    temporary++;
    return temporary;
}

int codegen_natural(struct node *natural) {
    printf("  %%%d = alloca %s, align %d\n",temporary,type_to_llvm(natural->annotation),type_alignment(natural->annotation));
    printf("  store %s %s, %s* %%%d, align %d\n",
            type_to_llvm(natural->annotation),
            natural->token,
            type_to_llvm(natural->annotation),
            temporary,type_alignment(natural->annotation));

    temporary++;

    printf("  %%%d = load %s, %s* %%%d, align %d\n",temporary,type_to_llvm(natural->annotation),
                        type_to_llvm(natural->annotation),temporary-1,type_alignment(natural->annotation));
    
    temporary++;

    if(natural->annotation!=sext_category){
                if(conversion_is_possible(natural->annotation,sext_category)){
                    printf("  %%%d = sext %s %%%d to %s\n",temporary,type_to_llvm(natural->annotation),temporary-1,type_to_llvm(sext_category));
                }else{
                    printf("  %%%d = trunc %s %%%d to %s\n",temporary,type_to_llvm(natural->annotation),temporary-1,type_to_llvm(sext_category));
                }
                temporary++;
                
            }

    return temporary-1;
}

int codegen_decimal(struct node *decimal) {
    printf("  %%%d = alloca %s, align %d\n",temporary,type_to_llvm(decimal->annotation),type_alignment(decimal->annotation));
    printf("  store %s %s, %s* %%%d, align %d\n",
            type_to_llvm(decimal->annotation),
            decimal->token,
            type_to_llvm(decimal->annotation),
            temporary,type_alignment(decimal->annotation));

    temporary++;

    printf("  %%%d = load %s, %s* %%%d, align %d\n",temporary,type_to_llvm(decimal->annotation),
                        type_to_llvm(decimal->annotation),temporary-1,type_alignment(decimal->annotation));
    
    temporary++;

    if(decimal->annotation!=sext_category){
                if(conversion_is_possible(decimal->annotation,sext_category)){
                    printf("  %%%d = sext %s %%%d to %s\n",temporary,type_to_llvm(decimal->annotation),temporary-1,type_to_llvm(sext_category));
                }else{
                    printf("  %%%d = trunc %s %%%d to %s\n",temporary,type_to_llvm(decimal->annotation),temporary-1,type_to_llvm(sext_category));
                }
                temporary++;
                
            }

    return temporary-1;
}

int codegen_identifier(struct node *identifier) {
    printf("  %%%d = add i32 %%%s, 0\n", temporary, identifier->token);
    return temporary++;
}

int codegen_chrlit(struct node *chrlit,struct table * func_table){
    printf("  %%%d = alloca %s, align %d\n",temporary,type_to_llvm(Char),type_alignment(Char));
    printf("  store %s %s, %s* %%%d, align %d\n",
            type_to_llvm(Char),
            initialized_value(chrlit),
            type_to_llvm(Char),
            temporary,type_alignment(Char));
    
    temporary++;

    printf("  %%%d = load %s, %s* %%%d, align %d\n",temporary,type_to_llvm(Char),
                        type_to_llvm(Char),temporary-1,type_alignment(Char));
    
    temporary++;

    if(Char!=sext_category){
                if(conversion_is_possible(Char,sext_category)){
                    printf("  %%%d = sext %s %%%d to %s\n",temporary,type_to_llvm(Char),temporary-1,type_to_llvm(sext_category));
                }else{
                    printf("  %%%d = trunc %s %%%d to %s\n",temporary,type_to_llvm(Char),temporary-1,type_to_llvm(sext_category));
                }
                temporary++;
                
            }

    return temporary-1;
}

int codegen_call(struct node *call,struct table * func_table) {

    if(!strcmp(getchild(call,0)->token,"putchar")){
        sext_category=Int;
        int e1=codegen_expression(getchild(call,1),func_table);
        printf("  %%%d = call i32 @putchar(i32 %%%d)\n",temporary,e1);
        temporary++;
    }


    // struct node *arguments = getchild(call, 1);
    // char *arguments_str = malloc(1);
    // *arguments_str = '\0';
    // struct node *argument;
    // int curr = 0;
    // while((argument = getchild(arguments, curr++)) != NULL) {
    //     int e = codegen_expression(argument);
    //     char str[18];
    //     if(curr > 1)
    //         sprintf(str, ", i32 %%%d", e);
    //     else
    //         sprintf(str, "i32 %%%d", e);
    //     arguments_str = realloc(arguments_str, strlen(arguments_str) + strlen(str) + 1);
    //     strcat(arguments_str, str);
    // }
    // printf("  %%%d = tail call i32 @_%s(%s)\n", temporary, getchild(call, 0)->token, arguments_str);



    return temporary;
}

int codegen_ifthenelse(struct node *ifthenelse) {
    // int label_id = temporary++;
    // printf("  %%%d = alloca i32\n", label_id);
    // int e = codegen_expression(getchild(ifthenelse, 0));
    // printf("  %%%d = icmp ne i32 %%%d, 0\n", temporary, e);
    // printf("  br i1 %%%d, label %%L%dthen, label %%L%delse\n", temporary++, label_id, label_id);
    // printf("L%dthen:\n", label_id);
    // int e1 = codegen_expression(getchild(ifthenelse, 1));
    // printf("  store i32 %%%d, i32* %%%d\n", e1, label_id);
    // printf("  br label %%L%dend\n", label_id);
    // printf("L%delse:\n", label_id);
    // int e2 = codegen_expression(getchild(ifthenelse, 2));
    // printf("  store i32 %%%d, i32* %%%d\n", e2, label_id);
    // printf("  br label %%L%dend\n", label_id);
    // printf("L%dend:\n", label_id);
    // printf("  %%%d = load i32, i32* %%%d\n", temporary, label_id);
    return temporary++;
}


void codegen_ifthenelse_ret(struct node *ifthenelse) {
    // int e = codegen_expression(getchild(ifthenelse, 0));
    // int label_id = temporary;
    // printf("  %%%d = icmp ne i32 %%%d, 0\n", temporary, e);
    // printf("  br i1 %%%d, label %%L%dthen, label %%L%delse\n", temporary++, label_id, label_id);
    // printf("L%dthen:\n", label_id);
    // if(getchild(ifthenelse, 1)->category == If) {
    //     codegen_ifthenelse_ret(getchild(ifthenelse, 1));
    // } else {
    //     int e1 = codegen_expression(getchild(ifthenelse, 1));
    //     printf("  ret i32 %%%d\n", e1);
    // }
    // printf("L%delse:\n", label_id);
    // if(getchild(ifthenelse, 2)->category == If) {
    //     codegen_ifthenelse_ret(getchild(ifthenelse, 2));
    // } else {
    //     int e2 = codegen_expression(getchild(ifthenelse, 2));
    //     printf("  ret i32 %%%d\n", e2);
    // }
}

int conversion_is_possible(enum category type1,enum category type2){
    if(type1==Double && type2!=Double){
        return 0;
    }
    if(type1==Int && type2!=Double && type2!=Int){
        return 0;
    }
    if(type1==Short && type2!=Double && type2!=Int && type2!=Short){
        return 0;
    }
    return 1;
}

int codegen_expression(struct node *expression,struct table * func_table) {
    int tmp = -1;
    struct symbol * symbol_temp;
    switch(expression->category) {
        case Identifier:
            if(search_symbol(func_table,expression->token)!=NULL){
                symbol_temp = search_symbol(func_table,expression->token);
                tmp=symbol_temp->temporary;
            }else{
                symbol_temp=search_symbol(table,expression->token);
                tmp=-2;
            }
            if(tmp==-2){
                printf("  %%%d = load %s, %s* @%s, align %d\n",temporary,type_to_llvm(symbol_temp->type),
                        type_to_llvm(symbol_temp->type),symbol_temp->identifier,type_alignment(symbol_temp->type));
            }else{
                printf("  %%%d = load %s, %s* %%%d, align %d\n",temporary,type_to_llvm(symbol_temp->type),
                        type_to_llvm(symbol_temp->type),symbol_temp->temporary,type_alignment(symbol_temp->type));
            }
            temporary++;
            if(symbol_temp->type!=sext_category){
                if(conversion_is_possible(symbol_temp->type,sext_category)){
                    printf("  %%%d = sext %s %%%d to %s\n",temporary,type_to_llvm(symbol_temp->type),temporary-1,type_to_llvm(sext_category));
                }else{
                    printf("  %%%d = trunc %s %%%d to %s\n",temporary,type_to_llvm(symbol_temp->type),temporary-1,type_to_llvm(sext_category));
                }
                temporary++;
            }
            tmp=temporary-1;
            break;
        case Natural:
            tmp = codegen_natural(expression);
            break;
        case Decimal:
            tmp = codegen_decimal(expression);
            break;
        case If:
            tmp = codegen_ifthenelse(expression);
            break;
        case Add:
            tmp = codegen_add(expression,func_table);
            break;
        case Sub:
            tmp = codegen_sub(expression,func_table);
            break;
        case Mul:
            tmp = codegen_mul(expression,func_table);
            break;
        case Div:
            tmp = codegen_div(expression,func_table);
            break;
        case Mod:
            tmp = codegen_mod(expression,func_table);
            break;
        case Store:
            tmp=codegen_store(expression,func_table);
            break;
        case Minus:
            tmp=codegen_minus(expression,func_table);
            break;
        case Plus:
            tmp=codegen_plus(expression,func_table);
            break;
        case Not:
            tmp=codegen_not(expression,func_table);
            break;
        case BitWiseOr:
            tmp=codegen_bitwiseor(expression,func_table);
            break;
        case BitWiseXor:
            tmp=codegen_bitwisexor(expression,func_table);
            break;
        case BitWiseAnd:
            tmp=codegen_bitwiseand(expression,func_table);
            break;
        case Eq:
            tmp=codegen_eq(expression,func_table);
            break;
        case Call:
            tmp=codegen_call(expression,func_table);
            break;
        case ChrLit:
            tmp=codegen_chrlit(expression,func_table);
            break;
        default:
            break;
    }
    return tmp;
}



void codegen_globaldeclaration(struct node * declaration){

    printf("@%s = dso_local global %s %s, align %d\n",
    getchild(declaration,1)->token,type_to_llvm(getchild(declaration,0)->category),initialized_value(getchild(declaration,2)),type_alignment(getchild(declaration,0)->category));

}

void codegen_localdeclaration(struct node * declaration,struct table * func_table){
    // int expr;
    int temp;
    printf("  %%%d = alloca %s, align %d\n",temporary++,type_to_llvm(getchild(declaration,0)->category),type_alignment(getchild(declaration,0)->category));
    search_symbol(func_table,getchild(declaration,1)->token)->temporary=temporary-1;
    temp=temporary-1;
    if(getchild(declaration,2)!=NULL){
        if(initialized_value(getchild(declaration,2))!=NULL){
            printf("  store %s %s, %s* %%%d, align %d\n",
            type_to_llvm(getchild(declaration,0)->category),
            initialized_value(getchild(declaration,2)),
            type_to_llvm(getchild(declaration,0)->category),
            temporary-1,type_alignment(getchild(declaration,0)->category));
        }else{
            sext_category=getchild(declaration,0)->category;
            codegen_expression(getchild(declaration,2),func_table);
            printf("  store %s %%%d, %s* %%%d, align %d\n",
            type_to_llvm(getchild(declaration,0)->category),
            temporary-1,
            type_to_llvm(getchild(declaration,0)->category),
            temp,type_alignment(getchild(declaration,0)->category));
        }
    }else{
        printf("  store %s %d, %s* %%%d, align %d\n",
        type_to_llvm(getchild(declaration,0)->category),
        0,
        type_to_llvm(getchild(declaration,0)->category),
        temporary-1,type_alignment(getchild(declaration,0)->category));


    }

}

void codegen_parameterdeclaration(struct node * declaration,struct table * func_table){
    printf("  %%%d = alloca %s, align %d\n",temporary++,type_to_llvm(getchild(declaration,0)->category),type_alignment(getchild(declaration,0)->category));


        printf("  store %s %%%d, %s* %%%d, align %d\n",
        type_to_llvm(getchild(declaration,0)->category),
        search_symbol(func_table,getchild(declaration,1)->token)->temporary-1,
        type_to_llvm(getchild(declaration,0)->category),
        temporary-1,type_alignment(getchild(declaration,0)->category));


}

void codegen_if(struct node * node_if){}

void codegen_while(struct node * node_while){
    printf("  br label %%%d\n\n",temporary);
    printf("%d:\n",temporary++);
    //printf("");


    //associar as impressões dos loads

}

void codegen_return(struct node * node_return,struct table * func_table){
    if(getchild(node_return,0)->category!=Null){
         if(func_table->symbols!=NULL){

             sext_category=func_table->symbols->next->type;
             int e1=codegen_expression(getchild(node_return,0),func_table);
             printf("  ret %s %%%d\n",type_to_llvm(sext_category),e1);
         }
     }else{
         if(func_table->symbols!=NULL){
         printf("  ret %s\n", to_lower(categoryToString(func_table->symbols->next->type)));
         }
     }

}

void codegen_statement(struct node * statement,struct table * func_table){
    switch (statement->category){
        case If:
            codegen_if(statement);
            break;
        case While:
            codegen_while(statement);
            break;
        case Return:
            codegen_return(statement,func_table);
            break;
        default:
            codegen_expression(statement,func_table);
            break;
    }
}



void codegen_funcbody(struct node * funcbody,struct table * func_table){
    struct node_list *function = funcbody->children;
    int return_done=0;
    while((function = function->next) != NULL){
        if(function->node->category==Return){
            return_done=1;
        }
        switch(function->node->category){
            case Declaration:
                codegen_localdeclaration(function->node,func_table);
                break;
            default:
                codegen_statement(function->node,func_table);
                break;
        }
    }

     if(!return_done){
        if(func_table->symbols->next->type==Void){
            printf("  ret void\n");
        }else{
            printf("  ret %s %d\n",type_to_llvm(func_table->symbols->next->type),0);
        }
         
     }
}

void codegen_parameters(struct node *parameters,struct table * func_table) {
    struct node *parameter;

    int curr = 0;
    if(getchild(getchild(parameters, 0),0)->category!=Void){
        //temporary=0;
        while((parameter = getchild(parameters, curr++)) != NULL) {
            if(curr > 1)
                printf(", ");
            //printf("type: %s",categoryToString(getchild(parameter, 1)->category));
            printf("%s %%%d",type_to_llvm(getchild(parameter, 0)->category), temporary-1);
            search_symbol(func_table,getchild(parameter, 1)->token)->temporary=temporary;
            temporary++;
        }
    }
    printf(") {\n");
    curr = 0;
    if(getchild(getchild(parameters, 0),0)->category!=Void){
        while((parameter = getchild(parameters, curr++)) != NULL) {
            codegen_parameterdeclaration(parameter,func_table);
            // printf("%s %%%d",type_to_llvm(getchild(parameter, 0)->category), temporary);
            // search_symbol(func_table,getchild(parameter, 1)->token)->temporary=temporary;
            // temporary++;
        }
    }
}

void codegen_functiondefinition(struct node *function) {
    struct table * local_table=search_table(table,getchild(function, 1)->token);
    temporary = 1;
    printf("\ndefine dso_local %s @%s(",type_to_llvm(getchild(function,0)->category), getchild(function, 1)->token);
    codegen_parameters(getchild(function, 2),local_table);
    if(getchild(function,0)->category!=Void){
        printf("  %%%d = alloca %s, align %d\n",temporary++,type_to_llvm(getchild(function,0)->category),type_alignment(getchild(function,0)->category));


        printf("  store %s %d, %s* %%%d, align %d\n",
        type_to_llvm(getchild(function,0)->category),
        0,
        type_to_llvm(getchild(function,0)->category),
        temporary-1,type_alignment(getchild(function,0)->category));
    }


    codegen_funcbody(getchild(function,3),local_table);



    //ALTERAR ESTE RETURN PARA DENTRO DA FUNCAO FUNC_BODY()
    // if(getchild(function,0)->category!=Void){
    //      printf("  ret %s %d\n", type_to_llvm(getchild(function,0)->category),0);
    // }else{
    //     printf("  ret %s\n", type_to_llvm(getchild(function,0)->category));
    // }

    printf("}\n\n");
}

// code generation begins here, with the AST root node
void codegen_program(struct node *program) {
    // predeclared functions
    printf("declare i32 @getchar(i32)\n");
    printf("declare i32 @putchar(i32)\n\n");

    // generate the code for each function
    struct node_list *function = program->children;
    while((function = function->next) != NULL)
        switch (function->node->category)
        {
        case FuncDeclaration:
            /* code */
            break;
        case FuncDefinition:
            codegen_functiondefinition(function->node);//maybe wrong
            break;
        case Declaration:
            codegen_globaldeclaration(function->node);
        default:
            break;
        }

}