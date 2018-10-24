#include "stdafx.h"

#include "DoPAR.h"

const float inv_sqrt_2pi = 0.398942280401433f;
// operating files systems
long FileLength(const string& FName)
{
	ifstream InF(FName.c_str(), ios::in | ios::binary);
	if (!InF) return 0;

	InF.seekg(0, InF.end);
	long Length = InF.tellg() / sizeof(char);
	InF.seekg(0, InF.beg);

	InF.close();

	return Length;
}
bool fileExists(const string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}
bool Write(const string FPathName, vector<uchar> Data)
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
bool GetNextRowParameters(short Cno, vector<string>& ValidParStr, vector<string>& ParV)
{
	if ((unsigned short)Cno >= ValidParStr.size()) {
		cout << endl << "Wrong arguments!";
		char ch; cin >> ch; exit(0);
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
bool iFileExistYN(const string& PFileName)
{
	struct stat stFileInfo;
	int intStat = stat(PFileName.c_str(), &stFileInfo);
	if (intStat != 0) return false;
	return true;
}
bool ReadTxtFiles(const string PFName, vector<string>& ResLines)
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
void showMat(const cv::String& winname, const cv::Mat& mat)
{// Show a Mat object quickly. For testing purposes only.
	assert(!mat.empty());
	cv::namedWindow(winname);
	cv::imshow(winname, mat);
	cv::waitKey(0);
	cv::destroyWindow(winname);
}
vector<uchar> MatToVec_8UTouchar(Mat mat) {
	vector<uchar> array;
	if (mat.isContinuous()) {
		array.assign(mat.datastart, mat.dataend);
	}
	else {
		for (int i = 0; i < mat.rows; ++i) {
			array.insert(array.end(), mat.ptr<uchar>(i), mat.ptr<uchar>(i) + mat.cols);
		}
	}
	return array;
}
vector<float> MatToVec_8UTofloat(Mat mat) {
	vector<uchar> array;
	if (mat.isContinuous()) {
		array.assign(mat.datastart, mat.dataend);
	}
	else {
		for (int i = 0; i < mat.rows; ++i) {
			array.insert(array.end(), mat.ptr<uchar>(i), mat.ptr<uchar>(i) + mat.cols);
		}
	}
	vector<float> array_float(array.begin(), array.end());
	return array_float;
}

Mat VecToMat_ucharTo8U(vector<uchar> vec, int rows) {
	// vector to Mat, need the same data type!
	return Mat(vec, true).reshape(1, rows);
}
Mat VecToMat_floatTo8U(vector<float> vec, int rows) {
	// vector to Mat, need the same data type!
	vector<uchar> vec_uchar(vec.begin(), vec.end());
	return Mat(vec_uchar, true).reshape(1, rows);
}

// index convert
size_idx convertIndexANN_TIs(int level, size_idx index, size_idx TIsize_, size_idx blockSize_) {
	//convert ANNSearch_nearest_x_index to index_TIs
	size_idx tin, i, j, height, bias, ANNTIsize;
	height = TIsize_ - blockSize_ + 1;
	ANNTIsize = height *height;
	bias = blockSize_ / 2;

	tin = index / ANNTIsize;
	index %= ANNTIsize;
	i = index / height + bias;
	j = index % height + bias;

	return (tin*TIsize_*TIsize_ + i*TIsize_ + j);
}
size_idx trim(size_idx SIZE, size_idx index) {
	//Toroidal
	if (index < 0) index += SIZE;
	return index % SIZE;
}


template<typename T>
void idxToCoord(T idx, T dimension, T &i, T &j, T &k) {
	k = idx % dimension;
	j = (idx / dimension) % dimension;
	i = idx / (dimension*dimension);
}
template<typename T>
void TIsToRegular(T idx_TIs, T TIsize2d_, T &idx_regular, T &TInum) {
	idx_regular = idx_TIs % TIsize2d_;
	TInum = idx_TIs / TIsize2d_;

	//if (TInum < 0 || TInum >= MultiTIsNum) {
	//	printf("\nTIsToRegular: idx%d, TInum%d", idx_TIs, TInum);
	//	_getch();
	//}
}

//!read multiple TIs
bool ReadMultipleTIs(vector<string> &filelist, string inputfilename, string key) {
	string prefix, filename;
	int inputnum;
	
	if (inputfilename.rfind(key) != string::npos) {
		prefix = inputfilename.substr(0, inputfilename.rfind(key) + 2);
		inputnum = atoi(inputfilename.substr(inputfilename.rfind(key) + 2, inputfilename.rfind('.')).c_str());

		for (int num = 0; num < 100; num++) {
			filename = prefix + to_string(num) + ".png";
			if (fileExists(filename) && num!=inputnum) {
				filelist.push_back(filename);
				cout << endl << filename;
			}
			else if (!fileExists(filename) && num < inputnum) {
				cout << endl << filename <<" not found, quit"; _getch();
				return false;
			}
		}
	}

	return true;
}
void LoadtoMat(vector<Mat> &Matlist, vector<string> FNlist, bool cropYN = true) {
	//Load address to Mat; crop if not square; 
	Matlist.reserve(FNlist.size());
	Mat tempmat;
	for (int i = 0; i < FNlist.size(); i++) {
		tempmat = cv::imread(FNlist[i], CV_LOAD_IMAGE_ANYDEPTH);
		if (tempmat.cols != tempmat.rows) {
			cout << endl << "cols != rows, crop to square";
			int mindim = min(tempmat.cols, tempmat.rows);
			Mat cropedMat = tempmat(Rect(0, 0, mindim, mindim));
			cropedMat.copyTo(tempmat);
		}
		if (i > 0 && !(Matlist[i - 1].cols == tempmat.cols && Matlist[i - 1].rows == tempmat.rows)) {
			cout << endl << "Multiple TIs size not equal, quit"; _getch(); exit(0);
		}
		Matlist.push_back(tempmat);		
	}
}
bool checkTIbinary(vector<Mat> Matlist) {
	//check TI is grayscale or binary
	Mat maskMat;
	bool allbinary = true;
	for (int i = 0; i < Matlist.size(); i++) {
		inRange(Matlist[i], 255, 255, maskMat);
		if (countNonZero(maskMat) != countNonZero(Matlist[i])) {
			allbinary = false;
			break;
		}
	}
	return allbinary;
}
// init vector
template<typename T>
void initPermutation(T dimension, bool sim2D, vector<T>& permutationlist) {
	// random permutation (precomputed)
	T Size = dimension * dimension * dimension;
	if (sim2D) Size = dimension * dimension;

	permutationlist.clear();
	permutationlist.resize(Size);
	for (T i = 0; i <Size; ++i) {
		permutationlist[i] = i;
	}
}




DoPAR::DoPAR(){	

}
DoPAR::~DoPAR(){
}

void DoPAR::cleardata() {
	for (int level = 0; level < MULTIRES - 1; ++level) {
		m_volume[level].clear();
		//m_exemplar_x[level].clear();	m_exemplar_y[level].clear();	m_exemplar_z[level].clear();
		KCoherence_x[level].clear();	KCoherence_y[level].clear();	KCoherence_z[level].clear();
		isUnchanged_x[level].clear();	isUnchanged_y[level].clear();	isUnchanged_z[level].clear();
		nearestIdx_x[level].clear();	nearestIdx_y[level].clear();	nearestIdx_z[level].clear();
		nearestWeight_x[level].clear(); nearestWeight_y[level].clear(); nearestWeight_z[level].clear();
		Origin_x[level].clear();		Origin_y[level].clear();		Origin_z[level].clear();
		IndexHis_x[level].clear();		IndexHis_y[level].clear();		IndexHis_z[level].clear();
		PosHis[level].clear();
		SelectedPos[level].clear();

		m_volume[level].shrink_to_fit();
		//m_exemplar_x[level].shrink_to_fit();	m_exemplar_y[level].shrink_to_fit();	m_exemplar_z[level].shrink_to_fit();
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

void DoPAR::GetStarted(string CurExeFile, int TIseries)
{
	ReadRunPar_series(CurExeFile, TIseries);

	DoANNOptimization(TIseries);
	
	if (batchYN)
		for (batchsequenceNo = 1; batchsequenceNo < batchsequenceMax; batchsequenceNo++){
			ReadRunPar_series(CurExeFile, TIseries);
			DoANNOptimization(TIseries);
		}
}

// ================ read parameters ==========
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
		string PFName = Path + "PAR_Setup.DAT";//!changed to series 
		vector<string> TmpLines;
		if (!ReadTxtFiles(PFName, TmpLines)) {
			cout << endl;
			cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
			cout << endl << " Failed to open file '" << PFName.c_str() << "' !";
			cout << endl << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
			cout << endl;
			cout << endl << "Press any key to quit ...";
			_getch(); exit(0);
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
			_getch(); exit(0);
		}
	}//Read setup file
	short Row(-1);

	vector<string> ParV;
	///////////////////////// check random seed
	if (useRandomSeed) {
		cout << endl << "use Random Seed";
		srand((unsigned)time(NULL));
		std::random_device rd;  
		std::mt19937 mersennetwistergenerator(rd());
	}
	else {
		cout << endl << "use Fixed Seed = 0";
		srand(0);
		std::mt19937 mersennetwistergenerator(0);
	}
	//uniform_real_distribution<double> probabilitydistribution(0.0, 1.0);

	///////////////////////// Number of realizations
	GetNextRowParameters(++Row, ResLines, ParV);
	if (ParV.size() > 0) {
		if (ParV.size() > 0) NumRealization = atoi(ParV[0].c_str());
	}


	// for release version, set parameters fixed
	HisEqYN = true;
	GenerateTI = true;
	PatternEntropyAnalysisYN = false;
	PrintHisYN = true;
	factorIndex = 2.0f;
	factorPos = 2.0f;
	factorC = 2.0f;

	//GetNextRowParameters(++Row, ResLines, ParV);
	//if (ParV.size() > 0) {
	//	useRandomSeed = true;
	//	int c = 0;
	//	//if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) DMtransformYN = false; else DMtransformYN = true; c++;}
	//	//if (ParV.size() > c) {FixedLayerDir = atoi(ParV[c].c_str()) - 1; c++;}
	//	
	//	//if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) MultipleTIsYN = false; else MultipleTIsYN = true; c++; }
	//	
	//	if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) HisEqYN = false; else HisEqYN = true; c++; }
	//	if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) GenerateTI = false; else GenerateTI = true;  c++; }
	//	if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) PatternEntropyAnalysisYN = false; else PatternEntropyAnalysisYN = true; c++;	}
	//	if (ParV.size() > c) { if (atoi(ParV[c].c_str()) == 0) PrintHisYN = false; else PrintHisYN = true; c++;}
	//	if (ParV.size() > c) { factorIndex = atof(ParV[c].c_str()); c++; }
	//	if (ParV.size() > c) { factorPos = atof(ParV[c].c_str()); c++; }
	//	if (ParV.size() > c) { factorC = atof(ParV[c].c_str()); c++; factorC *= 100;	if (factorC == 0) ColorHis_ON = false; }
	//}
	
	// read temp blockSize
	GetNextRowParameters(++Row, ResLines, ParV);
	tempblockSize.clear();
	for (int c = 0; c < ParV.size(); c++){
		if (atoi(ParV[c].c_str()) == -1){
			blockSizeconfigYN = false;
			break;
		}
		blockSizeconfigYN = true;
		tempblockSize.push_back(atoi(ParV[c].c_str()));
	}
	//// read temp maxiteration
	//GetNextRowParameters(++Row, ResLines, ParV);
	//tempMAXITERATION.clear();
	//for (int c = 0; c < ParV.size(); c++){
	//	MAXITERATIONconfigYN = true;
	//	tempMAXITERATION.push_back(atoi(ParV[c].c_str()));
	//}
	if (blockSizeconfigYN || MAXITERATIONconfigYN) outputmultilevel = true;



	if (Row >= ResLines.size() - 1) {
		printf("\nFinish series, quit.");
		_getch();
		exit(0);
	}
	//!jump, according to series num
	for (int i = 0; i < TIseries; i++)
		Row+=3;
	///////////////////////// Working directory
	if (Row >= ResLines.size()-1) {
		printf("\nFinish series, quit.");
		_getch();
		exit(0);
	}
	
	workpath = ResLines[++Row];
	if (workpath.back() != '\\') workpath += '\\';


	///////////////////////// Specify training images in XY, XZ and YZ-plane	
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (MultipleTIsYN == false) { FNameXY.clear(); FNameXZ.clear(); FNameYZ.clear(); }

		if (ParV.size() > 0) {
			if (ParV.size() > 0) {
				if (batchYN == false && ParV[0].back() == '*') {
					batchYN = true; MultipleTIsYN = false;
					batchFileList_XY.clear(); //batchFileList_XZ.clear(); batchFileList_YZ.clear();
					batchsequenceNo = 0;

					//std::string strSearch = "C:\\Users\\dell\\Downloads\\FractureZone_8x\\FractureZone*";
					std::string strSearch = workpath + ParV[0];
					WIN32_FIND_DATAA ffd;
					HANDLE hFind = FindFirstFileA(strSearch.c_str(), &ffd);
					do{
						batchFileList_XY.push_back(ffd.cFileName);
					} while (FindNextFileA(hFind, &ffd) != 0);

					batchsequenceMax = batchFileList_XY.size();
				}

				if (batchYN) ParV[0] = batchFileList_XY[batchsequenceNo]; //FNameXY.push_back(workpath + batchFileList_XY[batchsequenceNo]);
				FNameXY.push_back(workpath + ParV[0]);
				if (!fileExists(FNameXY[0])) { cout << endl << "Cannot find: " << endl << FNameXY[0]; _getch(); exit(0); }
				else cout << endl << "XY" << endl << FNameXY[0]; 
				if (MultipleTIsYN) ReadMultipleTIs(FNameXY, FNameXY[0], "XY");		
			}


			if (ParV.size() > 1) {
				if (batchYN && ParV[1].back() == '*') {
					batchFileList_XZ.clear(); 

					std::string strSearch = workpath + ParV[1];
					WIN32_FIND_DATAA ffd;
					HANDLE hFind = FindFirstFileA(strSearch.c_str(), &ffd);
					do{
						batchFileList_XZ.push_back(ffd.cFileName);
					} while (FindNextFileA(hFind, &ffd) != 0);

					if (batchsequenceMax != batchFileList_XZ.size()){
						cout << endl << "batch size XY != XZ, quit";
						_getch(); exit(0);
					}
				}

				if (batchYN) ParV[1] = batchFileList_XZ[batchsequenceNo];
				FNameXZ.push_back(workpath + ParV[1]);
				if (!fileExists(FNameXZ[0])) { cout << endl << "Cannot find: " << endl << FNameXZ[0]; _getch(); exit(0); }
				else cout << endl << "XZ" << endl << FNameXZ[0];
				if (MultipleTIsYN) ReadMultipleTIs(FNameXZ, FNameXZ[0], "XZ");
			}


			if (ParV.size() > 2) {
				if (batchYN && ParV[2].back() == '*') {
					batchFileList_YZ.clear();

					std::string strSearch = workpath + ParV[2];
					WIN32_FIND_DATAA ffd;
					HANDLE hFind = FindFirstFileA(strSearch.c_str(), &ffd);
					do{
						batchFileList_YZ.push_back(ffd.cFileName);
					} while (FindNextFileA(hFind, &ffd) != 0);

					if (batchsequenceMax != batchFileList_YZ.size()){
						cout << endl << "batch size XY/XZ != YZ, quit";
						_getch(); exit(0);
					}
				}

				if (batchYN) ParV[2] = batchFileList_YZ[batchsequenceNo];
				FNameYZ.push_back(workpath + ParV[2]);
				if (!fileExists(FNameYZ[0])) { cout << endl << "Cannot find: " << endl << FNameYZ[0]; _getch(); exit(0); }
				else cout << endl << "YZ" << endl << FNameYZ[0];
				if (MultipleTIsYN) ReadMultipleTIs(FNameYZ, FNameYZ[0], "YZ");
			}


			if (FNameXY.size() != FNameYZ.size() || FNameXY.size() != FNameXZ.size()) {
				cout << endl << "Multiple TIs num not the same" << endl << FNameYZ[0]; _getch(); exit(0);
			}
			else if (MultipleTIsYN && FNameXY.size()>1) {
				MultiTIsNum = FNameXY.size();
				cout << endl << "Multiple TIs num=" << MultiTIsNum;
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
			cout << endl << "2D simulation ON, just use the first (set of) TI.";
			SIM2D_YN = true;
			outputmultilevel = true;
		}

		if (ParV[0].back() == '*'){
			if (batchYN != true) {
				cout << endl << "inconsistant with batchYN, quit"; _getch(); exit(0);
			}
			ParV[0] = batchFileList_XY[batchsequenceNo];
		}

		//no extension
		tempoutputfilename = ParV[0].substr(0, ParV[0].rfind('.') == string::npos ? ParV[0].length() : ParV[0].rfind('.'));

		//optional parameter: output size
		if (ParV.size() > 1) { outputsizeatlastlevel = atoi(ParV[1].c_str()); }
	}
	outputfilename = tempoutputfilename;


	if (SIM2D_YN) FixedLayerDir = -1;
	if (FixedLayerDir >= 0 && FixedLayerDir < 3) {
		cout << endl << "enable fixed layer, modify directional weight: " << "1.0-->" << DirectionalWeight;
	}

	if (MultiTIsNum > 1 && MultipleTIsYN) parameterstring += "_TI" + to_string(MultiTIsNum);
	if (DMtransformYN && HisEqYN) parameterstring += "_Eq" + to_string((int)HisEqYN);
	//parameterstring += "DM" + to_string((int)DMtransformYN);
	parameterstring += "I" + to_string((int)factorIndex) + "P" + to_string((int)factorPos) + "C" + to_string((int)factorC);
	if (FixedLayerDir > -1 && FixedLayerDir < 3)  parameterstring += "Fix" + to_string(FixedLayerDir) + "W" + to_string((int)(100 * DirectionalWeight));

	if (SIM2D_YN) parameterstring = "";
	
	//outputfilename += parameterstring;
	// Note: For release version, delete parameter names
}

// ================ load 2D TIs ==========
void DoPAR::initTIbasedparameters(vector<Mat>& XY, vector<Mat>& XZ, vector<Mat>& YZ) {
	//already checked X,Y,Z TIs same num,size
	int dimension = XY[0].cols;
	////based on Output dimension (not TI), choose multi-level parameters
	if (outputsizeatlastlevel == 0) outputsizeatlastlevel = dimension;
	MULTIRES = ceil(1e-5 + log2(outputsizeatlastlevel / 32.0));

	if (outputsizeatlastlevel < 40) { cout << endl << "Output size < 40, too small!"; _getch(); exit(0); }
	else if (outputsizeatlastlevel > 600) { cout << endl << "Output size > 600, too big!"; _getch(); exit(0); }

	cout << endl << "TI dimension=" << dimension <<" Output dimension="<< outputsizeatlastlevel << " MULTIRES=" << MULTIRES;

	if (dimension < 128) {
		cout << endl << "for small TI, test just using one level";
		MULTIRES = 1;	blockSize = { 8 };		MAXITERATION = { 25 };
	}
	if (PatternEntropyAnalysisYN) {
		cout << endl << "Pattern Entropy Analysis, one level";
		MULTIRES = 1;	blockSize = { 12 };		MAXITERATION = { 3 };
	}
	else {
		ANNerror.resize(MULTIRES, 0.0);	blockSize.resize(MULTIRES);	MAXITERATION.resize(MULTIRES);	
		vector<int> defaultblockSize = {8 ,8, 6, 6, 6};
		vector<int> defaultMAXITERATION = {25, 9, 3, 2, 2};
		vector<double> defaultANNerror = {0.0, 0.0, 0.0, 0.5, 1.0};

		if (MULTIRES < 3 || MULTIRES>5) {
			cout << endl << "MULTIRES not right, quit"; _getch(); exit(0);
		}

		for (int l = 0; l < MULTIRES; l++){
			if (blockSizeconfigYN) blockSize[l] = tempblockSize[l];
			else blockSize[l] = defaultblockSize[l];

			if (MAXITERATIONconfigYN) MAXITERATION[l] = tempMAXITERATION[l];
			else MAXITERATION[l] = defaultMAXITERATION[l];

			ANNerror[l] = defaultANNerror[l];
		}
	}

	//crop to fit multi-level
	int multileveltimes = pow(2, MULTIRES - 1);
	if (dimension % multileveltimes != 0) {
		dimension = dimension / multileveltimes * multileveltimes;
		cout << endl << "TIs are croped to " << dimension << " to fit multi-grid";
		for (int n = 0; n < XY.size(); n++) {
			Mat tempMat;
			XY[n](Rect(0, 0, dimension, dimension)).copyTo(tempMat);
			tempMat.copyTo(XY[n]);
			XZ[n](Rect(0, 0, dimension, dimension)).copyTo(tempMat);
			tempMat.copyTo(XZ[n]);
			YZ[n](Rect(0, 0, dimension, dimension)).copyTo(tempMat);
			tempMat.copyTo(YZ[n]);
		}
	}

	//! add additional space, later will crop to original size, to deal with Toroidal problem
	outputsizeatlastlevel += pow(2, MULTIRES - 1) * blockSize[0];
	
	// allocate TIsize, OUTsize
	TIsize.resize(MULTIRES);	TIsize[MULTIRES - 1] = XY[0].cols;
	OUTsize.resize(MULTIRES);	OUTsize[MULTIRES - 1] = outputsizeatlastlevel;
	m_volume.resize(MULTIRES);
}

bool DoPAR::loadExemplar() {
	//////exemplar_x --> YZ, exemplar_y --> ZX, exemplar_z --> XY
	//////using imagej, XY slice is XY, ememplar_z
	//////ZX slice can be attained by: 1. reslice top + flip virtical 2. then rotate 90 degrees left
	//////YZ slice is done by: reslice left
	//------in imagej:
	//exemplar_x -> XY
	//exemplar_y -> XZ
	//exemplar_z -> YZ

	//------------ Load Mat ---------------------
	vector<Mat> MatlistXY, MatlistXZ, MatlistYZ;
	LoadtoMat(MatlistXY, FNameXY);
	LoadtoMat(MatlistXZ, FNameXZ);
	LoadtoMat(MatlistYZ, FNameYZ);
	for (int s = 0; s < FNameXY.size(); s++) {
		//check X,Y,Z TI dimension equal
		if (!(MatlistXY[s].rows == MatlistXZ[s].rows && MatlistXY[s].rows == MatlistYZ[s].rows
			&& MatlistXY[s].cols == MatlistXZ[s].cols && MatlistXY[s].cols == MatlistYZ[s].cols)) {
			cout << endl << "X,Y,Z TIs dimension not equal, quit"; _getch(); exit(0);
		}
		//check X,Y,Z TI the same or not, if yes need to switch row,col
		if (FNameXY[s] == FNameXZ[s] && FNameXY[s] == FNameYZ[s]) {
			cout << endl << "Same TI for XYZ, flip 2nd TI by switching X,Y: "<< FNameXY[s];
			flip(MatlistXZ[s], MatlistXZ[s], 0);
			flip(MatlistXZ[s], MatlistXZ[s], 1);
		}
	}
	// check TI is grayscale or binary
	if (checkTIbinary(MatlistXY) && checkTIbinary(MatlistXZ) && checkTIbinary(MatlistYZ)) {
		cout << endl << "TI(s) binary, enable distance map transformation";
		DMtransformYN = true;
	}
	else {
		cout << endl << "TI(s) NOT binary, disable distance map transformation";
		cout << endl << "Not recommended to use greyscale TI, continue??";
		_getch();
		DMtransformYN = false; 		
		//ColorHis_ON = false; 		
		HisEqYN = false;
	}
	
	//------------ decide parameters based on TI dimension
	initTIbasedparameters(MatlistXY, MatlistXZ, MatlistYZ);

	//------------ Assign vectors ---------------------
	allocateVectors();

	//------------ DM transform ---------------------
	invertpaddingDMtransform(MatlistXY, MatlistXZ, MatlistYZ, TIs_XY[MULTIRES-1], TIs_XZ[MULTIRES - 1], TIs_YZ[MULTIRES - 1]);

	//------------ multi-level resize -----------------
	if (MULTIRES > 1) {
		cout << endl << "use Lancoz filter to resize.";		//tested: better than Gaussian and INTER_AREA
		for (int l = MULTIRES - 2; l >= 0; --l) {
			for (int n = 0; n < MultiTIsNum; n++) {
				Mat next_XY, next_XZ, next_YZ;
				double ratio = pow(0.5, (MULTIRES - 1 - l));
				resize(MatlistXY[n], next_XY, Size(), ratio, ratio, INTER_LANCZOS4);
				resize(MatlistXZ[n], next_XZ, Size(), ratio, ratio, INTER_LANCZOS4);
				resize(MatlistYZ[n], next_YZ, Size(), ratio, ratio, INTER_LANCZOS4);
				// convert mat to vector
				TIs_XY[l][n] = MatToVec_8UTofloat(next_XY);
				TIs_XZ[l][n] = MatToVec_8UTofloat(next_XZ);
				TIs_YZ[l][n] = MatToVec_8UTofloat(next_YZ);
				//imwrite("L"+to_string(l)+".png", next_XY); _getch();
			}
		}
	}

	//------------ histogram equalization -----------------
	Solid_Upper_noeq = Solid_Upper[MULTIRES - 1];
	Pore_Lower_noeq = Pore_Lower[MULTIRES - 1];
	if (DMtransformYN && HisEqYN) {
		cout << endl << "apply histogram equalization";
		for (int l = MULTIRES - 1; l >= 0; --l) {
			equalizeHistograms(l, TIs_XY[l], TIs_XZ[l], TIs_YZ[l]);
		}
	}
	if (DMtransformYN) {
		computeporosityrequired();
	}

	//------------ Analyze PCA,Pattern entropy,Generate DM TI -----------------	
	analyze();
	
	//Mat temp = VecToMat_floatTo8U(TIs_XZ[MULTIRES - 1][MultiTIsNum-1], TIsize[MULTIRES - 1]);	//tested:right
	//imwrite("eqL2.png", temp);	_getch();

	return true;
}

// ================ allocation ==========
void DoPAR::allocateVectors() {
	//TIs, m_volume
	TIs_XY.resize(MULTIRES);	TIs_XZ.resize(MULTIRES);	TIs_YZ.resize(MULTIRES);
	for (int level = MULTIRES - 1; level >= 0; --level) {
		TIsize[level] = TIsize[MULTIRES - 1] / pow(2, MULTIRES - 1 - level);
		OUTsize[level] = OUTsize[MULTIRES - 1] / pow(2, MULTIRES - 1 - level);
		if (SIM2D_YN) m_volume[level].resize(OUTsize[level] * OUTsize[level]);
		else m_volume[level].resize(OUTsize[level] * OUTsize[level] * OUTsize[level]);
		
		TIs_XY[level].resize(MultiTIsNum);	
		TIs_XZ[level].resize(MultiTIsNum);
		TIs_YZ[level].resize(MultiTIsNum);
		long ti2dsize = TIsize[level] * TIsize[level];
		for (int i = 0; i < MultiTIsNum; i++) {
			TIs_XY[level][i].resize(ti2dsize, 0);
			TIs_XZ[level][i].resize(ti2dsize, 0);
			TIs_YZ[level][i].resize(ti2dsize, 0);
		}
	}
	//DMap
	Solid_Upper.resize(MULTIRES);	Pore_Upper.resize(MULTIRES);	Pore_Lower.resize(MULTIRES);
	porosity_required.resize(MULTIRES);
	//K-coherence
	isUnchanged_x.resize(MULTIRES); isUnchanged_y.resize(MULTIRES);		isUnchanged_z.resize(MULTIRES);
	nearestIdx_x.resize(MULTIRES);	nearestIdx_y.resize(MULTIRES);		nearestIdx_z.resize(MULTIRES);
	nearestWeight_x.resize(MULTIRES); nearestWeight_y.resize(MULTIRES); nearestWeight_z.resize(MULTIRES);
	Origin_x.resize(MULTIRES);		Origin_y.resize(MULTIRES);			Origin_z.resize(MULTIRES);
	//Histogram
	IndexHis_x.resize(MULTIRES);	IndexHis_y.resize(MULTIRES);		IndexHis_z.resize(MULTIRES);
	PosHis.resize(MULTIRES);
	SelectedPos.resize(MULTIRES);
	avgIndexHis.resize(MULTIRES); avgPosHis.resize(MULTIRES);
	ColorHis_exemplar.resize(MULTIRES);
	ColorHis_synthesis.resize(MULTIRES);

	for (int level = 0; level < MULTIRES; ++level) {
		size_idx Soutput = OUTsize[level] * OUTsize[level] * OUTsize[level];
		if (SIM2D_YN) Soutput = OUTsize[level] * OUTsize[level];
		isUnchanged_x[level].assign(Soutput, false);		isUnchanged_y[level].assign(Soutput, false);		isUnchanged_z[level].assign(Soutput, false);
		nearestIdx_x[level].assign(Soutput, -1);		nearestIdx_y[level].assign(Soutput, -1);		nearestIdx_z[level].assign(Soutput, -1);
		nearestWeight_x[level].assign(Soutput, min_dist);	nearestWeight_y[level].assign(Soutput, min_dist);	nearestWeight_z[level].assign(Soutput, min_dist);
		Origin_x[level].assign(Soutput, -1);			Origin_y[level].assign(Soutput, -1);			Origin_z[level].assign(Soutput, -1);
		SelectedPos[level].assign(Soutput, -1);

		size_idx Sti = TIsize[level] * TIsize[level];
		//PosHis size=3*TI
		PosHis[level].assign(Sti * 3, 0);
		//sparse grid IndexHis
		IndexHis_x[level].assign(Sti * 0.25, 0);		IndexHis_y[level].assign(Sti * 0.25, 0);		IndexHis_z[level].assign(Sti * 0.25, 0);
	}
}

bool DoPAR::loadVolume() {
	cout << endl << "Use Random initial.";
	InitRandomVolume(0);
	return true;
}
//randomly assign Origin & color
void DoPAR::InitRandomVolume(int level) {
	size_idx OUTsize_ = OUTsize[level];
	size_idx OUTsize2d_ = OUTsize_* OUTsize_;
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx TIsize2d_ = TIsize_* TIsize_;
	size_idx Size = OUTsize2d_* OUTsize_;
	if (SIM2D_YN) Size = OUTsize2d_;
	
	uniform_int_distribution<> distr(0, (TIsize2d_ * MultiTIsNum) - 1);	// define the range [a,b]

	vector<size_idx> randomidx2d_TIs(3), TIn(3), idx2d(3);
	for (size_idx xyz = 0; xyz < Size; ++xyz) {
		randomidx2d_TIs[0] = distr(mersennetwistergenerator);
		randomidx2d_TIs[1] = distr(mersennetwistergenerator);
		randomidx2d_TIs[2] = distr(mersennetwistergenerator);

		Origin_x[level][xyz] = randomidx2d_TIs[0];
		isUnchanged_x[level][xyz] = false;
		TIn[0] = randomidx2d_TIs[0] / TIsize2d_;
		idx2d[0] = randomidx2d_TIs[0] % TIsize2d_;
		if (SIM2D_YN) m_volume[level][xyz] = TIs_XY[level][TIn[0]][idx2d[0]];
		if (!SIM2D_YN) {			
			Origin_y[level][xyz] = randomidx2d_TIs[1];
			isUnchanged_y[level][xyz] = false;
			TIn[1] = randomidx2d_TIs[1] / TIsize2d_;
			idx2d[1] = randomidx2d_TIs[1] % TIsize2d_;

			Origin_z[level][xyz] = randomidx2d_TIs[2];
			isUnchanged_z[level][xyz] = false;
			TIn[2] = randomidx2d_TIs[2] / TIsize2d_;
			idx2d[2] = randomidx2d_TIs[2] % TIsize2d_;

			m_volume[level][xyz] = round( (TIs_XY[level][TIn[0]][idx2d[0]] + TIs_XZ[level][TIn[1]][idx2d[1]] + TIs_YZ[level][TIn[2]][idx2d[2]]) / 3.0);
		}

		//if (Origin_x[level][xyz] >= MultiTIsNum*TIsize2d_ || Origin_y[level][xyz] >= MultiTIsNum*TIsize2d_ || Origin_z[level][xyz] >= MultiTIsNum*TIsize2d_) {
		//	printf("origin %d %d %d", Origin_x[level][xyz], Origin_y[level][xyz], Origin_z[level][xyz]);
		//}
	}
}

//load 3D model and assign fixed 

//vector<uchar> DoPAR::load3Dmodel(const char* filename){
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
//		_getch(); exit(0);
//	}
//
//	return fileData;
//}
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

// ============== distance map ===============
void binaryChar(vector<short>& DMap, vector<char>& Binarised, short threshold = 110) {
	//input  vector<short> DMap		//output vector<char>Binarised
	for (long i = 0; i < DMap.size(); i++) {
		if (DMap[i] <= threshold) Binarised[i] = 0;
		else Binarised[i] = 1;
	}
}
void binaryUchar(vector<short>& DMap, vector<uchar>& Binarised, short threshold) {
	//input  vector<short> DMap		//output vector<uchar>Binarised
	for (long i = 0; i < DMap.size(); i++) {
		if (DMap[i] <= threshold) Binarised[i] = 0;
		else Binarised[i] = 255;
	}
}
void binaryUchar(vector<uchar>& model, short threshold) {

	for (size_idx i = 0; i < model.size(); i++) {
		if (model[i] <= threshold) model[i] = 0;
		else model[i] = 255;
	}

}
vector<unsigned short> BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg) {
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
vector<short> GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN) {
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
template<typename T>
vector<short> GetDMap_Euclidean(vector<T>& vect, short dimension) {
	assert(vect.size() == dimension*dimension);

	vector<short> vecshort(vect.begin(), vect.end());
	vector<char> vecchar(vect.size());
	
	binaryChar(vecshort, vecchar);
	vecshort = GetDMap(dimension, dimension, 1, vecchar, 2, true);
	
	for (long idx = 0; idx < vecshort.size(); ++idx) {			//get Euclidean distance
		vecshort[idx] = (vecshort[idx] / abs(vecshort[idx])) * round(sqrt(abs(vecshort[idx])));
	}

	return vecshort;
}

void DoPAR::transformDMs(vector<vector<size_color> >& listXY, vector<vector<size_color> >& listXZ, vector<vector<size_color> >& listYZ) {
	//input lists of vector<size_color>, output DM transformed lists of vector<size_color>
	int TIsize_ = sqrt(listXY[0].size());
	assert(TIsize_ == TIsize[MULTIRES - 1]);

	vector<vector<short> > DMlist_XY(listXY.size()), DMlist_XZ(listXZ.size()), DMlist_YZ(listYZ.size());
	short minall(255), maxall(0), min_XY(255), min_XZ(255), min_YZ(255), max_XY(0), max_XZ(0), max_YZ(0);
	
	// get vector<short>DM
	for (int i = 0; i < MultiTIsNum; i++) {
		DMlist_XY[i] = GetDMap_Euclidean(listXY[i], TIsize_);
		DMlist_XZ[i] = GetDMap_Euclidean(listXZ[i], TIsize_);
		DMlist_YZ[i] = GetDMap_Euclidean(listYZ[i], TIsize_);
		// get min, max
		min_XY = min(min_XY, *min_element(DMlist_XY[i].begin(), DMlist_XY[i].end()));
		max_XY = max(max_XY, *max_element(DMlist_XY[i].begin(), DMlist_XY[i].end()));
		min_XZ = min(min_XZ, *min_element(DMlist_XZ[i].begin(), DMlist_XZ[i].end()));
		max_XZ = max(max_XZ, *max_element(DMlist_XZ[i].begin(), DMlist_XZ[i].end()));
		min_YZ = min(min_YZ, *min_element(DMlist_YZ[i].begin(), DMlist_YZ[i].end()));
		max_YZ = max(max_YZ, *max_element(DMlist_YZ[i].begin(), DMlist_YZ[i].end()));
	}
	
	// calculate DM range
	if (!HisEqYN) {
		minall = max(min_XY, max(min_XZ, min_YZ));
		maxall = min(max_XY, min(max_XZ, max_YZ));
	}
	else {
		minall = min(min_XY, min(min_XZ, min_YZ));
		maxall = max(max_XY, max(max_XZ, max_YZ));
	}
	double scale = 1.0;
	if (maxall - minall > 255) { 
		cout << endl << "resize value range to 0-255!"; 
		scale = 255.0 / (maxall - minall);
	}

	// shift short to 0-255 and assign to size_color
	for (int n = 0; n < MultiTIsNum; n++) {
		for (long i = 0; i < TIsize_*TIsize_; i++) {
			if (DMlist_XY[n][i] < 0) DMlist_XY[n][i] = scale * (max(0, DMlist_XY[n][i] - minall) + 1);
			else DMlist_XY[n][i] = scale * (min(DMlist_XY[n][i] - minall, maxall - minall));
			
			if (DMlist_XZ[n][i] < 0) DMlist_XZ[n][i] = scale * (max(0, DMlist_XZ[n][i] - minall) + 1);
			else DMlist_XZ[n][i] = scale * (min(DMlist_XZ[n][i] - minall, maxall - minall));
			
			if (DMlist_YZ[n][i] < 0) DMlist_YZ[n][i] = scale * (max(0, DMlist_YZ[n][i] - minall) + 1);
			else DMlist_YZ[n][i] = scale * (min(DMlist_YZ[n][i] - minall, maxall - minall));
		}
		// convert vector<short> to vector<size_color>
		listXY[n] = vector<size_color>(DMlist_XY[n].begin(), DMlist_XY[n].end());
		listXZ[n] = vector<size_color>(DMlist_XZ[n].begin(), DMlist_XZ[n].end());
		listYZ[n] = vector<size_color>(DMlist_YZ[n].begin(), DMlist_YZ[n].end());
	}
	// update
	Solid_Upper[MULTIRES-1] = -minall * scale;
	Pore_Upper[MULTIRES - 1] = (maxall - minall) * scale;	//total bins
	Pore_Lower[MULTIRES - 1] = Solid_Upper[MULTIRES - 1] + 1;
}

void DoPAR::invertpaddingDMtransform(vector<Mat>& XY, vector<Mat>& XZ, vector<Mat>& YZ, vector<vector<size_color> >& TIsXY, vector<vector<size_color> >& TIsXZ, vector<vector<size_color> >& TIsYZ) {
	// input Mat lists; output invert padded DM transformed vector lists
	if (!DMtransformYN) return;

	vector<vector<size_color> > paddedTIs_x(MultiTIsNum), paddedTIs_y(MultiTIsNum), paddedTIs_z(MultiTIsNum);
	int dimension = XY[0].cols;
	int padded_TIsize = dimension + 2;
	
	//--------- add invert padding border to vector lists
	for (int n = 0; n < MultiTIsNum; n++) {	
		copyMakeBorder(XY[n], XY[n], 1, 1, 1, 1, BORDER_REPLICATE);
		copyMakeBorder(XZ[n], XZ[n], 1, 1, 1, 1, BORDER_REPLICATE);
		copyMakeBorder(YZ[n], YZ[n], 1, 1, 1, 1, BORDER_REPLICATE);
		// mat to vector
		paddedTIs_x[n].resize(padded_TIsize*padded_TIsize);
		paddedTIs_y[n].resize(padded_TIsize*padded_TIsize);
		paddedTIs_z[n].resize(padded_TIsize*padded_TIsize);
		paddedTIs_x[n] = MatToVec_8UTofloat(XY[n]);
		paddedTIs_y[n] = MatToVec_8UTofloat(XZ[n]);
		paddedTIs_z[n] = MatToVec_8UTofloat(YZ[n]);
		// invert border	
		for (int x = 0; x < padded_TIsize; x++) {
			int idx1 = x, idx2 = (padded_TIsize - 1)*padded_TIsize + x;
			paddedTIs_x[n][idx1] = (255 - paddedTIs_x[n][idx1]); paddedTIs_x[n][idx2] = (255 - paddedTIs_x[n][idx2]);
			paddedTIs_y[n][idx1] = (255 - paddedTIs_y[n][idx1]); paddedTIs_y[n][idx2] = (255 - paddedTIs_y[n][idx2]);
			paddedTIs_z[n][idx1] = (255 - paddedTIs_z[n][idx1]); paddedTIs_z[n][idx2] = (255 - paddedTIs_z[n][idx2]);
		}
		for (int y = 0; y < padded_TIsize; y++) {
			int idx1 = y*padded_TIsize, idx2 = (padded_TIsize - 1) + y*padded_TIsize;
			paddedTIs_x[n][idx1] = (255 - paddedTIs_x[n][idx1]); paddedTIs_x[n][idx2] = (255 - paddedTIs_x[n][idx2]);
			paddedTIs_y[n][idx1] = (255 - paddedTIs_y[n][idx1]); paddedTIs_y[n][idx2] = (255 - paddedTIs_y[n][idx2]);
			paddedTIs_z[n][idx1] = (255 - paddedTIs_z[n][idx1]); paddedTIs_z[n][idx2] = (255 - paddedTIs_z[n][idx2]);
		}
	}

	//---------- Distance map transformation

	transformDMs(paddedTIs_x, paddedTIs_y, paddedTIs_z);

	//---------- vector to Mat, then crop, then Mat to vector again
	Mat tempmat, tempcropped;
	for (int n = 0; n < MultiTIsNum; n++) {
		// vector to Mat, need the same data type!
		tempmat = VecToMat_floatTo8U(paddedTIs_x[n], padded_TIsize);
		// crop Mat
		tempcropped = tempmat(Rect(1, 1, dimension, dimension));
		tempcropped.copyTo(XY[n]);
		// Mat to vector
		TIsXY[n] = MatToVec_8UTofloat(XY[n]);

		tempmat = VecToMat_floatTo8U(paddedTIs_y[n], padded_TIsize);
		tempcropped = tempmat(Rect(1, 1, dimension, dimension));
		tempcropped.copyTo(XZ[n]);
		TIsXZ[n] = MatToVec_8UTofloat(XZ[n]);

		tempmat = VecToMat_floatTo8U(paddedTIs_z[n], padded_TIsize);
		tempcropped = tempmat(Rect(1, 1, dimension, dimension));
		tempcropped.copyTo(YZ[n]);
		TIsYZ[n] = MatToVec_8UTofloat(YZ[n]);
	}

	//imwrite("xy4.png", XY[4]);	_getch();
}

void DoPAR::equalizeHistograms(int level, vector<vector<size_color>>& TIsXY, vector<vector<size_color>>& TIsXZ, vector<vector<size_color>>& TIsYZ) {
	
	long TI2dsize = TIsXY[0].size(), total = TI2dsize *MultiTIsNum;
	if (!SIM2D_YN) 	total *=3;
	
	// Compute accumulate histogram
	vector<long> hist(256, 0);
	size_color maxv(0);
	for (int n = 0; n < MultiTIsNum; n++) {
		maxv = max(maxv, *max_element(TIsXY[n].begin(), TIsXY[n].end()));
		if (!SIM2D_YN) maxv = max(maxv, max(*max_element(TIsXZ[n].begin(), TIsXZ[n].end()), *max_element(TIsYZ[n].begin(), TIsYZ[n].end())));		
		for (long i = 0; i < TI2dsize; ++i) {
			hist[TIsXY[n][i]]++;
			if (SIM2D_YN) continue;
			hist[TIsXZ[n][i]]++;
			hist[TIsYZ[n][i]]++;
		}
	}

	// Compute scale
	uchar n_bins = maxv + 1;
	double scale;
	scale = min(255.0, n_bins - 1.0) / total;
	Pore_Upper[level] = min(255, n_bins - 1);

	// Build LUT from accumulate histrogram
	vector<uchar> lut(n_bins, 0);
	long sum = 0;
	for (uchar i = 0; i < n_bins; ++i) {
		sum += hist[i];
		//round((sum - hist[0])/(total-hist[0])*(bins-1));			//here hist[0]=0		
		lut[i] = min((uchar)maxv, (uchar)round(sum * scale));		// the value is saturated in range [0, max_val]
	}

	// equalization without press
	for (int n = 0; n < MultiTIsNum; n++) {
		for (long i = 0; i < TI2dsize; ++i) {
			TIsXY[n][i] = lut[TIsXY[n][i]];
			if (SIM2D_YN) continue;
			TIsXZ[n][i] = lut[TIsXZ[n][i]];
			TIsYZ[n][i] = lut[TIsYZ[n][i]];
		}
	}

	// update
	Solid_Upper[level] = lut[Solid_Upper_noeq];
	Pore_Lower[level] = lut[Pore_Lower_noeq];
}

// ================ analysis ===========
void DoPAR::computeporosityrequired() {
	for (int l = MULTIRES - 1; l >= 0; --l) {
		long porecount(0);
		uchar solidup = Solid_Upper[MULTIRES - 1];
		
		for (int n = 0; n < MultiTIsNum; n++) {
			porecount += count_if(TIs_XY[l][n].begin(), TIs_XY[l][n].end(), [solidup](uchar i) {return i> solidup; });
			if (!SIM2D_YN) {
				porecount += count_if(TIs_XZ[l][n].begin(), TIs_XZ[l][n].end(), [solidup](uchar i) {return i> solidup; });
				porecount += count_if(TIs_YZ[l][n].begin(), TIs_YZ[l][n].end(), [solidup](uchar i) {return i> solidup; });
			}
		}

		if (SIM2D_YN) 	porosity_required[l] = porecount*1.0f / (TIs_XY[l][0].size());
		else porosity_required[l] = porecount*1.0f / (3 * TIs_XY[l][0].size());
		
		if (HisEqYN || l == MULTIRES - 1) cout << endl << "level" << l << " Solid_Upper=" << (int)Solid_Upper[l] << " Pore_Lower=" << (int)Pore_Lower[l] << " porosity=" << porosity_required[l];
	}
}

void DoPAR::analyze() {
	//if (GenerateTI) testPCA();

	if (PatternEntropyAnalysisYN) {
		//double entropy;
		//if (!DMtransformYN) cout << endl << "noDM:";
		//else cout << endl << "DM:";

		//for (int templatesize = 4; templatesize < 34; templatesize += 2) {
		//	if (!DMtransformYN) {
		//		patternentropyanalysis(templatesize, matyz, entropy);
		//	}
		//	else {
		//		Mat DM1;
		//		if (HisEqYN) {	//DM 8UC1, uchar
		//			vector<uchar> tmpchar;
		//			DM1 = Mat(TIsize[MULTIRES - 1], TIsize[MULTIRES - 1], CV_8UC1);
		//			tmpchar = vector<uchar>(m_exemplar_x[MULTIRES - 1].begin(), m_exemplar_x[MULTIRES - 1].end());
		//			DM1 = Mat(tmpchar, true).reshape(1, DM1.rows);
		//		}
		//		else {	//DM 32FC1, float. matchTemplate only accept 8U or 32F
		//			vector<float> tempfloat;
		//			DM1 = Mat(TIsize[MULTIRES - 1], TIsize[MULTIRES - 1], CV_32FC1);
		//			tempfloat = vector<float>(m_exemplar_x[MULTIRES - 1].begin(), m_exemplar_x[MULTIRES - 1].end());
		//			DM1 = Mat(tempfloat, true).reshape(1, DM1.rows);
		//		}
		//		patternentropyanalysis(templatesize, DM1, entropy);
		//	}
		//}
		//_getch();
	}

	if (GenerateTI && DMtransformYN) {						//Generate DM TI
		//ostringstream name;
		//for (int lv = MULTIRES - 1; lv >= 0; --lv) {
		//	int TEXSIZE_ = TIsize[lv];
		//	Mat DM1, DM2, DM3;

		//	//if (HisEqYN) {
		//	vector<uchar> tmpchar;
		//	DM1 = Mat(TEXSIZE_, TEXSIZE_, CV_8UC1);
		//	tmpchar = vector<uchar>(m_exemplar_x[lv].begin(), m_exemplar_x[lv].end());
		//	DM1 = Mat(tmpchar, true).reshape(1, DM1.rows);// vector to mat, need the same data type!
		//	name << "DM1_S" << (short)Solid_Upper[lv] << "_L" << to_string(lv) << ".png";
		//	imwrite(name.str(), DM1);	name.str("");

		//	if (!SIM2D_YN) {
		//		//if (HisEqYN) {
		//		vector<uchar> tmpchar;
		//		DM2 = Mat(TEXSIZE_, TEXSIZE_, CV_8UC1);
		//		tmpchar = vector<uchar>(m_exemplar_y[lv].begin(), m_exemplar_y[lv].end());
		//		DM2 = Mat(tmpchar, true).reshape(1, DM2.rows);
		//		name << "DM2_S" << (short)Solid_Upper[lv] << "_L" << to_string(lv) << ".png";
		//		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), DM2);
		//		name.str("");

		//		//if (HisEqYN) {
		//		//vector<uchar> tmpchar;
		//		DM3 = Mat(TEXSIZE_, TEXSIZE_, CV_8UC1);
		//		tmpchar = vector<uchar>(m_exemplar_z[lv].begin(), m_exemplar_z[lv].end());
		//		DM3 = Mat(tmpchar, true).reshape(1, DM3.rows);
		//		name << "DM3_S" << (short)Solid_Upper[lv] << "_L" << to_string(lv) << ".png";
		//		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(name.str(), DM3);
		//		name.str("");
		//	}
		//}
		//cout << endl << "output TI.";	
	}
}

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
//void DoPAR::testPCA() {
//	//test PCA TI and back-project
//	int level = MULTIRES - 1;
//	const int TIsize_ = TIsize[level];
//	int N = 5;		//11*11,256->246
//	int D_NEIGHBOR = (1 + 2 * N)*(1 + 2 * N);
//	double PCA_RATIO_VARIANCE = 0.99;
//
//	cout << endl << "input PCA_RATIO:";
//	cin >> PCA_RATIO_VARIANCE;
//	if (PCA_RATIO_VARIANCE>1.0) { cout << endl << "wrong value, use 99%"; PCA_RATIO_VARIANCE = 0.99; }
//	if (PCA_RATIO_VARIANCE <= 0) { cout << endl << "no PCA"; return; }
//	int MINDIMS = 2;
//	int MAXDIMS = 20;
//	cout << endl << "input MINDIMS, MAXDIMS:";
//	cin >> MINDIMS >> MAXDIMS;
//	if (MINDIMS < 2) { cout << endl << "MINDIMS<2, use 2"; MINDIMS = 2; }
//	if (MAXDIMS > D_NEIGHBOR) { cout << endl << "MINDIMS<D_NEIGHBOR, use D_NEIGHBOR"; MAXDIMS = D_NEIGHBOR; }
//
//
//	int sizeneighbor = D_NEIGHBOR * (TIsize_ - 2 * N) * (TIsize_ - 2 * N);
//	vector<size_color> m_neighbor_x(sizeneighbor, 0), m_neighbor_y(sizeneighbor, 0), m_neighbor_z(sizeneighbor, 0);
//	CvMat* mp_neighbor_pca_average_x(NULL); CvMat* mp_neighbor_pca_average_y(NULL); CvMat* mp_neighbor_pca_average_z(NULL);
//	CvMat* mp_neighbor_pca_projected_x(NULL); CvMat* mp_neighbor_pca_projected_y(NULL); CvMat* mp_neighbor_pca_projected_z(NULL);
//	CvMat* mp_neighbor_pca_eigenvec_x(NULL); CvMat* mp_neighbor_pca_eigenvec_y(NULL); CvMat* mp_neighbor_pca_eigenvec_z(NULL);
//
//	int numData = (TIsize_ - 2 * N) * (TIsize_ - 2 * N);
//	CvMat* p_source_x = cvCreateMat(numData, D_NEIGHBOR, CV_32F);	//rows='area' numData, cols=dimension (Neighbour size)
//	CvMat* p_source_y = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
//	CvMat* p_source_z = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
//	int row = 0;
//	for (int v = N; v < TIsize_ - N; ++v) {
//		for (int u = N; u < TIsize_ - N; ++u) {
//			int col = 0;
//			for (int dv = -N; dv <= N; ++dv) {
//				for (int du = -N; du <= N; ++du) {
//					ANNidx index = (TIsize_ * (v + dv) + u + du);
//					cvmSet(p_source_x, row, col, m_exemplar_x[level][index]);	//set p_source_x(row,col) to m_examplar_x(idx)
//					cvmSet(p_source_y, row, col, m_exemplar_y[level][index]);
//					cvmSet(p_source_z, row, col, m_exemplar_z[level][index]);
//
//					m_neighbor_x[D_NEIGHBOR * row + col] = m_exemplar_x[level][index];
//					m_neighbor_y[D_NEIGHBOR * row + col] = m_exemplar_y[level][index];
//					m_neighbor_z[D_NEIGHBOR * row + col] = m_exemplar_z[level][index];
//					++col;
//				}
//			}
//			++row;
//		}
//	}
//
//	// PCA calculation (obtain all eigenvectors of the input covariance matrix)
//	////////每一行表示一个样本
//	//////CvMat* pData = cvCreateMat( 总的样本数, 每个样本的维数, CV_32FC1 );
//	if (mp_neighbor_pca_average_x != NULL) cvReleaseMat(&mp_neighbor_pca_average_x);
//	if (mp_neighbor_pca_average_y != NULL) cvReleaseMat(&mp_neighbor_pca_average_y);
//	if (mp_neighbor_pca_average_z != NULL) cvReleaseMat(&mp_neighbor_pca_average_z);
//	//CvMat* pMean = cvCreateMat(1, 样本的维数, CV_32FC1);
//	mp_neighbor_pca_average_x = cvCreateMat(1, D_NEIGHBOR, CV_32F);
//	mp_neighbor_pca_average_y = cvCreateMat(1, D_NEIGHBOR, CV_32F);
//	mp_neighbor_pca_average_z = cvCreateMat(1, D_NEIGHBOR, CV_32F);
//	//pEigVals中的每个数表示一个特征值
//	//CvMat* pEigVals = cvCreateMat(1, min(总的样本数,样本的维数), CV_32FC1);
//	CvMat* p_eigenValues_x = cvCreateMat(1, D_NEIGHBOR, CV_32F);
//	CvMat* p_eigenValues_y = cvCreateMat(1, D_NEIGHBOR, CV_32F);
//	CvMat* p_eigenValues_z = cvCreateMat(1, D_NEIGHBOR, CV_32F);
//	//每一行表示一个特征向量
//	//CvMat* pEigVecs = cvCreateMat( min(总的样本数,样本的维数), 样本的维数, CV_32FC1);
//	CvMat* p_eigenVectors_all_x = cvCreateMat(D_NEIGHBOR, D_NEIGHBOR, CV_32F);
//	CvMat* p_eigenVectors_all_y = cvCreateMat(D_NEIGHBOR, D_NEIGHBOR, CV_32F);
//	CvMat* p_eigenVectors_all_z = cvCreateMat(D_NEIGHBOR, D_NEIGHBOR, CV_32F);
//	//PCA处理,计算出平均向量pMean,特征值pEigVals和特征向量pEigVecs
//	//cvCalcPCA(pData, pMean, pEigVals, pEigVecs, CV_PCA_DATA_AS_ROW);
//	//now have better function //PCA pca(data, mean, PCA::DATA_AS_ROW, 0.95);
//	cvCalcPCA(p_source_x, mp_neighbor_pca_average_x, p_eigenValues_x, p_eigenVectors_all_x, CV_PCA_DATA_AS_ROW);
//	cvCalcPCA(p_source_y, mp_neighbor_pca_average_y, p_eigenValues_y, p_eigenVectors_all_y, CV_PCA_DATA_AS_ROW);
//	cvCalcPCA(p_source_z, mp_neighbor_pca_average_z, p_eigenValues_z, p_eigenVectors_all_z, CV_PCA_DATA_AS_ROW);
//	// Decide amount of dimensionality reduction
//	double contribution_total_x = 0;
//	double contribution_total_y = 0;
//	double contribution_total_z = 0;
//	for (int i = 0; i < D_NEIGHBOR; ++i) {
//		contribution_total_x += cvmGet(p_eigenValues_x, 0, i);
//		contribution_total_y += cvmGet(p_eigenValues_y, 0, i);
//		contribution_total_z += cvmGet(p_eigenValues_z, 0, i);
//	}
//
//	int dimPCA_x = 0;
//	int dimPCA_y = 0;
//	int dimPCA_z = 0;
//
//	double contribution_acc_x = 0;
//	double contribution_acc_y = 0;
//	double contribution_acc_z = 0;
//	for (int i = 0; i < D_NEIGHBOR; ++i) {
//		double ratio_x = contribution_acc_x / contribution_total_x;
//		double ratio_y = contribution_acc_y / contribution_total_y;
//		double ratio_z = contribution_acc_z / contribution_total_z;
//		if (ratio_x < PCA_RATIO_VARIANCE || dimPCA_x < MINDIMS) {
//			contribution_acc_x += cvmGet(p_eigenValues_x, 0, i);
//			++dimPCA_x;
//		}
//		if (ratio_y < PCA_RATIO_VARIANCE) {
//			contribution_acc_y += cvmGet(p_eigenValues_y, 0, i);
//			++dimPCA_y;
//		}
//		if (ratio_z < PCA_RATIO_VARIANCE) {
//			contribution_acc_z += cvmGet(p_eigenValues_z, 0, i);
//			++dimPCA_z;
//		}
//		if (PCA_RATIO_VARIANCE <= ratio_x && PCA_RATIO_VARIANCE <= ratio_y && PCA_RATIO_VARIANCE <= ratio_z
//			&& dimPCA_x >= MINDIMS) break;
//		if (dimPCA_x >= MAXDIMS) break;
//	}
//
//	cout << endl;
//	printf("PCA reduction (x): %d -> %d\n", D_NEIGHBOR, dimPCA_x);
//	printf("PCA reduction (y): %d -> %d\n", D_NEIGHBOR, dimPCA_y);
//	printf("PCA reduction (z): %d -> %d\n", D_NEIGHBOR, dimPCA_z);
//
//
//	// Trim total eigenvectors into partial eigenvectors
//	if (mp_neighbor_pca_eigenvec_x != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x);
//	if (mp_neighbor_pca_eigenvec_y != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y);
//	if (mp_neighbor_pca_eigenvec_z != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z);
//	mp_neighbor_pca_eigenvec_x = cvCreateMat(dimPCA_x, D_NEIGHBOR, CV_32F);
//	mp_neighbor_pca_eigenvec_y = cvCreateMat(dimPCA_y, D_NEIGHBOR, CV_32F);
//	mp_neighbor_pca_eigenvec_z = cvCreateMat(dimPCA_z, D_NEIGHBOR, CV_32F);
//	memcpy(mp_neighbor_pca_eigenvec_x->data.fl, p_eigenVectors_all_x->data.fl, sizeof(ANNcoord)* dimPCA_x * D_NEIGHBOR);
//	memcpy(mp_neighbor_pca_eigenvec_y->data.fl, p_eigenVectors_all_y->data.fl, sizeof(ANNcoord)* dimPCA_y * D_NEIGHBOR);
//	memcpy(mp_neighbor_pca_eigenvec_z->data.fl, p_eigenVectors_all_z->data.fl, sizeof(ANNcoord)* dimPCA_z * D_NEIGHBOR);
//	// PCA projection
//	//CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
//	if (mp_neighbor_pca_projected_x != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x);
//	if (mp_neighbor_pca_projected_y != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y);
//	if (mp_neighbor_pca_projected_z != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z);
//	//选出前P个特征向量(主成份),然后投影,结果保存在pResult中，pResult中包含了P个系数
//	//CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
//	mp_neighbor_pca_projected_x = cvCreateMat(numData, dimPCA_x, CV_32F);
//	mp_neighbor_pca_projected_y = cvCreateMat(numData, dimPCA_y, CV_32F);
//	mp_neighbor_pca_projected_z = cvCreateMat(numData, dimPCA_z, CV_32F);
//	//cvProjectPCA( pData, pMean, pEigVecs, pResult );
//	cvProjectPCA(p_source_x, mp_neighbor_pca_average_x, mp_neighbor_pca_eigenvec_x, mp_neighbor_pca_projected_x);
//	cvProjectPCA(p_source_y, mp_neighbor_pca_average_y, mp_neighbor_pca_eigenvec_y, mp_neighbor_pca_projected_y);
//	cvProjectPCA(p_source_z, mp_neighbor_pca_average_z, mp_neighbor_pca_eigenvec_z, mp_neighbor_pca_projected_z);
//
//	//============ TEST TI PCA backproject result
//	if (true) {
//		CvMat* backproject_x = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
//		cvBackProjectPCA(mp_neighbor_pca_projected_x, mp_neighbor_pca_average_x, mp_neighbor_pca_eigenvec_x, backproject_x);
//		Mat backprojectMat_x = cvarrToMat(backproject_x);
//		Mat PCAbackprojectDM1 = Mat(TIsize_ - 2 * N, TIsize_ - 2 * N, CV_8UC1);
//
//		CvMat* backproject_y = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
//		cvBackProjectPCA(mp_neighbor_pca_projected_y, mp_neighbor_pca_average_y, mp_neighbor_pca_eigenvec_y, backproject_y);
//		Mat backprojectMat_y = cvarrToMat(backproject_y);
//		Mat PCAbackprojectDM2 = Mat(TIsize_ - 2 * N, TIsize_ - 2 * N, CV_8UC1);
//
//		CvMat* backproject_z = cvCreateMat(numData, D_NEIGHBOR, CV_32F);
//		cvBackProjectPCA(mp_neighbor_pca_projected_z, mp_neighbor_pca_average_z, mp_neighbor_pca_eigenvec_z, backproject_z);
//		Mat backprojectMat_z = cvarrToMat(backproject_z);
//		Mat PCAbackprojectDM3 = Mat(TIsize_ - 2 * N, TIsize_ - 2 * N, CV_8UC1);
//
//		int row = 0;
//		int cols = ((2 * N + 1)*(2 * N + 1) - 1) * 0.5;
//
//		for (int v = 0; v < TIsize_ - 2 * N; ++v) {
//			for (int u = 0; u < TIsize_ - 2 * N; ++u) {
//				int tempv = backprojectMat_x.at<ANNcoord>(row, cols);
//				if (tempv < 0) tempv = 0;	else if (tempv > 255) tempv = 255;
//				PCAbackprojectDM1.at<uchar>(v, u) = tempv;
//
//				tempv = backprojectMat_y.at<ANNcoord>(row, cols);
//				if (tempv < 0) tempv = 0;	else if (tempv > 255) tempv = 255;
//				PCAbackprojectDM2.at<uchar>(v, u) = tempv;
//
//				tempv = backprojectMat_z.at<ANNcoord>(row, cols);
//				if (tempv < 0) tempv = 0;	else if (tempv > 255) tempv = 255;
//				PCAbackprojectDM3.at<uchar>(v, u) = tempv;
//
//				++row;
//			}
//		}
//
//		imwrite("PCA_TI1.png", PCAbackprojectDM1);
//		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite("PCA_TI2.png", PCAbackprojectDM2);
//		if (!(FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite("PCA_TI3.png", PCAbackprojectDM3);
//		cvReleaseMat(&backproject_x);
//		cvReleaseMat(&backproject_y);
//		cvReleaseMat(&backproject_z);
//		cout << endl << "PCA back projected image outputed.";
//	}
//
//	// release CV matrices
//	cvReleaseMat(&p_source_x);
//	cvReleaseMat(&p_source_y);
//	cvReleaseMat(&p_source_z);
//	cvReleaseMat(&p_eigenValues_x);
//	cvReleaseMat(&p_eigenValues_y);
//	cvReleaseMat(&p_eigenValues_z);
//	cvReleaseMat(&p_eigenVectors_all_x);
//	cvReleaseMat(&p_eigenVectors_all_y);
//	cvReleaseMat(&p_eigenVectors_all_z);
//}


// =========== main procedures =============
void DoPAR::init() {
	// load TI
	if (!loadExemplar()) return;

	// K-Coherence
	computeKCoherence_MultipleTIs();

	// load Model
	if (!loadVolume()) return;


	colorweight.resize(MULTIRES);
	indexweight.resize(MULTIRES);	
	posweight.resize(MULTIRES);
	for (int i = 0; i < MULTIRES; i++) {
		// init ColorHis
		if (ColorHis_ON) initColorHis_exemplar();
		colorweight[i] = factorC / (OUTsize[i] * OUTsize[i] * OUTsize[i] / ColorHis_BinNum);
		// init IndexHis, PosHis
		//~TI_
		avgIndexHis[i] = ceil((1.0f * OUTsize[i] * (OUTsize[i] * 0.5) * (OUTsize[i] * 0.5)) / ((TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)*(TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)));
		if (SIM2D_YN) avgIndexHis[i] = ceil(MultiTIsNum* (1.0f * (OUTsize[i] * 0.5) * (OUTsize[i] * 0.5)) / ((TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)*(TIsize[i] * 0.5 - blockSize[i] * 0.5 + 1)));
		indexweight[i] = factorIndex / avgIndexHis[i];
		//~1/3 TI_
		avgPosHis[i] = ceil((OUTsize[i] * OUTsize[i] * OUTsize[i]) / ((TIsize[i] - 2)*(TIsize[i] - 2)) / 3.0);
		if (SIM2D_YN) avgPosHis[i] = ceil(MultiTIsNum* (OUTsize[i] * OUTsize[i]) / ((TIsize[i] - 2)*(TIsize[i] - 2)));
		posweight[i] = factorPos / avgPosHis[i];

		cout << endl << "colorweight=" << colorweight[i] << "  indexweight=" << indexweight[i] << "  posweight=" << posweight[i];
	}
}

void DoPAR::DoANNOptimization(int TIseries) {
	time_t StartTime;	time(&StartTime);
	unsigned long t1, t2, t3;

	init();

	for (int numsim = 0; numsim < NumRealization; ++numsim) {
		cout << endl << "Realization :  " << numsim + 1 << "========================================";
		if (numsim > 0) {
			allocateVectors();
			InitRandomVolume(0);
		}

		for (int curlevel = 0; curlevel < MULTIRES; curlevel++) {
			cout << endl << "=============level: " << curlevel << "===============";

			FIRSTRUN = true;
			initPermutation(OUTsize[curlevel], SIM2D_YN, m_permutation);

			for (int loop = 0; loop < MAXITERATION[curlevel]; loop++) {
				shuffle(m_permutation.begin(), m_permutation.end(), mersennetwistergenerator);
				if (loop % (int)ceil(MAXITERATION[curlevel] * 0.5) == 0)
					cout << endl << "iteration: " << loop;
				//the first run on level0 should be started by search	
				if (curlevel == 0 && loop == 0) searchVolume(curlevel, loop);

				t1 = GetTickCount();
				optimizeVolume(curlevel, loop);
				t2 = GetTickCount();
				if (loop % (int)ceil(MAXITERATION[curlevel] * 0.5) == 0)
					cout << endl << "optmize: " << (t2 - t1) / 1000.0 << " s";

				if (searchVolume(curlevel, loop)) {
					cout << endl << "converged, skip to next level.";
					break;
				}
				t3 = GetTickCount();
				if (loop % (int)ceil(MAXITERATION[curlevel] * 0.5) == 0)
					cout << endl << "search: " << (t3 - t2) / 1000.0 << " s";

				if (curlevel == MULTIRES - 1 && loop == MAXITERATION[curlevel] - 1) optimizeVolume(curlevel, loop);	//do one optimization after the last search

				//if ((SIM2D_YN && curlevel == 1) ) {		// output for 2D tests
				//	outputmodel(curlevel);
				//	_getch();
				//}
			}

			if (curlevel == MULTIRES - 1 || outputmultilevel) {// ouput model & histogram
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
	cout << endl << "Total reconstruction time: " << unsigned long(NewTime - StartTime) << " s (" << unsigned long(NewTime - StartTime) / 60 << " min)";
	cleardata();
}

void DoPAR::upsampleVolume(int level) {
	// update nearestIdx & IndexHis for next level
	// color does not matter
	bool minimumsampleYN = true;
	if (factorIndex == 0 && factorPos == 0) minimumsampleYN = false;

	size_idx OUTsize_ = OUTsize[level];
	size_idx Sx = OUTsize_;
	if (SIM2D_YN) Sx = 1;
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx OUTsize3d_ = Sx * OUTsize2d_;
	size_idx doubleOUTsize_ = 2 * OUTsize_;
	size_idx doubleOUTsize2d_ = 4 * OUTsize2d_;
	size_idx doubleTIsize_ = 2 * TIsize_;

	//X	
	for (size_idx i = 0; i < Sx; ++i) {
		size_idx iSyz = i*OUTsize2d_;
		size_idx sumidx_di = 2 * i * doubleOUTsize2d_;									//(2 * i)*(2 * Sy)*(2 * Sz)
		for (size_idx j = 0; j < OUTsize_; j += GRID) {					//sparse grid
			size_idx jSz = j*OUTsize_;
			size_idx sumidx_dj = 2 * j * doubleOUTsize_;								//(2 * j)*(2 * Sz)
			for (size_idx k = 0; k < OUTsize_; k += GRID) {				//sparse grid
				size_idx idx3d = iSyz + jSz + k;
				size_idx nidx2d = nearestIdx_x[level][idx3d];
				size_idx rnidx2d;
				//random upsample
				if (!minimumsampleYN) {
					rnidx2d = KCoherence_x[level][nidx2d][(rand() % (COHERENCENUM))];
				}
				else {
					//minimum upsample		//!tested: better
					vector<long> Indexhiscomparelist(COHERENCENUM);
					int minidx = 0;
					for (int n = 0; n < COHERENCENUM; n++) {
						Indexhiscomparelist[n] = IndexHis_x[level][sparseIdx_TIs(level, KCoherence_x[level][nidx2d][n])];
						if (Indexhiscomparelist[n] < Indexhiscomparelist[minidx])
							minidx = n;
					}
					rnidx2d = KCoherence_x[level][nidx2d][minidx];
					IndexHis_x[level][sparseIdx_TIs(level, rnidx2d)]++;
				}

				size_idx coordx = nidx2d / TIsize_;		size_idx coordy = nidx2d % TIsize_;
				size_idx rcoordx = rnidx2d / TIsize_;	size_idx rcoordy = rnidx2d % TIsize_;
				//!! no need to change for TIs
				coordx *= 2;	coordy *= 2;
				rcoordx *= 2;	rcoordy *= 2;
				nidx2d = coordx*doubleTIsize_ + coordy;					//new doubled nidx2d & rnidx2d
				rnidx2d = rcoordx*doubleTIsize_ + rcoordy;
				size_idx didx3d = sumidx_di + sumidx_dj + 2 * k;					//doubled didx3d

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


	if (!SIM2D_YN) {
		//Y
		for (size_idx j = 0; j < OUTsize_; ++j) {
			size_idx jSz = j*OUTsize_;
			size_idx sumidx_dj = 2 * j * doubleOUTsize_;
			for (size_idx i = 0; i < OUTsize_; i += GRID) {					//sparse grid	
				size_idx iSyz = i*OUTsize2d_;
				size_idx sumidx_di = 2 * i * doubleOUTsize2d_;
				for (size_idx k = 0; k < OUTsize_; k += GRID) {				//sparse grid
					size_idx idx3d = iSyz + jSz + k;
					size_idx nidx2d = nearestIdx_y[level][idx3d];
					size_idx rnidx2d;
					////random upsample
					if (!minimumsampleYN) {
						rnidx2d = KCoherence_y[level][nidx2d][(rand() % (COHERENCENUM))];
					}
					else {
						//minimum upsample
						vector<long> Indexhiscomparelist(COHERENCENUM);
						int minidx = 0;
						for (int n = 0; n < COHERENCENUM; n++) {
							Indexhiscomparelist[n] = IndexHis_y[level][sparseIdx_TIs(level, KCoherence_y[level][nidx2d][n])];
							if (Indexhiscomparelist[n] < Indexhiscomparelist[minidx])
								minidx = n;
						}
						rnidx2d = KCoherence_y[level][nidx2d][minidx];
						IndexHis_y[level][sparseIdx_TIs(level, rnidx2d)]++;
					}

					size_idx coordx = nidx2d / TIsize_;		size_idx coordy = nidx2d % TIsize_;
					size_idx rcoordx = rnidx2d / TIsize_;	size_idx rcoordy = rnidx2d % TIsize_;

					coordx *= 2;	coordy *= 2;
					rcoordx *= 2;	rcoordy *= 2;
					nidx2d = coordx*doubleTIsize_ + coordy;					//new doubled nidx2d & rnidx2d
					rnidx2d = rcoordx*doubleTIsize_ + rcoordy;
					size_idx didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

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
				size_idx iSyz = i*OUTsize2d_;
				size_idx sumidx_di = 2 * i * doubleOUTsize2d_;
				for (size_idx j = 0; j < OUTsize_; j += GRID) {				//sparse grid
					size_idx jSz = j*OUTsize_;
					size_idx sumidx_dj = 2 * j * doubleOUTsize_;

					size_idx idx3d = iSyz + jSz + k;
					size_idx nidx2d = nearestIdx_z[level][idx3d];
					size_idx rnidx2d;
					////random upsample
					if (!minimumsampleYN) {
						rnidx2d = KCoherence_z[level][nidx2d][(rand() % (COHERENCENUM))];
					}
					else {
						//minimum upsample
						vector<long> Indexhiscomparelist(COHERENCENUM);
						int minidx = 0;
						for (int n = 0; n < COHERENCENUM; n++) {
							Indexhiscomparelist[n] = IndexHis_z[level][sparseIdx_TIs(level, KCoherence_z[level][nidx2d][n])];
							if (Indexhiscomparelist[n] < Indexhiscomparelist[minidx])
								minidx = n;
						}
						rnidx2d = KCoherence_z[level][nidx2d][minidx];
						IndexHis_z[level][sparseIdx_TIs(level, rnidx2d)]++;
					}

					size_idx coordx = nidx2d / TIsize_;		size_idx coordy = nidx2d % TIsize_;
					size_idx rcoordx = rnidx2d / TIsize_;	size_idx rcoordy = rnidx2d % TIsize_;

					coordx *= 2;	coordy *= 2;
					rcoordx *= 2;	rcoordy *= 2;
					nidx2d = coordx*doubleTIsize_ + coordy;					//new doubled nidx2d & rnidx2d
					rnidx2d = rcoordx*doubleTIsize_ + rcoordy;
					size_idx didx3d = sumidx_di + sumidx_dj + 2 * k;			//doubled didx3d

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

bool DoPAR::setNearestIndex(int level, vector<size_idx>& nearestIdx, vector<size_dist>& nearestWeight, vector<size_hiscount>&IndexHis,
	size_idx idx3d, size_idx newNearestIdx, size_dist dis) {
	// only used in upsampling
	//update IndexHis & NearestIndex, store EuDis^-0.6 -- search step
	size_idx TIsize_ = TIsize[level];

	size_idx formerNearestIdx = nearestIdx[idx3d];
	nearestWeight[idx3d] = 1.0f / dis;

	if (formerNearestIdx == newNearestIdx)	return true;
	nearestIdx[idx3d] = newNearestIdx;												//update nearestIdx

	if (formerNearestIdx < TIsize_ * TIsize_ && formerNearestIdx >= 0) {
		size_idx sparsedFormerNearestIdx = sparseIdx_TIs(level, formerNearestIdx);		//update IndexHis sparse grid 
		if (IndexHis[sparsedFormerNearestIdx] > 0)	IndexHis[sparsedFormerNearestIdx]--;
	}

	IndexHis[sparseIdx_TIs(level, newNearestIdx)]++;									//update IndexHis sparse grid 	
	return false;
}

// =========== K-coherence precompute =============
void DoPAR::computeKCoherence_MultipleTIs() {
	cout << endl << "K=" << COHERENCENUM << " compute K-coherence...";
	unsigned long time_start = clock();

	KCoherence_x.resize(MULTIRES);
	KCoherence_y.resize(MULTIRES);
	KCoherence_z.resize(MULTIRES);
	// check KC multiTIs usage is uniform or not
	vector<vector<long>> KCusage_TIs_x(MULTIRES), KCusage_TIs_y(MULTIRES), KCusage_TIs_z(MULTIRES);

	for (int level = 0; level < MULTIRES; ++level) {
		size_idx TIsize_ = TIsize[level];
		size_idx TIsize2d_ = TIsize_*TIsize_;
		size_idx blockSize_ = blockSize[level];
		size_idx width = TIsize_ - blockSize_ + 1;
		size_idx height = TIsize_ - blockSize_ + 1;
		size_idx maxSize2d = TIsize_ * height;
		size_idx dim = blockSize_ * blockSize_;
		size_idx bias = blockSize_ * 0.5;
		cout << endl << "level: " << level << " Template size="<< blockSize_ << " Dimension=" << dim;

		size_idx numData = width * height * MultiTIsNum;
		size_idx KCoherenceSize = TIsize2d_ * MultiTIsNum;
		
		KCusage_TIs_x[level].resize(MultiTIsNum);
		KCusage_TIs_y[level].resize(MultiTIsNum);
		KCusage_TIs_z[level].resize(MultiTIsNum);

		KCoherence_x[level].resize(KCoherenceSize);
		KCoherence_y[level].resize(KCoherenceSize);
		KCoherence_z[level].resize(KCoherenceSize);

		// ----------- add template to p_source_x [numData][dim]
		ANNpointArray p_source_x, p_source_y, p_source_z;
		p_source_x = annAllocPts(numData, dim);			//rows='area' numData, cols=dimension (Neighbour size)
		p_source_y = annAllocPts(numData, dim);
		p_source_z = annAllocPts(numData, dim);
		size_idx row = 0;
		for (int tin = 0; tin < MultiTIsNum; tin++) {
			for (size_idx i = 0; i < width; ++i) {
				for (size_idx j = 0; j < height; ++j) {
					size_idx col = 0;
					size_idx index0 = TIsize_ * i + j;
					for (size_idx m = 0; m <blockSize_; ++m) {
						for (size_idx n = 0; n <blockSize_; ++n) {
							size_idx index = index0 + m * TIsize_ + n;			//[i+m][j+n]
							p_source_x[row][col] = TIs_XY[level][tin][index];
							if (!SIM2D_YN) {
								p_source_y[row][col] = TIs_XZ[level][tin][index];
								p_source_z[row][col] = TIs_YZ[level][tin][index];
							}
							++col;
						}
					}
					++row;
				}
			}
		}

		// ----------- build ANNkd_tree
		ANNkd_tree*  kdTree_x;
		ANNkd_tree*  kdTree_y;
		ANNkd_tree*  kdTree_z;
		kdTree_x = new ANNkd_tree(p_source_x, numData, dim);
		kdTree_y = new ANNkd_tree(p_source_y, numData, dim);
		kdTree_z = new ANNkd_tree(p_source_z, numData, dim);

		// ----------- ANNkd_tree search
#pragma omp parallel
		{
			for (int tin = 0; tin < MultiTIsNum; tin++) {
#pragma omp for nowait schedule(static)
				for (size_idx idx = 0; idx < maxSize2d; idx++) {
					if (idx%TIsize_ >= width) continue;
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
					// load query pattern
					long num = 0;
					for (size_idx m = 0; m < blockSize_; ++m) {
						for (size_idx n = 0; n < blockSize_; ++n) {
							size_idx index = idx + TIsize_ * m + n;		//[i+m][j+n]

							queryPt_x[num] = TIs_XY[level][tin][index];
							if (!SIM2D_YN) {
								queryPt_y[num] = TIs_XZ[level][tin][index];
								queryPt_z[num] = TIs_YZ[level][tin][index];
							}
							num++;
						}
					}
					// ANNsearch
					kdTree_x->annkSearch(queryPt_x, COHERENCENUM, ann_index_x, ann_dist_x, ANNerror[level]);
					if (!SIM2D_YN) {
						kdTree_y->annkSearch(queryPt_y, COHERENCENUM, ann_index_y, ann_dist_y, ANNerror[level]);
						kdTree_z->annkSearch(queryPt_z, COHERENCENUM, ann_index_z, ann_dist_z, ANNerror[level]);
					}
					// Set K-Coherence shifted to same range of TI_index
					size_idx shift_TIs = tin*TIsize2d_;
					size_idx bias_TIindex = (idx + bias*TIsize_ + bias) + shift_TIs;
					KCoherence_x[level][bias_TIindex].resize(COHERENCENUM);
					KCoherence_y[level][bias_TIindex].resize(COHERENCENUM);
					KCoherence_z[level][bias_TIindex].resize(COHERENCENUM);
					for (int k = 0; k < COHERENCENUM; ++k) {
						KCoherence_x[level][bias_TIindex][k] = convertIndexANN_TIs(level, ann_index_x[k], TIsize_, blockSize_);
						if (!SIM2D_YN) {
							KCoherence_y[level][bias_TIindex][k] = convertIndexANN_TIs(level, ann_index_y[k], TIsize_, blockSize_);
							KCoherence_z[level][bias_TIindex][k] = convertIndexANN_TIs(level, ann_index_z[k], TIsize_, blockSize_);
						}

						//count KC multiTIs usage
						auto TIsn = KCoherence_x[level][bias_TIindex][k] / TIsize2d_;
						long& usageaddr_x = KCusage_TIs_x[level][TIsn];
#pragma omp atomic
						usageaddr_x++;
						if (!SIM2D_YN) {
							TIsn = KCoherence_y[level][bias_TIindex][k] / TIsize2d_;
							long& usageaddr_y = KCusage_TIs_y[level][TIsn];
#pragma omp atomic
							usageaddr_y++;
							TIsn = KCoherence_z[level][bias_TIindex][k] / TIsize2d_;
							long& usageaddr_z = KCusage_TIs_z[level][TIsn];
#pragma omp atomic
							usageaddr_z++;
						}
					}
					// release
					annDeallocPt(queryPt_x);		annDeallocPt(queryPt_y);		annDeallocPt(queryPt_z);
					delete[] ann_index_x;		delete[] ann_index_y;		delete[] ann_index_z;
					delete[] ann_dist_x;		delete[] ann_dist_y;		delete[] ann_dist_z;
				}//#pragma omp for nowait schedule(static)
			}
		}// #pragma omp parallel

		//release
		annClose();
		delete kdTree_x;				delete kdTree_y;				delete kdTree_z;
		annDeallocPts(p_source_x);		annDeallocPts(p_source_y);		annDeallocPts(p_source_z);

		// check KC multiTIs usage is uniform or not
		if (MultiTIsNum > 1) {
			vector<double> KCusageratio_TIs_x(MultiTIsNum), KCusageratio_TIs_y(MultiTIsNum), KCusageratio_TIs_z(MultiTIsNum);
			long total = accumulate(KCusage_TIs_x[level].begin(), KCusage_TIs_x[level].end(), 0);
			cout << fixed;
			for (int s = 0; s < MultiTIsNum; s++) {
				KCusageratio_TIs_x[s] = 100.0 * KCusage_TIs_x[level][s] / total;
				KCusageratio_TIs_y[s] = 100.0 * KCusage_TIs_y[level][s] / total;
				KCusageratio_TIs_z[s] = 100.0 * KCusage_TIs_z[level][s] / total;
				cout << endl << setprecision(1) << "KC usage TI(" << s << "):  X "
					<< KCusageratio_TIs_x[s] << "   Y " << KCusageratio_TIs_y[s] << "   Z " << KCusageratio_TIs_z[s];
			}
		}
	}

	long time_end = clock();
	cout << endl << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC << " s";
}

// ================ search (M-step) ===============


bool DoPAR::searchVolume(int level, int loop) {
	
	bool enhancelowindexYN = false;
	if (level<1 && factorIndex>0 &&loop> MAXITERATION[level] / 2) enhancelowindexYN = true;
	// const
	bool isUnchanged = true;	
	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx Size = OUTsize2d_*OUTsize_;
	if (SIM2D_YN) Size = OUTsize2d_;	
	size_idx start = blockSize_ * 0.5;				//-cstart<=x<=cend
	size_idx end = (blockSize_ - 1) * 0.5;
	size_idx cstart(start), cend(end);
	size_idx poshisadd[3] = { 0, TIsize2d_, TIsize2d_ * 2 };

	//AssignFixedLayer(level, FixedLayerDir);

#pragma omp parallel 
{
	//XY
#pragma omp for nowait schedule(static)
	for (size_idx i2 = 0; i2 < Size; ++i2) {
		int ori = 0;
		size_idx idx = m_permutation[i2];
		size_idx i, j, k;
		idxToCoord(idx, OUTsize_, i, j, k);
		// check skip
		if (j % GRID != 0 || k % GRID != 0)			continue;
		if (isUnchangedBlock(level, 0, i, j, k))	continue;

		// temp
		size_idx iSyz = OUTsize2d_ * i, jSz = j * OUTsize_;
		size_idx bestTIIdx_regular, bestIdx_TIs;
		size_hiscount besthis(0);
		size_dist minError(max_dist), minDis(max_dist);
		vector<size_idx> compareIdx;
		int compareNum = 0;
		compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		vector<size_color> current_pattern(blockSize_*blockSize_);

		// Load current pattern
		size_idx tempindex = 0, VCurIdx, index2;
		for (size_idx du = -start; du <= end; ++du) {
			VCurIdx = iSyz + OUTsize_ * trim(OUTsize_, j + du);
			for (size_idx dv = -start; dv <= end; ++dv) {
				index2 = VCurIdx + trim(OUTsize_, k + dv);
				current_pattern[tempindex++] = m_volume[level][index2];
			}
		}

		// get 3didx
		for (size_idx u = -cstart; u <= cend; ++u) {
			size_idx sumidx_posx, temp3didx;
			sumidx_posx = iSyz + trim(OUTsize_, j + u)*OUTsize_;
			for (size_idx v = -cstart; v <= cend; ++v) {
				temp3didx = sumidx_posx + trim(OUTsize_, k + v);
				size_idx temporigin_Idxregular, temporigin_TInum, tempTIidx_TIs;
				tempTIidx_TIs = Origin_x[level][temp3didx];
				TIsToRegular(tempTIidx_TIs, TIsize2d_, temporigin_Idxregular, temporigin_TInum);

				size_idx eposx = (temporigin_Idxregular / TIsize_) - u;
				size_idx eposy = (temporigin_Idxregular % TIsize_) - v;
				// boundary check
				if (!(eposx >= start && eposx < TIsize_ - end && eposy >= start && eposy < TIsize_ - end))	continue;

				tempTIidx_TIs = tempTIidx_TIs - (u*TIsize_ + v);			//origin - (u,v)

				for (int l = 0; l < COHERENCENUM; ++l) {
					// get KCoherence
					size_idx temp2didx_TIs = KCoherence_x[level][tempTIidx_TIs][l];
					size_idx temp2didx_Idxregular, temp2didx_TInum;
					TIsToRegular(temp2didx_TIs, TIsize2d_, temp2didx_Idxregular, temp2didx_TInum);

					// check duplicate
					int p = 0;
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx_TIs)	break;
					}if (p < compareNum)	continue;

					// calc square distance 
					size_dist curDis = getFullDistance(level, TIs_XY[level][temp2didx_TInum], temp2didx_Idxregular, current_pattern);

					// calc his weight
					size_dist curhis = IndexHis_x[level][sparseIdx_TIs(level, temp2didx_TIs)];	// IndexHis sparse grid
					size_dist tempHisDiff = max(0.0f, 1.0f*(curhis - avgIndexHis[level]));
					// manual control indexhis
					//if (tempHisDiff > IndexHisManualControl*avgIndexHis[level]) curError = max_dist;
					if (enhancelowindexYN && (curhis < avgIndexHis[level])) {
						tempHisDiff = 1.0f*(curhis - avgIndexHis[level]);
					}
					size_dist IndexHisWeight = 1.0f + indexweight[level] * tempHisDiff;
					size_dist curError = IndexHisWeight * curDis;
					// keep minError
					if (curError < minError) {
						minError = curError;
						minDis = curDis;
						bestIdx_TIs = temp2didx_TIs;
						besthis = curhis;
					}
					else if (curError - minError < 1e-8) {			// if Error same, first compare IndexHis
						if (curhis < besthis) {
							minDis = curDis;
							bestIdx_TIs = temp2didx_TIs;
							besthis = curhis;
						}
						else if (curhis == besthis && !FIRSTRUN) {
							if (PosHis[level][temp2didx_Idxregular + poshisadd[ori]] < PosHis[level][bestIdx_TIs%TIsize2d_ + poshisadd[ori]])
								bestIdx_TIs = temp2didx_TIs;
						}
					}

					compareNum++;
					compareIdx.push_back(temp2didx_TIs);
				}
			}
		}
		// get bestTIIdx
		if (fabs(max_dist - minError) > 1.0f) {
			nearestWeight_x[level][idx] = 1.0f / minDis;
		}
		else {
			bestTIIdx_regular = getRandomNearestIndex(level, IndexHis_x[level]);
			bestIdx_TIs = (rand() % MultiTIsNum) * TIsize2d_ + bestTIIdx_regular;
			nearestWeight_x[level][idx] = 1e-3f;
		}

		// update nearestIdx
		size_idx formerNearestIdx = nearestIdx_x[level][idx];
		if (formerNearestIdx != bestIdx_TIs) {
			nearestIdx_x[level][idx] = bestIdx_TIs;

			//if (bestIdx_TIs < 0 || bestIdx_TIs >= MultiTIsNum*TIsize2d_) {
			//	printf("\nsearch: idx%d bestIdx_TIs%d", idx, bestIdx_TIs);
			//	_getch();
			//}

			// update indexhis
			if (formerNearestIdx < MultiTIsNum * TIsize2d_ && formerNearestIdx >= 0) {
				size_hiscount& addressFormerNearestIdx = IndexHis_x[level][sparseIdx_TIs(level, formerNearestIdx)];
				if (addressFormerNearestIdx > 0)
#pragma omp atomic
					addressFormerNearestIdx--;
			}
			size_hiscount& addressbestTIIdx = IndexHis_x[level][sparseIdx_TIs(level, bestIdx_TIs)];
#pragma omp atomic
			addressbestTIIdx++;
			isUnchanged = false;
		}
	}//for (size_idx i2 = 0; i2 < Size; ++i2) {

	//XZ
#pragma omp for nowait schedule(static)
	for (size_idx i2 = 0; i2 < Size; ++i2) {
		int ori = 1;
		if (SIM2D_YN && ori != 0) continue;
		size_idx idx = m_permutation[i2];
		size_idx i, j, k;
		idxToCoord(idx, OUTsize_, i, j, k);
		// check skip
		if (ori == 0 && (j % GRID != 0 || k % GRID != 0))		continue;
		else if (ori == 1 && (i % GRID != 0 || k % GRID != 0))	continue;
		else if (ori == 2 && (i % GRID != 0 || j % GRID != 0))	continue;
		if (isUnchangedBlock(level, ori, i, j, k)) continue;

		// temp
		size_idx iSyz = OUTsize2d_ * i, jSz = j * OUTsize_;
		size_idx bestTIIdx_regular, bestIdx_TIs;
		size_hiscount besthis(0);
		size_dist minError(max_dist), minDis(max_dist);
		vector<size_idx> compareIdx;
		int compareNum = 0;
		compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		vector<size_color> current_pattern(blockSize_*blockSize_);
		// Load current pattern
		size_idx tempindex = 0, VCurIdx, index2;
		for (size_idx du = -start; du <= end; ++du) {
			VCurIdx = OUTsize2d_ * trim(OUTsize_, i + du) + jSz;
			for (size_idx dv = -start; dv <= end; ++dv) {
				index2 = VCurIdx + trim(OUTsize_, k + dv);
				current_pattern[tempindex++] = m_volume[level][index2];
			}
		}

		// get 3didx
		for (size_idx u = -cstart; u <= cend; ++u) {
			size_idx sumidx_posx, temp3didx;
			sumidx_posx = trim(OUTsize_, i + u)*OUTsize2d_ + jSz;
			for (size_idx v = -cstart; v <= cend; ++v) {
				temp3didx = sumidx_posx + trim(OUTsize_, k + v);
				size_idx temporigin_Idxregular, temporigin_TInum, tempTIidx_TIs;
				tempTIidx_TIs = Origin_y[level][temp3didx];
				TIsToRegular(tempTIidx_TIs, TIsize2d_, temporigin_Idxregular, temporigin_TInum);
				size_idx eposx = (temporigin_Idxregular / TIsize_) - u;
				size_idx eposy = (temporigin_Idxregular % TIsize_) - v;
				// boundary check
				if (!(eposx >= start && eposx < TIsize_ - end && eposy >= start && eposy < TIsize_ - end))	continue;

				tempTIidx_TIs = tempTIidx_TIs - (u*TIsize_ + v);			//origin - (u,v)
				for (int l = 0; l < COHERENCENUM; ++l) {
					// get KCoherence
					size_idx temp2didx_TIs = KCoherence_y[level][tempTIidx_TIs][l];
					size_idx temp2didx_Idxregular, temp2didx_TInum;
					TIsToRegular(temp2didx_TIs, TIsize2d_, temp2didx_Idxregular, temp2didx_TInum);

					// check duplicate
					int p = 0;
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx_TIs)	break;
					}if (p < compareNum)	continue;

					// calc square distance 
					size_dist curDis = getFullDistance(level, TIs_XZ[level][temp2didx_TInum], temp2didx_Idxregular, current_pattern);

					// calc his weight
					size_hiscount curhis = IndexHis_y[level][sparseIdx_TIs(level, temp2didx_TIs)];	// IndexHis sparse grid
					size_hiscount tempHisDiff = max(0.0f, 1.0f*(curhis - avgIndexHis[level]));
					// manual control indexhis
					//if (tempHisDiff > IndexHisManualControl*avgIndexHis[level]) curError = max_dist;
					if (enhancelowindexYN && (curhis < avgIndexHis[level])) {
						tempHisDiff = 1.0f*(curhis - avgIndexHis[level]);
					}
					size_dist IndexHisWeight = 1.0f + indexweight[level] * tempHisDiff;
					size_dist curError = IndexHisWeight * curDis;
					// keep minError
					if (curError < minError) {
						minError = curError;
						minDis = curDis;
						bestIdx_TIs = temp2didx_TIs;
						besthis = curhis;
					}
					else if (curError - minError < 10e-9) {			// if Error same, first compare IndexHis
						if (curhis < besthis) {
							minDis = curDis;
							bestIdx_TIs = temp2didx_TIs;
							besthis = curhis;
						}
						else if (curhis == besthis && !FIRSTRUN) {
							if (PosHis[level][temp2didx_Idxregular + poshisadd[ori]] < PosHis[level][bestIdx_TIs%TIsize2d_ + poshisadd[ori]])
								bestIdx_TIs = temp2didx_TIs;
						}
					}
					compareNum++;
					compareIdx.push_back(temp2didx_TIs);
				}
			}
		}
		// get bestTIIdx
		if (fabs(max_dist - minError) > 1.0f) {
			nearestWeight_y[level][idx] = 1.0f / minDis;
		}
		else {
			bestTIIdx_regular = getRandomNearestIndex(level, IndexHis_y[level]);
			bestIdx_TIs = (rand() % MultiTIsNum) * TIsize2d_ + bestTIIdx_regular;
			nearestWeight_y[level][idx] = 1e-3f;
		}

		// update nearestIdx
		size_idx formerNearestIdx = nearestIdx_y[level][idx];
		if (formerNearestIdx != bestIdx_TIs) {
			nearestIdx_y[level][idx] = bestIdx_TIs;
			// update indexhis
			if (formerNearestIdx < MultiTIsNum * TIsize2d_ && formerNearestIdx >= 0) {
				size_hiscount& addressFormerNearestIdx = IndexHis_y[level][sparseIdx_TIs(level, formerNearestIdx)];
				if (addressFormerNearestIdx > 0)
#pragma omp atomic
					addressFormerNearestIdx--;
			}
			size_hiscount& addressbestTIIdx = IndexHis_y[level][sparseIdx_TIs(level, bestIdx_TIs)];
#pragma omp atomic
			addressbestTIIdx++;
			isUnchanged = false;
		}
	}//for (size_idx i2 = 0; i2 < Size; ++i2) {

	//YZ
#pragma omp for nowait schedule(static)
	for (size_idx i2 = 0; i2 < Size; ++i2) {
		int ori = 2;
		if (SIM2D_YN && ori != 0) continue;
		size_idx idx = m_permutation[i2];
		size_idx i, j, k;
		idxToCoord(idx, OUTsize_, i, j, k);
		// check skip
		if (ori == 0 && (j % GRID != 0 || k % GRID != 0))		continue;
		else if (ori == 1 && (i % GRID != 0 || k % GRID != 0))	continue;
		else if (ori == 2 && (i % GRID != 0 || j % GRID != 0))	continue;
		if (isUnchangedBlock(level, ori, i, j, k)) continue;
		// temp
		size_idx iSyz = OUTsize2d_ * i, jSz = j * OUTsize_;
		size_idx bestTIIdx_regular, bestIdx_TIs;
		size_hiscount besthis(0);
		size_dist minError(max_dist), minDis(max_dist);
		vector<size_idx> compareIdx;
		int compareNum = 0;
		compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		vector<size_color> current_pattern(blockSize_*blockSize_);

		// Load current pattern
		size_idx tempindex = 0, VCurIdx, index2;
		for (size_idx du = -start; du <= end; ++du) {
			VCurIdx = OUTsize2d_ * trim(OUTsize_, i + du) + k;
			for (size_idx dv = -start; dv <= end; ++dv) {
				index2 = VCurIdx + OUTsize_ * trim(OUTsize_, j + dv);
				current_pattern[tempindex++] = m_volume[level][index2];
			}
		}

		// get 3didx
		for (size_idx u = -cstart; u <= cend; ++u) {
			size_idx sumidx_posx, temp3didx;
			sumidx_posx = trim(OUTsize_, i + u)*OUTsize2d_ + k;
			for (size_idx v = -cstart; v <= cend; ++v) {
				temp3didx = sumidx_posx + trim(OUTsize_, j + v)*OUTsize_;
				size_idx temporigin_Idxregular, temporigin_TInum, tempTIidx_TIs;
				tempTIidx_TIs = Origin_z[level][temp3didx];
				TIsToRegular(tempTIidx_TIs, TIsize2d_, temporigin_Idxregular, temporigin_TInum);
				size_idx eposx = (temporigin_Idxregular / TIsize_) - u;
				size_idx eposy = (temporigin_Idxregular % TIsize_) - v;
				// boundary check
				if (!(eposx >= start && eposx < TIsize_ - end && eposy >= start && eposy < TIsize_ - end))	continue;

				tempTIidx_TIs = tempTIidx_TIs - (u*TIsize_ + v);			//origin - (u,v)
				for (int l = 0; l < COHERENCENUM; ++l) {
					// get KCoherence
					size_idx temp2didx_TIs = KCoherence_z[level][tempTIidx_TIs][l];
					size_idx temp2didx_Idxregular, temp2didx_TInum;
					TIsToRegular(temp2didx_TIs, TIsize2d_, temp2didx_Idxregular, temp2didx_TInum);

					// check duplicate
					int p = 0;
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx_TIs)	break;
					}if (p < compareNum)	continue;

					// calc square distance 
					size_dist curDis = getFullDistance(level, TIs_YZ[level][temp2didx_TInum], temp2didx_Idxregular, current_pattern);

					// calc his weight
					size_hiscount curhis = IndexHis_z[level][sparseIdx_TIs(level, temp2didx_TIs)];	// IndexHis sparse grid
					size_hiscount tempHisDiff = max(0.0f, 1.0f*(curhis - avgIndexHis[level]));
					// manual control indexhis
					//if (tempHisDiff > IndexHisManualControl*avgIndexHis[level]) curError = max_dist;
					if (enhancelowindexYN && (curhis < avgIndexHis[level])) {
						tempHisDiff = 1.0f*(curhis - avgIndexHis[level]);
					}
					size_dist IndexHisWeight = 1.0f + indexweight[level] * tempHisDiff;
					size_dist curError = IndexHisWeight * curDis;

					// keep minError
					if (curError < minError) {
						minError = curError;
						minDis = curDis;
						bestIdx_TIs = temp2didx_TIs;
						besthis = curhis;
					}
					else if (curError - minError < 10e-9) {			// if Error same, first compare IndexHis
						if (curhis < besthis) {
							minDis = curDis;
							bestIdx_TIs = temp2didx_TIs;
							besthis = curhis;
						}
						else if (curhis == besthis && !FIRSTRUN) {
							if (PosHis[level][temp2didx_Idxregular + poshisadd[ori]] < PosHis[level][bestIdx_TIs%TIsize2d_ + poshisadd[ori]])
								bestIdx_TIs = temp2didx_TIs;
						}
					}

					compareNum++;
					compareIdx.push_back(temp2didx_TIs);
				}
			}
		}
		// get bestTIIdx
		if (fabs(max_dist - minError) > 1.0f) {
			nearestWeight_z[level][idx] = 1.0f / minDis;
		}
		else {
			bestTIIdx_regular = getRandomNearestIndex(level, IndexHis_z[level]);
			bestIdx_TIs = (rand() % MultiTIsNum) * TIsize2d_ + bestTIIdx_regular;
			nearestWeight_z[level][idx] = 1e-3f;
		}

		// update nearestIdx
		size_idx formerNearestIdx = nearestIdx_z[level][idx];
		if (formerNearestIdx != bestIdx_TIs) {
			nearestIdx_z[level][idx] = bestIdx_TIs;
			// update indexhis
			if (formerNearestIdx < MultiTIsNum * TIsize2d_ && formerNearestIdx >= 0) {
				size_hiscount& addressFormerNearestIdx = IndexHis_z[level][sparseIdx_TIs(level, formerNearestIdx)];
				if (addressFormerNearestIdx > 0)
#pragma omp atomic
					addressFormerNearestIdx--;
			}
			size_hiscount& addressbestTIIdx = IndexHis_z[level][sparseIdx_TIs(level, bestIdx_TIs)];
#pragma omp atomic
			addressbestTIIdx++;
			isUnchanged = false;
		}
	}//for (size_idx i2 = 0; i2 < Size; ++i2) {
}//#pragma omp parallel 
	return isUnchanged;
}

//size_dist DoPAR::getFullDistance(int level, vector<size_color>& exemplar, size_idx idx2d, CvMat * dataMat) {
//	//2d square distance
//	size_dist sum = 0.0f;
//	size_idx R = static_cast<size_idx>(blockSize[level] * 0.5);
//	size_idx n = 0;
//	size_idx Sx = TIsize[level];
//	size_idx tempIdx;
//	size_dist dif;
//	size_idx x = idx2d / Sx, y = idx2d % Sx;
//	//[R,Sx-R+1) [R,Sx-R]
//	if (x - R< 0 || x + R > Sx || y - R<0 || y + R> Sx) {
//		printf("\ngetFullDistance() boundary");
//		_getch();
//		exit(0);
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

template<typename T>
size_dist DoPAR::getFullDistance(int level, vector<T>& TI, size_idx idx2d, vector<T>& pattern) {
	//2d square distance
	size_idx TIsize_ = TIsize[level], TIsize2d_ = TIsize_*TIsize_, R = blockSize[level]/2;

	size_dist sum(0), dif;
	size_idx x = idx2d / TIsize_, y = idx2d % TIsize_, tempIdx, n(0);

	for (size_idx i = -R; i < R; ++i) {
		tempIdx = idx2d + i*TIsize_;
		for (size_idx j = -R; j < R; ++j) {
			dif = TI[tempIdx + j] - pattern[n++];
			sum += (dif * dif);
		}
	}
	return (sum < min_dist) ? min_dist : sum;
}
template<typename T>
size_dist DoPAR::getFullDistance_TIs(int level, vector<vector<T>>& TIs, size_idx idx2d_TIs, vector<T>& pattern) {
	//2d square distance
	size_idx TIsize_ = TIsize[level], TIsize2d_ = TIsize_*TIsize_, R = blockSize[level]/2;
	size_idx idx2d = idx2d_TIs % TIsize2d_, TInum = idx2d_TIs / TIsize2d_;
	
	size_dist sum(0), dif;
	size_idx x = idx2d / TIsize_, y = idx2d % TIsize_, tempIdx, n(0);

	for (size_idx i = -R; i < R; ++i) {
		tempIdx = idx2d + i*TIsize_;
		for (size_idx j = -R; j < R; ++j) {
			dif = TIs[TInum][tempIdx + j] - pattern[n++];
			sum += (dif * dif);
		}
	}
	return (sum < min_dist) ? min_dist : sum;
}

size_idx DoPAR::getRandomNearestIndex(int level, vector<size_hiscount>& IndexHis) {
	// get minimum nearest index

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
			tempidx = iSyz + trim(Sz, tj) * Sz;
			for (size_idx tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_x[level][tempidx + trim(Sz, tk)])			//[i][tj][tk]
					return false;
			}
		}
		break;
	case(1) :	// Y
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trim(Sz, ti) * Syz + jSz;
			for (int tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_y[level][tempidx + trim(Sz, tk)])			//[ti][j][tk]
					return false;
			}
		}
		break;
	case(2) :	// Z
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trim(Sz, ti) * Syz + k;
			for (int tj = j - start; tj <= j + end; ++tj){
				if (!isUnchanged_z[level][tempidx + trim(Sz, tj)*Sz])		//[ti][tj][k]
					return false;
			}
		}
		break;
	}
	return true;
}

// ================ optimization (E-step) =========

void DoPAR::optimizeVolume(int level, int loop) {
	
	bool offsetYN = false;
	bool enhancelowposYN = false;
	if (level < 1 && factorPos>0 && loop > MAXITERATION[level] / 2) enhancelowposYN = true;
	// const
	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;
	size_idx Size = OUTsize_*OUTsize_*OUTsize_;
	if (SIM2D_YN) Size = OUTsize2d_;
	size_idx SizePosHis = 3 * TIsize2d_;
	if (SIM2D_YN) SizePosHis = TIsize2d_;
	size_idx candSize = (blockSize_ / GRID) * (blockSize_ / GRID);	//candidate has sparse grid
	size_idx start = (blockSize_ / (2 * GRID)) + 1;		
	size_idx end = start;					
	size_idx s1 = -blockSize_ * 0.5;
	size_idx e1 = (blockSize_ - 1) * 0.5;
	size_idx poshisadd[3] = { 0, TIsize2d_, TIsize2d_ * 2 };
	
#pragma omp parallel for schedule(static)
	for (size_idx i2 = 0; i2 < Size; ++i2) {
		size_idx idx = m_permutation[i2];			//[i][j][k]	
		size_idx i, j, k;
		idxToCoord(idx, OUTsize_, i, j, k);

		size_idx iSyz = i*OUTsize2d_, jSz = j*OUTsize_;
		size_dist weight_acc(FLT_MIN), minweight(max_dist);
		size_color color_acc = 0.0f, color_avg = 0.0f;
		size_idx tempx0ori[3] = { (j / GRID) * GRID , (i / GRID) * GRID , (i / GRID) * GRID };
		size_idx tempy0ori[3] = { (k / GRID) * GRID , (k / GRID) * GRID , (j / GRID) * GRID };
		//discrete solver
		vector<size_color> colorCand_x(0), colorCand_y(0), colorCand_z(0);
		colorCand_x.reserve(candSize);	colorCand_y.reserve(candSize);	colorCand_z.reserve(candSize);
		vector<size_idx> posCand_x(0), posCand_y(0), posCand_z(0);
		posCand_x.reserve(candSize);	posCand_y.reserve(candSize);	posCand_z.reserve(candSize);
		vector<size_idx> posCand_tinum_x(0), posCand_tinum_y(0), posCand_tinum_z(0);
		posCand_tinum_x.reserve(candSize); posCand_tinum_y.reserve(candSize); posCand_tinum_z.reserve(candSize);

		//XY
		{
			bool allcontinue(true);		
			int ori = 0;
			if (SIM2D_YN && ori != 0) continue;
			for (size_idx l = start; l >= -end; --l) {
				size_idx tempx = tempx0ori[ori] + l * GRID;
				size_idx deltaxori[3] = { j - tempx , i - tempx , i - tempx };
				size_idx sumidx_tempx, tempidx3d;
				sumidx_tempx = iSyz + trim(OUTsize_, tempx)*OUTsize_;
				for (size_idx h = start; h >= -end; --h) {
					size_idx tempy = tempy0ori[ori] + h * GRID;
					size_idx deltayori[3] = { k - tempy ,  k - tempy , j - tempy };
					if (deltaxori[ori] < s1 || deltaxori[ori] > e1 || deltayori[ori] < s1 || deltayori[ori] > e1)
						continue;
					
					tempidx3d = sumidx_tempx + trim(OUTsize_, tempy);
					// get nearestidx_TIs
					size_idx tempnearestidx_TIs = nearestIdx_x[level][tempidx3d];
					size_dist tempnearestweight = nearestWeight_x[level][tempidx3d];
					size_idx tempnearestidx_Idxregular, tempnearestidx_TInum;
					TIsToRegular(tempnearestidx_TIs, TIsize2d_, tempnearestidx_Idxregular, tempnearestidx_TInum);

					// shift tempnearestidx_Idxregular
					size_idx coordx = tempnearestidx_Idxregular / TIsize_ + deltaxori[ori];
					size_idx coordy = tempnearestidx_Idxregular % TIsize_ + deltayori[ori];
					if (coordx < 0 || coordy < 0 || coordx >= TIsize_ || coordy >= TIsize_) continue;
					
					allcontinue = false;

					// get nearestidx2d, color
					size_idx tempnearestidx_shifted = TIsize_ * coordx + coordy;
					size_color tempcolor = TIs_XY[level][tempnearestidx_TInum][tempnearestidx_shifted];

					size_dist weightc(max_dist), weightp(max_dist);
					// PosHis weight
					size_idx tempPosIdx = tempnearestidx_shifted + poshisadd[ori];
					size_dist tempHisDiff = max(0.0f, 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]));
					if (enhancelowposYN && (PosHis[level][tempPosIdx] < avgPosHis[level])) {
						tempHisDiff = PosHis[level][tempPosIdx] - avgPosHis[level];
					}
					weightp = 1.0f / (1.0f + tempHisDiff * posweight[level]);
					// Color weight
					if (ColorHis_ON && !FIRSTRUN) {
						//size_dist coloroffset = max(0.0f, 1.0f * (tempcolor - Solid_Upper[level]) / (Pore_Lower[level] - Solid_Upper[level]));
						//if (!offsetYN) coloroffset = min(coloroffset, 1.0f);
						//size_dist tempColorHisDiff = max(0.0f, coloroffset *(poretotal_synthesis - poretotal_required) / poretotal_required);
						//weightc = 1.0f / (1.0f + tempColorHisDiff * factorC);
						size_dist tempColorHisDiff = max(0.0f, 1.0f*(ColorHis_synthesis[level][floor(colorhis_compressratio* tempcolor)] - ColorHis_exemplar[level][floor(colorhis_compressratio* tempcolor)]));
						weightc = 1.0f / (1.0f + tempColorHisDiff * colorweight[level]);
					}
					minweight = min(weightc, weightp);

					// discrete solver
					colorCand_x.push_back(tempcolor);
					posCand_x.push_back(tempnearestidx_shifted);
					posCand_tinum_x.push_back(tempnearestidx_TInum);

					//// modify weight according to fix layer
					if (FixedLayerDir > 0 && FixedLayerDir < 3 && FixedLayerDir != ori) minweight *= DirectionalWeight;

					// accumulate color
					size_dist weight = tempnearestweight * minweight;
					color_acc += weight * tempcolor;
					weight_acc += weight;
				}
			}			

			//if (allcontinue) {
			//	printf("\n allcontinue_x: idx=%d, i=%d j=%d k=%d, permutation=%d, outsize=%d", idx, i, j, k, m_permutation[i2], OUTsize_);
			//	idxToCoord(idx, OUTsize_, i, j, k);
			//	printf("\nidx=%d, i=%d j=%d k=%d", idx, i, j, k);
			//	_getch();
			//}
			//if (posCand_x.size() == 0) {
			//	printf("\n posCand_x.size()=0");
			//	_getch();
			//}
		}//XY

		//XZ
		{
			bool allcontinue(true);
			int ori = 1;
			if (!SIM2D_YN) 
			for (size_idx l = start; l >= -end; --l) {
				size_idx tempx = tempx0ori[ori] + l * GRID;
				size_idx deltaxori[3] = { j - tempx , i - tempx , i - tempx };
				size_idx sumidx_tempx, tempidx3d;
				sumidx_tempx = trim(OUTsize_, tempx)*OUTsize2d_ + jSz;
				for (size_idx h = start; h >= -end; --h) {
					size_idx tempy = tempy0ori[ori] + h * GRID;
					size_idx deltayori[3] = { k - tempy ,  k - tempy , j - tempy };
					if (deltaxori[ori] < s1 || deltaxori[ori] > e1 || deltayori[ori] < s1 || deltayori[ori] > e1)
						continue;

					tempidx3d = sumidx_tempx + trim(OUTsize_, tempy);
					// get nearestidx_TIs
					size_idx tempnearestidx_TIs = nearestIdx_y[level][tempidx3d];
					size_dist tempnearestweight = nearestWeight_y[level][tempidx3d];
					size_idx tempnearestidx_Idxregular, tempnearestidx_TInum;
					TIsToRegular(tempnearestidx_TIs, TIsize2d_, tempnearestidx_Idxregular, tempnearestidx_TInum);

					// shift tempnearestidx_Idxregular
					size_idx coordx = tempnearestidx_Idxregular / TIsize_ + deltaxori[ori];
					size_idx coordy = tempnearestidx_Idxregular % TIsize_ + deltayori[ori];
					if (coordx < 0 || coordy < 0 || coordx >= TIsize_ || coordy >= TIsize_) continue;
					
					allcontinue = false;

					// get nearestidx2d, color
					size_idx tempnearestidx_shifted = TIsize_ * coordx + coordy;
					size_color tempcolor = TIs_XZ[level][tempnearestidx_TInum][tempnearestidx_shifted];

					size_dist weightc(max_dist), weightp(max_dist);
					// PosHis weight
					size_idx tempPosIdx = tempnearestidx_shifted + poshisadd[ori];
					size_dist tempHisDiff = max(0.0f, 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]));				
					if (enhancelowposYN && (PosHis[level][tempPosIdx] < avgPosHis[level])) {
						tempHisDiff = 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]);
					}
					weightp = 1.0f / (1.0f + tempHisDiff * posweight[level]);
					// Color weight
					if (ColorHis_ON && !FIRSTRUN) {
						//size_dist coloroffset = max(0.0f, 1.0f * (tempcolor - Solid_Upper[level]) / (Pore_Lower[level] - Solid_Upper[level]));
						//if (!offsetYN) coloroffset = min(coloroffset, 1.0f);
						//size_dist tempColorHisDiff = max(0.0f, coloroffset *(poretotal_synthesis - poretotal_required) / poretotal_required);
						//weightc = 1.0f / (1.0f + tempColorHisDiff * factorC);
						size_dist tempColorHisDiff = max(0.0f, 1.0f*(ColorHis_synthesis[level][floor(colorhis_compressratio* tempcolor)] - ColorHis_exemplar[level][floor(colorhis_compressratio* tempcolor)]));
						weightc = 1.0f / (1.0f + tempColorHisDiff * colorweight[level]);
					}
					minweight = min(weightc, weightp);

					// discrete solver
					colorCand_y.push_back(tempcolor);
					posCand_y.push_back(tempnearestidx_shifted);
					posCand_tinum_y.push_back(tempnearestidx_TInum);

					//// modify weight according to fix layer
					if (FixedLayerDir > 0 && FixedLayerDir < 3 && FixedLayerDir != ori) minweight *= DirectionalWeight;

					// accumulate color
					size_dist weight = tempnearestweight * minweight;
					color_acc += weight * tempcolor;
					weight_acc += weight;
				}
			}

			//if (allcontinue) {
			//	printf("\n allcontinue_y: idx=%d, i=%d j=%d k=%d, permutation=%d, outsize=%d", idx, i, j, k, m_permutation[i2], OUTsize_);
			//	idxToCoord(idx, OUTsize_, i, j, k);
			//	printf("\nidx=%d, i=%d j=%d k=%d", idx, i, j, k);
			//	_getch();
			//}
			//if (posCand_y.size() == 0) {
			//	printf("\n posCand_y.size()=0");
			//	_getch();
			//}
		}//XZ


		//YZ
		{
			bool allcontinue(true);
			int ori = 2;
			if (!SIM2D_YN)
			for (size_idx l = start; l >= -end; --l) {
				size_idx tempx = tempx0ori[ori] + l * GRID;
				size_idx deltaxori[3] = { j - tempx , i - tempx , i - tempx };
				size_idx sumidx_tempx, tempidx3d;
				sumidx_tempx = trim(OUTsize_, tempx)*OUTsize2d_ + k;
				for (size_idx h = start; h >= -end; --h) {
					size_idx tempy = tempy0ori[ori] + h * GRID;
					size_idx deltayori[3] = { k - tempy ,  k - tempy , j - tempy };
					if (deltaxori[ori] < s1 || deltaxori[ori] > e1 || deltayori[ori] < s1 || deltayori[ori] > e1)
						continue;

					tempidx3d = sumidx_tempx + trim(OUTsize_, tempy)*OUTsize_;
					// get nearestidx_TIs
					size_idx tempnearestidx_TIs = nearestIdx_z[level][tempidx3d];
					size_dist tempnearestweight = nearestWeight_z[level][tempidx3d];
					size_idx tempnearestidx_Idxregular, tempnearestidx_TInum;
					TIsToRegular(tempnearestidx_TIs, TIsize2d_, tempnearestidx_Idxregular, tempnearestidx_TInum);

					// shift tempnearestidx_Idxregular
					size_idx coordx = tempnearestidx_Idxregular / TIsize_ + deltaxori[ori];
					size_idx coordy = tempnearestidx_Idxregular % TIsize_ + deltayori[ori];
					if (coordx < 0 || coordy < 0 || coordx >= TIsize_ || coordy >= TIsize_) continue;

					allcontinue = false;

					// get nearestidx2d, color
					size_idx tempnearestidx_shifted = TIsize_ * coordx + coordy;
					size_color tempcolor = TIs_YZ[level][tempnearestidx_TInum][tempnearestidx_shifted];

					size_dist weightc(max_dist), weightp(max_dist);
					// PosHis weight
					size_idx tempPosIdx = tempnearestidx_shifted + poshisadd[ori];
					size_dist tempHisDiff = max(0.0f, 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]));			
					if (enhancelowposYN && (PosHis[level][tempPosIdx] < avgPosHis[level])) {
						tempHisDiff = 1.0f*(PosHis[level][tempPosIdx] - avgPosHis[level]);
					}
					weightp = 1.0f / (1.0f + tempHisDiff * posweight[level]);
					// Color weight
					if (ColorHis_ON && !FIRSTRUN) {
						//size_dist coloroffset = max(0.0f, 1.0f * (tempcolor - Solid_Upper[level]) / (Pore_Lower[level] - Solid_Upper[level]));
						//if (!offsetYN) coloroffset = min(coloroffset, 1.0f);
						//size_dist tempColorHisDiff = max(0.0f, coloroffset *(poretotal_synthesis - poretotal_required) / poretotal_required);
						//weightc = 1.0f / (1.0f + tempColorHisDiff * factorC);
						size_dist tempColorHisDiff = max(0.0f, 1.0f*(ColorHis_synthesis[level][floor(colorhis_compressratio* tempcolor)] - ColorHis_exemplar[level][floor(colorhis_compressratio* tempcolor)]));
						weightc = 1.0f / (1.0f + tempColorHisDiff * colorweight[level]);
					}
					minweight = min(weightc, weightp);

					// discrete solver
					colorCand_z.push_back(tempcolor);
					posCand_z.push_back(tempnearestidx_shifted);
					posCand_tinum_z.push_back(tempnearestidx_TInum);

					//// modify weight according to fix layer
					if (FixedLayerDir > 0 && FixedLayerDir < 3 && FixedLayerDir != ori) minweight *= DirectionalWeight;

					// accumulate color
					size_dist weight = tempnearestweight * minweight;
					color_acc += weight * tempcolor;
					weight_acc += weight;
				}
			}

			//if (allcontinue) {
			//	printf("\n allcontinue_z: idx=%d, i=%d j=%d k=%d, permutation=%d, outsize=%d", idx, i, j, k, m_permutation[i2], OUTsize_);
			//	idxToCoord(idx, OUTsize_, i, j, k);
			//	printf("\nidx=%d, i=%d j=%d k=%d", idx, i, j, k);
			//	_getch();
			//}
			//if (posCand_z.size() == 0) {
			//	printf("\n posCand_z.size()=0");
			//	_getch();
			//}
		}//YZ


		// least solver
		color_avg = 1.0f * color_acc / weight_acc;	

		// discrete solver	
		size_dist minDis_x(10e7), minDis_y(10e7), minDis_z(10e7);
		size_idx closestIdx_x(0), closestIdx_y(0), closestIdx_z(0), closestIdx_tinum_x(0), closestIdx_tinum_y(0), closestIdx_tinum_z(0);
		int bestorder(0);
		// pointers
		vector<size_hiscount> *indexhis[3] = { &IndexHis_x[level], &IndexHis_y[level], &IndexHis_z[level] };
		vector<size_idx> *origin[3] = { &Origin_x[level], &Origin_y[level], &Origin_z[level] };
		vector<bool> *isunchanged[3] = { &isUnchanged_x[level], &isUnchanged_y[level], &isUnchanged_z[level] };
		size_dist *mindis[3] = { &minDis_x, &minDis_y , &minDis_z };
		size_idx *closestidx[3] = { &closestIdx_x , &closestIdx_y , &closestIdx_z };	
		size_idx *closestidx_tinum[3] = { &closestIdx_tinum_x, &closestIdx_tinum_y, &closestIdx_tinum_z };
		vector<size_color> *colorcand[3] = { &colorCand_x , &colorCand_y, &colorCand_z };
		vector<size_idx> *poscand[3] = { &posCand_x , &posCand_y, &posCand_z };
		vector<size_idx> *poscand_tinum[3] = { &posCand_tinum_x , &posCand_tinum_y, &posCand_tinum_z };
		// update origin, isUnchangeblock (add tempnearestidx_TInum)
		for (int ori = 0; ori < 3; ori++) {
			if (SIM2D_YN && ori != 0) continue;
			bestorder = 0;
			for (int s = 0; s < (*poscand[ori]).size(); s++) {
				size_dist tempColorDiff = fabs((*colorcand[ori])[s] - color_avg);
				if (tempColorDiff < *mindis[ori]) {
					*mindis[ori] = tempColorDiff;
					bestorder = s;
				}
				else if (tempColorDiff - *mindis[ori] < 1e-5) {	//if colordiff same, compare PosHis, then IndexHis
					if (PosHis[level][(*poscand[ori])[s] + poshisadd[ori]] < PosHis[level][(*poscand[ori])[bestorder] + poshisadd[ori]])
						bestorder = s;
					else if (PosHis[level][(*poscand[ori])[s] + poshisadd[ori]] == PosHis[level][(*poscand[ori])[bestorder] + poshisadd[ori]]
						&& (*indexhis[ori])[sparseIdx_TIs(level, (*poscand[ori])[s])] < (*indexhis[ori])[sparseIdx_TIs(level, bestorder)])
						bestorder = s;
				}
			}
			
			*closestidx[ori] = (*poscand[ori])[bestorder];
			*closestidx_tinum[ori] = (*poscand_tinum[ori])[bestorder];
			// add tempnearestidx_TInum to update origin, isUnchangeblock
			size_idx closestidx_TIs = *closestidx[ori] + (*closestidx_tinum[ori]) * TIsize2d_;
			if ((*origin[ori])[idx] != closestidx_TIs) {
				(*origin[ori])[idx] = closestidx_TIs;
				(*isunchanged[ori])[idx] = false;
			}
			else (*isunchanged[ori])[idx] = true;

			//if ((*origin[ori])[idx] >= MultiTIsNum*TIsize2d_) {
			//	printf("\noptimize: origin[%d][%d]=%d, close[ti%d][idx%d] best[%d]", ori, idx, (*origin[ori])[idx], (*poscand_tinum[ori])[bestorder], (*poscand[ori])[bestorder], bestorder);
			//	_getch();
			//}
		}//for (int ori = 0; ori < 3; ori++)


		// update color, pos
		size_color newcolor(0);
		size_idx newPos(0);												//PosHis size = 3TI		
		if (minDis_x <= minDis_y && minDis_x < minDis_z) {
			if (closestIdx_tinum_x >= MultiTIsNum || closestIdx_tinum_x < 0 || closestIdx_x<0 || closestIdx_x>=TIsize2d_) {
				printf("\nclosestIdx_tinum_x= %d, closestIdx_x= %d", closestIdx_tinum_x, closestIdx_x);
			}

			newcolor = TIs_XY[level][closestIdx_tinum_x][closestIdx_x];
			newPos = closestIdx_x;										// TI*0+Pos_x	
		}
		else if (minDis_y <= minDis_z && minDis_y < minDis_x) {
			if (closestIdx_tinum_y >= MultiTIsNum || closestIdx_tinum_y < 0 || closestIdx_y<0 || closestIdx_y >= TIsize2d_) {
				printf("\nclosestIdx_tinum_y= %d, closestIdx_y= %d", closestIdx_tinum_y, closestIdx_y);
			}

			newcolor = TIs_XZ[level][closestIdx_tinum_y][closestIdx_y];
			newPos = TIsize2d_ + closestIdx_y;							// TI*1+Pos_y
		}
		else if (minDis_z <= minDis_x && minDis_z < minDis_y) {
			if (closestIdx_tinum_z >= MultiTIsNum || closestIdx_tinum_z < 0 || closestIdx_z<0 || closestIdx_z >= TIsize2d_) {
				printf("\nclosestIdx_tinum_z= %d, closestIdx_z= %d", closestIdx_tinum_z, closestIdx_z);
			}
			
			newcolor = TIs_YZ[level][closestIdx_tinum_z][closestIdx_z];
			newPos = TIsize2d_ * 2 + closestIdx_z;						// TI*2+Pos_z			
		}
		else {															// if minDis_z==minDis_y==minDis_x rand.
			if (closestIdx_tinum_x >= MultiTIsNum || closestIdx_tinum_x < 0 || closestIdx_x<0 || closestIdx_x >= TIsize2d_) {
				printf("\nclosestIdx_tinum_x= %d, closestIdx_x= %d", closestIdx_tinum_x, closestIdx_x);
			}
			if (closestIdx_tinum_y >= MultiTIsNum || closestIdx_tinum_y < 0 || closestIdx_y<0 || closestIdx_y >= TIsize2d_) {
				printf("\nclosestIdx_tinum_y= %d, closestIdx_y= %d", closestIdx_tinum_y, closestIdx_y);
			}
			if (closestIdx_tinum_z >= MultiTIsNum || closestIdx_tinum_z < 0 || closestIdx_z<0 || closestIdx_z >= TIsize2d_) {
				printf("\nclosestIdx_tinum_z= %d, closestIdx_z= %d", closestIdx_tinum_z, closestIdx_z);
			}

			switch (rand() % 3){
			case(0) : newcolor = TIs_XY[level][closestIdx_tinum_x][closestIdx_x]; newPos = closestIdx_x;					break;
			case(1) : newcolor = TIs_XZ[level][closestIdx_tinum_y][closestIdx_y]; newPos = TIsize2d_ + closestIdx_y;		break;
			case(2) : newcolor = TIs_YZ[level][closestIdx_tinum_z][closestIdx_z]; newPos = TIsize2d_ * 2 + closestIdx_z;	break;
			}
		}

		// update poshis
		size_idx formerPos = SelectedPos[level][idx];										
		size_hiscount& addressformerPos = PosHis[level][formerPos];	
		if (formerPos < SizePosHis && formerPos >= 0 && addressformerPos>0)
#pragma omp atomic
			addressformerPos--;
		size_hiscount& addressnewPos = PosHis[level][newPos];
#pragma omp atomic
		addressnewPos++;
		
		// update SelectedPos
		SelectedPos[level][idx] = newPos;													
				
		//update colorhis
		if (ColorHis_ON && !FIRSTRUN) {
//			if (m_volume[level][idx] <= Solid_Upper[level] && newcolor > Solid_Upper[level])
//#pragma omp atomic
//				poretotal_synthesis++;
//			else if (m_volume[level][idx] > Solid_Upper[level] && newcolor <= Solid_Upper[level])
//				if (poretotal_synthesis>0)
//#pragma omp atomic
//					poretotal_synthesis--;
			size_hiscount& addressfomerColor = ColorHis_synthesis[level][floor(colorhis_compressratio* m_volume[level][idx])];	// update ColorHis
			if (addressfomerColor>0)
#pragma omp atomic
				addressfomerColor--;
			size_hiscount& addressnewColor = ColorHis_synthesis[level][floor(colorhis_compressratio* newcolor)];
#pragma omp atomic
			addressnewColor++;
		}
		
		// update m_volume
		m_volume[level][idx] = newcolor;

	}//for (size_idx i2 = 0; i2 < Size; ++i2) 


	//if (!FIRSTRUN && ColorHis_ON && (HisEqYN || level == MULTIRES-1) && poretotal_required>0)
	//	printf("loss=%d ", 100*(poretotal_synthesis - poretotal_required) / poretotal_required);

	if (FIRSTRUN) {
		if (ColorHis_ON) initColorHis_synthesis(level);
		FIRSTRUN = false;
	}
}


// ========= Color Histogram for optimize step =======

void DoPAR::initColorHis_exemplar() {
	for (int level = 0; level < MULTIRES; level++) {
		size_idx Size2d = TIsize[level] * TIsize[level];
		size_idx Size3d = TIsize[level] * TIsize[level] * TIsize[level];

		ColorHis_exemplar[level].resize(ColorHis_BinNum, 0);

		for (int n = 0; n < MultiTIsNum; n++) {
			vector<size_color>* p[3] = { &TIs_XY[level][n], &TIs_XZ[level][n], &TIs_YZ[level][n] };
			for (int ori = 0; ori < 3; ++ori) {
				for (size_idx i = 0; i < Size2d; ++i) {
					size_color c = (*p[ori])[i];
					ColorHis_exemplar[level][floor(c*colorhis_compressratio)] ++;
				}
			}
		}
		
		int actualBinNum = 0;
		for (int bin = 0; bin < ColorHis_BinNum; bin++) {
			if (ColorHis_exemplar[level][bin] >0) actualBinNum++;
		}
		cout << endl << "actualBinNum= " << actualBinNum;

		//!!!need to rescale to fit ColorHis_synthesis!!!
		float factor = Size3d*1.0f / (3 * Size2d * MultiTIsNum);

		for (int bin = 0; bin < ColorHis_BinNum; bin++) {
			ColorHis_exemplar[level][bin] = ceil(ColorHis_exemplar[level][bin]*factor);
			if (ColorHis_exemplar[level][bin]>0) ColorHis_exemplar[level][bin] -= 1;		//similar to IndexHis,PosHis
		}
	}
}

void DoPAR::initColorHis_synthesis(int level) {
	ColorHis_synthesis[level].resize(ColorHis_BinNum, 0L);
	size_idx Size3d = OUTsize[level] * OUTsize[level] * OUTsize[level];

	for (size_idx i = 0; i < Size3d; i++) {
		ColorHis_synthesis[level][floor(m_volume[level][i]*colorhis_compressratio)]++;
	}
}

// ================ output ====================
void DoPAR::crop3Dmodel(int level, int cropl, vector<uchar>&model) {
	assert(pow(model.size(), 1.0 / 3.0) == OUTsize[level]);
	size_idx OUTsize_ = OUTsize[level], idx, tempidxi, tempidxij;
	size_idx Sx = 1;
	if (!SIM2D_YN) Sx = OUTsize_;
	size_idx croppedsize = (OUTsize_ - 2 * cropl)*(OUTsize_ - 2 * cropl)*max(1L, Sx - 2 * cropl);
	vector<uchar> tempoutput;
	tempoutput.resize(croppedsize);

	size_idx n = 0;
	for (int i = 0; i < Sx; i++) {
		if (!SIM2D_YN && (i<cropl || i>Sx - cropl - 1)) continue;
		tempidxi = i* OUTsize_*OUTsize_;
		for (int j = 0; j < OUTsize_; j++) {
			if (j<cropl || j>OUTsize_ - cropl - 1) continue;
			tempidxij = tempidxi + j * OUTsize_;
			for (int k = 0; k < OUTsize_; k++) {
				if (k<cropl || k>OUTsize_ - cropl - 1) continue;
				idx = tempidxij + k;
				tempoutput[n++] = model[idx];
			}
		}
	}
	assert(n == croppedsize);
	swap(tempoutput, model);
}

void DoPAR::outputmodel(int level) {

	vector<uchar> tempUchar(m_volume[level].begin(), m_volume[level].end());
	string tempoutputfilename = outputfilename;
	size_idx OUTsize_ = OUTsize[level];

	bool cropYN = true;
	if (cropYN && level == MULTIRES - 1) {
		int cropl = pow(2, MULTIRES - 2)* blockSize[0];
		crop3Dmodel(level, cropl, tempUchar);
		OUTsize_ = OUTsize_ - 2 * cropl;
	}


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
			binaryUchar(tempUchar, (Solid_Upper[level] + Pore_Lower[level]) / 2);
			//vector<short> tempshort(m_volume[level].begin(), m_volume[level].end());					
			//binaryUchar(tempshort, tempUchar, (Solid_Upper[MULTIRES-1] + Pore_Lower[MULTIRES - 1]) / 2);// binary thresholded to 0&255
			tempM = Mat(tempUchar, true).reshape(1, tempM.rows);
			i = 1;
			tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + ".png";
			nonrepeatFPName = tempoutputfilename;
			while (fileExists(nonrepeatFPName) == true) {
				nonrepeatFPName = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_" + to_string(i) + ".png";
				i++;
			}
			imwrite(nonrepeatFPName, tempM);
		}
	}
	else {//3D
		if (GenerateTI) {
			tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + "DM.RAW";
			Write(outputpath + tempoutputfilename, tempUchar);
		}


		if (DMtransformYN) {
			// binary model
			binaryUchar(tempUchar, (Solid_Upper[level] + Pore_Lower[level]) / 2);
			//vector<short> tempshort(m_volume[level].begin(), m_volume[level].end());
			//binaryUchar(tempshort, tempUchar, (Solid_Upper[level] + Pore_Lower[level]) / 2);						// binary thresholded to 0&255
			//binaryUchar(tempshort, tempUchar, (Solid_Upper[MULTIRES - 1] + Pore_Lower[MULTIRES - 1]) / 2);

		}
		if (level == MULTIRES - 1) tempoutputfilename = outputfilename + ".RAW";
		else tempoutputfilename = outputfilename + "_Size" + to_string(OUTsize_) + ".RAW";
		Write(outputpath + tempoutputfilename, tempUchar);
	}


	cout << endl << "output done. output size=" << OUTsize_;

}

void DoPAR::writeHistogram(int level) {
	size_idx OUTsize_ = OUTsize[level];
	size_idx TIsize_ = TIsize[level];
	size_idx blockSize_ = blockSize[level];
	size_idx TIsize2d_ = TIsize_*TIsize_;
	size_idx OUTsize2d_ = OUTsize_*OUTsize_;

	size_idx Sx = OUTsize_; if (SIM2D_YN) Sx = 1;
	int cropedIndexHisStartX = blockSize_ * 0.25;
	int cropedIndexHisWidth = TIsize_ / 2 - blockSize_ / 2 + 1;
	int cropedIndexHisStartY = blockSize_ * 0.25;
	int cropedIndexHisHeight = TIsize_ / 2 - blockSize_ / 2 + 1;
	int cropedPosHisStartX = 1;
	int cropedPosHisWidth = TIsize_ - 2;
	int cropedPosHisStartY = 1;
	int cropedPosHisHeight = TIsize_ - 2;
	size_idx idx_i, idx_j, idx3d, idx2d;
	Mat tempMat;
	ostringstream name;
	string outputMainFileName = outputfilename.substr(0, outputfilename.find('.'));

	unsigned short deltaIndexCount(1), deltaPosCount(1);
	vector<unsigned short> Index_x, Index_y, Index_z;
	Index_x.resize(TIsize2d_, 0); Index_y.resize(TIsize2d_, 0); Index_z.resize(TIsize2d_, 0);
	vector<unsigned short> pos_x, pos_y, pos_z;
	pos_x.resize(TIsize2d_, 0);	pos_y.resize(TIsize2d_, 0);	pos_z.resize(TIsize2d_, 0);

	name.str("");
	tempMat = Mat(TIsize_ * 0.5, TIsize_ * 0.5, CV_16UC1);
	vector<unsigned short> tempIHx = vector<unsigned short>(IndexHis_x[level].begin(), IndexHis_x[level].end());
	tempMat = Mat(tempIHx, true).reshape(1, tempMat.rows);
	Mat cropedIndexHisMat_x = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
	//Mat cropedIndexHisMat_x = tempMat;

	name << outputMainFileName << "_x" << "_IndexHis_L" << level << ".png";
	string tempname = name.str();
	int i(1);
	while (fileExists(tempname) == true) {
		tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
		i++;
	}
	if (SIM2D_YN || (FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(tempname, cropedIndexHisMat_x);	//must be unsigned [short]!		

	if (!SIM2D_YN && !(FNameXY == FNameXZ && FNameXY == FNameYZ)) {
		name.str("");
		tempMat = Mat(TIsize_ * 0.5, TIsize_ * 0.5, CV_16UC1);
		vector<unsigned short> tempIHy = vector<unsigned short>(IndexHis_y[level].begin(), IndexHis_y[level].end());
		tempMat = Mat(tempIHy, true).reshape(1, tempMat.rows);
		Mat cropedIndexHisMat_y = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
		name << outputMainFileName << "_y" << "_IndexHis_L" << level << ".png";
		tempname = name.str();
		i = 1;
		while (fileExists(tempname) == true) {
			tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
			i++;
		}
		//imwrite(name.str(), cropedIndexHisMat_y);

		name.str("");
		tempMat = Mat(TIsize_ * 0.5, TIsize_ * 0.5, CV_16UC1);
		vector<unsigned short> tempIHz = vector<unsigned short>(IndexHis_z[level].begin(), IndexHis_z[level].end());
		tempMat = Mat(tempIHz, true).reshape(1, tempMat.rows);
		Mat cropedIndexHisMat_z = tempMat(Rect(cropedIndexHisStartX, cropedIndexHisStartY, cropedIndexHisWidth, cropedIndexHisHeight));
		name << outputMainFileName << "_z" << "_IndexHis_L" << level << ".png";
		tempname = name.str();
		i = 1;
		while (fileExists(tempname) == true) {
			tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
			i++;
		}
		//imwrite(name.str(), cropedIndexHisMat_z);

		name.str("");
		tempMat = cropedIndexHisMat_x + cropedIndexHisMat_y + cropedIndexHisMat_z;
		name << outputMainFileName << "_IndexHis_merged.png";
		tempname = name.str();
		i = 1;
		while (fileExists(tempname) == true) {
			tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
			i++;
		}
		imwrite(name.str(), tempMat);
	}


	//if (SIM2D_YN) return;
	for (size_idx i = 0; i < Sx; i += 1) {									//PosHis not sparsed
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
	Mat cropedPosHisMat_x = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
	//Mat cropedPosHisMat_x = tempMat;
	name << outputMainFileName << "_x" << "_PosHis_L" << level << ".png";
	tempname = name.str();
	i = 1;
	while (fileExists(tempname) == true) {
		tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
		i++;
	}
	if (SIM2D_YN || (FNameXY == FNameXZ && FNameXY == FNameYZ)) imwrite(tempname, cropedPosHisMat_x);

	if (!SIM2D_YN && !(FNameXY == FNameXZ && FNameXY == FNameYZ)) {
		name.str("");
		tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
		tempMat = Mat(pos_y, true).reshape(1, tempMat.rows);
		Mat cropedPosHisMat_y = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
		name << outputMainFileName << "_y" << "_PosHis_L" << level << ".png";
		tempname = name.str();
		i = 1;
		while (fileExists(tempname) == true) {
			tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
			i++;
		}
		//imwrite(name.str(), cropedPosHisMat_y);

		name.str("");
		tempMat = Mat(TIsize_, TIsize_, CV_16UC1);
		tempMat = Mat(pos_z, true).reshape(1, tempMat.rows);
		Mat cropedPosHisMat_z = tempMat(Rect(cropedPosHisStartX, cropedPosHisStartY, cropedPosHisWidth, cropedPosHisHeight));
		name << outputMainFileName << "_z" << "_PosHis_L" << level << ".png";
		tempname = name.str();
		i = 1;
		while (fileExists(tempname) == true) {
			tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
			i++;
		}
		//imwrite(name.str(), cropedPosHisMat_z);

		name.str("");
		tempMat = cropedPosHisMat_x + cropedPosHisMat_y + cropedPosHisMat_z;
		name << outputMainFileName << "_PosHis_L" << level << "_merged.png";
		tempname = name.str();
		i = 1;
		while (fileExists(tempname) == true) {
			tempname = tempname.substr(0, tempname.find('.')) + "_" + to_string(i) + ".png";
			i++;
		}
		imwrite(name.str(), tempMat);
	}

	cout << endl << "croped Histograms are plotted.";
}

