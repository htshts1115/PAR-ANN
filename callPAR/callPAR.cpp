// callPAR.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

int main()
{
	//system(("curl.exe -b cookie.txt -d test=" + line + "  http://example.com").c_str());
	
	std::string str = "\"C:\\Users\\Tianshen Huang\\Documents\\PRM\\PAR\\PAR ANN\\x64\\Release\\PAR-GO.exe\"";

	for (int i = 0; i < 30; i++) {
		system((str+" "+ std::to_string(i)).c_str());
	}
	
	//system("\"C:\\Users\\Tianshen Huang\\Documents\\PRM\\PAR\\PAR ANN\\x64\\Release\\PAR-GO.exe\" 2");





	//auto str = _T("1");
	//DWORD exitCode = 0;
	//SHELLEXECUTEINFO ShExecInfo = { 0 };
	//ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	//ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	//ShExecInfo.hwnd = NULL;
	//ShExecInfo.lpVerb = _T("open");
	//ShExecInfo.lpFile = _T("XXX.exe");
	//ShExecInfo.lpParameters = str;
	//ShExecInfo.lpDirectory = 0;
	//ShExecInfo.nShow = SW_SHOW;
	//ShExecInfo.hInstApp = NULL;
	//ShellExecuteEx(&ShExecInfo);
	
	//if (WaitForSingleObject(ShExecInfo.hProcess, INFINITE) == 0) {
	//	GetExitCodeProcess(ShExecInfo.hProcess, &exitCode);
	//	if (exitCode != 0) {
	//		return false;
	//	}
	//	else {
	//		return true;
	//	}
	//}
	//else {
	//	return false;
	//}

	return 0;
}

