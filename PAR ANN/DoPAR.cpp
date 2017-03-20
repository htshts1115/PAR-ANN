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
	m_neighbor_x.resize(MULTIRES);
	m_neighbor_y.resize(MULTIRES);
	m_neighbor_z.resize(MULTIRES);
	mp_neighbor_pca_average_x.resize(MULTIRES, NULL);
	mp_neighbor_pca_average_y.resize(MULTIRES, NULL);
	mp_neighbor_pca_average_z.resize(MULTIRES, NULL);
	mp_neighbor_pca_projected_x.resize(MULTIRES, NULL);
	mp_neighbor_pca_projected_y.resize(MULTIRES, NULL);
	mp_neighbor_pca_projected_z.resize(MULTIRES, NULL);
	mp_neighbor_pca_eigenvec_x.resize(MULTIRES, NULL);
	mp_neighbor_pca_eigenvec_y.resize(MULTIRES, NULL);
	mp_neighbor_pca_eigenvec_z.resize(MULTIRES, NULL);
	m_neighbor_kdTree_ptr_x.resize(MULTIRES);
	m_neighbor_kdTree_ptr_y.resize(MULTIRES);
	m_neighbor_kdTree_ptr_z.resize(MULTIRES);
	mp_neighbor_kdTree_x.resize(MULTIRES, NULL);
	mp_neighbor_kdTree_y.resize(MULTIRES, NULL);
	mp_neighbor_kdTree_z.resize(MULTIRES, NULL);
	m_histogram_exemplar.resize(MULTIRES);
	m_histogram_synthesis.resize(MULTIRES);
	m_positionhistogram_exemplar.resize(MULTIRES);
	m_positionhistogram_synthesis.resize(MULTIRES);
	m_volume.resize(MULTIRES);
	m_volume_position.resize(MULTIRES);
	m_volume_nearest_x_index.resize(MULTIRES);
	m_volume_nearest_y_index.resize(MULTIRES);
	m_volume_nearest_z_index.resize(MULTIRES);
	m_volume_nearest_x_dist.resize(MULTIRES);
	m_volume_nearest_y_dist.resize(MULTIRES);
	m_volume_nearest_z_dist.resize(MULTIRES);
	m_permutation_xyz.resize(MULTIRES);
	valuechange.resize(MULTIRES);
	
	WEIGHT_POSITIONHISTOGRAM.resize(MULTIRES);
	MAXITERATION.resize(MULTIRES);
	valuechangethreshold.resize(MULTIRES);
	gaussiankernel.resize(MULTIRES);
	absoluteneigh.resize(MULTIRES);
	// [end] multi-res memory allocation---------------
}

DoPAR::~DoPAR()
{
	for (int level = 0; level < MULTIRES; ++level) {
		if (mp_neighbor_kdTree_x[level] != NULL) delete mp_neighbor_kdTree_x[level];
		if (mp_neighbor_kdTree_y[level] != NULL) delete mp_neighbor_kdTree_y[level];
		if (mp_neighbor_kdTree_z[level] != NULL) delete mp_neighbor_kdTree_z[level];
		if (mp_neighbor_pca_average_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_x[level]);
		if (mp_neighbor_pca_average_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_y[level]);
		if (mp_neighbor_pca_average_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_z[level]);
		if (mp_neighbor_pca_projected_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x[level]);
		if (mp_neighbor_pca_projected_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y[level]);
		if (mp_neighbor_pca_projected_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z[level]);
		if (mp_neighbor_pca_eigenvec_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x[level]);
		if (mp_neighbor_pca_eigenvec_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y[level]);
		if (mp_neighbor_pca_eigenvec_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z[level]);
	}
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

bool DoPAR::Read(const string FPathName, vector<uchar>& Data)
{
	Data.clear();

	long Length = FileLength(FPathName.c_str());
	if (Length < 1) {
		//cout << endl << "File is empty!";
		return false;
	}

	ifstream InF(FPathName.c_str(), ios::in | ios::binary);
	if (!InF) return false;

	Data.reserve(Length);

	long BlockNum = 1000000L; //~1M

	long ActualBlkNum = floor(1.0*Length / BlockNum);

	if (ActualBlkNum > 0) {//Read the file one block after another				
		const long BlockSize = sizeof(char)*BlockNum;

		char* Mblock = new char[BlockSize];

		for (long RowNum = 0; RowNum < ActualBlkNum; ++RowNum) {
			if (!InF.read(reinterpret_cast<char *>(Mblock), BlockSize))
			{
				delete[] Mblock; InF.close(); return false;
			}

			for (long idx = 0; idx<BlockNum; ++idx)
				Data.push_back(Mblock[idx]);
		}

		delete[] Mblock;
	}//Read the file one block after another

	BlockNum = Length - ActualBlkNum*BlockNum;

	if (BlockNum > 0) {
		const long BlockSize = sizeof(char)*BlockNum;
		char* Mblock = new char[BlockSize];

		if (!InF.read(reinterpret_cast<char *>(Mblock), BlockSize))
		{
			delete[] Mblock; InF.close(); return false;
		}

		for (long idx = 0; idx<BlockNum; ++idx)
			Data.push_back(Mblock[idx]);

		delete[] Mblock;
	}

	InF.close();  return true;
}

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool DoPAR::Write(const string FPathName, vector<uchar> Data)
{
	if (FPathName.size() == 0) return false;
	if (Data.size() == 0) return false;

	//if (PARz == 1) {
	//	string tempFPathName2D = FPathName;
	//	//tempFPathName2D.substr(0, tempFPathName2D.rfind('.') == std::string::npos ? tempFPathName2D.length() : tempFPathName2D.rfind('.'));
	//	tempFPathName2D = tempFPathName2D.substr(0, tempFPathName2D.find('.')) + ".pbm";
	//	ofstream OFile(tempFPathName2D.c_str(), ios::ate);
	//	if (!OFile) { cout << endl << "!OFile"; return false; }
	//
	//	OFile << "P1" << endl;
	//	//OFile << "# Created by Paint Shop Pro" << endl;
	//	OFile << PARx << " " << PARy << endl;
	//
	//	for (long i = 0; i < Data.size(); ++i){
	//		if ((Data[i] - 0) == -2) OFile << 2 << ' ';
	//		else OFile << Data[i] - 0 << ' ';
	//	}
	//
	//	OFile.close();
	//	return true;
	//}
	//else {
		int i(1);
		string tempFPathName2D = FPathName;
		while (fileExists(tempFPathName2D) == true){
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
		for (idx = 0; idx<Steps; ++idx) {
			for (int i = 0; i<iSTEPLENGTH; ++i) Buffer[i] = Data[DataIdx++];
			if (!OFile.write(reinterpret_cast<char *>(Buffer), BlockSize)) {
				delete[] Buffer; return false;
			}
		}

		delete[] Buffer;

		if (RemainNum>0) {
			char* Buff = new char[sizeof(char)*RemainNum];
			for (int i = 0; i<RemainNum; ++i) Buff[i] = Data[DataIdx++];
			if (!OFile.write(reinterpret_cast<char *>(Buff), sizeof(char)*RemainNum)) {
				delete[] Buff; return false;
			}
			delete[] Buff;
		}

		OFile.close();

		return true;
	//}//else
}

bool DoPAR::GetNextRowParameters(short Cno, vector<string>& ValidParStr, vector<string>& ParV)
{
	if ((unsigned short)Cno >= ValidParStr.size()) {
		cout << endl << "Wrong arguments!";
		char ch; cin >> ch; exit(1);
	}

	ParV.clear();

	string TmpS = "";
	for (unsigned long Idx = 0; Idx<ValidParStr[Cno].size(); ++Idx) {
		if (ValidParStr[Cno][Idx] == ',' || ValidParStr[Cno][Idx] == ' ' || ValidParStr[Cno][Idx] == '\n') {
			if (TmpS.size()>0) { ParV.push_back(TmpS); }
			TmpS = "";
		}
		else { TmpS += ValidParStr[Cno][Idx]; }
	} //for(long InIdx=0; InIdx<LineLst1[LNidx].size(); ++InIdx)	
	if (TmpS.size()>0) { ParV.push_back(TmpS); }

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
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) FNameXY = workpath + ParV[0];
			if (ParV.size() > 1) FNameXZ = workpath + ParV[1];
			if (ParV.size() > 2) FNameYZ = workpath + ParV[2];
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

	//----------read 3D model
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) modelFilename3D = workpath + ParV[0];
		}
		//////seperate filename and format!
		tempoutputformat = ".RAW";
		tempoutputfilename = ParV[0].substr(0, ParV[0].rfind('.') == std::string::npos ? ParV[0].length() : ParV[0].rfind('.'));
	}

	outputfilename = tempoutputfilename + "_ANN" + tempoutputformat;

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

	outputpath = workpath;

	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		//if (ParV.size() > 0) PARx = atoi(ParV[0].c_str());
		//if (ParV.size() > 1) PARy = atoi(ParV[1].c_str());
		//if (ParV.size() > 2) PARz = atoi(ParV[2].c_str());
		//PARxy = PARx*PARy;
		//GetNextRowParameters(++Row, ResLines, ParV);

		if (ParV.size() > 0) outputpath = ResLines[Row];
		CreateDirectoryA(outputpath.c_str(), NULL); //ofstream cannot create folder!
	
		//if (PARx < 3 || PARy < 3 || PARz < 1)
		//{
		//	cout << endl;
		//	cout << endl << "====================================================";
		//	cout << endl << " Inappropriate dimensionns for resultant 3D model !";
		//	cout << endl << " (" << PARx << "x" << PARy << "x" << PARz << ")";
		//	cout << endl << "====================================================";
		//	cout << endl;
		//	_getch(); exit(1);
		//}
	}	
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

void DoPAR::ReadPBMImage(string FName, char DirID, double UpPro)
{
	//DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
	//       otherwise a single image for three direction 

	vector<string> ResLines;
	ReadTxtFiles(FName, ResLines);

	if (ResLines.size() == 0) {
		cout << endl << "===================================";
		cout << endl << "Fail to read the pbm file !!";
		cout << endl << FName;
		cout << endl << "===================================";
		_getch(); exit(1);
	}

	vector<uchar> OImg2D;

	long S1, S2;
	long PoreNum(0);
	//!!deal with pbm file header!
	bool FormatYN(false);
	bool finddimension(false);

	if (ResLines[0][0] == 'P') FormatYN = true;
	if (FormatYN == false) {
		cout << endl << "Fail to read the pbm file !!";
		_getch(); exit(1);
	}

	for (long Line = 1; Line < ResLines.size(); ++Line) {
		if (ResLines[Line][0] == '#') continue;
		if (Line <= 2) {
			if (finddimension == false){
				vector<long> Dim;
				string OneItem("");
				for (long idx = 0; idx < ResLines[Line].size(); ++idx) {
					if (ResLines[Line][idx] < '0' || ResLines[Line][idx] > '9') {
						if (OneItem != "") {
							short Val = atoi(OneItem.c_str());
							Dim.push_back(Val);
						}
						OneItem = "";
					}
					else {
						OneItem += ResLines[Line][idx];
					}
				}

				if (OneItem != "") {
					short Val = atoi(OneItem.c_str());
					Dim.push_back(Val);
				}

				S1 = Dim[0]; S2 = Dim[1];
				OImg2D.reserve(S1*S2);
				finddimension = true;
				continue;
			}
		}//(Line <= 2)

		string OneItem("");
		for (long idx = 0; idx < ResLines[Line].size(); ++idx) {
			if (ResLines[Line][idx] >= '0' && ResLines[Line][idx] <= '9') {
				OneItem += ResLines[Line][idx];
				unsigned short Val = atoi(OneItem.c_str());
				if (Val == 1) PoreNum++; //1 - pore
				OImg2D.push_back(Val);
				OneItem = "";
			}

		}
	}

	double Porosity = 1.0*PoreNum / OImg2D.size();

	if (UpPro < 0.5) UpPro = 0.5;
	if (Porosity > UpPro) {
		for (long idx = 0; idx < OImg2D.size(); ++idx)
			OImg2D[idx] = 1 - OImg2D[idx];
		// 1 - pore, 0 - grain
		Porosity = 1 - Porosity;
	}

	cout << endl << "Porosity: " << Porosity;

	switch (DirID) {
	case '1': {//DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
				  XYSx = S1; XYSy = S2;
				  XY2DImg.swap(OImg2D);
				  PorosityXY = Porosity;
				  break;
	}
	case '2': {//XZ
				  XZSx = S1; XZSz = S2;
				  XZ2DImg.swap(OImg2D);
				  PorosityXZ = Porosity;
				  break;
	}
	case '3': {//YZ
				  YZSy = S1; YZSz = S2;
				  YZ2DImg.swap(OImg2D);
				  PorosityYZ = Porosity;
				  break;
	}
	default: {
				 XYSx = XZSx = YZSy = S1;
				 XYSy = XZSz = YZSz = S2;
				 XY2DImg.resize(OImg2D.size(), 0);
				 XZ2DImg.resize(OImg2D.size(), 0);
				 YZ2DImg.resize(OImg2D.size(), 0);
				 for (long idx = 0; idx < OImg2D.size(); ++idx) {
					 XY2DImg[idx] = OImg2D[idx];
					 XZ2DImg[idx] = OImg2D[idx];
					 YZ2DImg[idx] = OImg2D[idx];
				 }
				 PorosityXY = PorosityXZ = PorosityYZ = Porosity;
	}
	}

	cout << endl << FName;
	cout << endl << "Dimensions of this training image: (" << S1 << "  " << S2 << ")";

	TIx = S1; TIy = S2;
}

void DoPAR::GetStarted(string CurExeFile)
{
	ReadRunPar(CurExeFile);


	DoANNOptimization();
}


///========================== 190217 Kopf. optimization based =====================

//load 3D model raw file
vector<uchar> DoPAR::load3Dmodel(const char* filename)
{
	// open the file:
	std::streampos fileSize;
	std::ifstream file(filename, std::ios::binary);

	// get its size:
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// read the data:
	std::vector<uchar> fileData(fileSize);
	file.read((char*)&fileData[0], fileSize);

	if (fileData.size() != TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]) {
		cout << endl << "Error: Model size = " << fileData.size() << " SHOULD BE:" << TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0];
		_getch(); exit(1);
	}

	return fileData;
}

// Show a Mat object quickly. For testing purposes only.
void DoPAR::showMat(const cv::String& winname, const cv::Mat& mat)
{
	assert(!mat.empty());
	cv::namedWindow(winname);
	cv::imshow(winname, mat);
	cv::waitKey(0);
	cv::destroyWindow(winname);
}


//MULTIRES: larger number means finner level
const int DoPAR::N[MULTIRES] = {4};		//Kopf{ 3, 4, 4 }; Chen{3,4,4}; Turner{5,5,5}	// neighborhood size: (2 * N + 1)^2
int DoPAR::TEXSIZE[MULTIRES];
int DoPAR::D_NEIGHBOR[MULTIRES];
int DoPAR::NEIGHBORSIZE[MULTIRES];

const int DoPAR::NUM_HISTOGRAM_BIN = 16;	//for color histogram	Kopf used 16
vector<int> DoPAR::CHANNEL_MAXVALUE;		//for color histogram

//const double DoPAR::WEIGHT_HISTOGRAM = NUM_HISTOGRAM_BIN;		//for accelerate convergence original 10.0

int DoPAR::NUM_CHANNEL = 1;

const double DoPAR::PCA_RATIO_VARIANCE = 0.95;	//Kopf used 0.95

const double DoPAR::ErrorBound = 2.0;			//Kopf used 2.0

const int DoPAR::ANNsearchk = 5;	

const double DoPAR::gaussiansigma = 6.0;		//gaussian fall-off function in optimization phase, higher sigma means more uniform


void DoPAR::DoANNOptimization(){
	
	init();
	
	time_t StartTime;
	time(&StartTime);

	for (int curlevel = 0; curlevel < MULTIRES; curlevel++){
		cout << endl << "=============level: " << curlevel << "===============";
	
		for (int loop = 0; loop < MAXITERATION[curlevel]; loop++){
			cout << endl << "---------iteration: " << loop+1<<"------------";
			valuechange[curlevel] = 0.0;
			
			searchVolume(curlevel);
			optimizeVolume(curlevel);

			//if converge break;
			if (valuechange[curlevel] < valuechangethreshold[curlevel]) break;		//David Turner used 1.0e-3
		}
		outputmodel(curlevel);
		//_getch();

		if (curlevel < MULTIRES - 1) {
			upsampleVolume(curlevel);
			if (!POSITIONHIS_ON){
				calcHistogram_exemplar(curlevel + 1);
				calcHistogram_synthesis(curlevel + 1);
			}
			if (POSITIONHIS_ON){
				InitRandomVolumePosition(curlevel + 1);
				calcPositionHistogram_exemplar(curlevel + 1);
				calcPositionHistogram_synthesis(curlevel + 1);		
			}
		}
	}

	time_t NewTime;
	time(&NewTime);
	cout << endl << "Total reconstruction time: " << long(NewTime - StartTime);

	//outputmodel(MULTIRES-1);

	if (POSITIONHIS_ON){
		//show position histogram	vector->mat
		long rows = TEXSIZE[MULTIRES-1];
		long cols = 3 * rows;
		showHistogram(m_positionhistogram_synthesis[MULTIRES - 1], rows, cols, MULTIRES - 1);
	}
}

void DoPAR::outputmodel(int level){
	if (!TRUE && level==MULTIRES-1){
		for (long idx = 0; idx < m_volume[level].size(); idx++){
			if (m_volume[level][idx] > 127.0) m_volume[level][idx] = 255.0;
			else m_volume[level][idx] = 0.0;
		}
	}

	vector<int> tempvec = vector<int>(m_volume[level].begin(), m_volume[level].end());
	Model = vector<uchar>(tempvec.begin(), tempvec.end());

	Write(outputpath + outputfilename, Model);
	cout << endl << "output done.";
}

void DoPAR::initthreshold(){
	if (MULTIRES == 1){
		valuechangethreshold = { 0.5 };
		MAXITERATION = { 10 };
		//int maxl0 = 3 * NUM_CHANNEL*(2 * N[0] + 1)*(2 * N[0] + 1);
		//for (int s = 0; s < maxl0; s++)	PredefinedL0idx.push_back(s);
	}
	else if (MULTIRES == 2){
		valuechangethreshold = { 0.5, 0.1 }; //higher threshold means faster convergence, coarser accuracy
		MAXITERATION = { 20, 15 };
		//int maxl0 = 3 * NUM_CHANNEL*(2 * N[0] + 1)*(2 * N[0] + 1);
		//for (int s = 0; s < maxl0; s++)	PredefinedL0idx.push_back(s);
		//int maxl1 = 3 * NUM_CHANNEL*(2 * N[1] + 1)*(2 * N[1] + 1);
		//for (int s = 0; s < maxl1; s++)	PredefinedL1idx.push_back(s);
	}
	else if (MULTIRES == 3){
		valuechangethreshold = { 0.5, 0.1, 0.1 }; //higher threshold means faster convergence, coarser accuracy
		MAXITERATION = { 20, 15, 10 };
		//int maxl0 = 3 * NUM_CHANNEL*(2 * N[0] + 1)*(2 * N[0] + 1);
		//for (int s = 0; s < maxl0; s++)	PredefinedL0idx.push_back(s);
		//int maxl1 = 3 * NUM_CHANNEL*(2 * N[1] + 1)*(2 * N[1] + 1);
		//for (int s = 0; s < maxl1; s++)	PredefinedL1idx.push_back(s);
		//int maxl2 = 3 * NUM_CHANNEL*(2 * N[2] + 1)*(2 * N[2] + 1);
		//for (int s = 0; s < maxl2; s++)	PredefinedL2idx.push_back(s);
	}
	else if (MULTIRES == 4){
		valuechangethreshold = { 0.5, 0.5, 0.25, 0.1 }; //higher threshold means faster convergence, coarser accuracy
		MAXITERATION = { 20, 15, 10, 10 };
		//int maxl0 = 3 * NUM_CHANNEL*(2 * N[0] + 1)*(2 * N[0] + 1);
		//for (int s = 0; s < maxl0; s++)	PredefinedL0idx.push_back(s);
		//int maxl1 = 3 * NUM_CHANNEL*(2 * N[1] + 1)*(2 * N[1] + 1);
		//for (int s = 0; s < maxl1; s++)	PredefinedL1idx.push_back(s);
		//int maxl2 = 3 * NUM_CHANNEL*(2 * N[2] + 1)*(2 * N[2] + 1);
		//for (int s = 0; s < maxl2; s++)	PredefinedL2idx.push_back(s);
		//int maxl3 = 3 * NUM_CHANNEL*(2 * N[3] + 1)*(2 * N[3] + 1);
		//for (int s = 0; s < maxl3; s++)	PredefinedL3idx.push_back(s);
	}
}

void DoPAR::initabsoluteneigh(){
	int n;
	for (int level = 0; level < MULTIRES; level++){
		absoluteneigh[level].resize(NEIGHBORSIZE[level]);
		n = 0;
		for (int y = -N[level]; y <= N[level]; y++){
			for (int x = -N[level]; x <= N[level]; x++){
				absoluteneigh[level][n] = y*TEXSIZE[level] + x;
				n++;
			}
		}	
	}
}

void DoPAR::init() {
	if (!loadExemplar()) return;

	if (!loadVolume()) return;

	initthreshold();	

	calcNeighbor();
	InitGaussianKernel();
	initabsoluteneigh();

	if (!POSITIONHIS_ON){
		WEIGHT_HISTOGRAM = double(NUM_HISTOGRAM_BIN);
		calcHistogram_exemplar(0);
		calcHistogram_synthesis(0);
		cout << endl << "Color Histogram Initialized.";
	}
	if (POSITIONHIS_ON){
		InitRandomVolumePosition(0);
		cout << endl << "Init Random PositionIndex Done.";
		calcPositionHistogram_exemplar(0);
		calcPositionHistogram_synthesis(0);
		cout << endl << "Position Histogram Initialized.";
	}
}

void DoPAR::InitGaussianKernel(){
	const double PI = 4.0*atan(1.0);
	
	for (int level = 0; level < MULTIRES; level++){
		//initialize
		gaussiankernel[level].resize(NEIGHBORSIZE[level]);
		double sum = 0.0;
		double scale = -0.5 / (gaussiansigma*gaussiansigma);
		double cons = -scale / PI;
		int size = 2 * N[level] + 1;
		//compute kernel
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j <size; j++)
			{
				int x = i - (size - 1) / 2;
				int y = j - (size - 1) / 2;
				gaussiankernel[level][i*size + j] = cons * exp(scale * (x*x + y*y));
				sum += gaussiankernel[level][i*size + j];
			}
		}
		//normalize
		for (int i = 0; i < NEIGHBORSIZE[level]; i++)
		{
			gaussiankernel[level][i] /= sum;
		}
	}
}

bool DoPAR::loadExemplar() {
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
		NEIGHBORSIZE[level] = (2 * N[level] + 1) * (2 * N[level] + 1);
		D_NEIGHBOR[level] = NUM_CHANNEL * NEIGHBORSIZE[level];

		// [begin] memory allocation -------------------------------------------
		m_exemplar_x[level].resize(NUM_CHANNEL * TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_y[level].resize(NUM_CHANNEL * TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_z[level].resize(NUM_CHANNEL * TEXSIZE[level] * TEXSIZE[level]);
		m_neighbor_x[level].resize(D_NEIGHBOR[level] * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
		m_neighbor_y[level].resize(D_NEIGHBOR[level] * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
		m_neighbor_z[level].resize(D_NEIGHBOR[level] * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
		m_volume[level].resize(NUM_CHANNEL * TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_position[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_nearest_x_index[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_nearest_y_index[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_nearest_z_index[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_nearest_x_dist[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_nearest_y_dist[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_volume_nearest_z_dist[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_permutation_xyz[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		// [end] memory allocation -------------------------------------------

		for (int v = 0; v < TEXSIZE[level]; ++v) {
			for (int u = 0; u < TEXSIZE[level]; ++u) {
				long index = NUM_CHANNEL * (TEXSIZE[level] * v + u);
				long index2 = NUM_CHANNEL * (TEXSIZE[level] * v + u);
				for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
					m_exemplar_x[level][index + ch] = (unsigned char)img_x->imageData[index2 + ch];
					m_exemplar_y[level][index + ch] = (unsigned char)img_y->imageData[index2 + ch];
					m_exemplar_z[level][index + ch] = (unsigned char)img_z->imageData[index2 + ch];
				}
			}
		}
		//cvShowImage("t", img_x);
		//cvWaitKey(0);
		if (level == 0) continue;
		// go to the coarser level
		IplImage *img_next_x = cvCreateImage(cvSize(TEXSIZE[level] / 2, TEXSIZE[level] / 2), img_depth, img_nChannels);
		IplImage *img_next_y = cvCreateImage(cvSize(TEXSIZE[level] / 2, TEXSIZE[level] / 2), img_depth, img_nChannels);
		IplImage *img_next_z = cvCreateImage(cvSize(TEXSIZE[level] / 2, TEXSIZE[level] / 2), img_depth, img_nChannels);
		cvResize(img_x, img_next_x, CV_INTER_AREA);		//CV_INTER_AREA for shrink
		cvResize(img_y, img_next_y, CV_INTER_AREA);		//CV_INTER_CUBIC for enlarge 
		cvResize(img_z, img_next_z, CV_INTER_AREA);		//CV_INTER_LINEAR for speed
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

	CHANNEL_MAXVALUE.resize(NUM_CHANNEL);
	for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
		CHANNEL_MAXVALUE[ch] = 256;
	}

	cout << endl << "load TIs done.";
	return true;
}

bool DoPAR::loadVolume(){
	//----------------convert Model(vector<uchar>) to m_volume (vector<vector<int>> (multires,ch*x*y*z))		
	if (FALSE){//for comparing, use random initial model
		InitRandomVolume(0);
		cout << endl << "Init Random Volume.";
	}	
	else{//load from Model, later can also load from file
		if (fileExists(modelFilename3D.c_str()) == true){
			Model = load3Dmodel(modelFilename3D.c_str());
			
			vector<int> tempvec = vector<int>(Model.begin(), Model.end());
			m_volume[0] = vector<double>(tempvec.begin(), tempvec.end());

			cout << endl << "load 3D model done.";
		}
		else{
			cout << endl << "3D initial model doesn't exist. Use Random initial.";
			InitRandomVolume(0);
			cout << endl << "Init Random Volume Done.";
		}
	}

	return true;
}

void DoPAR::InitRandomVolume(int level) {
	vector<double>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	for (int xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
		long index2 = NUM_CHANNEL * (rand() % (TEXSIZE[level] * TEXSIZE[level]));
		int ori = rand() % 3;
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			m_volume[level][NUM_CHANNEL * xyz + ch] = p[ori]->operator[](index2 + ch);
		}
	}
}

void DoPAR::calcNeighbor() {
	//initialize kdtree for certain template/neighbourhood

	cout << endl << "calcNeighbor...";
	for (int level = 0; level < MULTIRES; ++level) {
		cout <<endl<< "level:" << level;
		int numData = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
		//ann_index range=[0,numData), corresponds to (x,y) where x/y range=[N[level],TEXSIZE[level]-N[level])!
		CvMat* p_source_x = cvCreateMat(numData, D_NEIGHBOR[level], CV_64F);	//rows='area' numData, cols=dimension (Neighbour size)
		CvMat* p_source_y = cvCreateMat(numData, D_NEIGHBOR[level], CV_64F);
		CvMat* p_source_z = cvCreateMat(numData, D_NEIGHBOR[level], CV_64F);
		int row = 0;
		for (int v = N[level]; v < TEXSIZE[level] - N[level]; ++v) {
			for (int u = N[level]; u < TEXSIZE[level] - N[level]; ++u) {
				int col = 0;
				for (int dv = -N[level]; dv <= N[level]; ++dv) {
					for (int du = -N[level]; du <= N[level]; ++du) {
						long index = NUM_CHANNEL * (TEXSIZE[level] * (v + dv) + u + du);
						for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
							cvmSet(p_source_x, row, col, m_exemplar_x[level][index + ch]);	//set p_source_x(row,col) to m_examplar_x(idx)
							cvmSet(p_source_y, row, col, m_exemplar_y[level][index + ch]);
							cvmSet(p_source_z, row, col, m_exemplar_z[level][index + ch]);
							m_neighbor_x[level][D_NEIGHBOR[level] * row + col] = m_exemplar_x[level][index + ch];
							m_neighbor_y[level][D_NEIGHBOR[level] * row + col] = m_exemplar_y[level][index + ch];
							m_neighbor_z[level][D_NEIGHBOR[level] * row + col] = m_exemplar_z[level][index + ch];
							++col;
						}
					}
				}
				++row;
			}
		}
		// PCA calculation (obtain all eigenvectors of the input covariance matrix)

		////////每一行表示一个样本
		//////CvMat* pData = cvCreateMat( 总的样本数, 每个样本的维数, CV_32FC1 );
		if (mp_neighbor_pca_average_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_x[level]);
		if (mp_neighbor_pca_average_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_y[level]);
		if (mp_neighbor_pca_average_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_z[level]);
		//CvMat* pMean = cvCreateMat(1, 样本的维数, CV_32FC1);
		mp_neighbor_pca_average_x[level] = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		mp_neighbor_pca_average_y[level] = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		mp_neighbor_pca_average_z[level] = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		//pEigVals中的每个数表示一个特征值
		//CvMat* pEigVals = cvCreateMat(1, min(总的样本数,样本的维数), CV_32FC1);
		CvMat* p_eigenValues_x = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		CvMat* p_eigenValues_y = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		CvMat* p_eigenValues_z = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		//每一行表示一个特征向量
		//CvMat* pEigVecs = cvCreateMat( min(总的样本数,样本的维数), 样本的维数, CV_32FC1);
		CvMat* p_eigenVectors_all_x = cvCreateMat(D_NEIGHBOR[level], D_NEIGHBOR[level], CV_64F);
		CvMat* p_eigenVectors_all_y = cvCreateMat(D_NEIGHBOR[level], D_NEIGHBOR[level], CV_64F);
		CvMat* p_eigenVectors_all_z = cvCreateMat(D_NEIGHBOR[level], D_NEIGHBOR[level], CV_64F);
		//PCA处理,计算出平均向量pMean,特征值pEigVals和特征向量pEigVecs
		//cvCalcPCA(pData, pMean, pEigVals, pEigVecs, CV_PCA_DATA_AS_ROW);
		//now have better function //PCA pca(data, mean, PCA::DATA_AS_ROW, 0.95);

		cvCalcPCA(p_source_x, mp_neighbor_pca_average_x[level], p_eigenValues_x, p_eigenVectors_all_x, CV_PCA_DATA_AS_ROW);
		cvCalcPCA(p_source_y, mp_neighbor_pca_average_y[level], p_eigenValues_y, p_eigenVectors_all_y, CV_PCA_DATA_AS_ROW);
		cvCalcPCA(p_source_z, mp_neighbor_pca_average_z[level], p_eigenValues_z, p_eigenVectors_all_z, CV_PCA_DATA_AS_ROW);
		// Decide amount of dimensionality reduction
		double contribution_total_x = 0;
		double contribution_total_y = 0;
		double contribution_total_z = 0;
		for (int i = 0; i < D_NEIGHBOR[level]; ++i) {
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
		for (int i = 0; i < D_NEIGHBOR[level]; ++i) {
			double ratio_x = contribution_acc_x / contribution_total_x;
			double ratio_y = contribution_acc_y / contribution_total_y;
			double ratio_z = contribution_acc_z / contribution_total_z;
			if (ratio_x < PCA_RATIO_VARIANCE) {	//now have better function //PCA pca(data, mean, PCA::DATA_AS_ROW, 0.95);
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
			if (PCA_RATIO_VARIANCE <= ratio_x && PCA_RATIO_VARIANCE <= ratio_y && PCA_RATIO_VARIANCE <= ratio_z) break;
		}
		
		cout << endl;
		printf("PCA reduction (x): %d -> %d\n", D_NEIGHBOR[level], dimPCA_x);
		printf("PCA reduction (y): %d -> %d\n", D_NEIGHBOR[level], dimPCA_y);
		printf("PCA reduction (z): %d -> %d\n", D_NEIGHBOR[level], dimPCA_z);

		// Trim total eigenvectors into partial eigenvectors
		if (mp_neighbor_pca_eigenvec_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x[level]);
		if (mp_neighbor_pca_eigenvec_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y[level]);
		if (mp_neighbor_pca_eigenvec_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z[level]);
		mp_neighbor_pca_eigenvec_x[level] = cvCreateMat(dimPCA_x, D_NEIGHBOR[level], CV_64F);
		mp_neighbor_pca_eigenvec_y[level] = cvCreateMat(dimPCA_y, D_NEIGHBOR[level], CV_64F);
		mp_neighbor_pca_eigenvec_z[level] = cvCreateMat(dimPCA_z, D_NEIGHBOR[level], CV_64F);
		memcpy(mp_neighbor_pca_eigenvec_x[level]->data.db, p_eigenVectors_all_x->data.db, sizeof(double)* dimPCA_x * D_NEIGHBOR[level]);
		memcpy(mp_neighbor_pca_eigenvec_y[level]->data.db, p_eigenVectors_all_y->data.db, sizeof(double)* dimPCA_y * D_NEIGHBOR[level]);
		memcpy(mp_neighbor_pca_eigenvec_z[level]->data.db, p_eigenVectors_all_z->data.db, sizeof(double)* dimPCA_z * D_NEIGHBOR[level]);
		// PCA projection
		////// And copy the PCA results:
		////Mat mean = pca.mean.clone();
		////Mat eigenvalues = pca.eigenvalues.clone();
		////Mat eigenvectors = pca.eigenvectors.clone();
		////now better function
		////dst = pca.project(mat); //CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
		if (mp_neighbor_pca_projected_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x[level]);
		if (mp_neighbor_pca_projected_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y[level]);
		if (mp_neighbor_pca_projected_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z[level]);
		//选出前P个特征向量(主成份),然后投影,结果保存在pResult中，pResult中包含了P个系数
		//CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
		mp_neighbor_pca_projected_x[level] = cvCreateMat(numData, dimPCA_x, CV_64F);
		mp_neighbor_pca_projected_y[level] = cvCreateMat(numData, dimPCA_y, CV_64F);
		mp_neighbor_pca_projected_z[level] = cvCreateMat(numData, dimPCA_z, CV_64F);
		//cvProjectPCA( pData, pMean, pEigVecs, pResult );
		cvProjectPCA(p_source_x, mp_neighbor_pca_average_x[level], mp_neighbor_pca_eigenvec_x[level], mp_neighbor_pca_projected_x[level]);
		cvProjectPCA(p_source_y, mp_neighbor_pca_average_y[level], mp_neighbor_pca_eigenvec_y[level], mp_neighbor_pca_projected_y[level]);
		cvProjectPCA(p_source_z, mp_neighbor_pca_average_z[level], mp_neighbor_pca_eigenvec_z[level], mp_neighbor_pca_projected_z[level]);
		// kd-tree construction
		m_neighbor_kdTree_ptr_x[level].resize(numData);
		m_neighbor_kdTree_ptr_y[level].resize(numData);
		m_neighbor_kdTree_ptr_z[level].resize(numData);
		for (int i = 0; i < numData; ++i) {
			//point array, row = 1, col = PCA dimension
			m_neighbor_kdTree_ptr_x[level][i] = &mp_neighbor_pca_projected_x[level]->data.db[dimPCA_x * i];	//ANNpoint* from PCA projection
			//std::vector<uchar> array(mat.rows*mat.cols);
			//if (mat.isContinuous())
			//	array = mat.data;
			m_neighbor_kdTree_ptr_y[level][i] = &mp_neighbor_pca_projected_y[level]->data.db[dimPCA_y * i];
			m_neighbor_kdTree_ptr_z[level][i] = &mp_neighbor_pca_projected_z[level]->data.db[dimPCA_z * i];
		}
		if (mp_neighbor_kdTree_x[level] != NULL) delete mp_neighbor_kdTree_x[level];
		if (mp_neighbor_kdTree_y[level] != NULL) delete mp_neighbor_kdTree_y[level];
		if (mp_neighbor_kdTree_z[level] != NULL) delete mp_neighbor_kdTree_z[level];
		//data point array = m_neighbor_kdTree_ptr_x, number of points = numData, dimension = dimPCA_x
		mp_neighbor_kdTree_x[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_x[level][0], numData, dimPCA_x); //ANNkd_tree
		mp_neighbor_kdTree_y[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_y[level][0], numData, dimPCA_y);
		mp_neighbor_kdTree_z[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_z[level][0], numData, dimPCA_z);
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

	cout << endl << "calcNeighbor done.";
}

void DoPAR::searchVolume(int level) {
	long time_start = clock();
	const double min_dist = 0.00000000001;
	cout << endl <<"phase1:searching";
	vector<long> nearest_x_index_old = m_volume_nearest_x_index[level];	//size = area
	vector<long> nearest_y_index_old = m_volume_nearest_y_index[level];
	vector<long> nearest_z_index_old = m_volume_nearest_z_index[level];
	double global_energy_new = 0;
	long cnt_nearest_index_new = 0;
	//========For index histogram counting, shuffle the order. ========
	initPermutation(level);	//shuffle m_permutation_xyz

	int process = 0, displayprocess = -1;
	for (long i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i) {
		//for each point in volume	(the sequence doesnt matter, all the points are searched and then optimize)
		if (level == MULTIRES - 1){
			process = i * 100 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
			if (process > displayprocess){
				displayprocess = process;
				cout << "\r" << displayprocess << "%..";
			}
		}
		//========For index histogram counting, use shuffled index========
		long i2 = m_permutation_xyz[level][i];	//x,y,z is random order
		int x = i2 % TEXSIZE[level];
		int y = (i2 / TEXSIZE[level]) % TEXSIZE[level];
		int z = i2 / (TEXSIZE[level] * TEXSIZE[level]);
		//int x = i %  TEXSIZE[level];
		//int y = (i / TEXSIZE[level]) % TEXSIZE[level];
		//int z = i / (TEXSIZE[level] * TEXSIZE[level]);

		// obtain current neighborhood_x from volume
		CvMat* current_neighbor_x = cvCreateMat(1, D_NEIGHBOR[level], CV_64F); //rows = 1, cols = dimesnion
		CvMat* current_neighbor_y = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		CvMat* current_neighbor_z = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		long index = 0;
		for (int dv = -N[level]; dv <= N[level]; ++dv) {	//N is neighbourhood size. 
			for (int du = -N[level]; du <= N[level]; ++du) {
				long index2_x = NUM_CHANNEL * (TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + dv) + TEXSIZE[level] * trimIndex(level, y + du) + x);
				long index2_y = NUM_CHANNEL * (TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + du) + TEXSIZE[level] * y + trimIndex(level, x + dv));
				long index2_z = NUM_CHANNEL * (TEXSIZE[level] * TEXSIZE[level] * z + TEXSIZE[level] * trimIndex(level, y + dv) + trimIndex(level, x + du));
				for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
					cvmSet(current_neighbor_x, 0, index + ch, m_volume[level][index2_x + ch]); //set current_neighbor_x(0,col) to m_volume(idx)
					cvmSet(current_neighbor_y, 0, index + ch, m_volume[level][index2_y + ch]);
					cvmSet(current_neighbor_z, 0, index + ch, m_volume[level][index2_z + ch]);
				}
				index += NUM_CHANNEL;
			}
		}
		// PCA projection
		int dimPCA_x = mp_neighbor_pca_eigenvec_x[level]->rows;	//computed already
		int dimPCA_y = mp_neighbor_pca_eigenvec_y[level]->rows;
		int dimPCA_z = mp_neighbor_pca_eigenvec_z[level]->rows;
		CvMat* current_neighbor_x_projected = cvCreateMat(1, dimPCA_x, CV_64F);	//project current_neighbor_x to current_neighbor_x_projected, dimension = dimPCA_x
		CvMat* current_neighbor_y_projected = cvCreateMat(1, dimPCA_y, CV_64F);
		CvMat* current_neighbor_z_projected = cvCreateMat(1, dimPCA_z, CV_64F);
		cvProjectPCA(current_neighbor_x, mp_neighbor_pca_average_x[level], mp_neighbor_pca_eigenvec_x[level], current_neighbor_x_projected);
		cvProjectPCA(current_neighbor_y, mp_neighbor_pca_average_y[level], mp_neighbor_pca_eigenvec_y[level], current_neighbor_y_projected);
		cvProjectPCA(current_neighbor_z, mp_neighbor_pca_average_z[level], mp_neighbor_pca_eigenvec_z[level], current_neighbor_z_projected);
		// ANN search!
		//!! error may occur if index > int
		
		//==========multiple nearest index, position control=========
		ANNidxArray ann_index_x = new ANNidx[ANNsearchk];
		ANNidxArray ann_index_y = new ANNidx[ANNsearchk];
		ANNidxArray ann_index_z = new ANNidx[ANNsearchk];
		ANNdistArray ann_dist_x = new ANNdist[ANNsearchk];
		ANNdistArray ann_dist_y = new ANNdist[ANNsearchk];
		ANNdistArray ann_dist_z = new ANNdist[ANNsearchk];
		//int ann_index_x;
		//int ann_index_y;
		//int ann_index_z;
		//double ann_dist_x;
		//double ann_dist_y;
		//double ann_dist_z;
		
		// ANN search. error bound = 2.0; Kopf used 2.0
		mp_neighbor_kdTree_x[level]->annkSearch(current_neighbor_x_projected->data.db, ANNsearchk, ann_index_x, ann_dist_x, ErrorBound);	
		mp_neighbor_kdTree_y[level]->annkSearch(current_neighbor_y_projected->data.db, ANNsearchk, ann_index_y, ann_dist_y, ErrorBound);
		mp_neighbor_kdTree_z[level]->annkSearch(current_neighbor_z_projected->data.db, ANNsearchk, ann_index_z, ann_dist_z, ErrorBound);
		//mp_neighbor_kdTree_x[level]->annkSearch(current_neighbor_x_projected->data.db, 1, &ann_index_x, &ann_dist_x, ErrorBound);	
		//mp_neighbor_kdTree_y[level]->annkSearch(current_neighbor_y_projected->data.db, 1, &ann_index_y, &ann_dist_y, ErrorBound);
		//mp_neighbor_kdTree_z[level]->annkSearch(current_neighbor_z_projected->data.db, 1, &ann_index_z, &ann_dist_z, ErrorBound);
		
		// CV release
		cvReleaseMat(&current_neighbor_x);
		cvReleaseMat(&current_neighbor_y);
		cvReleaseMat(&current_neighbor_z);
		cvReleaseMat(&current_neighbor_x_projected);
		cvReleaseMat(&current_neighbor_y_projected);
		cvReleaseMat(&current_neighbor_z_projected);
		
		int selected_index_x(0), selected_index_y(0), selected_index_z(0);
		//==========multiple nearest index, Index Histogram matching=========
		if (POSITIONHIS_ON){
			selected_index_x = indexhistmatching_ann_index(level,0, ann_index_x, ann_dist_x);
			selected_index_y = indexhistmatching_ann_index(level,1, ann_index_y, ann_dist_y);
			selected_index_z = indexhistmatching_ann_index(level,2, ann_index_z, ann_dist_z);
		
			//update index histogram		
			updateIndexHistogram(level, 0, ann_index_x[selected_index_x]);
			updateIndexHistogram(level, 1 ,ann_index_y[selected_index_y]);
			updateIndexHistogram(level, 2, ann_index_z[selected_index_z]);
		}

		//update nearest_index, nearest_dist
		m_volume_nearest_x_index[level][i] = ann_index_x[selected_index_x];
		m_volume_nearest_y_index[level][i] = ann_index_y[selected_index_y];
		m_volume_nearest_z_index[level][i] = ann_index_z[selected_index_z];
		m_volume_nearest_x_dist[level][i] = ann_dist_x[selected_index_x] + min_dist;
		m_volume_nearest_y_dist[level][i] = ann_dist_y[selected_index_y] + min_dist;
		m_volume_nearest_z_dist[level][i] = ann_dist_z[selected_index_z] + min_dist;
		//m_volume_nearest_x_index[level][i] = ann_index_x;
		//m_volume_nearest_y_index[level][i] = ann_index_y;
		//m_volume_nearest_z_index[level][i] = ann_index_z;
		//m_volume_nearest_x_dist[level][i] = ann_dist_x + min_dist;
		//m_volume_nearest_y_dist[level][i] = ann_dist_y + min_dist;
		//m_volume_nearest_z_dist[level][i] = ann_dist_z + min_dist;

		//search all points then optimize! Not search one point optimize one point!
		if (m_volume_nearest_x_index[level][i] != nearest_x_index_old[i]) ++cnt_nearest_index_new;	//just for illustration
		if (m_volume_nearest_y_index[level][i] != nearest_y_index_old[i]) ++cnt_nearest_index_new;
		if (m_volume_nearest_z_index[level][i] != nearest_z_index_old[i]) ++cnt_nearest_index_new;
		global_energy_new += m_volume_nearest_x_dist[level][i];	//just for illustration
		global_energy_new += m_volume_nearest_y_dist[level][i];
		global_energy_new += m_volume_nearest_z_dist[level][i];
	}
	long time_end = clock();
	cout << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
	cout << " updated NN: " << cnt_nearest_index_new <<", global energy: " << global_energy_new;
}

void DoPAR::initPermutation(int level) {
	for (long i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i) {
		m_permutation_xyz[level][i] = i;
	}
	shuffle(m_permutation_xyz[level].begin(), m_permutation_xyz[level].end(), mersennetwistergenerator);
}

void DoPAR::optimizeVolume(int level) {
	long time_start = clock();
	initPermutation(level);	//shuffle m_permutation_xyz
	cout <<endl<<"phase2:optimizing";
	int process = 0, displayprocess = -1;
	for (long i2 = 0; i2 < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i2) {
		long i = m_permutation_xyz[level][i2];	//x,y,z is random order
		int x = i % TEXSIZE[level];
		int y = (i / TEXSIZE[level]) % TEXSIZE[level];
		int z = i / (TEXSIZE[level] * TEXSIZE[level]);
		double weight_acc = 0.0;
		vector<double> color_acc(NUM_CHANNEL, 0.0);

		if (level == MULTIRES - 1){
			process = i2 * 100 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
			if (process > displayprocess){
				displayprocess = process;
				cout << "\r" << displayprocess << "%..";
			}
		}

		//========discrete solver=================
		//colorset and its positionset [3*NEIGHBORSIZE]	//just for NUM_CHANNEL=1
		vector<double> colorset(3 * NEIGHBORSIZE[level], 10e5);
		vector<long> positionset(3 * NEIGHBORSIZE[level], 1000000);
		vector<double> positionfrequency(3 * NEIGHBORSIZE[level], 10e2);

		int m = 0;	
		//for every voxel's neighbourhood, in 3 orientation
		for (int dv = -N[level]; dv <= N[level]; ++dv) {
			for (int du = -N[level]; du <= N[level]; ++du) {
				////==========Tahmasebi algorithm: just consider center=========
				////He use PCA=100%, high histogram weight, 1/10 template size, discrete, only centre
				//if (dv != 0 || du != 0) { ++m; continue; }

				if ((z + dv <0 || z + dv>TEXSIZE[level] - 1) || (y + du <0 || y + du >TEXSIZE[level] - 1)
					|| (z + du <0 || z + du >TEXSIZE[level] - 1) || (x + dv <0 || x + dv >TEXSIZE[level] - 1)
					|| (y + dv <0 || y + dv >TEXSIZE[level] - 1) || (x + du <0 || x + du >TEXSIZE[level] - 1))
				{
					m++;
					continue;
				}
				//corresponding index of neighbour
				long index2_x = TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + dv) + TEXSIZE[level] * trimIndex(level, y + du) + x;
				long index2_y = TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + du) + TEXSIZE[level] * y + trimIndex(level, x + dv);
				long index2_z = TEXSIZE[level] * TEXSIZE[level] * z + TEXSIZE[level] * trimIndex(level, y + dv) + trimIndex(level, x + du);
				long index2[3] = { index2_x, index2_y, index2_z };
				vector<long>   * volume_nearest_index[3] = { &m_volume_nearest_x_index[level], &m_volume_nearest_y_index[level], &m_volume_nearest_z_index[level] };
				vector<double>* volume_nearest_dist[3] = { &m_volume_nearest_x_dist[level], &m_volume_nearest_y_dist[level], &m_volume_nearest_z_dist[level] };
				vector<double>* neighbor[3] = { &m_neighbor_x[level], &m_neighbor_y[level], &m_neighbor_z[level] };
				for (int ori = 0; ori < 3; ++ori) {//3 orientation
					long    nearest_index = (*volume_nearest_index[ori])[index2[ori]];
					double nearest_dist = (*volume_nearest_dist[ori])[index2[ori]];
					double* p_neighbor = &(*neighbor[ori])[D_NEIGHBOR[level] * nearest_index];
					// color of overlapping neighborhood pixel
					vector<double> color(NUM_CHANNEL);
					for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
						color[ch] = p_neighbor[NUM_CHANNEL * (NEIGHBORSIZE[level] - 1 - m) + ch];	
						//index2 ~ m, index ~ NEIGHBORSIZE[level] - 1 - m; the position is symmetrical!
												
						//========discrete solver=================
						//record each color and its position
						colorset[ori*NEIGHBORSIZE[level] + m] = color[ch];
						if (POSITIONHIS_ON){
							//relative index to centre:(NEIGHBORSIZE[level] - 1)/2 - m	absolute index:annconvert(nearest_index) + absolute[relative]
							positionset[ori*NEIGHBORSIZE[level] + m] = ori*TEXSIZE[level] * TEXSIZE[level] 
																	+ (convertIndexANN(level, nearest_index) + absoluteneigh[level][NEIGHBORSIZE[level] - 1 - m]);				
						}
					}
					// blending weight of this color according to matching distance
					double weight = pow(nearest_dist, -0.6);	//L2norm(i.e.2)*-0.6 = -1.2 = 0.8(i.e.r)-2	
					
					// modify weight according to histogram matching
					if (!POSITIONHIS_ON){
						double histogram_matching = 0.0;
						for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
							int bin = (int)(color[ch] * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
							double histogram_exemplar = m_histogram_exemplar[level][ch][bin];		// [0, 1]
							double histogram_synthesis = m_histogram_synthesis[level][ch][bin];		// [0, 1]
							histogram_matching += max(0.0, histogram_synthesis - histogram_exemplar);		// [0, 1]
						}
						weight *= 1.0 / (1.0 + WEIGHT_HISTOGRAM * histogram_matching);	//additional weight to accelerate convergence
					}				
					//// modify weight according to Position Histogram matching
					if (POSITIONHIS_ON){
						double positionhistogram_matching = 0.0;
						double positionhistogram_exemplar = m_positionhistogram_exemplar[level][positionset[ori*NEIGHBORSIZE[level] + m]];
						double positionhistogram_synthesis = m_positionhistogram_synthesis[level][positionset[ori*NEIGHBORSIZE[level] + m]];
						positionfrequency[ori*NEIGHBORSIZE[level] + m] = positionhistogram_synthesis;

						positionhistogram_matching = max(0.0, positionhistogram_synthesis - positionhistogram_exemplar);		// [0, 1]
						//additional weight to accelerate convergence
						//changed to gaussian distribution, std = averagef
						double stddev = 1.0 / m_positionhistogram_exemplar[level].size() /** 2.0/3.0*/;
						double gaussianprob = gaussian_pdf(positionhistogram_matching, 0.0, stddev);
						weight *= gaussianprob / gaussian_pdf(0.0, 0.0, stddev);
						//former used linear weight:
						//weight *= 1.0 / (1.0 + WEIGHT_POSITIONHISTOGRAM[level] * positionhistogram_matching);	
					}				

					//========Gaussian fall-off===============
					weight *= gaussiankernel[level][m] * (gaussiankernel[level].size());		//or NEIGHBORSIZE[level] - 1 - m

					// accumulate color
					for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
						color_acc[ch] += weight * color[ch];										
					}
					weight_acc += weight;
				}//3 orientation
				++m;
			}
		}//for every voxel's neighbourhood, in 3 orientation

		// old & new colors for this voxel
		vector<double> color_old(NUM_CHANNEL);
		vector<double> color_new(NUM_CHANNEL);
		//old & new position index for this voxel
		long position_old, position_new;
		int closestindex;

		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			color_old[ch] = m_volume[level][NUM_CHANNEL * i + ch];
			color_new[ch] = color_acc[ch] / weight_acc;		//least square solver

			//========discrete solver=================
			//first calculate the weighted average color, then find the most similar color existed in exemplar, each color corresponds to a m_volume_nearest_index
			closestindex = FindClosestColorIndex(level, colorset, positionfrequency, color_new[ch]);			
			color_new[ch] = colorset[closestindex];		//update with the existed most similar color		

			valuechange[level] += abs(color_new[ch] - color_old[ch]);			//mean absolute voxel change, show convergence
		}

		////histogram update
		if (!POSITIONHIS_ON){
			updateHistogram_synthesis(level, color_old, color_new);
		}	
		//Position histogram update
		if (POSITIONHIS_ON){
			position_new = positionset[closestindex];
			position_old = m_volume_position[level][i];
			updatePositionHistogram_synthesis(level, position_old, position_new);
			m_volume_position[level][i] = position_new;
		}

		// voxel update
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			m_volume[level][NUM_CHANNEL * i + ch] = color_new[ch];		
		}		

	}//for every voxel
	
	valuechange[level] /= TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];		//mean absolute voxel change, show convergence

	long time_end = clock();
	cout << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC << " Mean change in voxel value = " << valuechange[level];
}

void DoPAR::upsampleVolume(int level) {	
	cout << endl << endl << "Upsample from level "<< level <<" to level " << level + 1;
	static const bool flag[8][8] = {
		{ true, false, false, false, false, false, false, false },
		{ true, true, false, false, false, false, false, false },
		{ true, false, true, false, false, false, false, false },
		{ true, true, true, true, false, false, false, false },
		{ true, false, false, false, true, false, false, false },
		{ true, true, false, false, true, true, false, false },
		{ true, false, true, false, true, false, true, false },
		{ true, true, true, true, true, true, true, true } };
	for (int z = 0; z < TEXSIZE[level]; ++z) {
		for (int y = 0; y < TEXSIZE[level]; ++y) {
			for (int x = 0; x < TEXSIZE[level]; ++x) {
				int index[8];
				for (int dz = 0; dz < 2; ++dz) {
					for (int dy = 0; dy < 2; ++dy) {
						for (int dx = 0; dx < 2; ++dx) {
							//index at level
							index[4 * dz + 2 * dy + dx] = NUM_CHANNEL * (
								TEXSIZE[level] * TEXSIZE[level] * min(z + dz, TEXSIZE[level] - 1) +
								TEXSIZE[level] * trimIndex(level, y + dy) +
								trimIndex(level, x + dx));
						}
					}
				}
				for (int dz = 0; dz < 2; ++dz) {
					for (int dy = 0; dy < 2; ++dy) {
						for (int dx = 0; dx < 2; ++dx) {
							//index at level+1
							int index2 = NUM_CHANNEL * (TEXSIZE[level + 1] * TEXSIZE[level + 1] * (2 * z + dz) + TEXSIZE[level + 1] * (2 * y + dy) + 2 * x + dx);
							vector<double> color(NUM_CHANNEL, 0);
							int cnt = 0;
							for (int i = 0; i < 8; ++i) {
								if (flag[4 * dz + 2 * dy + dx][i]) {
									for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
										color[ch] += m_volume[level][index[i] + ch];
									}
									++cnt;
								}
							}
							for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
								color[ch] /= cnt;
								m_volume[level + 1][index2 + ch] = color[ch];
							}
						}
					}
				}
			}
		}
	}
}

void DoPAR::calcHistogram_synthesis(int level) {
	m_histogram_synthesis[level].clear();
	m_histogram_synthesis[level].resize(NUM_CHANNEL, vector<double>(NUM_HISTOGRAM_BIN, 0));
	double delta_histogram = 1. / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	for (int i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i) {
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			double c = m_volume[level][NUM_CHANNEL * i + ch];
			int bin = (int)(c * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
			m_histogram_synthesis[level][ch][bin] += delta_histogram;
		}
	}
}
void DoPAR::updateHistogram_synthesis(int level, const std::vector<double>& color_old, const std::vector<double>& color_new) {
	double delta_histogram = 1. / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
		int bin_old = (int)(color_old[ch] * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
		int bin_new = (int)(color_new[ch] * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
		m_histogram_synthesis[level][ch][bin_old] -= delta_histogram;
		m_histogram_synthesis[level][ch][bin_new] += delta_histogram;
	}
}
void DoPAR::calcHistogram_exemplar(int level) {
	m_histogram_exemplar[level].clear();
	m_histogram_exemplar[level].resize(NUM_CHANNEL, vector<double>(NUM_HISTOGRAM_BIN, 0));
	double delta_histogram = 1. / (3 * TEXSIZE[level] * TEXSIZE[level]);
	vector<double>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	for (int ori = 0; ori < 3; ++ori) {
		for (int i = 0; i < TEXSIZE[level] * TEXSIZE[level]; ++i) {
			for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
				double c = (*p[ori])[NUM_CHANNEL * i + ch];
				int bin = (int)(c * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
				m_histogram_exemplar[level][ch][bin] += delta_histogram;
			}
		}
	}
}

//============Position Histogram for optimize step====
void DoPAR::InitRandomVolumePosition(int level){
	long maxsize = 3 * (TEXSIZE[level]*TEXSIZE[level]);
	for (long xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
		m_volume_position[level][xyz] = xyz%maxsize;
	}
	shuffle(m_volume_position[level].begin(), m_volume_position[level].end(), mersennetwistergenerator);
}
void DoPAR::calcPositionHistogram_exemplar(int level){
	//initial uniform distribution
	m_positionhistogram_exemplar[level].clear();
	m_positionhistogram_exemplar[level].resize(3*(TEXSIZE[level]*TEXSIZE[level]),0.0);	
	for (long i = 0; i < m_positionhistogram_exemplar[level].size(); i++){
		m_positionhistogram_exemplar[level][i] = 1.0 / m_positionhistogram_exemplar[level].size();
	}

	WEIGHT_POSITIONHISTOGRAM[level] = 1.0* m_positionhistogram_exemplar[level].size();
}
void DoPAR::calcPositionHistogram_synthesis(int level){
	//initial uniform distribution
	m_positionhistogram_synthesis[level].clear();
	m_positionhistogram_synthesis[level].resize(3 * (TEXSIZE[level] * TEXSIZE[level]), 0.0);	
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	for (long i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; i++){
		long bin = m_volume_position[level][i];
		m_positionhistogram_synthesis[level][bin] += delta_histogram;
	}
}
void DoPAR::updatePositionHistogram_synthesis(int level, const long position_old, const long position_new){
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	m_positionhistogram_synthesis[level][position_old] -= delta_histogram;
	m_positionhistogram_synthesis[level][position_new] += delta_histogram;
}
//============Index Histogram for search step========
void DoPAR::initialIndexHistogram(int level){
	long numData = 3*(TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	m_indexhistogram_exemplar[level].resize(numData, 3.0 / numData);	// 3/3*Area = 1/Area
	m_indexhistogram_synthesis[level].resize(numData, 0.0);				
}
void DoPAR::updateIndexHistogram(int level, int orientation, const long annidx){
	long idx = annidx + orientation*(TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	m_indexhistogram_synthesis[level][idx] += delta_histogram;
}
int DoPAR::indexhistmatching_ann_index(int level, int orientation, ANNidxArray idxarray, ANNdistArray distarry){
	long  size = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	double difference(0.0), gaussianprob(0.0);
	double stddev = 1.0 / size;
	double probzero = gaussian_pdf(0.0, 0.0, stddev);
	double distvalue_acc(0.0), weight_acc(0.0); 

	for (int i = 0; i < ANNsearchk; i++){
		double weight = pow(distarry[i], -0.6); 
	
		long idx = idxarray[i] + orientation*size;
		difference = max(0.0 , m_indexhistogram_synthesis[level][idx] - m_indexhistogram_exemplar[level][idx]);
		gaussianprob = gaussian_pdf(difference, 0.0, stddev);
		weight *= gaussianprob / probzero;

		distvalue_acc += distarry[i]*weight;
		weight_acc += weight;
	}

	double refdist = distvalue_acc / weight_acc;

	vector<double> copydistarray(distarry, distarry + ANNsearchk);
	auto i = min_element(begin(copydistarray), end(copydistarray), [=](double x, double y)
	{
		return abs(x - refdist) < abs(y - refdist);
	});
	int closestindex = distance(begin(copydistarray), i);

	return closestindex;
}

void DoPAR::showHistogram(vector<double> &hisvec, long rows, long cols, int level){
	if (hisvec.size() == rows*cols) // check that the rows and cols match the size of your vector
	{		
		Mat hist = Mat(rows, cols, CV_64FC1);
		memcpy(hist.data, hisvec.data(), hisvec.size()*sizeof(double));	//float 32F, double 64F
		hist *= TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
		
		//cv::normalize(hist, hist, 0, 255, NORM_MINMAX);

		hist.convertTo(hist, CV_8UC1);	//convertTo just copy the value, no scaling

		imwrite("PositionHistogram.png", hist);
		cout << endl << "histogram plotted.";
	}
	else{
		cout << endl << "Error: wrong size of hisvec!";
	}
}

int DoPAR::FindClosestColorIndex(int level, vector<double> &color, vector<double> &weight, double referencecolor){
	//find nearest color value, then compare weight for all closest values
	//return the final index
	auto i = min_element(begin(color), end(color), [=](double x, double y)
	{
		return abs(x - referencecolor) < abs(y - referencecolor);
	});
	int closestindex = distance(begin(color), i);

	vector<int> filteredidx;
	vector<double> filteredweight;
	for (int j = 0; j < color.size(); j++){
		if (color[j] == color[closestindex]){
			filteredidx.push_back(j);
			filteredweight.push_back(weight[j]);
		}
	}

	auto k = min_element(begin(filteredweight), end(filteredweight));
	int weightedindex = distance(begin(filteredweight), k);

	return filteredidx[weightedindex];
}