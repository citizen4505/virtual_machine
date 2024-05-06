#include "vm.h"
#include "debugger.h"
int main() {

	VM vm;
	// predame debuggeru referenci na instanci VM, at s ni muze pracovat
	DBG dbg(&vm);
	// nahrani binarky do pameti stroje
	vm.load("rogue.obj");
	vm.run(true);
	return 0;
}