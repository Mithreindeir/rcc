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

typedef struct rig_node
{
	int value;

	int tmp;

	struct rig_node **edges;
	int num_edges;
} rig_node;

struct live_tmp
{
	int start;
	int end;
	int tmp;
};

/* The Control Flow Graph Structure for data flow analysis
 * To determine the live variables
 * A CFG is just a pointer to the first basic block
 * */

/*Basic Block For CFG*/
typedef struct basic_block
{
	tac_instr *block;
	int len;

	//Edges are just pointers to the start or end of a different
	//basic_block
	basic_block **succ, **pred;
	int num_succ, num_pred;

	//Temporary Variable Input/Outputs
	int *input, *output;
	int num_in, num_out;

} basic_block;

/*Basic Block Functions*/
basic_block *basic_block_init(tac_instr *first, tac_instr **next);
void basic_block_destroy(basic_block *block);

void basic_block_apred(basic_block *blk, basic_block *pred);
void basic_block_asucc(basic_block *blk, basic_block *succ);

tac_instr *basic_block_start(basic_block *block);
tac_instr *basic_block_last(basic_block *block);

basic_block *cfg_construct(tac_instr *enter);
void basic_block_live(basic_block * block);


#endif
