﻿#include "stdafx.h"

#include "DoPAR.h"


DoPAR::DoPAR(){	

}

DoPAR::~DoPAR(){
}

long DoPAR::FileLength(const string& FName)
{
	ifstream InF(FName.c_str(), ios::in | ios::binary);
	if (!InF) return 0;

	InF.seekg(0, InF.end);
	long Length = InF.tellg() / sizeof(char);
	InF.seekg(0, InF.beg);

	InF.close();

	return Length;
}

//bool DoPAR::Read(const string FPathName, vector<uchar>& Data)
//{
//	Data.clear();
//
//	long Length = FileLength(FPathName.c_str());
//	if (Length < 1) {
//		//cout << endl << "File is empty!";
//		return false;
//	}
//
//	ifstream InF(FPathName.c_str(), ios::in | ios::binary);
//	if (!InF) return false;
//
//	Data.reserve(Length);
//
//	long BlockNum = 1000000L; //~1M
//
//	long ActualBlkNum = floor(1.0*Length / BlockNum);
//
//	if (ActualBlkNum > 0) {//Read the file one block after another
//		const long BlockSize = sizeof(char)*BlockNum;
//
//		char* Mblock = new char[BlockSize];
//
//		for (long RowNum = 0; RowNum < ActualBlkNum; ++RowNum) {
//			if (!InF.read(reinterpret_cast<char *>(Mblock), BlockSize))
//			{
//				delete[] Mblock; InF.close(); return false;
//			}
//
//			for (long idx = 0; idx<BlockNum; ++idx)
//				Data.push_back(Mblock[idx]);
//		}
//
//		delete[] Mblock;
//	}//Read the file one block after another
//
//	BlockNum = Length - ActualBlkNum*BlockNum;
//
//	if (BlockNum > 0) {
//		const long BlockSize = sizeof(char)*BlockNum;
//		char* Mblock = new char[BlockSize];
//
//		if (!InF.read(reinterpret_cast<char *>(Mblock), BlockSize))
//		{
//			delete[] Mblock; InF.close(); return false;
//		}
//
//		for (long idx = 0; idx<BlockNum; ++idx)
//			Data.push_back(Mblock[idx]);
//
//		delete[] Mblock;
//	}
//
//	InF.close();  return true;
//}

inline bool fileExists(const string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool DoPAR::Write(const string FPathName, vector<uchar> Data)
{
	if (FPathName.size() == 0) return false;
	if (Data.size() == 0) return false;

	int i(1);
	string tempFPathName2D = FPathName;
	while (fileExists(tempFPathName2D) == true) {
		tempFPathName2D = FPathName.substr(0, FPathName.find('.')) + "_" + to_string(i) + ".RAW";
		i++;
	}
	ofstream OFile(tempFPathName2D.c_str(), ios::binary | ios::ate);
	if (!OFile) { cout << endl << "!OFile"; return false; }

	unsigned __int64 FSize = Data.size();
	const long BlockSize = sizeof(char)*iSTEPLENGTH;

	char* Buffer = new char[sizeof(char)*iSTEPLENGTH];
	long Steps = FSize / iSTEPLENGTH;
	long RemainNum = FSize % iSTEPLENGTH;

	long idx, DataIdx = 0;
	for (idx = 0; idx < Steps; ++idx) {
		for (int i = 0; i < iSTEPLENGTH; ++i) Buffer[i] = Data[DataIdx++];
		if (!OFile.write(reinterpret_cast<char *>(Buffer), BlockSize)) {
			delete[] Buffer; return false;
		}
	}

	delete[] Buffer;

	if (RemainNum > 0) {
		char* Buff = new char[sizeof(char)*RemainNum];
		for (int i = 0; i < RemainNum; ++i) Buff[i] = Data[DataIdx++];
		if (!OFile.write(reinterpret_cast<char *>(Buff), sizeof(char)*RemainNum)) {
			delete[] Buff; return false;
		}
		delete[] Buff;
	}

	OFile.close();

	return true;
}

bool DoPAR::GetNextRowParameters(short Cno, vector<string>& ValidParStr, vector<string>& ParV)
{
	if ((unsigned short)Cno >= ValidParStr.size()) {
		cout << endl << "Wrong arguments!";
		char ch; cin >> ch; exit(1);
	}

	ParV.clear();

	string TmpS = "";
	for (unsigned long Idx = 0; Idx < ValidParStr[Cno].size(); ++Idx) {
		if (ValidParStr[Cno][Idx] == ',' || ValidParStr[Cno][Idx] == ' ' || ValidParStr[Cno][Idx] == '\n') {
			if (TmpS.size() > 0) { ParV.push_back(TmpS); }
			TmpS = "";
		}
		else { TmpS += ValidParStr[Cno][Idx]; }
	} //for(long InIdx=0; InIdx<LineLst1[LNidx].size(); ++InIdx)
	if (TmpS.size() > 0) { ParV.push_back(TmpS); }

	return true;
}

bool DoPAR::iFileExistYN(const string& PFileName)
{
	struct stat stFileInfo;
	int intStat = stat(PFileName.c_str(), &stFileInfo);
	if (intStat != 0) return false;
	return true;
}

//void DoPAR::ReadRunPar(string CurExeFile)
//{
//	cout << endl << "===========================================";
//	cout << endl << "Set up your running parameters...    ";
//	cout << endl << "===========================================";
//
//	string tempoutputfilename;
//	string tempoutputformat;
//	string parametername;
//
//	string Path;
//	vector<string> ResLines;
//	{//Read setup file
//		string tmpstr, name;
//		iCGetDirFileName(CurExeFile, Path, name);
//		//string PFName = Path + "PAR-GO_Setup_series.DAT";//!changed to series 
//		string PFName = Path + "PAR-GO_Setup.DAT";
//		vector<string> TmpLines;
//		if (!ReadTxtFiles(PFName, TmpLines)) {
//			cout << endl;
//			cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
//			cout << endl << " Failed to open file '" << PFName.c_str() << "' !";
//			cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
//			cout << endl;
//			cout << endl << "Press any key to quit ...";
//			_getch(); exit(1);
//		}
//
//		for (unsigned long Row = 0; Row < TmpLines.size(); Row++) {
//			string::size_type Posidx = TmpLines[Row].find("//");
//			if (Posidx != string::npos) {
//				continue;
//			}
//			else {
//				Posidx = TmpLines[Row].find("/*");
//				if (Posidx != string::npos) continue;
//			}
//			ResLines.push_back(TmpLines[Row]);
//		} //for(int Row=0; Row < LineLst.size(); Row++)
//
//		if (ResLines.size() == 0) {
//			cout << endl << " ============================================================";
//			cout << endl << " Failed to open PAR-GO_Setup.DAT in current working directory !";
//			cout << endl << " ============================================================";
//			cout << endl << " Press any key to quit....";
//			_getch(); exit(1);
//		}
//	}//Read setup file
//	short Row(0);
//	
//	///////////////////////// check random seed
//	vector<string> ParV;
//	GetNextRowParameters(Row, ResLines, ParV);
//	if (ParV.size() > 0) {
//		useRandomSeed = true;		
//
//		//if (ParV.size() > 1) { if (atoi(ParV[1].c_str()) == 0) DMtransformYN = false; else DMtransformYN = true; }
//		if (ParV.size() > 0) FixedLayerDir = atoi(ParV[0].c_str()) - 1;
//		if (ParV.size() > 1) { if (atoi(ParV[1].c_str()) == 0) HisEqYN = false; else HisEqYN = true; }
//		if (ParV.size() > 2) { if (atoi(ParV[2].c_str()) == 0) GenerateTI = false; else GenerateTI = true; }
//		if (ParV.size() > 3) { if (atoi(ParV[3].c_str()) == 0) PatternEntropyAnalysisYN = false; else PatternEntropyAnalysisYN = true; }
//		if (ParV.size() > 4) { if (atoi(ParV[4].c_str()) == 0) PrintHisYN = false; else PrintHisYN = true; }	
//		if (ParV.size() > 5) factorPos = atof(ParV[5].c_str());
//		if (ParV.size() > 6) factorC = atof(ParV[6].c_str())*100;
//	}
//
//	if (useRandomSeed) {
//		cout << endl << "use Random Seed";
//		srand((unsigned)time(NULL));
//		mersennetwistergenerator = mt19937(randomseed());
//	}
//	else {
//		cout << endl << "use Fixed Seed = 0";
//		srand(0);
//		mersennetwistergenerator = mt19937(0);
//	}
//	probabilitydistribution = uniform_real_distribution<double>(0.0, 1.0);
//
//	///////////////////////// Number of realizations
//	GetNextRowParameters(++Row, ResLines, ParV);
//	if (ParV.size() > 0) {
//		if (ParV.size() > 0) NumRealization = atoi(ParV[0].c_str());
//	}
//
//	///////////////////////// Working directory
//	workpath = ResLines[++Row];
//	if (workpath.back() != '\\') workpath += '\\';
//	
//
//	///////////////////////// Specify training images in XY, XZ and YZ-plane
//	cout << endl << "Workpath:" << workpath;
//	if (ResLines.size() > ++Row) {
//		vector<string> ParV;
//		GetNextRowParameters(Row, ResLines, ParV);
//		if (ParV.size() > 0) {
//			if (ParV.size() > 0) { FNameXY = workpath + ParV[0]; if (!fileExists(FNameXY)) { cout << endl << "Cannot find: " << endl << FNameXY; _getch(); exit(1); } else cout << endl << ParV[0]; }
//			if (ParV.size() > 1) {
//				FNameXZ = workpath + ParV[1];
//				if (!fileExists(FNameXZ)) { cout << endl << "Cannot find: " << endl << FNameXZ; _getch(); exit(1); }
//				else cout << endl << ParV[1];
//			}
//			if (ParV.size() > 2) { 
//				FNameYZ = workpath + ParV[2];
//				if (!fileExists(FNameYZ)) { cout << endl << "Cannot find: " << endl << FNameYZ; _getch(); exit(1); }
//				else cout << endl << ParV[2];
//			}
//		}
//	}
//
//	///////////////////////// read 3D model name
//	outputpath = "";
//	if (ResLines.size() > ++Row) {
//		vector<string> ParV;
//		GetNextRowParameters(Row, ResLines, ParV);
//		if (ParV.size() > 0) {
//			if (ParV.size() > 0) modelFilename3D = outputpath + ParV[0];
//		}
//		//////seperate filename and format
//		auto idx = ParV[0].rfind('.');
//		if (idx != std::string::npos) tempoutputformat = ParV[0].substr(idx);
//		
//		/////2d simulation
//		if (tempoutputformat == ".png") {
//			cout << endl << "2D simulation ON, just use the first TI.";
//			SIM2D_YN = true;
//			COHERENCENUM = 21;			// to approximate ANN search
//		}
//		
//		tempoutputfilename = ParV[0].substr(0, ParV[0].rfind('.') == string::npos ? ParV[0].length() : ParV[0].rfind('.'));
//	}
//	outputfilename = tempoutputfilename;
//
//	if (SIM2D_YN) FixedLayerDir = -1;
//	if (FixedLayerDir >= 0 && FixedLayerDir < 3) {
//		cout << endl << "enable fixed layer, modify directional weight: " << "1.0-->" << DirectionalWeight;
//	}
//
//	if (DMtransformYN && HisEqYN) parameterstring += "_Eq" + to_string((int)HisEqYN);
//	parameterstring += "DM" + to_string((int)DMtransformYN) ;
//	parameterstring += "Phis" + to_string((int)(10*factorPos));
//	if (FixedLayerDir > -1 && FixedLayerDir < 3)  parameterstring += "Fix" + to_string(FixedLayerDir) + "W" + to_string((int)(100 * DirectionalWeight));
//	
//	if (SIM2D_YN) parameterstring = "";
//	outputfilename += parameterstring;
//}

void DoPAR::ReadRunPar_series(string CurExeFile, int TIseries)
{
	cout << endl << "===========================================";
	cout << endl << "Set up your running parameters...    Series:" << TIseries;
	cout << endl << "===========================================";

	string tempoutputfilename;
	string tempoutputformat;
	string parametername;

	string Path;
	vector<string> ResLines;
	{//Read setup file
		string tmpstr, name;
		iCGetDirFileName(CurExeFile, Path, name);
		string PFName = Path + "PAR-GO_Setup_series.DAT";//!changed to series 
		vector<string> TmpLines;
		if (!ReadTxtFiles(PFName, TmpLines)) {
			cout << endl;
			cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
			cout << endl << " Failed to open file '" << PFName.c_str() << "' !";
			cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
			cout << endl;
			cout << endl << "Press any key to quit ...";
			_getch(); exit(1);
		}

		for (unsigned long Row = 0; Row < TmpLines.size(); Row++) {
			string::size_type Posidx = TmpLines[Row].find("//");
			if (Posidx != string::npos) {
				continue;
			}
			else {
				Posidx = TmpLines[Row].find("/*");
				if (Posidx != string::npos) continue;
			}
			ResLines.push_back(TmpLines[Row]);
		} //for(int Row=0; Row < LineLst.size(); Row++)

		if (ResLines.size() == 0) {
			cout << endl << " ============================================================";
			cout << endl << " Failed to open PAR-GO_Setup.DAT in current working directory !";
			cout << endl << " ============================================================";
			cout << endl << " Press any key to quit....";
			_getch(); exit(1);
		}
	}//Read setup file
	short Row(0);

	///////////////////////// check random seed
	vector<string> ParV;
	GetNextRowParameters(Row, ResLines, ParV);
	if (ParV.size() > 0) {
		useRandomSeed = true;
		int c = 0;
		//if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) DMtransformYN = false; else DMtransformYN = true; c++;}
		//if (ParV.size() > c) {FixedLayerDir = atoi(ParV[c].c_str()) - 1; c++;}
		if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) HisEqYN = false; else HisEqYN = true; c++; }
		if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) GenerateTI = false; else GenerateTI = true;  c++; }
		if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) PatternEntropyAnalysisYN = false; else PatternEntropyAnalysisYN = true; c++;	}
		if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) PrintHisYN = false; else PrintHisYN = true; c++;}
		if (ParV.size() > c) { factorPos = atof(ParV[c].c_str()); c++; }
		if (ParV.size() > c) { factorC = atof(ParV[c].c_str()); c++; }
	}

	if (useRandomSeed) {
		cout << endl << "use Random Seed";
		srand((unsigned)time(NULL));
		mersennetwistergenerator = mt19937(randomseed());
	}
	else {
		cout << endl << "use Fixed Seed = 0";
		srand(0);
		mersennetwistergenerator = mt19937(0);
	}
	probabilitydistribution = uniform_real_distribution<double>(0.0, 1.0);

	///////////////////////// Number of realizations
	GetNextRowParameters(++Row, ResLines, ParV);
	if (ParV.size() > 0) {
		if (ParV.size() > 0) NumRealization = atoi(ParV[0].c_str());
	}

	//jump, according to series num
	for (int i = 0; i < TIseries; i++)
		Row+=3;


	///////////////////////// Working directory
	if (Row == ResLines.size()-1) {
		printf("\nFinish series, quit.");
		_getch();
		exit(0);
	}
	
	workpath = ResLines[++Row];
	if (workpath.back() != '\\') workpath += '\\';


	///////////////////////// Specify training images in XY, XZ and YZ-plane	
	cout << endl << "Workpath:" << workpath;
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) { FNameXY = workpath + ParV[0]; if (!fileExists(FNameXY)) { cout << endl << "Cannot find: " << endl << FNameXY; _getch(); exit(1); } else cout << endl << ParV[0]; }
			if (ParV.size() > 1) {
				FNameXZ = workpath + ParV[1];
				if (!fileExists(FNameXZ)) { cout << endl << "Cannot find: " << endl << FNameXZ; _getch(); exit(1); }
				else cout << endl << ParV[1];
			}
			if (ParV.size() > 2) {
				FNameYZ = workpath + ParV[2];
				if (!fileExists(FNameYZ)) { cout << endl << "Cannot find: " << endl << FNameYZ; _getch(); exit(1); }
				else cout << endl << ParV[2];
			}
		}
	}

	///////////////////////// read 3D model name
	outputpath = "";
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) modelFilename3D = outputpath + ParV[0];
		}
		//////seperate filename and format
		auto idx = ParV[0].rfind('.');
		if (idx != std::string::npos) tempoutputformat = ParV[0].substr(idx);

		/////2d simulation
		if (tempoutputformat == ".png") {
			cout << endl << "2D simulation ON, just use the first TI.";
			SIM2D_YN = true;
			COHERENCENUM = 21;			// to approximate ANN search
		}

		tempoutputfilename = ParV[0].substr(0, ParV[0].rfind('.') == string::npos ? ParV[0].length() : ParV[0].rfind('.'));

		if (ParV.size() > 1) { outputsizeatlastlevel = atoi(ParV[1].c_str()); }
	}
	outputfilename = tempoutputfilename;

	if (SIM2D_YN) FixedLayerDir = -1;
	if (FixedLayerDir >= 0 && FixedLayerDir < 3) {
		cout << endl << "enable fixed layer, modify directional weight: " << "1.0-->" << DirectionalWeight;
	}

	if (DMtransformYN && HisEqYN) parameterstring += "_Eq" + to_string((int)HisEqYN);
	parameterstring += "DM" + to_string((int)DMtransformYN);
	parameterstring += "Ihis" + to_string((int)(10 * factorIndex)) + "Phis" + to_string((int)(10 * factorPos));
	if (FixedLayerDir > -1 && FixedLayerDir < 3)  parameterstring += "Fix" + to_string(FixedLayerDir) + "W" + to_string((int)(100 * DirectionalWeight));

	if (SIM2D_YN) parameterstring = "";
	outputfilename += parameterstring;
}


bool DoPAR::ReadTxtFiles(const string PFName, vector<string>& ResLines)
{
	{ vector<string> LineLst; ResLines.swap(LineLst); }

	ifstream FIn(PFName.c_str());
	if (!FIn) {
		//cout << endl;
		//cout << endl <<" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
		//cout << endl<< " Failed to open file '" << PFName.c_str() << "' !";
		//cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
		//cout << endl;
		//cout << endl << "Press any key to quit ...";
		return false;
	}

	string Sline;
	while (getline(FIn, Sline))
		ResLines.push_back(Sline);

	FIn.close();

	return true;
};

//vector<uchar> DoPAR::load3Dmodel(const char* filename)
//{//load 3D model raw file
//	// open the file:
//	streampos fileSize;
//	ifstream file(filename, ios::binary);
//
//	// get its size:
//	file.seekg(0, ios::end);
//	fileSize = file.tellg();
//	file.seekg(0, ios::beg);
//
//	// read the data:
//	vector<uchar> fileData(fileSize);
//	file.read((char*)&fileData[0], fileSize);
//
//	if (fileData.size() != TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]) {
//		cout << endl << "Error: Model size = " << fileData.size() << " SHOULD BE:" << TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0];
//		_getch(); exit(1);
//	}
//
//	return fileData;
//}

//void DoPAR::ReadPBMImage(string FName, char DirID, double UpPro)
//{
//	//DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
//	//       otherwise a single image for three direction
//
//	vector<string> ResLines;
//	ReadTxtFiles(FName, ResLines);
//
//	if (ResLines.size() == 0) {
//		cout << endl << "===================================";
//		cout << endl << "Fail to read the pbm file !!";
//		cout << endl << FName;
//		cout << endl << "===================================";
//		_getch(); exit(1);
//	}
//
//	vector<uchar> OImg2D;
//
//	long S1, S2;
//	long PoreNum(0);
//	//!!deal with pbm file header!
//	bool FormatYN(false);
//	bool finddimension(false);
//
//	if (ResLines[0][0] == 'P') FormatYN = true;
//	if (FormatYN == false) {
//		cout << endl << "Fail to read the pbm file !!";
//		_getch(); exit(1);
//	}
//
//	for (long Line = 1; Line < ResLines.size(); ++Line) {
//		if (ResLines[Line][0] == '#') continue;
//		if (Line <= 2) {
//			if (finddimension == false){
//				vector<long> Dim;
//				string OneItem("");
//				for (long idx = 0; idx < ResLines[Line].size(); ++idx) {
//					if (ResLines[Line][idx] < '0' || ResLines[Line][idx] > '9') {
//						if (OneItem != "") {
//							short Val = atoi(OneItem.c_str());
//							Dim.push_back(Val);
//						}
//						OneItem = "";
//					}
//					else {
//						OneItem += ResLines[Line][idx];
//					}
//				}
//
//				if (OneItem != "") {
//					short Val = atoi(OneItem.c_str());
//					Dim.push_back(Val);
//				}
//
//				S1 = Dim[0]; S2 = Dim[1];
//				OImg2D.reserve(S1*S2);
//				finddimension = true;
//				continue;
//			}
//		}//(Line <= 2)
//
//		string OneItem("");
//		for (long idx = 0; idx < ResLines[Line].size(); ++idx) {
//			if (ResLines[Line][idx] >= '0' && ResLines[Line][idx] <= '9') {
//				OneItem += ResLines[Line][idx];
//				unsigned short Val = atoi(OneItem.c_str());
//				if (Val == 1) PoreNum++; //1 - pore
//				OImg2D.push_back(Val);
//				OneItem = "";
//			}
//
//		}
//	}
//
//	double Porosity = 1.0*PoreNum / OImg2D.size();
//
//	if (UpPro < 0.5) UpPro = 0.5;
//	if (Porosity > UpPro) {
//		for (long idx = 0; idx < OImg2D.size(); ++idx)
//			OImg2D[idx] = 1 - OImg2D[idx];
//		// 1 - pore, 0 - grain
//		Porosity = 1 - Porosity;
//	}
//
//	cout << endl << "Porosity: " << Porosity;
//
//	switch (DirID) {
//	case '1': {//DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
//				  XYSx = S1; XYSy = S2;
//				  XY2DImg.swap(OImg2D);
//				  PorosityXY = Porosity;
//				  break;
//	}
//	case '2': {//XZ
//				  XZSx = S1; XZSz = S2;
//				  XZ2DImg.swap(OImg2D);
//				  PorosityXZ = Porosity;
//				  break;
//	}
//	case '3': {//YZ
//				  YZSy = S1; YZSz = S2;
//				  YZ2DImg.swap(OImg2D);
//				  PorosityYZ = Porosity;
//				  break;
//	}
//	default: {
//				 XYSx = XZSx = YZSy = S1;
//				 XYSy = XZSz = YZSz = S2;
//				 XY2DImg.resize(OImg2D.size(), 0);
//				 XZ2DImg.resize(OImg2D.size(), 0);
//				 YZ2DImg.resize(OImg2D.size(), 0);
//				 for (long idx = 0; idx < OImg2D.size(); ++idx) {
//					 XY2DImg[idx] = OImg2D[idx];
//					 XZ2DImg[idx] = OImg2D[idx];
//					 YZ2DImg[idx] = OImg2D[idx];
//				 }
//				 PorosityXY = PorosityXZ = PorosityYZ = Porosity;
//	}
//	}
//
//	cout << endl << FName;
//	cout << endl << "Dimensions of this training image: (" << S1 << "  " << S2 << ")";
//
//	TIx = S1; TIy = S2;
//}

void DoPAR::GetStarted(string CurExeFile, int TIseries)
{
	cout << endl << "===========================================";
	cout << endl << "PAR-KC                     ";
	cout << endl << "Tianshen Huang    th2@hw.ac.uk  ";
	time_t CurTime, MaxTime;
	time(&CurTime);
	CurTime /= 86400L;
	//cout << endl << CurTime; _getch(); exit(0);		//17598 = 08/03/2018
	MaxTime = (time_t)(17598 + 180);				//add 30*6 days
	if (CurTime > MaxTime) {
		cout << endl << "Code expired. Please contact the author.";
		_getch();
		exit(0);
	}
	
	//ReadRunPar(CurExeFile);
	ReadRunPar_series(CurExeFile, TIseries);

	DoANNOptimization(TIseries);
}

//void VectorShortToMat(const vector<short>& in, Mat& out)
//{
//	vector<short>::const_iterator it = in.begin();
//	MatIterator_<int> jt, end;
//	jt = out.begin<int>();
//	for (; it != in.end(); ++it) { *jt++ = (int)(*it); }
//}
//void VectorDoubleToMat(const vector<double>& in, Mat& out){
//	vector<double>::const_iterator it = in.begin();
//	MatIterator_<double> jt, end;
//	jt = out.begin<double>();
//	for (; it != in.end(); ++it) { *jt++ = (double)(*it); }
//}
//void VectorFloatToMat(const vector<float>& in, Mat& out){
//	vector<float>::const_iterator it = in.begin();
//	MatIterator_<float> jt, end;
//	jt = out.begin<float>();
//	for (; it != in.end(); ++it) { *jt++ = (float)(*it); }
//}
void calcstddev(int level, vector<float>& floatvector) {
	float sum = accumulate(floatvector.begin(), floatvector.end(), 0.0);
	float mean = sum / floatvector.size();

	vector<size_color> diff(floatvector.size());
	transform(floatvector.begin(), floatvector.end(), diff.begin(), [mean](float x) { return x - mean; });
	float sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	float stdev = sqrt(sq_sum / floatvector.size());

	cout << endl << "mean=" << mean << " stdev=" << stdev << " size=" << floatvector.size();
}

void DoPAR::showMat(const cv::String& winname, const cv::Mat& mat)
{// Show a Mat object quickly. For testing purposes only.
	assert(!mat.empty());
	cv::namedWindow(winname);
	cv::imshow(winname, mat);
	cv::waitKey(0);
	cv::destroyWindow(winname);
}

const float DoPAR::inv_sqrt_2pi = 0.398942280401433f;

/////////////////////////////////////////////////////////////
//exemplar_x --> YZ, exemplar_y --> ZX, exemplar_z --> XY
//using imagej, XY slice is XY, ememplar_z
//ZX slice can be attained by: 1. reslice top + flip virtical 2. then rotate 90 degrees left
//YZ slice is done by: reslice left
/////////////////////////////////////////////////////////////


void DoPAR::DoANNOptimization(int TIseries) {
	time_t StartTime;	time(&StartTime);
	unsigned long t1, t2, t3;
	
	init(TIseries);

	for (int numsim = 0; numsim < NumRealization; ++numsim) {
		cout << endl << "Realization :  " << numsim+1 << "========================================";
		if (numsim > 0) {
			allocateVectors();
			InitRandomVolume(0);
		}		

		for (int curlevel = 0; curlevel < MULTIRES; curlevel++) {
			cout << endl << "=============level: " << curlevel << "===============";

			FIRSTRUN = true;
			initPermutation(curlevel);


			for (int loop = 0; loop < MAXITERATION[curlevel]; loop++) {			
				shuffle(m_permutation.begin(), m_permutation.end(), mersennetwistergenerator);
				if (loop % (int)ceil(MAXITERATION[curlevel] * 0.5) == 0)
					cout << endl << "iteration: " << loop;
				//the first run on level0 should be started by search	
				if (curlevel == 0 && loop == 0) searchVolume(curlevel);			

				t1 = GetTickCount();
				optimizeVolume(curlevel);
				t2 = GetTickCount();
				if (loop % (int)ceil(MAXITERATION[curlevel] * 0.5) == 0)
					cout << endl << "optmize: " << (t2 - t1) / 1000.0 << " s";

				if (searchVolume(curlevel)) {
					cout << endl << "converged, skip to next level.";
					break;
				}
				t3 = GetTickCount();
				if (loop % (int)ceil(MAXITERATION[curlevel] * 0.5) == 0)
					cout << endl << "search: " << (t3 - t2) / 1000.0 << " s";

				if (curlevel == MULTIRES - 1 && loop == MAXITERATION[curlevel] - 1) optimizeVolume(curlevel);	//do one optimization after the last search

				if ((SIM2D_YN && MULTIRES==1) /*|| (GenerateTI && PrintHisYN && FIRSTRUN)*/) {		// output for 2D tests
					outputmodel(curlevel);
					_getch();
					//if (curlevel == MULTIRES - 2 && loop % 4 == 0) {
					//	outputmodel(curlevel);
					//	cout << "output every 4 iteration..  ";
					//}
					//else if (curlevel == MULTIRES - 1 && loop % 2 == 0) {
					//	outputmodel(curlevel);
					//	cout << "output every 2 iteration..  ";
					//}
				}
			}

			if (curlevel == MULTIRES - 1 || (curlevel == 0 && SIM2D_YN)/* || (GenerateTI && PrintHisYN)*/) {// ouput model & histogram
				outputmodel(curlevel);
				if (PrintHisYN) writeHistogram(curlevel);
			}

			if (curlevel < MULTIRES - 1) {// level up
				upsampleVolume(curlevel);			
				FIRSTRUN = true;
			}
		}
	}

	time_t NewTime;		time(&NewTime);
	cout << endl << "Total reconstruction time: " << unsigned long(NewTime - StartTime) << " s (" << unsigned long(NewTime - StartTime)/60 <<" min)";	
	cleardata();
}

void DoPAR::allocateVectors() {
	//if (level == 0) {
		isUnchanged_x.resize(MULTIRES); isUnchanged_y.resize(MULTIRES);		isUnchanged_z.resize(MULTIRES);
		nearestIdx_x.resize(MULTIRES);	nearestIdx_y.resize(MULTIRES);		nearestIdx_z.resize(MULTIRES);
		nearestWeight_x.resize(MULTIRES); nearestWeight_y.resize(MULTIRES); nearestWeight_z.resize(MULTIRES);
		Origin_x.resize(MULTIRES);		Origin_y.resize(MULTIRES);			Origin_z.resize(MULTIRES);
		IndexHis_x.resize(MULTIRES);	IndexHis_y.resize(MULTIRES);		IndexHis_z.resize(MULTIRES);
		PosHis.resize(MULTIRES);
		SelectedPos.resize(MULTIRES);
	//}

	for (int level = 0; level < MULTIRES; ++level) {
		size_idx Soutput = OUTsize[level] * OUTsize[level] *OUTsize[level];
		if (SIM2D_YN) Soutput = OUTsize[level] * OUTsize[level];
		isUnchanged_x[level].assign(Soutput, false);		isUnchanged_y[level].assign(Soutput, false);		isUnchanged_z[level].assign(Soutput, false);
		nearestIdx_x[level].assign(Soutput, 205000);		nearestIdx_y[level].assign(Soutput, 205000);		nearestIdx_z[level].assign(Soutput, 205000);
		nearestWeight_x[level].assign(Soutput, min_dist);	nearestWeight_y[level].assign(Soutput, min_dist);	nearestWeight_z[level].assign(Soutput, min_dist);
		Origin_x[level].assign(Soutput, 205000);			Origin_y[level].assign(Soutput, 205000);			Origin_z[level].assign(Soutput, 205000);		
		SelectedPos[level].assign(Soutput, 615000);

		size_idx Sti = TIsize[level] * TIsize[level];
		//PosHis size=3*TI
		PosHis[level].assign(Sti * 3, 0);
		//sparse grid IndexHis
		IndexHis_x[level].assign(Sti * 0.25, 0);		IndexHis_y[level].assign(Sti * 0.25, 0);		IndexHis_z[level].assign(Sti * 0.25, 0);
	}
}

void DoPAR::init(int TIseries) {
	int tempcore, tempthread;
	MaxThread = omp_get_num_procs();
	if (!SIM2D_YN) {
		if (TIseries == 0) {
			cout << endl << "Select maximum cores for CPU parallelization, no more than " << MaxThread / 2 << endl;
			cin >> tempcore;
			tempthread = max(1, tempcore * 2);
		}	

		if (tempthread <= MaxThread && TIseries == 0) {
			omp_set_dynamic(0);     // Explicitly disable dynamic teams
			MaxThread = tempthread;
			omp_set_num_threads(MaxThread);
		}
		else {
			omp_set_dynamic(0);
			omp_set_num_threads(MaxThread);
		}
	}
	else {
		omp_set_dynamic(0);     // Explicitly disable dynamic teams
		omp_set_num_threads(MaxThread);
	}
	
	// load TI
	if (!loadExemplar()) return;

	printf("\nOUTsize=%d", OUTsize[MULTIRES - 1]);

	// allocate memory for all global vectors
	allocateVectors();

	// load Model
	if (!loadVolume()) return;			

	// init deltaHis, linear factor
	for (int i = 0; i < MULTIRES; i++){
		avgIndexHis[i] = ceil((1.0f * OUTsize[i] * (OUTsize[i] * 0.5) * (OUTsize[i] * 0.5)) / ((TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)*(TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)));
		if (SIM2D_YN) avgIndexHis[i] = ceil((1.0f * (OUTsize[i] * 0.5) * (OUTsize[i] * 0.5)) / ((TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)*(TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)));
		//avgIndexHis[i] -= 1;
		avgPosHis[i] = ceil((OUTsize[i] * OUTsize[i] * OUTsize[i]) / ((TIsize[i] - 2)*(TIsize[i] - 2)) / 3.0);
		if (SIM2D_YN) avgPosHis[i] = ceil((OUTsize[i] * OUTsize[i]) / ((TIsize[i] - 2)*(TIsize[i] - 2)));
		//avgPosHis[i] -= 1;
	}


	//if (ColorHis_ON) initColorHis_exemplar();

	// K-Coherence
	computeKCoherence();
}

// load 2D Exemplar, initialize multi-level!
bool DoPAR::loadExemplar() {
	/////////////////////////////////////////////////////////////////
	//////exemplar_x --> YZ, exemplar_y --> ZX, exemplar_z --> XY
	//////using imagej, XY slice is XY, ememplar_z
	//////ZX slice can be attained by: 1. reslice top + flip virtical 2. then rotate 90 degrees left
	//////YZ slice is done by: reslice left
	/////////////////////////////////////////////////////////////////

	//!!in imagej:
	//exemplar_x -> XY
	//exemplar_y -> XZ
	//exemplar_z -> YZ

	//-------------- convert Mat to IplImage* --------------
	Mat matyz = cv::imread(FNameXY, CV_LOAD_IMAGE_ANYDEPTH);		 // ti grayscale, could be 16 bit!
	Mat matzx = cv::imread(FNameXZ, CV_LOAD_IMAGE_ANYDEPTH);
	Mat matxy = cv::imread(FNameYZ, CV_LOAD_IMAGE_ANYDEPTH);
	if (FNameXY == FNameXZ && FNameXY == FNameYZ) {
		cout << endl << "Only one TI, flip 2nd TI by switching X,Y";
		flip(matzx, matzx, 0);
		flip(matzx, matzx, 1);
	}

	if (matyz.cols != matyz.rows) {
		//cout << endl << "matyz.cols != matyz.rows"; _getch(); exit(0); 
		cout << endl << "matyz.cols != matyz.rows, crop to square";
		int mindim = min(matyz.cols, matyz.rows);
		Mat cropedMatyz = matyz(Rect(0, 0, mindim, mindim));
		cropedMatyz.copyTo(matyz);
	}
	if (matzx.cols != matzx.rows) {
		//cout << endl << "matzx.cols != matzx.rows"; _getch(); exit(0); 
		cout << endl << "matzx.cols != matzx.rows, crop to square";
		int mindim = min(matzx.cols, matzx.rows);
		Mat cropedMatzx = matzx(Rect(0, 0, mindim, mindim));
		cropedMatzx.copyTo(matzx);
	}
	if (matxy.cols != matxy.rows) {
		//cout << endl << "matxy.cols != matxy.rows"; _getch(); exit(0);
		cout << endl << "matxy.cols != matxy.rows, crop to square";
		int mindim = min(matxy.cols, matxy.rows);
		Mat cropedMatxy = matxy(Rect(0, 0, mindim, mindim));
		cropedMatxy.copyTo(matxy);
	}

	////check TI is grayscale or binary, if grayscale disable DM, if binary enable DM
	bool binaryYN = false;
	Mat tempMatyz, tempMatzx, tempMatxy;
	Mat maskyz, maskzx, maskxy;
	matyz.copyTo(tempMatyz);	matzx.copyTo(tempMatzx);	matxy.copyTo(tempMatxy);
	inRange(tempMatyz, 255, 255, maskyz);	inRange(tempMatzx, 255, 255, maskzx);	inRange(tempMatxy, 255, 255, maskxy);
	size_idx countNonZeroYZ, countNonZeroZX, countNonZeroXY;
	countNonZeroYZ = countNonZero(matyz);	countNonZeroZX = countNonZero(matzx);	countNonZeroXY = countNonZero(matxy);

	if (countNonZero(maskyz) != countNonZeroYZ || countNonZero(maskzx) != countNonZeroZX || countNonZero(maskxy) != countNonZeroXY) {
		cout << endl << "TI is not binary, disable distance map transformation & colorhis";
		DMtransformYN = false;
		ColorHis_ON = false;
	}
	else {
		DMtransformYN = true;
		cout << endl << "TI is binary, enable distance map transformation";
		cout << endl << "porosity: " << countNonZeroYZ*1.0f / (matyz.cols*matyz.rows)
			<< " " << countNonZeroZX*1.0f / (matzx.cols*matzx.rows) << " " << countNonZeroXY*1.0f / (matxy.cols*matxy.rows);
		porosityX = countNonZeroYZ*1.0f / (matyz.cols*matyz.rows);
		porosityY = countNonZeroZX*1.0f / (matzx.cols*matzx.rows);
		porosityZ = countNonZeroXY*1.0f / (matxy.cols*matxy.rows);
	}

	
	//--------------[begin] initial global parameters -------------
	int tempSize = matyz.cols;
	if (tempSize < 40) { cout << endl << "TI size < 40, too small!"; _getch(); exit(0); }
	else if (tempSize > 832) { cout << endl << "TI size > 832, too big!"; _getch(); exit(0); }

	ANNerror.assign(MULTIRES, 0.0);
	if (tempSize >= 600) {
		if (tempSize % 32 != 0) {
			int cropedsize = tempSize / 32 * 32;
			Mat cropedMatyz = matyz(Rect(0, 0, cropedsize, cropedsize));
			cropedMatyz.copyTo(matyz);
			Mat cropedMatzx = matzx(Rect(0, 0, cropedsize, cropedsize));
			cropedMatzx.copyTo(matzx);
			Mat cropedMatxy = matxy(Rect(0, 0, cropedsize, cropedsize));
			cropedMatxy.copyTo(matxy);
			cout << endl << "TIs are croped to " << cropedsize << " to fit multi-grid";
		}
		MULTIRES = 6;

		blockSize = { 10, 8, 8, 6, 6, 6 };
		MAXITERATION = { 30, 5, 3, 2, 2, 2 };

		ANNerror = { 0, 0, 0, 0.5, 1.0, 1.0 };		//for big model use ANN
	}
	else if (tempSize > 400) {
		if (PatternEntropyAnalysisYN) {
			MULTIRES = 1;
			blockSize = { 12 };
			MAXITERATION = { 3 };
		}
		else {
			if (tempSize % 16 != 0) {
				int cropedsize = tempSize / 16 * 16;
				Mat cropedMatyz = matyz(Rect(0, 0, cropedsize, cropedsize));
				cropedMatyz.copyTo(matyz);
				Mat cropedMatzx = matzx(Rect(0, 0, cropedsize, cropedsize));
				cropedMatzx.copyTo(matzx);
				Mat cropedMatxy = matxy(Rect(0, 0, cropedsize, cropedsize));
				cropedMatxy.copyTo(matxy);
				cout << endl << "TIs are croped to " << cropedsize << " to fit multi-grid";
			}
			//MULTIRES = 5;
			//blockSize = { 4, 4, 4, 2, 2 };
			//MAXITERATION = { 2, 2, 1, 1, 1 };
			//blockSize = { 12, 10, 8, 6, 6 };
			//MAXITERATION = { 20, 4, 2, 2, 2 };
			//blockSize = { 12, 10, 8, 6, 6 };
			//ANNerror = { 0, 0, 0, 0.5, 0.5 };
			
			MULTIRES = 5;
			blockSize = { 6, 6, 6, 6, 6 };
			MAXITERATION = { 20, 4, 2, 2, 2};
			ANNerror = { 0, 0, 0, 0.5, 0.5 };

		}
	}
	else if (tempSize >= 200) {
		if (PatternEntropyAnalysisYN) {
			MULTIRES = 1;
			blockSize = { 12 };
			MAXITERATION = { 3 };
		}
		else {
			if (tempSize % 8 != 0) {
				int cropedsize = tempSize / 8 * 8;
				Mat cropedMatyz = matyz(Rect(0, 0, cropedsize, cropedsize));
				cropedMatyz.copyTo(matyz);
				Mat cropedMatzx = matzx(Rect(0, 0, cropedsize, cropedsize));
				cropedMatzx.copyTo(matzx);
				Mat cropedMatxy = matxy(Rect(0, 0, cropedsize, cropedsize));
				cropedMatxy.copyTo(matxy);
				cout << endl << "TIs are croped to " << cropedsize << " to fit multi-grid";
			}

			//MULTIRES = 4;
			//blockSize = { 10, 8, 2, 2 };	//test
			//MAXITERATION = { 20, 4, 2, 1 };
			MULTIRES = 4;
			blockSize = { 6, 6, 6, 6};			//for smallest level [32,50], 6 is enough. unless the feature size is big,use8
			MAXITERATION = { 20, 4, 2, 2 };
			
			//blockSize = { 12, 10, 8, 8 };
			//MAXITERATION = { 40, 5, 3, 2 };
		}
	}
	else if (tempSize >= 128) {
		if (tempSize % 4 != 0) {
			int cropedsize = tempSize / 4 * 4;
			Mat cropedMatyz = matyz(Rect(0, 0, cropedsize, cropedsize));
			cropedMatyz.copyTo(matyz);
			Mat cropedMatzx = matzx(Rect(0, 0, cropedsize, cropedsize));
			cropedMatzx.copyTo(matzx);
			Mat cropedMatxy = matxy(Rect(0, 0, cropedsize, cropedsize));
			cropedMatxy.copyTo(matxy);
			cout << endl << "TIs are croped to " << cropedsize << " to fit multi-grid";
		}
		//MULTIRES = 3;
		//blockSize = { 4, 4, 4};
		//MAXITERATION = { 2, 2, 2 };
		
		MULTIRES = 2;
		blockSize = { 6, 6 };		//tested: coarse level big for quality, fine level small for speed
		MAXITERATION = { 20, 4 };	//tested: fine level does not need many iterations

	}
	else if (tempSize < 128) {
		cout << endl << "for small TI, test just using one level";
		MULTIRES = 1;
		blockSize = { 6 };
		MAXITERATION = { 20 };
		//MULTIRES = 2;
		//blockSize = { 16, 12 };
		//MAXITERATION = { 16, 8 };
	}

	//! add additional space, later will crop to original size, to deal with Toroidal problem
	int tempoutputsize = tempSize + pow(2, MULTIRES-1)* blockSize[0];
	outputsizeatlastlevel = max(outputsizeatlastlevel, tempoutputsize);


	TIsize.resize(MULTIRES);	
	OUTsize.resize(MULTIRES);

	avgIndexHis.resize(MULTIRES); avgPosHis.resize(MULTIRES);
	ColorHis_exemplar.resize(MULTIRES);
	ColorHis_synthesis.resize(MULTIRES);
	Solid_Upper.resize(MULTIRES);	Pore_Upper.resize(MULTIRES);	Pore_Lower.resize(MULTIRES);
	porosity_required.resize(MULTIRES);

	//--------------[end] initial global parameters -------------

	TIsize[MULTIRES - 1] = tempSize;
	OUTsize[MULTIRES - 1] = outputsizeatlastlevel;

	for (int level = MULTIRES - 1; level >= 0; --level) {	// size registration		
		TIsize[level] = TIsize[MULTIRES - 1] / pow(2, MULTIRES - 1 - level);
		OUTsize[level] = OUTsize[MULTIRES - 1] / pow(2, MULTIRES - 1 - level);
		// [begin] memory allocation -------------------------------------------
		m_exemplar_x.resize(MULTIRES);
		m_exemplar_y.resize(MULTIRES);
		m_exemplar_z.resize(MULTIRES);
		m_exemplar_x[level].resize(TIsize[level] * TIsize[level]);
		m_exemplar_y[level].resize(TIsize[level] * TIsize[level]);
		m_exemplar_z[level].resize(TIsize[level] * TIsize[level]);
		
		m_volume.resize(MULTIRES);
		if (SIM2D_YN) m_volume[level].resize(OUTsize[level] * OUTsize[level]);
		else m_volume[level].resize(OUTsize[level] * OUTsize[level] * OUTsize[level]);
		// [end] memory allocation -------------------------------------------
	}

	
	

	if (DMtransformYN) {
		//!add padding border to image
		copyMakeBorder(matyz, matyz, 1, 1, 1, 1, BORDER_REPLICATE);
		copyMakeBorder(matzx, matzx, 1, 1, 1, 1, BORDER_REPLICATE);
		copyMakeBorder(matxy, matxy, 1, 1, 1, 1, BORDER_REPLICATE);
		//imwrite("pd.png", matyz); _getch();

		// convert mat to vector
		vector<short> padded_x, padded_y, padded_z;
		int padded_TIsize = TIsize[MULTIRES - 1] + 2;
		padded_x.resize(padded_TIsize*padded_TIsize);
		padded_y.resize(padded_TIsize*padded_TIsize);
		padded_z.resize(padded_TIsize*padded_TIsize);
		if (matyz.isContinuous()) 	padded_x.assign(matyz.datastart, matyz.dataend);
		if (matzx.isContinuous()) 	padded_y.assign(matzx.datastart, matzx.dataend);
		if (matxy.isContinuous()) 	padded_z.assign(matxy.datastart, matxy.dataend);

		//!invert border
		for (int x = 0; x < padded_TIsize; x++) {
			int idx1 = x, idx2 = (padded_TIsize - 1)*padded_TIsize + x;
			padded_x[idx1] = (255 - padded_x[idx1]); padded_x[idx2] = (255 - padded_x[idx2]);
			padded_y[idx1] = (255 - padded_y[idx1]); padded_y[idx2] = (255 - padded_y[idx2]);
			padded_z[idx1] = (255 - padded_z[idx1]); padded_z[idx2] = (255 - padded_z[idx2]);
		}
		for (int y = 0; y < padded_TIsize; y++) {
			int idx1 = y*padded_TIsize, idx2 = (padded_TIsize - 1) + y*padded_TIsize;
			padded_x[idx1] = (255 - padded_x[idx1]); padded_x[idx2] = (255 - padded_x[idx2]);
			padded_y[idx1] = (255 - padded_y[idx1]); padded_y[idx2] = (255 - padded_y[idx2]);
			padded_z[idx1] = (255 - padded_z[idx1]); padded_z[idx2] = (255 - padded_z[idx2]);
		}	

		//Distance map transformation
		transformDM(MULTIRES - 1, padded_x, padded_y, padded_z);
		//if (DMtransformYN) transformDM(MULTIRES - 1, m_exemplar_x[MULTIRES - 1], m_exemplar_y[MULTIRES - 1], m_exemplar_z[MULTIRES - 1]);

		//!crop vector to mat
		vector<unsigned char> tmpchar = vector<unsigned char>(padded_x.begin(), padded_x.end());
		matyz = Mat(tmpchar, true).reshape(1, matyz.rows);// vector to mat, need the same data type!
		Mat cropedmatyz = matyz(Rect(1, 1, TIsize[MULTIRES-1], TIsize[MULTIRES - 1]));
		cropedmatyz.copyTo(matyz);
		tmpchar = vector<unsigned char>(padded_y.begin(), padded_y.end());
		matzx = Mat(tmpchar, true).reshape(1, matzx.rows);// vector to mat, need the same data type!
		Mat cropedmatzx = matzx(Rect(1, 1, TIsize[MULTIRES - 1], TIsize[MULTIRES - 1]));
		cropedmatzx.copyTo(matzx);
		tmpchar = vector<unsigned char>(padded_z.begin(), padded_z.end());
		matxy = Mat(tmpchar, true).reshape(1, matxy.rows);// vector to mat, need the same data type!
		Mat cropedmatxy = matxy(Rect(1, 1, TIsize[MULTIRES - 1], TIsize[MULTIRES - 1]));
		cropedmatxy.copyTo(matxy);

		//imwrite("dmpd_c.png", matyz); _getch();
	}
	// convert mat to vector
	if (matyz.isContinuous()) 	m_exemplar_x[MULTIRES - 1].assign(matyz.datastart, matyz.dataend);
	if (matzx.isContinuous()) 	m_exemplar_y[MULTIRES - 1].assign(matzx.datastart, matzx.dataend);
	if (matxy.isContinuous()) 	m_exemplar_z[MULTIRES - 1].assign(matxy.datastart, matxy.dataend);
	
	
	///////--------------Processing TI----------------------------------


	//Gaussian filter resizing better than opencv interpolation resize(inter_area)
	if (MULTIRES > 1) {										
		cout << endl << "use Gaussian filter to resize.";
		for (int l = MULTIRES - 1; l > 0; --l) {
			gaussImage(l, m_exemplar_x);
			gaussImage(l, m_exemplar_y);
			gaussImage(l, m_exemplar_z);
		}
	}


	//HisEqYN = true;
	if (DMtransformYN && HisEqYN) {
		cout << endl << "apply histogram equalization";
		_Solid_Upper = Solid_Upper[MULTIRES - 1];
		_Pore_Lower = Pore_Lower[MULTIRES - 1];
		
		for (int l = MULTIRES - 1; l >= 0; --l)
			equalizeHistogram(l, m_exemplar_x[l], m_exemplar_y[l], m_exemplar_z[l]);
	}
	for (int l = MULTIRES - 1; l >= 0; --l) {
		long porecount(0);
		size_color solidup = Solid_Upper[l];
		porecount += std::count_if(m_exemplar_x[l].begin(), m_exemplar_x[l].end(), [solidup](size_color i) {return i> solidup; });
		porecount += std::count_if(m_exemplar_y[l].begin(), m_exemplar_y[l].end(), [solidup](size_color i) {return i> solidup; });
		porecount += std::count_if(m_exemplar_z[l].begin(), m_exemplar_z[l].end(), [solidup](size_color i) {return i> solidup; });
		porosity_required[l] = porecount*1.0f / (3 * m_exemplar_x[l].size());
		if(HisEqYN || l==MULTIRES-1) cout << endl << "level"<<l<<" Solid_Upper=" << Solid_Upper[l] << " Pore_Lower=" << Pore_Lower[l]<< "porosity="<< porosity_required[l];
	}
	
	if (GenerateTI) {
		testPCA();
	}

	if (PatternEntropyAnalysisYN) {
		double entropy;
		if (!DMtransformYN) cout << endl << "noDM:";
		else cout << endl << "DM:";

		for (int templatesize = 4; templatesize < 34; templatesize += 2) {
			if (!DMtransformYN) {				
				patternentropyanalysis(templatesize, matyz, entropy);
			}
			else {
				Mat DM1;
				if (HisEqYN) {	//DM 8UC1, unsigned char
					vector<unsigned char> tmpchar;
					DM1 = Mat(TIsize[MULTIRES - 1], TIsize[MULTIRES - 1], CV_8UC1);
					tmpchar = vector<unsigned char>(m_exemplar_x[MULTIRES - 1].begin(), m_exemplar_x[MULTIRES - 1].end());
					DM1 = Mat(tmpchar, true).reshape(1, DM1.rows);
				}
				else {	//DM 32FC1, float. matchTemplate only accept 8U or 32F
					vector<float> tempfloat;
					DM1 = Mat(TIsize[MULTIRES - 1], TIsize[MULTIRES - 1], CV_32FC1);
					tempfloat = vector<float>(m_exemplar_x[MULTIRES - 1].begin(), m_exemplar_x[MULTIRES - 1].end());
					DM1 = Mat(tempfloat, true).reshape(1, DM1.rows);
				}					
				patternentropyanalysis(templatesize, DM1, entropy);
			}
		}

		_getch();
	}

	if (GenerateTI && DMtransformYN) {						//Generate DM TI
		ostringstream name;
		for (int lv = MULTIRES-1; lv >=0 ; --lv) {
		int TEXSIZE_ = TIsize[lv];
		Mat DM1, DM2, DM3;

		if (HisEqYN) {
			vector<unsigned char> tmpchar;
			DM1 = Mat(TEXSIZE_, TEXSIZE_, CV_8UC1);
			tmpchar = vector<unsigned char>(m_exemplar_x[lv].begin(), m_exemplar_x[lv].end());
			DM1 = Mat(tmpchar, true).reshape(1, DM1.rows);// vector to mat, need the same data type!
		}
		else {
			vector<unsigned short> tmpshort;
			DM1 = Mat(TEXSIZE_, TEXSIZE_, CV_16UC1);
			tmpshort = vector<unsigned short>(m_exemplar_x[lv].begin(), m_exemplar_x[lv].end());
			DM1 = Mat(tmpshort, true).reshape(1, DM1.rows);// vector to mat, need the same data type!
		}
		name << "DM1_S" << (short)Solid_Upper[lv] << "_L" << to_string(lv) << ".png";
		imwrite(name.str(), DM1);	name.str("");

		if (HisEqYN) {
			vector<unsigned char> tmpchar;
			DM2 = Mat(TEXSIZE_, TEXSIZE_, CV_8UC1);
			tmpchar = vector<unsigned char>(m_exemplar_y[lv].begin(), m_exemplar_y[lv].end());
			DM2 = Mat(tmpchar, true).reshape(1, DM2.rows);
		}
		else {
			vector<unsigned short> tmpshort;
			DM2 = Mat(TEXSIZE_, TEXSIZE_, CV_16UC1);
			tmpshort = vector<unsigned short>(m_exemplar_y[lv].begin(), m_exemplar_y[lv].end());
			DM2 = Mat(tmpshort, true).reshape(1, DM2.rows);
		}
		name << "DM2_S" << (short)Solid_Upper[lv] << "_L" << to_string(lv) << ".png";
		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), DM2);
		name.str("");

		if (HisEqYN) {
			vector<unsigned char> tmpchar;
			DM3 = Mat(TEXSIZE_, TEXSIZE_, CV_8UC1);
			tmpchar = vector<unsigned char>(m_exemplar_z[lv].begin(), m_exemplar_z[lv].end());
			DM3 = Mat(tmpchar, true).reshape(1, DM3.rows);
		}
		else {
			vector<unsigned short> tmpshort;
			DM3 = Mat(TEXSIZE_, TEXSIZE_, CV_16UC1);
			tmpshort = vector<unsigned short>(m_exemplar_z[lv].begin(), m_exemplar_z[lv].end());
			DM3 = Mat(tmpshort, true).reshape(1, DM3.rows);
		}
		name << "DM3_S" << (short)Solid_Upper[lv] << "_L" << to_string(lv) << ".png";
		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), DM3);
		name.str("");
		}
		cout << endl << "output TI.";	//_getch();
	}

	return true;
}

void DoPAR::testPCA() {
	//test PCA TI and back-project
	int level = MULTIRES - 1;
	const int TIsize_ = TIsize[level];
	int N = 5;		//11*11,256->246
	int D_NEIGHBOR = (1 + 2 * N)*(1 + 2 * N);
	double PCA_RATIO_VARIANCE = 0.99;

	cout << endl << "input PCA_RATIO:";
	cin >> PCA_RATIO_VARIANCE;
	if (PCA_RATIO_VARIANCE>1.0) { cout << endl << "wrong value, use 99%"; PCA_RATIO_VARIANCE = 0.99; }
	if (PCA_RATIO_VARIANCE <= 0) { cout << endl << "no PCA"; return; }
	int MINDIMS = 2;
	int MAXDIMS = 20;
	cout << endl << "input MINDIMS, MAXDIMS:";
	cin >> MINDIMS >> MAXDIMS;
	if (MINDIMS < 2) { cout << endl << "MINDIMS<2, use 2"; MINDIMS = 2; }
	if (MAXDIMS > D_NEIGHBOR) { cout << endl << "MINDIMS<D_NEIGHBOR, use D_NEIGHBOR"; MAXDIMS = D_NEIGHBOR; }


	int sizeneighbor = D_NEIGHBOR * (TIsize_ - 2 * N) * (TIsize_ - 2 * N);
	vector<size_color> m_neighbor_x(sizeneighbor, 0), m_neighbor_y(sizeneighbor, 0), m_neighbor_z(sizeneighbor, 0);
	CvMat* mp_neighbor_pca_average_x(NULL); CvMat* mp_neighbor_pca_average_y(NULL); CvMat* mp_neighbor_pca_average_z(NULL);
	CvMat* mp_neighbor_pca_projected_x(NULL); CvMat* mp_neighbor_pca_projected_y(NULL); CvMat* mp_neighbor_pca_projected_z(NULL);
	CvMat* mp_neighbor_pca_eigenvec_x(NULL); CvMat* mp_neighbor_pca_eigenvec_y(NULL); CvMat* mp_neighbor_pca_eigenvec_z(NULL);

	int numData = (TIsize_ - 2 * N) * (TIsize_ - 2 * N);
	CvMat* p_source_x = cvCreateMat(numData, D_NEIGHBOR, CV_32F);	//rows='area' numData, cols=dimension (Neighbour size)
	CvMat* p_source_y = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
	CvMat* p_source_z = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
	int row = 0;
	for (int v = N; v < TIsize_ - N; ++v) {
		for (int u = N; u < TIsize_ - N; ++u) {
			int col = 0;
			for (int dv = -N; dv <= N; ++dv) {
				for (int du = -N; du <= N; ++du) {
					ANNidx index = (TIsize_ * (v + dv) + u + du);
					cvmSet(p_source_x, row, col, m_exemplar_x[level][index]);	//set p_source_x(row,col) to m_examplar_x(idx)
					cvmSet(p_source_y, row, col, m_exemplar_y[level][index]);
					cvmSet(p_source_z, row, col, m_exemplar_z[level][index]);

					m_neighbor_x[D_NEIGHBOR * row + col] = m_exemplar_x[level][index];
					m_neighbor_y[D_NEIGHBOR * row + col] = m_exemplar_y[level][index];
					m_neighbor_z[D_NEIGHBOR * row + col] = m_exemplar_z[level][index];
					++col;
				}
			}
			++row;
		}
	}

	// PCA calculation (obtain all eigenvectors of the input covariance matrix)
	////////每一行表示一个样本
	//////CvMat* pData = cvCreateMat( 总的样本数, 每个样本的维数, CV_32FC1 );
	if (mp_neighbor_pca_average_x != NULL) cvReleaseMat(&mp_neighbor_pca_average_x);
	if (mp_neighbor_pca_average_y != NULL) cvReleaseMat(&mp_neighbor_pca_average_y);
	if (mp_neighbor_pca_average_z != NULL) cvReleaseMat(&mp_neighbor_pca_average_z);
	//CvMat* pMean = cvCreateMat(1, 样本的维数, CV_32FC1);
	mp_neighbor_pca_average_x = cvCreateMat(1, D_NEIGHBOR, CV_32F);
	mp_neighbor_pca_average_y = cvCreateMat(1, D_NEIGHBOR, CV_32F);
	mp_neighbor_pca_average_z = cvCreateMat(1, D_NEIGHBOR, CV_32F);
	//pEigVals中的每个数表示一个特征值
	//CvMat* pEigVals = cvCreateMat(1, min(总的样本数,样本的维数), CV_32FC1);
	CvMat* p_eigenValues_x = cvCreateMat(1, D_NEIGHBOR, CV_32F);
	CvMat* p_eigenValues_y = cvCreateMat(1, D_NEIGHBOR, CV_32F);
	CvMat* p_eigenValues_z = cvCreateMat(1, D_NEIGHBOR, CV_32F);
	//每一行表示一个特征向量
	//CvMat* pEigVecs = cvCreateMat( min(总的样本数,样本的维数), 样本的维数, CV_32FC1);
	CvMat* p_eigenVectors_all_x = cvCreateMat(D_NEIGHBOR, D_NEIGHBOR, CV_32F);
	CvMat* p_eigenVectors_all_y = cvCreateMat(D_NEIGHBOR, D_NEIGHBOR, CV_32F);
	CvMat* p_eigenVectors_all_z = cvCreateMat(D_NEIGHBOR, D_NEIGHBOR, CV_32F);
	//PCA处理,计算出平均向量pMean,特征值pEigVals和特征向量pEigVecs
	//cvCalcPCA(pData, pMean, pEigVals, pEigVecs, CV_PCA_DATA_AS_ROW);
	//now have better function //PCA pca(data, mean, PCA::DATA_AS_ROW, 0.95);
	cvCalcPCA(p_source_x, mp_neighbor_pca_average_x, p_eigenValues_x, p_eigenVectors_all_x, CV_PCA_DATA_AS_ROW);
	cvCalcPCA(p_source_y, mp_neighbor_pca_average_y, p_eigenValues_y, p_eigenVectors_all_y, CV_PCA_DATA_AS_ROW);
	cvCalcPCA(p_source_z, mp_neighbor_pca_average_z, p_eigenValues_z, p_eigenVectors_all_z, CV_PCA_DATA_AS_ROW);
	// Decide amount of dimensionality reduction
	double contribution_total_x = 0;
	double contribution_total_y = 0;
	double contribution_total_z = 0;
	for (int i = 0; i < D_NEIGHBOR; ++i) {
		contribution_total_x += cvmGet(p_eigenValues_x, 0, i);
		contribution_total_y += cvmGet(p_eigenValues_y, 0, i);
		contribution_total_z += cvmGet(p_eigenValues_z, 0, i);
	}

	int dimPCA_x = 0;
	int dimPCA_y = 0;
	int dimPCA_z = 0;

	double contribution_acc_x = 0;
	double contribution_acc_y = 0;
	double contribution_acc_z = 0;
	for (int i = 0; i < D_NEIGHBOR; ++i) {
		double ratio_x = contribution_acc_x / contribution_total_x;
		double ratio_y = contribution_acc_y / contribution_total_y;
		double ratio_z = contribution_acc_z / contribution_total_z;
		if (ratio_x < PCA_RATIO_VARIANCE || dimPCA_x < MINDIMS) {
			contribution_acc_x += cvmGet(p_eigenValues_x, 0, i);
			++dimPCA_x;
		}
		if (ratio_y < PCA_RATIO_VARIANCE) {
			contribution_acc_y += cvmGet(p_eigenValues_y, 0, i);
			++dimPCA_y;
		}
		if (ratio_z < PCA_RATIO_VARIANCE) {
			contribution_acc_z += cvmGet(p_eigenValues_z, 0, i);
			++dimPCA_z;
		}
		if (PCA_RATIO_VARIANCE <= ratio_x && PCA_RATIO_VARIANCE <= ratio_y && PCA_RATIO_VARIANCE <= ratio_z
			&& dimPCA_x >= MINDIMS) break;
		if (dimPCA_x >= MAXDIMS) break;
	}

	cout << endl;
	printf("PCA reduction (x): %d -> %d\n", D_NEIGHBOR, dimPCA_x);
	printf("PCA reduction (y): %d -> %d\n", D_NEIGHBOR, dimPCA_y);
	printf("PCA reduction (z): %d -> %d\n", D_NEIGHBOR, dimPCA_z);


	// Trim total eigenvectors into partial eigenvectors
	if (mp_neighbor_pca_eigenvec_x != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x);
	if (mp_neighbor_pca_eigenvec_y != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y);
	if (mp_neighbor_pca_eigenvec_z != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z);
	mp_neighbor_pca_eigenvec_x = cvCreateMat(dimPCA_x, D_NEIGHBOR, CV_32F);
	mp_neighbor_pca_eigenvec_y = cvCreateMat(dimPCA_y, D_NEIGHBOR, CV_32F);
	mp_neighbor_pca_eigenvec_z = cvCreateMat(dimPCA_z, D_NEIGHBOR, CV_32F);
	memcpy(mp_neighbor_pca_eigenvec_x->data.fl, p_eigenVectors_all_x->data.fl, sizeof(ANNcoord)* dimPCA_x * D_NEIGHBOR);
	memcpy(mp_neighbor_pca_eigenvec_y->data.fl, p_eigenVectors_all_y->data.fl, sizeof(ANNcoord)* dimPCA_y * D_NEIGHBOR);
	memcpy(mp_neighbor_pca_eigenvec_z->data.fl, p_eigenVectors_all_z->data.fl, sizeof(ANNcoord)* dimPCA_z * D_NEIGHBOR);
	// PCA projection
	//CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
	if (mp_neighbor_pca_projected_x != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x);
	if (mp_neighbor_pca_projected_y != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y);
	if (mp_neighbor_pca_projected_z != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z);
	//选出前P个特征向量(主成份),然后投影,结果保存在pResult中，pResult中包含了P个系数
	//CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
	mp_neighbor_pca_projected_x = cvCreateMat(numData, dimPCA_x, CV_32F);
	mp_neighbor_pca_projected_y = cvCreateMat(numData, dimPCA_y, CV_32F);
	mp_neighbor_pca_projected_z = cvCreateMat(numData, dimPCA_z, CV_32F);
	//cvProjectPCA( pData, pMean, pEigVecs, pResult );
	cvProjectPCA(p_source_x, mp_neighbor_pca_average_x, mp_neighbor_pca_eigenvec_x, mp_neighbor_pca_projected_x);
	cvProjectPCA(p_source_y, mp_neighbor_pca_average_y, mp_neighbor_pca_eigenvec_y, mp_neighbor_pca_projected_y);
	cvProjectPCA(p_source_z, mp_neighbor_pca_average_z, mp_neighbor_pca_eigenvec_z, mp_neighbor_pca_projected_z);

	//// kd-tree construction
	//m_neighbor_kdTree_ptr_x[level].resize(numData);
	//m_neighbor_kdTree_ptr_y[level].resize(numData);
	//m_neighbor_kdTree_ptr_z[level].resize(numData);
	//for (int i = 0; i < numData; ++i) {
	//	//ANNpoint* point array, row = 1, col = PCA dimension
	//	m_neighbor_kdTree_ptr_x[level][i] = &mp_neighbor_pca_projected_x->data.fl[dimPCA_x * i];	
	////ANNpoint* from PCA projection
	////vector<uchar> array(mat.rows*mat.cols);
	////if (mat.isContinuous())
	////	array = mat.data;
	//	m_neighbor_kdTree_ptr_y[level][i] = &mp_neighbor_pca_projected_y->data.fl[dimPCA_y * i];
	//	m_neighbor_kdTree_ptr_z[level][i] = &mp_neighbor_pca_projected_z->data.fl[dimPCA_z * i];
	//}
	//if (mp_neighbor_kdTree_x[level] != NULL) delete mp_neighbor_kdTree_x[level];
	//if (mp_neighbor_kdTree_y[level] != NULL) delete mp_neighbor_kdTree_y[level];
	//if (mp_neighbor_kdTree_z[level] != NULL) delete mp_neighbor_kdTree_z[level];
	////ANNpoint* data point array = m_neighbor_kdTree_ptr_x, number of points = numData, dimension = dimPCA_x
	//mp_neighbor_kdTree_x[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_x[level][0], numData, dimPCA_x); //ANNkd_tree
	//mp_neighbor_kdTree_y[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_y[level][0], numData, dimPCA_y);
	//mp_neighbor_kdTree_z[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_z[level][0], numData, dimPCA_z);

	//============ TEST TI PCA backproject result
	if (true) {
		CvMat* backproject_x = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
		cvBackProjectPCA(mp_neighbor_pca_projected_x, mp_neighbor_pca_average_x, mp_neighbor_pca_eigenvec_x, backproject_x);
		Mat backprojectMat_x = cvarrToMat(backproject_x);
		Mat PCAbackprojectDM1 = Mat(TIsize_ - 2 * N, TIsize_ - 2 * N, CV_8UC1);

		CvMat* backproject_y = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
		cvBackProjectPCA(mp_neighbor_pca_projected_y, mp_neighbor_pca_average_y, mp_neighbor_pca_eigenvec_y, backproject_y);
		Mat backprojectMat_y = cvarrToMat(backproject_y);
		Mat PCAbackprojectDM2 = Mat(TIsize_ - 2 * N, TIsize_ - 2 * N, CV_8UC1);

		CvMat* backproject_z = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
		cvBackProjectPCA(mp_neighbor_pca_projected_z, mp_neighbor_pca_average_z, mp_neighbor_pca_eigenvec_z, backproject_z);
		Mat backprojectMat_z = cvarrToMat(backproject_z);
		Mat PCAbackprojectDM3 = Mat(TIsize_ - 2 * N, TIsize_ - 2 * N, CV_8UC1);

		int row = 0;
		int cols = ((2 * N + 1)*(2 * N + 1) - 1) * 0.5;

		for (int v = 0; v < TIsize_ - 2 * N; ++v) {
			for (int u = 0; u < TIsize_ - 2 * N; ++u) {
				int tempv = backprojectMat_x.at<ANNcoord>(row, cols);
				if (tempv < 0) tempv = 0;	else if (tempv > 255) tempv = 255;
				PCAbackprojectDM1.at<uchar>(v, u) = tempv;

				tempv = backprojectMat_y.at<ANNcoord>(row, cols);
				if (tempv < 0) tempv = 0;	else if (tempv > 255) tempv = 255;
				PCAbackprojectDM2.at<uchar>(v, u) = tempv;

				tempv = backprojectMat_z.at<ANNcoord>(row, cols);
				if (tempv < 0) tempv = 0;	else if (tempv > 255) tempv = 255;
				PCAbackprojectDM3.at<uchar>(v, u) = tempv;

				++row;
			}
		}

		imwrite("PCA_TI1.png", PCAbackprojectDM1);
		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite("PCA_TI2.png", PCAbackprojectDM2);
		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite("PCA_TI3.png", PCAbackprojectDM3);
		cvReleaseMat(&backproject_x);
		cvReleaseMat(&backproject_y);
		cvReleaseMat(&backproject_z);
		cout << endl << "PCA back projected image outputed.";
	}

	// release CV matrices
	cvReleaseMat(&p_source_x);
	cvReleaseMat(&p_source_y);
	cvReleaseMat(&p_source_z);
	cvReleaseMat(&p_eigenValues_x);
	cvReleaseMat(&p_eigenValues_y);
	cvReleaseMat(&p_eigenValues_z);
	cvReleaseMat(&p_eigenVectors_all_x);
	cvReleaseMat(&p_eigenVectors_all_y);
	cvReleaseMat(&p_eigenVectors_all_z);
}

void DoPAR::gaussImage(int level, vector<vector<size_color>>& exemplar) {
	if (level == 0) return;

	//const float GAUSSWEIGHT[3][3] = {	{ 0.0625, 0.1250, 0.0625 },
	//									{ 0.1250, 0.2500, 0.1250 },
	//									{ 0.0625, 0.1250, 0.0625 } };  //sigma=0.85
	const float GAUSSWEIGHT[3][3] = {
		{ 0.0509,    0.1238,    0.0509 },
		{ 0.1238,    0.3012,    0.1238 },
		{ 0.0509,    0.1238,    0.0509 }
	};//sigma=0.75

	size_idx width = TIsize[level];
	size_idx cwidth = static_cast<size_idx>(width / 2);
	float sumcolor, sumweight(FLT_MIN);

	for (int i = 0; i < cwidth; i++) {
		for (int j = 0; j < cwidth; j++) {
			sumweight = 0.0f;
			sumcolor = 0.0f;
			for (int k = 2 * i - 1; k < 2 * i + 1; k++) {
				for (int l = 2 * j - 1; l <= 2 * j + 1; ++l) {
					if (k >= 0 && k < width && l >= 0 && l < width) {
						float w = GAUSSWEIGHT[k - 2 * i + 1][l - 2 * j + 1];
						sumcolor += w * exemplar[level][k*width + l];
						sumweight += w;
					}
				}
			}
			exemplar[level - 1][i*cwidth + j] = sumcolor / sumweight;
		}
	}
}

void DoPAR::equalizeHistogram(int level, vector<size_color>& exemplarX, vector<size_color>& exemplarY, vector<size_color>& exemplarZ)
{
	long total = exemplarX.size() + exemplarY.size() + exemplarZ.size();

	unsigned short maxv = max(*max_element(exemplarX.begin(), exemplarX.end()), max(*max_element(exemplarY.begin(), exemplarY.end()), *max_element(exemplarZ.begin(), exemplarZ.end())));
	unsigned short n_bins = maxv + 1;

	// Compute histogram
	vector<long> hist(n_bins, 0);
	for (long i = 0; i < exemplarX.size(); ++i)
		hist[exemplarX[i]]++;
	for (long j = 0; j < exemplarY.size(); ++j)
		hist[exemplarY[j]]++;
	for (long k = 0; k < exemplarZ.size(); ++k)
		hist[exemplarZ[k]]++;

	// Build LUT from cumulative histrogram

	// Compute scale
	double scale = min(255.0, n_bins - 1.0) / total;	//limit bins up to 256

														// Initialize lut
	vector<unsigned short> lut(n_bins, 0);
	//i++;
	long sum = 0;
	int lastv = -1, actualbin = -1, pressedSolid_Upper = 0, pressedPore_Lower = 0;
	vector<unsigned short> actuallist(n_bins, 0);
	for (unsigned short i = 0; i < hist.size(); ++i) {
		sum += hist[i];
		// the value is saturated in range [0, max_val]
		//lut[i] = max(unsigned short(0), min(unsigned short(floor(sum * scale)), maxv));
		lut[i] = max(unsigned short(0), min(unsigned short(round(sum * scale)), maxv));

		if (lut[i] != lastv) {
			lastv = lut[i];
			actualbin++;
		}
		actuallist[i] = actualbin;
		if (i == _Solid_Upper) pressedSolid_Upper = actualbin;
		if (i == _Pore_Lower) pressedPore_Lower = actualbin;
	}

	bool ispressed = false;

	if (!ispressed) {
		// equalization without press
		for (long k = 0; k < exemplarX.size(); ++k)
			exemplarX[k] = lut[exemplarX[k]];
		for (long k = 0; k < exemplarY.size(); ++k)
			exemplarY[k] = lut[exemplarY[k]];
		for (long k = 0; k < exemplarZ.size(); ++k)
			exemplarZ[k] = lut[exemplarZ[k]];
		//Solid_Upper[level] = lut[Solid_Upper[MULTIRES - 1]];
		//Pore_Upper[level] = min(255, n_bins - 1);
		//Pore_Lower[level] = lut[Pore_Lower[MULTIRES - 1]];
		Solid_Upper[level] = lut[_Solid_Upper];
		Pore_Upper[level] = min(255, n_bins - 1);
		Pore_Lower[level] = lut[_Pore_Lower];
	}
	else {
		// equalization with compress
		for (long k = 0; k < exemplarX.size(); ++k)
			exemplarX[k] = actuallist[exemplarX[k]];
		for (long k = 0; k < exemplarY.size(); ++k)
			exemplarY[k] = actuallist[exemplarY[k]];
		for (long k = 0; k < exemplarZ.size(); ++k)
			exemplarZ[k] = actuallist[exemplarZ[k]];
	}


	int mincount = total / min(255.0, n_bins - 1.0) / 10;

	if (!ispressed) {
		//check first ? mincount
		int firstbincountX = count(exemplarX.begin(), exemplarX.end(), lut[0]);
		int firstbincountY = count(exemplarY.begin(), exemplarY.end(), lut[0]);
		int firstbincountZ = count(exemplarZ.begin(), exemplarZ.end(), lut[0]);
		if (firstbincountX < mincount || firstbincountY < mincount || firstbincountZ < mincount) {
			replace(exemplarX.begin(), exemplarX.end(), lut[0], lut[1]);
			replace(exemplarY.begin(), exemplarY.end(), lut[0], lut[1]);
			replace(exemplarZ.begin(), exemplarZ.end(), lut[0], lut[1]);
		}
		//check last bin ? mincount
		int lastbincountX = count(exemplarX.begin(), exemplarX.end(), lut[n_bins - 1]);
		int lastbincountY = count(exemplarY.begin(), exemplarY.end(), lut[n_bins - 1]);
		int lastbincountZ = count(exemplarZ.begin(), exemplarZ.end(), lut[n_bins - 1]);
		if (lastbincountX < mincount || lastbincountY < mincount || lastbincountZ < mincount) {
			replace(exemplarX.begin(), exemplarX.end(), lut[n_bins - 1], lut[n_bins - 1 - 1]);
			replace(exemplarY.begin(), exemplarY.end(), lut[n_bins - 1], lut[n_bins - 1 - 1]);
			replace(exemplarZ.begin(), exemplarZ.end(), lut[n_bins - 1], lut[n_bins - 1 - 1]);
		}
	}
	else {
		//check first ? mincount
		int firstbincountX = count(exemplarX.begin(), exemplarX.end(), actuallist[0]);
		int firstbincountY = count(exemplarY.begin(), exemplarY.end(), actuallist[0]);
		int firstbincountZ = count(exemplarZ.begin(), exemplarZ.end(), actuallist[0]);
		if (firstbincountX < mincount || firstbincountY < mincount || firstbincountZ < mincount) {
			replace(exemplarX.begin(), exemplarX.end(), actuallist[0], actuallist[1]);
			replace(exemplarY.begin(), exemplarY.end(), actuallist[0], actuallist[1]);
			replace(exemplarZ.begin(), exemplarZ.end(), actuallist[0], actuallist[1]);
		}
		Solid_Upper[level] = pressedSolid_Upper;
		Pore_Lower[level] = pressedPore_Lower;

		//check last bin ? mincount
		int lastbincountX = count(exemplarX.begin(), exemplarX.end(), actuallist[actualbin]);
		int lastbincountY = count(exemplarY.begin(), exemplarY.end(), actuallist[actualbin]);
		int lastbincountZ = count(exemplarZ.begin(), exemplarZ.end(), actuallist[actualbin]);
		if (lastbincountX < mincount || lastbincountY < mincount || lastbincountZ < mincount) {
			replace(exemplarX.begin(), exemplarX.end(), actuallist[actualbin], actuallist[actualbin - 1]);
			replace(exemplarY.begin(), exemplarY.end(), actuallist[actualbin], actuallist[actualbin - 1]);
			replace(exemplarZ.begin(), exemplarZ.end(), actuallist[actualbin], actuallist[actualbin - 1]);
			actualbin--;
		}
		Pore_Upper[level] = actualbin;
	}



	//cout << endl <<"lastbin="<< actualbin << " mincount=" << mincount << " lastbincountX=" << lastbincountX << " Y=" << lastbincountY << " Z=" << lastbincountZ; 
	//_getch();

	cout << endl << "level " << level << ":   max before=" << maxv << "   actual bins=" << actualbin + 1;
}


void DoPAR::initPermutation(int level) {// random permutation (precomputed)
	size_idx Size = OUTsize[level] * OUTsize[level] * OUTsize[level];
	if (SIM2D_YN) Size = OUTsize[level] * OUTsize[level];
	
	m_permutation.clear();
	m_permutation.resize(Size);
	for (size_idx i = 0; i <Size; ++i) {
		m_permutation[i] = i;
	}
}

// Init Random Volume
bool DoPAR::loadVolume() {
	cout << endl << "Use Random initial.";
	InitRandomVolume(0);
	return true;
}
//randomly assign Origin & color
void DoPAR::InitRandomVolume(int level) {
	const size_idx TEXSIZE_ = OUTsize[level];
	const size_idx blockSize_ = blockSize[level];
	const size_idx Sx = TEXSIZE_;
	const size_idx Sy = TEXSIZE_;
	const size_idx Sz = TEXSIZE_;
	const size_idx Sxy = Sx * Sy;
	const size_idx Sxz = Sx * Sz;
	const size_idx Syz = Sy * Sz;
	size_idx Size = Sxy * Sz;
	if (SIM2D_YN) Size = Sxy;
	vector<size_idx> randomidx2d(3);
	vector<size_color>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	int ori;

	if (SIM2D_YN) {
		for (size_idx xyz = 0; xyz < Size; ++xyz) {
			randomidx2d[0] = rand() % Syz;
			Origin_x[level][xyz] = randomidx2d[0];
			isUnchanged_x[level][xyz] = false;

			m_volume[level][xyz] = m_exemplar_x[level][randomidx2d[0]];
		}
	}
	else {
		for (size_idx xyz = 0; xyz < Size; ++xyz) {
			randomidx2d[0] = rand() % Syz;
			Origin_x[level][xyz] = randomidx2d[0];
			isUnchanged_x[level][xyz] = false;

			randomidx2d[1] = rand() % Sxz;
			Origin_y[level][xyz] = randomidx2d[1];
			isUnchanged_y[level][xyz] = false;

			randomidx2d[2] = rand() % Sxy;
			Origin_z[level][xyz] = randomidx2d[2];
			isUnchanged_z[level][xyz] = false;

			m_volume[level][xyz] = (m_exemplar_x[level][randomidx2d[0]] + m_exemplar_y[level][randomidx2d[1]] + m_exemplar_z[level][randomidx2d[2]]) / 3.0f;
		}
	}

	if (SIM2D_YN && MULTIRES == 1) {
		outputmodel(level);
	}
}

// assign fixed layer
//void DoPAR::AssignFixedLayer(int level, int dir) {
//	//! need modification when TIsize!=OUTsize
//
//	//assign origin, m_volume in direction(dir)
//
//	if (SIM2D_YN || dir<0 || dir>2) return;
//	//! need modification when TIsize!=OUTsize
//	size_idx TEXSIZE_ = TEXSIZE[level];
//	size_idx Sx = TEXSIZE_;
//	size_idx Sy = TEXSIZE_;
//	size_idx Sz = TEXSIZE_;
//	size_idx Sxy = Sx * Sy;
//	size_idx Sxz = Sx * Sz;
//	size_idx Syz = Sy * Sz;
//	size_idx temp3didx;
//	size_idx baseidx_top, baseidx_middle, baseidx_bottom;
//
//	switch (dir)
//	{
//	case(0) :	
//		baseidx_top = 0;						//fix YZ layer, [0][j][k]
//		baseidx_middle = (Sx / 2 - 1) * Syz;	//fix YZ layer, in the middle of x direction [1/2x-1][j][k]		
//		baseidx_bottom = (Sx - 1) * Syz;		//fix YZ layer, [Sx-1][j][k]	
//
//		for (size_idx jk = 0; jk < Syz; jk++) {
//			//temp3didx = baseidx_top + jk;
//			//Origin_x[level][temp3didx] = jk;
//			//m_volume[level][temp3didx] = m_exemplar_x[level][jk];
//
//			temp3didx = baseidx_middle + jk;
//			Origin_x[level][temp3didx] = jk;
//			m_volume[level][temp3didx] = m_exemplar_x[level][jk];
//			
//			//temp3didx = baseidx_bottom + jk;
//			//Origin_x[level][temp3didx] = jk;
//			//m_volume[level][temp3didx] = m_exemplar_x[level][jk];
//		}
//		break;
//	case(1) :
//		baseidx_top = 0;						//fix ZX layer, [i][0][k]
//		baseidx_middle = Sz*(Sy / 2 - 1);		//fix ZX layer, in the middle of y direction [i][1/2y-1][k]
//		baseidx_bottom = Sz*(Sy-1);				//fix ZX layer, [i][Sy-1][k]
//
//		size_idx ik2d, ik3d;
//		for (size_idx i = 0; i < Sx; i++) {
//			for (size_idx k = 0; k < Sz; k++) {
//				ik2d = i*Sz + k;
//				ik3d = i*Syz + k;
//
//				//temp3didx = baseidx_top + ik3d;
//				//Origin_y[level][temp3didx] = ik2d;
//				//m_volume[level][temp3didx] = m_exemplar_y[level][ik2d];
//
//				temp3didx = baseidx_middle + ik3d;
//				Origin_y[level][temp3didx] = ik2d;
//				m_volume[level][temp3didx] = m_exemplar_y[level][ik2d];
//				
//				//temp3didx = baseidx_bottom + ik3d;
//				//Origin_y[level][temp3didx] = ik2d;
//				//m_volume[level][temp3didx] = m_exemplar_y[level][ik2d];
//			}
//		}
//		break;
//	case(2) :
//		baseidx_top = 0;						//fix XY layer, [i][j][0]
//		baseidx_middle = Sz / 2 - 1;			//fix XY layer, in the middle of z direction [i][j][1/2z]
//		baseidx_bottom = Sz - 1;				//fix XY layer, [i][j][Sz-1]
//
//		size_idx ij2d, ij3d;
//		for (size_idx i = 0; i < Sx; i++) {
//			for (size_idx j = 0; j < Sy; j++) {
//				ij2d = i*Sy + j;
//				ij3d = i*Syz + j*Sz;
//
//				//temp3didx = baseidx_top + ij3d;
//				//Origin_z[level][temp3didx] = ij2d;
//				//m_volume[level][temp3didx] = m_exemplar_z[level][ij2d];
//
//				temp3didx = baseidx_middle + ij3d;
//				Origin_z[level][temp3didx] = ij2d;
//				m_volume[level][temp3didx] = m_exemplar_z[level][ij2d];
//
//				//temp3didx = baseidx_bottom + ij3d;
//				//Origin_z[level][temp3didx] = ij2d;
//				//m_volume[level][temp3didx] = m_exemplar_z[level][ij2d];
//			}
//		}	
//		break;
//	default:
//		break;
//	}
//}


// clear data
void DoPAR::cleardata(int level) {
	m_volume[level].clear();
	m_exemplar_x[level].clear();	m_exemplar_y[level].clear();	m_exemplar_z[level].clear();
	KCoherence_x[level].clear();	KCoherence_y[level].clear();	KCoherence_z[level].clear();
	isUnchanged_x[level].clear();	isUnchanged_y[level].clear();	isUnchanged_z[level].clear();
	nearestIdx_x[level].clear();	nearestIdx_y[level].clear();	nearestIdx_z[level].clear();
	nearestWeight_x[level].clear(); nearestWeight_y[level].clear(); nearestWeight_z[level].clear();
	Origin_x[level].clear();		Origin_y[level].clear();		Origin_z[level].clear();
	IndexHis_x[level].clear();		IndexHis_y[level].clear();		IndexHis_z[level].clear();
	PosHis[level].clear();
	SelectedPos[level].clear();

	m_volume[level].shrink_to_fit();
	m_exemplar_x[level].shrink_to_fit();	m_exemplar_y[level].shrink_to_fit();	m_exemplar_z[level].shrink_to_fit();
	KCoherence_x[level].shrink_to_fit();	KCoherence_y[level].shrink_to_fit();	KCoherence_z[level].shrink_to_fit();
	isUnchanged_x[level].shrink_to_fit();	
	isUnchanged_y[level].shrink_to_fit();	
	isUnchanged_z[level].shrink_to_fit();
	nearestIdx_x[level].shrink_to_fit();	
	nearestIdx_y[level].shrink_to_fit();	
	nearestIdx_z[level].shrink_to_fit();
	nearestWeight_x[level].shrink_to_fit();	nearestWeight_y[level].shrink_to_fit();	nearestWeight_z[level].shrink_to_fit();
	Origin_x[level].shrink_to_fit();		Origin_y[level].shrink_to_fit();		Origin_z[level].shrink_to_fit();
	IndexHis_x[level].shrink_to_fit();		IndexHis_y[level].shrink_to_fit();		IndexHis_z[level].shrink_to_fit();
	PosHis[level].shrink_to_fit();
	SelectedPos[level].shrink_to_fit();

	if (level == MULTIRES - 1) { 
		m_permutation.clear(); 
		m_permutation.shrink_to_fit(); 
	}
}
void DoPAR::cleardata() {
	for (int level = 0; level < MULTIRES - 1; ++level) {
		m_volume[level].clear();
		m_exemplar_x[level].clear();	m_exemplar_y[level].clear();	m_exemplar_z[level].clear();
		KCoherence_x[level].clear();	KCoherence_y[level].clear();	KCoherence_z[level].clear();
		isUnchanged_x[level].clear();	isUnchanged_y[level].clear();	isUnchanged_z[level].clear();
		nearestIdx_x[level].clear();	nearestIdx_y[level].clear();	nearestIdx_z[level].clear();
		nearestWeight_x[level].clear(); nearestWeight_y[level].clear(); nearestWeight_z[level].clear();
		Origin_x[level].clear();		Origin_y[level].clear();		Origin_z[level].clear();
		IndexHis_x[level].clear();		IndexHis_y[level].clear();		IndexHis_z[level].clear();
		PosHis[level].clear();
		SelectedPos[level].clear();

		m_volume[level].shrink_to_fit();
		m_exemplar_x[level].shrink_to_fit();	m_exemplar_y[level].shrink_to_fit();	m_exemplar_z[level].shrink_to_fit();
		KCoherence_x[level].shrink_to_fit();	KCoherence_y[level].shrink_to_fit();	KCoherence_z[level].shrink_to_fit();
		isUnchanged_x[level].shrink_to_fit();
		isUnchanged_y[level].shrink_to_fit();
		isUnchanged_z[level].shrink_to_fit();
		nearestIdx_x[level].shrink_to_fit();
		nearestIdx_y[level].shrink_to_fit();
		nearestIdx_z[level].shrink_to_fit();
		nearestWeight_x[level].shrink_to_fit();	nearestWeight_y[level].shrink_to_fit();	nearestWeight_z[level].shrink_to_fit();
		Origin_x[level].shrink_to_fit();		Origin_y[level].shrink_to_fit();		Origin_z[level].shrink_to_fit();
		IndexHis_x[level].shrink_to_fit();		IndexHis_y[level].shrink_to_fit();		IndexHis_z[level].shrink_to_fit();
		PosHis[level].shrink_to_fit();
		SelectedPos[level].shrink_to_fit();
	}

	m_permutation.clear();
	m_permutation.shrink_to_fit();
}

// write model
void DoPAR::outputmodel(int level) {
	//vector<uchar> tempUchar(m_volume[level].size());
	vector<uchar> tempUchar(m_volume[level].begin(), m_volume[level].end());
	string tempoutputfilename = outputfilename;
	short resizedSolid_Upper;
	size_idx OUTsize_ = OUTsize[level];

	if (SIM2D_YN) {
		if (!DMtransformYN) {
			tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + ".png";

			int i(1);
			string nonrepeatFPName = tempoutputfilename;
			while (fileExists(nonrepeatFPName) == true) {
				nonrepeatFPName = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_" + to_string(i) + ".png";
				i++;
			}

			Mat tempM = Mat(OUTsize_, OUTsize_, CV_8UC1);
			tempM = Mat(tempUchar, true).reshape(1, tempM.rows);
			imwrite(nonrepeatFPName, tempM);
		}
		else if (DMtransformYN) {
			tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + "DM.png";
			int i(1);
			string nonrepeatFPName = tempoutputfilename;
			while (fileExists(nonrepeatFPName) == true) {
				nonrepeatFPName = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_" + to_string(i) + ".png";
				i++;
			}
			Mat tempM = Mat(OUTsize_, OUTsize_, CV_8UC1);
			tempM = Mat(tempUchar, true).reshape(1, tempM.rows);
			if (GenerateTI)	imwrite(nonrepeatFPName, tempM);

			// binary model
			vector<short> tempshort(m_volume[level].begin(), m_volume[level].end());
			//binaryUchar(tempshort, tempUchar, Solid_Upper[level] + 1);						// binary thresholded to 0&255
			binaryUchar(tempshort, tempUchar, (Solid_Upper[MULTIRES-1] + Pore_Lower[MULTIRES - 1]) / 2);
			tempM = Mat(tempUchar, true).reshape(1, tempM.rows);
			i = 1;
			nonrepeatFPName = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_" + to_string(i) + ".png";
			while (fileExists(nonrepeatFPName) == true) {
				nonrepeatFPName = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_" + to_string(i) + ".png";
				i++;
			}
			imwrite(nonrepeatFPName, tempM);
		}
	}
	else {
		if (GenerateTI) {
			tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + "DM.RAW";
			Write(outputpath + tempoutputfilename, tempUchar);
		}


		if (DMtransformYN) {
			// binary model
			vector<short> tempshort(m_volume[level].begin(), m_volume[level].end());
			binaryUchar(tempshort, tempUchar, (Solid_Upper[level] + Pore_Lower[level]) / 2);						// binary thresholded to 0&255
			//binaryUchar(tempshort, tempUchar, (Solid_Upper[MULTIRES - 1] + Pore_Lower[MULTIRES - 1]) / 2);
		}
		//tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "_Size" + to_string(TEXSIZE[level]) + ".RAW";
		tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + ".RAW";
		Write(outputpath + tempoutputfilename, tempUchar);
	}


	cout << endl << "output done.";

	//ofstream colorhis_syn("colorhis_syn.csv");
	//ofstream colorhis_ti("colorhis_ti.csv");
	//int vsize = ColorHis_synthesis[level].size();
	//for (int n = 0; n<vsize; n++){
	//	colorhis_syn << ColorHis_synthesis[level][n] << endl;
	//	colorhis_ti << ColorHis_exemplar[level][n] << endl;
	//}
	//colorhis_syn.close();
	//colorhis_ti.close();
	//cout << endl << "colorhis outputed.";
}

// ============== Pattern entropy analysis ===========
void DoPAR::patternentropyanalysis(int templatesize, Mat &exemplar, double &entropy) {
	//compute pattern entropy for given template,TI
	int Width = exemplar.cols;
	int Height = exemplar.rows;
	//initial
	Mat countmask = Mat::zeros(Height - templatesize + 1, Width - templatesize + 1, CV_8UC1);
	vector<long> patterncount, patternloc;
	patterncount.reserve((Width - templatesize)*(Height - templatesize));
	patternloc.reserve((Width - templatesize)*(Height - templatesize));
	long totalcount(0);
	entropy = 0;
	float threshval = 4.0f;

	//build pattern database
	for (int y = 0; y < Height - templatesize + 1; y++) {
		//if (y % (Height / 10) == 0)cout << ".";
#pragma omp parallel for schedule(static)
		for (int x = 0; x < Width - templatesize + 1; x++) {
			//skip 
			if (countmask.at<uchar>(y, x) != 0) continue;
			//template matching
			Mat matchdst;		//32-bit, (H-h+1, W-w+1)
			Mat tempmat = exemplar(Rect(x, y, templatesize, templatesize));
			matchTemplate(exemplar, tempmat, matchdst, TM_SQDIFF);
			//build database
			Mat dstmask;
			long tempcount;

			if (matchdst.at<float>(y, x) < -threshval || matchdst.at<float>(y, x) > threshval) {
				threshval = abs(matchdst.at<float>(y, x));
			}
			inRange(matchdst, -threshval, threshval, dstmask);	//dstmask 255&0
			tempcount = countNonZero(dstmask);		

#pragma omp critical (patternanalysis)
			{
				//check again, due to OpenMP
				if (countmask.at<uchar>(y, x) == 0) {
					patterncount.push_back(tempcount);
					patternloc.push_back(y*Width + x);
					totalcount += tempcount;
					countmask.setTo(1, dstmask);
				}
			}

		}
	//#pragma omp parallel for schedule(static)
	}

	if (patterncount.size() != patternloc.size()) { cout << endl << "error:patterncount.size!=patternloc.size"; _getch(); exit(0); }
	long tempsize = (Height - templatesize + 1)*(Width - templatesize + 1);
	Mat tempmat;
	inRange(countmask, 2, tempsize, tempmat);
	if (countNonZero(tempmat)>0) { cout << endl << "error:countmask has >1:" << countNonZero(tempmat); }
	if (countNonZero(countmask)< tempsize) { cout << endl << "error:countmask has 0: " << tempsize - countNonZero(countmask); }

	//compute entropy	
	for (int i = 0; i < patterncount.size(); i++) {
		double pi = patterncount[i] * 1.0 / totalcount;
		entropy -= pi * log(pi);
	}
	cout << endl << "template:" << templatesize << " entropy=   " << entropy << "   pattern count=   " << patterncount.size();
}



// ============== distance map ===============
void DoPAR::binaryChar(vector<short>& DMap, vector<char>& Binarised, short threshold = 110) {
	//input  vector<short> DMap		//output vector<char>Binarised
	for (long i = 0; i < DMap.size(); i++) {
		if (DMap[i] <= threshold) Binarised[i] = 0;
		else Binarised[i] = 1;
	}
}
void DoPAR::binaryUchar(vector<short>& DMap, vector<uchar>& Binarised, short threshold) {
	//input  vector<short> DMap		//output vector<uchar>Binarised
	for (long i = 0; i < DMap.size(); i++) {
		if (DMap[i] <= threshold) Binarised[i] = 0;
		else Binarised[i] = 255;
	}
}
vector<unsigned short> DoPAR::BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg) {
	//(1) tSx, tSy, tSz: 3 dimensions of image OImg
	//       tSz = 1 - for a 2D image, otherwise tSz >=3
	//(2) OImg is a binary image, <= 0 for solid, > 0 for pores

	int Sx(3), Sy(3), Sz(3), SizeXY, Size;

	if (tSx > 3) Sx = tSx;
	if (tSy > 3) Sy = tSy;
	if (tSz > 0) Sz = tSz;

	SizeXY = Sx*Sy;
	Size = SizeXY*Sz;

	vector<unsigned short> DMap;

	if (OImg.size() != Size) {
		cout << "Distance Transfromation Error: The size of original image is wrong !" << endl;
		cout << OImg.size() << " != " << Size << endl;
		cout << "Dimensions: (8) " << Sx << "x" << Sy << "x" << Sz << endl;
		return DMap;
	}

	vector<long> ForeIdxV(27, 0);
	vector<long> Gx(27, 0), G2x(27, 0), AbsGx(27, 0), AbsVal(27);
	vector<long> Gy(27, 0), G2y(27, 0), AbsGy(27, 0);
	vector<long> Gz(27, 0), G2z(29, 0), AbsGz(27, 0);
	//The difference of relative coordination of current voxel and its 26-neighbours

	{  //Initialize constant parameters
		for (long CNum = 0, k = -1; k < 2; ++k) {
			for (long j = -1; j < 2; ++j) {
				for (long i = -1; i < 2; ++i) {
					Gx[CNum] = -i; G2x[CNum] = 2 * Gx[CNum];
					Gy[CNum] = -j; G2y[CNum] = 2 * Gy[CNum];
					Gz[CNum] = -k; G2z[CNum] = 2 * Gz[CNum];
					AbsGx[CNum] = abs(Gx[CNum]);
					AbsGy[CNum] = abs(Gy[CNum]);
					AbsGz[CNum] = abs(Gz[CNum]);
					AbsVal[CNum] = AbsGx[CNum] + AbsGy[CNum] + AbsGz[CNum];
					ForeIdxV[CNum++] = i + j*Sx + k*SizeXY;
				}
			}
		}
	}  //Initialize constant parameters

	DMap.resize(Size, 0);

	unsigned short MaxDistV = 65500;

	{ //Initialize DMap and clear up OImg
		for (long idx = 0; idx < Size; ++idx)
			if (OImg[idx] > 0)
				DMap[idx] = MaxDistV;
	} //Initialize DMap and clear up OImg

	vector<_XyzStrType> TgtImg(Size);  //coordinates (x,y,z) ///@~@

	short x, y, z, i, j, k, CNum;
	long idx, NIdx, CurVal;
	_XyzStrType TVal;

	TVal.x = 0; TVal.y = 0; TVal.z = 0;

	//cout<<" scanning forwards...";
	for (idx = -1, z = 0; z < Sz; ++z) {
		//if (z % 100 == 0) cout << "*";
		for (y = 0; y < Sy; ++y) {
			for (x = 0; x < Sx; ++x) { 	//if(++JCnt >= JStepNum) {JCnt=0; cout<<".";}
				if (DMap[++idx] < 1) {
					TgtImg[idx] = TVal;
				}
				else {
					for (CNum = -1, k = z - 1; k < z + 2; ++k) {
						if (k < 0 || k >= Sz) { CNum += 9; continue; }
						for (j = y - 1; j < y + 2; ++j) {
							if (j < 0 || j >= Sy) { CNum += 3; continue; }
							for (i = x - 1; i < x + 2; ++i) {
								++CNum; if (i < 0 || i >= Sx) continue;
								if (CNum > 12) goto BarJump1;

								NIdx = idx + ForeIdxV[CNum];

								if (DMap[NIdx] == MaxDistV) continue;

								CurVal = AbsVal[CNum] + G2x[CNum] * TgtImg[NIdx].x
									+ G2y[CNum] * TgtImg[NIdx].y
									+ G2z[CNum] * TgtImg[NIdx].z
									+ DMap[NIdx];

								if (CurVal < DMap[idx]) {
									DMap[idx] = CurVal;
									TgtImg[idx].x = TgtImg[NIdx].x + Gx[CNum];
									TgtImg[idx].y = TgtImg[NIdx].y + Gy[CNum];
									TgtImg[idx].z = TgtImg[NIdx].z + Gz[CNum];
								} //if(CurVal < DMap[idx])
							}
						}
					}
				BarJump1:;
				}
			}
		}
	}

	//cout<<"  scanning backwards...";
	for (idx = Size, z = Sz - 1; z >= 0; --z) {
		//if (z % 100 == 0) cout << ".";
		for (y = Sy - 1; y >= 0; --y) {
			for (x = Sx - 1; x >= 0; --x) { //if(++JCnt >= JStepNum) {JCnt=0; cout<<".";}
				if (DMap[--idx] <= 0) continue;
				for (CNum = 27, k = z + 1; k > z - 2; --k) {
					if (k < 0 || k >= Sz) { CNum -= 9; continue; }
					for (j = y + 1; j > y - 2; --j) {
						if (j < 0 || j >= Sy) { CNum -= 3; continue; }
						for (i = x + 1; i > x - 2; --i) {
							--CNum;  if (i < 0 || i >= Sx) continue;
							if (CNum < 14) goto BarJump2;

							NIdx = idx + ForeIdxV[CNum];
							CurVal = AbsVal[CNum] + G2x[CNum] * TgtImg[NIdx].x
								+ G2y[CNum] * TgtImg[NIdx].y
								+ G2z[CNum] * TgtImg[NIdx].z
								+ DMap[NIdx];

							if (CurVal < DMap[idx]) {
								DMap[idx] = CurVal;
								TgtImg[idx].x = TgtImg[NIdx].x + Gx[CNum];
								TgtImg[idx].y = TgtImg[NIdx].y + Gy[CNum];
								TgtImg[idx].z = TgtImg[NIdx].z + Gz[CNum];
							}
						}
					}
				}
			BarJump2:;
			}
		}
	}

	return DMap;
}
vector<short> DoPAR::GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN) {
	////(0) Sz = 1: for 2D image
	////(1) OImg: <= 0 for solid, > 0 for pores
	////(2) DM_Type = 0: for solid phase,
	////    DM_Type = 1: for pore phase,
	////    DM_Type = 2: for both solid and pore phase,
	////(3) DisValYN: Reture type - distance value if DisValYN = ture, otherwise return sequence number

	//(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false)

	if (DM_Type != 0 && DM_Type != 1) DM_Type = 2;

	vector<short> DMap;

	if (DM_Type == 1 || DM_Type == 2) {
		vector<unsigned short> tDMap;

		tDMap = BarDMap(Sx, Sy, Sz, OImg);

		DMap.resize(tDMap.size(), 0);

		unsigned short MaxDis(1);
		for (long idx = 0; idx < tDMap.size(); ++idx) {
			if (tDMap[idx] < 1) continue;
			if (tDMap[idx] > 32760L)
				DMap[idx] = 32761L;
			else
				DMap[idx] = tDMap[idx];

			if (DMap[idx] > MaxDis)
				MaxDis = DMap[idx];
		}

		tDMap.clear();

		if (!DisValYN) {
			vector<bool> UsedYN(MaxDis + 1, false);
			for (long idx = 0; idx < DMap.size(); ++idx) {
				if (DMap[idx] > 0) {
					UsedYN[DMap[idx]] = true;
				}
			}

			vector<long> DisSeq(MaxDis + 1, -1);

			long ID(0);
			for (long ij = 1; ij < UsedYN.size(); ++ij) {
				if (UsedYN[ij])
					DisSeq[ij] = ++ID;
			}

			for (long idx = 0; idx < DMap.size(); ++idx) {
				if (DMap[idx] > 0) {
					DMap[idx] = DisSeq[DMap[idx]];
				}
			}
		}
	}//DM_Type == 1 || DM_Type == 2

	if (DM_Type == 0 || DM_Type == 2) {
		vector<char> OD;

		OD.resize(OImg.size(), 0);
		for (long idx = 0; idx < OImg.size(); ++idx) {
			if (OImg[idx] <= 0) {
				OD[idx] = 1;
			}
		}

		vector<unsigned short> tDMap;

		tDMap = BarDMap(Sx, Sy, Sz, OD);

		if (DMap.size() == 0) {
			DMap.resize(tDMap.size(), 0);
		}

		unsigned short MaxDis(1);
		for (long idx = 0; idx < tDMap.size(); ++idx) {
			if (tDMap[idx] < 1) continue;
			if (tDMap[idx] > 32760L)
				DMap[idx] = -32761L;
			else
				DMap[idx] = -1L * tDMap[idx];

			if (-DMap[idx] > MaxDis)
				MaxDis = -DMap[idx];
		}

		tDMap.clear();

		if (!DisValYN) {
			vector<bool> UsedYN(MaxDis + 1, false);

			for (long idx = 0; idx < DMap.size(); ++idx) {
				if (DMap[idx] < 0) {
					UsedYN[-DMap[idx]] = true;
				}
			}

			vector<long> DisSeq(MaxDis + 1, -1);

			long ID(0);
			for (long ij = 1; ij < UsedYN.size(); ++ij) {
				if (UsedYN[ij])
					DisSeq[ij] = ++ID;
			}

			for (long idx = 0; idx < DMap.size(); ++idx) {
				if (DMap[idx] < 0) {
					DMap[idx] = -DisSeq[-DMap[idx]];
				}
			}
		}
	}//DM_Type == 1 || DM_Type == 2

	return DMap;
}

void DoPAR::transformDM(int level, vector<size_color>& exemplar1, vector<size_color>& exemplar2, vector<size_color>& exemplar3) {
	// redistribute TI based on DM, no need to resize to 0-255
	// first transform to DMap, then linear project (just make -s and +p to positive values)
	const short TEXSIZE_ = sqrt(exemplar1.size());
	if (exemplar1.size() != exemplar2.size() || exemplar1.size() != exemplar3.size()) { cout << endl << "exemplars size different!"; getch(); exit(0); }

	vector<short> DMap_x(exemplar1.begin(), exemplar1.end());
	vector<short> DMap_y(exemplar2.begin(), exemplar2.end());
	vector<short> DMap_z(exemplar3.begin(), exemplar3.end());
	vector<char> tempchar(exemplar1.size());
	
	binaryChar(DMap_x, tempchar);
	DMap_x = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, true);
	binaryChar(DMap_y, tempchar);
	DMap_y = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, true);
	binaryChar(DMap_z, tempchar);
	DMap_z = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, true);
	//binaryChar(DMap_x, tempchar);
	//DMap_x = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);
	//binaryChar(DMap_y, tempchar);
	//DMap_y = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);
	//binaryChar(DMap_z, tempchar);
	//DMap_z = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);

	for (long idx = 0; idx < DMap_x.size(); ++idx) {	//get the true distance
		DMap_x[idx] = (DMap_x[idx] / abs(DMap_x[idx])) * round(sqrt(abs(DMap_x[idx])));
		DMap_y[idx] = (DMap_y[idx] / abs(DMap_y[idx])) * round(sqrt(abs(DMap_y[idx])));
		DMap_z[idx] = (DMap_z[idx] / abs(DMap_z[idx])) * round(sqrt(abs(DMap_z[idx])));
	}

	// prepare, get maxDis & minDis for TIs
	short minVal, maxVal, minVal1, minVal2, minVal3, maxVal1, maxVal2, maxVal3;	//total min, max for 3TIs; and separately
	minVal = maxVal = DMap_x[0];	
	minVal1 = maxVal1 = DMap_x[0]; minVal2 = maxVal2 = DMap_y[0]; minVal3 = maxVal3 = DMap_z[0];
	for (long idx = 0; idx < DMap_x.size(); ++idx) {
		if (DMap_x[idx] == 0) { cout << endl << "DMap_x[" << idx << "]= 0!!"; _getch(); }
		if (DMap_x[idx] < minVal1) minVal1 = DMap_x[idx];
		if (DMap_x[idx] > maxVal1) maxVal1 = DMap_x[idx];
		
		if (DMap_y[idx] < minVal2) minVal2 = DMap_y[idx];
		if (DMap_y[idx] > maxVal2) maxVal2 = DMap_y[idx];

		if (DMap_z[idx] < minVal3) minVal3 = DMap_z[idx];
		if (DMap_z[idx] > maxVal3) maxVal3 = DMap_z[idx];
	}
	
	//if (!HisEqYN) {
		minVal = max(minVal1, max(minVal2, minVal3));
		maxVal = min(maxVal1, min(maxVal2, maxVal3));
	//}
	//else {
	//	minVal = min(minVal1, min(minVal2, minVal3));
	//	maxVal = max(maxVal1, max(maxVal2, maxVal3));
	//}
	

	// transform to exemplar			// no need to resize to 0-255!   min -> 0, +1 -> -min
	Solid_Upper[level] =  - minVal;
	Pore_Upper[level] = maxVal - minVal;	//total bins
	Pore_Lower[level] = Solid_Upper[level] + 1;

	for (long i = 0; i < DMap_x.size(); i++) {
		if (DMap_x[i] < 0) DMap_x[i] = max(0, DMap_x[i] - minVal) +1;
		else DMap_x[i] = min(DMap_x[i] - minVal, maxVal - minVal) ;

		if (DMap_y[i] < 0) DMap_y[i] = max(0, DMap_y[i] - minVal) +1;
		else DMap_y[i] = min(DMap_y[i] - minVal , maxVal - minVal);

		if (DMap_z[i] < 0) DMap_z[i] = max(0, DMap_z[i] - minVal) +1;
		else DMap_z[i] = min(DMap_z[i] - minVal , maxVal - minVal) ;




	}

	//convert from vector<short> to vector<float>
	exemplar1 = vector<size_color>(DMap_x.begin(), DMap_x.end());
	exemplar2 = vector<size_color>(DMap_y.begin(), DMap_y.end());
	exemplar3 = vector<size_color>(DMap_z.begin(), DMap_z.end());
}

void DoPAR::transformDM(int level, vector<short>& exemplar1, vector<short>& exemplar2, vector<short>& exemplar3) {
	// redistribute TI based on DM, no need to resize to 0-255
	// first transform to DMap, then linear project (just make -s and +p to positive values)
	const short TEXSIZE_ = sqrt(exemplar1.size());

	if (exemplar1.size() != exemplar2.size() || exemplar1.size() != exemplar3.size()) { cout << endl << "exemplars size different!"; getch(); exit(0); }

	vector<short> DMap_x(exemplar1.begin(), exemplar1.end());
	vector<short> DMap_y(exemplar2.begin(), exemplar2.end());
	vector<short> DMap_z(exemplar3.begin(), exemplar3.end());
	vector<char> tempchar(exemplar1.size());

	binaryChar(DMap_x, tempchar);
	DMap_x = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, true);
	binaryChar(DMap_y, tempchar);
	DMap_y = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, true);
	binaryChar(DMap_z, tempchar);
	DMap_z = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, true);
	//binaryChar(DMap_x, tempchar);
	//DMap_x = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);
	//binaryChar(DMap_y, tempchar);
	//DMap_y = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);
	//binaryChar(DMap_z, tempchar);
	//DMap_z = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);

	for (long idx = 0; idx < DMap_x.size(); ++idx) {	//get the true distance
		DMap_x[idx] = (DMap_x[idx] / abs(DMap_x[idx])) * round(sqrt(abs(DMap_x[idx])));
		DMap_y[idx] = (DMap_y[idx] / abs(DMap_y[idx])) * round(sqrt(abs(DMap_y[idx])));
		DMap_z[idx] = (DMap_z[idx] / abs(DMap_z[idx])) * round(sqrt(abs(DMap_z[idx])));
	}

	// prepare, get maxDis & minDis for TIs
	short minVal, maxVal, minVal1, minVal2, minVal3, maxVal1, maxVal2, maxVal3;	//total min, max for 3TIs; and separately
	minVal = maxVal = DMap_x[0];
	minVal1 = maxVal1 = DMap_x[0]; minVal2 = maxVal2 = DMap_y[0]; minVal3 = maxVal3 = DMap_z[0];
	for (long idx = 0; idx < DMap_x.size(); ++idx) {
		if (DMap_x[idx] == 0) { cout << endl << "DMap_x[" << idx << "]= 0!!"; _getch(); }
		if (DMap_x[idx] < minVal1) minVal1 = DMap_x[idx];
		if (DMap_x[idx] > maxVal1) maxVal1 = DMap_x[idx];

		if (DMap_y[idx] < minVal2) minVal2 = DMap_y[idx];
		if (DMap_y[idx] > maxVal2) maxVal2 = DMap_y[idx];

		if (DMap_z[idx] < minVal3) minVal3 = DMap_z[idx];
		if (DMap_z[idx] > maxVal3) maxVal3 = DMap_z[idx];
	}

	//if (!HisEqYN) {
	minVal = max(minVal1, max(minVal2, minVal3));
	maxVal = min(maxVal1, min(maxVal2, maxVal3));
	//}
	//else {
	//	minVal = min(minVal1, min(minVal2, minVal3));
	//	maxVal = max(maxVal1, max(maxVal2, maxVal3));
	//}


	// transform to exemplar			// no need to resize to 0-255!   min -> 0, +1 -> -min
	Solid_Upper[level] = -minVal;
	Pore_Upper[level] = maxVal - minVal;	//total bins
	Pore_Lower[level] = Solid_Upper[level] + 1;

	for (long i = 0; i < DMap_x.size(); i++) {
		if (DMap_x[i] < 0) DMap_x[i] = max(0, DMap_x[i] - minVal) + 1;
		else DMap_x[i] = min(DMap_x[i] - minVal, maxVal - minVal);

		if (DMap_y[i] < 0) DMap_y[i] = max(0, DMap_y[i] - minVal) + 1;
		else DMap_y[i] = min(DMap_y[i] - minVal, maxVal - minVal);

		if (DMap_z[i] < 0) DMap_z[i] = max(0, DMap_z[i] - minVal) + 1;
		else DMap_z[i] = min(DMap_z[i] - minVal, maxVal - minVal);




	}

	//convert from vector<short> to vector<float>
	exemplar1 = vector<short>(DMap_x.begin(), DMap_x.end());
	exemplar2 = vector<short>(DMap_y.begin(), DMap_y.end());
	exemplar3 = vector<short>(DMap_z.begin(), DMap_z.end());
}


// =========== K-coherence search =============
void DoPAR::computeKCoherence(){
	cout << endl << "K="<< COHERENCENUM <<" compute K-coherence...";
	unsigned long time_start = clock();

	KCoherence_x.resize(MULTIRES);
	KCoherence_y.resize(MULTIRES);
	KCoherence_z.resize(MULTIRES);

	if (SIM2D_YN) {
		for (int level = 0; level < MULTIRES; ++level) {
			size_idx TEXSIZE_ = TIsize[level];
			size_idx blockSize_ = blockSize[level];

			size_idx width = TEXSIZE_ - blockSize_ + 1;
			size_idx height = TEXSIZE_ - blockSize_ + 1;
			size_idx maxSize2d = TEXSIZE_ * height;
			size_idx dim = blockSize_ * blockSize_;
			size_idx bias = blockSize_ * 0.5;
			size_idx numData = width * height;

			cout << endl << "level: " << level << " Dimension=" << dim;

			KCoherence_x[level].resize(TEXSIZE_ * TEXSIZE_);
			ANNkd_tree*  kdTree_x;
			ANNpointArray p_source_x;
			p_source_x = annAllocPts(numData, dim);			//rows='area' numData, cols=dimension (Neighbour size)

			size_idx row = 0;
			for (size_idx i = 0; i < width; ++i) {
				for (size_idx j = 0; j < height; ++j) {
					size_idx col = 0;
					size_idx index0 = TEXSIZE_ * i + j;
					for (size_idx m = 0; m <blockSize_; ++m) {
						for (size_idx n = 0; n <blockSize_; ++n) {
							size_idx index = index0 + m * TEXSIZE_ + n;		//[i+m][j+n]
							p_source_x[row][col] = m_exemplar_x[level][index];

							++col;
						}
					}
					++row;
				}
			}
			//ANNpoint* data point array = m_neighbor_kdTree_ptr_x, number of points = numData, dimension
			kdTree_x = new ANNkd_tree(p_source_x, numData, dim);		//build ANNkd_tree

#pragma omp parallel for schedule(static)
			for (size_idx idx = 0; idx < maxSize2d; idx++) {
				if (idx%TEXSIZE_ >= width) continue;
				ANNpoint queryPt_x;
				queryPt_x = annAllocPt(dim);
				ANNidxArray ann_index_x = new size_idx[COHERENCENUM];
				ANNdistArray ann_dist_x = new size_dist[COHERENCENUM];

				int num = 0;
				for (size_idx m = 0; m < blockSize_; ++m) {
					for (size_idx n = 0; n < blockSize_; ++n) {
						size_idx index = idx + TEXSIZE_ * m + n;	//[i+m][j+n]
						queryPt_x[num] = m_exemplar_x[level][index];
						num++;
					}
				}

				kdTree_x->annkSearch(queryPt_x, COHERENCENUM, ann_index_x, ann_dist_x, ANNerror[level]);

				//Set K-Coherence
				size_idx bias_TIindex = idx + bias*TEXSIZE_ + bias;
				KCoherence_x[level][bias_TIindex].resize(COHERENCENUM);
				for (int k = 0; k < COHERENCENUM; ++k) {
					KCoherence_x[level][bias_TIindex][k] = convertIndexANN(level, ann_index_x[k]);
				}

				annDeallocPt(queryPt_x);		
				delete[] ann_index_x;		
				delete[] ann_dist_x;		
			}//#pragma omp parallel for schedule(static)

			 //release
			annClose();
			delete kdTree_x;				
			annDeallocPts(p_source_x);		

		}
	}
	else {
		for (int level = 0; level < MULTIRES; ++level) {
			size_idx TEXSIZE_ = TIsize[level];
			size_idx blockSize_ = blockSize[level];

			size_idx width = TEXSIZE_ - blockSize_ + 1;
			size_idx height = TEXSIZE_ - blockSize_ + 1;
			size_idx maxSize2d = TEXSIZE_ * height;
			size_idx dim = blockSize_ * blockSize_;
			size_idx bias = blockSize_ * 0.5;
			size_idx numData = width * height;

			cout << endl << "level: " << level << " Dimension=" << dim;

			KCoherence_x[level].resize(TEXSIZE_ * TEXSIZE_);
			KCoherence_y[level].resize(TEXSIZE_ * TEXSIZE_);
			KCoherence_z[level].resize(TEXSIZE_ * TEXSIZE_);
			ANNkd_tree*  kdTree_x;
			ANNkd_tree*  kdTree_y;
			ANNkd_tree*  kdTree_z;
			ANNpointArray p_source_x, p_source_y, p_source_z;
			p_source_x = annAllocPts(numData, dim);			//rows='area' numData, cols=dimension (Neighbour size)
			p_source_y = annAllocPts(numData, dim);
			p_source_z = annAllocPts(numData, dim);

			size_idx row = 0;
			for (size_idx i = 0; i < width; ++i) {
				for (size_idx j = 0; j < height; ++j) {
					size_idx col = 0;
					size_idx index0 = TEXSIZE_ * i + j;
					for (size_idx m = 0; m <blockSize_; ++m) {
						for (size_idx n = 0; n <blockSize_; ++n) {
							size_idx index = index0 + m * TEXSIZE_ + n;		//[i+m][j+n]
							p_source_x[row][col] = m_exemplar_x[level][index];
							p_source_y[row][col] = m_exemplar_y[level][index];
							p_source_z[row][col] = m_exemplar_z[level][index];

							++col;
						}
					}
					++row;
				}
			}
			//ANNpoint* data point array = m_neighbor_kdTree_ptr_x, number of points = numData, dimension
			kdTree_x = new ANNkd_tree(p_source_x, numData, dim);		//build ANNkd_tree
			kdTree_y = new ANNkd_tree(p_source_y, numData, dim);
			kdTree_z = new ANNkd_tree(p_source_z, numData, dim);

#pragma omp parallel for schedule(static)
			for (size_idx idx = 0; idx < maxSize2d; idx++) {
				if (idx%TEXSIZE_ >= width) continue;
				ANNpoint queryPt_x, queryPt_y, queryPt_z;
				queryPt_x = annAllocPt(dim);
				queryPt_y = annAllocPt(dim);
				queryPt_z = annAllocPt(dim);
				ANNidxArray ann_index_x = new size_idx[COHERENCENUM];
				ANNidxArray ann_index_y = new size_idx[COHERENCENUM];
				ANNidxArray ann_index_z = new size_idx[COHERENCENUM];
				ANNdistArray ann_dist_x = new size_dist[COHERENCENUM];
				ANNdistArray ann_dist_y = new size_dist[COHERENCENUM];
				ANNdistArray ann_dist_z = new size_dist[COHERENCENUM];

				int num = 0;
				for (size_idx m = 0; m < blockSize_; ++m) {
					for (size_idx n = 0; n < blockSize_; ++n) {
						size_idx index = idx + TEXSIZE_ * m + n;	//[i+m][j+n]
						queryPt_x[num] = m_exemplar_x[level][index];
						queryPt_y[num] = m_exemplar_y[level][index];
						queryPt_z[num] = m_exemplar_z[level][index];
						num++;
					}
				}

				kdTree_x->annkSearch(queryPt_x, COHERENCENUM, ann_index_x, ann_dist_x, ANNerror[level]);
				kdTree_y->annkSearch(queryPt_y, COHERENCENUM, ann_index_y, ann_dist_y, ANNerror[level]);
				kdTree_z->annkSearch(queryPt_z, COHERENCENUM, ann_index_z, ann_dist_z, ANNerror[level]);

				//Set K-Coherence biased means shifted
				size_idx bias_TIindex = idx + bias*TEXSIZE_ + bias;
				KCoherence_x[level][bias_TIindex].resize(COHERENCENUM);
				KCoherence_y[level][bias_TIindex].resize(COHERENCENUM);
				KCoherence_z[level][bias_TIindex].resize(COHERENCENUM);
				for (int k = 0; k < COHERENCENUM; ++k) {
					KCoherence_x[level][bias_TIindex][k] = convertIndexANN(level, ann_index_x[k]);
					KCoherence_y[level][bias_TIindex][k] = convertIndexANN(level, ann_index_y[k]);
					KCoherence_z[level][bias_TIindex][k] = convertIndexANN(level, ann_index_z[k]);
				}

				annDeallocPt(queryPt_x);		annDeallocPt(queryPt_y);		annDeallocPt(queryPt_z);
				delete[] ann_index_x;		delete[] ann_index_y;		delete[] ann_index_z;
				delete[] ann_dist_x;		delete[] ann_dist_y;		delete[] ann_dist_z;
			}//#pragma omp parallel for schedule(static)

			 //release
			annClose();
			delete kdTree_x;				delete kdTree_y;				delete kdTree_z;
			annDeallocPts(p_source_x);		annDeallocPts(p_source_y);		annDeallocPts(p_source_z);


			//bool checkYN = true;
			////for (int level = 0; level < MULTIRES; level++) {
			//int r = static_cast<size_idx>(blockSize[level] * 0.5);
			//int tisize = TIsize[level];
			//for (int i = 0; i < TEXSIZE_ * TEXSIZE_; i++) {
			//	if (!checkYN) break;
			//	if (i / tisize < r || i / tisize > tisize - r  || i%tisize <r || i%tisize > tisize - r ) continue;
			//	//printf("%d ",i);
			//	for (int n = 0; n < COHERENCENUM; n++) {
			//		int idx2d_x = KCoherence_x[level][i][n];					
			//		int x_x = idx2d_x / tisize;
			//		int x_y = idx2d_x % tisize;
			//		if (x_x - r< 0 || x_x + r>tisize  || x_y - r<0 || x_y + r>tisize ) checkYN = false;
			//		int idx2d_y = KCoherence_y[level][i][n];
			//		int y_x = idx2d_y / tisize;
			//		int y_y = idx2d_y % tisize;
			//		if (y_x - r< 0 || y_x + r>tisize  || y_y - r<0 || y_y + r>tisize ) checkYN = false;
			//		int idx2d_z = KCoherence_z[level][i][n];
			//		int z_x = idx2d_z / tisize;
			//		int z_y = idx2d_z % tisize;
			//		if (z_x - r< 0 || z_x + r>tisize || z_y - r<0 || z_y + r>tisize ) checkYN = false;
			//	}
			//}
			////}
			//if (!checkYN) printf("KCoherence exceed");
			////else printf("KCoherence right");
		}
	}
	

	long time_end = clock();
	cout << endl << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC << " s";
}

// ================ phase 1: search (M-step)===========================
bool DoPAR::searchVolume(int level) {
	size_dist PreviousDis;
	if (!FIRSTRUN) PreviousDis = TotalDis;
	else TotalDis = 0;

	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx Size = OUTsize_*OUTsize_*OUTsize_;


	size_idx start = static_cast<size_idx>(blockSize_ * 0.5);			//5	//4	//4	//3			//-cstart<=x<=cend
	size_idx end = static_cast<size_idx>((blockSize_-1) * 0.5);			//4	//3	//3	//2
	size_idx cstart(start), cend(end);
	bool isUnchanged = true;	

	//AssignFixedLayer(level, FixedLayerDir);

#pragma omp parallel 
	{
	if (!SIM2D_YN) {
		//For Z
#pragma omp for nowait schedule(static)
		for (size_idx i2 = 0; i2 < Size; ++i2) {
			size_idx idx = m_permutation[i2];			//[i][j][k] idx=i*Syz+j*Sz+k
			size_idx k = idx % OUTsize_;
			size_idx j = (idx / OUTsize_) % OUTsize_;
			size_idx i = idx / OUTsize2d_;
			if (i % GRID != 0 || j % GRID != 0)	continue;						//sparse grid			
			if (isUnchangedBlock(level, 2, i, j, k)) continue;			//check neighbours all unchanged or not

			CvMat* current_neighbor = cvCreateMat(1, blockSize_*blockSize_, CV_32F);
			size_idx index = 0;	
			for (size_idx du = -start; du <= end; ++du) {	//N is neighbourhood size.
				bool trimYN = false;
				size_idx VCurIdx1 = OUTsize2d_ * trimIndex(level, i + du) + k;
				//if (i + du<0 || i + du>= TEXSIZE_) trimYN = true;
				for (size_idx dv = -start; dv <= end; ++dv) {				
					size_idx index2 = VCurIdx1 + OUTsize_ * trimIndex(level, j + dv);			//[i+du][j+dv][k]
					
					//if (j + dv<0 || j + dv>= TEXSIZE_) trimYN = true;

					if (trimYN) cvmSet(current_neighbor, 0, index, -1);
					else cvmSet(current_neighbor, 0, index, m_volume[level][index2]);

					index++;
				}
			}

			int countz(1);
			size_dist besthis(0), curhis(0);											//deal with equal Error, first compare indexhis
			int curbin(0), bestbin(0);													//then compare colorhis

			vector<size_idx> compareIdx;
			size_dist curDis, curError, IndexHisWeight;
			size_dist tempHisDiff;
			bool countedYN = false;
			size_dist minError = INFINITY, minDis = INFINITY;
			size_idx bestTIIdx(0);
			size_idx eposx, eposy;
			size_idx temp2didx, temp3didx, tempTIidx;
			size_idx sumidx_posx;
			int compareNum = 0;
			compareIdx.clear();
			compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
			for (size_idx u = -cstart; u <= cend; ++u) {
				sumidx_posx = trimIndex(level, i + u)*OUTsize2d_ + k;
				for (size_idx v = -cstart; v <= cend; ++v) {
					temp3didx = sumidx_posx + trimIndex(level, j + v)*OUTsize_;				//[posx][posy][k]    [i+u][j+v]
					eposx = (Origin_z[level][temp3didx] / TIsize_) - u;
					eposy = (Origin_z[level][temp3didx] % TIsize_) - v;

					if (!(eposx >= start && eposx < TIsize_ - end && eposy >= start && eposy < TIsize_ - end))
						continue;


					tempTIidx = eposx*TIsize_ + eposy;										//origin - (u,v)
					for (int l = 0; l < COHERENCENUM; ++l) {							//[0,COHERENCENUM]				
						temp2didx = KCoherence_z[level][tempTIidx][l];

						int p = 0;
						for (; p < compareNum; ++p) {
							if (compareIdx[p] == temp2didx)	break;
						}
						if (p < compareNum)	continue;

						curDis = getFullDistance(level, m_exemplar_z[level], temp2didx, current_neighbor);

						//IndexHis needs sparse grid
						curhis = IndexHis_z[level][sparseIdx(level, temp2didx)];
						tempHisDiff = max(0.0f, 1.0f*(curhis - avgIndexHis[level]));
						IndexHisWeight = 1.0f + factorIndex * tempHisDiff;
						curError = IndexHisWeight * curDis;
						countedYN = true;
						if (tempHisDiff > IndexHisManualControl*avgIndexHis[level]) curError = FLT_MAX;	//!manually control indexhis



						curbin = (int)m_exemplar_z[level][temp2didx];
						if (curError < minError) {								//min error			
							minError = curError;
							minDis = curDis;
							bestTIIdx = temp2didx;
							besthis = curhis;
							bestbin = curbin;
						}
						else if (curError - minError < 10e-9) {	// if Error same, first compare IndexHis
							if (curhis < besthis) {
								minDis = curDis;
								bestTIIdx = temp2didx;
								besthis = curhis;
								bestbin = curbin;
							}
							else if (curhis == besthis && !FIRSTRUN) {
								if (PosHis[level][temp2didx + 2 * TIsize2d_] < PosHis[level][bestTIIdx + 2 * TIsize2d_])
									bestTIIdx = temp2didx;
								// if IndexHis same and color not the same, compare ColorHis
								//else if (ColorHis_ON)
								//	if (ColorHis_synthesis[level][curbin] - ColorHis_exemplar[level][curbin] < ColorHis_synthesis[level][bestbin] - ColorHis_exemplar[level][bestbin]) {
								//		bestTIIdx = temp2didx;
								//		bestbin = curbin;
								//	}
							}
						}
						compareNum++;
						compareIdx.push_back(temp2didx);
					}//for (int l = 0; l < COHERENCENUM; ++l){	
				}//for (int v = -start; v <= end; ++v){
			}//for (int u = -start; u <= end; ++u){
			if (fabs(FLT_MAX - minError) > 1.0f) {
				nearestWeight_z[level][idx] = 1.0f / minDis;
				size_idx formerNearestIdx = nearestIdx_z[level][idx];
				if (formerNearestIdx != bestTIIdx) {
					nearestIdx_z[level][idx] = bestTIIdx;											//update nearestIdx
					if (formerNearestIdx < TIsize2d_ && formerNearestIdx >= 0) {
						size_hiscount& addressFormerNearestIdx = IndexHis_z[level][sparseIdx(level, formerNearestIdx)];
						if (addressFormerNearestIdx>0)
#pragma omp atomic
						addressFormerNearestIdx--;
					}
					size_hiscount& addressbestTIIdx = IndexHis_z[level][sparseIdx(level, bestTIIdx)];
#pragma omp atomic
					addressbestTIIdx++;							//update IndexHis sparse grid 	
					isUnchanged = false;
				}
			}
			else {
				bestTIIdx = getRandomNearestIndex(level, IndexHis_z[level]);
				nearestWeight_z[level][idx] = 1.0f * 0.01;
				size_idx formerNearestIdx = nearestIdx_z[level][idx];
				if (formerNearestIdx != bestTIIdx) {
					nearestIdx_z[level][idx] = bestTIIdx;											//update nearestIdx				
					if (formerNearestIdx < TIsize2d_ && formerNearestIdx >= 0) {
						size_hiscount& addressFormerNearestIdx = IndexHis_z[level][sparseIdx(level, formerNearestIdx)];
						if (addressFormerNearestIdx>0)
#pragma omp atomic
						addressFormerNearestIdx--;
					}
					size_hiscount& addressbestTIIdx = IndexHis_z[level][sparseIdx(level, bestTIIdx)];
#pragma omp atomic
					addressbestTIIdx++;							//update IndexHis sparse grid 	
					isUnchanged = false;
				}
			}
			cvReleaseMat(&current_neighbor);
		}//for (size_idx i2 = 0; i2 < Size; ++i2) {	//Z
		//#pragma omp for nowait schedule(static)


		//For Y	
#pragma omp for nowait schedule(static)
			for (size_idx i2 = 0; i2 < Size; ++i2) {
				size_idx idx = m_permutation[i2];			//[i][j][k] idx=i*Syz+j*Sz+k
				size_idx k = idx % OUTsize_;
				size_idx j = (idx / OUTsize_) % OUTsize_;
				size_idx i = idx / OUTsize2d_;
				if (i % GRID != 0 || k % GRID != 0)	continue;						//sparse grid			
				if (isUnchangedBlock(level, 1, i, j, k)) continue;			//check neighbours all unchanged or not

				CvMat* current_neighbor = cvCreateMat(1, blockSize_*blockSize_, CV_32F);
				size_idx index = 0;
				size_idx jSz = j*OUTsize_;
				
				for (size_idx du = -start; du <= end; ++du) {	//N is neighbourhood size.
					bool trimYN = false;
					
					size_idx VCurIdx2 = OUTsize2d_ * trimIndex(level, i + du) + jSz;
					//if (i + du<0 || i + du >= TEXSIZE_) trimYN = true;
					for (size_idx dv = -start; dv <= end; ++dv) {
						size_idx index2 = VCurIdx2 + trimIndex(level, k + dv);		//[i+du][j][k+dv]

						//if (k + dv<0 || k + dv>= TEXSIZE_) trimYN = true;
						
						if (trimYN) cvmSet(current_neighbor, 0, index, -1);
						else cvmSet(current_neighbor, 0, index, m_volume[level][index2]);
						index++;
					}
				}

				size_dist besthis(0), curhis(0);											//deal with equal Error, first compare indexhis
				int curbin(0), bestbin(0);													//then compare colorhis

				vector<size_idx> compareIdx;
				size_dist curDis, curError, IndexHisWeight;
				size_dist tempHisDiff;
				bool countedYN = false;
				size_dist minError = INFINITY, minDis = INFINITY;
				size_idx bestTIIdx;
				size_idx eposx, eposy;
				size_idx temp2didx, temp3didx, tempTIidx;
				size_idx sumidx_posx;
				int compareNum = 0;
				compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
				for (size_idx u = -cstart; u <= cend; ++u) {
					sumidx_posx = trimIndex(level, i + u)*OUTsize2d_ + jSz;
					for (size_idx v = -cstart; v <= cend; ++v) {
						temp3didx = sumidx_posx + trimIndex(level, k + v);			//[posx][j][posy]
						eposx = (Origin_y[level][temp3didx] / TIsize_) - u;
						eposy = (Origin_y[level][temp3didx] % TIsize_) - v;

						if (!(eposx >= start && eposx < TIsize_ - end && eposy >= start && eposy < TIsize_ - end))
							continue;
						//if (!(eposx >= 0 && eposx < Sx && eposy >= 0 && eposy < Sy))
						//	continue;

						tempTIidx = eposx*TIsize_ + eposy;
						for (int l = 0; l < COHERENCENUM; ++l) {					//[0,COHERENCENUM]				
							temp2didx = KCoherence_y[level][tempTIidx][l];
							int p = 0;
							for (; p < compareNum; ++p) {
								if (compareIdx[p] == temp2didx)	break;
							}
							if (p < compareNum)	continue;

							curDis = getFullDistance(level, m_exemplar_y[level], temp2didx, current_neighbor);

							curhis = IndexHis_y[level][sparseIdx(level, temp2didx)];
							tempHisDiff = max(0.0f, 1.0f*(curhis - avgIndexHis[level]));
							IndexHisWeight = 1.0f + factorIndex * tempHisDiff;
							curError = IndexHisWeight * curDis;
							countedYN = true;
							if (tempHisDiff > IndexHisManualControl*avgIndexHis[level]) curError = FLT_MAX;	//!manually control indexhis

							curbin = (int)m_exemplar_y[level][temp2didx];
							if (minError > curError) {								//min error			
								minError = curError;
								minDis = curDis;
								bestTIIdx = temp2didx;
								besthis = curhis;
								bestbin = curbin;
							}
							else if (curError - minError < 1e-8) {
								if (curhis < besthis) {
									minDis = curDis;
									bestTIIdx = temp2didx;
									besthis = curhis;
									bestbin = curbin;
								}
								else if (curhis == besthis && !FIRSTRUN) {
									if (PosHis[level][temp2didx + TIsize2d_] < PosHis[level][bestTIIdx + TIsize2d_])
										bestTIIdx = temp2didx;
									// if IndexHis same and color not the same, compare ColorHis
									//else if (ColorHis_ON)
									//	if (ColorHis_synthesis[level][curbin] - ColorHis_exemplar[level][curbin] < ColorHis_synthesis[level][bestbin] - ColorHis_exemplar[level][bestbin]) {
									//		bestTIIdx = temp2didx;
									//		bestbin = curbin;
									//	}
								}
							}
							compareNum++;
							compareIdx.push_back(temp2didx);
						}//for (int l = 0; l < COHERENCENUM; ++l){	
					}//for (int v = -start; v <= end; ++v){
				}//for (int u = -start; u <= end; ++u){

				if (fabs(FLT_MAX - minError) > 1.0f) {
					size_idx formerNearestIdx = nearestIdx_y[level][idx];
					nearestWeight_y[level][idx] = 1.0f / minDis;
					if (formerNearestIdx != bestTIIdx) {
						nearestIdx_y[level][idx] = bestTIIdx;											//update nearestIdx
						if (formerNearestIdx < TIsize2d_ && formerNearestIdx >= 0) {
							size_hiscount& addressFormerNearestIdx = IndexHis_y[level][sparseIdx(level, formerNearestIdx)];
							if (addressFormerNearestIdx>0)
#pragma omp atomic
							addressFormerNearestIdx--;
						}
						size_hiscount& addressbestTIIdx = IndexHis_y[level][sparseIdx(level, bestTIIdx)];
#pragma omp atomic
						addressbestTIIdx++;							//update IndexHis sparse grid 	
						isUnchanged = false;
					}
				}
				else {
					//if (!countedYN) cout << "bad ";
					//else cout << "infinity.";
					bestTIIdx = getRandomNearestIndex(level, IndexHis_y[level]);
					size_idx formerNearestIdx = nearestIdx_y[level][idx];
					nearestWeight_y[level][idx] = 1.0f * 0.01;
					if (formerNearestIdx != bestTIIdx) {
						nearestIdx_y[level][idx] = bestTIIdx;											//update nearestIdx
						if (formerNearestIdx < TIsize2d_ && formerNearestIdx >= 0) {
							size_hiscount& addressFormerNearestIdx = IndexHis_y[level][sparseIdx(level, formerNearestIdx)];
							if (addressFormerNearestIdx>0)
#pragma omp atomic
							addressFormerNearestIdx--;
						}
						size_hiscount& addressbestTIIdx = IndexHis_y[level][sparseIdx(level, bestTIIdx)];
#pragma omp atomic
						addressbestTIIdx++;							//update IndexHis sparse grid 	
						isUnchanged = false;
					}
				}
				cvReleaseMat(&current_neighbor);
			}//for (size_idx i2 = 0; i2 < Size; ++i2) {	//Y
		//#pragma omp for nowait schedule(static)
		}//if(!SIM2D_YN)
		
		//For X		
#pragma omp for schedule(static)
		for (size_idx i2 = 0; i2 < Size; ++i2) {
			size_idx idx = m_permutation[i2];			//[i][j][k] idx=i*Syz+j*Sz+k
			size_idx k = idx % OUTsize_;
			size_idx j = (idx / OUTsize_) % OUTsize_;
			size_idx i = idx / OUTsize2d_;
			if (j % GRID != 0 || k % GRID != 0)	continue;					//sparse grid			
			if (isUnchangedBlock(level, 0, i, j, k)) continue;			//check neighbours all unchanged or not

			CvMat* current_neighbor = cvCreateMat(1, blockSize_*blockSize_, CV_32F);
			size_idx index = 0;
			size_idx iSyz = OUTsize2d_ * i;
			for (size_idx du = -start; du <= end; ++du) {	//N is neighbourhood size.
				bool trimYN = false;
				size_idx VCurIdx3 = iSyz + OUTsize_ * trimIndex(level, j + du);
				//if (j + du<0 || j + du>= TEXSIZE_) trimYN = true;
				for (size_idx dv = -start; dv <= end; ++dv) {
					size_idx index2 = VCurIdx3 + trimIndex(level, k + dv);								//[i][j+du][k+dv]

					//if (k + dv<0 || k + dv>= TEXSIZE_) trimYN = true;

					if (trimYN) cvmSet(current_neighbor, 0, index, -1);
					else cvmSet(current_neighbor, 0, index, m_volume[level][index2]);						//set current_neighbor(0,col) to m_volume(idx)
					index++;
				}
			}

			size_dist besthis(0), curhis(0);											//deal with equal Error, first compare indexhis
			int curbin(0), bestbin(0);													//then compare colorhis

			vector<size_idx> compareIdx;
			size_dist curDis, curError, IndexHisWeight;
			size_dist tempHisDiff;
			bool countedYN = false;
			size_dist minError = INFINITY, minDis = INFINITY;
			size_idx bestTIIdx;
			size_idx eposx, eposy;
			size_idx temp2didx, temp3didx, tempTIidx;
			size_idx sumidx_posx;
			int compareNum = 0;
			compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
			for (size_idx u = -cstart; u <= cend; ++u) {
				sumidx_posx = iSyz + trimIndex(level, j + u)*OUTsize_;
				for (size_idx v = -cstart; v <= cend; ++v) {
					temp3didx = sumidx_posx + trimIndex(level, k + v);			//[i][posx][posy]
					eposx = (Origin_x[level][temp3didx] / TIsize_) - u;
					eposy = (Origin_x[level][temp3didx] % TIsize_) - v;

					//if (!(eposx >= 0 && eposx < Sz && eposy >= 0 && eposy < Sy))
					if (!(eposx >= start && eposx < TIsize_ - end && eposy >= start && eposy < TIsize_ - end))
					{
						//printf("\nj,k = %d,%d", j, k);
						//printf("\nu,v = %d,%d", u, v);
						//printf("\nori = %d,%d", eposx+u, eposy+v);
						//printf("\nepos = %d,%d", eposx, eposy);
						//_getch();
						continue;
					}
						

					tempTIidx = eposx*TIsize_ + eposy;								//[x][y] idx=x*Sz+y			
					for (int l = 0; l < COHERENCENUM; ++l) {						//[0,COHERENCENUM]				
						temp2didx = KCoherence_x[level][tempTIidx][l];
						int p = 0;
						for (; p < compareNum; ++p) {
							if (compareIdx[p] == temp2didx)	break;
						}
						if (p < compareNum)	continue;
						
						curDis = getFullDistance(level, m_exemplar_x[level], temp2didx, current_neighbor);

						curhis = IndexHis_x[level][sparseIdx(level, temp2didx)];
						tempHisDiff = max(0.0f, 1.0f*(curhis - avgIndexHis[level]));
						IndexHisWeight = 1.0f + factorIndex * tempHisDiff;
						curError = IndexHisWeight * curDis;
						countedYN = true;
						if (tempHisDiff > IndexHisManualControl*avgIndexHis[level]) curError = FLT_MAX;	//!manually control indexhis

						curbin = (int)m_exemplar_x[level][temp2didx];
						if (minError > curError) {								//min error			
							minError = curError;
							minDis = curDis;
							bestTIIdx = temp2didx;
							besthis = curhis;
							bestbin = curbin;
						}
						else if (curError - minError < 1e-8) {
							if (curhis < besthis) {
								minDis = curDis;
								bestTIIdx = temp2didx;
								besthis = curhis;
								bestbin = curbin;
							}
							else if (curhis == besthis && !FIRSTRUN) {
								if (PosHis[level][temp2didx] < PosHis[level][bestTIIdx])
									bestTIIdx = temp2didx;
							}
						}
						compareNum++;
						compareIdx.push_back(temp2didx);
					}//for (int l = 0; l < COHERENCENUM; ++l){	
				}//for (int v = -start; v <= end; ++v){
			}//for (int u = -start; u <= end; ++u){

			if (fabs(FLT_MAX - minError) > 1.0f) {
				size_idx formerNearestIdx = nearestIdx_x[level][idx];
				nearestWeight_x[level][idx] = 1.0f / minDis;
				if (formerNearestIdx != bestTIIdx) {
					nearestIdx_x[level][idx] = bestTIIdx;											//update nearestIdx				
					if (formerNearestIdx < TIsize2d_ && formerNearestIdx >= 0) {
						size_hiscount& addressFormerNearestIdx = IndexHis_x[level][sparseIdx(level, formerNearestIdx)];
						if (addressFormerNearestIdx>0)
#pragma omp atomic
						addressFormerNearestIdx--;
					}
					size_hiscount& addressbestTIIdx = IndexHis_x[level][sparseIdx(level, bestTIIdx)];
#pragma omp atomic
					addressbestTIIdx++;							//update IndexHis sparse grid 	
					isUnchanged = false;
				}
			}
			else {
				bestTIIdx = getRandomNearestIndex(level, IndexHis_x[level]);
				size_idx formerNearestIdx = nearestIdx_x[level][idx];
				nearestWeight_x[level][idx] = 1.0f * 0.01;
				if (formerNearestIdx != bestTIIdx) {
					nearestIdx_x[level][idx] = bestTIIdx;											//update nearestIdx				
					if (formerNearestIdx < TIsize2d_ && formerNearestIdx >= 0) {
						size_hiscount& addressFormerNearestIdx = IndexHis_x[level][sparseIdx(level, formerNearestIdx)];
						if (addressFormerNearestIdx>0)
#pragma omp atomic
						addressFormerNearestIdx--;
					}
					size_hiscount& addressbestTIIdx = IndexHis_x[level][sparseIdx(level, bestTIIdx)];
#pragma omp atomic
					addressbestTIIdx++;							//update IndexHis sparse grid 	
					isUnchanged = false;
				}
			}
			cvReleaseMat(&current_neighbor);
		}//for (size_idx i2 = 0; i2 < Size; ++i2) {	//X
	//#pragma omp parallel for
	}//#pragma omp parallel
	return isUnchanged;
}


size_idx DoPAR::getRandomNearestIndex(int level, vector<size_hiscount>& IndexHis) {
	size_idx TEXSIZE_h = TIsize[level]/2;
	size_idx start = 5, end = TEXSIZE_h - 5;
	if (end <= start) { start = 3; end = TEXSIZE_h - 3; }
	//size_idx start = 3, end = TEXSIZE_h - 3;
	
	size_idx coordx, coordy, tempidx, tempidxj;
	size_hiscount minVal = LONG_MAX, curVal = 0;

	for (size_idx i = start; i < end; i += GRID){
		tempidx = i* TEXSIZE_h;
		for (size_idx j = start; j < end; j += GRID){
			tempidxj = tempidx + j;
			//!!IndexHis is sparsed
			curVal = IndexHis[tempidxj] + IndexHis[tempidxj + TEXSIZE_h]
				+ IndexHis[tempidxj + 1]	+ IndexHis[tempidxj + TEXSIZE_h + 1];
			if (curVal < minVal) {
				coordx = i; coordy = j; 
				minVal = curVal;
			}
		}
	}
	//cout << endl << "(i,j)=" << coordx << "," << coordy;
	
	coordx *= 2; coordy *= 2;
	coordx += rand() % 4; coordy += rand() % 4;
	return (coordx*TIsize[level] + coordy);
}

size_dist DoPAR::getFullDistance(int level, vector<size_color>& exemplar, size_idx idx2d, CvMat * dataMat) {
	//2d square distance
	size_dist sum = 0.0f;
	size_idx R = static_cast<size_idx>(blockSize[level] * 0.5);
	size_idx n = 0;
	size_idx Sx = TIsize[level];
	size_idx tempIdx;
	size_dist dif;
	size_idx x = idx2d / Sx, y = idx2d % Sx;

	//[R,Sx-R+1) [R,Sx-R]
	if (x - R< 0 || x + R > Sx  || y - R<0 || y + R> Sx ) {
		printf("\ngetFullDistance() boundary");
		_getch();	
		exit(0);
	}

	for (size_idx i = -R; i < R; ++i) {		
		tempIdx = idx2d + i*Sx;
		for (size_idx j = -R; j < R; ++j) {
			dif = exemplar[tempIdx + j] - cvmGet(dataMat, 0, n++);
			sum += (dif * dif);			
		}
	}
	return (sum < min_dist) ? min_dist : sum;
	

}

//size_dist DoPAR::getFullDistance(int level, vector<size_color>& exemplar, size_idx idx2d, CvMat * dataMat, bool shrinkYN) {
//	//2d square distance, if shrinkYN==true, then R = R/2
//	size_dist sum = 0.0f;
//	size_idx R = static_cast<size_idx>(blockSize[level] * 0.5);
//	if (shrinkYN) R = ceil(R*0.5);
//	size_idx n = 0;
//	size_idx Sx = TEXSIZE[level];
//	size_idx tempIdx;
//	size_dist dif;
//	size_idx x = idx2d / Sx, y = idx2d % Sx;
//
//	if (x< R || x > Sx - R - 1 || y< R || y> Sx - R - 1) {//near boundary
//		for (size_idx i = -R; i < R; ++i) {
//			tempIdx = trimIndex(level, x + i)*Sx;
//			for (size_idx j = -R; j < R; ++j) {
//				dif = exemplar[tempIdx + trimIndex(level, y + j)] - cvmGet(dataMat, 0, n++);
//				sum += (dif * dif);
//			}
//		}
//		return (sum < min_dist) ? min_dist : sum;
//	}
//
//	for (size_idx i = -R; i < R; ++i) {
//		tempIdx = idx2d + i*Sx;
//		for (size_idx j = -R; j < R; ++j) {
//			dif = exemplar[tempIdx + j] - cvmGet(dataMat, 0, n++);
//			sum += (dif * dif);
//		}
//	}
//	return (sum < min_dist) ? min_dist : sum;
//}

bool DoPAR::isUnchangedBlock(int level, int direction, size_idx i, size_idx j, size_idx k) {
	// look up all neighbourhood in m_volume[i][j][k], check if all is unchanged (if anyone has changed (isUnchanged_==false), return false)
	const size_idx Sz = OUTsize[level];
	const size_idx jSz = j*Sz;
	const size_idx Syz = OUTsize[level] * OUTsize[level];
	const size_idx iSyz = i*Syz;
	size_idx start = static_cast<size_idx>(blockSize[level] * 0.5);			//4	//4	//3	//3
	size_idx end = static_cast<size_idx>((blockSize[level] - 1) * 0.5);		//3	//3	//2	//2
	if (level > 0 && end>1) {
		start -= 1;															//4	//3	//2	//2
		end -= 1;															//3	//2	//1	//1
	}

	size_idx tempidx;

	switch (direction){
	case(0) :	// X
		for (size_idx tj = j - start; tj <= j + end; ++tj){
			tempidx = iSyz + trimIndex(level, tj) * Sz;
			for (size_idx tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_x[level][tempidx + trimIndex(level, tk)])			//[i][tj][tk]
					return false;
			}
		}
		break;
	case(1) :	// Y
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trimIndex(level, ti) * Syz + jSz;
			for (int tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_y[level][tempidx + trimIndex(level, tk)])			//[ti][j][tk]
					return false;
			}
		}
		break;
	case(2) :	// Z
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trimIndex(level, ti) * Syz + k;
			for (int tj = j - start; tj <= j + end; ++tj){
				if (!isUnchanged_z[level][tempidx + trimIndex(level, tj)*Sz])		//[ti][tj][k]
					return false;
			}
		}
		break;
	}
	return true;
}


// ================ phase 2: optimization (E-step)=====================
void DoPAR::optimizeVolume(int level) {
	
	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx Size = OUTsize_*OUTsize_*OUTsize_;
	size_idx SizePosHis = 3 * TIsize2d_;
	if (SIM2D_YN) SizePosHis = TIsize2d_;


	size_idx candSize = static_cast<size_idx>(blockSize_ / GRID) * static_cast<size_idx>(blockSize_ / GRID);	//candidate has sparse grid
	size_idx start = static_cast<size_idx>(blockSize_ / (2 * GRID)) + 1;		
	size_idx end = start;					
	//if (shrinkDistanceTemplate) {
	//	start = ceil(start*0.5);
	//	end = start;
	//}

	size_idx s1 = -static_cast<size_idx>(blockSize_ * 0.5);
	size_idx e1 = static_cast<size_idx>((blockSize_ - 1) * 0.5);

	
#pragma omp parallel for schedule(static)
	for (size_idx i2 = 0; i2 < Size; ++i2) {
		size_idx idx = m_permutation[i2];			//[i][j][k]	
		size_idx k = idx % OUTsize_;
		size_idx j = (idx / OUTsize_) % OUTsize_;
		size_idx i = idx / OUTsize2d_;

		size_dist weight_acc = FLT_MIN, weight;
		size_color color_acc = 0.0f;
		size_color color_avg = 0.0f;
		size_idx tempnearestidx, tempidx;
		size_dist tempnearestweight;
		size_color tempcolor;
		size_idx tempx0, tempy0;
		size_idx tempx, tempy, deltax, deltay;
		size_idx iSyz = i*OUTsize2d_;
		size_idx jSz = j*OUTsize_;
		size_idx sumidx_tempx;
		size_dist tempHisDiff;
		//discrete solver
		vector<size_color> colorCand_x, colorCand_y, colorCand_z;
		colorCand_x.reserve(candSize);	colorCand_y.reserve(candSize);	colorCand_z.reserve(candSize);
		vector<size_idx> posCand_x, posCand_y, posCand_z;
		posCand_x.reserve(candSize);	posCand_y.reserve(candSize);	posCand_z.reserve(candSize);

		size_dist minweight, weightp, weightc;
		size_dist tempColorHisDiff, tempPosIdx;
		int inttempcolor;

		if (!SIM2D_YN) {
			// For Z	
			tempx0 = (i / GRID) * GRID;
			tempy0 = (j / GRID) * GRID;
			for (size_idx l = start; l >= -end; --l) {
				tempx = tempx0 + l * GRID;
				deltax = i - tempx;
				sumidx_tempx = trimIndex(level, tempx)*OUTsize2d_ + k;
				for (size_idx h = start; h >= -end; --h) {
					tempy = tempy0 + h * GRID;
					deltay = j - tempy;
					if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
						continue;

					tempidx = sumidx_tempx + trimIndex(level, tempy)*OUTsize_;									//[tempx][tempy][k]

					tempnearestidx = nearestIdx_z[level][tempidx];
					tempnearestweight = nearestWeight_z[level][tempidx];									//nearestidx from search step, weight=eudis^-0.6, larger means closer

					//tempnearestidx = Sz * trimIndex(level, tempnearestidx / Sz + deltax) + trimIndex(level, tempnearestidx%Sz + deltay);				
					size_idx coordx = tempnearestidx / TIsize_ + deltax;
					size_idx coordy = tempnearestidx%TIsize_ + deltay;
					if (coordx< 0 || coordy<0 || coordx >= TIsize_ || coordy >= TIsize_) continue;
					tempnearestidx = TIsize_ * coordx + coordy;
					tempcolor = m_exemplar_z[level][tempnearestidx];


					weightc = FLT_MAX;
					weightp = FLT_MAX;

					tempPosIdx = tempnearestidx + TIsize2d_ * 2;												//PosHis size=3TI!
					tempHisDiff = max(0.0f,  1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]));
					weightp = 1.0f / (1.0f + tempHisDiff * factorPos);				
					if (tempHisDiff > PosHisManualControl*avgPosHis[level]) weightp = FLT_MIN;

					if (ColorHis_ON && !FIRSTRUN) {		
						size_dist coloroffset = 1.0f * (tempcolor - Solid_Upper[level]) / (Pore_Lower[level] - Solid_Upper[level]);
						if (coloroffset <= 0.0f) tempColorHisDiff = 0.0f;
						else tempColorHisDiff = max(0.0f, coloroffset *100.0f*TIsize[level] * (poretotal_synthesis - poretotal_required) / poretotal_required);
						//inttempcolor = (int)tempcolor;
						//tempColorHisDiff = max(0.0f, 1.0f*(ColorHis_synthesis[level][inttempcolor] - ColorHis_exemplar[level][inttempcolor]) / (1 + ColorHis_exemplar[level][inttempcolor]));	
						
						weightc = 1.0f / (1.0f + tempColorHisDiff * factorC);
					}
					minweight = min(weightc, weightp);
					if (fabs(minweight - FLT_MIN)<1e-5) continue;


					colorCand_z.push_back(tempcolor);														//discrete solver
					posCand_z.push_back(tempnearestidx);

					weight = tempnearestweight * minweight;
					////// modify weight according to fix layer
					if (FixedLayerDir == 0 || FixedLayerDir == 1) weight *= DirectionalWeight;

					color_acc += weight * tempcolor;
					weight_acc += weight;
				}
			}

			// For Y	
			tempx0 = (i / GRID) * GRID;
			tempy0 = (k / GRID) * GRID;
			for (size_idx l = start; l >= -end; --l) {
				tempx = tempx0 + l * GRID;
				deltax = i - tempx;
				sumidx_tempx = trimIndex(level, tempx)*OUTsize2d_ + jSz;
				for (size_idx h = start; h >= -end; --h) {
					tempy = tempy0 + h * GRID;
					deltay = k - tempy;
					if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
						continue;

					tempidx = sumidx_tempx + trimIndex(level, tempy);									//[tempx][j][tempy]

					tempnearestidx = nearestIdx_y[level][tempidx];										//nearestidx from search step, weight=eudis^-0.6				
					tempnearestweight = nearestWeight_y[level][tempidx];

					//tempnearestidx = Sz * trimIndex(level, tempnearestidx / Sz + deltax) + trimIndex(level, tempnearestidx%Sz + deltay);
					size_idx coordx = tempnearestidx / TIsize_ + deltax;
					size_idx coordy = tempnearestidx%TIsize_ + deltay;
					if (coordx< 0 || coordy<0 || coordx >= TIsize_ || coordy >= TIsize_) continue;
					tempnearestidx = TIsize_ * coordx + coordy;

					tempcolor = m_exemplar_y[level][tempnearestidx];

					weightc = FLT_MAX;
					weightp = FLT_MAX;

					tempPosIdx = tempnearestidx + TIsize2d_;													//PosHis size=3TI!
					tempHisDiff = max(0.0f, 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]));
					weightp = 1.0f / (1.0f + tempHisDiff * factorPos);
					if (tempHisDiff > PosHisManualControl*avgPosHis[level]) weightp = FLT_MIN;

					if (ColorHis_ON && !FIRSTRUN) {
						size_dist coloroffset = 1.0f * (tempcolor - Solid_Upper[level]) / (Pore_Lower[level] - Solid_Upper[level]);						
						if (coloroffset <= 0.0f) tempColorHisDiff = 0.0f;
						else tempColorHisDiff = max(0.0f, coloroffset *100.0f*TIsize[level] * (poretotal_synthesis - poretotal_required) / poretotal_required);
						//inttempcolor = (int)tempcolor;
						//tempColorHisDiff = max(0.0f, 1.0f*(ColorHis_synthesis[level][inttempcolor] - ColorHis_exemplar[level][inttempcolor]) / (1 + ColorHis_exemplar[level][inttempcolor]));	

						weightc = 1.0f / (1.0f + tempColorHisDiff * factorC);
					}
					minweight = min(weightc, weightp);
					if (fabs(minweight - FLT_MIN)<1e-5) continue;

					colorCand_y.push_back(tempcolor);													//discrete solver
					posCand_y.push_back(tempnearestidx);

					weight = tempnearestweight * minweight;
					////// modify weight according to fix layer
					if (FixedLayerDir == 0 || FixedLayerDir == 2) weight *= DirectionalWeight;

					color_acc += weight * tempcolor;
					weight_acc += weight;
				}
			}
		}//if(!SIM2D_YN)

		// For X	
		tempx0 = (j / GRID) * GRID;
		tempy0 = (k / GRID) * GRID;
		for (size_idx l = start; l >= -end; --l) {
			tempx = tempx0 + l * GRID;
			deltax = j - tempx;
			sumidx_tempx = iSyz + trimIndex(level, tempx)*OUTsize_;
			for (size_idx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = k - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_tempx + trimIndex(level, tempy);									//[i][tempx][tempy]
				
				tempnearestidx = nearestIdx_x[level][tempidx];
				tempnearestweight = nearestWeight_x[level][tempidx];								//nearestidx from search step, weight=eudis^-0.6
				
				//tempnearestidx = Sz * trimIndex(level, tempnearestidx / Sz + deltax) + trimIndex(level, tempnearestidx%Sz + deltay);
				size_idx coordx = tempnearestidx / TIsize_ + deltax;
				size_idx coordy = tempnearestidx%TIsize_ + deltay;
				if (coordx< 0 || coordy<0 || coordx >= TIsize_ || coordy >= TIsize_) continue;
				tempnearestidx = TIsize_ * coordx + coordy;

				tempcolor = m_exemplar_x[level][tempnearestidx];
				
				weightc = FLT_MAX;
				weightp = FLT_MAX;

				tempPosIdx = tempnearestidx;
				tempHisDiff = max(0.0f, 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]));
				weightp = 1.0f / (1.0f + tempHisDiff * factorPos);
				if (tempHisDiff > PosHisManualControl*avgPosHis[level]) weightp = FLT_MIN;

				if (ColorHis_ON && !FIRSTRUN) {
					size_dist coloroffset = 1.0f * (tempcolor - Solid_Upper[level]) / (Pore_Lower[level] - Solid_Upper[level]);
					if (coloroffset <= 0.0f) tempColorHisDiff = 0.0f;
					else tempColorHisDiff = max(0.0f, coloroffset *100.0f*TIsize[level] *(poretotal_synthesis - poretotal_required) / poretotal_required);
					//inttempcolor = (int)tempcolor;
					//tempColorHisDiff = max(0.0f, 1.0f*(ColorHis_synthesis[level][inttempcolor] - ColorHis_exemplar[level][inttempcolor]) / (1 + ColorHis_exemplar[level][inttempcolor]));	

					weightc = 1.0f / (1.0f + tempColorHisDiff * factorC);
				}
				minweight = min(weightc, weightp);
				if (fabs(minweight - FLT_MIN)<1e-5) continue;

				colorCand_x.push_back(tempcolor);													//discrete solver
				posCand_x.push_back(tempnearestidx);

				weight = tempnearestweight * minweight;
				////// modify weight according to fix layer
				if (FixedLayerDir == 1 || FixedLayerDir == 2) weight *= DirectionalWeight;

				color_acc += weight * tempcolor;
				weight_acc += weight;
			}
		}
		
		color_avg = 1.0f * color_acc / weight_acc;					// least solver

		// Discrete solver
		size_dist minDis_x = INFINITY;		size_dist minDis_y = INFINITY;		size_dist minDis_z = INFINITY;
		size_idx closestIdx_x, closestIdx_y, closestIdx_z;	
		size_dist tempColorDiff;
		int bestorder=0;

		if (!SIM2D_YN) {
			// Z	
			bestorder = 0;
			for (int s = 0; s < colorCand_z.size(); s++) {
				tempColorDiff = abs(colorCand_z[s] - color_avg);
				if (tempColorDiff < minDis_z) {
					minDis_z = tempColorDiff;
					bestorder = s;
				}
				else if (tempColorDiff - minDis_z < 10e-9) {				//if colordiff same, compare PosHis, then IndexHis
					if (PosHis[level][posCand_z[s] + 2 * TIsize2d_] < PosHis[level][posCand_z[bestorder] + 2 * TIsize2d_])
						bestorder = s;
					else if (PosHis[level][posCand_z[s] + 2 * TIsize2d_] == PosHis[level][posCand_z[bestorder] + 2 * TIsize2d_]
						&& IndexHis_z[level][sparseIdx(level, posCand_z[s])] < IndexHis_z[level][sparseIdx(level, bestorder)])
						bestorder = s;
				}
			}
			closestIdx_z = posCand_z[bestorder];

			if (Origin_z[level][idx] != closestIdx_z) {											// update Z origin, isUnchangeblock
				Origin_z[level][idx] = closestIdx_z;
				isUnchanged_z[level][idx] = false;
			}
			else isUnchanged_z[level][idx] = true;


			// Y
			bestorder = 0;
			for (int s = 0; s < colorCand_y.size(); s++) {
				tempColorDiff = abs(colorCand_y[s] - color_avg);
				if (tempColorDiff < minDis_y) {
					minDis_y = tempColorDiff;
					bestorder = s;
				}
				else if (tempColorDiff - minDis_y < 10e-9) {				//if colordiff same, compare PosHis, then IndexHis
					if (PosHis[level][posCand_y[s] + TIsize2d_] < PosHis[level][posCand_y[bestorder] + TIsize2d_])
						bestorder = s;
					else if (PosHis[level][posCand_y[s] + TIsize2d_] == PosHis[level][posCand_y[bestorder] + TIsize2d_]
						&& IndexHis_y[level][sparseIdx(level, posCand_y[s])] < IndexHis_y[level][sparseIdx(level, bestorder)])
						bestorder = s;
				}
			}
			closestIdx_y = posCand_y[bestorder];
			if (Origin_y[level][idx] != closestIdx_y) {											// update Y origin, isUnchangeblock
				Origin_y[level][idx] = closestIdx_y;
				isUnchanged_y[level][idx] = false;
			}
			else isUnchanged_y[level][idx] = true;
		}//if(!SIM2D_YN)

		// X	
		bestorder = 0;
		for (int s = 0; s < colorCand_x.size(); s++) {
			tempColorDiff = abs(colorCand_x[s] - color_avg);
			if (tempColorDiff < minDis_x) {
				minDis_x = tempColorDiff;
				bestorder = s;
			}
			else if (tempColorDiff - minDis_x < 10e-9) {				//if colordiff same, compare PosHis, then IndexHis
				if (PosHis[level][posCand_x[s]] < PosHis[level][posCand_x[bestorder]])
					bestorder = s;
				else if (PosHis[level][posCand_x[s]] == PosHis[level][posCand_x[bestorder]]
					&& IndexHis_x[level][sparseIdx(level, posCand_x[s])] < IndexHis_x[level][sparseIdx(level, bestorder)])
					bestorder = s;
			}
		}
		closestIdx_x = posCand_x[bestorder];
		if (Origin_x[level][idx] != closestIdx_x) {											// update X origin, isUnchangeblock
			Origin_x[level][idx] = closestIdx_x;
			isUnchanged_x[level][idx] = false;
		}
		else isUnchanged_x[level][idx] = true;

		size_idx newPos;																	//PosHis size = 3TI		
		if (minDis_x <= minDis_y && minDis_x < minDis_z) {
			tempcolor = m_exemplar_x[level][closestIdx_x];
			newPos = closestIdx_x;															// TI*0+Pos_x	
		}
		else if (minDis_y <= minDis_z && minDis_y < minDis_x) {
			tempcolor = m_exemplar_y[level][closestIdx_y];
			newPos = TIsize2d_ + closestIdx_y;													// TI*1+Pos_y
		}
		else if (minDis_z <= minDis_x && minDis_z < minDis_y) {
			tempcolor = m_exemplar_z[level][closestIdx_z];
			newPos = TIsize2d_ * 2 + closestIdx_z;												// TI*2+Pos_z			
		}
		else {																				// if minDis_z==minDis_y==minDis_x rand.
			int ori = rand() % 3;
			switch (ori){
			case(0) :
				tempcolor = m_exemplar_x[level][closestIdx_x];
				newPos = closestIdx_x;
				break;
			case(1) :
				tempcolor = m_exemplar_y[level][closestIdx_y];
				newPos = TIsize2d_ + closestIdx_y;
				break;
			case(2) :
				tempcolor = m_exemplar_z[level][closestIdx_z];
				newPos = TIsize2d_ * 2 + closestIdx_z;
				break;
			}		
		}
		if (testNoDiscrete) tempcolor = color_avg;	

		size_idx formerPos = SelectedPos[level][idx];										
		size_hiscount& addressformerPos = PosHis[level][formerPos];							// update PosHis
		if (formerPos < SizePosHis && formerPos >= 0 && addressformerPos>0)
#pragma omp atomic
			addressformerPos--;
		size_hiscount& addressnewPos = PosHis[level][newPos];
#pragma omp atomic
		addressnewPos++;
		SelectedPos[level][idx] = newPos;													// update SelectedPos
				
		if (ColorHis_ON && !FIRSTRUN) {
//			size_hiscount& addressfomerColor = ColorHis_synthesis[level][(int)m_volume[level][idx]];	// update ColorHis
//			if (addressfomerColor>0)
//#pragma omp atomic
//				addressfomerColor--;			
//			size_hiscount& addressnewColor = ColorHis_synthesis[level][(int)tempcolor];
//#pragma omp atomic
//			addressnewColor++;

			if (m_volume[level][idx] <= Solid_Upper[level] && tempcolor > Solid_Upper[level])
#pragma omp atomic
				poretotal_synthesis++;
			else if (m_volume[level][idx] > Solid_Upper[level] && tempcolor <= Solid_Upper[level])
				if (poretotal_synthesis>0)
#pragma omp atomic
					poretotal_synthesis--;
		}
	
		m_volume[level][idx] = tempcolor;													// update m_volume

	}//for (size_idx i2 = 0; i2 < Size; ++i2) {
//#pragma omp parallel for schedule(static)

	if (!FIRSTRUN && (HisEqYN || level == MULTIRES-1)) 
		printf("porosity loss = %d", 100*(poretotal_synthesis - poretotal_required) / poretotal_required);

	if (FIRSTRUN) {
		if (ColorHis_ON) initColorHis_synthesis(level);
		FIRSTRUN = false;
	}
}


// ========= Index Histogram for search step =========
bool DoPAR::setNearestIndex(int level, vector<size_idx>& nearestIdx, vector<size_dist>& nearestWeight, vector<size_hiscount>&IndexHis,
	size_idx idx3d, size_idx newNearestIdx, size_dist dis) {
	//update IndexHis & NearestIndex, store EuDis^-0.6 -- search step
	size_idx TIsize_ = TIsize[level];

	size_idx formerNearestIdx = nearestIdx[idx3d];
	nearestWeight[idx3d] = 1.0f / dis;
	//nearestWeight[idx3d] = pow(dis, -0.6f);										//update nearestWeight

	if (formerNearestIdx == newNearestIdx)	return true;
	nearestIdx[idx3d] = newNearestIdx;												//update nearestIdx

	if (formerNearestIdx < TIsize_ * TIsize_ && formerNearestIdx >= 0) {
		size_idx sparsedFormerNearestIdx = sparseIdx(level, formerNearestIdx);		//update IndexHis sparse grid 
		if (IndexHis[sparsedFormerNearestIdx] > 0)	IndexHis[sparsedFormerNearestIdx]--;
	}

	IndexHis[sparseIdx(level, newNearestIdx)]++;									//update IndexHis sparse grid 	
	return false;
}


// ========= Color Histogram for optimize step =======
//void DoPAR::initColorHis_exemplar() {
//
//	for (int level = 0; level < MULTIRES; level++) {
//		size_idx Size2d = TIsize[level] * TIsize[level];
//		ColorHis_exemplar[level].resize(ColorHis_BinNum, 0);
//		vector<size_color>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
//		for (int ori = 0; ori < 3; ++ori) {
//			for (size_idx i = 0; i < Size2d; ++i) {
//				size_color c = (*p[ori])[i];
//				ColorHis_exemplar[level][(int)c] ++;
//			}
//		}
//		
//		//int actualBinNum = 0;
//		//for (int bin = 0; bin < ColorHis_BinNum; bin++) {
//		//	if (ColorHis_exemplar[level][bin] >0) actualBinNum++;
//		//}
//		//cout << endl << "actualBinNum= " << actualBinNum;
//
//		//!!!need to rescale to fit ColorHis_synthesis!!!
//		//float factor = Size3d*1.0f / (3 * Size2d);
//		//for (int bin = 0; bin < ColorHis_BinNum; bin++) {
//		//	ColorHis_exemplar[level][bin] = ceil(ColorHis_exemplar[level][bin]*factor);
//		//	if (ColorHis_exemplar[level][bin]>0) ColorHis_exemplar[level][bin] -= 1;		//similar to IndexHis,PosHis
//		//}
//	}
//}
void DoPAR::initColorHis_synthesis(int level) {

	ColorHis_synthesis[level].resize(ColorHis_BinNum, 0L);
	
	poretotal_required = m_volume[level].size() * porosity_required[level];
	//!!can add a random shift value to porosity

	poretotal_synthesis = 0;
	for (size_idx i = 0; i < m_volume[level].size(); i++) {
		ColorHis_synthesis[level][(int)m_volume[level][i]]++;

		if (m_volume[level][i] > Solid_Upper[level]) poretotal_synthesis++;
	}

	if (HisEqYN || level == MULTIRES-1)cout << endl << "poretotal=" << poretotal_synthesis << " pore_require=" << poretotal_required; 
}



void DoPAR::writeHistogram(int level) {
	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx Size = OUTsize_*OUTsize_*OUTsize_;

	const int cropedIndexHisStartX = blockSize_ * 0.25;
	const int cropedIndexHisWidth = TIsize_/2 - blockSize_/2 + 1;
	const int cropedIndexHisStartY = blockSize_ * 0.25;
	const int cropedIndexHisHeight = TIsize_ /2 - blockSize_/2 + 1;
	const int cropedPosHisStartX = 1;
	const int cropedPosHisWidth = TIsize_ - 2;
	const int cropedPosHisStartY = 1;
	const int cropedPosHisHeight = TIsize_ - 2;
	size_idx idx_i, idx_j, idx3d, idx2d;
	Mat tempMat;
	ostringstream name;
	string outputMainFileName = outputfilename.substr(0, outputfilename.find('.'));

	short deltaIndexCount(1), deltaPosCount(1);
	vector<unsigned short> Index_x, Index_y, Index_z;
	Index_x.resize(TIsize2d_, 0); Index_y.resize(TIsize2d_, 0); Index_z.resize(TIsize2d_, 0);
	vector<unsigned short> pos_x, pos_y, pos_z;
	pos_x.resize(TIsize2d_, 0);	pos_y.resize(TIsize2d_, 0);	pos_z.resize(TIsize2d_, 0);

	//int tmpavg_x = TEXSIZE_ * TEXSIZE_ * TEXSIZE_ / 4 / (TEXSIZE_/2 - blockSize_/2 +1) / (TEXSIZE_ / 2 - blockSize_ / 2 + 1);
	//for (int x = blockSize_ / 4; x < TEXSIZE_ / 2 - blockSize_ / 4 + 1; x++) {
	//	cout << endl;
	//	for (int y = blockSize_ / 4; y < TEXSIZE_ / 2 - blockSize_ / 4 + 1; y++) {
	//		cout << IndexHis_x[level][y + (TEXSIZE_ / 2) * x] - tmpavg_x << " ";
	//	}	
	//}_getch();

	//name.str("");
	//tempMat = Mat(Sx * 0.5, Sy * 0.5, CV_16UC1);
	//vector<unsigned short> tempIHx = vector<unsigned short>(IndexHis_x[level].begin(), IndexHis_x[level].end());
	//tempMat = Mat(tempIHx, true).reshape(1, tempMat.rows);
	//Mat cropedIndexHisMat_x = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	//int i(1);
	//name << outputMainFileName <<"_x" << "_IndexHis_L" << level << ".png";
	//string tempname = name.str();
	//while (fileExists(tempname) == true) {
	//	tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
	//	i++;
	//}
	//imwrite(tempname, cropedIndexHisMat_x);	//must be unsigned [short]!		
	//if (!SIM2D_YN) {
	//	name.str("");
	//	tempMat = Mat(Sx * 0.5, Sy * 0.5, CV_16UC1);
	//	vector<unsigned short> tempIHy = vector<unsigned short>(IndexHis_y[level].begin(), IndexHis_y[level].end());
	//	tempMat = Mat(tempIHy, true).reshape(1, tempMat.rows);
	//	Mat cropedIndexHisMat_y = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	//	name << outputMainFileName << "_y" << "_IndexHis_L" << level << ".png";
	//	if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), cropedIndexHisMat_y);

	//	name.str("");
	//	tempMat = Mat(Sx * 0.5, Sy * 0.5, CV_16UC1);
	//	vector<unsigned short> tempIHz = vector<unsigned short>(IndexHis_z[level].begin(), IndexHis_z[level].end());
	//	tempMat = Mat(tempIHz, true).reshape(1, tempMat.rows);
	//	Mat cropedIndexHisMat_z = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	//	name << outputMainFileName << "_z" << "_IndexHis_L" << level << ".png";
	//	if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), cropedIndexHisMat_z);

	//	name.str("");
	//	tempMat = cropedIndexHisMat_x + cropedIndexHisMat_y + cropedIndexHisMat_z;
	//	name << outputMainFileName << "_IndexHis_merged.png";
	//	//if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), tempMat);	
	//}

	for (size_idx i = 0; i < OUTsize_; i += 1) {									//IndexHis is sparsed. 
		idx_i = i*OUTsize2d_;
		for (size_idx j = 0; j < OUTsize_; j += 1) {
			idx_j = j*OUTsize_;
			for (size_idx k = 0; k < OUTsize_; k += 1) {
				idx3d = idx_i + idx_j + k;
				if (j % 2 == 0 && k % 2 == 0) {
					idx2d = nearestIdx_x[level][idx3d];						//X
					Index_x[idx2d] += deltaIndexCount;
				}
				if (i % 2 == 0 && k % 2 == 0) {
					idx2d = nearestIdx_y[level][idx3d];						//Y
					Index_y[idx2d] += deltaIndexCount;
				}
				if (i % 2 == 0 && j % 2 == 0) {
					idx2d = nearestIdx_z[level][idx3d];						//Z
					Index_z[idx2d] += deltaIndexCount;
				}
			}
		}
	}
	name.str("");
	tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
	tempMat = Mat(Index_x, true).reshape(1, tempMat.rows);
	//Mat cropedIndexHisMat_x = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	Mat cropedIndexHisMat_x = tempMat;
	name << outputMainFileName <<"_IndexHis_L" << level << "_0.png";
	imwrite(name.str(), cropedIndexHisMat_x);	
	if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) {
		name.str("");
		tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
		tempMat = Mat(Index_y, true).reshape(1, tempMat.rows);
		Mat cropedIndexHisMat_y = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
		name << outputMainFileName << "_IndexHis_L" << level << "_1.png";
		//imwrite(name.str(), cropedIndexHisMat_y);
	
		name.str("");
		tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
		tempMat = Mat(Index_z, true).reshape(1, tempMat.rows);
		Mat cropedIndexHisMat_z = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
		name << outputMainFileName << "_IndexHis_L" << level << "_2.png";
		//imwrite(name.str(), cropedIndexHisMat_z);
	
		name.str("");
		tempMat = cropedIndexHisMat_x + cropedIndexHisMat_y + cropedIndexHisMat_z;
		name << outputMainFileName << "_IndexHis_L" << level << "_merged.png";
		//imwrite(name.str(), tempMat);		
	}


	if (SIM2D_YN) return;
	for (size_idx i = 0; i < OUTsize_; i += 1) {									//PosHis not sparsed
		idx_i = i*OUTsize2d_;
		for (size_idx j = 0; j < OUTsize_; j += 1) {
			idx_j = j*OUTsize_;
			for (size_idx k = 0; k < OUTsize_; k += 1) {
				idx3d = idx_i + idx_j + k;
				idx2d = SelectedPos[level][idx3d];
				if (idx2d < TIsize2d_) {
					pos_x[idx2d] += deltaPosCount;							//X	
				}
				else if (idx2d < 2 * TIsize2d_) {
					pos_y[idx2d - TIsize2d_] += deltaPosCount;					//Y
				}
				else {
					pos_z[idx2d - 2 * TIsize2d_] += deltaPosCount;				//Z
				}
			}
		}
	}
	name.str("");
	tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
	tempMat = Mat(pos_x, true).reshape(1, tempMat.rows);
	//Mat cropedPosHisMat_x = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
	Mat cropedPosHisMat_x = tempMat;

	name << outputMainFileName << "_x" << "_PosHis_L" << level << ".png";
	imwrite(name.str(), cropedPosHisMat_x);		

	if (!SIM2D_YN) {
		name.str("");
		tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
		tempMat = Mat(pos_y, true).reshape(1, tempMat.rows);
		Mat cropedPosHisMat_y = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
		name << outputMainFileName << "_y" << "_PosHis_L" << level << ".png";
		//if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), cropedPosHisMat_y);

		name.str("");
		tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
		tempMat = Mat(pos_z, true).reshape(1, tempMat.rows);
		Mat cropedPosHisMat_z = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
		name << outputMainFileName << "_z" << "_PosHis_L" << level << ".png";
		//if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), cropedPosHisMat_z);

		name.str("");
		tempMat = cropedPosHisMat_x + cropedPosHisMat_y + cropedPosHisMat_z;
		name << outputMainFileName << "_PosHis_L" << level << "_merged.png";
		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), tempMat);
	}

	cout << endl << "croped Histograms are plotted.";
}

void DoPAR::upsampleVolume(int level) {	
// update nearestIdx & IndexHis for next level
// color does not matter
	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx Size = OUTsize_*OUTsize_*OUTsize_;
	
	size_idx doubleOUTsize_ = 2 * OUTsize_;
	size_idx doubleOUTsize2d_ = 4 * OUTsize2d_;
	size_idx doubleTIsize_ = 2 * TIsize_;

	size_idx Sx = OUTsize_;
	if (SIM2D_YN) Sx = 1;

	size_idx idx3d, didx3d, iSyz, jSz, sumidx_di, sumidx_dj;
	size_idx nidx2d, rnidx2d;
	size_idx coordx, coordy;
	size_idx rcoordx, rcoordy;
	

	//X
	for (size_idx i = 0; i < Sx; ++i) {
		iSyz = i*OUTsize2d_;
		sumidx_di = 2 * i * doubleOUTsize2d_;									//(2 * i)*(2 * Sy)*(2 * Sz)
		for (size_idx j = 0; j < OUTsize_; j += GRID) {					//sparse grid
			jSz = j*OUTsize_;
			sumidx_dj = 2 * j * doubleOUTsize_;								//(2 * j)*(2 * Sz)
			for (size_idx k = 0; k < OUTsize_; k += GRID) {				//sparse grid
				idx3d = iSyz + jSz + k;
				nidx2d = nearestIdx_x[level][idx3d];
				//rnidx2d = KCoherence_x[level][nidx2d][1 + static_cast<unsigned int>(rand() % (COHERENCENUM - 1))];
				rnidx2d = KCoherence_x[level][nidx2d][static_cast<unsigned int>(rand() % (COHERENCENUM))];
				coordx = nidx2d / TIsize_;	coordy = nidx2d % TIsize_;
				rcoordx = rnidx2d / TIsize_;	rcoordy = rnidx2d % TIsize_;

				coordx *= 2;	coordy *= 2;
				rcoordx *= 2;	rcoordy *= 2;
				nidx2d = coordx*doubleTIsize_ + coordy;					//new doubled nidx2d & rnidx2d
				rnidx2d = rcoordx*doubleTIsize_ + rcoordy;
				didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d, nidx2d, 1.0f);													//[di][dj][dk]				[coordx][coordy]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID*doubleOUTsize_, nidx2d + GRID*doubleTIsize_, 1.0f);			//[di][dj+GRID][dk]			[coordx+GRID][coordy]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID, nidx2d + GRID, 1.0f);										//[di][dj][dk+GRID]			[coordx][coordy+GRID]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID*doubleOUTsize_ + GRID, nidx2d + GRID*doubleTIsize_ + GRID, 1.0f);//[di][dj+GRID][dk+GRID]	[coordx+GRID][coordy+GRID]

				if (!SIM2D_YN) {
					didx3d += doubleOUTsize2d_;
					setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
						, didx3d, rnidx2d, 1.0f);												//[di+1][dj][dk]			[rcoordx][rcoordy]
					setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
						, didx3d + GRID*doubleOUTsize_, rnidx2d + GRID*doubleTIsize_, 1.0f);	//[di+1][dj+GRID][dk]		[rcoordx+GRID][rcoordy]
					setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
						, didx3d + GRID, rnidx2d + GRID, 1.0f);									//[di+1][dj][dk+GRID]		[rcoordx][rcoordy+GRID]
					setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
						, didx3d + GRID*doubleOUTsize_ + GRID, rnidx2d + GRID*doubleTIsize_ + GRID, 1.0f);			//[di+1][dj+GRID][dk+GRID]	[rcoordx+GRID][rcoordy+GRID]
				}
			}
		}
	}//X

	
	if(!SIM2D_YN) {
		//Y
		for (size_idx j = 0; j < OUTsize_; ++j) {
			jSz = j*OUTsize_;
			sumidx_dj = 2 * j * doubleOUTsize_;
			for (size_idx i = 0; i < OUTsize_; i += GRID) {					//sparse grid	
				iSyz = i*OUTsize2d_;
				sumidx_di = 2 * i * doubleOUTsize2d_;
				for (size_idx k = 0; k < OUTsize_; k += GRID) {				//sparse grid
					idx3d = iSyz + jSz + k;
					nidx2d = nearestIdx_y[level][idx3d];
					//rnidx2d = KCoherence_y[level][nidx2d][1 + static_cast<unsigned int>(rand() % (COHERENCENUM - 1))];
					rnidx2d = KCoherence_y[level][nidx2d][static_cast<unsigned int>(rand() % (COHERENCENUM))];
					coordx = nidx2d / TIsize_;	coordy = nidx2d % TIsize_;
					rcoordx = rnidx2d / TIsize_;	rcoordy = rnidx2d % TIsize_;

					coordx *= 2;	coordy *= 2;
					rcoordx *= 2;	rcoordy *= 2;
					nidx2d = coordx*doubleTIsize_ + coordy;					//new doubled nidx2d & rnidx2d
					rnidx2d = rcoordx*doubleTIsize_ + rcoordy;
					didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d, nidx2d, 1.0f);													//[di][dj][dk]				[coordx][coordy]
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d + GRID*doubleOUTsize2d_, nidx2d + GRID*doubleTIsize_, 1.0f);							//[di+GRID][dj][dk]			[coordx+GRID][coordy]
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d + GRID, nidx2d + GRID, 1.0f);									//[di][dj][dk+GRID]			[coordx][coordy+GRID]
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d + GRID*doubleOUTsize2d_ + GRID, nidx2d + GRID*doubleTIsize_ + GRID, 1.0f);			//[di+GRID][dj][dk+GRID]	[coordx+GRID][coordy+GRID]

					didx3d += doubleOUTsize_;
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d, rnidx2d, 1.0f);												//[di][dj+1][dk]			[rcoordx][rcoordy]
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d + GRID*doubleOUTsize2d_, rnidx2d + GRID*doubleTIsize_, 1.0f);							//[di+GRID][dj+1][dk]		[rcoordx+GRID][rcoordy]
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d + GRID, rnidx2d + GRID, 1.0f);									//[di][dj+1][dk+GRID]		[rcoordx][rcoordy+GRID]
					setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
						, didx3d + GRID*doubleOUTsize2d_ + GRID, rnidx2d + GRID*doubleTIsize_ + GRID, 1.0f);			//[di+GRID][dj+1][dk+GRID]	[rcoordx+GRID][rcoordy+GRID]
				}
			}
		}//Y

		 //Z
		for (size_idx k = 0; k < OUTsize_; ++k) {
			for (size_idx i = 0; i < OUTsize_; i += GRID) {					//sparse grid	
				iSyz = i*OUTsize2d_;
				sumidx_di = 2 * i * doubleOUTsize2d_;
				for (size_idx j = 0; j < OUTsize_; j += GRID) {				//sparse grid
					jSz = j*OUTsize_;
					sumidx_dj = 2 * j * doubleOUTsize_;

					idx3d = iSyz + jSz + k;
					nidx2d = nearestIdx_z[level][idx3d];
					//rnidx2d = KCoherence_z[level][nidx2d][1 + static_cast<unsigned int>(rand() % (COHERENCENUM - 1))];
					rnidx2d = KCoherence_z[level][nidx2d][static_cast<unsigned int>(rand() % (COHERENCENUM))];
					coordx = nidx2d / TIsize_;	coordy = nidx2d % TIsize_;
					rcoordx = rnidx2d / TIsize_;	rcoordy = rnidx2d % TIsize_;

					coordx *= 2;	coordy *= 2;
					rcoordx *= 2;	rcoordy *= 2;
					nidx2d = coordx*doubleTIsize_ + coordy;					//new doubled nidx2d & rnidx2d
					rnidx2d = rcoordx*doubleTIsize_ + rcoordy;
					didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d, nidx2d, 1.0f);												//[di][dj][dk]				[coordx][coordy]
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d + GRID*doubleOUTsize2d_, nidx2d + GRID*doubleTIsize_, 1.0f);							//[di+GRID][dj][dk]			[coordx+GRID][coordy]
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d + GRID*doubleOUTsize_, nidx2d + GRID, 1.0f);								//[di][dj+GRID][dk]			[coordx][coordy+GRID]
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d + GRID*doubleOUTsize2d_ + GRID*doubleOUTsize_, nidx2d + GRID*doubleTIsize_ + GRID, 1.0f);		//[di+GRID][dj+GRID][dk]	[coordx+GRID][coordy+GRID]

					didx3d += 1;
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d, rnidx2d, 1.0f);												//[di][dj][dk+1]			[rcoordx][rcoordy]
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d + GRID*doubleOUTsize2d_, rnidx2d + GRID*doubleTIsize_, 1.0f);						//[di+GRID][dj][dk+1]		[rcoordx+GRID][rcoordy]
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d + GRID*doubleOUTsize_, rnidx2d + GRID, 1.0f);								//[di][dj+GRID][dk+1]		[rcoordx][rcoordy+GRID]
					setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
						, didx3d + GRID*doubleOUTsize2d_ + GRID*doubleOUTsize_, rnidx2d + GRID*doubleTIsize_ + GRID, 1.0f);		//[di+GRID][dj+GRID][dk+1]	[rcoordx+GRID][rcoordy+GRID]
				}
			}
		}//Z
	}//if(!SIM2D_YN)

	cout << endl << "upsampled from " << level << " to " << level + 1;
}














