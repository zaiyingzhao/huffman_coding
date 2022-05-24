#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "encode.h"
#define NSYMBOLS 256

static int symbol_count[NSYMBOLS];

// ノードを表す構造体
typedef struct node
{
  int symbol;
  int count;
  struct node *left;
  struct node *right;
} Node;

static int err; //実行時半分程度の割合で数値がおかしくなる(2数の和が負になってしまう)のでそのエラー表示用

// 以下このソースで有効なstatic関数のプロトタイプ宣言

// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// symbol_count をリセットする関数
static void reset_count(void);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数
static Node *build_tree(void);

//深さ優先探索を行いとツリー構造を表示する関数
static void traverse_tree(const int depth, const Node *np, int trigger);


// 以下 static関数の実装
static void count_symbols(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "error: cannot open %s\n", filename);
	    exit(1);
    }

    *symbol_count = (int*)calloc(NSYMBOLS, sizeof(int));
    int c;
    while (fread(&c, sizeof(char), 1, fp) != 0) { //それぞれの文字コードに対応する番号にその文字の出現回数を格納する
        symbol_count[c]++;
    }

    fclose(fp);
}


static void reset_count(void)
{
    for (int i = 0 ; i < NSYMBOLS ; i++) {
        symbol_count[i] = 0;
    }
}


static Node *create_node(int symbol, int count, Node *left, Node *right)
{
    Node *ret = (Node *)malloc(sizeof(Node));
    *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right};
    return ret;
}

static Node *pop_min(int *n, Node *nodep[]) 
{
    // Find the node with the smallest count
    // カウントが最小のノードを見つけてくる
    int argmin = 0;
    for (int i = 0; i < *n; i++) {
	    if (nodep[i]->count < nodep[argmin]->count) {
	        argmin = i;
	    }
    }
    
    Node *node_min = nodep[argmin]; //最小出現回数のノード
    
    // Remove the node pointer from nodep[]
    // 見つかったノード以降の配列を前につめていく
    for (int i = argmin; i < (*n) - 1; i++) {
	    nodep[i] = nodep[i + 1];
    }
    // 合計ノード数を一つ減らす
    (*n)--;
    
    return node_min;
}

static Node *build_tree(void)
{
    int n = 0;
    Node *nodep[NSYMBOLS];
    
    for (int i = 0; i < NSYMBOLS; i++) {
	// カウントの存在しなかったシンボルには何もしない
	    if (symbol_count[i] == 0) continue;
	
	    nodep[n++] = create_node(i, symbol_count[i], NULL, NULL);
    }

    const int dummy = -1; // ダミー用のsymbol を用意しておく
    while (n >= 2) {
	Node *node1 = pop_min(&n, nodep);
	Node *node2 = pop_min(&n, nodep); //出現回数が少ない2つのノード, ここまでで合計ノード数nは2だけ減っていることに注意
	
	// Create a new node
	// 選ばれた2つのノードを元に統合ノードを新規作成
	// 作成したノードはnodep にどうすればよいか?
	
    int count_added = node1->count + node2->count; //出現回数を合計
    nodep[n] = create_node(dummy, count_added, node2, node1); //統合ノード作成, 文字は一律に-1(dummy), もともとnodep[n]にあったノードは前につめられているのでここに代入して大丈夫
    n++;
    }

    printf("nodep[1]->count = %d\n", nodep[1]->count); //確認用, 負の値ならおかしいのでもう一度実行する
    if (nodep[1]->count < 0) {
        err = 1;
    }

    // なぜ以下のコードで木を返したことになるか少し考えてみよう
    return (n==0)?NULL:nodep[0];
}


// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
// 現状は何もしていない（再帰してたどっているだけ）
static void traverse_tree(const int depth, const Node *np, int trigger) //triggerは左移動のとき0, 右移動のとき1
{			  
    static int huffmancode[100] = {0}; //ハフマン符号を格納する配列 
    static char tree[100] = {' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|',' ','|'}; //枝表示用の配列
    static int linenumber; //現状の枝の数
    static int successiveleft = 0; //連続で左を探索した回数, これによってtreeの更新時どれだけ枝を復活させるか変わる

    if (np == NULL || np->left == NULL) { 
        if (trigger == 1) {
            successiveleft = 0; //右を探索するので0に初期化
            printf("\n");
            for (int i = 0; i < 2*(depth-1)+1; i++) {
                printf("%c", tree[i]);
            }
            printf("|\n");
            for (int i = 0; i < 2*(depth-1)+1; i++) {
                printf("%c", tree[i]);
            }
            printf("+-"); //ここまでがツリー構造のための出力
            if (np->symbol == '\n') {
                printf(" \\n: ");
            }
            else if (np->symbol == '\0') {
                printf(" NULL: ");
            }
            else if (np->symbol == ' ') {
                printf(" _: ");
            }
            else {
                printf(" %c: ", np->symbol);
            }
            for (int i = 1; i < depth; i++) {
                printf("%d", huffmancode[i]);
            }
            printf("1"); //ここまでで得られたハフマンコードが出力される
            tree[2*depth-1] = ' ';

        }
        else if (trigger == 0) {
            if (linenumber > depth) {
                for (int i = 2*(depth-successiveleft); i < 24; i++) {
                    if (i%2 == 1) {
                        tree[i] = '|'; //treeを初期化
                    }
                }
            }
            successiveleft++;  //左を探索したため
            printf("+-");
            if (np->symbol == '\n') {
                printf(" \\n: ");
            }
            else if (np->symbol == '\0') {
                printf(" NULL: ");
            }
            else if (np->symbol == ' ') {
                printf(" _: ");
            }
            else {
                printf(" %c: ", np->symbol);
            }
            for (int i = 1; i < depth; i++) {
                printf("%d", huffmancode[i]);
            }
            printf("0");
            tree[2*(depth - successiveleft)-1] = ' ';
            linenumber = depth;
        }

        return;
    }

    //ここからツリー構造のための出力を規定し, また配列に数字を入れハフマンコードを設定する
    if (trigger == 1) {
        successiveleft = 0;
        huffmancode[depth] = 1;
        printf("\n");
        for (int i = 0; i < 2*(depth-1)+1; i++) {
                printf("%c", tree[i]);
            }
        printf("|\n");
        for (int i = 0; i < 2*(depth-1)+1; i++) {
                printf("%c", tree[i]);
            }
        printf("+-");
    }
    if (trigger == 0) {
        huffmancode[depth] = 0; //初めすべての要素は0だが右の子ノードをtraverseする際に0->1に更新する可能性があるのでこれは必要
        printf("+-");
        if (linenumber > depth) {
            for (int i = 2; i < 24; i++) {
                if (i%2 == 1) {
                    tree[i] = '|'; //treeを初期化
                }
            }
        }
        successiveleft++;
    }    

    if (depth == 0) {
        printf("-");
    }
    
    traverse_tree(depth + 1, np->left, 0);
    traverse_tree(depth + 1, np->right, 1);
}

// この関数は外部 (main) で使用される (staticがついていない)
int encode(const char *filename)
{
    reset_count();
    count_symbols(filename);
    Node *root = build_tree();

    if (root == NULL){
	    fprintf(stderr,"A tree has not been constructed.\n");
        return EXIT_FAILURE;
    }

    traverse_tree(0, root, 2); //初めtriggerは1でも0でもない値にする

    if (err == 1) { //2数の和がなぜか負になってしまったとき実行者にもう一度実行することを促す
        printf("\nUNEXPECTED ERROR HAS OCCURRED! RUN THIS PROGRAM AGAIN! もう一度実行してください！\n"); //謎のエラーが出たときにはセグフォになってしまうのでこのエラー表示は出力されない…
        return EXIT_FAILURE;
    }

    fprintf(stdout, "\nA tree has been constructed successfully.\n");
    
    return EXIT_SUCCESS;
}
