﻿#include "stdafx.h"
#include "DoPAR.h"

DoPAR::DoPAR(){
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
}

DoPAR::~DoPAR(){
	cleardata(MULTIRES - 1);
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

void DoPAR::ReadRunPar(string CurExeFile)
{
	cout << endl << "===========================================";
	cout << endl << "Set up your running parameters... ";
	cout << endl << "===========================================";

	string tempoutputfilename;
	string tempoutputformat;

	string Path;
	vector<string> ResLines;
	{//Read setup file
		string tmpstr, name;
		iCGetDirFileName(CurExeFile, Path, name);
		string PFName = Path + "PAR ANN Setup.DAT";
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
			cout << endl << " Failed to open Setup.DAT in current working directory !!!";
			cout << endl << " ============================================================";
			cout << endl << " Press any key to quit....";
			_getch(); exit(1);
		}
	}//Read setup file

	short Row(0);
	/////////////////////////Working directory
	workpath = ResLines[Row];
	if (workpath.back() != '\\') workpath += '\\';
	//////////////////////////* Specify training images in XY, XZ and YZ-plane

	cout << endl << "Workpath:" << workpath;
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) { FNameXY = workpath + ParV[0]; if (!fileExists(FNameXY)) { cout << endl << "Cannot find: " << endl << FNameXY; _getch(); exit(1); } else cout << endl << ParV[0]; }
			if (ParV.size() > 1) { FNameXZ = workpath + ParV[1]; if (!fileExists(FNameXZ)) { cout << endl << "Cannot find: " << endl << FNameXZ; _getch(); exit(1); } else cout << endl << ParV[1]; }
			if (ParV.size() > 2) { FNameYZ = workpath + ParV[2]; if (!fileExists(FNameYZ)) { cout << endl << "Cannot find: " << endl << FNameYZ; _getch(); exit(1); } else cout << endl << ParV[2]; }
		}
	}

	//Identical3DYN = false;
	//if (FNameXY == FNameXZ && FNameXY == FNameYZ)
	//	Identical3DYN = true;

	outputpath = "";
	//if (ResLines.size() > ++Row) {
	//	vector<string> ParV;
	//	GetNextRowParameters(Row, ResLines, ParV);
	//	if (ParV.size() > 0) outputpath = ResLines[Row];
	//	CreateDirectoryA(outputpath.c_str(), NULL); //ofstream cannot create folder!
	//}

	//----------read 3D model
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) modelFilename3D = outputpath + ParV[0];
		}
		//////seperate filename and format!
		tempoutputformat = ".RAW";
		tempoutputfilename = ParV[0].substr(0, ParV[0].rfind('.') == string::npos ? ParV[0].length() : ParV[0].rfind('.'));
	}



	outputfilename = tempoutputfilename + tempoutputformat;

	double UpPorosity = 0.50;

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

vector<uchar> DoPAR::load3Dmodel(const char* filename)
{//load 3D model raw file
	// open the file:
	streampos fileSize;
	ifstream file(filename, ios::binary);

	// get its size:
	file.seekg(0, ios::end);
	fileSize = file.tellg();
	file.seekg(0, ios::beg);

	// read the data:
	vector<uchar> fileData(fileSize);
	file.read((char*)&fileData[0], fileSize);

	if (fileData.size() != TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]) {
		cout << endl << "Error: Model size = " << fileData.size() << " SHOULD BE:" << TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0];
		_getch(); exit(1);
	}

	return fileData;
}

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

void DoPAR::GetStarted(string CurExeFile)
{
	ReadRunPar(CurExeFile);

	DoANNOptimization();
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

	vector<ANNcoord> diff(floatvector.size());
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

////////////////////////////////////////////////////////////////////////////////////////
//MULTIRES: larger number means finner level
const int DoPAR::blockSize[MULTIRES] = {8, 8, 6, 6};
ANNidx DoPAR::TEXSIZE[MULTIRES];
const short DoPAR::MAXITERATION[MULTIRES] = {40, 20, 10, 5};

////////////////////////////////////////////////////////////////////////////////////////


void DoPAR::DoANNOptimization() {
	init();

	time_t StartTime;	time(&StartTime);
	unsigned long t1, t2, t3;
	for (int curlevel = 0; curlevel < MULTIRES; curlevel++) {
		cout << endl << "=============level: " << curlevel << "===============";		
		
		initPermutation(curlevel);
		for (int loop = 0; loop < MAXITERATION[curlevel]; loop++) {
			if ((curlevel == 0 && loop % 10 == 0) || (curlevel == 1 && loop % 5 == 0) || (curlevel == 2 && loop % 3 == 0) || (curlevel > 2))
				cout << endl << "iteration: " << loop;
			if (curlevel == 0 && loop == 0) searchVolume(curlevel);			//!!the first run on level0 should be started by search	

			t1 = GetTickCount();
			optimizeVolume(curlevel);
			t2 = GetTickCount();
			if ((curlevel == 1 && loop % 5 == 0) || (curlevel == 2 && loop % 3 == 0) || (curlevel > 2))
				cout << endl << "optmize: " << (t2 - t1) / 1000.0 << " s.";
	
			if (searchVolume(curlevel)){
				cout << endl << "converged, skip to next level.";
				break;
			}
			t3 = GetTickCount();
			if ((curlevel == 1 && loop % 5 == 0) || (curlevel == 2 && loop % 3 == 0) || (curlevel > 2))
				cout << endl << "search: " << (t3 - t2) / 1000.0 << " s.";
		}	
		if (/*true*/curlevel == MULTIRES - 1 || TEXSIZE[curlevel] >= 256) {// ouput model & histogram
			outputmodel(curlevel);
			writeHistogram(curlevel);
		}
		if (curlevel < MULTIRES - 1) {// level up
			allocateVectors(curlevel+1);
			upsampleVolume(curlevel);
	
			cleardata(curlevel);	
		}
	}

	time_t NewTime;		time(&NewTime);
	cout << endl << "Total reconstruction time: " << unsigned long(NewTime - StartTime);
	cleardata(MULTIRES - 1);
}

void DoPAR::allocateVectors(int level) {
	const ANNidx S2d_ = TEXSIZE[level] * TEXSIZE[level];
	const ANNidx S3d_ = S2d_*TEXSIZE[level];

	if (level == 0) {
		isUnchanged_x.resize(MULTIRES); isUnchanged_y.resize(MULTIRES);		isUnchanged_z.resize(MULTIRES);
		nearestIdx_x.resize(MULTIRES);	nearestIdx_y.resize(MULTIRES);		nearestIdx_z.resize(MULTIRES);
		nearestWeight_x.resize(MULTIRES); nearestWeight_y.resize(MULTIRES); nearestWeight_z.resize(MULTIRES);
		Origin_x.resize(MULTIRES);		Origin_y.resize(MULTIRES);			Origin_z.resize(MULTIRES);
		IndexHis_x.resize(MULTIRES);	IndexHis_y.resize(MULTIRES);		IndexHis_z.resize(MULTIRES);
		PosHis.resize(MULTIRES);
		SelectedPos.resize(MULTIRES);
	}
	isUnchanged_x[level].resize(S3d_, false);		isUnchanged_y[level].resize(S3d_, false);		isUnchanged_z[level].resize(S3d_, false);
	nearestIdx_x[level].resize(S3d_, 205000);		nearestIdx_y[level].resize(S3d_, 205000);		nearestIdx_z[level].resize(S3d_, 205000);
	nearestWeight_x[level].resize(S3d_, min_dist);	nearestWeight_y[level].resize(S3d_, min_dist);	nearestWeight_z[level].resize(S3d_, min_dist);
	Origin_x[level].resize(S3d_, 205000);			Origin_y[level].resize(S3d_, 205000);			Origin_z[level].resize(S3d_, 205000);	
	//!PosHis size=3*TI
	SelectedPos[level].resize(S3d_, 615000);
	PosHis[level].resize(S2d_*3, 0.0f);
	//!sparse grid IndexHis
	IndexHis_x[level].resize(S2d_ / 4, 0.0f);		IndexHis_y[level].resize(S2d_ / 4, 0.0f);		IndexHis_z[level].resize(S2d_ / 4, 0.0f);
}

void DoPAR::init() {
	// load TI
	if (!loadExemplar()) return;

	// allocate memory for all global vectors (level0), init His=0
	allocateVectors(0);

	// load Model
	if (!loadVolume()) return;			

	// K-Coherence
	computeKCoherence();				

	// init deltaHis, linear factor
	for (int i = 0; i < MULTIRES; i++){
		deltaIndexHis[i] = (1.0f * (TEXSIZE[i] - blockSize[i] + 2)*(TEXSIZE[i] - blockSize[i] + 2)) / (TEXSIZE[i] * TEXSIZE[i] * TEXSIZE[i]);
		deltaPosHis[i] = 1.0f / TEXSIZE[i];
		avgIndexHis[i] = deltaIndexHis[i] * TEXSIZE[i];
		avgPosHis[i] = deltaPosHis[i] * TEXSIZE[i] / 3.0f;
		factorIndex[i] = 2.0f * TEXSIZE[i];
		factorPos[i] = 3 * 3 * 2.0f * TEXSIZE[i];
	}
}

void DoPAR::initPermutation(int level) {// random permutation (precomputed)
	ANNidx Size = TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
	m_permutation.clear();
	m_permutation.resize(Size);
	for (ANNidx i = 0; i <Size; ++i) {
		m_permutation[i] = i;
	}
}

void DoPAR::InitRandomVolume(int level) {
	//randomly assign Origin & color
	const ANNidx TEXSIZE_ = TEXSIZE[level];
	const ANNidx blockSize_ = blockSize[level];
	const ANNidx Sx = TEXSIZE_;
	const ANNidx Sy = TEXSIZE_;
	const ANNidx Sz = TEXSIZE_;
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	vector<ANNidx> randomidx2d(3);
	vector<ANNcoord>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	int ori;

	for (ANNidx xyz = 0; xyz < Size; ++xyz) {
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
		//ori = rand() % 3;
		//m_volume[level][xyz] = p[ori]->operator[](randomidx2d[ori]);
	}
}

void DoPAR::cleardata(int level) {
	//vector<ANNdist> tvf;	vector<ANNidx> tvl;		vector<bool> tvb;
	//m_volume[level].swap(tvf);
	//m_exemplar_x[level].swap(tvf);	m_exemplar_y[level].swap(tvf);	m_exemplar_z[level].swap(tvf);
	
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
	isUnchanged_x[level].shrink_to_fit();	isUnchanged_y[level].shrink_to_fit();	isUnchanged_z[level].shrink_to_fit();
	nearestIdx_x[level].shrink_to_fit();	nearestIdx_y[level].shrink_to_fit();	nearestIdx_z[level].shrink_to_fit();
	nearestWeight_x[level].shrink_to_fit();	nearestWeight_y[level].shrink_to_fit();	nearestWeight_z[level].shrink_to_fit();
	Origin_x[level].shrink_to_fit();		Origin_y[level].shrink_to_fit();		Origin_z[level].shrink_to_fit();
	IndexHis_x[level].shrink_to_fit();		IndexHis_y[level].shrink_to_fit();		IndexHis_z[level].shrink_to_fit();
	PosHis[level].shrink_to_fit();
	SelectedPos[level].shrink_to_fit();

	if (level == MULTIRES - 1) { 
		m_permutation.clear(); m_permutation.shrink_to_fit(); 
	}
}

// 2D Exemplar 
bool DoPAR::loadExemplar() {

	/////////////////////////////////////////////////////////////
	//exemplar_x --> YZ, exemplar_y --> ZX, exemplar_z --> XY
	//using imagej, XY slice is XY, ememplar_z
	//ZX slice can be attained by: 1. reslice top + flip virtical 2. then rotate 90 degrees left
	//YZ slice is done by: reslice left
	/////////////////////////////////////////////////////////////

	//---------------convert Mat to IplImage*---------------
	Mat matyz = cv::imread(FNameXY, CV_LOAD_IMAGE_ANYDEPTH);		 // ti grayscale, could be 16 bit!
	Mat matzx = cv::imread(FNameXZ, CV_LOAD_IMAGE_ANYDEPTH);
	Mat matxy = cv::imread(FNameYZ, CV_LOAD_IMAGE_ANYDEPTH);

	TEXSIZE[MULTIRES - 1] = matyz.cols;
	if (matyz.cols != matyz.rows) { cout << endl << "matyz.cols != matyz.rows"; _getch(); exit(0); }

	for (int level = MULTIRES - 1; level >= 0; --level) {	// size registration		
		TEXSIZE[level] = TEXSIZE[MULTIRES - 1] / pow(2, MULTIRES - 1 - level);
		if (TEXSIZE[MULTIRES - 1] % (ANNidx)pow(2, MULTIRES - 1) != 0) { cout << endl << "TI size not right for multi-level"; _getch(); exit(1); }
		// [begin] memory allocation -------------------------------------------
		m_exemplar_x.resize(MULTIRES);
		m_exemplar_y.resize(MULTIRES);
		m_exemplar_z.resize(MULTIRES);
		m_exemplar_x[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_y[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_z[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_volume.resize(MULTIRES);
		m_volume[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		// [end] memory allocation -------------------------------------------
	}

	// convert mat to vector
	if (matyz.isContinuous()) 	m_exemplar_x[MULTIRES - 1].assign(matyz.datastart, matyz.dataend);
	if (matzx.isContinuous()) 	m_exemplar_y[MULTIRES - 1].assign(matzx.datastart, matzx.dataend);
	if (matxy.isContinuous()) 	m_exemplar_z[MULTIRES - 1].assign(matxy.datastart, matxy.dataend);
		
	if (DISTANCEMAP_ON) {									// redistribute TI based on DM, no need to resize to 0-255
		cout << endl << "apply Distance Map transformation.";
		transformDM(m_exemplar_x[MULTIRES - 1], m_exemplar_y[MULTIRES - 1], m_exemplar_z[MULTIRES - 1]);
	}

	if (MULTIRES > 1) {										//! gauss filter resizing better than opencv interpolation resize(inter_area)
		cout << endl << "use Gaussian filter to resize.";
		for (int l = MULTIRES - 1; l > 0; --l) {		
			gaussImage(l, m_exemplar_x);
			gaussImage(l, m_exemplar_y);
			gaussImage(l, m_exemplar_z);
		}
	}

	return true;
}

void DoPAR::gaussImage(int level, vector<vector<ANNcoord>>& exemplar){
	if (level == 0) return;

	const float GAUSSWEIGHT[3][3] = {	{ 0.0625, 0.1250, 0.0625 },
										{ 0.1250, 0.2500, 0.1250 },
										{ 0.0625, 0.1250, 0.0625 } };
	ANNidx width = TEXSIZE[level];
	ANNidx cwidth = static_cast<ANNidx>(width/2);
	float sumcolor, sumweight;

	for (int i = 0; i < cwidth; i++) {
		for (int j = 0; j < cwidth; j++) {
			sumweight = 0.0f; 
			sumcolor = 0.0f;
			for (int k = 2 * i - 1; k < 2 * i + 1; k++) {
				for (int l = 2 * j - 1; l <= 2 * j + 1; ++l) {
					if (k >= 0 && k < width && l >= 0 && l < width)	{
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


//=============== distance map ===============
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

			//if (tDMap[idx] > MaxDis)
			//     MaxDis = tDMap[idx];

			if (DMap[idx] > MaxDis)
				MaxDis = DMap[idx];
		}

		tDMap.clear();

		if (!DisValYN) {
			vector<bool> UsedYN(MaxDis + 1, false);
			//for (long idx = 0; idx < tDMap.size(); ++idx) {
			//     if (tDMap[idx] > 0) {
			//           UsedYN[tDMap[idx]] = true;
			//     }
			//}

			//tDMap.clear();

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

			//if (tDMap[idx] > MaxDis)
			//     MaxDis = tDMap[idx];

			if (-DMap[idx] > MaxDis)
				MaxDis = -DMap[idx];
		}

		tDMap.clear();

		if (!DisValYN) {
			vector<bool> UsedYN(MaxDis + 1, false);
			//for (long idx = 0; idx < tDMap.size(); ++idx) {
			//     if (tDMap[idx] > 0) {
			//           UsedYN[tDMap[idx]] = true;
			//     }
			//}

			//tDMap.clear();
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

void DoPAR::transformDM(vector<ANNcoord>& exemplar1, vector<ANNcoord>& exemplar2, vector<ANNcoord>& exemplar3) {
	// redistribute TI based on DM, no need to resize to 0-255
	// first transform to DMap, then linear project (just make -s and +p to positive values)
	const short TEXSIZE_ = TEXSIZE[MULTIRES - 1];
	if (exemplar1.size() != exemplar2.size() || exemplar1.size() != exemplar3.size()) { cout << endl << "exemplars size different!"; getch(); exit(0); }
	vector<short> DMap_x(exemplar1.begin(), exemplar1.end());
	vector<short> DMap_y(exemplar2.begin(), exemplar2.end());
	vector<short> DMap_z(exemplar3.begin(), exemplar3.end());
	vector<char> tempchar(exemplar1.size());
	
	// compute DMap (-s & +p)			//default threshold = 110
	binaryChar(DMap_x, tempchar);
	DMap_x = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);
	binaryChar(DMap_y, tempchar);
	DMap_y = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);
	binaryChar(DMap_z, tempchar);
	DMap_z = GetDMap(TEXSIZE_, TEXSIZE_, 1, tempchar, 2, false);

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
	minVal = min(minVal1, min(minVal2, minVal3));
	maxVal = max(maxVal1, max(maxVal2, maxVal3));

	// transform to exemplar	// no need to resize to 0-255!   min -> 0, +1 -> -min
	Solid_Upper = -1 - minVal;
	Pore_Upper = maxVal - minVal - 1;
	for (long i = 0; i < DMap_x.size(); i++) {
		if (DMap_x[i] < 0) DMap_x[i] -= minVal;
		else DMap_x[i] -= minVal + 1;

		if (DMap_y[i] < 0) DMap_y[i] -= minVal;
		else DMap_y[i] -= minVal + 1;

		if (DMap_z[i] < 0) DMap_z[i] -= minVal;
		else DMap_z[i] -= minVal + 1;
	}

	//convert back to vector<float>
	exemplar1 = vector<ANNcoord>(DMap_x.begin(), DMap_x.end());
	exemplar2 = vector<ANNcoord>(DMap_y.begin(), DMap_y.end());
	exemplar3 = vector<ANNcoord>(DMap_z.begin(), DMap_z.end());

	if (true) {					//Generate DM TI
		ostringstream name;
		vector<unsigned short> tempushort(DMap_x.size());

		tempushort = vector<unsigned short>(DMap_x.begin(), DMap_x.end());		
		Mat DM1 = Mat(TEXSIZE_, TEXSIZE_, CV_16UC1);
		DM1 = Mat(tempushort, true).reshape(1, DM1.rows);					// vector to mat, need the same data type!
		name << "DM1_S" << (short)Solid_Upper << ".png";
		imwrite(name.str(), DM1);	name.str("");

		tempushort = vector<unsigned short>(DMap_y.begin(), DMap_y.end());
		Mat DM2 = Mat(TEXSIZE_, TEXSIZE_, CV_16UC1);
		DM2 = Mat(tempushort, true).reshape(1, DM2.rows);
		name << "DM2_S" << (short)Solid_Upper << ".png";
		imwrite(name.str(), DM2);	name.str("");

		tempushort = vector<unsigned short>(DMap_z.begin(), DMap_z.end());
		Mat DM3 = Mat(TEXSIZE_, TEXSIZE_, CV_16UC1);
		DM3 = Mat(tempushort, true).reshape(1, DM3.rows);
		name << "DM3_S" << (short)Solid_Upper << ".png";
		imwrite(name.str(), DM3);	name.str("");
		
		cout << endl << "output DM TI.";	//_getch();
	}
}


// load Volume
bool DoPAR::loadVolume() {
	//----------------convert Model(vector<uchar>) to m_volume (vector<vector<int>> (multires,x*y*z))
	//load from Model, later can also load from file		//level 0
	//if (fileExists(modelFilename3D.c_str()) == true) {
	//	vector<uchar> model = load3Dmodel(modelFilename3D.c_str());
	//	if (model.size() != TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]) { cout << endl << "Loaded Model size=" << model.size() << " should be=" << TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]; _getch(); exit(1); }
	//	m_volume[0] = vector<ANNcoord>(model.begin(), model.end());	
	//	cout << endl << "load 3D model done.";
	//}
	//else {
		cout << endl << "Use Random initial.";
		InitRandomVolume(0);
	//}
	return true;
}

void DoPAR::outputmodel(int level) {
	vector<uchar> tempUchar(m_volume[level].size());
	string tempoutputfilename = outputfilename;
	short resizedSolid_Upper;

	// binary model
	vector<short> tempshort(m_volume[level].begin(), m_volume[level].end());
	binaryUchar(tempshort, tempUchar, Solid_Upper);						// binary thresholded to 0&255
	tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "_L" + to_string(level) + "_binary.RAW";
	Write(outputpath + tempoutputfilename, tempUchar);	

	// Distance model up to 16 bit grayscale
	if (DISTANCEMAP_ON) {												
		ANNcoord scalingfactor(1.0f);
		if (Pore_Upper > 255) {											// scale to [0-255], for output vector<uchar>
			scalingfactor = 255.0f / Pore_Upper;
			resizedSolid_Upper = Solid_Upper * scalingfactor;
			transform(m_volume[level].begin(), m_volume[level].end(), tempUchar.begin(),
				std::bind2nd(std::multiplies<ANNcoord>(), scalingfactor));			//multiply by scalingfactor
		}
		tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "_L" + to_string(level)
			+ "_S" + to_string(resizedSolid_Upper) + ".RAW";
		Write(outputpath + tempoutputfilename, tempUchar);
	}

	cout << endl << "output done.";
}


// =========== K-coherence search =============
void DoPAR::computeKCoherence(){
	cout << endl << "K-coherence...";
	unsigned long time_start = clock();

	KCoherence_x.resize(MULTIRES);
	KCoherence_y.resize(MULTIRES);
	KCoherence_z.resize(MULTIRES);
	//==========multiple nearest index, position control=========
	ANNidxArray ann_index_x = new ANNidx[COHERENCENUM];
	ANNidxArray ann_index_y = new ANNidx[COHERENCENUM];
	ANNidxArray ann_index_z = new ANNidx[COHERENCENUM];
	ANNdistArray ann_dist_x = new ANNdist[COHERENCENUM];
	ANNdistArray ann_dist_y = new ANNdist[COHERENCENUM];
	ANNdistArray ann_dist_z = new ANNdist[COHERENCENUM];

	for (int level = 0; level < MULTIRES; ++level) {
		ANNidx TEXSIZE_ = TEXSIZE[level];
		ANNidx blockSize_ = blockSize[level];

		ANNidx width = TEXSIZE_ - blockSize_ + 1;
		ANNidx height = TEXSIZE_ - blockSize_ + 1;
		ANNidx dim = blockSize_ * blockSize_;
		ANNidx bias = blockSize_ / 2;
		ANNidx numData = width * height;

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
		ANNpoint queryPt_x, queryPt_y, queryPt_z;
		queryPt_x = annAllocPt(dim);
		queryPt_y = annAllocPt(dim);
		queryPt_z = annAllocPt(dim);

		ANNidx row = 0;
		for (ANNidx i = 0; i < width; ++i) {
			for (ANNidx j = 0; j < height; ++j) {
				ANNidx col = 0;
				ANNidx index0 = TEXSIZE_ * i + j;
				for (ANNidx m = 0; m <blockSize_; ++m) {
					for (ANNidx n = 0; n <blockSize_; ++n) {
						ANNidx index = index0 + m * TEXSIZE_ + n;		//[i+m][j+n]
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

		for (ANNidx i = 0; i < width; ++i) {
			for (ANNidx j = 0; j < height; ++j) {
				ANNidx num = 0;
				ANNidx TIindex = TEXSIZE_ * i + j;
				for (ANNidx m = 0; m < blockSize_; ++m) {
					for (ANNidx n = 0; n < blockSize_; ++n) {
						ANNidx index = TIindex + TEXSIZE_ * m + n;	//[i+m][j+n]
						queryPt_x[num] = m_exemplar_x[level][index];
						queryPt_y[num] = m_exemplar_y[level][index];
						queryPt_z[num] = m_exemplar_z[level][index];
						num++;
					}
				}
				kdTree_x->annkSearch(queryPt_x, COHERENCENUM, ann_index_x, ann_dist_x, 0);
				kdTree_y->annkSearch(queryPt_y, COHERENCENUM, ann_index_y, ann_dist_y, 0);
				kdTree_z->annkSearch(queryPt_z, COHERENCENUM, ann_index_z, ann_dist_z, 0);

				//Set K-Coherence
				ANNidx bias_TIindex = TIindex + bias*TEXSIZE_ + bias;
				KCoherence_x[level][bias_TIindex].resize(COHERENCENUM);
				KCoherence_y[level][bias_TIindex].resize(COHERENCENUM);
				KCoherence_z[level][bias_TIindex].resize(COHERENCENUM);
				for (int k = 0; k < COHERENCENUM; ++k) {
					KCoherence_x[level][bias_TIindex][k] = convertIndexANN(level, ann_index_x[k]);		//direction=0
					KCoherence_y[level][bias_TIindex][k] = convertIndexANN(level, ann_index_y[k]);		//direction=1
					KCoherence_z[level][bias_TIindex][k] = convertIndexANN(level, ann_index_z[k]);		//direction=2
				}
			}
		}

		//release
		annClose();
		delete kdTree_x;				delete kdTree_y;				delete kdTree_z;
		annDeallocPts(p_source_x);		annDeallocPts(p_source_y);		annDeallocPts(p_source_z);
		annDeallocPt(queryPt_x);		annDeallocPt(queryPt_y);		annDeallocPt(queryPt_z);
	}
	delete[] ann_index_x;		delete[] ann_index_y;		delete[] ann_index_z;
	delete[] ann_dist_x;		delete[] ann_dist_y;		delete[] ann_dist_z;

	cout << endl << "K-coherence done.";
	long time_end = clock();
	cout << " clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
}


//================= phase 1: search ===========================
bool DoPAR::searchVolume(int level) {
	const ANNidx TEXSIZE_ = TEXSIZE[level];
	const ANNidx blockSize_ = blockSize[level];
	const ANNidx Sx = TEXSIZE_;
	const ANNidx Sy = TEXSIZE_;
	const ANNidx Sz = TEXSIZE_;
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const ANNidx start = static_cast<ANNidx>(blockSize_ / 2);			//4	//4	//3	//3
	const ANNidx end = static_cast<ANNidx>((blockSize_-1) / 2);			//3	//3	//2	//2
	ANNidx cstart(start), cend(end);
	if (level > 0 && end>1) {
		cstart -=1;														//4	//3	//2	//2
		cend -=1;														//3	//2	//1	//1
	}

	vector<ANNidx> compareIdx; 
	ANNdist curDis, curError, IndexHisWeight;
	CvMat* current_neighbor = cvCreateMat(1, blockSize_*blockSize_, CV_32F);		//rows = 1, cols = dimesnion
	bool isUnchanged = true;
	int direction;

	//For Z
	direction = 2;
	shuffle(m_permutation.begin(), m_permutation.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation[i2];			//[i][j][k] idx=i*Sxy+j*Sx+k
		ANNidx k = idx % Sx;
		ANNidx j = (idx / Sx) % Sy;
		ANNidx i = idx / Sxy;
		if (i % GRID != 0 || j % GRID != 0)	continue;						//sparse grid			
		if (isUnchangedBlock(level, direction, i, j, k)) continue;			//check neighbours all unchanged or not

		ANNidx index = 0;
		for (ANNidx du = -start; du <= end; ++du) {	//N is neighbourhood size.
			ANNidx VCurIdx1 = Sxy * trimIndex(level, i + du) + k;
			for (ANNidx dv = -start; dv <= end; ++dv) {
				ANNidx index2 = VCurIdx1 + Sx * trimIndex(level, j + dv);			//[i+du][j+dv][k]
				cvmSet(current_neighbor, 0, index, m_volume[level][index2]);
				index++;
			}
		}

		ANNdist minError = INFINITY, minDis = INFINITY;
		ANNidx bestTIIdx;
		ANNidx eposx, eposy;
		ANNidx temp2didx, temp3didx, tempTIidx;
		ANNidx sumidx_posx;
		int compareNum = 0;
		compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		for (ANNidx u = -cstart; u <= cend; ++u) {
			sumidx_posx = trimIndex(level, i + u)*Sxy + k;
			for (ANNidx v = -cstart; v <= cend; ++v) {
				temp3didx = sumidx_posx + trimIndex(level, j + v)*Sx;				//[posx][posy][k]
				eposx = (Origin_z[level][temp3didx] / Sx) - u;
				eposy = (Origin_z[level][temp3didx] % Sx) - v;

				if (!(eposx >= start && eposx < Sx - end && eposy >= start && eposy < Sy - end))
					continue;		

				tempTIidx = eposx*Sx + eposy;
				for (int l = 0; l < COHERENCENUM; ++l) {							//[0,COHERENCENUM]				
					temp2didx = KCoherence_z[level][tempTIidx][l];
					int p = 0;
					//compareNum = compareIdx.size();
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx)	break;
					}
					if (p < compareNum)	continue;
					//IndexHis needs sparse grid
					IndexHisWeight = 1.0f + factorIndex[level] * max(0.0f, IndexHis_z[level][sparseIdx(level, temp2didx)] - avgIndexHis[level]);			
					curDis = getFullDistance(level, m_exemplar_z[level], temp2didx, current_neighbor);
					curError = IndexHisWeight * curDis;

					if (minError > curError) {								//min error			
						minError = curError;
						minDis = curDis;									
						bestTIIdx = temp2didx;
					}
					compareNum++;
					compareIdx.push_back(temp2didx);
				}//for (int l = 0; l < COHERENCENUM; ++l){	
			}//for (int v = -start; v <= end; ++v){
		}//for (int u = -start; u <= end; ++u){

		if (minError < INFINITY) {
			//update NearestIndex, IndexHis, store EuDis
			if (!setNearestIndex(level, nearestIdx_z[level], nearestWeight_z[level], IndexHis_z[level], idx, bestTIIdx, minDis))	
				isUnchanged = false;
		}
		else {
			cout << "bad point...\n";	//_getch();
			bestTIIdx = getRandomNearestIndex(level, IndexHis_z[level]);
			setNearestIndex(level, nearestIdx_z[level], nearestWeight_z[level], IndexHis_z[level], idx, bestTIIdx, 100);
			isUnchanged = false;
		}
	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {	//Z

	//For Y	
	direction = 1;
	shuffle(m_permutation.begin(), m_permutation.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation[i2];			//[i][j][k] idx=i*Sxy+j*Sx+k
		ANNidx k = idx % Sx;				
		ANNidx j = (idx / Sx) % Sy;			
		ANNidx i = idx / Sxy;				
		if (i % GRID != 0 || k % GRID != 0)	continue;						//sparse grid			
		if (isUnchangedBlock(level, direction, i, j, k)) continue;			//check neighbours all unchanged or not

		ANNidx index = 0;		
		ANNidx jSx = j*Sx;
		for (ANNidx du = -start; du <= end; ++du) {	//N is neighbourhood size.
			ANNidx VCurIdx2 = Sxy * trimIndex(level, i + du) + jSx;
			for (ANNidx dv = -start; dv <= end; ++dv) {
				ANNidx index2 = VCurIdx2 + trimIndex(level, k + dv);		//[i+du][j][k+dv]
				cvmSet(current_neighbor, 0, index, m_volume[level][index2]);					
				index++;
			}
		}
		
		ANNdist minError = INFINITY, minDis = INFINITY;
		ANNidx bestTIIdx;
		ANNidx eposx, eposy;
		ANNidx temp2didx, temp3didx, tempTIidx;
		ANNidx sumidx_posx;
		int compareNum = 0;
		compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		for (ANNidx u = -cstart; u <= cend; ++u) {
			sumidx_posx = trimIndex(level, i + u)*Sxy + jSx;
			for (ANNidx v = -cstart; v <= cend; ++v) {
				temp3didx = sumidx_posx + trimIndex(level, k + v);			//[posx][j][posy]
				eposx = (Origin_y[level][temp3didx] / Sx) - u;
				eposy = (Origin_y[level][temp3didx] % Sy) - v;
				
				if (!(eposx >= start && eposx < Sx - end && eposy >= start && eposy < Sy - end)) 
					continue;

				tempTIidx = eposx*Sx + eposy;
				for (int l = 0; l < COHERENCENUM; ++l) {					//[0,COHERENCENUM]				
					temp2didx = KCoherence_y[level][tempTIidx][l];
					int p = 0;
					//compareNum = compareIdx.size();
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx)	break;
					}
					if (p < compareNum)	continue;

					IndexHisWeight = 1.0f + factorIndex[level] * max(0.0f, IndexHis_y[level][sparseIdx(level, temp2didx)] - avgIndexHis[level]);			//IndexHis needs sparse grid
					curDis = getFullDistance(level, m_exemplar_y[level], temp2didx, current_neighbor);
					curError = IndexHisWeight * curDis;

					if (minError > curError) {								//min error			
						minError = curError;
						minDis = curDis;									
						bestTIIdx = temp2didx;
					}
					compareNum++;
					compareIdx.push_back(temp2didx);
				}//for (int l = 0; l < COHERENCENUM; ++l){	
			}//for (int v = -start; v <= end; ++v){
		}//for (int u = -start; u <= end; ++u){

		if (minError < INFINITY) {
			//update NearestIndex, IndexHis, store EuDis
			if (!setNearestIndex(level, nearestIdx_y[level], nearestWeight_y[level], IndexHis_y[level], idx, bestTIIdx, minDis))	
				isUnchanged = false;
		}
		else {
			cout << "bad point...\n";	//_getch();
			bestTIIdx = getRandomNearestIndex(level, IndexHis_y[level]);
			setNearestIndex(level, nearestIdx_y[level], nearestWeight_y[level], IndexHis_y[level], idx, bestTIIdx, 100);
			isUnchanged = false;
		}
	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {	//Y

	//For X		
	direction = 0;
	shuffle(m_permutation.begin(), m_permutation.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation[i2];			//[i][j][k] idx=i*Sxy+j*Sx+k
		ANNidx k = idx % Sx;				
		ANNidx j = (idx / Sx) % Sy;		
		ANNidx i = idx / Sxy;				
		if (j % GRID != 0 || k % GRID != 0)	continue;					//sparse grid			
		if(isUnchangedBlock(level,direction, i,j,k)) continue;			//check neighbours all unchanged or not
		
		ANNidx index = 0;		
		ANNidx iSxy = Sxy * i;
		for (ANNidx du = -start; du <= end; ++du) {	//N is neighbourhood size.
			ANNidx VCurIdx3 = iSxy + Sx * trimIndex(level, j + du);
			for (ANNidx dv = -start; dv <= end; ++dv) {
				ANNidx index2 = VCurIdx3 + trimIndex(level, k + dv);								//[i][j+du][k+dv]
				cvmSet(current_neighbor, 0, index, m_volume[level][index2]);						//set current_neighbor(0,col) to m_volume(idx)
				index++;
			}
		}

		ANNdist minError = INFINITY, minDis = INFINITY;
		ANNidx bestTIIdx;
		ANNidx eposx, eposy;
		ANNidx temp2didx, temp3didx, tempTIidx;
		ANNidx sumidx_posx;
		int compareNum = 0;
		compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		for (ANNidx u = -cstart; u <= cend; ++u){
			sumidx_posx = iSxy + trimIndex(level, j + u)*Sx;
			for (ANNidx v = -cstart; v <= cend; ++v){
				temp3didx = sumidx_posx + trimIndex(level, k + v);			//[i][posx][posy]
				eposx = (Origin_x[level][temp3didx] / Sx) - u;
				eposy = (Origin_x[level][temp3didx] % Sx) - v;
				
				if (!(eposx >= start && eposx < Sx - end && eposy >= start && eposy < Sy - end)) 
					continue;

				tempTIidx = eposx*Sx + eposy;								//[x][y] idx=x*Sx+y			
				for (int l = 0; l < COHERENCENUM; ++l){						//[0,COHERENCENUM]				
					temp2didx = KCoherence_x[level][tempTIidx][l];
					int p = 0;
					//compareNum = compareIdx.size();
					for (; p < compareNum; ++p){
						if (compareIdx[p] == temp2didx)	break;
					}
					if (p < compareNum)	continue;
					
					//IndexHis needs sparse grid
					IndexHisWeight = 1.0f + factorIndex[level] * max(0.0f, IndexHis_x[level][sparseIdx(level, temp2didx)] - avgIndexHis[level]);
					curDis = getFullDistance(level, m_exemplar_x[level], temp2didx, current_neighbor);
					curError = IndexHisWeight * curDis;

					if (minError > curError){								//min error			
						minError = curError;
						minDis = curDis;									//minDis=eudis^2
						bestTIIdx = temp2didx;
					}
					compareNum++;
					compareIdx.push_back(temp2didx);
				}//for (int l = 0; l < COHERENCENUM; ++l){	
			}//for (int v = -start; v <= end; ++v){
		}//for (int u = -start; u <= end; ++u){

		if (minError < INFINITY){
			//update NearestIndex, IndexHis, store EuDis
			if (!setNearestIndex(level, nearestIdx_x[level], nearestWeight_x[level], IndexHis_x[level], idx, bestTIIdx, minDis))
				isUnchanged = false;	
		}else{
			cout << "bad point...\n";	//_getch();			
			bestTIIdx = getRandomNearestIndex(level, IndexHis_x[level]);
			setNearestIndex(level, nearestIdx_x[level], nearestWeight_x[level], IndexHis_x[level], idx, bestTIIdx, 100);
			isUnchanged = false;
		}
	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {	//X

	cvReleaseMat(&current_neighbor);
	return isUnchanged;
}

ANNidx DoPAR::getRandomNearestIndex(int level, vector<ANNdist>& IndexHis) {
	ANNidx TEXSIZE_h = TEXSIZE[level]/2;
	ANNidx start = 5, end = TEXSIZE_h - 5;
	if (end <= start) { start = 3; end = TEXSIZE_h - 3; }
	ANNidx coordx, coordy, tempidx;
	ANNdist minVal = INFINITY, curVal = 0.0f;

	for (ANNidx i = start; i < end; i += 2){
		tempidx = i* TEXSIZE_h;
		for (ANNidx j = start; j < end; j += 2){
			//!!IndexHis is sparsed
			curVal = IndexHis[tempidx + j] + IndexHis[tempidx + TEXSIZE_h + j]
				+ IndexHis[tempidx + j + 1]	+ IndexHis[tempidx + TEXSIZE_h + j + 1];
			if (minVal > curVal) { coordx = i; coordy = j; }
		}
	}
	coordx *= 2; coordy *= 2;
	coordx += rand() % 4; coordy += rand() % 4;
	return (coordx*TEXSIZE[level] + coordy);
}

ANNdist DoPAR::getFullDistance(int level, vector<ANNcoord>& exemplar, ANNidx idx2d, CvMat * dataMat) {
	//2d square distance
	ANNdist sum = 0.0f;
	ANNidx R = static_cast<ANNidx>(blockSize[level] / 2);
	ANNidx n = 0;
	ANNidx Sx = TEXSIZE[level];
	ANNidx tempIdx;

	for (ANNidx i = -R; i < R; ++i) {
		tempIdx = idx2d + i*Sx;
		for (ANNidx j = -R; j < R; ++j) {
			ANNdist dif = exemplar[tempIdx + j] - cvmGet(dataMat, 0, n++);
			sum += (dif * dif);
		}
	}
	return (sum < min_dist) ? min_dist : sum;
}

bool DoPAR::isUnchangedBlock(int level, int direction, ANNidx i, ANNidx j, ANNidx k) {
	// look up all neighbourhood in m_volume[i][j][k], check if all is unchanged (if anyone has changed (isUnchanged_==false), return false)
	const ANNidx Sx = TEXSIZE[level];
	const ANNidx jSx = j*Sx;
	const ANNidx Sxy = TEXSIZE[level] * TEXSIZE[level];
	const ANNidx iSxy = i*Sxy;
	ANNidx start = static_cast<ANNidx>(blockSize[level] / 2);			//4	//4	//3	//3
	ANNidx end = static_cast<ANNidx>((blockSize[level] - 1) / 2);		//3	//3	//2	//2
	if (level > 0 && end>1) {
		start -= 1;														//4	//3	//2	//2
		end -= 1;														//3	//2	//1	//1
	}

	ANNidx tempidx;

	switch (direction){
	case(0) :	// X
		for (ANNidx tj = j - start; tj <= j + end; ++tj){
			tempidx = iSxy + trimIndex(level, tj) * Sx;
			for (ANNidx tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_x[level][tempidx + trimIndex(level, tk)])			//[i][tj][tk]
					return false;
			}
		}
		break;
	case(1) :	// Y
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trimIndex(level, ti) * Sxy + jSx;
			for (int tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_y[level][tempidx + trimIndex(level, tk)])			//[ti][j][tk]
					return false;
			}
		}
		break;
	case(2) :	// Z
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trimIndex(level, ti) * Sxy + k;
			for (int tj = j - start; tj <= j + end; ++tj){
				if (!isUnchanged_z[level][tempidx + trimIndex(level, tj)*Sx])		//[ti][tj][k]
					return false;
			}
		}
		break;
	}
	return true;
}


//================= phase 2: optimization =====================
void DoPAR::optimizeVolume(int level) {
	//cout << endl << "optimize...";

	const ANNidx TEXSIZE_ = TEXSIZE[level];
	const ANNidx blockSize_ = blockSize[level];
	const ANNidx Sx = TEXSIZE_;
	const ANNidx Sy = TEXSIZE_;
	const ANNidx Sz = TEXSIZE_;
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const ANNidx candSize = static_cast<ANNidx>(blockSize_ / 2) * static_cast<ANNidx>(blockSize_ / 2);	//candidate has sparse grid
	const ANNidx start = static_cast<ANNidx>(blockSize_ / (2 * GRID)) + 1;	//3	//3	//2	//2
	const ANNidx end = start;					
	ANNidx s1 = -static_cast<ANNidx>(blockSize_ / 2);						//-4//-4//-3//-3
	ANNidx e1 = static_cast<ANNidx>((blockSize_ - 1) / 2);					//3	//3	//2	//2
	if (level > 0 && e1>1) {								//reduce average blurring
		s1 += 1;															//-3//-3//-2//-2
		e1 -= 1;															//2	//2	//1	//1
	}
	
	shuffle(m_permutation.begin(), m_permutation.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation[i2];			//[i][j][k]	
		ANNidx k = idx % Sx;
		ANNidx j = (idx / Sx) % Sy;
		ANNidx i = idx / Sxy;

		ANNdist weight_acc = 0.0f, weight;
		ANNcoord color_acc = 0.0f;
		ANNcoord color_avg = 0.0f;
		ANNidx tempnearestidx, tempidx;
		ANNdist tempnearestweight;
		ANNcoord tempcolor;
		ANNidx tempx0, tempy0;
		ANNidx tempx, tempy, deltax, deltay;
		ANNidx iSxy = i*Sxy;
		ANNidx jSx = j*Sx;
		ANNidx sumidx_tempx;
		//discrete solver
		vector<ANNcoord> colorCand_x, colorCand_y, colorCand_z;
		colorCand_x.reserve(candSize);	colorCand_y.reserve(candSize);	colorCand_z.reserve(candSize);
		vector<ANNidx> posCand_x, posCand_y, posCand_z;
		posCand_x.reserve(candSize);	posCand_y.reserve(candSize);	posCand_z.reserve(candSize);

		// For Z	
		tempx0 = (i / GRID) * GRID;
		tempy0 = (j / GRID) * GRID;
		for (ANNidx l = start; l >= -end; --l) {
			tempx = tempx0 + l * GRID;
			deltax = i - tempx;
			sumidx_tempx = trimIndex(level, tempx)*Sxy + k;
			for (ANNidx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = j - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_tempx + trimIndex(level, tempy)*Sx;								//[tempx][tempy][k]
				
				tempnearestidx = nearestIdx_z[level][tempidx];			
				tempnearestweight = nearestWeight_z[level][tempidx];								//nearestidx from search step, weight=eudis^-0.6

				tempnearestidx += deltax * Sx + deltay;
				//if (level > 0 && (tempnearestidx >= Sxy || tempnearestidx < 0)) continue; 
				//{
				//	//cout << endl << "deltax=" << deltax << " deltay=" << deltay;
				//	if (tempx*Sxy + tempy*Sx + k != tempidx) { cout << endl << "tempidx wrong"; _getch(); }
				//	else cout << endl << " x=" << tempx << " y=" << tempy << " z=" << k << " " << nearestIdx_z[level][tempidx];
				//	_getch();
				//}
				tempcolor = m_exemplar_z[level][tempnearestidx];
				
				colorCand_z.push_back(tempcolor);													//discrete solver
				posCand_z.push_back(tempnearestidx);
				
				tempnearestidx += Sxy * 2;															//PosHis size=3TI!
				weight = 1.0f + factorPos[level] * max(0.0f, PosHis[level][tempnearestidx] - avgPosHis[level]);	// 1/3 is average PosHis
				weight = tempnearestweight / weight;
				color_acc += weight * tempcolor;
				weight_acc += weight;
			}
		}		
		
		// For Y	
		tempx0 = (i / GRID) * GRID;
		tempy0 = (k / GRID) * GRID;
		for (ANNidx l = start; l >= -end; --l) {
			tempx = tempx0 + l * GRID;
			deltax = i - tempx;
			sumidx_tempx = trimIndex(level, tempx)*Sxy + jSx;
			for (ANNidx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = k - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_tempx + trimIndex(level, tempy);									//[tempx][j][tempy]
				
				tempnearestidx = nearestIdx_y[level][tempidx];										//nearestidx from search step, weight=eudis^-0.6				
				tempnearestweight = nearestWeight_y[level][tempidx];

				tempnearestidx += deltax * Sx + deltay;
				tempcolor = m_exemplar_y[level][tempnearestidx];
			
				colorCand_y.push_back(tempcolor);													//discrete solver
				posCand_y.push_back(tempnearestidx);

				tempnearestidx += Sxy * 1;															//PosHis size=3TI!
				weight = 1.0f + factorPos[level] * max(0.0f, PosHis[level][tempnearestidx] - avgPosHis[level]);
				weight = tempnearestweight / weight;
				color_acc += weight * tempcolor;
				weight_acc += weight;
			}
		}
		
		// For X	
		tempx0 = (j / GRID) * GRID;
		tempy0 = (k / GRID) * GRID;
		for (ANNidx l = start; l >= -end; --l) {
			tempx = tempx0 + l * GRID;
			deltax = j - tempx;
			sumidx_tempx = iSxy + trimIndex(level, tempx)*Sx;
			for (ANNidx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = k - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_tempx + trimIndex(level, tempy);											//[i][tempx][tempy]
				
				tempnearestidx = nearestIdx_x[level][tempidx];
				tempnearestweight = nearestWeight_x[level][tempidx];										//nearestidx from search step, weight=eudis^-0.6
				
				tempnearestidx += deltax * Sx + deltay;
				tempcolor = m_exemplar_x[level][tempnearestidx];
				
				colorCand_x.push_back(tempcolor);															//discrete solver
				posCand_x.push_back(tempnearestidx);

				weight = 1.0f + factorPos[level] * max(0.0f, PosHis[level][tempnearestidx] - avgPosHis[level]);	//PosHis weighted
				weight = tempnearestweight / weight;
				color_acc += weight * tempcolor;
				weight_acc += weight;
			}
		}
		
		//if (weight_acc == 0) { cout << endl << "weight_acc=0  color_acc=" << color_acc; _getch(); }
		color_avg = color_acc / weight_acc;					// least solver

		// Discrete solver
		ANNdist minDis_x = INFINITY;		ANNdist minDis_y = INFINITY;		ANNdist minDis_z = INFINITY;
		ANNidx closestIdx_x, closestIdx_y, closestIdx_z;
		ANNdist tempdis, tempnum;
		
		// Z	
		auto temporder = min_element(colorCand_z.begin(), colorCand_z.end(), [=](ANNcoord x, ANNcoord y) {
			return abs(x - color_avg) < abs(y - color_avg);
		});												//compute discrete color,origin
		tempnum = distance(colorCand_z.begin(), temporder);
		minDis_z = abs(colorCand_z[tempnum] - color_avg);
		closestIdx_z = posCand_z[tempnum];

		if (Origin_z[level][idx] != closestIdx_z) {											// update Z origin, isUnchangeblock
			Origin_z[level][idx] = closestIdx_z;
			isUnchanged_z[level][idx] = false;
		}
		else isUnchanged_z[level][idx] = true;

		// Y	
		temporder = min_element(colorCand_y.begin(), colorCand_y.end(), [=](ANNcoord x, ANNcoord y) {
			return abs(x - color_avg) < abs(y - color_avg);
		});												//compute discrete color,origin
		tempnum = distance(colorCand_y.begin(), temporder);
		minDis_y = abs(colorCand_y[tempnum] - color_avg);
		closestIdx_y = posCand_y[tempnum];

		if (Origin_y[level][idx] != closestIdx_y) {											// update Y origin, isUnchangeblock
			Origin_y[level][idx] = closestIdx_y;
			isUnchanged_y[level][idx] = false;
		}
		else isUnchanged_y[level][idx] = true;

		// X	
		temporder = min_element(colorCand_x.begin(), colorCand_x.end(), [=](ANNcoord x, ANNcoord y) {
			return abs(x - color_avg) < abs(y - color_avg);
		});												//compute discrete color,origin
		tempnum = distance(colorCand_x.begin(), temporder);
		minDis_x = abs(colorCand_x[tempnum] - color_avg);
		closestIdx_x = posCand_x[tempnum];

		if (Origin_x[level][idx] != closestIdx_x) {											// update X origin, isUnchangeblock
			Origin_x[level][idx] = closestIdx_x;
			isUnchanged_x[level][idx] = false;
		}
		else isUnchanged_x[level][idx] = true;


		ANNidx newPos;																		//PosHis size = 3TI
		
		//if (minDis_z <= minDis_x && minDis_z < minDis_y) {//Z
		//	tempcolor = m_exemplar_z[level][closestIdx_z];
		//	newPos = Sxy * 2 + closestIdx_z;												
		//}
		//else if (minDis_y <= minDis_z && minDis_y < minDis_x) {//Y
		//	tempcolor = m_exemplar_y[level][closestIdx_y];
		//	newPos = Sxy + closestIdx_y;													
		//}
		//else {//X
		//	tempcolor = m_exemplar_x[level][closestIdx_x];
		//	newPos = closestIdx_x;													
		//}
		if (minDis_x <= minDis_y && minDis_x < minDis_z) {
			tempcolor = m_exemplar_x[level][closestIdx_x];
			newPos = closestIdx_x;															// TI*0+Pos_x	
		}
		else if (minDis_y <= minDis_z && minDis_y < minDis_x) {
			tempcolor = m_exemplar_y[level][closestIdx_y];
			newPos = Sxy + closestIdx_y;													// TI*1+Pos_y
		}
		else if (minDis_z <= minDis_x && minDis_z < minDis_y) {
			tempcolor = m_exemplar_z[level][closestIdx_z];
			newPos = Sxy * 2 + closestIdx_z;												// TI*2+Pos_z			
		}
		else {
			int ori = rand() % 3;
			switch (ori){
			case(0) :
				tempcolor = m_exemplar_x[level][closestIdx_x];
				newPos = closestIdx_x;
				break;
			case(1) :
				tempcolor = m_exemplar_y[level][closestIdx_y];
				newPos = Sxy + closestIdx_y;
				break;
			case(2) :
				tempcolor = m_exemplar_z[level][closestIdx_z];
				newPos = Sxy * 2 + closestIdx_z;
				break;
			}		
		}
		updatePosHis(level, PosHis[level], SelectedPos[level], idx, newPos);				// update PosHis 
		m_volume[level][idx] = tempcolor;													// update m_volume color

	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {
}


//========== Index Histogram for search step =========
bool DoPAR::setNearestIndex(int level, vector<ANNidx>& nearestIdx, vector<ANNdist>& nearestWeight, vector<ANNdist>&IndexHis,
	ANNidx idx3d, ANNidx newNearestIdx, ANNdist dis) {
	//update IndexHis	//update NearestIndex, store EuDis^-0.6 -- search step
	ANNidx formerNearestIdx = nearestIdx[idx3d];
	//nearestWeight[idx3d] = pow(dis, -0.6f);											//update nearestWeight
	nearestWeight[idx3d] = 1.0f / dis;

	if (formerNearestIdx == newNearestIdx)	return true;
	nearestIdx[idx3d] = newNearestIdx;												//update nearestIdx

	if (formerNearestIdx < TEXSIZE[level] * TEXSIZE[level] && formerNearestIdx >= 0) {
		ANNidx sparsedFormerNearestIdx = sparseIdx(level, formerNearestIdx);		//update IndexHis sparse grid 
		if (IndexHis[sparsedFormerNearestIdx] > 0)	IndexHis[sparsedFormerNearestIdx] -= deltaIndexHis[level];	
	}

	IndexHis[sparseIdx(level, newNearestIdx)] += deltaIndexHis[level];				//update IndexHis sparse grid 	
	return false;
}

//========== Position Histogram for optimize step ====
void DoPAR::updatePosHis(int level, vector<ANNdist>& PosHis, vector<ANNidx>& selectedPos, ANNidx idx3d, ANNidx newPos) {
	// update PosHis -- optimize step
	// no sparse grid!
	ANNidx formerPos = selectedPos[idx3d];
	if (formerPos < 3 * TEXSIZE[level] * TEXSIZE[level] && formerPos>=0)
		if (PosHis[formerPos] > 0)	PosHis[formerPos] -= deltaPosHis[level];
	
	selectedPos[idx3d] = newPos;
	PosHis[newPos] += deltaPosHis[level];
}

void DoPAR::writeHistogram(int level) {
	const ANNidx TEXSIZE_ = TEXSIZE[level];
	const ANNidx blockSize_ = blockSize[level];
	const ANNidx Sx = TEXSIZE_;
	const ANNidx Sy = TEXSIZE_;
	const ANNidx Sz = TEXSIZE_;
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const int cropedIndexHisStartX = blockSize_ / 2;
	const int cropedIndexHisWidth = Sx - blockSize_ + 1;
	const int cropedIndexHisStartY = blockSize_ / 2;
	const int cropedIndexHisHeight = Sy - blockSize_ + 1;
	const int cropedPosHisStartX = 1;
	const int cropedPosHisWidth = Sx - 2;
	const int cropedPosHisStartY = 1;
	const int cropedPosHisHeight = Sy - 2;
	ANNidx idx_i, idx_j, idx3d, idx2d;
	Mat tempMat = Mat(Sx, Sy, CV_16UC1);
	ostringstream name;

	short deltaIndexCount(1), deltaPosCount(1);
	vector<unsigned short> Index_x, Index_y, Index_z;
	Index_x.resize(Sxy, 0); Index_y.resize(Sxy, 0); Index_z.resize(Sxy, 0);
	vector<unsigned short> pos_x, pos_y, pos_z;
	pos_x.resize(Sxy, 0);	pos_y.resize(Sxy, 0);	pos_z.resize(Sxy, 0);


	for (ANNidx i = 0; i < Sx; i += 1) {									//IndexHis is sparsed. 
		idx_i = i*Sxy;
		for (ANNidx j = 0; j < Sy; j += 1) {
			idx_j = j*Sx;
			for (ANNidx k = 0; k < Sz; k += 1) {
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
	tempMat = Mat(Index_x, true).reshape(1, tempMat.rows);
	Mat cropedIndexHisMat_x = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	name << "IndexHis_L" << level << "_0.png";
	imwrite(name.str(), cropedIndexHisMat_x);	name.str("");

	tempMat = Mat(Index_y, true).reshape(1, tempMat.rows);
	Mat cropedIndexHisMat_y = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	name << "IndexHis_L" << level << "_1.png";
	imwrite(name.str(), cropedIndexHisMat_y);	name.str("");

	tempMat = Mat(Index_z, true).reshape(1, tempMat.rows);
	Mat cropedIndexHisMat_z = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	name << "IndexHis_L" << level << "_2.png";
	imwrite(name.str(), cropedIndexHisMat_z);	name.str("");


	for (ANNidx i = 0; i < Sx; i += 1) {									//PosHis not sparsed
		idx_i = i*Sxy;
		for (ANNidx j = 0; j < Sy; j += 1) {
			idx_j = j*Sx;
			for (ANNidx k = 0; k < Sz; k += 1) {
				idx3d = idx_i + idx_j + k;
				idx2d = SelectedPos[level][idx3d];
				if (idx2d < Sxy) {
					pos_x[idx2d] += deltaPosCount;							//X	
				}
				else if (idx2d < 2 * Sxy) {
					pos_y[idx2d - Sxy] += deltaPosCount;					//Y
				}
				else {
					pos_z[idx2d - 2 * Sxy] += deltaPosCount;				//Z
				}
			}
		}
	}
	tempMat = Mat(pos_x, true).reshape(1, tempMat.rows);
	Mat cropedPosHisMat_x = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
	name << "PosHis_L" << level << "_0.png";
	imwrite(name.str(), cropedPosHisMat_x);		name.str("");

	tempMat = Mat(pos_y, true).reshape(1, tempMat.rows);
	Mat cropedPosHisMat_y = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
	name << "PosHis_L" << level << "_1.png";
	imwrite(name.str(), cropedPosHisMat_y);		name.str("");

	tempMat = Mat(pos_z, true).reshape(1, tempMat.rows);
	Mat cropedPosHisMat_z = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
	name << "PosHis_L" << level << "_2.png";
	imwrite(name.str(), cropedPosHisMat_z);		name.str("");

	tempMat = cropedPosHisMat_x + cropedPosHisMat_y + cropedPosHisMat_z;
	name << "PosHis_L" << level << "_merged.png";
	imwrite(name.str(), tempMat);		name.str("");

	cout << endl << "croped Histograms are plotted.";
}


void DoPAR::upsampleVolume(int level) {	
	//cout << endl << "upsample from " << level << " to " << level + 1;
	const ANNidx TEXSIZE_ = TEXSIZE[level];
	const ANNidx blockSize_ = blockSize[level];
	const ANNidx Sx = TEXSIZE_;
	const ANNidx Sy = TEXSIZE_;
	const ANNidx Sz = TEXSIZE_;
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const ANNidx dSx = 2 * Sx;
	const ANNidx dSxy = dSx * dSx;

	ANNidx idx3d, didx3d, iSxy, jSx, sumidx_di, sumidx_dj;
	ANNidx nidx2d, rnidx2d;
	ANNidx coordx, coordy;
	ANNidx rcoordx, rcoordy;

	//X
	for (ANNidx i = 0; i < Sx; ++i){	
		iSxy = i*Sxy;
		sumidx_di = 2 * i * dSxy;									//(2 * i)*(2 * Sx)*(2 * Sx)
		for (ANNidx j = 0; j < Sy; j += GRID){					//sparse grid
			jSx = j*Sx;
			sumidx_dj = 2 * j * dSx;								//(2 * j)*(2 * Sx)
			for (ANNidx k = 0; k < Sz; k += GRID){				//sparse grid
				idx3d = iSxy + jSx + k;		
				nidx2d = nearestIdx_x[level][idx3d];
				rnidx2d = KCoherence_x[level][nidx2d][1 + static_cast<unsigned int>(rand() % (COHERENCENUM - 1))];
				coordx = nidx2d / Sx;	coordy = nidx2d % Sx;
				rcoordx = rnidx2d / Sx;	rcoordy = rnidx2d % Sx;

				coordx *= 2;	coordy *= 2;
				rcoordx *= 2;	rcoordy *= 2;
				nidx2d = coordx*dSx + coordy;					//new doubled nidx2d & rnidx2d
				rnidx2d = rcoordx*dSx + rcoordy;		
				didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d, nidx2d, 1.0f);													//[di][dj][dk]				[coordx][coordy]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID*dSx, nidx2d + GRID*dSx, 1.0f);								//[di][dj+GRID][dk]			[coordx+GRID][coordy]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID, nidx2d + GRID, 1.0f);										//[di][dj][dk+GRID]			[coordx][coordy+GRID]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID*dSx + GRID, nidx2d + GRID*dSx + GRID, 1.0f);				//[di][dj+GRID][dk+GRID]	[coordx+GRID][coordy+GRID]

				didx3d += dSxy;
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d , rnidx2d, 1.0f);												//[di+1][dj][dk]			[rcoordx][rcoordy]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID*dSx, rnidx2d + GRID*dSx, 1.0f);							//[di+1][dj+GRID][dk]		[rcoordx+GRID][rcoordy]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID, rnidx2d + GRID, 1.0f);									//[di+1][dj][dk+GRID]		[rcoordx][rcoordy+GRID]
				setNearestIndex(level + 1, nearestIdx_x[level + 1], nearestWeight_x[level + 1], IndexHis_x[level + 1]
					, didx3d + GRID*dSx + GRID, rnidx2d + GRID*dSx + GRID, 1.0f);			//[di+1][dj+GRID][dk+GRID]	[rcoordx+GRID][rcoordy+GRID]
			}
		}
	}//X

	//Y
	for (ANNidx j = 0; j < Sy; ++j) {
		jSx = j*Sx;
		sumidx_dj = 2 * j * dSx;
		for (ANNidx i = 0; i < Sx; i += GRID) {					//sparse grid	
			iSxy = i*Sxy;
			sumidx_di = 2 * i * dSxy;							
			for (ANNidx k = 0; k < Sz; k += GRID) {				//sparse grid
				idx3d = iSxy + jSx + k;
				nidx2d = nearestIdx_y[level][idx3d];
				rnidx2d = KCoherence_y[level][nidx2d][1 + static_cast<unsigned int>(rand() % (COHERENCENUM - 1))];
				coordx = nidx2d / Sx;	coordy = nidx2d % Sx;
				rcoordx = rnidx2d / Sx;	rcoordy = rnidx2d % Sx;

				coordx *= 2;	coordy *= 2;
				rcoordx *= 2;	rcoordy *= 2;
				nidx2d = coordx*dSx + coordy;					//new doubled nidx2d & rnidx2d
				rnidx2d = rcoordx*dSx + rcoordy;
				didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d, nidx2d, 1.0f);													//[di][dj][dk]				[coordx][coordy]
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d + GRID*dSxy, nidx2d + GRID*dSx, 1.0f);							//[di+GRID][dj][dk]			[coordx+GRID][coordy]
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d + GRID, nidx2d + GRID, 1.0f);									//[di][dj][dk+GRID]			[coordx][coordy+GRID]
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d + GRID*dSxy + GRID, nidx2d + GRID*dSx + GRID, 1.0f);			//[di+GRID][dj][dk+GRID]	[coordx+GRID][coordy+GRID]

				didx3d += dSx;
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d, rnidx2d, 1.0f);												//[di][dj+1][dk]			[rcoordx][rcoordy]
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d + GRID*dSxy, rnidx2d + GRID*dSx, 1.0f);							//[di+GRID][dj+1][dk]		[rcoordx+GRID][rcoordy]
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d + GRID, rnidx2d + GRID, 1.0f);									//[di][dj+1][dk+GRID]		[rcoordx][rcoordy+GRID]
				setNearestIndex(level + 1, nearestIdx_y[level + 1], nearestWeight_y[level + 1], IndexHis_y[level + 1]
					, didx3d + GRID*dSxy + GRID, rnidx2d + GRID*dSx + GRID, 1.0f);			//[di+GRID][dj+1][dk+GRID]	[rcoordx+GRID][rcoordy+GRID]
			}
		}
	}//Y

	//Z
	for (ANNidx k = 0; k < Sz; ++k) {
		for (ANNidx i = 0; i < Sx; i += GRID) {					//sparse grid	
			iSxy = i*Sxy;
			sumidx_di = 2 * i * dSxy;
			for (ANNidx j = 0; j < Sy; j += GRID) {				//sparse grid
				jSx = j*Sx;
				sumidx_dj = 2 * j * dSx;

				idx3d = iSxy + jSx + k;
				nidx2d = nearestIdx_z[level][idx3d];
				rnidx2d = KCoherence_z[level][nidx2d][1 + static_cast<unsigned int>(rand() % (COHERENCENUM - 1))];
				coordx = nidx2d / Sx;	coordy = nidx2d % Sx;
				rcoordx = rnidx2d / Sx;	rcoordy = rnidx2d % Sx;

				coordx *= 2;	coordy *= 2;
				rcoordx *= 2;	rcoordy *= 2;
				nidx2d = coordx*dSx + coordy;					//new doubled nidx2d & rnidx2d
				rnidx2d = rcoordx*dSx + rcoordy;
				didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d, nidx2d, 1.0f);												//[di][dj][dk]				[coordx][coordy]
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d + GRID*dSxy, nidx2d + GRID*dSx, 1.0f);							//[di+GRID][dj][dk]			[coordx+GRID][coordy]
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d + GRID*dSx, nidx2d + GRID, 1.0f);								//[di][dj+GRID][dk]			[coordx][coordy+GRID]
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d + GRID*dSxy + GRID*dSx, nidx2d + GRID*dSx + GRID, 1.0f);		//[di+GRID][dj+GRID][dk]	[coordx+GRID][coordy+GRID]

				didx3d += 1;
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d, rnidx2d, 1.0f);												//[di][dj][dk+1]			[rcoordx][rcoordy]
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d + GRID*dSxy, rnidx2d + GRID*dSx, 1.0f);						//[di+GRID][dj][dk+1]		[rcoordx+GRID][rcoordy]
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d + GRID*dSx, rnidx2d + GRID, 1.0f);								//[di][dj+GRID][dk+1]		[rcoordx][rcoordy+GRID]
				setNearestIndex(level + 1, nearestIdx_z[level + 1], nearestWeight_z[level + 1], IndexHis_z[level + 1]
					, didx3d + GRID*dSxy + GRID*dSx, rnidx2d + GRID*dSx + GRID, 1.0f);		//[di+GRID][dj+GRID][dk+1]	[rcoordx+GRID][rcoordy+GRID]
			}
		}
	}//Z

	//long count1_x(0), count1_y(0), count1_z(0), count2_x(0), count2_y(0), count2_z(0);
	//for (long i = 0; i < nearestIdx_z[level].size(); i++) {
	//	if (nearestIdx_x[level][i] < Sx*Sx && nearestIdx_x[level][i] >= 0) { count1_x++; }
	//	if (nearestIdx_y[level][i] < Sx*Sx && nearestIdx_y[level][i] >= 0) { count1_y++; }
	//	if (nearestIdx_z[level][i] < Sx*Sx && nearestIdx_z[level][i] >= 0) { count1_z++; }
	//}
	//for (long i = 0; i < nearestIdx_z[level + 1].size(); i++) {
	//	if (nearestIdx_x[level + 1][i] < dSx*dSx && nearestIdx_x[level + 1][i] >= 0) { count2_x++; }
	//	if (nearestIdx_y[level + 1][i] < dSx*dSx && nearestIdx_y[level + 1][i] >= 0) { count2_y++; }
	//	if (nearestIdx_z[level + 1][i] < dSx*dSx && nearestIdx_z[level + 1][i] >= 0) { count2_z++; }
	//}
	//cout << endl << "count1_x=" << count1_x << "  count1_y=" << count1_y << "  count1_z=" << count1_z;		//8192
	//cout << endl << "count2_x=" << count2_x << "  count2_y=" << count2_y << "  count2_z=" << count2_z;		//65536

	//if (curlevel > 0) {
	//	long count3 = 0;
	//	for (long i = 0; i < nearestIdx_z[curlevel].size(); i++) {
	//		if (nearestIdx_z[curlevel][i] < TEXSIZE[curlevel] * TEXSIZE[curlevel] && nearestIdx_z[curlevel][i] >= 0) {
	//			cout << endl << "x=" << i / (TEXSIZE[curlevel] * TEXSIZE[curlevel]) << " y=" << (i / TEXSIZE[curlevel]) % TEXSIZE[curlevel] << " z=" << i%TEXSIZE[curlevel] << " " << nearestIdx_z[curlevel][i];
	//			count3++;
	//		}
	//	}
	//	cout << endl << "count3 =" << count3; _getch();
	//}
}














