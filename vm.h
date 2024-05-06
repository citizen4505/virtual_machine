#pragma once
 
#include <iostream>
#include <fstream>

class VM{
public:
	/*
	*	Konstruktor vola metody pro inicializaci pameti a nulovani registru a flagu.
	*/
	VM();

	/*
	* Metoda prevezme binarni soubor. Nastavi PC, a zapise si program do pameti.
	* @param const char*: nazev souboru, nebo uplna cesta k souboru
	*/
	void load(const char*);

	/*
	* Zpracuje instrukci na adrese PC
	*/
	void step(void);

	/*
	* Zpracuje pocet instrukci podle predaneho parametru
	*/
	void step(int);

	/*
	* Zacne zpracovavat vsechny instrukce od aktualniho PC
	* @param bool: spusteni stroje na hodnote true
	*/
	void run(bool);

	/*
	* Vraci obsah pameti na adrese
	* @param uint16_t: adresa pameti zapsana v 16-tkove notaci
	* @return uint16_t: hexa obsah pameti na pozadovane adrese
	*/
	uint16_t getMemoryValue(uint16_t);

	/*
	* Vraci obsah registru dle parametru
	* @param int: index v poli registru
	* @return uint16_t: hodnota registru predaneho v parametru funkce
	*/
	uint16_t getRegisterValue(int);

protected:
	// pomocna promenna pro zastaveni stroje na instrukci HALT x25
	bool running = false;

	// Pametova oblast
	uint16_t memory[65535];

	// Oblast obsahu registru
	uint16_t registers[12];

	typedef enum {
		R0 = 0,
		R1,
		R2,
		R3,
		R4,
		R5,
		R6,
		R7,
		PC,
		ZRO,
		POS,
		NEG
	}Registers;

	typedef enum {
		BR = 0,
		/*
		* Aritmeticky soucet
		* 
		* Dekodovani:
		* --------------------------------------------------
		* |15       12|11     9|8         6|5|4  3|2      0|
		* |    0101   |   DR   |     SR    |0| 00 |   SR2  |
		* --------------------------------------------------
		* 
		* --------------------------------------------------
		* |15       12|11     9|8         6|5|4           0|
		* |    0101   |   DR   |     SR    |1|    IMM5     |
		* --------------------------------------------------
		* 
		* Pseudokod:
		* if (bit[5] == 0)
		*		DR = SR1 + SR2
		* else
		*		DR = SR1 + extend(IMM5)
		* 
		* Priklad:
		* ADD R2, R3, R4	; R2 <- R3 + R4
		* ADD R2, R3, #3	; R2 <- R3 + 3
		*/
		ADD = 1,
		LD = 2,
		ST = 3,
		JSR_JSRR = 4, // + JSRR
		AND = 5,
		LDR = 6,
		STR = 7,
		RTI = 8,
		NOT = 9,
		/*
		* Load indirect, nacte hodnotu z pameti do registru
		*
		* Dekodovani:
		* --------------------------------------------------
		* |15       12|11     9|8                         0|
		* |    1010   |   DR   |        PCoffset9          |
		* --------------------------------------------------
		*
		* Pseudokod:
		*
		* Priklad:
		* LDI R4, LOC	; R4 <- mem[mem[LOC]]
		*/
		LDI = 10,
		STI = 11,
		JMP_RET = 12, // + RET
		RES = 13,
		LEA = 14,
		TRAP = 15
	}instructions;

	typedef enum {
		TRAP_GETC = 0x20,	// ziska znak z klavesnice. Bez promptu
		TRAP_OUT = 0x21,	// znak na vystup
		TRAP_PUTS = 0x22,	// string na vystup
		TRAP_IN = 0x23,		// ziska znak z klavesnice. S promptem
		TRAP_PUTSP = 0x24,	// byte stringu na vystup
		TRAP_HALT = 0x25	// zastavi program
	}trap;

	/*
	* Metoda precte instrukci v pameti na kterou ukazuje registr PC a bitovym posunem ziska OP kod, ktery vraci.
	* return uint16_t : op kod 
	* return uint16_t : instrukce
	*/
	std::pair <uint16_t, uint16_t> fetch(void);
	
	/*
	* Metoda slouzi ke zpracovani dane instrukce
	* @param pair: operacni kod a instrukce pro zpracovani
	*/
	void eval(std::pair <uint16_t, uint16_t>);
	
	/*
	* Metoda doplni n-bitove cislo na velikost 16-ti bitu
	* @param uint16_t: predana instrukce, z ktere budeme zpracovavat pocet bitu dle druheho parametru
	* @param int: pocet bitu ke zpracovani
	* @return uint16_t: vraci n-bitove cislo doplnene o nuly nebo jednicky (dle typu instrukce)
	*/
	uint16_t extend(uint16_t, int);

	/*
	* Nastavi priznakove registry dle hodnoty registru v parametru
	* @param: uint16_t: predany registr podle ktereho nastavime znamenkove registry
	*/
	void update_flag(uint16_t);

	/*
	* Inicializace pameti na hodnoty 0xfd00.
	*/
	void resetMemory(void);
	
	/*
	* Vynulovani registru
	*/
	void resetRegisters(void);

	/*
	* Nuluje hodnoty priznakovych registru
	*/
	void resetFlags(void);
};
