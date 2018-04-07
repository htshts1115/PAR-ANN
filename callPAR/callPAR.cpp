// callPAR.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <string>
#include <iostream>
#include <conio.h>

using namespace std;

int main(int argc, const char *argv[])
{
	//std::string str = "\"C:\\Users\\Tianshen Huang\\Documents\\PRM\\PAR\\PAR ANN\\x64\\Release\\PAR-GO.exe\"";
	
	std::cout << "Please drop the PAR-GO.exe and press [Enter] when done ...\n";
	std::string file_name;
	for (int ch = _getch(); ch != '\r'; ch = _getch()) {
		file_name += ch;
		while (_kbhit())
			file_name += _getch();
	}

	for (int i = 0; i < 30; i++) {
		system((file_name +" "+ std::to_string(i)).c_str());
		//_getch();
	}



	//std::vector< std::string > files;
	//for (int ch = _getch(); ch != '\r'; ch = _getch()) {
	//	std::string file_name;
	//	if (ch == '\"') {  // path containing spaces. read til next '"' ...
	//		while ((ch = _getch()) != '\"')
	//			file_name += ch;
	//	}
	//	else { // path not containing spaces. read as long as chars are coming rapidly.
	//		file_name += ch;
	//		while (_kbhit())
	//			file_name += _getch();
	//	}
	//	files.push_back(file_name);
	//}
	//std::cout << "You dropped these files:\n";
	//for (auto & i : files)
	//	std::cout << i << '\n';
	//_getch();
	//return 0;
}

