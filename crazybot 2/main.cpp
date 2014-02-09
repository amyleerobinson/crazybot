// Don't make me explain all the basic libraries.
#include <iostream>

#include "CFramework.h" // We're gonna need this!


// You should really know what this function is for if you plan to
// use/contribute to the code.
int main()
{
	CFramework frmwrk;
	frmwrk.Init();
	char c;
	do {
		std::cin.get(c);
	} while (c != 'e');
	return 0;
}