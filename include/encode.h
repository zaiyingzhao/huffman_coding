#pragma once

typedef struct node Node;

/*  
struct node{
    int symbol;
    int count;
    Node *left;
    Node *right;
};
*/

// ファイルをエンコードし木のrootへのポインタを返す
int encode(const char *filename);
// Treeを走査して表示する

//void traverse_tree(const int depth, const Node *root);

