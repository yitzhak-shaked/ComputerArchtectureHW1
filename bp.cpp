/* 046267 Computer Architecture - HW #1                                      */
/* This file should hold your implementation of the predictor simulator      */
/* Athors: Shaked Yitzhak - 322704776, Idan Simon - TODO: Fill          */
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "bp_api.h"
#include <iostream>
#include <bitset>

/******************************************************************************
 * Constants
 *****************************************************************************/
const unsigned FSM_SIZE = 4; // Size of the FSM
const unsigned FSM_VECTOR_MAX_SIZE = 256; // Maximum size of the FSM
const unsigned MAX_BTB_SIZE = 32;
const unsigned MAX_HISTORY_SIZE = 8;
const unsigned MAX_TAG_SIZE = 32;

/******************************************************************************
 * usings
 *****************************************************************************/
using std::cout;
using std::endl;
using std::cerr;
using std::bitset;

// cout <<  // WHENDONE: Remove

/******************************************************************************
 * Declarations
 *****************************************************************************/
class History;
class FSM;
class BTBEntry;
class BTB;

/******************************************************************************
 * Helper Functions declarations
 *****************************************************************************/
uint32_t get_bits(uint32_t value, unsigned start, unsigned end);
int log2(int x);
uint32_t calculate_tag(uint32_t pc);
int power(int base, int exp);

/******************************************************************************
 * Class Declarations
 *****************************************************************************/
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
	uint32_t get_history() const { return get_bits(this->history, 0, this->historySize-1); }
	// void set_history_size(unsigned historySize) { this->historySize = historySize; }

	// Operator=
	History& operator=(const History& other) {
		if (this == &other) {
			return *this;
		}
		this->historySize = other.historySize;
		this->history = other.history;
		return *this;
	}

	// Functions
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
	unsigned get_fsmState() const { return this->fsmState; }
	bool get_fsmIsTaken() const { return this->fsmState >= this->fsmSize / 2; }
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
	FSM fsmVec[FSM_VECTOR_MAX_SIZE];

public:
	BTBEntry(uint32_t tag=0, uint32_t targetPc=0, unsigned historySize=1, unsigned initailFsmState=1);

	BTBEntry& operator=(const BTBEntry& other);

	// Setters and Getters
	uint32_t get_tag() const;
	uint32_t get_targetPc() const;
	uint32_t get_history() const;
	bool get_isTaken(int fsmIndex) const;

	void set_tag(uint32_t tag);
	void set_targetPc(uint32_t targetPc);
	// void set_fsmState(unsigned fsmState) { this->fsmVec.set_fsmState(fsmState); }
	
	// Functions
	void update_history(bool taken);
	void update_fsm(int fsmIndex, bool taken);

	void recreate_entry(uint32_t tag, uint32_t targetPc);
};

// BTB class
class BTB {
private:
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	FSM fsmState;
	
	BTBEntry btbEntries[MAX_BTB_SIZE];

public:
	// Constructor and Destructor
	BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState);
	~BTB();

	// Setters and Getters
	unsigned get_btbSize() { return this->btbSize; }
	unsigned get_historySize() { return this->historySize; }
	unsigned get_tagSize() { return this->tagSize; }
	
	void set_btbSize(unsigned btbSize) { this->btbSize = btbSize; }
	void set_historySize(unsigned historySize) { this->historySize = historySize; }
	void set_tagSize(unsigned tagSize) { this->tagSize = tagSize; }
	void set_fsmState(unsigned fsmState) { this->fsmState.set_fsmState(fsmState); }
	
	// Operator=
	BTB& operator=(const BTB& other);

	// Functions
	bool get_BTB_entry_prediction(uint32_t tag, uint32_t pc, uint32_t *dst);
	void update_BTB_entry(uint32_t tag, uint32_t pc, uint32_t targetPc, bool taken);
};

/******************************************************************************
 * Global variables
 *****************************************************************************/
BTB globalBTB = BTB(0, MAX_HISTORY_SIZE, MAX_TAG_SIZE, 2);
History globalHistory = History(MAX_HISTORY_SIZE);
FSM globalFSMVector[FSM_VECTOR_MAX_SIZE];  // TODO: Validate.
unsigned gInitalFsmState = 2;
bool gIsGlobalHist = false;
bool gIsGlobalTable = false;
int gXORMethod = 0;

bool lastTaken = false;
unsigned gFlushCounter = 0;
unsigned gBranchCounter = 0;


/******************************************************************************
 * Helper Functions
 *****************************************************************************/
uint32_t get_bits(uint32_t value, unsigned start, unsigned end) {
	// Get bits from start to end (inclusive)
	if (start > end || start < 0 || end >= 32) {
		cerr << "Error: get_bits recieved an invalid bit range" << endl;
		return 0;
	}
	// printf("\nget_bits(%u, %u, %u)\n", value, start, end); // TODO: Remove
	uint32_t mask;
	mask = (1u << (end - start + 1u)) - 1u;
	mask = mask << start;

	
	// printf("value: 0b%s\n", std::bitset<32>(value).to_string().c_str()); // TODO: Remove
	// printf("Mask: 0b%s\n", std::bitset<32>(mask).to_string().c_str());
	// printf("result: 0b%s\n", std::bitset<32>((value & mask) >> start).to_string().c_str());
	return (value & mask) >> start;
}

int log2(int x) {
	int log = 0;
	while (x > 1) {
		x >>= 1;
		log++;
	}
	return log;
}

uint32_t calculate_tag(uint32_t pc) {
	unsigned start = 2 + log2(globalBTB.get_btbSize());
	return get_bits(pc, start, start + globalBTB.get_tagSize() - 2);
}

int power(int base, int exp) {
	int result = 1;
	for (int i = 0; i < exp; ++i) {
		result *= base;
	}
	return result;
}
 /*****************************************************************************
 * Target Functions
 *****************************************************************************/

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared) {
	// Initialize global variables
	gInitalFsmState = fsmState;
	gIsGlobalHist = isGlobalHist;
	gIsGlobalTable = isGlobalTable;
	gXORMethod = Shared;

	// Initialize the BTB
	// globalBTB = new BTB(btbSize, historySize, tagSize, fsmState);
	globalBTB = BTB(btbSize, historySize, tagSize, fsmState);
	globalHistory = History(historySize);

	// if (isGlobalHist) {
		// globalHistory = new History(historySize);
	// }

	if (isGlobalTable) {
		for (unsigned i = 0; i < btbSize; ++i) {
			FSM &currentFSM = globalFSMVector[i];
			currentFSM.set_fsmState(fsmState);
			currentFSM.set_fsmSize(FSM_SIZE);
		}
	}

	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst) {
	lastTaken = globalBTB.get_BTB_entry_prediction(calculate_tag(pc), pc, dst);
	return lastTaken;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
	// Update stats
	gBranchCounter++;
	// if (pred_dst != targetPc) {
	if (taken != lastTaken) {
		gFlushCounter++;
	}
	// Update the BTB
	globalBTB.update_BTB_entry(calculate_tag(pc), pc, targetPc, taken);
}

void BP_GetStats(SIM_stats *curStats) {
	// Update the stats
	curStats->flush_num = gFlushCounter;
	curStats->br_num = gBranchCounter;
	curStats->size = globalBTB.get_btbSize() * (log2(globalBTB.get_btbSize()) +  globalBTB.get_tagSize() + 30);
	if (gIsGlobalHist) {
		curStats->size += globalBTB.get_historySize();
	} else {
		curStats->size += globalBTB.get_historySize() * globalBTB.get_btbSize();
	}
	if (gIsGlobalTable) {
		curStats->size += 2 * power(2, globalBTB.get_historySize());
	} else {
		curStats->size += globalBTB.get_btbSize() * 2 * power(2, globalBTB.get_historySize());
	}
}

/******************************************************************************
 * BTBEntry Class Implementation
 *****************************************************************************/
BTBEntry::BTBEntry(uint32_t tag, uint32_t targetPc, unsigned historySize, unsigned initailFsmState)
	: tag(tag), targetPc(targetPc), history(historySize) {
	for (unsigned i = 0; i < FSM_VECTOR_MAX_SIZE; ++i) {
		this->fsmVec[i].set_fsmState(initailFsmState);
	}
}

BTBEntry& BTBEntry::operator=(const BTBEntry& other) {
	if (this == &other) {
		return *this;
	}
	this->tag = other.tag;
	this->targetPc = other.targetPc;
	this->history = other.history;
	return *this;
}

// Setters and Getters
uint32_t BTBEntry::get_tag() const { return this->tag; }
uint32_t BTBEntry::get_targetPc() const { return this->targetPc; }
uint32_t BTBEntry::get_history() const { return gIsGlobalHist ? globalHistory.get_history() : this->history.get_history(); }
bool BTBEntry::get_isTaken(int fsmIndex) const { return this->fsmVec[fsmIndex].get_fsmIsTaken(); }

void BTBEntry::set_tag(uint32_t tag) { this->tag = tag; }
void BTBEntry::set_targetPc(uint32_t targetPc) { this->targetPc = targetPc; }

// Functions
void BTBEntry::update_history(bool taken) { this->history.update_history(taken); }
void BTBEntry::update_fsm(int fsmIndex, bool taken) { this->fsmVec[fsmIndex].update_fsm(taken); }

void BTBEntry::recreate_entry(uint32_t tag, uint32_t targetPc) {
	this->tag = tag;
	this->targetPc = targetPc;
	this->history = History(globalBTB.get_historySize());
	for (unsigned i = 0; i < FSM_VECTOR_MAX_SIZE; ++i) {
		this->fsmVec[i].set_fsmState(gInitalFsmState);
	}
}

/******************************************************************************
 * BTB Class Implementation
 *****************************************************************************/
BTB::BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState)
	: btbSize(btbSize), historySize(historySize), tagSize(tagSize) {
	this->fsmState = FSM(fsmState, FSM_SIZE);
	// this->btbEntries = new BTBEntry[btbSize];
	for (unsigned i = 0; i < btbSize; ++i) {
		this->btbEntries[i] = BTBEntry(0, 0, historySize);
	}
}

BTB::~BTB() {
	// delete[] this->btbEntries;
	// this->btbEntries = nullptr;
}

BTB& BTB::operator=(const BTB& other) {
	if (this == &other) {
		return *this;
	}
	this->btbSize = other.btbSize;
	this->historySize = other.historySize;
	this->tagSize = other.tagSize;
	this->fsmState = other.fsmState;
	for (unsigned i = 0; i < btbSize; ++i) {
		this->btbEntries[i] = other.btbEntries[i];
	}
	return *this;
}

bool BTB::get_BTB_entry_prediction(uint32_t tag, uint32_t pc, uint32_t *dst) {
	// Direct access to the BTB entry
	BTBEntry &entry = btbEntries[get_bits(pc, 2, 2 + log2(btbSize) - 1)];

	// If the entry exists, return its prediction
	// TOASK: What if both tag and LSB bits are the same for two different branches?
	if (entry.get_tag() == tag) {
		bool isTaken;
		int fsmIndex = entry.get_history();
		if (gIsGlobalHist) {
			switch (gXORMethod) {
				case 0: // Not using XOR
					break;
				
				case 1: // Using XOR on LSB (from the third bit)
					fsmIndex ^= get_bits(pc, 2, 2 + this->get_historySize() - 1);
					break;
			
				case 2: // Using XOR on mid (from the 17h bit)
					fsmIndex ^= get_bits(pc, 16, 16 + this->get_historySize() - 1);
					break;
				
				default:
					cerr << "Invalid Shared value: " << gXORMethod << endl;
					break;
				}
		} 
		if (gIsGlobalTable) {
			isTaken = globalFSMVector[fsmIndex].get_fsmIsTaken();
		} else {
			isTaken = entry.get_isTaken(fsmIndex);
		}
		*dst = isTaken ? entry.get_targetPc() : pc + 4;
		return isTaken;
	}
	
	// Default target address
	*dst = pc + 4;
	return false; 
}

void BTB::update_BTB_entry(uint32_t tag, uint32_t pc, uint32_t targetPc, bool taken) {
	// Direct access to the BTB entry
	BTBEntry &entry = btbEntries[get_bits(pc, 2, 2 + log2(btbSize) - 1)];
	
	// If the entry does not exists create a new one
	if (entry.get_tag() != tag) {
		entry.recreate_entry(tag, targetPc);
	}
	
	// Update the FSM state
	int fsmIndex = entry.get_history();
	if (gIsGlobalHist) {
		switch (gXORMethod) {
			case 0: // Not using XOR
			break;
			
			case 1: // Using XOR on LSB (from the third bit)
			fsmIndex ^= get_bits(pc, 2, 2 + this->get_historySize() - 1);
			break;
			
			case 2: // Using XOR on mid (from the 17h bit)
			fsmIndex ^= get_bits(pc, 16, 16 + this->get_historySize() - 1);
			break;
			
			default:
			cerr << "Invalid Shared value: " << gXORMethod << endl;
			break;
		}
	}
	if (gIsGlobalTable) {
		globalFSMVector[fsmIndex].update_fsm(taken);
	} else {
		entry.update_fsm(fsmIndex, taken);
	}
	
	// Update History
	if (gIsGlobalHist) {
		globalHistory.update_history(taken);
	} else {
		entry.update_history(taken);
	}
}



/******************************************************************************
 * Test Functions
 *****************************************************************************/
void test_history() {
	History history(3);
	cout << " History size: 3" << endl;
	cout << "History initial state: " << bitset<32>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 1 - push 1 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 2 - push 0 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 3 - push 1 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 4 - push 1 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 5 - push 0 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 6 - push 1 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 7 - push 0 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(false);
	cout << "Step 8 - push 0 - History: " << bitset<32>(history.get_history()) << endl;
	history.update_history(true);
	cout << "Step 9 - push 1 - History: " << bitset<32>(history.get_history()) << endl;
}

void test_fsm() {
	FSM fsm(2, 4);
	cout << "FSM initial state: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(true);
	cout << "Step 1 - push 1 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(false);
	cout << "Step 2 - push 0 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(false);
	cout << "Step 3 - push 0 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(false);
	cout << "Step 4 - push 0 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(false);
	cout << "Step 5 - push 0 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(true);
	cout << "Step 6 - push 1 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(true);
	cout << "Step 7 - push 1 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(true);
	cout << "Step 8 - push 1 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(false);
	cout << "Step 9 - push 0 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(true);
	cout << "Step 10 - push 1 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
	fsm.update_fsm(true);
	cout << "Step 11 - push 1 - FSM: " << fsm.get_fsmState() << " - Taken: " << fsm.get_fsmIsTaken() << endl;
}

void test_power() {
	cout << "Power(2, 3) = " << power(2, 3) << endl;
	cout << "Power(2, 4) = " << power(2, 4) << endl;
	cout << "Power(2, 5) = " << power(2, 5) << endl;
	cout << "Power(3, 3) = " << power(3, 3) << endl;
	cout << "Power(4, 4) = " << power(4, 4) << endl;
	cout << "Power(5, 5) = " << power(5, 5) << endl;
}

void test_log2() {
	cout << "log2(1) = " << log2(1) << endl;
	cout << "log2(2) = " << log2(2) << endl;
	cout << "log2(3) = " << log2(3) << endl;
	cout << "log2(4) = " << log2(4) << endl;
	cout << "log2(5) = " << log2(5) << endl;
	cout << "log2(6) = " << log2(6) << endl;
	cout << "log2(7) = " << log2(7) << endl;
	cout << "log2(8) = " << log2(8) << endl;
}

void test_get_bits() {
	// cout << "get_bits(0b11111111, 0, 7) = " << bitset<8>(get_bits(0b11111111, 0, 7)) << endl;
	// cout << "get_bits(0b11111111, 1, 6) = " << bitset<8>(get_bits(0b11111111, 1, 6)) << endl;
	// cout << "get_bits(0b11111111, 2, 5) = " << bitset<8>(get_bits(0b11111111, 2, 5)) << endl;
	// cout << "get_bits(0b11111111, 3, 4) = " << bitset<8>(get_bits(0b11111111, 3, 4)) << endl;
	// cout << "get_bits(0b11111111, 4, 3) = " << bitset<8>(get_bits(0b11111111, 4, 3)) << endl;
	// cout << "get_bits(0b11001100, 4, 4) = " << bitset<8>(get_bits(0b11001100, 4, 4)) << endl;
	// cout << "get_bits(0b11001100, 3, 3) = " << bitset<8>(get_bits(0b11001100, 3, 3)) << endl;
	cout << "get_bits(0b11001100, 2, 6) = " << bitset<8>(get_bits(0b11001100, 2, 6)) << endl;
	cout << "get_bits(0b11001100, 2, 7) = " << bitset<8>(get_bits(0b11001100, 2, 7)) << endl;
	cout << "get_bits(0b11001100, 0, 4) = " << bitset<8>(get_bits(0b11001100, 0, 4)) << endl;
	cout << "get_bits(0b11001100, 0, 3) = " << bitset<8>(get_bits(0b11001100, 0, 3)) << endl;
}

void test_calculate_tag() {
	globalBTB = BTB(4, 4, 3, 4);
	cout << "calculate_tag(0b00110011) = " << bitset<8>(calculate_tag(0b00110011)) << endl;
	cout << "calculate_tag(0b11111111) = " << bitset<8>(calculate_tag(0b11111111)) << endl;
	cout << "calculate_tag(0b01010101) = " << bitset<8>(calculate_tag(0b01010101)) << endl;
	cout << "calculate_tag(0b11100011) = " << bitset<8>(calculate_tag(0b11100011)) << endl;
}
/******************************************************************************
 * Main Function for Testing
 *****************************************************************************/
int _main() {
	// test_history();
	// test_fsm();
	// test_power();
	// test_log2();
	test_get_bits();
	// test_calculate_tag();

	return 0;
}

/* Output:
0x1230 N 0x1234
0x87654 N 0x87658
0x1230 N 0x1234
0x87654 N 0x87658
0x1230 N 0x1234
0x87654 N 0x87658
0x87654 N 0x87658
0x10c N 0x110
0x87654 N 0x87658
flush_num: 9, br_num: 9, size: 134b
*/