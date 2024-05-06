#include <iostream>
#include "vm.h"

class DBG{
public:
	// konstruktor prebira ukazatel na instanci objektu VM
	VM* vm;

	DBG();
	DBG(VM*);
	void registerDump(int);
	void memoryDump(uint16_t);
	void memoryDump(uint16_t, uint16_t);
	void registerAnalyze(void);
	void instructionAnalyze(void);
};