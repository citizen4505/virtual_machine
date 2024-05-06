#include "debugger.h"

DBG::DBG() {

}

DBG::DBG(VM* p_vm) {
	// ukazatel predany v parametru si ulozime do instancni ukazatelove promenne, pres kterou budeme pristupovat k verejnym metodam objektu VM
	vm = p_vm;
}

void DBG::registerDump(int register_index) {
	uint16_t regVal = vm->getRegisterValue(register_index);
	std::cout << std::hex << regVal << std::endl;
}

void DBG::memoryDump(uint16_t memory_address) {
	uint16_t memVal = vm->getMemoryValue(memory_address);
	std::cout << std::hex << "0x" << memory_address << ":" << memVal << std::endl;
}

void DBG::memoryDump(uint16_t start_memory_address, uint16_t end_memory_address) {
	for (uint16_t i = start_memory_address; i <= end_memory_address; i++) {
		uint16_t memVal = vm->getMemoryValue(i);
		std::cout << std::hex << "0x" << i << ":" << memVal << std::endl;
	}
}

void DBG::registerAnalyze(void) {
	std::cout << "==================================================" << std::endl;
	std::cout << "Registers:" << std::endl;
	std::cout << "==================================================" << std::endl;

	std::cout << "PC: " << std::hex << "x" << vm->getRegisterValue(8) << std::endl;
	std::cout << "Z: " << std::hex << "x" << vm->getRegisterValue(9) << " | "
		<< "P: " << std::hex << "x" << vm->getRegisterValue(10) << " | "
		<< "N: " << std::hex << "x" << vm->getRegisterValue(11) << std::endl;
	std::cout << "R0: " << std::hex << "x" << vm->getRegisterValue(0) << " | "
		<< "R1: " << std::hex << "x" << vm->getRegisterValue(1) << " | "
		<< "R2: " << std::hex << "x" << vm->getRegisterValue(2) << " | "
		<< "R3: " << std::hex << "x" << vm->getRegisterValue(3) << " | "
		<< "R4: " << std::hex << "x" << vm->getRegisterValue(4) << " | "
		<< "R5: " << std::hex << "x" << vm->getRegisterValue(5) << " | "
		<< "R6: " << std::hex << "x" << vm->getRegisterValue(6) << " | "
		<< "R7: " << std::hex << "x" << vm->getRegisterValue(7) << std::endl;
}

void DBG::instructionAnalyze(void) {
	uint16_t instruction = vm->getMemoryValue(vm->getRegisterValue(8));
	const char* instruction_name[] = { "BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR", "RTI", "NOT", "LDI", "STI", "JMP", "RES", "LEA", "TRAP" };

	std::cout << "==================================================" << std::endl;
	std::cout << "Instruction:" << std::endl;
	std::cout << "==================================================" << std::endl;
	std::cout << std::hex << "x" << vm->getRegisterValue(8) << ":x" << instruction << " | "
		<< "OP: " << instruction_name[(instruction >> 12)] << " | ";
	switch (instruction >> 12) {
	case 0: {
		std::cout << std::hex 
			<< "N: x" << ((instruction >> 11) & 0x1) << " | "
			<< "Z: x" << ((instruction >> 10) & 0x1) << " | "
			<< "P: x" << ((instruction >> 9) & 0x1) << " | "
			<< "PCoffset9: x" << ((instruction) & 0x1ff)
			<< std::endl;
		break;
	}
	case 1: {
		// imm5
		if ((instruction >> 5) & 0x1){
			std::cout << std::hex
				<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
				<< "SR1: x" << ((instruction >> 6) & 0x7) << " | "
				<< "imm5: x" << (instruction & 0x3f)
				<< std::endl;
		}
		else {
			std::cout << std::hex
				<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
				<< "SR1: x" << ((instruction >> 6) & 0x7) << " | "
				<< "SR2: x" << (instruction & 0x7)
				<< std::endl;
		}
		break;
	}
	case 2: {
		std::cout << std::hex
			<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "PCoffset9: x" << ((instruction) & 0x1ff)
			<< std::endl;
		break;
	}
	case 3: {
		std::cout << std::hex
			<< "SR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "PCoffset9: x" << ((instruction) & 0x1ff)
			<< std::endl;
		break;
	}
	case 4: {
		// JSR
		if ((instruction >> 11) & 0x1) {
			std::cout << std::hex
				<< "PCoffset11: x" << (instruction & 0x7ff)
				<< std::endl;
		}
		else {
			std::cout << std::hex
				<< "BaseR: x" << ((instruction >> 6) & 0x7)
				<< std::endl;
		}
		break;
	}
	case 5: {
		// imm5
		if ((instruction >> 5) & 0x1) {
			std::cout << std::hex
				<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
				<< "SR1: x" << ((instruction >> 6) & 0x7) << " | "
				<< "imm5: x" << (instruction & 0x3f)
				<< std::endl;
		}
		else {
			std::cout << std::hex
				<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
				<< "SR1: x" << ((instruction >> 6) & 0x7) << " | "
				<< "SR2: x" << (instruction & 0x7)
				<< std::endl;
		}
		break;
	}
	case 6: {
		std::cout << std::hex
			<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "BaseR: x" << ((instruction >> 6) & 0x7) << " | "
			<< "offset6: x" << (instruction & 0x3f)
			<< std::endl;
		break;
	}
	case 7: {
		std::cout << std::hex
			<< "SR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "BaseR: x" << ((instruction >> 6) & 0x7) << " | "
			<< "offset6: x" << (instruction & 0x3f)
			<< std::endl;
		break;
	}
	case 8: {
		break;
	}
	case 9: {
		std::cout << std::hex
			<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "SR: x" << ((instruction >> 6) & 0x7) << " | "
			<< std::endl;
		break;
	}
	case 10: {
		std::cout << std::hex
			<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "PCoffset9: x" << ((instruction) & 0x1ff)
			<< std::endl;
		break;
	}
	case 11: {
		std::cout << std::hex
			<< "SR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "PCoffset9: x" << ((instruction) & 0x1ff)
			<< std::endl;
		break;
	}
	case 12: {
		std::cout << std::hex
			<< "BaseR: x" << ((instruction >> 6) & 0x7)
			<< std::endl;
		break;
		break;
	}
	case 13: {
		break;
	}
	case 14: {
		std::cout << std::hex
			<< "DR: x" << ((instruction >> 9) & 0x7) << " | "
			<< "PCoffset9: x" << ((instruction) & 0x1ff)
			<< std::endl;
		break;
		break;
	}
	case 15: {
		std::cout << std::hex
			<< "trapvect8: x" << ((instruction) & 0x7f)
			<< std::endl;
		break;
		break;
	}
	default: {
		break;
	}
	}
}