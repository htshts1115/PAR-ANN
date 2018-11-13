// PAR.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DoPAR.h"


int main(int argc, const char* argv[])
{


	//SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);	//!ignore error!!
	string CurWorkExeFile = argv[0];

	cout		 << "=============================================================================";
	cout << endl << "Software:	(PAR-GO) Pore Architecture Reconstruction using Global Optimization";
	cout << endl << "Institution:	Heriot-Watt University, 2018";
	cout << endl << "Authors:	T. Huang, Z. Jiang*, M. I. J. van Dijke, S. Geiger";
	cout << endl << "Contact:	Zeyun.Jiang@hw.ac.uk";
	cout << endl << "=============================================================================";

	//check time
	time_t CurTime, MaxTime;
	time(&CurTime);
	CurTime /= 86400L;
	//cout << endl << CurTime; _getch(); exit(0);		//17848 = 13/11/2018
	MaxTime = (time_t)(17848 + 100);				//add 30*6 days
	if (CurTime > MaxTime) {
		cout << endl << "Code expired. Please contact the author.";
		_getch();
		exit(0);
	}

	// set global OPENMP CPU
	int tempcore, tempthread;
	int MaxThread = omp_get_num_procs();
	cout << endl<< endl << "Select maximum cores for CPU parallelization, no more than " << MaxThread / 2 << endl;
	cin >> tempcore;
	tempthread = max(1, tempcore * 2);
	omp_set_dynamic(0); // Explicitly disable dynamic teams
	if (tempthread <= MaxThread) 	   
		omp_set_num_threads(tempthread);
	else 
		omp_set_num_threads(MaxThread);
	

	int i = 0;
	while (true)
	{
		DoPAR DoItNow;
		DoItNow.GetStarted(CurWorkExeFile, i);
		i++;
	}



	return 0;
}


//std::string strSearch = "C:\\Users\\dell\\Downloads\\FractureZone_8x\\FractureZone*";
//WIN32_FIND_DATAA ffd;
//HANDLE hFind = FindFirstFileA(strSearch.c_str(), &ffd);
//
//do
//{
//	std::string strFile = ffd.cFileName;
//	cout << endl << strFile;
//	_getch();
//} while (FindNextFileA(hFind, &ffd) != 0);