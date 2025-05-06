/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */
/*************************************************************************
 * Includes
 ************************************************************************/
#include "bp_api.h"
#include <iostream>
#include <bitset>

/*************************************************************************
 * Constants
 ************************************************************************/
const unsigned FSM_SIZE = 2; // Size of the FSM
const unsigned GFSM_VECTOR_MAX_SIZE = 3; // Maximum size of the FSM

/*************************************************************************
 * usings
 ************************************************************************/
using std::cout;
using std::endl;
using std::cerr;
using std::bitset;

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
	// Constructor
	History(unsigned historySize=1) {
		this->historySize = historySize;
		this->history = 0;
	}

	// Setters and Getters
	uint32_t get_history() { return this->history; }
	// void set_history_size(unsigned historySize) { this->historySize = historySize; }

	void update_history(bool taken) {
		this->history = (this->history << 1) | (taken ? 1 : 0);
	}

};

// FSM class
class FSM {
private:
	unsigned fsmState;
	unsigned fsmSize;
public:
	FSM(unsigned fsmState=2, unsigned fsmSize=FSM_SIZE) : fsmState(fsmState), fsmSize(fsmSize) { }

	// Setters and Getters
	unsigned get_fsmState() { return this->fsmState; }
	void set_fsmState(unsigned fsmState) { this->fsmState = fsmState; }
	void set_fsmSize(unsigned fsmSize) { this->fsmSize = fsmSize; }

	void update_fsm(bool taken) {
		if (taken) {
			if (this->fsmState < this->fsmSize - 1) {
				this->fsmState++;
			}
		} else {
			if (this->fsmState > 0) {
				this->fsmState--;
			}
		}
	}
};

// BTBEntry class
class BTBEntry {
private:
	uint32_t tag;
	uint32_t targetPc;
	History history;
	FSM fsmState;

public:
	BTBEntry(uint32_t tag=0, uint32_t targetPc=0, unsigned historySize=1, unsigned initailFsmState=1)
		: tag(tag), targetPc(targetPc), history(historySize) {
			this->fsmState.set_fsmState(initailFsmState);
		}

	BTBEntry& operator=(const BTBEntry& other) {
		if (this == &other) {
			return *this;
		}
		this->tag = other.tag;
		this->targetPc = other.targetPc;
		this->history = other.history;
		return *this;
	}

	// Setters and Getters
	uint32_t get_tag() { return this->tag; }
	uint32_t get_targetPc() { return this->targetPc; }
	uint32_t get_history() { return this->history.get_history(); }

	void set_tag(uint32_t tag) { this->tag = tag; }
	void set_targetPc(uint32_t targetPc) { this->targetPc = targetPc; }
	void update_history(bool taken) { this->history.update_history(taken); }
};

// BTB class
class BTB {
private:
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	FSM fsmState;
	
	BTBEntry *btbEntries;

public:
	// Constructor and Destructor
	BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState);
	~BTB();

	uint32_t get_btbEntryTarget(uint32_t tag, uint32_t pc);
};

/*************************************************************************
 * Global variables
 ************************************************************************/
BTB *globalBTB = nullptr;
History *globalHistory = nullptr;
FSM globalFSMVector[GFSM_VECTOR_MAX_SIZE];  // TODO: Validate.
bool gIsGlobalHist;
bool gIsGlobalTable;
int gShared;

/*************************************************************************
 * Helper Functions
 ************************************************************************/


 /*************************************************************************
 * Target Functions
 ************************************************************************/
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared) {
	// Initialize global variables
	gIsGlobalHist = isGlobalHist;
	gIsGlobalTable = isGlobalTable;
	gShared = Shared;

	// Initialize the BTB
	globalBTB = new BTB(btbSize, historySize, tagSize, fsmState);

	if (isGlobalHist) {
		globalHistory = new History(historySize);
	}

	if (isGlobalTable) {
		for (unsigned i = 0; i < btbSize; ++i) {
			FSM &currentFSM = globalFSMVector[i];
			currentFSM.set_fsmState(fsmState);
			currentFSM.set_fsmSize(FSM_SIZE);
		}
	}

	// TODO: Complete implementation

	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst) {
	// TODO: Complete implementation

	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
	// TODO: Complete implementation

	return;
}

void BP_GetStats(SIM_stats *curStats) {
	// TODO: Complete implementation

	return;
}

/*************************************************************************
 * BTB Class Implementation
 ************************************************************************/
BTB::BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState)
	: btbSize(btbSize), historySize(historySize), tagSize(tagSize) {
	this->fsmState = FSM(fsmState, FSM_SIZE);
	this->btbEntries = new BTBEntry[btbSize];
	for (unsigned i = 0; i < btbSize; ++i) {
		this->btbEntries[i] = BTBEntry(0, 0, historySize);
	}
}

BTB::~BTB() {
	delete[] this->btbEntries;
	this->btbEntries = nullptr;
}

uint32_t BTB::get_btbEntryTarget(uint32_t tag, uint32_t pc) {
	// Ditrct access to the BTB entry
	uint32_t mask = 0 + btbSize - 1;
	BTBEntry &entry = btbEntries[tag & mask];

	// If the entry exists, return it
	if (entry.get_tag() == tag) {
		return entry.get_targetPc();
	}

	// If the entry does not exist, create a new one
	entry = BTBEntry(tag, 0, historySize);
	
	return pc + 4; // Default target address
}


/******************************************************************************
 * Test Functions
 ************************************************************************/
void test_history() {
	History history(4);
	cout << "History initial state: " << bitset<4>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 1 - push 1 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 2 - push 0 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 3 - push 1 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 4 - push 1 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 5 - push 0 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 6 - push 1 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 7 - push 0 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 8 - push 0 - History: " << bitset<4>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 9 - push 1 - History: " << bitset<4>(history.get_history()) << endl;
}

/****************************************************************************
 * Main Function for Testing
 ************************************************************************/
int main() {
	test_history();

	return 0;
}

