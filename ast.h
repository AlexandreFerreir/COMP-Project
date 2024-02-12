#ifndef _AST_H
#define _AST_H

enum category {Undef,Null,Void,Decimal,Double,Natural,Short,Int,Identifier,ChrLit,Char,BitWiseOr,BitWiseXor,BitWiseAnd,Call,Comma,Store,Plus,Minus,Not,Mod,Div,Mul,Sub,Add,Ge,Le,Gt,Lt,Ne,Eq,And,Or,If,While,Return,StatList,ParamDeclaration, FuncBody, ParamList, FuncDefinition, FuncDeclaration, Declaration, Program , Unknown,Error};

enum type {int_type, char_type, null_type};


struct node {
    enum category category;
    char *token;
    enum category annotation;
    int do_annotation;
    int line,column;
    struct node * paramList;
    struct node_list *children;
};

struct node_list {
    struct node *node;
    struct node_list *next;
};

struct node *newnode(enum category category, char *token);
void addchild(struct node *parent, struct node *child);
void print_tree(struct node *root,int level);
void addbro(struct node *bro, struct node *newbro);
void assignBrothers(struct node *parent, struct node_list *children);
void assignTypeSpec(struct node *node, enum category categoria);
int checkChildren(struct node *node);
struct node * returnCheckChildren(struct node *node);

const char *categoryToString(enum category cat);

void deallocate(struct node * node);

void print_tree_with_annotations(struct node * root,int level);

#endif
