////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "systemclass.h"

int main() {
	SystemClass *System;
	bool result;

	// Создайте системный объект.
	System = new SystemClass;
	if (!System) {
		return 0;
	}

	// Инициализировать системный объект.
	result = System->Initialize();
	if (!result) {
		return 0;
	}

	// Зациклите и запустите функцию системного фрейма.
	while (System->Online() == true) {
		System->Frame();
	}

	// Освободите системный объект.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}
