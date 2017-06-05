#include "stdafx.h"
#include "DoPAR.h"

DoPAR::DoPAR()
{
	srand((unsigned)time(NULL));
	mersennetwistergenerator = mt19937(randomseed());
	probabilitydistribution = uniform_real_distribution<double>(0.0, 1.0);

	// [begin] multi-res memory allocation-------------------------------------------
	m_exemplar_x.resize(MULTIRES);
	m_exemplar_y.resize(MULTIRES);
	m_exemplar_z.resize(MULTIRES);

	m_volume.resize(MULTIRES);

	// [end] multi-res memory allocation---------------
}

DoPAR::~DoPAR()
{
	//for (int level = 0; level < MULTIRES; ++level) {
	//	if (mp_neighbor_kdTree_x[level] != NULL) delete mp_neighbor_kdTree_x[level];
	//	if (mp_neighbor_kdTree_y[level] != NULL) delete mp_neighbor_kdTree_y[level];
	//	if (mp_neighbor_kdTree_z[level] != NULL) delete mp_neighbor_kdTree_z[level];
	//	if (mp_neighbor_pca_average_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_x[level]);
	//	if (mp_neighbor_pca_average_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_y[level]);
	//	if (mp_neighbor_pca_average_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_z[level]);
	//	if (mp_neighbor_pca_projected_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x[level]);
	//	if (mp_neighbor_pca_projected_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y[level]);
	//	if (mp_neighbor_pca_projected_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z[level]);
	//	if (mp_neighbor_pca_eigenvec_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x[level]);
	//	if (mp_neighbor_pca_eigenvec_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y[level]);
	//	if (mp_neighbor_pca_eigenvec_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z[level]);
	//}
	//annClose();
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
			//FNameAddition.clear(); FNameAddition.reserve(20);
			//TIs.reserve(20);
			//for (int i = 3; i < ParV.size(); i++){
			//	FNameAddition.push_back(workpath + ParV[i]);
			//}
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

	//if (ResLines.size() > ++Row) {
	//	vector<string> ParV;
	//	GetNextRowParameters(Row, ResLines, ParV);
	//	if (BIMODAL_ON && !DISTANCEMAP_ON) {
	//		if (ParV.size() > 0) {
	//			if (ParV.size() > 0) { Solid_Upper = atoi(ParV[0].c_str()); }
	//			if (ParV.size() > 1) { Pore_Lower = atoi(ParV[1].c_str()); }
	//		}
	//	}
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
	/////////////////////////resolution, upper limit of porosity, template type, template radius, minimum pattern existence, scan type, iteration
	//if (ResLines.size() > ++Row) {
	//	vector<string> ParV;
	//	GetNextRowParameters(Row, ResLines, ParV);
	//	if (ParV.size() > 0) {
	//		if (ParV.size() > 0) patchtemplate = atoi(ParV[0].c_str());
	//		//cotemplate = 2 * patchtemplate;
	//		if (ParV.size() > 1) overlap = atoi(ParV[1].c_str());
	//		if (ParV.size() > 2) CLextendr = atoi(ParV[2].c_str());
	//		if (ParV.size() > 3) porelppercentage = atof(ParV[3].c_str());
	//		if (ParV.size() > 4) CLKeep = atoi(ParV[4].c_str());
	//		if (ParV.size() > 5) OLCLKeep = atoi(ParV[5].c_str());
	//		fillgap = overlap;
	//
	//		//if (ParV.size() > 2) patchcandidate = atoi(ParV[2].c_str());
	//		//if (ParV.size() > 4) globalsplitYN = atoi(ParV[4].c_str());
	//		//if (ParV.size() > 4) fillgap = atoi(ParV[4].c_str());
	//		//if (ParV.size() > 4) minEAsamplerate = atof(ParV[4].c_str());
	//		//if (ParV.size() > 5) maxinterval = atoi(ParV[5].c_str());
	//		//if (ParV.size() > 6) edgewidth = atoi(ParV[6].c_str());
	//		//if (ParV.size() > 2) scanprop = atof(ParV[2].c_str());
	//		//if (ParV.size() > 2) Tweight = atof(ParV[2].c_str());
	//	}
	//	////outputfilename will include parameters!
	//	//string Morphologyinfo = "";
	//	string sampleinfo = "T" + to_string(patchtemplate) + "OL" + to_string(overlap) + "Sp" + to_string(int(porelppercentage * 100)) + "Excl" + to_string(CLextendr) + "CK" + to_string(CLKeep) + "OK" + to_string(OLCLKeep); /*+ "Es" + to_string(int(minEAsamplerate * 100)) + "Cd" + to_string(patchcandidate)*//* + "EW" + to_string((1 + edgewidth) * 2)*/ /*+ "FL" + to_string(fillgap)*/;//+ "Mb" + to_string(minimumsampleblock) + "En" + to_string(int(entropyshreshold * 100)) + "Eg" + to_string(int(edgeareasamplepercentage * 100))
	//	//if (minEAsamplerate == 0) sampleinfo = "T" + to_string(patchtemplate) + "OL" + to_string(overlap) + "Sp" + to_string(int(samplepercentage * 100)) /*+ "NoEs"*/ + "Cd" + to_string(patchcandidate) /*+ "EW" + to_string((1 + edgewidth) * 2) *//*+ "FL" + to_string(fillgap)*/;
	//	outputfilename = tempoutputfilename + "_" + sampleinfo + tempoutputformat;//"R" + to_string(templateradius) +
	//}
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
const int DoPAR::blockSize[MULTIRES] = {8, 6, 6};
ANNidx DoPAR::TEXSIZE[MULTIRES];
const ANNdist DoPAR::factor[MULTIRES] = { 64.0f, 128.0f, 256.0f };
const short DoPAR::MAXITERATION[MULTIRES] = {40, 20, 10};

////////////////////////////////////////////////////////////////////////////////////////


//float DoPAR::Solid_Upper;				//for redistribute distancemap model. e.g. 192: 0-192 solid
//float DoPAR::Pore_Lower;				//for redistribute distancemap model
//short DoPAR::DistanceThreshold;			//for binarise distance model.  DistanceThreshold=(Solid_Upper+Pore_Lower)/2
//vector<short> DoPAR::ProjectDMapMaxBins;


void DoPAR::DoANNOptimization() {
	init();

	time_t StartTime;
	time(&StartTime);

	for (int curlevel = 0; curlevel < MULTIRES; curlevel++) {
		cout << endl << "=============level: " << curlevel << "===============";
		initPermutation(curlevel);

		int convergencecount(0), energyincreasecount(0);
		for (int loop = 0; loop < MAXITERATION[curlevel]; loop++) {
			if ((curlevel==0 && loop%10==0) || (curlevel == 1 && loop % 5 == 0) || (curlevel==2 && loop%3==0) || (curlevel > 2))
				cout << endl << "iteration: " << loop;

			if(searchVolume(curlevel)) 
				break;

			optimizeVolume(curlevel);

		}//loop in one level
		
		if (curlevel >= MULTIRES - 1 || TEXSIZE[curlevel] >= 128) {//draw histogram graph && ouput model	
			outputmodel(curlevel);

			ANNidx cols = TEXSIZE[curlevel];
			ANNidx rows = 3 * cols;

			//if (POSITIONHIS_ON)	//show position histogram				
			//	writeHistogram(true, curlevel, m_positionhistogram_synthesis[curlevel], rows, cols, "PosHis_" + parameterstring + "_L" + to_string(curlevel) + ".png");
		}

		if (curlevel < MULTIRES - 1) {//level up
			upsampleVolume(curlevel);
			//if (COLORHIS_ON || DISCRETETHRESHOLD_ON) { initHistogram_synthesis(curlevel + 1); }
			//if (POSITIONHIS_ON) {	initPositionHistogram_synthesis(curlevel + 1);	}		
			cleardata(curlevel);	//release vector
		}
	}

	time_t NewTime;
	time(&NewTime);
	cout << endl << "Total reconstruction time: " << long(NewTime - StartTime);
}

void DoPAR::init() {
	//load TI
	if (!loadExemplar()) return;		
	
	//ostringstream name;
	//Mat DM1 = Mat(TEXSIZE[0], TEXSIZE[0], CV_8UC1);
	//DM1 = Mat(m_exemplar_x[0], true).reshape(1, DM1.rows);
	//name << "DM" << "_L" << 0;
	//name << ".png";
	//imwrite(name.str(), DM1);	name.str(""); //name.clear();not necessary
	//Mat DM2 = Mat(TEXSIZE[1], TEXSIZE[1], CV_8UC1);
	//DM2 = Mat(m_exemplar_x[1], true).reshape(1, DM2.rows);
	//name << "DM" << "_L" << 1;
	//name << ".png";
	//imwrite(name.str(), DM2);	name.str(""); //name.clear();not necessary

	//load Model
	if (!loadVolume()) return;			


	//K-Coherence
	unsigned long t1 = GetTickCount();
	computeKCoherence();				
	unsigned long t2 = GetTickCount();
	cout << "Computing KCoherence in "	<< static_cast<double>(t2 - t1) / (1000.0)	<< " s." << endl; 


	//initial Histogram
	for (int i = 0; i < MULTIRES; i++){
		deltaIndexHis[i] = (1.0f * (TEXSIZE[i] - blockSize[i] + 2)*(TEXSIZE[i] - blockSize[i] + 2)) / (TEXSIZE[i] * TEXSIZE[i] * TEXSIZE[i]);
		deltaPosHis[i] = 1.0f / TEXSIZE[i];
	}

	

}

void DoPAR::initPermutation(int level) {// random permutation (precomputed)
	ANNidx Size = TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
	m_permutation_xyz.clear();
	m_permutation_xyz.resize(Size);
	for (ANNidx i = 0; i <Size; ++i) {
		m_permutation_xyz[i] = i;
	}
}

bool DoPAR::loadExemplar() {
	
	/////////////////////////////////////////////////////////////
	//exemplar_x --> YZ, exemplar_y --> ZX, exemplar_z --> XY
	//using imagej, XY slice is XY, ememplar_z
	//ZX slice can be attained by: 1. reslice top + flip virtical 2. then rotate 90 degrees left
	//YZ slice is done by: reslice left
	/////////////////////////////////////////////////////////////

	//---------------convert Mat to IplImage*---------------
	Mat matxy = cv::imread(FNameXY, CV_LOAD_IMAGE_GRAYSCALE); // ti grayscale
	Mat matxz = cv::imread(FNameXZ, CV_LOAD_IMAGE_GRAYSCALE);
	Mat matyz = cv::imread(FNameYZ, CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* img_x = cvCloneImage(&(IplImage)matxy);
	IplImage* img_y = cvCloneImage(&(IplImage)matxz);
	IplImage* img_z = cvCloneImage(&(IplImage)matyz);


	// build image pyramid
	int img_depth = img_x->depth;
	int img_nChannels = img_x->nChannels;
	for (int level = MULTIRES - 1; level >= 0; --level) {
		// size registration
		TEXSIZE[level] = img_x->width;
		// check multi-level size correct
		if (TEXSIZE[MULTIRES - 1] % (ANNidx)pow(2, MULTIRES - 1) != 0) { cout << endl << "TI size not right for multi-level"; _getch(); exit(1); }

		// [begin] memory allocation -------------------------------------------
		m_exemplar_x[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_y[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_z[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_volume[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		// [end] memory allocation -------------------------------------------

		for (int v = 0; v < TEXSIZE[level]; ++v) {
			for (int u = 0; u < TEXSIZE[level]; ++u) {
				ANNidx index = (TEXSIZE[level] * v + u);
				ANNidx index2 = (TEXSIZE[level] * v + u);
				m_exemplar_x[level][index] = (unsigned char)img_x->imageData[index2];
				m_exemplar_y[level][index] = (unsigned char)img_y->imageData[index2];
				m_exemplar_z[level][index] = (unsigned char)img_z->imageData[index2];
			}
		}


		if (level == 0) continue;
		// go to the coarser level
		cv::Mat tempmat(TEXSIZE[level] / 2, TEXSIZE[level] / 2, CV_8UC1);
		cv::resize(matxy, tempmat, tempmat.size(), 0, 0, INTER_AREA);
		IplImage* img_next_x = cvCloneImage(&(IplImage)tempmat);
		cv::resize(matxz, tempmat, tempmat.size(), 0, 0, INTER_AREA);
		IplImage* img_next_y = cvCloneImage(&(IplImage)tempmat);
		cv::resize(matyz, tempmat, tempmat.size(), 0, 0, INTER_AREA);
		IplImage* img_next_z = cvCloneImage(&(IplImage)tempmat);


		cvReleaseImage(&img_x);
		cvReleaseImage(&img_y);
		cvReleaseImage(&img_z);
		img_x = img_next_x;
		img_y = img_next_y;
		img_z = img_next_z;
	}
	cvReleaseImage(&img_x);
	cvReleaseImage(&img_y);
	cvReleaseImage(&img_z);

	cout << endl << "load TIs done.";
	return true;
}

bool DoPAR::loadVolume() {
	//----------------convert Model(vector<uchar>) to m_volume (vector<vector<int>> (multires,x*y*z))
	//load from Model, later can also load from file		//level 0
	if (fileExists(modelFilename3D.c_str()) == true) {
		vector<uchar> model = load3Dmodel(modelFilename3D.c_str());
		if (model.size() != TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]) { cout << endl << "Loaded Model size=" << model.size() << " should be=" << TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]; _getch(); exit(1); }

		m_volume[0] = vector<ANNcoord>(model.begin(), model.end());	
		cout << endl << "load 3D model done.";
	}
	else {
		cout << endl << "3D initial model doesn't exist. Use Random initial.";
		InitRandomVolume(0);				//no need to convert again.
	}
	return true;
}
void DoPAR::InitRandomVolume(int level) {
	vector<ANNcoord>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	for (int xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
		ANNidx index2 = (rand() % (TEXSIZE[level] * TEXSIZE[level]));
		int ori = rand() % 3;

		m_volume[level][xyz] = p[ori]->operator[](index2);
	}
}

void DoPAR::outputmodel(int level) {
	//if (true) {
	//	DynamicThreshold(level);
	//}

	vector<uchar> tempmodel;
	tempmodel = vector<uchar>(m_volume[level].begin(), m_volume[level].end());
	string tempoutputfilename = outputfilename;
	if (level != MULTIRES - 1) tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "_L" + to_string(level) + ".RAW";

	Write(outputpath + tempoutputfilename, tempmodel);

	////============= Convert to binary model [leve]==========================
	//if (DISTANCEMAP_ON){
	//	vector<char> tempchar(tempmodel.size());
	//	//output an extra binary model
	//	vector<short> shortmodel(tempmodel.begin(), tempmodel.end());
	//	BinariseThreshold(shortmodel, tempchar, Solid_Upper);
	//	//BinariseImg(shortmodel, porosityTI);
	//	tempmodel = vector<uchar>(tempchar.begin(), tempchar.end());
	//	tempoutputfilename = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_binary.RAW";
	//	Write(outputpath + tempoutputfilename, tempmodel);
	//}
	cout << endl << "output done.";
}



void DoPAR::cleardata(int level) {
	m_volume[level].clear();


}


//========K-Coherence Search=====================//
void DoPAR::computeKCoherence(){
	cout << endl << "K-coherence...";

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
		delete kdTree_x;		delete kdTree_y;		delete kdTree_z;
		annDeallocPts(p_source_x);		annDeallocPts(p_source_y);		annDeallocPts(p_source_z);
		annDeallocPt(queryPt_x);		annDeallocPt(queryPt_y);		annDeallocPt(queryPt_z);
	}
	delete[] ann_index_x;		delete[] ann_index_y;		delete[] ann_index_z;
	delete[] ann_dist_x;		delete[] ann_dist_y;		delete[] ann_dist_z;

	cout << endl << "K-coherence done.";
}


//================= phase 1: search ===========================

bool DoPAR::searchVolume(int level) {
	long time_start = clock();	cout << endl << "search...";

	const ANNidx TEXSIZE_ = TEXSIZE[level];
	const ANNidx blockSize_ = blockSize[level];
	const ANNdist min_dist = 0.00001f;
	const ANNidx Sx = TEXSIZE_;
	const ANNidx Sy = TEXSIZE_;
	const ANNidx Sz = TEXSIZE_;
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const ANNidx R = static_cast<ANNidx>(blockSize_ / 2);	
	vector<ANNidx> compareIdx; 
	ANNdist curDis, curError, IndexHisWeight;
	CvMat* current_neighbor = cvCreateMat(1, blockSize_*blockSize_, CV_32F);		//rows = 1, cols = dimesnion
	bool isUnchanged = true;
	int direction;

	//For Z
	direction = 2;
	shuffle(m_permutation_xyz.begin(), m_permutation_xyz.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation_xyz[i2];			//[i][j][k] idx=i*Sxy+j*Sx+k
		ANNidx k = idx % Sx;
		ANNidx j = (idx / Sx) % Sy;
		ANNidx i = idx / Sxy;
		if (i % GRID != 0 || j % GRID != 0)	continue;						//sparse grid			
		if (isUnchangedBlock(level, direction, i, j, k)) continue;			//check neighbours all unchanged or not

		ANNidx index = 0;
		for (ANNidx du = -R; du < R; ++du) {	//N is neighbourhood size.
			ANNidx VCurIdx1 = Sxy * trimIndex(level, i + du) + k;
			for (ANNidx dv = -R; dv < R; ++dv) {
				ANNidx index2 = VCurIdx1 + Sx * trimIndex(level, j + dv);		//[i+du][j+dv][k]
				cvmSet(current_neighbor, 0, index, m_volume[level][index2]);
				index++;
			}
		}

		ANNdist minError = 10e9, minDis = 10e9;
		ANNidx bestTIIdx;
		ANNidx eposx, eposy;
		ANNidx temp2didx, temp3didx, tempTIidx;
		ANNidx sumidx_posx;
		int compareNum;
		compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		for (int u = -R; u < R; ++u) {
			sumidx_posx = trimIndex(level, i + u)*Sxy;
			for (int v = -R; v < R; ++v) {
				temp3didx = sumidx_posx + trimIndex(level, j + v)*Sx + k;			//[posx][posy][k]
				eposx = (Origin_z[level][temp3didx] / TEXSIZE_) - u;
				eposy = (Origin_z[level][temp3didx] % TEXSIZE_) - v;
				if (!(eposx >= R && eposx < Sx - R && eposy >= R && eposy < Sy - R)) continue;

				tempTIidx = eposx*Sx + eposy;
				for (int l = 0; l < COHERENCENUM; ++l) {							//[0,COHERENCENUM]				
					temp2didx = KCoherence_z[level][tempTIidx][l];
					int p = 0;
					compareNum = compareIdx.size();
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx)	break;
					}
					if (p < compareNum)	continue;

					IndexHisWeight = 1.0f + factor[level] * max(0.0f, IndexHis_z[level][temp2didx] - 1.0f);			//IndexHis needs sparse grid
					curDis = getFullDistance(level, m_exemplar_z[level], temp2didx, current_neighbor);
					curError = IndexHisWeight * curDis;

					if (minError > curError) {								//min error			
						minError = curError;
						minDis = curDis;									
						bestTIIdx = temp2didx;
					}
					compareIdx.push_back(temp2didx);
				}//for (int l = 0; l < COHERENCENUM; ++l){	
			}//for (int v = -R; v < R; ++v){
		}//for (int u = -R; u < R; ++u){

		if (minError < 10e9) {
			//update NearestIndex, IndexHis, store EuDis
			if (!setNearestIndex(level, nearestIdx_z[level], nearestWeight_z[level], IndexHis_z[level], idx, bestTIIdx, minDis))	
				isUnchanged = false;
		}
		else {
			cout << "bad point...\n";	_getch();
			//bestCoord = xexemplar->getRandomPos();
			//setNearestIndex(direction, i, j, k, bestCoord, xexemplar, min_dist);	
			//isUnchanged = false;
		}
	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {

	//For Y	
	direction = 1;
	shuffle(m_permutation_xyz.begin(), m_permutation_xyz.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation_xyz[i2];			//[i][j][k] idx=i*Sxy+j*Sx+k
		ANNidx k = idx % Sx;				
		ANNidx j = (idx / Sx) % Sy;			
		ANNidx i = idx / Sxy;				
		if (i % GRID != 0 || k % GRID != 0)	continue;						//sparse grid			
		if (isUnchangedBlock(level, direction, i, j, k)) continue;			//check neighbours all unchanged or not
		
		ANNidx index = 0;		
		ANNidx TXy = Sx * j;
		for (ANNidx du = -R; du < R; ++du) {	//N is neighbourhood size.
			ANNidx VCurIdx2 = Sxy * trimIndex(level, i + du) + TXy;				
			for (ANNidx dv = -R; dv < R; ++dv) {
				ANNidx index2 = VCurIdx2 + trimIndex(level, k + dv);		//[i+du][j][k+dv]
				cvmSet(current_neighbor, 0, index, m_volume[level][index2]);					
				index++;
			}
		}
		
		ANNdist minError = 10e9, minDis = 10e9;
		ANNidx bestTIIdx;
		ANNidx eposx, eposy;
		ANNidx temp2didx, temp3didx, tempTIidx;
		ANNidx sumidx_j = j*Sx;
		ANNidx sumidx_posx;
		int compareNum;
		compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		for (int u = -R; u < R; ++u) {
			sumidx_posx = trimIndex(level, i + u)*Sxy;
			for (int v = -R; v < R; ++v) {		
				temp3didx = sumidx_posx + sumidx_j + trimIndex(level, k + v);	//[posx][j][posy]
				eposx = (Origin_y[level][temp3didx] / TEXSIZE_) - u;
				eposy = (Origin_y[level][temp3didx] % TEXSIZE_) - v;
				if (!(eposx >= R && eposx < Sx - R && eposy >= R && eposy < Sy - R)) continue;

				tempTIidx = eposx*Sx + eposy;
				for (int l = 0; l < COHERENCENUM; ++l) {						//[0,COHERENCENUM]				
					temp2didx = KCoherence_y[level][tempTIidx][l];
					int p = 0;
					compareNum = compareIdx.size();
					for (; p < compareNum; ++p) {
						if (compareIdx[p] == temp2didx)	break;
					}
					if (p < compareNum)	continue;

					IndexHisWeight = 1.0f + factor[level] * max(0.0f, IndexHis_y[level][temp2didx] - 1.0f);			//IndexHis needs sparse grid
					curDis = getFullDistance(level, m_exemplar_y[level], temp2didx, current_neighbor);
					curError = IndexHisWeight * curDis;

					if (minError > curError) {								//min error			
						minError = curError;
						minDis = curDis;									
						bestTIIdx = temp2didx;
					}
					compareIdx.push_back(temp2didx);
				}//for (int l = 0; l < COHERENCENUM; ++l){	
			}//for (int v = -R; v < R; ++v){
		}//for (int u = -R; u < R; ++u){

		if (minError < 10e9) {
			//update NearestIndex, IndexHis, store EuDis
			if (!setNearestIndex(level, nearestIdx_y[level], nearestWeight_y[level], IndexHis_y[level], idx, bestTIIdx, minDis))	
				isUnchanged = false;
		}
		else {
			cout << "bad point...\n";	_getch();
			//bestCoord = xexemplar->getRandomPos();
			//setNearestIndex(direction, i, j, k, bestCoord, xexemplar, min_dist);	
			//isUnchanged = false;
		}
	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {

	//For X		
	direction = 0;
	shuffle(m_permutation_xyz.begin(), m_permutation_xyz.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation_xyz[i2];			//[i][j][k] idx=i*Sxy+j*Sx+k
		ANNidx k = idx % Sx;				
		ANNidx j = (idx / Sx) % Sy;		
		ANNidx i = idx / Sxy;				
		if (j % GRID != 0 || k % GRID != 0)	continue;					//sparse grid			
		if(isUnchangedBlock(level,direction, i,j,k)) continue;			//check neighbours all unchanged or not
				
		ANNidx index = 0;		
		ANNidx TXYz = Sxy * i;
		for (ANNidx du = -R; du < R; ++du) {	//N is neighbourhood size.
			ANNidx VCurIdx3 = TXYz + Sx * trimIndex(level, j + du);
			for (ANNidx dv = -R; dv < R; ++dv) {
				ANNidx index2 = VCurIdx3 + trimIndex(level, k + dv);								//[i][j+du][k+dv]
				cvmSet(current_neighbor, 0, index, m_volume[level][index2]);						//set current_neighbor_x(0,col) to m_volume(idx)
				index++;
			}
		}

		ANNdist minError = 10e9, minDis = 10e9;
		ANNidx bestTIIdx;
		ANNidx eposx, eposy;
		ANNidx temp2didx, temp3didx, tempTIidx;
		ANNidx sumidx_i = i*Sxy;
		ANNidx sumidx_posx;
		int compareNum;
		compareIdx.clear(); compareIdx.reserve(blockSize_ * blockSize_ * COHERENCENUM);
		for (int u = -R; u < R; ++u){
			sumidx_posx = trimIndex(level, j + u)*Sx;
			for (int v = -R; v < R; ++v){
				temp3didx = sumidx_i + sumidx_posx + trimIndex(level, k + v);	//[i][posx][posy]
				eposx = (Origin_x[level][temp3didx] / TEXSIZE_) - u;
				eposy = (Origin_x[level][temp3didx] % TEXSIZE_) - v;
				if (!(eposx >= R && eposx < Sx - R && eposy >= R && eposy < Sy - R)) continue;

				tempTIidx = eposx*Sx + eposy;								//[x][y] idx=x*Sx+y			
				for (int l = 0; l < COHERENCENUM; ++l){						//[0,COHERENCENUM]				
					temp2didx = KCoherence_x[level][tempTIidx][l];
					int p = 0;
					compareNum = compareIdx.size();
					for (; p < compareNum; ++p){
						if (compareIdx[p] == temp2didx)	break;
					}
					if (p < compareNum)	continue;
	
					IndexHisWeight = 1.0f + factor[level] * max(0.0f, IndexHis_x[level][temp2didx] - 1.0f);			//IndexHis needs sparse grid
					curDis = getFullDistance(level, m_exemplar_x[level], temp2didx, current_neighbor);
					curError = IndexHisWeight * curDis;

					if (minError > curError){								//min error			
						minError = curError;
						minDis = curDis;									//minDis=eudis^2
						bestTIIdx = temp2didx;
					}
					compareIdx.push_back(temp2didx);
				}//for (int l = 0; l < COHERENCENUM; ++l){	
			}//for (int v = -R; v < R; ++v){
		}//for (int u = -R; u < R; ++u){

		if (minError < 10e9){
			//update NearestIndex, IndexHis, store EuDis
			if (!setNearestIndex(level, nearestIdx_x[level], nearestWeight_x[level], IndexHis_x[level], idx, bestTIIdx, minDis))
				isUnchanged = false;	
		}else{
			cout << "bad point...\n";	_getch();			
			//bestCoord = xexemplar->getRandomPos();
			//setNearestIndex(direction, i, j, k, bestCoord, xexemplar, min_dist);	
			//isUnchanged = false;
		}
	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {

	long time_end = clock();
	cout << " clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
	return isUnchanged;
}

ANNdist DoPAR::getFullDistance(int level, vector<ANNcoord>& exemplar, ANNidx idx2d, CvMat * dataMat) {
	ANNdist min_dist = 0.00001f;
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
	// look up all neighbourhood in m_volume[i][j][k], check if all is unchanged (if anyone has changed, return false)
	const ANNidx Sx = TEXSIZE[level];
	const ANNidx jSx = j*Sx;
	const ANNidx Sxy = TEXSIZE[level] * TEXSIZE[level];
	const ANNidx iSxy = i*Sxy;
	ANNidx start, end;
	start = end = static_cast<ANNidx>(blockSize[level] / (2 * GRID)) + 1;
	ANNidx tempidx;

	switch (direction){
	case(0) :	// X
		for (ANNidx tj = j - start; tj <= j + end; ++tj){
			tempidx = iSxy + trimIndex(level, tj) * Sx;
			for (ANNidx tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_x[level][tempidx + trimIndex(level, tk)])		//[i][tj][tk]
					return false;
			}
		}
		break;
	case(1) :	// Y
		for (int ti = i - start; ti <= i + end; ++ti){
			tempidx = trimIndex(level, ti) * Sxy + jSx;
			for (int tk = k - start; tk <= k + end; ++tk){
				if (!isUnchanged_y[level][tempidx + trimIndex(level, tk)])		//[ti][j][tk]
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
	long time_start = clock();	cout << endl << "optimize...";

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
	const ANNidx start = static_cast<ANNidx>(blockSize_ / (2 * GRID)) + 1;
	const ANNidx end = start;
	ANNidx s1 = -static_cast<ANNidx>(blockSize_ / 2);
	ANNidx e1 = static_cast<ANNidx>((blockSize_ - 1) / 2);
	if (level != 0) {								//reduce average blurring
		if (s1<-1) s1 += 1;
		if (e1>1) e1 -= 1;
	}

	
	shuffle(m_permutation_xyz.begin(), m_permutation_xyz.end(), mersennetwistergenerator);
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		ANNidx idx = m_permutation_xyz[i2];			//[i][j][k]
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
		ANNidx sumidx_i = i*Sxy;
		ANNidx sumidx_j = j*Sx;
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
			sumidx_tempx = trimIndex(level, tempx)*Sxy;
			for (ANNidx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = j - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_tempx + trimIndex(level, tempy)*Sx + k;							//[tempx][tempy][k]
				
				tempnearestidx = nearestIdx_z[level][tempidx];
				tempnearestweight = nearestWeight_z[level][tempidx];								//nearestidx from search step, weight=eudis^-0.6

				tempnearestidx += deltax * Sx + deltay;
				tempcolor = m_exemplar_z[level][tempnearestidx];
				
				colorCand_z.push_back(tempcolor);													//discrete solver
				posCand_z.push_back(tempnearestidx);

				//only PosHis is not sparse grid
				weight = 1.0f + factor[level] * max(0.0f, PosHis[level][tempnearestidx] - 1.0f);	//here 1.0 is average PosHis
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
			sumidx_tempx = trimIndex(level, tempx)*Sxy;
			for (ANNidx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = k - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_tempx + sumidx_j + trimIndex(level, tempy);						//[tempx][j][tempy]
				
				tempnearestidx = nearestIdx_y[level][tempidx];										//nearestidx from search step, weight=eudis^-0.6
				tempnearestweight = nearestWeight_y[level][tempidx];

				tempnearestidx += deltax * Sx + deltay;
				tempcolor = m_exemplar_y[level][tempnearestidx];
			
				colorCand_y.push_back(tempcolor);													//discrete solver
				posCand_y.push_back(tempnearestidx);

				//coord.swapXY();
				weight = 1.0f + factor[level] * max(0.0f, PosHis[level][tempnearestidx] - 1.0f);
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
			sumidx_tempx = trimIndex(level, tempx)*Sx;
			for (ANNidx h = start; h >= -end; --h) {
				tempy = tempy0 + h * GRID;
				deltay = k - tempy;
				if (deltax < s1 || deltax > e1 || deltay < s1 || deltay > e1)
					continue;

				tempidx = sumidx_i + sumidx_tempx + trimIndex(level, tempy);						//[i][tempx][tempy]
				
				tempnearestidx = nearestIdx_x[level][tempidx];
				tempnearestweight = nearestWeight_x[level][tempidx];								//nearestidx from search step, weight=eudis^-0.6
				tempnearestidx += deltax * Sx + deltay;
				tempcolor = m_exemplar_x[level][tempnearestidx];
				
				colorCand_x.push_back(tempcolor);													//discrete solver
				posCand_x.push_back(tempnearestidx);

				weight = 1.0f + factor[level] * max(0.0f, PosHis[level][tempnearestidx] - 1.0f);
				weight = tempnearestweight / weight;
				color_acc += weight * tempcolor;
				weight_acc += weight;
			}
		}

		// least solver
		color_avg = color_acc / weight_acc;

		// Discrete solver
		ANNdist xMinDis = 10e9;		ANNdist yMinDis = 10e9;		ANNdist zMinDis = 10e9;
		ANNidx closestIdx_x, closestIdx_y, closestIdx_z;
		ANNdist tempdis, tempnum;
		// X	
		auto temporder = min_element(colorCand_x.begin(), colorCand_x.end(), [=](ANNcoord x, ANNcoord y){
			return abs(x - color_avg) < abs(y - color_avg);
		});												//compute discrete color,origin
		tempnum = distance(colorCand_x.begin(), temporder);
		xMinDis = abs(colorCand_x[tempnum] - color_avg);
		closestIdx_x = posCand_x[tempnum];

		if (Origin_x[level][idx] != closestIdx_x) {											// update X isUnchangeblock, origin
			Origin_x[level][idx] = closestIdx_x;
			isUnchanged_x[level][idx] = false;
		}
		else isUnchanged_x[level][idx] = true;

		// Y	
		temporder = min_element(colorCand_y.begin(), colorCand_y.end(), [=](ANNcoord x, ANNcoord y) {
			return abs(x - color_avg) < abs(y - color_avg);
		});												//compute discrete color,origin
		tempnum = distance(colorCand_y.begin(), temporder);
		yMinDis = abs(colorCand_y[tempnum] - color_avg);
		closestIdx_y = posCand_y[tempnum];

		if (Origin_y[level][idx] != closestIdx_y) {											// update Y isUnchangeblock, origin
			Origin_y[level][idx] = closestIdx_y;
			isUnchanged_y[level][idx] = false;
		}
		else isUnchanged_y[level][idx] = true;

		// Z	
		temporder = min_element(colorCand_z.begin(), colorCand_z.end(), [=](ANNcoord x, ANNcoord y) {
			return abs(x - color_avg) < abs(y - color_avg);
		});												//compute discrete color,origin
		tempnum = distance(colorCand_z.begin(), temporder);
		zMinDis = abs(colorCand_z[tempnum] - color_avg);
		closestIdx_x = posCand_z[tempnum];

		if (Origin_z[level][idx] != closestIdx_z) {											// update Z isUnchangeblock, origin
			Origin_z[level][idx] = closestIdx_z;
			isUnchanged_z[level][idx] = false;
		}
		else isUnchanged_z[level][idx] = true;


		
		if (xMinDis < yMinDis && xMinDis < zMinDis) {
			tempcolor = m_exemplar_x[level][closestIdx_x];
			updatePosHis(level, PosHis[level], SelectedPos[level], idx, closestIdx_x);		// update PosHis 
		}
		else if (yMinDis < xMinDis && yMinDis < zMinDis) {
			tempcolor = m_exemplar_y[level][closestIdx_y];
			updatePosHis(level, PosHis[level], SelectedPos[level], idx, closestIdx_y);
		}
		else {
			tempcolor = m_exemplar_z[level][closestIdx_z];
			updatePosHis(level, PosHis[level], SelectedPos[level], idx, closestIdx_z);
		}
		m_volume[level][idx] = tempcolor;													// update m_volume color

	}//for (ANNidx i2 = 0; i2 < Size; ++i2) {

	long time_end = clock();
	cout << " clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
}


//---------- Index Histogram for search step ---------
void DoPAR::initIndexHis(){
	// init IndexHis, NearestIndex
}

bool DoPAR::setNearestIndex(int level, vector<ANNidx>& nearestIdx, vector<ANNdist>& nearestWeight, vector<ANNdist>&IndexHis,
	ANNidx idx3d, ANNidx newNearestIdx, ANNdist dis) {
	//update IndexHis	//update NearestIndex, store EuDis^-0.6 -- search step
	ANNidx formerNearestIdx = nearestIdx[idx3d];
	nearestWeight[idx3d] = pow(dis, -0.6f);										//update nearestWeight

	if (formerNearestIdx == newNearestIdx)	return true;
	nearestIdx[idx3d] = newNearestIdx;											//update nearestIdx
	
	formerNearestIdx = sparseIdx(level, formerNearestIdx);						//sparse grid update IndexHis
	if (IndexHis[formerNearestIdx] > 0)	IndexHis[formerNearestIdx] -= deltaIndexHis[level];
	IndexHis[newNearestIdx] += deltaIndexHis[level];
	return false;
}


//============Position Histogram for optimize step====
void DoPAR::initPosHis(){
	// init PosHis, Origin

}

void DoPAR::updatePosHis(int level, vector<ANNdist>& PosHis, vector<ANNidx>& selectedPos, ANNidx idx3d, ANNidx newPos) {
	// update PosHis -- optimize step
	// no sparse grid!
	ANNidx formerPos = selectedPos[idx3d];
	if (PosHis[formerPos] > 0)	PosHis[formerPos] -= deltaPosHis[level];
	
	selectedPos[idx3d] = newPos;
	PosHis[newPos] += deltaPosHis[level];
}



void DoPAR::upsampleVolume(int level) {
	cout << endl << endl << "Upsample from level " << level << " to level " << level + 1;
	static const bool flag[8][8] = {
		{ true, false, false, false, false, false, false, false },
		{ true, true, false, false, false, false, false, false },
		{ true, false, true, false, false, false, false, false },
		{ true, true, true, true, false, false, false, false },
		{ true, false, false, false, true, false, false, false },
		{ true, true, false, false, true, true, false, false },
		{ true, false, true, false, true, false, true, false },
		{ true, true, true, true, true, true, true, true } };
	//ANNidx Size1 = TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level], Size2 = TEXSIZE[level+1] * TEXSIZE[level+1] * TEXSIZE[level+1];
	ANNidx Sxy1 = TEXSIZE[level] * TEXSIZE[level], Sxy2 = TEXSIZE[level + 1] * TEXSIZE[level + 1];

	for (ANNidx z = 0; z < TEXSIZE[level]; ++z) {
		for (ANNidx y = 0; y < TEXSIZE[level]; ++y) {
			for (ANNidx x = 0; x < TEXSIZE[level]; ++x) {
				ANNidx index[8];
				for (ANNidx dz = 0; dz < 2; ++dz) {
					for (ANNidx dy = 0; dy < 2; ++dy) {
						for (ANNidx dx = 0; dx < 2; ++dx) {
							//index at level
							index[4 * dz + 2 * dy + dx] =
								Sxy1 * min(z + dz, TEXSIZE[level] - 1) + TEXSIZE[level] * trimIndex(level, y + dy) + trimIndex(level, x + dx);
						}
					}
				}
				for (ANNidx dz = 0; dz < 2; ++dz) {
					for (ANNidx dy = 0; dy < 2; ++dy) {
						for (ANNidx dx = 0; dx < 2; ++dx) {
							//index at level+1
							ANNidx index2 = Sxy2 * (2 * z + dz) + TEXSIZE[level + 1] * (2 * y + dy) + 2 * x + dx;
							ANNcoord color(0.0f);
							int cnt = 0;
							for (int i = 0; i < 8; ++i) {
								if (flag[4 * dz + 2 * dy + dx][i]) {
									color += m_volume[level][index[i]];
									++cnt;
								}
							}
							color /= cnt;
							m_volume[level + 1][index2] = color;
						}
					}
				}
			}
		}
	}

}

void DoPAR::writeHistogram(bool scaling, int level, vector<float> &hisvec, int rows, int cols, const string filename) {
	if (filename.size() == 0) { cout << endl << "writeIndexHistogram Error: empty filename"; return; };
	if (hisvec.size() != rows*cols) { cout << endl << "writeIndexHistogram Error: wrong size of hisvec"; return; };

	Mat hist = Mat(rows, cols, CV_32FC1);
	hist = Mat(hisvec, true).reshape(1, hist.rows);

	if (scaling) hist *= TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
	hist.convertTo(hist, CV_8UC1);	//convertTo just copy the value, no scaling

	short i(0);
	string tempFPathName = filename;
	while (fileExists(tempFPathName) == true) {
		tempFPathName = filename.substr(0, filename.find('.')) + "_" + to_string(i) + ".png";
		i++;
	}//rename, not overwrite

	imwrite(tempFPathName, hist);
	cout << endl << "histogram plotted.";
}













