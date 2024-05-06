#include "vm.h"
 
VM::VM() {
	this->resetMemory();
	this->resetRegisters();
	this->resetFlags();
}

void VM::load(const char* image_name) {
	unsigned char temp;
	uint8_t two_bytes[2] = {0x00, 0x00};
	bool bytes_for_pc = true;
	int memory_index = 0;
	int i = 0;

	// otevreme soubor ke cteni s testem na chybu
	std::ifstream rom_image = std::ifstream(image_name, std::ios::binary);
	if (!rom_image.is_open()) {
		std::cout << "Error: File path is wrong or file doesnt exist" << std::endl;
	}

	// REF.: https://www.programmersought.com/article/26696240059/
	// cteme data ze souboru po jednom bajtu za cyklus dokud tam neco je
	while (rom_image.read((char*)&temp, 1)) {
		
		// nabereme prvni dva bajty
		two_bytes[i] = temp;
		// inkrementujeme pro zapis dalsich dvou bajtu do pole 'two_bytes'
		i++;

		// jakmile nam inkrementor rekne ze jsou oba prvky pole naplneny
		if (i == 2) {
			// provedeme bitovy posun pro prvnich 8 bitu z prvniho precteneho bajtu a pridame k tomu zbyvajicich 8 bitu z druheho bajtu logickym OR
			uint16_t assembled_instruction = (two_bytes[0] << 8) | (two_bytes[1]);

			// DEBUG ONLY !!!!!!!!!!!!!!!!!!!!!!!!!!
			//std::cout << "DEBUG ONLY: " << std::hex << assembled_instruction << std::endl;

			// pokud jsme nabrali prvni dva bajty, musime si je ulozit do registru PC
			if (bytes_for_pc == true) {
				bytes_for_pc = false;
				this->registers[PC] = assembled_instruction;
				// nove nastaveny PC nam urcuje na jakou adresu budeme zapisovat precteny program do programove pameti
				memory_index = this->registers[PC];
			}
			else {
				// vysledne 2 bajty (16 bitu) zapiseme na adresu do pameti
				this->memory[memory_index] = assembled_instruction;
				memory_index++;
			}

			// nakonec si vsechno vynulujeme abysme mohli jit odznova
			two_bytes[0] = 0x00;
			two_bytes[1] = 0x00;
			assembled_instruction = 0x00;
			i = 0;
		}
	}

	// zavreme soubor
	rom_image.close();
}

std::pair <uint16_t, uint16_t> VM::fetch(void) {
	// precteme instrukci na adrese v pameti na kterou nam ukazuje PC
	uint16_t instruction = this->memory[this->registers[PC]];
	// bitovym posunem o 12 bitu ziskame posledni 4 bity instrukce (ta obsahuje operacni kod)
	uint16_t op = instruction >> 12;
	// inkrementujeme PC na dalsi adresu v pameti
	this->registers[PC]++;

	return std::make_pair(op, instruction);
}

void VM::eval(std::pair <uint16_t, uint16_t> op_and_instruction) {
	
	uint16_t op = op_and_instruction.first;
	uint16_t instruction = op_and_instruction.second;

	switch (op) {
		case this->BR: {
			uint16_t pc_offset = this->extend(instruction & 0x1ff, 9);
			uint16_t flag_negative = (instruction >> 11) & 1;
			uint16_t flag_zero = (instruction >> 10) & 1;
			uint16_t flag_positive = (instruction >> 9) & 1;
			uint16_t positive = (this->registers[POS] >> 12) & 1;
			uint16_t zero = ((this->registers[ZRO] >> 12) & 1);
			uint16_t negative = ((this->registers[NEG] >> 12) & 1);
			if ( ((flag_positive & positive) == 1) || ((flag_negative & negative) == 1) || ((flag_zero & zero) == 1)) {
				this->registers[PC] = this->registers[PC] + pc_offset;
			}
			break;
		}
		case this->ADD: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 6) & 0x7;
			// test na 5-ty bit instrukce > immediate mod
			uint16_t imm = (instruction >> 5) & 0x1;
			if (imm) {
				// prvni parametr pro metodu extend je upraven dle "instruction & 0x1f" abychom si vytahli pouze 5 bitu, ktere chceme zpracovat 
				uint16_t imm5 = this->extend(instruction & 0x1f, 5);
				// aritmeticky soucet registru a konstanty
				this->registers[dst_reg] = this->registers[src_reg] + imm5;
			}
			else {
				// pomoci masky "0x7" si vytahneme posledni 3 bity z instrukce k zadefinovani druheho zdrojoveho registru
				uint16_t second_src_reg = instruction & 0x7;
				// aritmeticky soucet obsahu dvou registru
				this->registers[dst_reg] = this->registers[src_reg] + this->registers[second_src_reg];
			}
			this->update_flag(this->registers[dst_reg]);
			break;
		}
		case this->LD: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// ziskame offset pro PC
			uint16_t pc_offset = this->extend((instruction & 0x1ff), 9);
			// nastaveni hodonty ciloveho registru podle adresy na PC + offset9
			this->registers[dst_reg] = this->memory[this->registers[PC] + pc_offset];
			this->update_flag(this->registers[dst_reg]);
			break;
		}
		case this->ST: {
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 9) & 0x7;
			// ziskame offset pro PC
			uint16_t pc_offset = this->extend((instruction & 0x1ff), 9);
			// na adresu v pameti na kterou ukazuje PC + offset, zapiseme obsah zdrojoveho registru
			this->memory[this->registers[PC] + pc_offset] = this->registers[src_reg];
			break;
		}
		case this->JSR_JSRR: {
			// ulozime adresu PC do registru R7
			this->registers[R7] = this->registers[PC] + 1;
			// JSR
			if ((instruction >> 11) & 0x1) {
				this->registers[PC] = (this->registers[PC] + 1) + extend((instruction & 0x7ff), 11);
			}
			// JSRR
			else {
				uint16_t baseR = (instruction >> 6) & 0x7;
				this->registers[PC] = this->registers[baseR];
			}
			break;
		}
		case this->AND: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 6) & 0x7;
			// test na 5-ty bit instrukce > immediate mod
			uint16_t imm = (instruction >> 5) & 0x1;
			if (imm) {
				// prvni parametr pro metodu extend je upraven dle "instruction & 0x1f" abychom si vytahli pouze 5 bitu, ktere chceme zpracovat 
				uint16_t imm5 = this->extend(instruction & 0x1f, 5);
				// logicky soucet registru a konstanty
				this->registers[dst_reg] = this->registers[src_reg] & imm5;
			}
			else {
				// pomoci masky "0x7" si vytahneme posledni 3 bity z instrukce k zadefinovani druheho zdrojoveho registru
				uint16_t second_src_reg = instruction & 0x7;
				// logicky soucet obsahu dvou registru
				this->registers[dst_reg] = this->registers[src_reg] & this->registers[second_src_reg];
			}
			this->update_flag(this->registers[dst_reg]);
			break;
		}
		case this->LDR: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 6) & 0x7;
			// nastaveni offsetu
			uint16_t offset = this->extend(instruction & 0x3f, 6);
			// nastaveni hodnoty ciloveho registru podle obsahu zdrojoveho registru + offsetu
			this->registers[dst_reg] = this->memory[this->registers[src_reg] + offset];
			this->update_flag(this->registers[src_reg]);
			break;
		}
		case this->STR: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 6) & 0x7;
			// nastaveni offsetu
			uint16_t offset = this->extend(instruction & 0x3f, 6);
			// zapis do pameti
			this->memory[this->registers[src_reg] + offset] = this->registers[dst_reg];
			break;
		}
		case this->RTI: {
			break;
		}
		case this->NOT: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 6) & 0x7;
			// negace
			this->registers[dst_reg] = ~this->registers[src_reg];
			this->update_flag(this->registers[dst_reg]);
			break;
		}
		case this->LDI: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// z instrukce vytahneme 9 bitu pro ziskani pc offsetu
			uint16_t pc_offset = extend(instruction & 0x1FF, 9);
			// pricteme pc_offset s registrem PC
			this->registers[dst_reg] = this->memory[this->registers[PC] + pc_offset];
			this->update_flag(dst_reg);
			break;
		}
		case this->STI: {
			// nastaveni zdrojoveho registru
			uint16_t src_reg = (instruction >> 9) & 0x7;
			// nastaveni pc offsetu
			uint16_t pc_offset = extend(instruction & 0x1FF, 9);
			// zapis do pameti
			//this->memory[this->memory[this->registers[PC] + pc_offset]] = this->registers[src_reg];
			this->memory[this->registers[PC] + pc_offset] = this->registers[src_reg];
			break;
		}
		case this->JMP_RET: {
			// nastaveni zdrojoveho registru pro adresu PC
			uint16_t src_reg = (instruction >> 6) & 0x7;
			if (src_reg == 0x7) {
				// nastaveni hodnoty PC podle obsahu R7 (instrukce RET)
				this->registers[PC] = this->registers[R7];
			}
			else {
				// nastaveni nove hodnoty PC
				this->registers[PC] = this->registers[src_reg];
			}
			break;
		}
		case this->RES: {
			break;
		}
		case this->LEA: {
			// nastaveni ciloveho registru
			uint16_t dst_reg = (instruction >> 9) & 0x7;
			// nastaveni pc offsetu
			uint16_t pc_offset = extend(instruction & 0x1FF, 9);
			// ulozeni adresy (pc_offset + PC) do registru
			this->registers[dst_reg] = this->registers[PC] + pc_offset;
			this->update_flag(dst_reg);
			break;
		}
		case this->TRAP: {
			switch (instruction & 0xff) {
				case this->TRAP_GETC: {
					// vezme se znak ze stdin a pretypuje se. Vysledek jde do registru R0
					this->registers[R0] = (uint16_t)getchar();
					break;
				}
				case this->TRAP_OUT: {
					std::cout << (char)this->registers[R0];
					break;
				}
				case this->TRAP_PUTS: {
					// ukazatel prevezme adresu prvku pole "memory", index prvku se bere z registru R0
					uint16_t* c = &this->memory[this->registers[R0]];
					// dokud ukazatel ukazuje na hodnotu rozdilnou od nuly
					while (*c) {
						// hodnota na kterou se ukazuje, se pretypuje na typ "char" a pusti se na stdout
						// std::cout << (char)*c;
						putc((char)*c, stdout);
						++c;
					}
					fflush(stdout);
					this->update_flag(this->registers[R0]);
					break;
				}
				case this->TRAP_IN: {
					std::cout << "Enter a character: ";
					char c;
					std::cin >> c;
					this->registers[R0] = (uint16_t)c;
					this->update_flag(this->registers[R0]);
					break;
				}
				case this->TRAP_PUTSP: {
					// ukazatel prevezme adresu prvku pole "memory", index prvku se bere z registru R0
					uint16_t* c = &this->memory[this->registers[R0]];
					// dokud ukazatel ukazuje na hodnotu rozdilnou od nuly
					while (*c != 0x000A) {
						char char1 = (*c) & 0xff;
						//putc(char1, stdout);
						std::cout << char1;
						char char2 = (*c) >> 8;
						if (char2) {
							//putc(char2, stdout);
							std::cout << char2;
						}
						++c;
					}
					//fflush(stdout);
					break;
				}
				case this->TRAP_HALT: {
					std::cout << "HALT" << std::endl;
					// poslat signal pro zastaveni stroje
					this->running = false;
					break;
				}
			}
			break;
		}
		default: {
			// spatny op kod
			break;
		}
	}
}

uint16_t VM::extend(uint16_t x, int bit_count) {
	// testujeme n-ty bit (bit_count - 1) na hodnotu 1
	if ((x >> (bit_count - 1)) & 1) {
		// vezmeme puvodni hodnotu a doplnime ji o hodnoty 0xffff. Puvodnich n-bitu  (bit_count) zustava zachovano.
		x |= (0xffff << bit_count);
	}
	return x;
}

void VM::resetMemory(void) {
	for (uint16_t i = 0x0000; i < 0xffff; i++) {
		this->memory[i] = 0xfd00;
	}
}

void VM::resetRegisters(void) {
	this->registers[R0] = 0x0000;
	this->registers[R1] = 0x0000;
	this->registers[R2] = 0x0000;
	this->registers[R3] = 0x0000;
	this->registers[R4] = 0x0000;
	this->registers[R5] = 0x0000;
	this->registers[R6] = 0x0000;
	this->registers[R7] = 0x0000;
	this->registers[NEG] = 0x0000;
	this->registers[ZRO] = 0x0000;
	this->registers[POS] = 0x0000;
}

void VM::update_flag(uint16_t r) {
	this->resetFlags();

	if (r == 0) {
		this->registers[ZRO] = 0xffff;
	}
	else if (r >> 15) {
		this->registers[NEG] = 0xffff;
	}
	else {
		this->registers[POS] = 0xffff;
	}
}

void VM::resetFlags(void) {
	this->registers[NEG] = 0x0000;
	this->registers[POS] = 0x0000;
	this->registers[ZRO] = 0x0000;
}

uint16_t VM::getRegisterValue(int register_index) {
	return this->registers[register_index];
}

uint16_t VM::getMemoryValue(uint16_t memory_address) {
	return this->memory[memory_address];
}

void VM::step(void) {
	this->eval(this->fetch());
}

void VM::step(int number_of_steps) {
	for (int i = 0; i < number_of_steps; i++) {
		this->eval(this->fetch());
	}
}

void VM::run(bool running = true) {
	uint16_t startPC = this->registers[PC];
	this->running = running;

	while(running) {
		this->eval(this->fetch());
		// konec pametoveho prostoru
		if (this->registers[PC] == 0xffff) {
			this->registers[PC] = startPC;
		}
		// pokud je aktualni instrukce HALT
		if (this->running == false) {
			break;
		}
	}
}