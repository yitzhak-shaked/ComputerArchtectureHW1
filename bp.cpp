/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */
/*************************************************************************
 * Includes
 ************************************************************************/
#include "bp_api.h"
#include <iostream>

/*************************************************************************
 * usings
 ************************************************************************/
using std::cout;
using std::endl;
using std::cerr;

// cout <<  // WHENDONE: Remove

/*************************************************************************
 * Class Declarations
 ************************************************************************/
// History class
class History {
private:
	unsigned historySize;
	uint32_t history;

public:
	History(unsigned historySize=1) {
		this->historySize = historySize;
		this->history = 0;
	}

	void update_history(bool taken) { // TODO: Validate
		this->history = (this->history << 1) | (taken ? 1 : 0);
	}

	uint32_t get_history(){
		return this->history;
	}
};

// BTB class
class BTB {
private:
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	unsigned fsmState;
	
	uint32_t *tagList;
	uint32_t *targetPcList;
	History *historyList;

public:
	BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState){
		this->btbSize = btbSize;
		this->historySize = historySize;
		this->tagSize = tagSize;
		this->fsmState = fsmState;
	}
};

/*************************************************************************
 * Global variables
 ************************************************************************/


/*************************************************************************
 * Helper Functions
 ************************************************************************/


 /*************************************************************************
 * Target Functions
 ************************************************************************/
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared) {
	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst) {
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
	return;
}

void BP_GetStats(SIM_stats *curStats) {
	return;
}

/*************************************************************************
 * Class Implementations
 ************************************************************************/
// BTB class implementation
BTB::BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState) {
	// TODO: Validate and complete constructor
	this->btbSize = btbSize;
	this->historySize = historySize;
	this->tagSize = tagSize;
	this->fsmState = fsmState;

	this->tagList = new uint32_t[btbSize];
	this->targetPcList = new uint32_t[btbSize];
	this->historyList = new History[btbSize];
}


