#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "semantics.h"
#include <string.h>

// create a node of a given category with a given lexical symbol
struct node *newnode(enum category category, char *token) {
    struct node *new = malloc(sizeof(struct node));
    new->category = category;
    new->annotation=Unknown;
    new->paramList=NULL;
    new->do_annotation=0;
    new->line=0;
    new->column=0;
    new->token = token;
    new->children = malloc(sizeof(struct node_list));
    new->children->node = NULL;
    new->children->next = NULL;
    return new;
}

// append a node to the list of children of the parent node
void addchild(struct node *parent, struct node *child) {
    struct node_list *new = malloc(sizeof(struct node_list));
    new->node = child;
    new->next = NULL;
    struct node_list *children = parent->children;
    while(children->next != NULL)
        children = children->next;
    children->next = new;
}

// void addbro(struct node *bro, struct node *newbro) {
//     addchild(bro->parent,newbro);
// }

void assignBrothers(struct node *parent, struct node_list *children){
    struct node_list *temp = children;
    
    while(temp!=NULL){
        //if(temp->node!=NULL){
            if(temp->node->category!=Unknown){
                addchild(parent,temp->node);
            }else{
                assignBrothers(parent,temp->node->children->next);
            }
        //}
        temp=temp->next;
    }
    
}

const char *categoryToString(enum category cat) {
    switch (cat) {
        case Null: return "Null";
        case Void: return "Void";
        case Decimal: return "Decimal";
        case Double: return "Double";
        case Natural: return "Natural";
        case Short: return "Short";
        case Int: return "Int";
        case Identifier: return "Identifier";
        case ChrLit: return "ChrLit";
        case Char: return "Char";
        case BitWiseOr: return "BitWiseOr";
        case BitWiseXor: return "BitWiseXor";
        case BitWiseAnd: return "BitWiseAnd";
        case Call: return "Call";
        case Comma: return "Comma";
        case Store: return "Store";
        case Plus: return "Plus";
        case Minus: return "Minus";
        case Not: return "Not";
        case Mod: return "Mod";
        case Div: return "Div";
        case Mul: return "Mul";
        case Sub: return "Sub";
        case Add: return "Add";
        case Ge: return "Ge";
        case Le: return "Le";
        case Gt: return "Gt";
        case Lt: return "Lt";
        case Ne: return "Ne";
        case Eq: return "Eq";
        case And: return "And";
        case Or: return "Or";
        case If: return "If";
        case While: return "While";
        case Return: return "Return";
        case StatList: return "StatList";
        case ParamDeclaration: return "ParamDeclaration";
        case FuncBody: return "FuncBody";
        case ParamList: return "ParamList";
        case FuncDefinition: return "FuncDefinition";
        case FuncDeclaration: return "FuncDeclaration";
        case Declaration: return "Declaration";
        case Program: return "Program";
        case Undef: return "undef";
        default: return "UnknownNode";
    }
}

void print_tree(struct node *root,int level){
    for(int i=0;i<level;i++){
        printf("..");
    }
    if(root->token==NULL){
        printf("%s\n",categoryToString(root->category));
    }else{
        printf("%s(%s)\n",categoryToString(root->category),root->token);
    }
    if(root->children!=NULL){
    struct node_list *child = root->children->next;
        while(child!=NULL){
            //if(child->node!=NULL){  //possivel retirada
                print_tree(child->node,level+1);
            //}
            child=child->next;
        }
    }
}

void assignTypeSpec(struct node *node, enum category categoria){
    struct node_list *temp = node->children->next;
    while(temp!=NULL){
        if(temp->node->category==Declaration){
            temp->node->children->next->node->category=categoria;
        }
        
        temp=temp->next;
    }
}
int checkChildren(struct node *node){
    int filhos=0;
    struct node_list *temp = node->children->next;
    while(temp!=NULL){
        if(temp->node!=NULL){
            filhos++;
        }
        temp=temp->next;
    }
    return filhos;
}

struct node * returnCheckChildren(struct node *node){
    struct node_list *temp = node->children->next;
    while(temp!=NULL){
        if(temp->node!=NULL){
            return temp->node;
        }
        temp=temp->next;
    }
    return NULL;
}

// free the AST
void deallocate(struct node * node) {
    if(node != NULL) {
        struct node_list * child = node->children;
        while(child != NULL) {
            deallocate(child->node);
            struct node_list * tmp = child;
            child = child->next;
            free(tmp);
        }
        if(node->token != NULL)
            free(node->token);
        free(node);
    }
}

void print_tree_with_annotations(struct node * root,int level){
    //printf("Annotation:%s\n",categoryToString(root->annotation));
    int flag=1;
    struct node_list * temp_param;
    for(int i=0;i<level;i++){
        printf("..");
    }
    if(root->token==NULL){
        if (root->annotation == Unknown){
            printf("%s\n",categoryToString(root->category));
        }else{
            printf("%s - %s\n",categoryToString(root->category),to_lower(categoryToString(root->annotation)));
        }
    }else{
        if (root->annotation == Unknown){
            printf("%s(%s)\n",categoryToString(root->category),root->token);
        }else{
            if(!strcmp(root->token,"putchar")){
                printf("Identifier(putchar) - int(int)\n");
            }else if(!strcmp(root->token,"getchar")){
                printf("Identifier(getchar) - int(void)\n");
            }else
            if(root->paramList==NULL){
                printf("%s(%s) - %s\n",categoryToString(root->category),root->token,to_lower(categoryToString(root->annotation)));
            }else{
                printf("%s(%s) - %s(",categoryToString(root->category),root->token,to_lower(categoryToString(root->annotation)));
                temp_param=root->paramList->children->next;
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
        }
    }

    if(root->children!=NULL){
        struct node_list * child = root->children->next;
        while(child!=NULL){
            //if(child->node!=NULL){ //possivel retirada
            print_tree_with_annotations(child->node,level+1);
            //}
            child=child->next;
        }
    }
}


