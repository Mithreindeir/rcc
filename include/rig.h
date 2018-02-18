#ifndef RIG_H
#define RIG_H

#include "threecode.h"
#define IS_TMP(oper) (oper->tac_type == TAC_TEMP)

/* Register Interference Graph
 * Vertices represent temporary values from the three address code IR
 * Generated from the AST
 * Edges are temps that are alive together or involved in a mov
 * instruction
 * */

typedef struct rig_node {
	int value;

	int tmp;

	struct rig_node **edges;
	int num_edges;
} rig_node;

struct live_tmp {
	int start;
	int end;
	int tmp;
};

/* The Control Flow Graph Structure for data flow analysis
 * To determine the live variables
 * A CFG is just a pointer to the first basic block
 * */

/*Basic Block For CFG*/
typedef struct basic_block {
	tac_instr *block;
	int len;

	//Edges are just pointers to the start or end of a different
	//basic_block
	struct basic_block **succ, **pred;
	int num_succ, num_pred;

	//Used and Defined temporary Variables
	int *use, *def;
	int num_use, num_def;

	//Temporary Variable Input/Outputs
	int *input, *output;
	int num_in, num_out;

} basic_block;

/*Basic Block Functions*/
basic_block *basic_block_init(tac_instr * first, tac_instr ** next);
void basic_block_destroy(basic_block * block);

void basic_block_apred(basic_block * blk, basic_block * pred);
void basic_block_asucc(basic_block * blk, basic_block * succ);

tac_instr *basic_block_start(basic_block * block);
tac_instr *basic_block_last(basic_block * block);

basic_block *cfg_construct(tac_instr * enter);
void basic_block_initialize(basic_block * block);
/*	a=0
 *L1:   b=a+1
 *	c=c+b
 *	a=b+2
 *	if a < N goto L1
 *	return C
 * Algorithm:
 * Live-in=if temp t is in set use of node or path from n to node that
 * uses t but doesnt define it
 * t E in[N]
 * Live-out=if temp t is in set use of successor of n
 * t E out[N]
 *
 * So Constructing the sets in and out:
	* in[n] = use[n] U ('out[n] - def[n])
 * out[n] = U {in[s] | s E succ[n]}
 *
 * Start from back
 * n6: uses C defines nothing
 * n5: uses a
 * n4: uses b and def a
 * n3: uses c +b and def c
 * n2: use a def b
 * n1: def a
 *
 * Basic Block Algorithm For Calculating In And Out
 * In=all variables not explicitly defined before use
 * out=all variables used
 * Ex:
 * a = 2
 * b = a + d
 * c = b * a
 *
 * in=d
 * out=a,b,c,d
 *
 * */

int defined_values(tac_instr * instr);
void used_values(tac_instr * instr, int **arr, int *num);
void intarr_add(int **arr, int *num, int val);

#endif
