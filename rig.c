#include "rig.h"

basic_block *basic_block_init(tac_instr *first, tac_instr **next)
{
	basic_block *bb = malloc(sizeof(basic_block));

	bb->succ = NULL;
	bb->num_succ = 0;

	bb->pred = NULL;
	bb->num_pred = 0;

	bb->input = NULL;
	bb->num_in = 0;

	bb->output = NULL;
	bb->num_out = 0;

	//Removes link after the first condjmp/goto instr
	//Or if the instr is a label (the endpoint of a jump)
	//Labels can start basic blocks but not end them.
	bb->block = first;
	bb->len = 0;

	tac_instr *last = first;
	while (first) {
		if (first->tac_type == TAC_GOTO || first->tac_type == TAC_CONDJMP) {
			*next = first->next;
			first->next = NULL;
		} else if (bb->len && first->tac_type == TAC_LABEL) {
			*next = first;
			last->next = NULL;
			break;
		}

		first = first->next;
		if (bb->len) last = last->next;
		bb->len++;
	}

	return bb;
}

void basic_block_destroy(basic_block *block)
{
	if (!block) return;

	tac_instr *cur = block->block;
	tac_instr *next = cur;
	while (cur) {
		next = cur->next;
		tac_instr_destroy(cur);
		cur = next;
	}

	free(block->output);
	free(block->input);
	free(block->pred);
	free(block->succ);
	free(block);
}

tac_instr *basic_block_start(basic_block *block)
{
	return block->block;
}

tac_instr *basic_block_last(basic_block *block)
{
	tac_instr *cur = block->block;
	while (cur->next) cur = cur->next;
	return cur;
}

void basic_block_apred(basic_block *blk, basic_block *pred)
{
	blk->num_pred++;
	if (blk->num_pred==1) {
		blk->pred = malloc(sizeof(basic_block*));
	} else {
		blk->pred = realloc(blk->pred, sizeof(basic_block)*blk->num_pred);
	}

	blk->pred[blk->num_pred-1] = pred;
}

void basic_block_asucc(basic_block *blk, basic_block *succ)
{
	blk->num_succ++;
	if (blk->num_succ) {
		blk->succ = malloc(sizeof(basic_block*));
	} else {
		blk->succ = realloc(blk->succ, sizeof(basic_block*)*blk->num_succ);
	}
	blk->succ[blk->num_succ-1] = succ;
}

basic_block *cfg_construct(tac_instr *enter)
{
	basic_block **blocks = NULL;
	int num_blocks = 0;

	//Create array of all the basic blocks
	tac_instr *next = enter;
	//If the block starts with a label put it into an array
	int *label_block = NULL;
	while (next) {
		int labeln = -1;
		if (next->type == TAC_LABEL) labeln = next->label;
		basic_block *bb = basic_block_init(next, &next);
		//All basic blocks have a natural edge to the next
		//instruction unless there is an unconditional jump
		if (num_blocks) {
			tac_instr *lasti = basic_block_last(blocks[num_blocks-1]);
			if (lasti->type != TAC_GOTO) {
				basic_block_apred(bb, blocks[num_blocks-1]);
				basic_block_asucc(blocks[num_blocks-1], bb);
			}
		}

		num_blocks++;
		if (num_blocks == 1) {
			label_block = malloc(sizeof(int));
			blocks = malloc(sizeof(basic_block*));
		} else {
			label_block = realloc(label_block, sizeof(int) *num_block);
			blocks = realloc(blocks, sizeof(basic_block*) *num_blocks);
		}
		label_block[num_blocks-1] = labeln;
		blocks[num_blocks-1] = bb;
	}

	//Make edges to from jump/unconditional jumps
	for (int i = 0; i < num_blocks; i++) {
		tac_instr *last = basic_block_last(blocks[i]);
		int labeln = -1;
		if (last->tac_type == TAC_GOTO)
			labeln = last->label;
		else if (last->tac_type == TAC_CONDJMP)
			labeln = last->clabel;
		basic_block *edge = NULL;
		for (int j = 0; j < num_blocks; j++) {
			edge = label_block[i]==labeln ? blocks[j] : NULL;
			if (!edge)
				break;
		}
		if (labeln != -1 && edge) {
			basic_block_apred(edge, blocks[i]);
			basic_block_asucc(blocks[i], edge);
		}
	}
	//Get rid of any dangling blocks then return the enter point
	for (int i = 0; i < num_blocks; i++) {
		//No references to the block
		if (!blocks[i]->pred && !blocks[i]->succ) {
			basic_block_destroy(blocks[i]);
			blocks[i] = NULL;
		}
	}
	basic_block *enter = num_blocks > 0 ? blocks[0] : NULL;
	free(blocks[i]);
	return enter;
}

