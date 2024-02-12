#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"
#include <ctype.h>

void check_if(struct node *program,struct table *table,struct table *first_table);
void check_program_on_node(struct node_list *child,struct table *table,struct table *first_table);

int semantic_errors = 0;
extern int syn_line, syn_column;

struct symbol_list *symbol_table;

void free_table(struct table *table) {
    if (table == NULL) {
        return;
    }
    struct table *next_table;

    struct symbol *symbol = table->symbols;
    struct symbol *next_symbol;

    while (table != NULL) {
        while (symbol != NULL) {
            next_symbol = symbol->next;
            //free(symbol->paramList);
            free(symbol);
            symbol = next_symbol;
        }
        free(symbol);
        next_table = table->next;
        free(table);
        table = next_table;
    }
    free(table);
}

// void check_function(struct node *function) {
//     struct node *id = getchild(function, 0);
//     if(search_symbol(symbol_table, id->token) == NULL) {
//         insert_symbol(symbol_table, id->token, no_type, function);
//     } else {
//         printf("Identifier %s already declared\n", id->token);
//         semantic_errors++;
//     }
//     //check_parameters(getchild(function, 1));
//     //check_expression(getchild(function, 2));
// }

// semantic analysis begins here, with the AST root node


// insert a new symbol in the list, unless it is already there
struct symbol * insert_symbol(struct table *table, char *identifier, enum category type,int param,struct node * paramlist) {
    struct symbol *new = (struct symbol *) malloc(sizeof(struct symbol));
    new->identifier = strdup(identifier);
    new->type = type;
    new->param=param;
    new->temporary=0;
    new->paramList=paramlist;
    new->next = NULL;
    struct symbol *symbol = table->symbols;
    while(symbol != NULL) {
        if(symbol->next == NULL) {
            symbol->next = new;    /* insert new symbol at the tail of the list */
            break;
        } else if(strcmp(symbol->next->identifier, identifier) == 0) {
            free(new);
            return NULL;           /* return NULL if symbol is already inserted */
        }
        symbol = symbol->next;
    }
    return new;
}
struct table *add_table(struct table *table, char *identifier,enum category return_type){
    struct table *new = (struct table *) malloc(sizeof(struct table));
    struct table *temp = table;
    new->identifier=strdup(identifier);
    new->next=NULL;
    new->symbols=(struct symbol *) malloc(sizeof(struct symbol));
    new->symbols->next=NULL;
    insert_symbol(new,"return",return_type,0,NULL);
    while(temp!=NULL){
        if(strcmp(identifier,temp->identifier)==0){free(new);return temp;}
        if(temp->next==NULL){
             temp->next=new;
            return new;
        }
        temp=temp->next;
    }
    return NULL;
}
struct table * search_table(struct table * table,char * identifier){
    struct table *temp = table;
    while(temp!=NULL){
        if(!strcmp(temp->identifier,identifier)){
            return temp;
        }
        temp=temp->next;
    }
    return NULL;
}

int is_table(struct table * table,char * identifier){
    struct table *temp = table;
    while(temp!=NULL){
        if(!strcmp(temp->identifier,identifier)){
            return 1;
        }
        temp=temp->next;
    }
    return 0;
}

char * find_function_global(struct table *table,char *identifier){

    struct symbol * symbol=table->symbols->next;
    while(symbol != NULL) {
        if(strcmp(symbol->identifier, identifier) == 0) {
            while(symbol!=NULL){
                if(symbol->next!=NULL && is_table(table,symbol->next->identifier)){
                    return symbol->next->identifier;
                }else if(symbol->next==NULL){
                    return NULL;
                }
                symbol = symbol->next;
            }

        }
        symbol = symbol->next;
    }
    return NULL;
}

struct table *add_definition_table(struct table *table, char *identifier,enum category return_type){
    struct table *new = (struct table *) malloc(sizeof(struct table));
    struct table *temp = table;
    struct table *previous;
    new->identifier=strdup(identifier);
    new->next=NULL;
    new->symbols=(struct symbol *) malloc(sizeof(struct symbol));
    new->symbols->next=NULL;
    insert_symbol(new,"return",return_type,0,NULL);

    while(temp!=NULL){
        if(strcmp(identifier,temp->identifier)==0){free(new);return temp;}
        // if(temp->next==NULL){
        //      temp->next=new;
        //     return new;
        // }
        temp=temp->next;
    }
    char *next_function=find_function_global(table,identifier);
    temp = table;
    while(temp!=NULL){
        if(next_function!=NULL){
        if(strcmp(next_function,temp->identifier)==0){
            previous->next=new;
            new->next=temp;
            return new;
        }}
        if(temp->next==NULL){
              temp->next=new;
             return new;
        }
        previous=temp;
        temp=temp->next;
    }

    return NULL;


}

// look up a symbol by its identifier
struct symbol *search_symbol(struct table *table, char *identifier) {
    struct symbol *symbol;
    for(symbol = table->symbols->next; symbol != NULL; symbol = symbol->next)
        if(strcmp(symbol->identifier, identifier) == 0)
            return symbol;
    return NULL;
}


// void show_symbol_table() {
//     struct symbol_list *symbol;
//     for(symbol = symbol_table->next; symbol != NULL; symbol = symbol->next)
//         printf("Symbol %s : %s\n", symbol->identifier, type_name(symbol->type));
// }

struct node * get_identifier(struct node *node){
    struct node_list *temp=node->children->next;
    while (temp!=NULL)
    {
        if(temp->node->category==Identifier){
            return temp->node;
        }
        temp=temp->next;
    }
    return NULL;

}

struct node * get_paramlist_node(struct node *parent){
    struct  node_list * temp=parent->children->next;
    while(temp!=NULL){
        if(temp->node->category==ParamList){
            return temp->node;
        }
        temp=temp->next;
    }
    return NULL;
}

int has_type_annotation(struct node *parent,enum category type){
    struct  node_list * temp=parent->children->next;
    while(temp!=NULL){
        if(temp->node->annotation==type){
            return 1;
        }
        temp=temp->next;
    }
    return 0;
}

struct node * has_void_parameter(struct node * node){
    struct  node_list * temp=node->children->next;
    struct  node * void_node;
    struct  node * void_parent;
    int flag=0,counter=0;
    while(temp!=NULL){
        if(temp->node->category==ParamDeclaration){
            if(temp->node->children->next->node->category==Void){
                if(flag==0){
                    void_node=temp->node->children->next->node;
                }
                void_parent=temp->node;
                flag=1;
            }
            counter++;
        }
        temp=temp->next;
    }
    if(flag && counter>1){
        return void_node;
    }else if(flag && counter==1 && get_identifier(void_parent)!=NULL){
        return void_node;
    }else{
        return NULL;
    }
}
struct node * find_parameter(struct node_list *temp,char * Identifier){
    while(temp!=NULL){
        if(temp->node->category==ParamDeclaration){
            if(get_identifier(temp->node)!=NULL){
                if(!strcmp(Identifier,get_identifier(temp->node)->token)){
                    return get_identifier(temp->node);
                }
            }
            
        }
        temp=temp->next;
    }
    return NULL;
} 

int check_repeated_parameters(struct node * node){
    struct  node_list * temp=node->children->next;
    struct node *result;
    int flag=0;
    while(temp!=NULL){
        if(temp->node->category==ParamDeclaration){
            if(get_identifier(temp->node)!=NULL){
                result=find_parameter(temp->next,get_identifier(temp->node)->token);
                if(result!=NULL){
                    flag=1;
                    printf("Line %d, column %d: Symbol %s already defined\n",result->line,result->column,result->token);
                    semantic_errors++;
                }
            }
        }
        temp=temp->next;
    }
    if(flag){
        return 1;
    }else{
        return 0;
    }
    
}



int int_short_char(enum category type1,enum category type2){
    if((type1==Int && type2==Short) ||(type1==Short && type2==Int)){
        return 1;
    }
    if((type1==Int && type2==Char) ||(type1==Char && type2==Int)){
        return 1;
    }
    if((type1==Char && type2==Short) ||(type1==Short && type2==Char)){
        return 1;
    }
    if((type1==Int && type2==Double) ||(type1==Short && type2==Double) ||(type1==Char && type2==Double)){
        return 1;
    }
    return 0;
}

int conversion_declaration_types(enum category type1,enum category type2){
    if((type1==Int && type2==Short) ||(type1==Short && type2==Int)){
        return 1;
    }
    if((type1==Int && type2==Char) ||(type1==Char && type2==Int)){
        return 1;
    }
    if((type1==Char && type2==Short) ||(type1==Short && type2==Char)){
        return 1;
    }

    if((type1==Double && type2==Int) ||(type1==Double && type2==Short) ||(type1==Double && type2==Char)){
        return 1;
    }
    return 0;
}


void check_parameters_on_function_call(struct node * call,struct node * paramList){
    struct  node_list * temp=call->children->next->next;
    struct  node_list * temp_param;
    int flag=0;
    if(paramList==NULL || paramList->children->next->node->children->next->node->category==Void){
        temp_param=NULL;
    }else{
        temp_param=paramList->children->next;
    }
    int count_got=0,count_expected=0;
    while(temp!=NULL && temp_param!=NULL){
        count_expected++;
        count_got++;
        temp=temp->next;
        temp_param=temp_param->next;
    }
    if(temp!=NULL){
        while(temp!=NULL){
            count_got++;
            temp=temp->next;
        }
        printf("Line %d, column %d: Wrong number of arguments to function %s (got %d, required %d)\n"
        ,call->children->next->node->line,call->children->next->node->column,call->children->next->node->token,count_got,count_expected);
        semantic_errors++;
        flag=1;
    }else if(temp_param!=NULL){
        while(temp_param!=NULL){
            count_expected++;
            temp_param=temp_param->next;
        }
        printf("Line %d, column %d: Wrong number of arguments to function %s (got %d, required %d)\n",
        call->children->next->node->line,call->children->next->node->column,call->children->next->node->token,count_got,count_expected);
        semantic_errors++;
        flag=1;

    }


    if(!flag){
        if(paramList==NULL){
            temp_param=NULL;
        }else{
            temp_param=paramList->children->next;
        }
        temp=call->children->next->next;
        while(temp!=NULL && temp_param!=NULL){
            if(temp->node->annotation!=temp_param->node->children->next->node->category && !int_short_char(temp->node->annotation,temp_param->node->children->next->node->category)){
                printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",temp->node->line,temp->node->column,to_lower(categoryToString(temp->node->annotation)),to_lower(categoryToString(temp_param->node->children->next->node->category)));
                semantic_errors++;
            }
            temp=temp->next;
            temp_param=temp_param->next;
        }
    }

}

char * operator_to_string(enum category operator){
    switch (operator)
    {
        case Plus:
            return "+";
            break;
        case Minus:
            return "-";
            break;
        case Mul:
            return "*";
            break;
        case Div:
            return "/";
            break;
        case Add:
            return "+";
            break;
        case Sub:
            return "-";
            break;
        case Not:
            return "!";
            break;
        case BitWiseOr:
            return "|";
            break;
        case BitWiseXor:
            return "^";
            break;
        case Or:
            return "||";
            break;
        case And:
            return "&&";
            break;
        case BitWiseAnd:
            return "&";
            break;
        case Eq:
            return "==";
            break;
        case Le:
            return "<=";
            break;
        case Ne:
            return "!=";
            break;
        case Ge:
            return ">=";
            break;
        case Lt:
            return "<";
            break;
        case Gt:
            return ">";
            break;
        case Mod:
            return "%";
            break;
        case Store:
            return "=";
            break;
        case Comma:
            return ",";
            break;
        default:
            break;
    }
    return NULL;
}
 int same_parameters(struct node * list_1,struct node * list_2){
     struct node_list * temp_1=list_1->children->next;
     struct node_list * temp_2=list_2->children->next;
     while(temp_1!=NULL && temp_2!=NULL){
        if(temp_1->node->children->next->node->category!=temp_2->node->children->next->node->category){
            return 0;
        }
        temp_1=temp_1->next;
        temp_2=temp_2->next;
     }
     if(temp_1!=NULL || temp_2!=NULL){
        return 0;
     }
     return 1;
 }
//Line 277, column 5: Conflicting types (got int(int), expected double(double))
int check_if_existent_function_error(char * identifier,struct table * global_table,struct node * funcNode){
    struct node * paramList=get_paramlist_node(funcNode);
    struct symbol * symbol=search_symbol(global_table,identifier);
    if(symbol!=NULL){
    struct node_list * temp=paramList->children->next;
    struct node_list * temp_expected=symbol->paramList->children->next;
    int flag=1;
    
    if(!same_parameters(symbol->paramList,paramList) || symbol->type!=funcNode->children->next->node->category){
        printf("Line %d, column %d: Conflicting types (got %s(",get_identifier(funcNode)->line,get_identifier(funcNode)->column,to_lower(categoryToString(funcNode->children->next->node->category)));
        semantic_errors++;
        while(temp!=NULL){
            if(flag){
                printf("%s",to_lower(categoryToString(temp->node->children->next->node->category)));
                flag=0;
            }else{
                printf(",%s",to_lower(categoryToString(temp->node->children->next->node->category)));
            }
            temp=temp->next;
        }
        printf("), expected %s(",to_lower(categoryToString(symbol->type)));
        flag=1;
        while(temp_expected!=NULL){
            if(flag){
                printf("%s",to_lower(categoryToString(temp_expected->node->children->next->node->category)));
                flag=0;
            }else{
                printf(",%s",to_lower(categoryToString(temp_expected->node->children->next->node->category)));
            }
            temp_expected=temp_expected->next;
        }
        printf("))\n");
        return 1;
    }else{
        return 0;
    }
    }else{
        return 0;
    }

}
void print_parameters(struct node * paramList){
    struct node_list *temp=paramList->children->next;
    int flag=1;
    while(temp!=NULL){
        if(flag){
            printf("%s",to_lower(categoryToString(temp->node->children->next->node->category)));
            flag=0;
        }else{
            printf(",%s",to_lower(categoryToString(temp->node->children->next->node->category)));
        }
        temp=temp->next;
    }
}
void error_operator(int l, int c,enum category operator,enum category  first, enum category  second,struct node *first_paramList,struct node *second_paramList){
    
    if(first_paramList==NULL && second_paramList==NULL){
        printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",l,c,operator_to_string(operator),to_lower(categoryToString(first)),to_lower(categoryToString(second)));
        semantic_errors++;
    }else if(first_paramList!=NULL && second_paramList==NULL){
        printf("Line %d, column %d: Operator %s cannot be applied to types %s(",l,c,operator_to_string(operator),to_lower(categoryToString(first)));
        semantic_errors++;
        print_parameters(first_paramList);
        printf("), %s\n",to_lower(categoryToString(second)));
    }else if(first_paramList==NULL && second_paramList!=NULL){
        printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s(",l,c,operator_to_string(operator),to_lower(categoryToString(first)),to_lower(categoryToString(second)));
        semantic_errors++;
        print_parameters(second_paramList);
        printf(")\n");
    }else if(first_paramList!=NULL && second_paramList!=NULL){
        printf("Line %d, column %d: Operator %s cannot be applied to types %s(",l,c,operator_to_string(operator),to_lower(categoryToString(first)));
        semantic_errors++;
        print_parameters(first_paramList);
        printf("), %s(",to_lower(categoryToString(second)));
        print_parameters(second_paramList);
        printf(")\n");

    }

}


void check_program(struct node *program,struct table *table,struct table *first_table) {
    struct node_list *child = program->children->next;
    struct node * void_node;
    int flag;
    while(child!=NULL){
        switch (child->node->category)
        {
        case Declaration:
            check_program(child->node,table,first_table);
            if(child->node->children->next->node->category==Void){
                printf("Line %d, column %d: Invalid use of void type in declaration\n",child->node->children->next->next->node->line,child->node->children->next->next->node->column);
                semantic_errors++;
            }else{
                struct symbol * inserted=insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,NULL);
                if(child->node->children->next->next->next !=NULL && child->node->children->next->next->next->node->annotation!=Undef && child->node->children->next->node->category!=child->node->children->next->next->next->node->annotation && !conversion_declaration_types(child->node->children->next->node->category,child->node->children->next->next->next->node->annotation)){

                    printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                     get_identifier(child->node)->line,get_identifier(child->node)->column,to_lower(categoryToString(child->node->children->next->next->next->node->annotation)),to_lower(categoryToString(child->node->children->next->node->category)));
                     semantic_errors++;
                }else{

                
                
                if(table==first_table){
                
                if(inserted==NULL && child->node->children->next->node->category!=search_symbol(table,get_identifier(child->node)->token)->type && !conversion_declaration_types(child->node->children->next->node->category,search_symbol(table,get_identifier(child->node)->token)->type)){
                    // if(table==first_table){
                     printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                     get_identifier(child->node)->line,get_identifier(child->node)->column,to_lower(categoryToString(child->node->children->next->node->category)),to_lower(categoryToString(search_symbol(table,get_identifier(child->node)->token)->type)));
                     semantic_errors++;
                    // }else{
                    //     printf("Line %d, column %d: Symbol %s already defined\n",get_identifier(child->node)->line,get_identifier(child->node)->column,get_identifier(child->node)->token);

                    // }
                 }
                 }else{
                if(inserted==NULL){
                    printf("Line %d, column %d: Symbol %s already defined\n",get_identifier(child->node)->line,get_identifier(child->node)->column,get_identifier(child->node)->token);
                    semantic_errors++;
                
                }}
                }
            }

            break;
        case FuncDeclaration:
            void_node=has_void_parameter(get_paramlist_node(child->node));
            if(void_node==NULL){
                check_repeated_parameters(get_paramlist_node(child->node));

                if(!check_if_existent_function_error(get_identifier(child->node)->token,first_table,child->node)){
                    insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,get_paramlist_node(child->node));
                }
                
            }else{
                printf("Line %d, column %d: Invalid use of void type in declaration\n",void_node->line,void_node->column);
                semantic_errors++;
            }
            //check_program(child->node,add_table(first_table,strdup(get_identifier(child->node)),child->node->children->next->node->category),first_table);
            break;

        case FuncDefinition:
            void_node=has_void_parameter(get_paramlist_node(child->node));
            if(void_node==NULL){
                check_repeated_parameters(get_paramlist_node(child->node));

                //if(!check_if_existent_function_error(get_identifier(child->node)->token,first_table,child->node)){
                //     printf("\nERRO na funcao:%s\n",get_identifier(child->node)->token);
                // }
                //insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,get_paramlist_node(child->node));
                if(insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,get_paramlist_node(child->node))==NULL && is_table(first_table,get_identifier(child->node)->token)){
                    printf("Line %d, column %d: Symbol %s already defined\n",get_identifier(child->node)->line,get_identifier(child->node)->column,get_identifier(child->node)->token);
                    semantic_errors++;
                    //check_program(child->node,add_definition_table(first_table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category),first_table);
                }else{
                if(!check_if_existent_function_error(get_identifier(child->node)->token,first_table,child->node)){
                    check_program(child->node,add_definition_table(first_table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category),first_table);
                }}
            }else{
                printf("Line %d, column %d: Invalid use of void type in declaration\n",void_node->line,void_node->column);
                semantic_errors++;
            }
            break;

        case ParamDeclaration:
            if(child->node->children->next->node->category!=Void){
                insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,1,NULL);}
            break;

        case Identifier:
            if(child->node->do_annotation){
                struct symbol * searched_on_function=search_symbol(table,child->node->token);
                struct symbol * searched_on_global=search_symbol(first_table,child->node->token);
                if(searched_on_function!=NULL){
                    if(searched_on_function->paramList==NULL){
                        child->node->annotation=searched_on_function->type;
                    }else{
                        child->node->annotation=searched_on_function->type;
                        child->node->paramList=searched_on_function->paramList;
                    }
                }else if(searched_on_global!=NULL){
                    if(searched_on_global->paramList==NULL){
                        child->node->annotation=searched_on_global->type;
                    }else{
                        child->node->annotation=searched_on_global->type;
                        child->node->paramList=searched_on_global->paramList;
                    }
                }else{
                    child->node->annotation=Undef;
                    printf("Line %d, column %d: Unknown symbol %s\n",child->node->line,child->node->column,child->node->token);
                    semantic_errors++;
                }
            }
            break;

        case Comma:
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->next->node->annotation;
            if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) && 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                 //printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                child->node->annotation=Undef;
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier){
                child->node->annotation=Undef;
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->next->node->annotation==Undef){
                child->node->annotation=Undef;
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->annotation==Undef){
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }

            // }else if(child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Void){
            //     child->node->annotation=Undef;
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            break;
        //fazer segundo filho child->node->annotation=child->node->children->next->next->node->annotation;
        case Mul:
        case Sub:
        case Div:
        case Add:
            check_program(child->node,table,first_table);
            if(child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef){
                child->node->annotation=Undef;
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                 error_operator(child->node->line,child->node->column,
                 child->node->category,child->node->children->next->node->annotation,
                 child->node->children->next->next->node->annotation,
                 child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Void){
                child->node->annotation=Undef;
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);

            }else if(child->node->children->next->node->annotation==child->node->children->next->next->node->annotation){
                child->node->annotation=child->node->children->next->node->annotation;
            }else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
                child->node->annotation=Double;
            }else if(child->node->children->next->node->annotation==Int || child->node->children->next->next->node->annotation==Int){
                child->node->annotation=Int;
            }else if(child->node->children->next->node->annotation==Short || child->node->children->next->next->node->annotation==Short){
                child->node->annotation=Short;
            }else if(child->node->children->next->node->annotation==Char || child->node->children->next->next->node->annotation==Char){
                child->node->annotation=Char;
            }else{
                child->node->annotation=Undef;
            }
            //verificar se existe chamada de funcao vazia

            if(child->node->annotation!=Undef){
                if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) || 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                    error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    child->node->annotation=Undef;
                }
            }
            break;

        case Plus:
        case Minus:
        
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;
            if(child->node->category==Store && (child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef)){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));

                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }
            
            if((child->node->category==Plus || child->node->category==Minus) && (child->node->annotation==Void || child->node->annotation==Undef)){
                if(child->node->paramList==NULL){
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s\n",child->node->line,child->node->column,
                    operator_to_string(child->node->category),to_lower(categoryToString(child->node->annotation)));
                    semantic_errors++;
                }else{
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s(\n",child->node->line,child->node->column,
                    operator_to_string(child->node->category),to_lower(categoryToString(child->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->paramList);
                    printf(")\n");
                    }
                // error_operator(child->node->line,child->node->column,
                // child->node->category,child->node->children->next->node->annotation,
                // child->node->children->next->next->node->annotation,
                // child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Operator %s cannot be applied to type %s(\n",child->node->line,child->node->column,
                    operator_to_string(child->node->category),to_lower(categoryToString(child->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->paramList);
                    printf(")\n");
            }
            break;

        case Store:
            flag=0;
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;
            if(child->node->category==Store && child->node->children->next->node->category!=Identifier){
                printf("Line %d, column %d: Lvalue required\n",child->node->children->next->node->line,child->node->children->next->node->column);
                semantic_errors++;
            }else{
                if(child->node->children->next->node->annotation!=Double && child->node->children->next->next->node->annotation==Double){
                    error_operator(child->node->line,child->node->column,Store,child->node->children->next->node->annotation,child->node->children->next->next->node->annotation,child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    flag=1;
                }else if(child->node->children->next->node->annotation==Undef || child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Void){
                    error_operator(child->node->line,child->node->column,Store,child->node->children->next->node->annotation,child->node->children->next->next->node->annotation,child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    flag=1;
                }
                if(flag && ((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) ||  child->node->children->next->node->annotation==Undef)){
                    child->node->annotation=Undef;
                }
            }
            break;


        case Not:
            check_program(child->node,table,first_table);
            child->node->annotation=Int;
            if(child->node->children->next->node->annotation!=Int && child->node->children->next->node->annotation!=Char && child->node->children->next->node->annotation!=Short){

                if(child->node->children->next->node->paramList==NULL){
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s\n",child->node->line,child->node->column,
                operator_to_string(Not),to_lower(categoryToString(child->node->children->next->node->annotation)));
                    semantic_errors++;
                }else{
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s(",child->node->line,child->node->column,
                operator_to_string(Not),to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                print_parameters(child->node->children->next->node->paramList);
                printf(")\n");
                }
                
            }
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Operator %s cannot be applied to type %s(",child->node->line,child->node->column,
                operator_to_string(Not),to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                print_parameters(child->node->children->next->node->paramList);
                printf(")\n");
            }
            break;

        case BitWiseOr:
        case BitWiseXor:
        case Or:
        case And:
        case BitWiseAnd:
        case Mod:
            check_program(child->node,table,first_table);
            child->node->annotation=Int;
            if(child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));

                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else
            // if(child->node->category==Mod && child->node->children->next->next->node->annotation==Double){
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) || 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                    error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                }
            break;

        case Eq:
        case Le:
        case Ne:
        case Ge:
        case Lt:
        case Gt:
            check_program(child->node,table,first_table);
            child->node->annotation=Int;
            if((child->node->children->next->node->annotation==Undef && child->node->children->next->next->node->annotation!=Undef) || (child->node->children->next->node->annotation!=Undef && child->node->children->next->next->node->annotation==Undef)){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));

                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);

            }// else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            // if(child->node->category==Mod && child->node->children->next->next->node->annotation==Double){
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) || 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                    error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    child->node->annotation=Undef;
                }
            break;

        case Call:
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;

            struct symbol * symbol_call=search_symbol(first_table,child->node->children->next->node->token);
            struct symbol * symbol_call_local=search_symbol(table,child->node->children->next->node->token);
            if(symbol_call!=NULL){
                //if(!strcmp(get_identifier(child->node)->token,"chama")){printf("Chama entrou\n");}
                check_parameters_on_function_call(child->node,symbol_call->paramList);
            }else
            if(symbol_call_local!=NULL){
                check_parameters_on_function_call(child->node,symbol_call_local->paramList);
            }else{
                check_parameters_on_function_call(child->node,NULL);
            }
            break;

        case While:
        case If:
            check_program_on_node(child->node->children->next,table,first_table);
            //check_if(child->node,table,first_table);
            if(child->node->children->next->node->annotation!=Int && child->node->children->next->node->annotation!=Char && child->node->children->next->node->annotation!=Short){
                if(child->node->children->next->node->paramList==NULL){
                printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                child->node->children->next->node->line,child->node->children->next->node->column,to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(Int)));
                semantic_errors++;
                }else{
                    printf("Line %d, column %d: Conflicting types (got %s(",
                child->node->children->next->node->line,child->node->children->next->node->column,to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                print_parameters(child->node->children->next->node->paramList);
                printf("), expected %s)\n",to_lower(categoryToString(Int)));
                }
            }else
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Conflicting types (got %s(",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->children->next->node->paramList);
                    printf("), expected %s)\n",to_lower(categoryToString(table->symbols->next->type)));
            }
            check_program_on_node(child->node->children->next->next,table,first_table);
            if(child->node->children->next->next->next!=NULL){check_program_on_node(child->node->children->next->next->next,table,first_table);}
            break;

        case Return:
            check_program(child->node,table,first_table);
            if(child->node->children->next->node->annotation!=table->symbols->next->type && !int_short_char(child->node->children->next->node->annotation,table->symbols->next->type)){
                if(!(child->node->children->next->node->category==Null && table->symbols->next->type==Void)){
                if(child->node->children->next->node->category==Null){
                    printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                    child->node->line,child->node->column,
                    to_lower(categoryToString(Void)),to_lower(categoryToString(table->symbols->next->type)));
                    semantic_errors++;
                }else{
                if(child->node->children->next->node->paramList==NULL){
                    printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(table->symbols->next->type)));
                    semantic_errors++;
                }else{
                    printf("Line %d, column %d: Conflicting types (got %s(",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->children->next->node->paramList);
                    printf("), expected %s)\n",to_lower(categoryToString(table->symbols->next->type)));
                }
                }}
            }else
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Conflicting types (got %s(",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)));
                    print_parameters(child->node->children->next->node->paramList);
                    semantic_errors++;
                    printf("), expected %s)\n",to_lower(categoryToString(table->symbols->next->type)));
            }
            break;


        default:check_program(child->node,table,first_table);
            break;
        }
        child=child->next;
    }

}

char * to_lower(const char *string){
    char * str=strdup(string);
    for(int i=0;i<strlen(str);i++){
        str[i]=tolower(str[i]);
    }
    return str;
}


void print_symbol_table(struct table * tables) {
    struct table * aux_table = tables->next;
    struct node_list * temp_param;
    int flag=1;

    printf("===== Global Symbol Table =====\n");

    struct symbol *aux_symbol=tables->symbols->next;

    while (aux_symbol!=NULL){
        // if(!strcmp(aux_symbol->identifier,"putchar")){
        //     printf("putchar\tint(int)\n");
        // }else if(!strcmp(aux_symbol->identifier,"getchar")){
        //     printf("getchar\tint(void)\n");
        // }else
        if(aux_symbol->paramList==NULL){
            printf("%s\t%s\n",aux_symbol->identifier,to_lower(categoryToString(aux_symbol->type)));
        }else{
            printf("%s\t%s(",aux_symbol->identifier,to_lower(categoryToString(aux_symbol->type)));
            temp_param=aux_symbol->paramList->children->next;
            while(temp_param!=NULL){
                if(flag){
                    printf("%s",to_lower(categoryToString(temp_param->node->children->next->node->category)));
                    flag=0;
                }else{
                    printf(",%s",to_lower(categoryToString(temp_param->node->children->next->node->category)));
                }
                temp_param=temp_param->next;
            }
            printf(")\n");
            flag=1;
        }
        //printf("%s\n",aux_symbol->identifier);
        aux_symbol=aux_symbol->next;

    }


    while(aux_table!=NULL){
        printf("\n===== Function %s Symbol Table =====\n",aux_table->identifier);
        //printf("return\t%s\n", aux_table->type);
        aux_symbol=aux_table->symbols->next;
        while (aux_symbol!=NULL){
            if(aux_symbol->param){
                printf("%s\t%s\tparam\n",aux_symbol->identifier,to_lower(categoryToString(aux_symbol->type)));
            }else{
                printf("%s\t%s\n",aux_symbol->identifier,to_lower(categoryToString(aux_symbol->type)));
            }
            //printf("%s\n",aux_symbol->identifier);
            aux_symbol=aux_symbol->next;
        }
        aux_table=aux_table->next;
    }
    printf("\n");
}



//------------------- CHECK PROGRAM SÓ ANOTAÇÕES ----



void check_if(struct node *program,struct table *table,struct table *first_table) {
    struct node_list *child = program->children->next;
    
    while(child!=NULL){
        switch (child->node->category)
        {
        case Identifier:
            if(child->node->do_annotation){
                struct symbol * searched_on_function=search_symbol(table,child->node->token);
                struct symbol * searched_on_global=search_symbol(first_table,child->node->token);
                if(searched_on_function!=NULL){
                    if(searched_on_function->paramList==NULL){
                        child->node->annotation=searched_on_function->type;
                    }else{
                        child->node->annotation=searched_on_function->type;
                        child->node->paramList=searched_on_function->paramList;
                    }
                }else if(searched_on_global!=NULL){
                    if(searched_on_global->paramList==NULL){
                        child->node->annotation=searched_on_global->type;
                    }else{
                        child->node->annotation=searched_on_global->type;
                        child->node->paramList=searched_on_global->paramList;
                    }
                }else{
                    child->node->annotation=Undef;
                    //printf("Line %d, column %d: Unknown symbol %s\n",child->node->line,child->node->column,child->node->token);
                }
            }
            break;

        case Comma:
            check_if(child->node,table,first_table);
            child->node->annotation=child->node->children->next->next->node->annotation;
            break;
        case Mul:
        case Sub:
        case Div:
        case Add:
            check_if(child->node,table,first_table);
            if(child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef){
                child->node->annotation=Undef;
            }else if(child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Void){
                child->node->annotation=Undef;
            }else if(child->node->children->next->node->annotation==child->node->children->next->next->node->annotation){
                child->node->annotation=child->node->children->next->node->annotation;
            }else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
                child->node->annotation=Double;
            }else if(child->node->children->next->node->annotation==Int || child->node->children->next->next->node->annotation==Int){
                child->node->annotation=Int;
            }else if(child->node->children->next->node->annotation==Short || child->node->children->next->next->node->annotation==Short){
                child->node->annotation=Short;
            }else if(child->node->children->next->node->annotation==Char || child->node->children->next->next->node->annotation==Char){
                child->node->annotation=Char;
            }else{
                child->node->annotation=Undef;
            }
            break;

        case Plus:
        case Minus:
        case Store:
            check_if(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;
            break;


        case Not:
            check_if(child->node,table,first_table);
            child->node->annotation=Int;
            break;

        case BitWiseOr:
        case BitWiseXor:
        case Or:
        case And:
        case BitWiseAnd:
        case Mod:
            check_if(child->node,table,first_table);
            child->node->annotation=Int;
            break;

        case Eq:
        case Le:
        case Ne:
        case Ge:
        case Lt:
        case Gt:
            check_if(child->node,table,first_table);
            child->node->annotation=Int;
            break;

        case Call:
            check_if(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;
            break;


        default:check_if(child->node,table,first_table);
            break;
        }
        child=child->next;
    }

}




void check_program_on_node(struct node_list *child,struct table *table,struct table *first_table) {
    struct node * void_node;
    int flag;
    switch (child->node->category)
        {
        case Declaration:
            check_program(child->node,table,first_table);
            if(child->node->children->next->node->category==Void){
                printf("Line %d, column %d: Invalid use of void type in declaration\n",child->node->children->next->next->node->line,child->node->children->next->next->node->column);
                semantic_errors++;
            }else{
                struct symbol * inserted=insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,NULL);
                if(child->node->children->next->next->next !=NULL && child->node->children->next->next->next->node->annotation!=Undef && child->node->children->next->node->category!=child->node->children->next->next->next->node->annotation && !conversion_declaration_types(child->node->children->next->node->category,child->node->children->next->next->next->node->annotation)){

                    printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                     get_identifier(child->node)->line,get_identifier(child->node)->column,to_lower(categoryToString(child->node->children->next->next->next->node->annotation)),to_lower(categoryToString(child->node->children->next->node->category)));
                     semantic_errors++;
                }else{

                
                
                if(table==first_table){
                
                if(inserted==NULL && child->node->children->next->node->category!=search_symbol(table,get_identifier(child->node)->token)->type && !conversion_declaration_types(child->node->children->next->node->category,search_symbol(table,get_identifier(child->node)->token)->type)){
                    // if(table==first_table){
                     printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                     get_identifier(child->node)->line,get_identifier(child->node)->column,to_lower(categoryToString(child->node->children->next->node->category)),to_lower(categoryToString(search_symbol(table,get_identifier(child->node)->token)->type)));
                     semantic_errors++;
                    // }else{
                    //     printf("Line %d, column %d: Symbol %s already defined\n",get_identifier(child->node)->line,get_identifier(child->node)->column,get_identifier(child->node)->token);

                    // }
                 }
                 }else{
                if(inserted==NULL){
                    printf("Line %d, column %d: Symbol %s already defined\n",get_identifier(child->node)->line,get_identifier(child->node)->column,get_identifier(child->node)->token);
                    semantic_errors++;
                
                }}
                }
            }

            break;
        case FuncDeclaration:
            void_node=has_void_parameter(get_paramlist_node(child->node));
            if(void_node==NULL){
                check_repeated_parameters(get_paramlist_node(child->node));

                if(!check_if_existent_function_error(get_identifier(child->node)->token,first_table,child->node)){
                    insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,get_paramlist_node(child->node));
                }
                
            }else{
                printf("Line %d, column %d: Invalid use of void type in declaration\n",void_node->line,void_node->column);
                semantic_errors++;
            }
            //check_program(child->node,add_table(first_table,strdup(get_identifier(child->node)),child->node->children->next->node->category),first_table);
            break;

        case FuncDefinition:
            void_node=has_void_parameter(get_paramlist_node(child->node));
            if(void_node==NULL){
                check_repeated_parameters(get_paramlist_node(child->node));

                //if(!check_if_existent_function_error(get_identifier(child->node)->token,first_table,child->node)){
                //     printf("\nERRO na funcao:%s\n",get_identifier(child->node)->token);
                // }
                //insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,get_paramlist_node(child->node));
                if(insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,0,get_paramlist_node(child->node))==NULL && is_table(first_table,get_identifier(child->node)->token)){
                    printf("Line %d, column %d: Symbol %s already defined\n",get_identifier(child->node)->line,get_identifier(child->node)->column,get_identifier(child->node)->token);
                    semantic_errors++;
                    //check_program(child->node,add_definition_table(first_table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category),first_table);
                }else{
                if(!check_if_existent_function_error(get_identifier(child->node)->token,first_table,child->node)){
                    check_program(child->node,add_definition_table(first_table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category),first_table);
                }}
            }else{
                printf("Line %d, column %d: Invalid use of void type in declaration\n",void_node->line,void_node->column);
                semantic_errors++;
            }
            break;

        case ParamDeclaration:
            if(child->node->children->next->node->category!=Void){
                insert_symbol(table,strdup(get_identifier(child->node)->token),child->node->children->next->node->category,1,NULL);}
            break;

        case Identifier:
            if(child->node->do_annotation){
                struct symbol * searched_on_function=search_symbol(table,child->node->token);
                struct symbol * searched_on_global=search_symbol(first_table,child->node->token);
                if(searched_on_function!=NULL){
                    if(searched_on_function->paramList==NULL){
                        child->node->annotation=searched_on_function->type;
                    }else{
                        child->node->annotation=searched_on_function->type;
                        child->node->paramList=searched_on_function->paramList;
                    }
                }else if(searched_on_global!=NULL){
                    if(searched_on_global->paramList==NULL){
                        child->node->annotation=searched_on_global->type;
                    }else{
                        child->node->annotation=searched_on_global->type;
                        child->node->paramList=searched_on_global->paramList;
                    }
                }else{
                    child->node->annotation=Undef;
                    printf("Line %d, column %d: Unknown symbol %s\n",child->node->line,child->node->column,child->node->token);
                    semantic_errors++;
                }
            }
            break;

        case Comma:
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->next->node->annotation;
            if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) && 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                 //printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                child->node->annotation=Undef;
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier){
                child->node->annotation=Undef;
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->next->node->annotation==Undef){
                child->node->annotation=Undef;
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->annotation==Undef){
                error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }

            // }else if(child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Void){
            //     child->node->annotation=Undef;
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            break;
        //fazer segundo filho child->node->annotation=child->node->children->next->next->node->annotation;
        case Mul:
        case Sub:
        case Div:
        case Add:
            check_program(child->node,table,first_table);
            if(child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef){
                child->node->annotation=Undef;
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                 error_operator(child->node->line,child->node->column,
                 child->node->category,child->node->children->next->node->annotation,
                 child->node->children->next->next->node->annotation,
                 child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Void){
                child->node->annotation=Undef;
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);

            }else if(child->node->children->next->node->annotation==child->node->children->next->next->node->annotation){
                child->node->annotation=child->node->children->next->node->annotation;
            }else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
                child->node->annotation=Double;
            }else if(child->node->children->next->node->annotation==Int || child->node->children->next->next->node->annotation==Int){
                child->node->annotation=Int;
            }else if(child->node->children->next->node->annotation==Short || child->node->children->next->next->node->annotation==Short){
                child->node->annotation=Short;
            }else if(child->node->children->next->node->annotation==Char || child->node->children->next->next->node->annotation==Char){
                child->node->annotation=Char;
            }else{
                child->node->annotation=Undef;
            }
            //verificar se existe chamada de funcao vazia

            if(child->node->annotation!=Undef){
                if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) || 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                    error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    child->node->annotation=Undef;
                }
            }
            break;

        case Plus:
        case Minus:
        
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;
            if(child->node->category==Store && (child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef)){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));

                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }
            
            if((child->node->category==Plus || child->node->category==Minus) && (child->node->annotation==Void || child->node->annotation==Undef)){
                if(child->node->paramList==NULL){
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s\n",child->node->line,child->node->column,
                    operator_to_string(child->node->category),to_lower(categoryToString(child->node->annotation)));
                    semantic_errors++;
                }else{
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s(\n",child->node->line,child->node->column,
                    operator_to_string(child->node->category),to_lower(categoryToString(child->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->paramList);
                    printf(")\n");
                    }
                // error_operator(child->node->line,child->node->column,
                // child->node->category,child->node->children->next->node->annotation,
                // child->node->children->next->next->node->annotation,
                // child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Operator %s cannot be applied to type %s(\n",child->node->line,child->node->column,
                    operator_to_string(child->node->category),to_lower(categoryToString(child->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->paramList);
                    printf(")\n");
            }
            break;

        case Store:
            flag=0;
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;
            if(child->node->category==Store && child->node->children->next->node->category!=Identifier){
                printf("Line %d, column %d: Lvalue required\n",child->node->children->next->node->line,child->node->children->next->node->column);
                semantic_errors++;
            }else{
                if(child->node->children->next->node->annotation!=Double && child->node->children->next->next->node->annotation==Double){
                    error_operator(child->node->line,child->node->column,Store,child->node->children->next->node->annotation,child->node->children->next->next->node->annotation,child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    flag=1;
                }else if(child->node->children->next->node->annotation==Undef || child->node->children->next->node->annotation==Void || child->node->children->next->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Void){
                    error_operator(child->node->line,child->node->column,Store,child->node->children->next->node->annotation,child->node->children->next->next->node->annotation,child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    flag=1;
                }
                if(flag && ((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) ||  child->node->children->next->node->annotation==Undef)){
                    child->node->annotation=Undef;
                }
            }
            break;


        case Not:
            check_program(child->node,table,first_table);
            child->node->annotation=Int;
            if(child->node->children->next->node->annotation!=Int && child->node->children->next->node->annotation!=Char && child->node->children->next->node->annotation!=Short){

                if(child->node->children->next->node->paramList==NULL){
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s\n",child->node->line,child->node->column,
                operator_to_string(Not),to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                }else{
                    printf("Line %d, column %d: Operator %s cannot be applied to type %s(",child->node->line,child->node->column,
                operator_to_string(Not),to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                print_parameters(child->node->children->next->node->paramList);
                printf(")\n");
                }
                
            }
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Operator %s cannot be applied to type %s(",child->node->line,child->node->column,
                operator_to_string(Not),to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                print_parameters(child->node->children->next->node->paramList);
                printf(")\n");
            }
            break;

        case BitWiseOr:
        case BitWiseXor:
        case Or:
        case And:
        case BitWiseAnd:
        case Mod:
            check_program(child->node,table,first_table);
            child->node->annotation=Int;
            if(child->node->children->next->node->annotation==Undef || child->node->children->next->next->node->annotation==Undef){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));

                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
            }else
            // if(child->node->category==Mod && child->node->children->next->next->node->annotation==Double){
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) || 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                    error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                }
            break;

        case Eq:
        case Le:
        case Ne:
        case Ge:
        case Lt:
        case Gt:
            check_program(child->node,table,first_table);
            child->node->annotation=Int;
            if((child->node->children->next->node->annotation==Undef && child->node->children->next->next->node->annotation!=Undef) || (child->node->children->next->node->annotation!=Undef && child->node->children->next->next->node->annotation==Undef)){
                // printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
                // operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));

                error_operator(child->node->line,child->node->column,
                child->node->category,child->node->children->next->node->annotation,
                child->node->children->next->next->node->annotation,
                child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);

            }// else if(child->node->children->next->node->annotation==Double || child->node->children->next->next->node->annotation==Double){
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            // if(child->node->category==Mod && child->node->children->next->next->node->annotation==Double){
            //     printf("Line %d, column %d: Operator %s cannot be applied to types %s, %s\n",child->node->line,child->node->column,
            //     operator_to_string(child->node->category),to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(child->node->children->next->next->node->annotation)));
            // }
            if((child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier) || 
                (child->node->children->next->next->node->paramList!=NULL && child->node->children->next->next->node->category==Identifier)){
                    error_operator(child->node->line,child->node->column,
                    child->node->category,child->node->children->next->node->annotation,
                    child->node->children->next->next->node->annotation,
                    child->node->children->next->node->paramList,child->node->children->next->next->node->paramList);
                    child->node->annotation=Undef;
                }
            break;

        case Call:
            check_program(child->node,table,first_table);
            child->node->annotation=child->node->children->next->node->annotation;

            struct symbol * symbol_call=search_symbol(first_table,child->node->children->next->node->token);
            struct symbol * symbol_call_local=search_symbol(table,child->node->children->next->node->token);
            if(symbol_call!=NULL){
                //if(!strcmp(get_identifier(child->node)->token,"chama")){printf("Chama entrou\n");}
                check_parameters_on_function_call(child->node,symbol_call->paramList);
            }else
            if(symbol_call_local!=NULL){
                check_parameters_on_function_call(child->node,symbol_call_local->paramList);
            }else{
                check_parameters_on_function_call(child->node,NULL);
            }
            break;

        case While:
        case If:
            check_program_on_node(child->node->children->next,table,first_table);
            //check_if(child->node,table,first_table);
            if(child->node->children->next->node->annotation!=Int && child->node->children->next->node->annotation!=Char && child->node->children->next->node->annotation!=Short){
                if(child->node->children->next->node->paramList==NULL){
                printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                child->node->children->next->node->line,child->node->children->next->node->column,to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(Int)));
                semantic_errors++;
                }else{
                    printf("Line %d, column %d: Conflicting types (got %s(",
                child->node->children->next->node->line,child->node->children->next->node->column,to_lower(categoryToString(child->node->children->next->node->annotation)));
                semantic_errors++;
                print_parameters(child->node->children->next->node->paramList);
                printf("), expected %s)\n",to_lower(categoryToString(Int)));
                }
            }else
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Conflicting types (got %s(",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->children->next->node->paramList);
                    printf("), expected %s)\n",to_lower(categoryToString(table->symbols->next->type)));
            }
            check_program_on_node(child->node->children->next->next,table,first_table);
            if(child->node->children->next->next->next!=NULL){check_program_on_node(child->node->children->next->next->next,table,first_table);}
            break;

        case Return:
            check_program(child->node,table,first_table);
            if(child->node->children->next->node->annotation!=table->symbols->next->type && !int_short_char(child->node->children->next->node->annotation,table->symbols->next->type)){
                if(!(child->node->children->next->node->category==Null && table->symbols->next->type==Void)){
                if(child->node->children->next->node->category==Null){
                    printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                    child->node->line,child->node->column,
                    to_lower(categoryToString(Void)),to_lower(categoryToString(table->symbols->next->type)));
                    semantic_errors++;
                }else{
                if(child->node->children->next->node->paramList==NULL){
                    printf("Line %d, column %d: Conflicting types (got %s, expected %s)\n",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)),to_lower(categoryToString(table->symbols->next->type)));
                    semantic_errors++;
                }else{
                    printf("Line %d, column %d: Conflicting types (got %s(",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->children->next->node->paramList);
                    printf("), expected %s)\n",to_lower(categoryToString(table->symbols->next->type)));
                }
                }}
            }else
            if(child->node->children->next->node->paramList!=NULL && child->node->children->next->node->category==Identifier){
                printf("Line %d, column %d: Conflicting types (got %s(",
                    child->node->children->next->node->line,child->node->children->next->node->column,
                    to_lower(categoryToString(child->node->children->next->node->annotation)));
                    semantic_errors++;
                    print_parameters(child->node->children->next->node->paramList);
                    printf("), expected %s)\n",to_lower(categoryToString(table->symbols->next->type)));
            }
            break;


        default:check_program(child->node,table,first_table);
            break;
        }
        
    

    
}