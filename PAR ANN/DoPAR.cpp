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

	discrete_histogram_synthesis.resize(MULTIRES);
	m_histogram_synthesis.resize(MULTIRES);

	m_volume.resize(MULTIRES);

	m_volume_nearest_x_index.resize(MULTIRES);
	m_volume_nearest_y_index.resize(MULTIRES);
	m_volume_nearest_z_index.resize(MULTIRES);
	//m_volume_nearest_x_dist.resize(MULTIRES);
	//m_volume_nearest_y_dist.resize(MULTIRES);
	//m_volume_nearest_z_dist.resize(MULTIRES);
	m_volume_weight_x.resize(MULTIRES);
	m_volume_weight_y.resize(MULTIRES);
	m_volume_weight_z.resize(MULTIRES);

	absoluteneigh.resize(MULTIRES);
	m_volume_position.resize(MULTIRES);
	m_positionhistogram_exemplar.resize(MULTIRES);
	m_positionhistogram_synthesis.resize(MULTIRES);

	m_indexhistogram_synthesis.resize(MULTIRES);

	//ANNsearchk.resize(MULTIRES);

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
	annClose();
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

	for (int i = 0; i < MULTIRES; i++) parameterstring += to_string(N[i]);
	if (INDEXHIS_ON) parameterstring += "ih";
	if (COLORHIS_ON) parameterstring += "ch";
	if (DISCRETE_ON) parameterstring += "D";
	if (DISCRETETHRESHOLD_ON) parameterstring += "dt";

	outputfilename = tempoutputfilename + "_" + parameterstring + tempoutputformat;

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

const float DoPAR::inv_sqrt_2pi = 0.398942280401433f;
const ANNdist  DoPAR::alpha_[MULTIRES] = {64.0f, 128.0f, 256.0f};

///========================== 190217 Kopf. optimization based =====================

//MULTIRES: larger number means finner level
const int DoPAR::N[MULTIRES] = { 4,3,3 };		//Kopf{ 3, 4, 4 }; Chen{4,4,3}; Turner{5,5,5}	// neighborhood size: (2 * N + 1)^2
ANNidx DoPAR::TEXSIZE[MULTIRES];
int DoPAR::D_NEIGHBOR[MULTIRES];

const short DoPAR::NUM_HISTOGRAM_BIN = 64;						//for color histograms	Kopf used 16
short DoPAR::CHANNEL_MAXVALUE;									//for color histogram
const short DoPAR::DISCRETE_HISTOGRAM_BIN = 256;				//for thresholding, discrete values. e.g. default256
float DoPAR::perHisBin;
vector<float> DoPAR::delta_histogram_exemplar;
vector<float> DoPAR::delta_histogram_synthesis;
//vector<float> DoPAR::HisStdDev;									// accept maximum stddev*3 error

float DoPAR::Solid_Upper;				//for redistribute distancemap model. e.g. 192: 0-192 solid
float DoPAR::Pore_Lower;				//for redistribute distancemap model
short DoPAR::DistanceThreshold;			//for binarise distance model.  DistanceThreshold=(Solid_Upper+Pore_Lower)/2
vector<short> DoPAR::ProjectDMapMaxBins;

//vector<double> DoPAR::PCA_RATIO_VARIANCE;		//Kopf used 0.95
//vector<double> DoPAR::ErrorBound;				//Kopf used 2.0, we use different for multi levels
//vector<short> DoPAR::ANNsearchk;				//related to N[level]

const short DoPAR::MAXITERATION = 15;

void DoPAR::DoANNOptimization() {
	init();

	time_t StartTime;
	time(&StartTime);

	for (int curlevel = 0; curlevel < MULTIRES; curlevel++) {
		cout << endl << "=============level: " << curlevel << "===============";
		perpixel_energy_new = 0.0f;		
		perpixel_energy_old = 10e9;

		//initPermutation(curlevel);

		FIRSTRUN = true;
		int convergencecount(0), energyincreasecount(0);
		for (int loop = 0; loop < MAXITERATION - curlevel * 2; loop++) {
			cout << endl << "---------iteration: " << loop + 1 << "------------";

			searchVolume(curlevel);
			cout << endl << "optimize:";
			optimizeVolume(curlevel);

			FIRSTRUN = false;

			if (perpixel_energy_old - perpixel_energy_new < 0) energyincreasecount++;
			else if (perpixel_energy_old - perpixel_energy_new < perpixel_energy_old * 0.005 * (1+curlevel)) convergencecount++;
			perpixel_energy_old = perpixel_energy_new;
			if (energyincreasecount > 0) break;
			else if (convergencecount > 1) break;	//if change <1% for twice, then mark as converge
		}//loop in one level
		
		if (curlevel >= MULTIRES - 1 || TEXSIZE[curlevel] >= 128) {//draw histogram graph && ouput model	
			outputmodel(curlevel);		

			ANNidx cols = TEXSIZE[curlevel];
			ANNidx rows = 3 * cols;
			//if (INDEXHIS_ON) writeHistogram(curlevel, m_indexhistogram_synthesis[curlevel], 3 * (cols - 2 * N[curlevel]), cols - 2 * N[curlevel], "IndexHis_" + parameterstring + "_L" + to_string(curlevel) + ".png");
			if (POSITIONHIS_ON)	//show position histogram				
				writeHistogram(true, curlevel, m_positionhistogram_synthesis[curlevel], rows, cols, "PosHis_" + parameterstring + "_L" + to_string(curlevel) + ".png");
		}

		if (curlevel < MULTIRES - 1) {//level up
			FIRSTRUN = true;
			upsampleVolume(curlevel);
			if (COLORHIS_ON || DISCRETETHRESHOLD_ON) { initHistogram_synthesis(curlevel + 1); }
			if (POSITIONHIS_ON) {	initPositionHistogram_synthesis(curlevel + 1);	}		
			cleardata(curlevel);	//release vector
		}
	}

	time_t NewTime;
	time(&NewTime);
	cout << endl << "Total reconstruction time: " << long(NewTime - StartTime);

}

void DoPAR::init() {
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

	if (!loadVolume()) return;

	//if (!DISTANCEMAP_ON) calcNeighbor();
	computeKCoherence();

	//initial delta_histogram
	perHisBin = 1.0f * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE;
	delta_histogram_synthesis.resize(MULTIRES);
	delta_histogram_exemplar.resize(MULTIRES);
	for (short l = 0; l < MULTIRES; l++) {
		delta_histogram_synthesis[l] = 1.0f / (TEXSIZE[l] * TEXSIZE[l] * TEXSIZE[l]);
		delta_histogram_exemplar[l] = 1.0f / (3 * TEXSIZE[l] * TEXSIZE[l]);
	}

	initHistogram_exemplar();

	FIRSTRUN = true;
	if ((COLORHIS_ON || DISCRETETHRESHOLD_ON) && !DISTANCEMAP_ON) initHistogram_synthesis(0);
	if (INDEXHIS_ON && !DISTANCEMAP_ON) { initIndexHistogram(); }
	
	if (POSITIONHIS_ON) {
		initabsoluteneigh();
		initPositionHistogram_exemplar();
		initPositionHistogram_synthesis(0);
	}

	//=========== Bimodal Transform ===============
	if (DISTANCEMAP_ON && BIMODAL_ON) {
		BimodalRedistribution(m_exemplar_x[MULTIRES - 1], "BDM1.png");

		BimodalRedistribution(m_exemplar_y[MULTIRES - 1], "BDM2.png");

		BimodalRedistribution(m_exemplar_z[MULTIRES - 1], "BDM3.png");

		cout << endl << "Bimodal Distance Map outputed.";

		//=========== Bimodal Transform ===============
		if (fileExists(modelFilename3D.c_str()) == true) {
			BimodalRedistribution3D(m_volume[0], outputfilename);
			cout << endl << "Bimodal 3D input transformed.";
		}
		_getch();
	}
}

//void DoPAR::initPermutation(int level) {// random permutation (precomputed)
//	ANNidx Size = TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
//	m_permutation_xyz.clear();
//	m_permutation_xyz.resize(Size);
//	for (ANNidx i = 0; i <Size; ++i) {
//		m_permutation_xyz[i] = i;
//	}
//}

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

	//For distance map binarization, record original porosity (mean value of grey scale image)
	vector<double> porosityxyz(3, 0.0);
	Scalar tempmean = mean(matxy);
	porosityxyz[0] = tempmean.val[0] / 256.0;	PorosityX = porosityxyz[0];
	tempmean = mean(matxz);
	porosityxyz[1] = tempmean.val[0] / 256.0;	PorosityY = porosityxyz[1];
	tempmean = mean(matyz);
	porosityxyz[2] = tempmean.val[0] / 256.0;	PorosityZ = porosityxyz[2];
	porosityTI = (porosityxyz[0] + porosityxyz[1] + porosityxyz[2]) / 3.0;

	//Generate DM TI or not
	GenerateDMTI = true;

	// build image pyramid
	int img_depth = img_x->depth;
	int img_nChannels = img_x->nChannels;
	for (int level = MULTIRES - 1; level >= 0; --level) {
		// size registration
		TEXSIZE[level] = img_x->width;
		D_NEIGHBOR[level] = (2 * N[level] + 1) * (2 * N[level] + 1);
		// check multi-level size correct
		if (TEXSIZE[MULTIRES - 1] % (ANNidx)pow(2, MULTIRES - 1) != 0) { cout << endl << "TI size not right for multi-level"; _getch(); exit(1); }

		// [begin] memory allocation -------------------------------------------
		m_exemplar_x[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_y[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_exemplar_z[level].resize(TEXSIZE[level] * TEXSIZE[level]);
		m_neighbor_x[level].resize(D_NEIGHBOR[level] * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
		m_neighbor_y[level].resize(D_NEIGHBOR[level] * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
		m_neighbor_z[level].resize(D_NEIGHBOR[level] * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
		m_volume[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		if (POSITIONHIS_ON) m_volume_position[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_nearest_x_index[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_nearest_y_index[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_nearest_z_index[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_nearest_x_dist[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_nearest_y_dist[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_nearest_z_dist[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_weight_x[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_weight_y[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		//m_volume_weight_z[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
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

		//============ rethreshold to retain thin connections at coarser level! ===================
		vector<short> TempExistedBinX, TempExistedBinY, TempExistedBinZ;
		vector<float> TempExistedBinHisX, TempExistedBinHisY, TempExistedBinHisZ;
		if (false) {
			if (level == MULTIRES - 1) {
				calcTempHistogram(m_exemplar_x[MULTIRES - 1], TempExistedBinX, TempExistedBinHisX);
				calcTempHistogram(m_exemplar_y[MULTIRES - 1], TempExistedBinY, TempExistedBinHisY);
				calcTempHistogram(m_exemplar_z[MULTIRES - 1], TempExistedBinZ, TempExistedBinHisZ);
			}
			else {//level<MULTIRES-1
				vector<short> shortx = vector<short>(m_exemplar_x[level].begin(), m_exemplar_x[level].end());
				ProportionThreshold(shortx, TempExistedBinX, TempExistedBinHisX);
				m_exemplar_x[level] = vector<ANNcoord>(shortx.begin(), shortx.end());

				vector<short> shorty = vector<short>(m_exemplar_y[level].begin(), m_exemplar_y[level].end());
				ProportionThreshold(shorty, TempExistedBinY, TempExistedBinHisY);
				m_exemplar_y[level] = vector<ANNcoord>(shorty.begin(), shorty.end());

				vector<short> shortz = vector<short>(m_exemplar_z[level].begin(), m_exemplar_z[level].end());
				ProportionThreshold(shortz, TempExistedBinZ, TempExistedBinHisZ);
				m_exemplar_z[level] = vector<ANNcoord>(shortz.begin(), shortz.end());
			}
		}

		//============= Convert to distance model ==========================
		if (DISTANCEMAP_ON && level == MULTIRES - 1) {
			vector<char> tempchar(TEXSIZE[level] * TEXSIZE[level]);
			vector<short> shortx(m_exemplar_x[level].begin(), m_exemplar_x[level].end());
			vector<short> shorty(m_exemplar_y[level].begin(), m_exemplar_y[level].end());
			vector<short> shortz(m_exemplar_z[level].begin(), m_exemplar_z[level].end());

			short autothresholdvalue(110);
			bool binaryYN(true);
			char type('n');
			if (binaryYN) for (int i = 0; i < shortx.size(); i++) { if (shortx[i] < 255 && shortx[i]>0) { binaryYN = false; break; } }
			if (binaryYN) for (int i = 0; i < shorty.size(); i++) { if (shorty[i] < 255 && shorty[i]>0) { binaryYN = false; break; } }
			if (binaryYN) for (int i = 0; i < shortz.size(); i++) { if (shortz[i] < 255 && shortz[i]>0) { binaryYN = false; break; } }
			if (binaryYN == false && level == MULTIRES - 1) {
				do {
					cout << endl << "Choose thresholding value for grey TIs yourself? [y/n]";
					cin >> type;
				} while (!cin.fail() && type != 'y' && type != 'n');
				if (type == 'y') {
					cout << endl << "Input autothresholdvalue for TIs (default=108): ";
					cin >> autothresholdvalue;
				}
			}

			/*if (type == 'y') */BinariseThreshold(shortx, tempchar, autothresholdvalue);
			//else tempchar = BinariseImg(shortx, porosityxyz[0]);			//BinariseImg has problem!!! will generate small dots in big pores.
			shortx = GetDMap(TEXSIZE[level], TEXSIZE[level], 1, tempchar, 2, false);

			BinariseThreshold(shorty, tempchar, autothresholdvalue);
			shorty = GetDMap(TEXSIZE[level], TEXSIZE[level], 1, tempchar, 2, false);

			BinariseThreshold(shortz, tempchar, autothresholdvalue);
			shortz = GetDMap(TEXSIZE[level], TEXSIZE[level], 1, tempchar, 2, false);

			bool outputDM(true);
			if (outputDM) {
				//Decide solid_upper && pore_lower based on 3TIs
				PrepareDMapProjection(shortx, shorty, shortz, level);
				//redistribute distance values
				ProjectDMap(shortx, level);
				ProjectDMap(shorty, level);
				ProjectDMap(shortz, level);
			}

			m_exemplar_x[level] = vector<ANNcoord>(shortx.begin(), shortx.end());
			m_exemplar_y[level] = vector<ANNcoord>(shorty.begin(), shorty.end());
			m_exemplar_z[level] = vector<ANNcoord>(shortz.begin(), shortz.end());

			//draw distance model
			if (GenerateDMTI && level == MULTIRES - 1 && outputDM) {
				ostringstream name;

				Mat DM1 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
				DM1 = Mat(shortx, true).reshape(1, DM1.rows);
				name << "DM1_S" << (short)Solid_Upper << "P" << (short)Pore_Lower;	//if (MULTIRES > 1) name << "_L" << level;
				name << ".png";
				imwrite(name.str(), DM1);	name.str(""); //name.clear();not necessary

				Mat DM2 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
				DM2 = Mat(shorty, true).reshape(1, DM2.rows);
				name << "DM2_S" << (short)Solid_Upper << "P" << (short)Pore_Lower;	//if (MULTIRES > 1) name << "_L" << level;
				name << ".png";
				imwrite(name.str(), DM2);	name.str("");

				Mat DM3 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
				DM3 = Mat(shortz, true).reshape(1, DM3.rows);
				name << "DM3_S" << (short)Solid_Upper << "P" << (short)Pore_Lower; //if (MULTIRES > 1) name << "_L" << level;
				name << ".png";
				imwrite(name.str(), DM3);	name.str("");
			}

			if (BIMODAL_ON) {//prepare for bimodal transform
				NooutputDM(shortx, shorty, shortz);			//unknown bug
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

	CHANNEL_MAXVALUE = 256;
	if (DISTANCEMAP_ON) CHANNEL_MAXVALUE = DISCRETE_HISTOGRAM_BIN;

	cout << endl << "load TIs done.";
	return true;
}
//void DoPAR::calcNeighbor() {
//	//initialize kdtree for certain template/neighbourhood
//	cout << endl << "calcNeighbor...";
//	ANNsearchk.resize(MULTIRES);
//	ErrorBound.resize(MULTIRES);
//	PCA_RATIO_VARIANCE.resize(MULTIRES);
//	HisStdDev.resize(MULTIRES);
//	for (int level = 0; level < MULTIRES; ++level) {
//		// ============= ANNsearchk & ErrorBound now relates to neighbourhood size =================
//		//ANNsearchk[level] = 9;
//		ANNsearchk[level] = min(9, (MULTIRES-level)*4+1);
//		if (!INDEXHIS_ON) ANNsearchk[level] = 1;
//
//		//ErrorBound[level] = 0.5;
//		ErrorBound[level] = min(level+0.5, 2.0);
//		if (!INDEXHIS_ON) ErrorBound[level] = 1.0;
//
//		PCA_RATIO_VARIANCE[level] = max(0.95, 0.999-0.05*level);
//
//		HisStdDev[level] = min(0.015f, 0.005f*(MULTIRES - level));
//
//
//		cout << endl << "level:" << level;
//		int numData = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
//		//ann_index range=[0,numData), corresponds to (x,y) where x/y range=[N[level],TEXSIZE[level]-N[level])!
//		CvMat* p_source_x = cvCreateMat(numData, D_NEIGHBOR[level], CV_32F);	//rows='area' numData, cols=dimension (Neighbour size)
//		CvMat* p_source_y = cvCreateMat(numData, D_NEIGHBOR[level], CV_32F);
//		CvMat* p_source_z = cvCreateMat(numData, D_NEIGHBOR[level], CV_32F);
//		int row = 0;
//		for (int v = N[level]; v < TEXSIZE[level] - N[level]; ++v) {
//			for (int u = N[level]; u < TEXSIZE[level] - N[level]; ++u) {
//				int col = 0;
//				for (int dv = -N[level]; dv <= N[level]; ++dv) {
//					for (int du = -N[level]; du <= N[level]; ++du) {
//						ANNidx index = (TEXSIZE[level] * (v + dv) + u + du);
//						cvmSet(p_source_x, row, col, m_exemplar_x[level][index]);	//set p_source_x(row,col) to m_examplar_x(idx)
//						cvmSet(p_source_y, row, col, m_exemplar_y[level][index]);
//						cvmSet(p_source_z, row, col, m_exemplar_z[level][index]);
//
//						m_neighbor_x[level][D_NEIGHBOR[level] * row + col] = m_exemplar_x[level][index];
//						m_neighbor_y[level][D_NEIGHBOR[level] * row + col] = m_exemplar_y[level][index];
//						m_neighbor_z[level][D_NEIGHBOR[level] * row + col] = m_exemplar_z[level][index];
//						++col;
//					}
//				}
//				++row;
//			}
//		}
//		// PCA calculation (obtain all eigenvectors of the input covariance matrix)
//
//		////////每一行表示一个样本
//		//////CvMat* pData = cvCreateMat( 总的样本数, 每个样本的维数, CV_32FC1 );
//		if (mp_neighbor_pca_average_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_x[level]);
//		if (mp_neighbor_pca_average_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_y[level]);
//		if (mp_neighbor_pca_average_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_z[level]);
//		//CvMat* pMean = cvCreateMat(1, 样本的维数, CV_32FC1);
//		mp_neighbor_pca_average_x[level] = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
//		mp_neighbor_pca_average_y[level] = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
//		mp_neighbor_pca_average_z[level] = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
//		//pEigVals中的每个数表示一个特征值
//		//CvMat* pEigVals = cvCreateMat(1, min(总的样本数,样本的维数), CV_32FC1);
//		CvMat* p_eigenValues_x = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
//		CvMat* p_eigenValues_y = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
//		CvMat* p_eigenValues_z = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
//		//每一行表示一个特征向量
//		//CvMat* pEigVecs = cvCreateMat( min(总的样本数,样本的维数), 样本的维数, CV_32FC1);
//		CvMat* p_eigenVectors_all_x = cvCreateMat(D_NEIGHBOR[level], D_NEIGHBOR[level], CV_32F);
//		CvMat* p_eigenVectors_all_y = cvCreateMat(D_NEIGHBOR[level], D_NEIGHBOR[level], CV_32F);
//		CvMat* p_eigenVectors_all_z = cvCreateMat(D_NEIGHBOR[level], D_NEIGHBOR[level], CV_32F);
//		//PCA处理,计算出平均向量pMean,特征值pEigVals和特征向量pEigVecs
//		//cvCalcPCA(pData, pMean, pEigVals, pEigVecs, CV_PCA_DATA_AS_ROW);
//		//now have better function //PCA pca(data, mean, PCA::DATA_AS_ROW, 0.95);
//
//		cvCalcPCA(p_source_x, mp_neighbor_pca_average_x[level], p_eigenValues_x, p_eigenVectors_all_x, CV_PCA_DATA_AS_ROW);
//		cvCalcPCA(p_source_y, mp_neighbor_pca_average_y[level], p_eigenValues_y, p_eigenVectors_all_y, CV_PCA_DATA_AS_ROW);
//		cvCalcPCA(p_source_z, mp_neighbor_pca_average_z[level], p_eigenValues_z, p_eigenVectors_all_z, CV_PCA_DATA_AS_ROW);
//		// Decide amount of dimensionality reduction
//		double contribution_total_x = 0;
//		double contribution_total_y = 0;
//		double contribution_total_z = 0;
//		for (int i = 0; i < D_NEIGHBOR[level]; ++i) {
//			contribution_total_x += cvmGet(p_eigenValues_x, 0, i);
//			contribution_total_y += cvmGet(p_eigenValues_y, 0, i);
//			contribution_total_z += cvmGet(p_eigenValues_z, 0, i);
//		}
//
//		int dimPCA_x = 0;
//		int dimPCA_y = 0;
//		int dimPCA_z = 0;
//
//		char type;
//		do {
//			type = 'n';
//
//			dimPCA_x = 0;
//			dimPCA_y = 0;
//			dimPCA_z = 0;
//			double contribution_acc_x = 0;
//			double contribution_acc_y = 0;
//			double contribution_acc_z = 0;
//			for (int i = 0; i < D_NEIGHBOR[level]; ++i) {
//				double ratio_x = contribution_acc_x / contribution_total_x;
//				double ratio_y = contribution_acc_y / contribution_total_y;
//				double ratio_z = contribution_acc_z / contribution_total_z;
//				if (ratio_x < PCA_RATIO_VARIANCE[level]) {	//now have better function //PCA pca(data, mean, PCA::DATA_AS_ROW, 0.95);
//					contribution_acc_x += cvmGet(p_eigenValues_x, 0, i);
//					++dimPCA_x;
//				}
//				if (ratio_y < PCA_RATIO_VARIANCE[level]) {
//					contribution_acc_y += cvmGet(p_eigenValues_y, 0, i);
//					++dimPCA_y;
//				}
//				if (ratio_z < PCA_RATIO_VARIANCE[level]) {
//					contribution_acc_z += cvmGet(p_eigenValues_z, 0, i);
//					++dimPCA_z;
//				}
//				if (PCA_RATIO_VARIANCE[level] <= ratio_x && PCA_RATIO_VARIANCE[level] <= ratio_y && PCA_RATIO_VARIANCE[level] <= ratio_z) break;
//			}
//
//			cout << endl;
//			printf("PCA reduction (x): %d -> %d\n", D_NEIGHBOR[level], dimPCA_x);
//			printf("PCA reduction (y): %d -> %d\n", D_NEIGHBOR[level], dimPCA_y);
//			printf("PCA reduction (z): %d -> %d\n", D_NEIGHBOR[level], dimPCA_z);
//
//			//if (level == 0){
//			//	do{
//			//		cout << endl << "PCA_RATIO_VARIANCE= " << PCA_RATIO_VARIANCE << " Modify or not? [y / n]";
//			//		cin >> type;
//			//	} while (!cin.fail() && type != 'y' && type != 'n');
//			//	if (type == 'y'){
//			//		cout << endl << "new PCA_RATIO_VARIANCE(e.g.0.99)= ";
//			//		cin >> PCA_RATIO_VARIANCE;
//			//	}
//			//}
//		} while (type == 'y' && PCA_RATIO_VARIANCE[level] <= 1.0/* && PCA_RATIO_VARIANCE>0.85*/);
//
//		// Trim total eigenvectors into partial eigenvectors
//		if (mp_neighbor_pca_eigenvec_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x[level]);
//		if (mp_neighbor_pca_eigenvec_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y[level]);
//		if (mp_neighbor_pca_eigenvec_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z[level]);
//		mp_neighbor_pca_eigenvec_x[level] = cvCreateMat(dimPCA_x, D_NEIGHBOR[level], CV_32F);
//		mp_neighbor_pca_eigenvec_y[level] = cvCreateMat(dimPCA_y, D_NEIGHBOR[level], CV_32F);
//		mp_neighbor_pca_eigenvec_z[level] = cvCreateMat(dimPCA_z, D_NEIGHBOR[level], CV_32F);
//		memcpy(mp_neighbor_pca_eigenvec_x[level]->data.fl, p_eigenVectors_all_x->data.fl, sizeof(ANNcoord)* dimPCA_x * D_NEIGHBOR[level]);
//		memcpy(mp_neighbor_pca_eigenvec_y[level]->data.fl, p_eigenVectors_all_y->data.fl, sizeof(ANNcoord)* dimPCA_y * D_NEIGHBOR[level]);
//		memcpy(mp_neighbor_pca_eigenvec_z[level]->data.fl, p_eigenVectors_all_z->data.fl, sizeof(ANNcoord)* dimPCA_z * D_NEIGHBOR[level]);
//		// PCA projection
//		////// And copy the PCA results:
//		////Mat mean = pca.mean.clone();
//		////Mat eigenvalues = pca.eigenvalues.clone();
//		////Mat eigenvectors = pca.eigenvectors.clone();
//		////now better function
//		////dst = pca.project(mat); //CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
//		if (mp_neighbor_pca_projected_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x[level]);
//		if (mp_neighbor_pca_projected_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y[level]);
//		if (mp_neighbor_pca_projected_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z[level]);
//		//选出前P个特征向量(主成份),然后投影,结果保存在pResult中，pResult中包含了P个系数
//		//CvMat* pResult = cvCreateMat( 总的样本数, PCA变换后的样本维数(即主成份的数目)?, CV_32FC1 );
//		mp_neighbor_pca_projected_x[level] = cvCreateMat(numData, dimPCA_x, CV_32F);
//		mp_neighbor_pca_projected_y[level] = cvCreateMat(numData, dimPCA_y, CV_32F);
//		mp_neighbor_pca_projected_z[level] = cvCreateMat(numData, dimPCA_z, CV_32F);
//		//cvProjectPCA( pData, pMean, pEigVecs, pResult );
//		cvProjectPCA(p_source_x, mp_neighbor_pca_average_x[level], mp_neighbor_pca_eigenvec_x[level], mp_neighbor_pca_projected_x[level]);
//		cvProjectPCA(p_source_y, mp_neighbor_pca_average_y[level], mp_neighbor_pca_eigenvec_y[level], mp_neighbor_pca_projected_y[level]);
//		cvProjectPCA(p_source_z, mp_neighbor_pca_average_z[level], mp_neighbor_pca_eigenvec_z[level], mp_neighbor_pca_projected_z[level]);
//		// kd-tree construction
//		m_neighbor_kdTree_ptr_x[level].resize(numData);
//		m_neighbor_kdTree_ptr_y[level].resize(numData);
//		m_neighbor_kdTree_ptr_z[level].resize(numData);
//		for (int i = 0; i < numData; ++i) {
//			//ANNpoint* point array, row = 1, col = PCA dimension
//			m_neighbor_kdTree_ptr_x[level][i] = &mp_neighbor_pca_projected_x[level]->data.fl[dimPCA_x * i];	//ANNpoint* from PCA projection
//			//vector<uchar> array(mat.rows*mat.cols);
//			//if (mat.isContinuous())
//			//	array = mat.data;
//			m_neighbor_kdTree_ptr_y[level][i] = &mp_neighbor_pca_projected_y[level]->data.fl[dimPCA_y * i];
//			m_neighbor_kdTree_ptr_z[level][i] = &mp_neighbor_pca_projected_z[level]->data.fl[dimPCA_z * i];
//		}
//		if (mp_neighbor_kdTree_x[level] != NULL) delete mp_neighbor_kdTree_x[level];
//		if (mp_neighbor_kdTree_y[level] != NULL) delete mp_neighbor_kdTree_y[level];
//		if (mp_neighbor_kdTree_z[level] != NULL) delete mp_neighbor_kdTree_z[level];
//		//ANNpoint* data point array = m_neighbor_kdTree_ptr_x, number of points = numData, dimension = dimPCA_x
//		mp_neighbor_kdTree_x[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_x[level][0], numData, dimPCA_x); //ANNkd_tree
//		mp_neighbor_kdTree_y[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_y[level][0], numData, dimPCA_y);
//		mp_neighbor_kdTree_z[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_z[level][0], numData, dimPCA_z);
//
//		//============ TEST TI PCA backproject result
//		if (GenerateDMTI && level == MULTIRES - 1 && FALSE) {
//			CvMat* backproject_x = cvCreateMat(numData, D_NEIGHBOR[level], CV_32F);
//			cvBackProjectPCA(mp_neighbor_pca_projected_x[level], mp_neighbor_pca_average_x[level], mp_neighbor_pca_eigenvec_x[level], backproject_x);
//			Mat backprojectMat_x = cvarrToMat(backproject_x);
//			Mat PCAbackprojectDM1 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
//			PCAbackprojectDM1 = Mat(m_exemplar_x[level], true).reshape(1, PCAbackprojectDM1.rows);
//
//			CvMat* backproject_y = cvCreateMat(numData, D_NEIGHBOR[level], CV_32F);
//			cvBackProjectPCA(mp_neighbor_pca_projected_y[level], mp_neighbor_pca_average_y[level], mp_neighbor_pca_eigenvec_y[level], backproject_y);
//			Mat backprojectMat_y = cvarrToMat(backproject_y);
//			Mat PCAbackprojectDM2 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
//			PCAbackprojectDM2 = Mat(m_exemplar_y[level], true).reshape(1, PCAbackprojectDM2.rows);
//
//			CvMat* backproject_z = cvCreateMat(numData, D_NEIGHBOR[level], CV_32F);
//			cvBackProjectPCA(mp_neighbor_pca_projected_z[level], mp_neighbor_pca_average_z[level], mp_neighbor_pca_eigenvec_z[level], backproject_z);
//			Mat backprojectMat_z = cvarrToMat(backproject_z);
//			Mat PCAbackprojectDM3 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
//			PCAbackprojectDM3 = Mat(m_exemplar_z[level], true).reshape(1, PCAbackprojectDM3.rows);
//
//			int row = 0;
//			int cols = 0.5*((2 * N[level] + 1)*(2 * N[level] + 1) - 1);
//			for (int v = N[level]; v < TEXSIZE[level] - N[level]; ++v) {
//				for (int u = N[level]; u < TEXSIZE[level] - N[level]; ++u) {
//					PCAbackprojectDM1.at<uchar>(v, u) = backprojectMat_x.at<ANNcoord>(row, cols);
//					PCAbackprojectDM2.at<uchar>(v, u) = backprojectMat_y.at<ANNcoord>(row, cols);
//					PCAbackprojectDM3.at<uchar>(v, u) = backprojectMat_z.at<ANNcoord>(row, cols);
//					++row;
//				}
//			}
//
//			imwrite("PCAbackproject_DM1.png", PCAbackprojectDM1);
//			imwrite("PCAbackproject_DM2.png", PCAbackprojectDM2);
//			imwrite("PCAbackproject_DM3.png", PCAbackprojectDM3);
//			cvReleaseMat(&backproject_x);
//			cvReleaseMat(&backproject_y);
//			cvReleaseMat(&backproject_z);
//			cout << endl << "PCA back projected image outputed.";
//		}
//
//		// release CV matrices
//		cvReleaseMat(&p_source_x);
//		cvReleaseMat(&p_source_y);
//		cvReleaseMat(&p_source_z);
//		cvReleaseMat(&p_eigenValues_x);
//		cvReleaseMat(&p_eigenValues_y);
//		cvReleaseMat(&p_eigenValues_z);
//		cvReleaseMat(&p_eigenVectors_all_x);
//		cvReleaseMat(&p_eigenVectors_all_y);
//		cvReleaseMat(&p_eigenVectors_all_z);
//	}
//
//	cout << endl << "calcNeighbor done.";
//}
void DoPAR::computeKCoherence()
{
	cout << endl << "K-coherence...";

	//==========multiple nearest index, position control=========
	ANNidxArray ann_index_x = new ANNidx[CoherenceNUM];
	ANNidxArray ann_index_y = new ANNidx[CoherenceNUM];
	ANNidxArray ann_index_z = new ANNidx[CoherenceNUM];
	ANNdistArray ann_dist_x = new ANNdist[CoherenceNUM];
	ANNdistArray ann_dist_y = new ANNdist[CoherenceNUM];
	ANNdistArray ann_dist_z = new ANNdist[CoherenceNUM];

	for (int level = 0; level < MULTIRES; ++level) {
		ANNidx numData = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
		int dim = D_NEIGHBOR[level];
		cout << endl << "level: " << level << "Dimension=" << dim;
		
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

		int row = 0;
		for (int v = N[level]; v < TEXSIZE[level] - N[level]; ++v) {
			for (int u = N[level]; u < TEXSIZE[level] - N[level]; ++u) {
				int col = 0;
				for (int dv = -N[level]; dv <= N[level]; ++dv) {
					for (int du = -N[level]; du <= N[level]; ++du) {
						ANNidx index = (TEXSIZE[level] * (v + dv) + u + du);
						p_source_x[row][col] = m_exemplar_x[level][index];		//set p_source_x(row,col) to m_examplar_x(idx)
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

		//ANN search
		for (int v = N[level]; v < TEXSIZE[level] - N[level]; ++v) {
			for (int u = N[level]; u < TEXSIZE[level] - N[level]; ++u) {
				int num = 0;
				ANNidx TIindex = (TEXSIZE[level] * (v) + u);					//(v,u)
				for (int dv = -N[level]; dv <= N[level]; ++dv) {
					for (int du = -N[level]; du <= N[level]; ++du) {
						ANNidx index = TIindex + (TEXSIZE[level] * dv + du);
						queryPt_x[num] = m_exemplar_x[level][index];			//set queryPt_x(num) to m_examplar_x(idx)
						queryPt_y[num] = m_exemplar_y[level][index];
						queryPt_z[num] = m_exemplar_z[level][index];
						num++;
					}
				}

				kdTree_x->annkSearch(queryPt_x, CoherenceNUM, ann_index_x, ann_dist_x, 0);
				kdTree_y->annkSearch(queryPt_y, CoherenceNUM, ann_index_y, ann_dist_y, 0);
				kdTree_z->annkSearch(queryPt_z, CoherenceNUM, ann_index_z, ann_dist_z, 0);

				//Set K-Coherence
				for (int k = 0; k < CoherenceNUM; ++k)
				{
					KCoherence_x[level][TIindex][k] = convertIndexANN(level, ann_index_x[k]);
					KCoherence_y[level][TIindex][k] = convertIndexANN(level, ann_index_y[k]);
					KCoherence_z[level][TIindex][k] = convertIndexANN(level, ann_index_z[k]);
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



bool DoPAR::loadVolume() {
	//----------------convert Model(vector<uchar>) to m_volume (vector<vector<int>> (multires,x*y*z))
	//load from Model, later can also load from file		//level 0
	if (fileExists(modelFilename3D.c_str()) == true) {
		vector<uchar> model = load3Dmodel(modelFilename3D.c_str());
		if (model.size() != TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]) { cout << endl << "Loaded Model size=" << model.size() << " should be=" << TEXSIZE[0] * TEXSIZE[0] * TEXSIZE[0]; _getch(); exit(1); }
		else cout << endl << "3D model loaded.";

		if (DISTANCEMAP_ON) {
			//============= Convert to distance model ==========================
			long PoreNum = 0;
			short autothresholdvalue(0);
			bool binaryYN(true);
			if (binaryYN) for (int i = 0; i < model.size(); i++) { if ((model[i] != 0 && model[i] != 1) && (model[i] != 255 && model[i] != 0)) { binaryYN = false; break; } }
			if (binaryYN == false) {
				cout << endl << "Input autothresholdvalue for LoadedModel (default=110): ";
				cin >> autothresholdvalue;
			}
			for (ANNidx idx = 0; idx < model.size(); idx++) {
				if (model[idx] > autothresholdvalue) PoreNum++;
			}
			porosityModel = 1.0*PoreNum / model.size();
			cout << endl << "Loaded Model porosity= " << porosityModel;

			vector<short> shortmodel(model.begin(), model.end());
			vector<char> tempchar(model.size());
			BinariseThreshold(shortmodel, tempchar, autothresholdvalue);
			shortmodel = GetDMap(TEXSIZE[0], TEXSIZE[0], TEXSIZE[0], tempchar, 2, false);

			//redistribute distance values
			ProjectDMap(shortmodel, 0);

			m_volume[0] = vector<ANNcoord>(shortmodel.begin(), shortmodel.end());

			vector<uchar> tempmodel = vector<uchar>(shortmodel.begin(), shortmodel.end());
			string tempoutputfilename = outputfilename;
			tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "loadedDM" + ".RAW";
			Write(outputpath + tempoutputfilename, tempmodel);
		}
		else {
			m_volume[0] = vector<ANNcoord>(model.begin(), model.end());
		}
		cout << endl << "load 3D model done.";
	}
	else {
		cout << endl << "3D initial model doesn't exist. Use Random initial.";
		InitRandomVolume(0);				//no need to convert again.
		cout << endl << "Init Random Volume Done.";
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

void DoPAR::upsampleVolume(int level) {
	//============= Convert back to binary model first [leve]==========================
	vector<char> tempchar;
	vector<short> shortmodel;
	double porosity(0.0);
	if (DISTANCEMAP_ON) {
		tempchar.resize(m_volume[level].size());
		shortmodel = vector<short>(m_volume[level].begin(), m_volume[level].end());
		BinariseThreshold(shortmodel, tempchar, Solid_Upper);				//[0,1]
		for_each(tempchar.rbegin(), tempchar.rend(), [&](char n) { porosity += n; });
		porosity /= tempchar.size();
		//cout << endl << "porosity=" << porosity; _getch();

		m_volume[level] = vector<ANNcoord>(tempchar.begin(), tempchar.end());		//[0,1]
		transform(m_volume[level].begin(), m_volume[level].end(), m_volume[level].begin(), bind2nd(multiplies<ANNcoord>(), 255)); //[0,255]
	}

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

	//============= Convert to distance model [level+1]==========================
	if (DISTANCEMAP_ON) {
		shortmodel.clear();
		tempchar.clear(); tempchar.resize(m_volume[level + 1].size());
		shortmodel = vector<short>(m_volume[level + 1].begin(), m_volume[level + 1].end());

		//BinariseThreshold(shortmodel, tempchar, 128);
		BinariseImg(shortmodel, porosity);
		shortmodel = GetDMap(TEXSIZE[level + 1], TEXSIZE[level + 1], TEXSIZE[level + 1], tempchar, 2, false);

		//redistribute distance values
		ProjectDMap(shortmodel, level + 1);

		m_volume[level + 1] = vector<ANNcoord>(shortmodel.begin(), shortmodel.end());
	}
}

void DoPAR::outputmodel(int level) {
	//if (true) {
	//	DynamicThreshold(level);
	//}

	vector<uchar> tempmodel;
	tempmodel = vector<uchar>(m_volume[level].begin(), m_volume[level].end());
	string tempoutputfilename = outputfilename;
	if (level != MULTIRES-1) tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "_L" + to_string(level) + ".RAW";

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
void DoPAR::DynamicThreshold(int level) {
	long TIporecount(0);
	for (long i = 0; i < m_exemplar_x[MULTIRES - 1].size(); i++)  if (m_exemplar_x[MULTIRES - 1][i] >= Pore_Lower) TIporecount++;
	for (long i = 0; i < m_exemplar_y[MULTIRES - 1].size(); i++)  if (m_exemplar_y[MULTIRES - 1][i] >= Pore_Lower) TIporecount++;
	for (long i = 0; i < m_exemplar_z[MULTIRES - 1].size(); i++)  if (m_exemplar_z[MULTIRES - 1][i] >= Pore_Lower) TIporecount++;

	float TIporosity = TIporecount*1.0 / (m_exemplar_x[level].size() + m_exemplar_y[level].size() + m_exemplar_z[level].size());

	long Modelporecount(0), lastporecount(0), Idealporecount(TIporosity*m_volume[level].size());
	short thresholdvalue = Solid_Upper - 1;
	do {
		lastporecount = Modelporecount;

		thresholdvalue++;
		Modelporecount = 0;
		for (long i = 0; i < m_volume[level].size(); i++) {
			if (m_volume[level][i] > thresholdvalue) Modelporecount++;
		}
	} while (abs(Modelporecount - Idealporecount) < abs(lastporecount - Idealporecount) && thresholdvalue < Pore_Lower);

	thresholdvalue -= 1;
	for (long i = 0; i < m_volume[level].size(); i++) {
		if (m_volume[level][i] > thresholdvalue) m_volume[level][i] = 255;
		else m_volume[level][i] = 0;
	}

	cout << endl << "DynamicThreshold solid_upper=" << thresholdvalue;
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

void DoPAR::showMat(const cv::String& winname, const cv::Mat& mat)
{// Show a Mat object quickly. For testing purposes only.
	assert(!mat.empty());
	cv::namedWindow(winname);
	cv::imshow(winname, mat);
	cv::waitKey(0);
	cv::destroyWindow(winname);
}

void DoPAR::cleardata(int level) {
	//m_exemplar_x[level].clear();
	//m_exemplar_y[level].clear();
	//m_exemplar_z[level].clear();
	m_neighbor_x[level].clear();
	m_neighbor_y[level].clear();
	m_neighbor_z[level].clear();
	m_volume[level].clear();
	//m_permutation_xyz.clear();
	//m_volume_nearest_x_index[level].clear();
	//m_volume_nearest_y_index[level].clear();
	//m_volume_nearest_z_index[level].clear();
	//m_volume_nearest_x_dist[level].clear();
	//m_volume_nearest_y_dist[level].clear();
	//m_volume_nearest_z_dist[level].clear();
	//m_volume_weight_x[level].clear();
	//m_volume_weight_y[level].clear();
	//m_volume_weight_z[level].clear();
	//m_neighbor_kdTree_ptr_x[level].clear();
	//m_neighbor_kdTree_ptr_y[level].clear();
	//m_neighbor_kdTree_ptr_z[level].clear();
	//!!!Every use of new should be balanced by a delete, and every use of new[] should be balanced by delete[]
	//if (mp_neighbor_kdTree_x[level] != NULL) delete mp_neighbor_kdTree_x[level];
	//if (mp_neighbor_kdTree_y[level] != NULL) delete mp_neighbor_kdTree_y[level];
	//if (mp_neighbor_kdTree_z[level] != NULL) delete mp_neighbor_kdTree_z[level];
	//if (mp_neighbor_pca_average_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_x[level]);
	//if (mp_neighbor_pca_average_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_y[level]);
	//if (mp_neighbor_pca_average_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_average_z[level]);
	//if (mp_neighbor_pca_projected_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_x[level]);
	//if (mp_neighbor_pca_projected_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_y[level]);
	//if (mp_neighbor_pca_projected_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_projected_z[level]);
	//if (mp_neighbor_pca_eigenvec_x[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_x[level]);
	//if (mp_neighbor_pca_eigenvec_y[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_y[level]);
	//if (mp_neighbor_pca_eigenvec_z[level] != NULL) cvReleaseMat(&mp_neighbor_pca_eigenvec_z[level]);
}


//================= phase 1: search ===========================
void DoPAR::searchVolume(int level) {
	long time_start = clock();	cout << endl << "phase1:searching";
	ANNdist global_energy_new = 0.0f;
	const ANNdist min_dist = 0.00001f;

	const ANNidx Sx = TEXSIZE[level];
	const ANNidx Sy = TEXSIZE[level];
	const ANNidx Sz = TEXSIZE[level];
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const ANNidx IdxHis1 = 0;
	const ANNidx IdxHis2 = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	const ANNidx IdxHis3 = 2 * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	//const float dSxy = 1.0f / Sxy, dSx = 1.0f / Sx;

	//========For index histogram counting, shuffle the order. ========
	srand(time(NULL)); //jzy
	vector<bool> AccYN(Size, false);
	vector<long> RemIdxLst;	RemIdxLst.reserve(0.1*Sx*Sy*Sz);
	long RemIIdx = -1;
	/////shuffle(m_permutation_xyz.begin(), m_permutation_xyz.end(), mersennetwistergenerator);

	//==========multiple nearest index, position control=========
	ANNidxArray ann_index_x = new ANNidx[ANNsearchk];
	ANNidxArray ann_index_y = new ANNidx[ANNsearchk];
	ANNidxArray ann_index_z = new ANNidx[ANNsearchk];
	ANNdistArray ann_dist_x = new ANNdist[ANNsearchk];
	ANNdistArray ann_dist_y = new ANNdist[ANNsearchk];
	ANNdistArray ann_dist_z = new ANNdist[ANNsearchk];

	//===========obtain current neighborhood_x from volume, no need to new every loop====
	CvMat* current_neighbor_x = cvCreateMat(1, D_NEIGHBOR[level], CV_32F); //rows = 1, cols = dimesnion
	CvMat* current_neighbor_y = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
	CvMat* current_neighbor_z = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
	// PCA projection
	int dimPCA_x = mp_neighbor_pca_eigenvec_x[level]->rows;	//computed already
	int dimPCA_y = mp_neighbor_pca_eigenvec_y[level]->rows;
	int dimPCA_z = mp_neighbor_pca_eigenvec_z[level]->rows;
	//===========no need to new every loop=============
	//project current_neighbor_x to current_neighbor_x_projected, dimension = dimPCA_x
	CvMat* current_neighbor_x_projected = cvCreateMat(1, dimPCA_x, CV_32F);
	CvMat* current_neighbor_y_projected = cvCreateMat(1, dimPCA_y, CV_32F);
	CvMat* current_neighbor_z_projected = cvCreateMat(1, dimPCA_z, CV_32F);

	//===========Remove m_volume_nearest_x_dist[level][i];no need to store them=======
	ANNdist nearest_x_dist, nearest_y_dist, nearest_z_dist;

	ANNidx displayprocess = -1;
	ANNidx JCntNum = Size / 10.0 + 1;
	const ANNidx MULTIRES_1 = MULTIRES - 1;
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		//for each point in volume	(the sequence doesnt matter, all the points are searched and then optimize)
		if (level == MULTIRES_1) {
			if (i2 % JCntNum == 0) {
				displayprocess++;
				cout << "\r" << displayprocess * 10 << "%..";
			}
		}
		//========For index histogram counting, use shuffled index========
		ANNidx z = rand() % Sz;		//[0,Sz-1]
		ANNidx y = rand() % Sy;
		ANNidx x = rand() % Sx;
		ANNidx i = x + y*Sx + z*Sxy;
		//cout << endl << "i2=" << i2 << "  (" << x << ", " << y << ", " << z << ")";

		if (!AccYN[i]) {
			AccYN[i] = true;
		LoopAccYN:

			//// obtain current neighborhood_x from volume
			//CvMat* current_neighbor_x = cvCreateMat(1, D_NEIGHBOR[level], CV_32F); //rows = 1, cols = dimesnion
			//CvMat* current_neighbor_y = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);
			//CvMat* current_neighbor_z = cvCreateMat(1, D_NEIGHBOR[level], CV_32F);

			ANNidx index = 0;
			ANNidx TXy = Sx * y;
			ANNidx TXYz = Sxy * z;
			for (ANNidx dv = -N[level]; dv <= N[level]; ++dv) {	//N is neighbourhood size.
				ANNidx VCurIdx1 = Sxy * trimIndex(level, z + dv) + x;
				ANNidx VCurIdx2 = Sxy * trimIndex(level, z + dv) + TXy;
				ANNidx VCurIdx3 = TXYz + Sx * trimIndex(level, y + dv);
				for (ANNidx du = -N[level]; du <= N[level]; ++du) {
					ANNidx index2_x = VCurIdx1 + Sx * trimIndex(level, y + du);							// i + (dv, du, 0)
					ANNidx index2_y = VCurIdx2 + trimIndex(level, x + du);								// i + (dv, 0, du)
					ANNidx index2_z = VCurIdx3 + trimIndex(level, x + du);								// i + (0, dv, du)
					//ANNidx index2_x =  Sxy * trimIndex(level, z + dv) + Sx * trimIndex(level, y + du) + x;
					//ANNidx index2_y =  Sxy * trimIndex(level, z + dv) + Sx * y + trimIndex(level, x + du);
					//ANNidx index2_z =  Sxy * z + TEXSIZE[level] * trimIndex(level, y + dv) + trimIndex(level, x + du);

					cvmSet(current_neighbor_x, 0, index, m_volume[level][index2_x]);	//set current_neighbor_x(0,col) to m_volume(idx)
					cvmSet(current_neighbor_y, 0, index, m_volume[level][index2_y]);
					cvmSet(current_neighbor_z, 0, index, m_volume[level][index2_z]);
					index += 1;
				}
			}

			//// PCA projection
			//int dimPCA_x = mp_neighbor_pca_eigenvec_x[level]->rows;	//computed already
			//int dimPCA_y = mp_neighbor_pca_eigenvec_y[level]->rows;
			//int dimPCA_z = mp_neighbor_pca_eigenvec_z[level]->rows;
			////project current_neighbor_x to current_neighbor_x_projected, dimension = dimPCA_x
			//CvMat* current_neighbor_x_projected = cvCreateMat(1, dimPCA_x, CV_32F);
			//CvMat* current_neighbor_y_projected = cvCreateMat(1, dimPCA_y, CV_32F);
			//CvMat* current_neighbor_z_projected = cvCreateMat(1, dimPCA_z, CV_32F);

			cvProjectPCA(current_neighbor_x, mp_neighbor_pca_average_x[level], mp_neighbor_pca_eigenvec_x[level], current_neighbor_x_projected);
			cvProjectPCA(current_neighbor_y, mp_neighbor_pca_average_y[level], mp_neighbor_pca_eigenvec_y[level], current_neighbor_y_projected);
			cvProjectPCA(current_neighbor_z, mp_neighbor_pca_average_z[level], mp_neighbor_pca_eigenvec_z[level], current_neighbor_z_projected);

			// ANN search. error bound = 2.0; Kopf used 2.0
			mp_neighbor_kdTree_x[level]->annkSearch(current_neighbor_x_projected->data.fl, ANNsearchk, ann_index_x, ann_dist_x, ErrorBound);
			mp_neighbor_kdTree_y[level]->annkSearch(current_neighbor_y_projected->data.fl, ANNsearchk, ann_index_y, ann_dist_y, ErrorBound);
			mp_neighbor_kdTree_z[level]->annkSearch(current_neighbor_z_projected->data.fl, ANNsearchk, ann_index_z, ann_dist_z, ErrorBound);

			//// CV release
			//cvReleaseMat(&current_neighbor_x);
			//cvReleaseMat(&current_neighbor_y);
			//cvReleaseMat(&current_neighbor_z);
			//cvReleaseMat(&current_neighbor_x_projected);
			//cvReleaseMat(&current_neighbor_y_projected);
			//cvReleaseMat(&current_neighbor_z_projected);

			int selected_index_x(0), selected_index_y(0), selected_index_z(0);
			//==========multiple nearest index, Index Histogram matching=========
			if (INDEXHIS_ON && ANNsearchk>1) {
				selected_index_x = indexhistmatching_ann_index(level, 0, ann_index_x);
				selected_index_y = indexhistmatching_ann_index(level, 1, ann_index_y);
				selected_index_z = indexhistmatching_ann_index(level, 2, ann_index_z);

				ANNidx newidx, oldidx;
				//update index histogram
				newidx = ann_index_x[selected_index_x] + IdxHis1;
				oldidx = m_volume_nearest_x_index[level][i] + IdxHis1;	//ori*Nxy
				updateIndexHistogram(level, oldidx, newidx);			//level, old, new

				newidx = ann_index_y[selected_index_y] + IdxHis2;
				oldidx = m_volume_nearest_y_index[level][i] + IdxHis2;
				updateIndexHistogram(level, oldidx, newidx);

				newidx = ann_index_z[selected_index_z] + IdxHis3;
				oldidx = m_volume_nearest_z_index[level][i] + IdxHis3;
				updateIndexHistogram(level, oldidx, newidx);
			}

			//update nearest_index, nearest_dist
			m_volume_nearest_x_index[level][i] = ann_index_x[selected_index_x];
			m_volume_nearest_y_index[level][i] = ann_index_y[selected_index_y];
			m_volume_nearest_z_index[level][i] = ann_index_z[selected_index_z];
			//m_volume_nearest_x_dist[level][i] = ann_dist_x[selected_index_x] + min_dist;
			//m_volume_nearest_y_dist[level][i] = ann_dist_y[selected_index_y] + min_dist;
			//m_volume_nearest_z_dist[level][i] = ann_dist_z[selected_index_z] + min_dist;
			nearest_x_dist = ann_dist_x[selected_index_x] + min_dist;
			nearest_y_dist = ann_dist_y[selected_index_y] + min_dist;
			nearest_z_dist = ann_dist_z[selected_index_z] + min_dist;
			if (nearest_x_dist < 0) cout << endl << "nearest_x_dist=" << nearest_x_dist << " selected_index_x=" << selected_index_x;
			if (nearest_y_dist < 0) cout << endl << "nearest_y_dist=" << nearest_y_dist << " selected_index_y=" << selected_index_y;
			if (nearest_z_dist < 0) cout << endl << "nearest_z_dist=" << nearest_z_dist << " selected_index_z=" << selected_index_z;

			//update weight for optimize step
			//m_volume_weight_x[level][i] = nearest_x_dist;
			//m_volume_weight_y[level][i] = nearest_y_dist;
			//m_volume_weight_z[level][i] = nearest_z_dist;
			m_volume_weight_x[level][i] = pow(nearest_x_dist, -0.6f);
			m_volume_weight_y[level][i] = pow(nearest_y_dist, -0.6f);
			m_volume_weight_z[level][i] = pow(nearest_z_dist, -0.6f);

			//search all points then optimize! Not search one point optimize one point!
			global_energy_new += nearest_x_dist + nearest_y_dist + nearest_z_dist;
			if (global_energy_new < 0) cout << endl << "g_new=" << global_energy_new << " dist_x=" << nearest_x_dist << " dist_y=" << nearest_y_dist << " dist_z=" << nearest_z_dist;
			//global_energy_new += m_volume_nearest_x_dist[level][i] + m_volume_nearest_y_dist[level][i] + m_volume_nearest_z_dist[level][i];
		}//if (!AccYN[i])

		if (FALSE && i2 == Size - 1) {
			if (RemIIdx == -1) {
				RemIIdx++;
				for (ANNidx ij = 0; ij < Size; ++ij) {
					if (!AccYN[ij])	RemIdxLst.push_back(ij);
				}
				AccYN.clear();
			}
			if (RemIIdx < RemIdxLst.size()) {
				i = RemIdxLst[RemIIdx++];
				x = i % Sx;
				y = (i / Sx) % Sy;
				z = (i / Sxy) % Sz;
				////z = i*dSxy;	//buggy!
				////y = (i - z*Sxy)*dSx;
				////x = i % TEXSIZE[level];
				goto LoopAccYN;
			}
		}
	}//for every voxel

	 //Release ann_index
	//!!!Every use of new should be balanced by a delete, and every use of new[] should be balanced by delete[];
	//!!delete releases the memory of a single element allocated using new, delete[] releases the memory allocated for arrays of elements using new and a size in brackets [].
	delete[] ann_index_x;		delete[] ann_index_y;		delete[] ann_index_z;
	delete[] ann_dist_x;		delete[] ann_dist_y;		delete[] ann_dist_z;
	// CV release
	cvReleaseMat(&current_neighbor_x);
	cvReleaseMat(&current_neighbor_y);
	cvReleaseMat(&current_neighbor_z);
	cvReleaseMat(&current_neighbor_x_projected);
	cvReleaseMat(&current_neighbor_y_projected);
	cvReleaseMat(&current_neighbor_z_projected);

	long time_end = clock();
	cout << " done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
	global_energy_new /= (3 * Size * D_NEIGHBOR[level]);
	perpixel_energy_new = global_energy_new;
	cout << ", per pixel energy: " << perpixel_energy_new;
}


//---------- Index Histogram for search step ---------
void DoPAR::initIndexHistogram() {
	if (FIRSTRUN == false) { cout << endl << "Error: FIRSTRUN==false in initIndexHistogram()"; _getch(); exit(1); }

	for (int level = 0; level < MULTIRES; level++) {
		long numData = 3 * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);

		m_indexhistogram_synthesis[level].clear();
		m_indexhistogram_synthesis[level].resize(numData, 0.0f);
	}

	//writeIndexHistogram(level, m_indexhistogram_synthesis[level],  3 * (TEXSIZE[level] - 2 * N[level]), TEXSIZE[level] - 2 * N[level], "IndexHis.png");
}
void DoPAR::updateIndexHistogram(int level, const ANNidx oldannidx, const ANNidx newannidx) {
	if (FIRSTRUN) {
		m_indexhistogram_synthesis[level][newannidx] += 1.0f;
	}
	else {
		m_indexhistogram_synthesis[level][oldannidx] -= 1.0f;
		m_indexhistogram_synthesis[level][newannidx] += 1.0f;
	}
}
int DoPAR::indexhistmatching_ann_index(int level, int orientation, ANNidxArray& idxarray) {
	ANNidx size = orientation*(TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);

	//simple form works better: just select minimum frequency
	vector<float> frequecyarray(ANNsearchk, 0.0f);
	for (int i = 0; i < ANNsearchk; i++) {
		ANNidx idx = idxarray[i] + size;
		frequecyarray[i] = m_indexhistogram_synthesis[level][idx];
	}
	auto i = min_element(begin(frequecyarray), end(frequecyarray));

	return distance(begin(frequecyarray), i);
}



//================= phase 2: optimization =====================
void DoPAR::optimizeVolume(int level) {
	long time_start = clock();
	const ANNidx Sx = TEXSIZE[level];
	const ANNidx Sy = TEXSIZE[level];
	const ANNidx Sz = TEXSIZE[level];
	const ANNidx Sxy = Sx * Sy;
	const ANNidx Sxz = Sx * Sz;
	const ANNidx Syz = Sy * Sz;
	const ANNidx Size = Sxy * Sz;
	const ANNidx IdxHis1 = 0;
	const ANNidx IdxHis2 = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	const ANNidx IdxHis3 = 2 * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	//const float dSxy = 1.0f / Sxy, dSx = 1.0f / Sx;
	const ANNidx DNEIGHBOR = D_NEIGHBOR[level];

	
	const ANNdist devidedbyHisGaussianProbZero = 1.0f / gaussian_pdf(0.0f, 0.0f, HisStdDev[level]);

	//========discrete solver=================
	vector<ANNcoord> colorset(3 * DNEIGHBOR, -1);
	vector<ANNidx> positionset(3 * DNEIGHBOR, -1);

	//shuffle order
	vector<bool> AccYN(Size, false);
	vector<long> RemIdxLst;	RemIdxLst.reserve(0.1*Sx*Sy*Sz);
	long RemIIdx = -1;

	cout << endl << "phase2:optimizing...";
	ANNidx displayprocess = -1;
	ANNidx JCntNum = Size / 10.0 + 1;
	const ANNidx MULTIRES_1 = MULTIRES - 1;
	for (ANNidx i2 = 0; i2 < Size; ++i2) {
		if (level == MULTIRES_1) {
			if (i2 % JCntNum == 0) {
				displayprocess++;
				cout << "\r" << displayprocess * 10 << "%..";
			}
		}

		ANNidx z = rand() % Sz;
		ANNidx y = rand() % Sy;
		ANNidx x = rand() % Sx;
		ANNidx i = x + y*Sx + z*Sxy;

		if (!AccYN[i]) {
			AccYN[i] = true;
		SearchLoop:

			ANNdist weight_acc = 0.0f;
			ANNcoord color_acc = 0.0f;

			ANNidx m = 0;
			//for every voxel's neighbourhood, in 3 orientations
			ANNidx TXy = Sx * y;
			ANNidx TXYz = Sxy * z;

			ANNidx VCurIdx1, VCurIdx2, VCurIdx3;
			ANNidx index2_x, index2_y, index2_z;
			ANNidx  nearest_index;
			ANNdist weight;//ANNdist nearest_dist;
			//ANNcoord* p_neighbor;
			ANNcoord color;// color of overlapping neighborhood pixel
			ANNidx bin;
			ANNdist Hisgaussianprob;
			float histogram_matching;

			for (ANNidx dv = -N[level]; dv <= N[level]; ++dv) {	//N is neighbourhood size.
				VCurIdx1 = Sxy * trimIndex(level, z + dv) + x;
				VCurIdx2 = Sxy * trimIndex(level, z + dv) + TXy;
				VCurIdx3 = TXYz + Sx * trimIndex(level, y + dv);
				for (ANNidx du = -N[level]; du <= N[level]; ++du) {
					index2_x = VCurIdx1 + Sx * trimIndex(level, y + du);						// i + (dv, du, 0)
					index2_y = VCurIdx2 + trimIndex(level, x + du);								// i + (dv, 0, du)
					index2_z = VCurIdx3 + trimIndex(level, x + du);								// i + (0, dv, du)

					for (int ori = 0; ori < 3; ++ori) {//3 orientations		
						switch (ori) {
						case (0) : {
							nearest_index = m_volume_nearest_x_index[level][index2_x];
							//nearest_dist = m_volume_nearest_x_dist[level][index2_x];
							weight = m_volume_weight_x[level][index2_x];
							//p_neighbor = &m_neighbor_x[level][D_NEIGHBOR[level] * nearest_index];
							color = m_neighbor_x[level][DNEIGHBOR * nearest_index + DNEIGHBOR - 1 - m];
							break;
						}
						case (1) : {
							nearest_index = m_volume_nearest_y_index[level][index2_y];
							//nearest_dist = m_volume_nearest_y_dist[level][index2_y];
							weight = m_volume_weight_y[level][index2_y];
							//p_neighbor = &m_neighbor_y[level][D_NEIGHBOR[level] * nearest_index];
							color = m_neighbor_y[level][DNEIGHBOR * nearest_index + DNEIGHBOR - 1 - m];
							break;
						}
						default: {
							nearest_index = m_volume_nearest_z_index[level][index2_z];
							//nearest_dist = m_volume_nearest_z_dist[level][index2_z];
							weight = m_volume_weight_z[level][index2_z];
							//p_neighbor = &m_neighbor_z[level][D_NEIGHBOR[level] * nearest_index];
							color = m_neighbor_z[level][DNEIGHBOR * nearest_index + DNEIGHBOR - 1 - m];
							break;
						}
						}
						//ANNcoord color = p_neighbor[D_NEIGHBOR[level] - 1 - m];	//index2 ~ m, index ~ D_NEIGHBOR[level] - 1 - m; the position is symmetrical!

						//discrete solver
						if (DISCRETE_ON) {
							/*if (COLORHIS_ON)*/	colorset[ori*DNEIGHBOR + m] = color;
							if (POSITIONHIS_ON)		positionset[ori*DNEIGHBOR + m] = ori*(TEXSIZE[level] * TEXSIZE[level])
								+ convertIndexANN(level, nearest_index) + absoluteneigh[level][DNEIGHBOR - 1 - m];
						}

						//mean-shift to reduce bluring



						// modify weight according to Color histogram matching
						if (COLORHIS_ON) {
							bin = color * perHisBin;
							//only decrease weight, increase weight could cause overshooting
							histogram_matching = max(0.0f, m_histogram_synthesis[level][bin] - m_histogram_exemplar[level][bin]);
							//changed to gaussian distribution, std = accepted error				
							Hisgaussianprob = gaussian_pdf(histogram_matching, 0.0f, HisStdDev[level]);
							weight *= Hisgaussianprob * devidedbyHisGaussianProbZero;		//(0,1]
							//test linear weight, difference not obvious
							//weight /= 1 + alpha_[level] * histogram_matching;	
						}
						// modify weight according to Position Histogram matching
						else if (POSITIONHIS_ON) {
							histogram_matching = max(0.0f, 
								m_positionhistogram_synthesis[level][positionset[ori*DNEIGHBOR + m]] - m_positionhistogram_exemplar[level][positionset[ori*DNEIGHBOR + m]]);		// [0, 1]
							////test linear weight, difference not obvious
							//weight /= 1 + alpha_[level] * histogram_matching;	
							Hisgaussianprob = gaussian_pdf(histogram_matching, 0.0, HisStdDev[level]);
							weight *= Hisgaussianprob * devidedbyHisGaussianProbZero;
						}

						//min max weight
						//if (weight < 0.02f) weight = 0.02f;
						//else if (weight > 10.0f) weight = 10.0f;

						// accumulate color
						color_acc += weight * color;
						weight_acc += weight;
					}//3 orientations
					++m;
				}
			}//for every voxel's neighbourhood, in 3 orientations

			// old & new colors for this voxel
			ANNcoord color_old = m_volume[level][i];
			//least square solver
			ANNcoord color_new = 1.0f * color_acc / weight_acc;
		
			if (COLORHIS_ON ) { 
				//discrete solver
				if (DISCRETE_ON) { 	
					color_new = FindClosestColor(level, colorset, color_new);	//update with the existed most similar color	
				}
				//color histogram update
				updateHistogram_synthesis(level, color_old, color_new); 
			}	
			else if (POSITIONHIS_ON && DISCRETE_ON) {
				//discrete solver
				ANNidx closestindex = FindClosestIndex(level, colorset, positionset, color_new);

				updatePositionHistogram_synthesis(level, m_volume_position[level][i], positionset[closestindex]);
				color_new = colorset[closestindex];
				m_volume_position[level][i] = positionset[closestindex];			
			}

			// voxel update
			m_volume[level][i] = color_new;
		}//if (!AccYN[i])

		if (FALSE && i2 == Size - 1) {
			if (RemIIdx == -1) {
				RemIIdx++;
				for (ANNidx ij = 0; ij < Size; ++ij) {
					if (!AccYN[ij])	RemIdxLst.push_back(ij);
				}
				AccYN.clear();
			}
			if (RemIIdx < RemIdxLst.size()) {
				i = RemIdxLst[RemIIdx++];
				x = i % Sx;
				y = (i / Sx) % Sy;
				z = (i / Sxy) % Sz;
				goto SearchLoop;
			}
		}
	}//for every voxel

	//========= Dynamic Thresholding based on TI histogram =========
	if (DISCRETETHRESHOLD_ON) { DynamicThresholding(level); }

	long time_end = clock();
	cout << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
}


//============Position Histogram for optimize step====
void DoPAR::initabsoluteneigh() {
	int n;
	for (int level = 0; level < MULTIRES; level++) {
		absoluteneigh[level].resize(D_NEIGHBOR[level]);
		n = 0;
		for (int y = -N[level]; y <= N[level]; y++) {
			for (int x = -N[level]; x <= N[level]; x++) {
				absoluteneigh[level][n] = y*TEXSIZE[level] + x;
				n++;
			}
		}
	}
}
void DoPAR::initPositionHistogram_exemplar() {
	//initial uniform distribution
	for (int level = 0; level < MULTIRES; level++) {

		m_positionhistogram_exemplar[level].resize(3 * (TEXSIZE[level] * TEXSIZE[level]), 0.0f);
		for (ANNidx i = 0; i < m_positionhistogram_exemplar[level].size(); i++) {
			m_positionhistogram_exemplar[level][i] = 1.0f / m_positionhistogram_exemplar[level].size();
		}
	}
}
void DoPAR::initPositionHistogram_synthesis(int level) {
	m_positionhistogram_synthesis[level].resize(3 * (TEXSIZE[level] * TEXSIZE[level]), 0.0f);
	m_volume_position[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level], 0);

	//float delta_histogram = delta_histogram_synthesis[level];
	////initial uniform distribution
	//ANNidx maxsize = 3 * (TEXSIZE[level] * TEXSIZE[level]);
	//for (ANNidx xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
	//	m_volume_position[level][xyz] = xyz%maxsize;
	//}
	//shuffle(m_volume_position[level].begin(), m_volume_position[level].end(), mersennetwistergenerator);

	//for (ANNidx i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; i++) {
	//	m_positionhistogram_synthesis[level][m_volume_position[level][i]] += delta_histogram;
	//}
}
void DoPAR::updatePositionHistogram_synthesis(int level, const ANNidx position_old, const ANNidx position_new) {
	float delta_histogram = delta_histogram_synthesis[level];
	
	if (FIRSTRUN) {
		m_positionhistogram_synthesis[level][position_new] += delta_histogram;
		return;
	}
	
	m_positionhistogram_synthesis[level][position_old] -= delta_histogram;
	m_positionhistogram_synthesis[level][position_new] += delta_histogram;
}

//---------- Color histogram ---------------
void DoPAR::calcTempHistogram(vector<ANNcoord>& model, vector<short>& existedbin, vector<float>& existedbinHis) {
	vector<float> tempdiscreteHis(DISCRETE_HISTOGRAM_BIN, 0.0f);
	float delta_histogram = 1.0f / model.size();

	for (int i = 0; i < model.size(); ++i) {
		ANNcoord c = model[i];
		tempdiscreteHis[(int)c] += delta_histogram;
	}
	// record existed colorset (discrete)
	existedbin.clear();
	existedbinHis.clear();
	for (int c = 0; c < DISCRETE_HISTOGRAM_BIN; c++) {
		if (tempdiscreteHis[c] > 0.0) {
			existedbin.push_back(c);
			existedbinHis.push_back(tempdiscreteHis[c]);
		}
	}
}
void DoPAR::initHistogram_exemplar() {
	m_histogram_exemplar.resize(MULTIRES);
	existed_histogram_examplar.resize(MULTIRES);
	existed_bin_exemplar.resize(MULTIRES);
	discrete_histogram_exemplar.resize(MULTIRES);
	discrete_acchis_exemplar.resize(MULTIRES);

	for (int level = 0; level < MULTIRES; level++) {
		m_histogram_exemplar[level].resize(NUM_HISTOGRAM_BIN, 0);

		if (DISCRETETHRESHOLD_ON || (DISTANCEMAP_ON && level == MULTIRES - 1)) {
			if (m_exemplar_x[level].size() != m_exemplar_y[level].size() || m_exemplar_x[level].size() != m_exemplar_z[level].size()) {
				cout << endl << "m_exemplar size not match"; _getch(); exit(1);
			}
			else {
				auto maxi1 = max_element(m_exemplar_x[level].begin(), m_exemplar_x[level].end());
				auto maxi2 = max_element(m_exemplar_y[level].begin(), m_exemplar_y[level].end());
				auto maxi3 = max_element(m_exemplar_z[level].begin(), m_exemplar_z[level].end());
				short maxvalue = max(m_exemplar_x[level][maxi1 - m_exemplar_x[level].begin()], max(m_exemplar_y[level][maxi2 - m_exemplar_y[level].begin()], m_exemplar_z[level][maxi3 - m_exemplar_z[level].begin()]));

				discrete_histogram_exemplar[level].resize(maxvalue + 1, 0.0f);

				if (DISCRETETHRESHOLD_ON) {
					discrete_acchis_exemplar[level].resize(discrete_histogram_exemplar[level].size(), 0.0);

					existed_bin_exemplar[level].reserve(discrete_histogram_exemplar[level].size());

					existed_histogram_examplar[level].reserve(discrete_histogram_exemplar[level].size());
				}
			}
		}

		float discrete_delta_histogram = delta_histogram_exemplar[level];
		vector<ANNcoord>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
		for (int ori = 0; ori < 3; ++ori) {
			for (int i = 0; i < TEXSIZE[level] * TEXSIZE[level]; ++i) {
				ANNcoord c = (*p[ori])[i];
				if (COLORHIS_ON) {
					int bin = (int)(c * perHisBin);
					m_histogram_exemplar[level][bin] += discrete_delta_histogram;
				}

				if (DISCRETETHRESHOLD_ON || (DISTANCEMAP_ON && level == MULTIRES - 1)) {
					discrete_histogram_exemplar[level][(int)c] += discrete_delta_histogram;
				}
			}
		}

		if (DISCRETETHRESHOLD_ON) {
			// calc accumulate histogram (discrete)
			calcaccHistogram(discrete_histogram_exemplar[level], discrete_acchis_exemplar[level]);

			// record existed colorset (discrete)
			for (int c = 0; c < discrete_histogram_exemplar[level].size(); c++) {
				if (discrete_histogram_exemplar[level][c] > 0.0) {
					existed_bin_exemplar[level].push_back(c);
					existed_histogram_examplar[level].push_back(discrete_histogram_exemplar[level][c]);
				}
			}
		}
	}

	cout << endl << "initHistogram_exemplar done.";
	//for (int i = 0; i < existed_bin_exemplar[MULTIRES-1].size(); i++){
	//	cout << endl << existed_bin_exemplar[MULTIRES-1][i] << "     " << existed_histogram_examplar[MULTIRES-1][i];
	//}_getch();
}
void DoPAR::initHistogram_synthesis(int level) {
	//m_histogram_synthesis[level].clear();
	m_histogram_synthesis[level].resize(NUM_HISTOGRAM_BIN, 0.0f);
	//discrete_histogram_synthesis[level].clear();
	if (DISCRETETHRESHOLD_ON) discrete_histogram_synthesis[level].resize(DISCRETE_HISTOGRAM_BIN, 0.0);

	float delta_histogram = delta_histogram_synthesis[level];
	for (ANNidx i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i) {
		ANNcoord c = m_volume[level][i];
	
		m_histogram_synthesis[level][(int)(c * perHisBin)] += delta_histogram;
		if (DISCRETETHRESHOLD_ON) discrete_histogram_synthesis[level][(int)c] += delta_histogram;
	}
}
void DoPAR::updateHistogram_synthesis(int level, const ANNcoord color_old, const ANNcoord color_new) {
	float delta_histogram = delta_histogram_synthesis[level];
	m_histogram_synthesis[level][color_old * perHisBin] -= delta_histogram;
	m_histogram_synthesis[level][color_new * perHisBin] += delta_histogram;

	//if (DISCRETETHRESHOLD_ON) {
	//	discrete_histogram_synthesis[level][color_old] -= delta_histogram;
	//	discrete_histogram_synthesis[level][color_new] += delta_histogram;
	//}
}

//----------- Discrete solver  -----------
ANNcoord DoPAR::FindClosestColor(int level, vector<ANNcoord> &color, ANNcoord referencecolor) {
	//find nearest color value, then compare weight for all closest values
	//return the final index

	//if (DISTANCEMAP_ON) {
	//	if (referencecolor < Solid_Upper) referencecolor = floor(referencecolor);
	//	if (referencecolor > Pore_Lower) referencecolor = ceil(referencecolor);
	//}

	//auto i= min_element(begin(color), end(color), [=](ANNcoord x, ANNcoord y)
	//{
	//	return abs(x - referencecolor) < abs(y - referencecolor);
	//});
	//return distance(begin(color), i);
	return *min_element(begin(color), end(color), [=](ANNcoord x, ANNcoord y)
	{
		return abs(x - referencecolor) < abs(y - referencecolor);
	});
}

ANNidx DoPAR::FindClosestIndex(int level, vector<ANNcoord> &color, vector<ANNidx> &position, ANNcoord referencecolor) {
	ANNidx minidx = 0;
	float minhis = m_positionhistogram_synthesis[level][position[0]], temphis;
	ANNcoord mincolordif = abs(color[0] - referencecolor), tempcolordif;
	
	for (short i = 1; i < color.size(); i++) {
		tempcolordif = abs(color[i] - referencecolor);
		temphis = m_positionhistogram_synthesis[level][position[i]];
		if (tempcolordif < mincolordif) {
			minidx = i;
			mincolordif = tempcolordif;
			minhis = temphis;
		}
		else if (tempcolordif == mincolordif && temphis < minhis) {
			minidx = i;
			minhis = temphis;
		}
	}

	return minidx;
}







//----------- Dynamic thresholding, worse than proportionthreshold ---------
void DoPAR::calcaccHistogram(vector<float> &inputhis, vector<float> &acchis) {
	if (inputhis.size() != acchis.size()) { cout << endl << "accHistogram size " << acchis.size() << " not match " << inputhis.size(); _getch(); }
	acchis = inputhis;
	for (int i = 1; i < inputhis.size(); i++) {
		acchis[i] += acchis[i - 1];
	}
	if (acchis[acchis.size() - 1] > 1.001 || acchis[acchis.size() - 1] < 0.999) { cout << endl << "Error: acchis max=" << acchis[acchis.size() - 1]; _getch(); }
}
void DoPAR::DynamicThresholding(int level) {
	//if (PROPORTIONTHRESHOLD_ON) { cout << endl << "PROPORTIONTHRESHOLD_ON"; _getch(); }
	//if (DISCRETE_HISTOGRAM_BIN != CHANNEL_MAXVALUE[0]) { cout << endl << "DISCRETE_HISTOGRAM_BIN=" << DISCRETE_HISTOGRAM_BIN << " only accept " << CHANNEL_MAXVALUE[0]; _getch(); }
	ANNidx Size = TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
	//first calculate accumulate histogram
	vector<float> acchis_synthesis(DISCRETE_HISTOGRAM_BIN, 0.0);		//ch=0
	calcaccHistogram(discrete_histogram_synthesis[level], acchis_synthesis);

	//calc threshold value set
	vector<float> thresholdvalue(existed_bin_exemplar[level].size() - 1, 0.0);
	//----------- Linear solver, not very accurate ----------------------
	auto vectoriterator = upper_bound(acchis_synthesis.begin(), acchis_synthesis.end(), 0.0);
	for (int i = 0; i < existed_bin_exemplar[level].size() - 1; i++) {		//ignore the last threshold(should be max)
		float acc = discrete_acchis_exemplar[level][existed_bin_exemplar[level][i]];
		vectoriterator = upper_bound(acchis_synthesis.begin(), acchis_synthesis.end(), acc);
		int upper = vectoriterator - acchis_synthesis.begin();
		int lower = max(0, upper - 1);			//!should be lower_bound, but because the weighted average set is continuious, it is equal to upper-1
		if (lower == 0) {/* cout << endl << endl << "up_acc - low_acc=0" << endl;  */thresholdvalue[i] = 0.0; continue; }
		float up_acc = acchis_synthesis[upper];
		float low_acc = acchis_synthesis[lower];
		thresholdvalue[i] = 1.0*lower + (acc - low_acc) / (up_acc - low_acc);	//assume linear distribution within acchis_synthesis[lower,upper]
	}

	////============ Non-linear solver to calculate thresholdvalue ======================
	////PolynomialInterpolation(vector<double>& Xv, vector<double>& Yv, vector<double>& X)
	////Xv: trimed acchis_synthesis	Yv: 0,1,,...,DISCRETE_HISTOGRAM_BIN-1	X: discrete_acchis_exemplar[level][existed_bin_exemplar[level][i]] stored in thresholdvalue[]
	////!!Note: Xv must not have two same values
	//vector<double> Xv; Xv.reserve(DISCRETE_HISTOGRAM_BIN);
	//vector<double> Yv; Yv.reserve(DISCRETE_HISTOGRAM_BIN);
	//for (int i = 0; i < DISCRETE_HISTOGRAM_BIN; i++) {
	//	if (discrete_histogram_synthesis[level][i] >0) {
	//		Xv.push_back(acchis_synthesis[i]);
	//		Yv.push_back(i);
	//	}
	//}
	//for (int t = 0; t < thresholdvalue.size(); t++){ thresholdvalue[t] = discrete_acchis_exemplar[level][existed_bin_exemplar[level][t]]; }
	//
	//PolynomialInterpolation(Xv, Yv, thresholdvalue);	//output is thresholdvalue
	//////////////////////////////wrong results////////////////////////////////////////////
	//cout << endl;
	//for (int i = 0; i < thresholdvalue.size(); i++){ cout << endl << thresholdvalue[i]; }_getch();

	//for (int i = 0; i < existed_bin_exemplar[level].size(); i++){
	//	cout << endl << "existed_bin_exemplar[" << i << "]= " << existed_bin_exemplar[level][i] << " thresholdvalue= " << thresholdvalue[i];
	//}

	thresholdvalue.push_back(DISCRETE_HISTOGRAM_BIN - 1);
	//thresholding based on thresholdvalue set;
	for (ANNidx m = 0; m < Size; m++) {
		auto i = lower_bound(thresholdvalue.begin(), thresholdvalue.end(), m_volume[level][m]);
		int bin = i - thresholdvalue.begin();
		m_volume[level][m] = existed_bin_exemplar[level][bin];
		if (m_volume[level][m] > DISCRETE_HISTOGRAM_BIN - 1) m_volume[level][m] = DISCRETE_HISTOGRAM_BIN - 1;
	}

	//update color histogram after changing value
	initHistogram_synthesis(level);
}
////Non-linear solver
//void DoPAR::PolynomialInterpolation(vector<double>& Xv, vector<double>& Yv, vector<double>& X){
//	//(1) Xv, Yv are a set of points
//	//   Note that no two x-values are the same, otherwise this programm could collapse
//	//(2) A set of x values as input, will get y values as output
//
//	if (Xv.size() != Yv.size() || Xv.size() == 0 || X.size() == 0) { cout << endl << "PolynomialInterpolation:size error"; return; }
//
//	vector<double> XX, YRes;
//
//	XX.resize(Xv.size());
//
//	for (long ij = 0; ij < Xv.size(); ++ij) {
//		XX[ij] = 1.0;
//		for (long kl = 0; kl < Xv.size(); ++kl) {
//			if (kl == ij) continue;
//			if (Xv[ij] == Xv[kl]) { cout << endl << "!!!Error: Xv[ij]=Xv[kl]!!!"; _getch(); return; }
//			XX[ij] *= (Xv[ij] - Xv[kl]);
//		}
//	}
//
//	YRes.resize(X.size());
//
//	for (long idx = 0; idx < X.size(); ++idx) {
//		YRes[idx] = 0.0;
//		for (long ij = 0; ij < Xv.size(); ++ij) {
//			double Val = 1.0;
//			for (long kl = 0; kl < Xv.size(); ++kl) {
//				if (kl == ij) continue;
//				Val *= (X[idx] - Xv[kl]);
//			}
//			YRes[idx] += Val*Yv[ij] / XX[ij];
//		}
//	}
//
//	YRes.swap(X);
//}
//
//----------- Proportion Threshold, better ---------
void DoPAR::ProportionThreshold(vector<short>& Model, vector<short> BinNum, vector<float> Prob) {
	//(1) Model to be thresholded according to a distribution <BinNum, Prob>
	//(2) BinNum and Prob corresponds to histogram (), BinNum must be ordered. The last BinNum is biggest.

	if (BinNum.size() == 0 || Prob.size() != BinNum.size()) { cout << endl << "BinNum.size=" << BinNum.size() << " Prob.size=" << Prob.size(); _getch(); return; }
	if (Model.size() == 0) { cout << endl << "shortmodel.size=" << Model.size(); _getch(); return; }
	double check_sum_Prob(0.0);
	for_each(Prob.rbegin(), Prob.rend(), [&](double n) { check_sum_Prob += n; });
	if (check_sum_Prob > 1.001 || check_sum_Prob < 0.999) { cout << endl << "check_sum_Prob=" << check_sum_Prob; _getch(); return; }

	short MinVal = Model[0];
	for (long idx = 0; idx < Model.size(); ++idx) {
		if (Model[idx] < MinVal)
			MinVal = Model[idx];
	}

	vector<short> ResModel(Model.size(), BinNum[0]);

	//vector<long> shuffledidx(Model.size(), 0);
	//for (long idx = 0; idx < shuffledidx.size(); ++idx) shuffledidx[idx] = idx;

	for (long ij = BinNum.size() - 1; ij > 0; --ij) {	//start from the biggest.
		vector<char> Tmp;
		Tmp = BinariseImg(Model, Prob[ij]);

		//shuffle(shuffledidx.begin(), shuffledidx.end(), mersennetwistergenerator);

		for (long idx = 0; idx < Model.size(); ++idx) {
			if (Tmp[idx] == 1) {
				ResModel[idx] = BinNum[ij];
				Model[idx] = MinVal;
			}
		}
	}
	Model.swap(ResModel);
}


// ========= Distance Map ===========
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
vector<char> DoPAR::BinariseImg(vector<short>& DMap, double TPorosity) {
	//(1) Distance map: Pores have the most largest values
	//(2) TPorosity: Targeting porosity to determine the actual threshold value to binarise the image

	short MaxVal(0), MinVal(0);

	for (long idx = 0; idx < DMap.size(); ++idx) {
		if (DMap[idx] < MinVal) MinVal = DMap[idx];
		if (DMap[idx] > MaxVal) MaxVal = DMap[idx];
	}

	vector<long> CntValue(MaxVal - MinVal + 2, 0);

	for (long idx = 0; idx < DMap.size(); ++idx) {
		CntValue[DMap[idx] - MinVal]++;
	}

	long TotNum = TPorosity * DMap.size();
	long RemNum(0), SelIj(0), RemainingNum(0);

	for (long ij = CntValue.size() - 1; ij >= 0; --ij) {
		RemNum += CntValue[ij];
		if (RemNum > TotNum) {
			SelIj = ij;
			RemainingNum = TotNum - RemNum + CntValue[ij];
			break;
		}
	}

	//vector<char> Res(DMap.size(), 0);
	//
	//long RemNowC(0);
	//short TVal;
	//
	//vector<long> shuffledidx(DMap.size(), 0);
	//for (long idx = 0; idx < shuffledidx.size(); ++idx) shuffledidx[idx] = idx;
	//shuffle(shuffledidx.begin(), shuffledidx.end(), mersennetwistergenerator);
	//
	//for (long idx = 0; idx < DMap.size(); ++idx) {
	//	TVal = DMap[idx] - MinVal;
	//	//TVal = DMap[shuffledidx[idx]] - MinVal;			//change to random order!
	//	if (TVal < SelIj) continue;
	//	if (TVal > SelIj) {
	//		Res[idx] = 1;
	//		//Res[shuffledidx[idx]] = 1;
	//		continue;
	//	}
	//
	//	if (RemNowC > RemainingNum) continue;
	//	Res[idx] = 1; RemNowC++;
	//}

	vector<char> Res(DMap.size(), 0);

	vector<long> RemIdxLst;

	long RemNowC(0);
	short TVal;
	for (long idx = 0; idx < DMap.size(); ++idx) {
		TVal = DMap[idx] - MinVal;
		if (TVal < SelIj) continue;
		if (TVal > SelIj) {
			Res[idx] = 1;
			continue;
		}
		RemIdxLst.push_back(idx);
		//if (RemNowC > RemainingNum) continue;
		//Res[idx] = 1; RemNowC++;
	}

	//srand((unsigned)time(NULL));
	if (RemainingNum > 0) {
		//long SizeRI = RemIdxLst.size() - 1L;
		//for (long ij = 0; ij < RemIdxLst.size(); ++ij) {
		//	long Tij = SizeRI*probabilitydistribution(mersennetwistergenerator);		//random!
		//	long tmp = RemIdxLst[ij];
		//	RemIdxLst[ij] = RemIdxLst[Tij];
		//	RemIdxLst[Tij] = tmp;
		//}
		shuffle(RemIdxLst.begin(), RemIdxLst.end(), mersennetwistergenerator);

		for (long ij = 0; ij < RemainingNum; ++ij) {
			Res[RemIdxLst[ij]] = 1;
		}
	}

	return Res;
}

void DoPAR::BinariseThreshold(vector<short>& DMap, vector<char>& Binarised, short threshold) {
	//input  vector<short> DMap
	//output vector<char>Binarised
	if (Binarised.size() != DMap.size()) { cout << endl << "BinariseThreshold() size not matched"; _getch(); return; }
	for (long i = 0; i < DMap.size(); i++) {
		if (DMap[i] <= threshold) Binarised[i] = 0;
		else Binarised[i] = 1;
	}
}
void DoPAR::PrepareDMapProjection(vector<short>& TI1, vector<short>& TI2, vector<short>& TI3, int level) {
	//dynamically decide solid_upper, pore_lower && DistanceThreshold
	//compute RedistributeRatio for 3TI
	ProjectDMapMaxBins.resize(MULTIRES);

	//testDM
	//int pore1count(0), solid1count(0);

	short minVal, maxVal, minVal1, minVal2, minVal3, maxVal1, maxVal2, maxVal3;	//total min, max for 3TIs; and separately
	minVal = maxVal = TI1[0];	minVal1 = maxVal1 = TI1[0]; minVal2 = maxVal2 = TI2[0]; minVal3 = maxVal3 = TI3[0];
	for (long idx = 0; idx < TI1.size(); ++idx) {
		if (TI1[idx] < minVal1) minVal1 = TI1[idx];
		if (TI1[idx] > maxVal1) maxVal1 = TI1[idx];
		if (TI1[idx] == 0) { cout << endl << "DM1[" << idx << "]= 0!!"; _getch(); }
	}cout << endl << "TI1: porosity= " << PorosityX << " solid_min= " << minVal1 << " pore_max= " << maxVal1;
	for (long idx = 0; idx < TI2.size(); ++idx) {
		if (TI2[idx] < minVal2) minVal2 = TI2[idx];
		if (TI2[idx] > maxVal2) maxVal2 = TI2[idx];
		if (TI2[idx] == 0) { cout << endl << "DM2[" << idx << "]= 0!!"; _getch(); }
	}cout << endl << "TI2: porosity= " << PorosityY << " solid_min= " << minVal2 << " pore_max= " << maxVal2;
	for (long idx = 0; idx < TI3.size(); ++idx) {
		if (TI3[idx] < minVal3) minVal3 = TI3[idx];
		if (TI3[idx] > maxVal3) maxVal3 = TI3[idx];
		if (TI3[idx] == 0) { cout << endl << "DM3[" << idx << "]= 0!!"; _getch(); }
	}cout << endl << "TI3: porosity= " << PorosityZ << " solid_min= " << minVal3 << " pore_max= " << maxVal3;

	//minVal = min(minVal1, min(minVal2, minVal3));
	//maxVal = max(maxVal1, max(maxVal2, maxVal3));		//choose max and min, would have very large range with lowest frequency. in return some bins have extremly high values.

	//minVal = max(minVal1, max(minVal2, minVal3));
	//maxVal = min(maxVal1, min(maxVal2, maxVal3));		//choose min(max) and max(min) would cause 0 and 255 to have too high values sometimes.

	minVal = 1.0 / 3.0* (minVal1 + minVal2 + minVal3);
	maxVal = 1.0 / 3.0* (maxVal1 + maxVal2 + maxVal3);	//choose average would reduce the above influence.

	ProjectDMapMaxBins[level] = -minVal + maxVal;

	if (level == MULTIRES - 1) {
		float bingap = 1.0*DISCRETE_HISTOGRAM_BIN / ProjectDMapMaxBins[level];
		short peakgap = 2 * ceil(bingap);

		Solid_Upper = (-1 - minVal) * bingap;
		Pore_Lower = Solid_Upper + peakgap;
		DistanceThreshold = 0.5*(Solid_Upper + Pore_Lower);
		cout << endl << "Solid_Upper= " << Solid_Upper << "  Pore_Lower= " << Pore_Lower;
	}
}
void DoPAR::ProjectDMap(vector<short>& DMap, int level) {
	//linear projection for DMap
	//solid: [minVal,-1] to [solid_upper, 1]  ;  pore: [1,maxVal] to [DISCRETE_HISTOGRAM_BIN, pore_lower] Note the order has been reversed!
	//if requires to have same ratio for solid and pore, use the bigger compress ratio

	float bingap = 1.0* DISCRETE_HISTOGRAM_BIN / ProjectDMapMaxBins[level];

	for (long idx = 0; idx < DMap.size(); ++idx) {
		if (DMap[idx] > 0) {
			DMap[idx] = Pore_Lower + ((DMap[idx] - 1.0)*bingap);
			if (DMap[idx] > DISCRETE_HISTOGRAM_BIN - 1) DMap[idx] = DISCRETE_HISTOGRAM_BIN - 1;
		}
		else if (DMap[idx] < 0) {
			DMap[idx] = Solid_Upper - ((-DMap[idx] - 1.0) * bingap);
			if (DMap[idx] < 0) DMap[idx] = 0;
		}
	}
}

void DoPAR::NooutputDM(vector<short>& TI1, vector<short>& TI2, vector<short>& TI3)
{	//transfer DM to 0~-min+max. input TI1,TI2,TI3, output TI1,TI2,TI3
	//do not resize to 256, just use it for Bimodal transform

	short minVal, maxVal, minVal1, minVal2, minVal3, maxVal1, maxVal2, maxVal3;	//total min, max for 3TIs; and separately
	minVal = maxVal = TI1[0];	minVal1 = maxVal1 = TI1[0]; minVal2 = maxVal2 = TI2[0]; minVal3 = maxVal3 = TI3[0];
	for (long idx = 0; idx < TI1.size(); ++idx) {
		if (TI1[idx] < minVal1) minVal1 = TI1[idx];
		if (TI1[idx] > maxVal1) maxVal1 = TI1[idx];
		if (TI1[idx] == 0) { cout << endl << "DM1[" << idx << "]= 0!!"; _getch(); }
	}cout << endl << "TI1: porosity= " << PorosityX << " solid_min= " << minVal1 << " pore_max= " << maxVal1;
	for (long idx = 0; idx < TI2.size(); ++idx) {
		if (TI2[idx] < minVal2) minVal2 = TI2[idx];
		if (TI2[idx] > maxVal2) maxVal2 = TI2[idx];
		if (TI2[idx] == 0) { cout << endl << "DM2[" << idx << "]= 0!!"; _getch(); }
	}cout << endl << "TI2: porosity= " << PorosityY << " solid_min= " << minVal2 << " pore_max= " << maxVal2;
	for (long idx = 0; idx < TI3.size(); ++idx) {
		if (TI3[idx] < minVal3) minVal3 = TI3[idx];
		if (TI3[idx] > maxVal3) maxVal3 = TI3[idx];
		if (TI3[idx] == 0) { cout << endl << "DM3[" << idx << "]= 0!!"; _getch(); }
	}cout << endl << "TI3: porosity= " << PorosityZ << " solid_min= " << minVal3 << " pore_max= " << maxVal3;

	minVal = min(minVal1, min(minVal2, minVal3));
	maxVal = max(maxVal1, max(maxVal2, maxVal3));

	Solid_Upper = (-1 - minVal);
	Pore_Lower = (1 - minVal);
	DistanceThreshold = 0.5*(Solid_Upper + Pore_Lower);

	for (int i = 0; i < TI1.size(); i++) TI1[i] -= minVal;
	for (int j = 0; j < TI2.size(); j++) TI2[j] -= minVal;
	for (int k = 0; k < TI3.size(); k++) TI3[k] -= minVal;
}

void DoPAR::BimodalRedistribution(vector<float>& Res, string filename) {
	//Redistribute DM, so that it has double peak distribution, each side has similar proportion
	//Requires discrete_histogram_exemplar and accumulation function
	//So it should be called after initHistogram_exemplar()

	//!!!Level = MULTIRES-1!!!!

	float s1 = discrete_histogram_exemplar[MULTIRES - 1][Solid_Upper];
	float p1 = discrete_histogram_exemplar[MULTIRES - 1][Pore_Lower];
	float binprop = min(s1, p1);

	// calc accumulate histogram (discrete)
	vector<float> acchis(discrete_histogram_exemplar[MULTIRES - 1].size(), 0.0f);
	calcaccHistogram(discrete_histogram_exemplar[MULTIRES - 1], acchis);

	cout << endl << "Solid_Upper=" << Solid_Upper << " prop=" << s1 << " acc=" << acchis[Solid_Upper]
		<< endl << " Pore_Lower=" << Pore_Lower << " prop=" << p1 << " acc=" << acchis[Pore_Lower];

	vector<short> thresholdbin;
	thresholdbin.reserve(100);
	short halfbin(0);

	//first half
	float curacc(0.0f);
	bool firsthalfdone = false;
	for (int bin = 0; bin < 2 * ceil(1.0f / binprop); bin++) {
		curacc += binprop;
		auto i = lower_bound(acchis.begin(), acchis.end(), curacc);
		int b1 = i - acchis.begin(), b0 = b1 - 1;
		float value1 = acchis[b1], value0 = acchis[b0];

		if (b1 >= (short)Solid_Upper) {
			if (discrete_histogram_exemplar[MULTIRES - 1][Solid_Upper - 1]>0) {
				thresholdbin.push_back(Solid_Upper - 1);
				curacc = acchis[thresholdbin[bin]];
			}
			firsthalfdone = true;
			break;
		}
		else if (abs(value1 - curacc) > abs(value0 - curacc)) {
			if (bin - 1 < 0) { thresholdbin.push_back(b0); curacc = acchis[thresholdbin[bin]]; }
			else if (thresholdbin[bin - 1] != b0) { thresholdbin.push_back(b0);	curacc = acchis[thresholdbin[bin]]; }
			else bin--;
		}
		else { thresholdbin.push_back(b1); curacc = acchis[thresholdbin[bin]]; }

		//cout << " b1=" << b1 << " " << thresholdbin[bin];
		if (firsthalfdone || thresholdbin[bin] == (short)Solid_Upper - 1) break;
	}
	//push back twice Solid_Upper, in order to have a wieder gap between solid&pore
	thresholdbin.push_back(Solid_Upper); thresholdbin.push_back(Solid_Upper);
	thresholdbin.push_back(Pore_Lower);
	//second half
	bool secondhalf = false;
	curacc = acchis[Pore_Lower];
	for (int bin = thresholdbin.size(); bin < 2 * ceil(1.0f / binprop); bin++) {
		curacc += binprop;
		auto i = lower_bound(acchis.begin(), acchis.end(), curacc);
		int b1 = i - acchis.begin(), b0 = b1 - 1;
		float value1 = acchis[b1], value0 = acchis[b0];

		if (value1 >= 0.99f || value0 >= 0.99f) {
			thresholdbin.push_back(acchis.size() - 1);
			curacc = acchis[thresholdbin[bin]];
			secondhalf = true;
		}
		else if (abs(value1 - curacc) > abs(value0 - curacc)) {
			if (bin - 1 < 0) { thresholdbin.push_back(b0); curacc = acchis[thresholdbin[bin]]; }
			else if (thresholdbin[bin - 1] != b0) { thresholdbin.push_back(b0);	curacc = acchis[thresholdbin[bin]]; }
			else bin--;
		}
		else { thresholdbin.push_back(b1); curacc = acchis[thresholdbin[bin]]; }

		if (secondhalf) break;
	}
	//find half
	for (int bin = 0; bin < thresholdbin.size(); bin++)
		if (acchis[thresholdbin[bin]] >= 0.5f) { halfbin = bin; break; }

	// thresholdbin is done. now decide bingap & peakgap
	short peakgap;
	float bingap;
	//bool DMnogap = false;
	//if (!DMnogap) {
	//	bingap = 6;
	//	peakgap = 255 - bingap*(thresholdbin.size() - 2);
	//}
	//else {
	//	bingap = 255 / (thresholdbin.size() - 1);
	//	peakgap = bingap;
	//}

	bingap = 8.0;
	peakgap = 255.0 - bingap*(thresholdbin.size() - 2);
	while (peakgap < bingap * 15) {
		bingap -= 2;
		peakgap = 255.0 - bingap*(thresholdbin.size() - 2);
	}

	short Redis_Solid_Upper(0), Redis_Pore_Lower(0);

	//========= redistribute DM	=============
	for (ANNidx m = 0; m < Res.size(); m++) {
		auto i = lower_bound(thresholdbin.begin(), thresholdbin.end(), Res[m]);
		int num = i - thresholdbin.begin();

		if (num <= halfbin) {
			//if (thresholdbin[num] == Solid_Upper) Redis_Solid_Upper = num*bingap;
			//else if (thresholdbin[num] == Pore_Lower) Redis_Pore_Lower = num*bingap;
			Res[m] = num*bingap;
		}
		else {
			//if (thresholdbin[num] == Solid_Upper) Redis_Solid_Upper = (num - 1)*bingap + peakgap;
			//else if (thresholdbin[num] == Pore_Lower) Redis_Pore_Lower = (num - 1)*bingap + peakgap;
			Res[m] = (num - 1)*bingap + peakgap;
		}
	}
	auto s = lower_bound(thresholdbin.begin(), thresholdbin.end(), (short)Solid_Upper);
	short rs = s - thresholdbin.begin();
	if (rs <= halfbin) Redis_Solid_Upper = round(rs*bingap);
	else Redis_Solid_Upper = round((rs - 1)*bingap + peakgap);

	auto p = lower_bound(thresholdbin.begin(), thresholdbin.end(), (short)Pore_Lower);
	short rp = p - thresholdbin.begin();
	if (rp <= halfbin) Redis_Pore_Lower = round(rp*bingap);
	else Redis_Pore_Lower = round((rp - 1)*bingap + peakgap);

	cout << endl << "bingap = " << bingap << " peakgap = " << peakgap;
	cout << endl << "Redis_Solid_Upper= " << Redis_Solid_Upper << "  Redis_Pore_Lower= " << Redis_Pore_Lower << "  Halfbin= " << thresholdbin[halfbin];
	cout << endl << "\nThresholded bins: ";
	for (int i = 0; i < thresholdbin.size(); i++) {
		cout << " " << thresholdbin[i];
	}
	cout << endl << "Redistributed bins: ";
	for (int i = 0; i < thresholdbin.size(); i++) {
		if (i <= halfbin) 	cout << " " << i*bingap;
		else cout << " " << (i - 1)*bingap + peakgap;
	}

	// Write Bimodal image
	Mat BimodalMat = Mat(TEXSIZE[MULTIRES - 1], TEXSIZE[MULTIRES - 1], CV_8UC1);
	BimodalMat = Mat(Res, true).reshape(1, BimodalMat.rows);

	string tempoutputfilename = filename;
	tempoutputfilename = filename.substr(0, filename.find('.')) + "_S" + to_string(Redis_Solid_Upper) + "P" + to_string(Redis_Pore_Lower) + ".png";
	imwrite(tempoutputfilename, BimodalMat);
}
void DoPAR::BimodalRedistribution3D(vector<float>& Res, string filename) {
	//Redistribute DM, so that it has double peak distribution, each side has similar proportion
	//Requires discrete_histogram_exemplar and accumulation function
	//So it should be called after initHistogram_exemplar()

	//!!!!OUTPUT MODEL Level = 0!!!!!
	float s1 = discrete_histogram_exemplar[MULTIRES - 1][Solid_Upper];
	float p1 = discrete_histogram_exemplar[MULTIRES - 1][Pore_Lower];
	float binprop = min(s1, p1);

	// calc accumulate histogram (discrete)
	vector<float> acchis(discrete_histogram_exemplar[MULTIRES - 1].size(), 0.0f);
	calcaccHistogram(discrete_histogram_exemplar[MULTIRES - 1], acchis);

	cout << endl << "Solid_Upper=" << Solid_Upper << " prop=" << s1 << " acc=" << acchis[Solid_Upper]
		<< endl << " Pore_Lower=" << Pore_Lower << " prop=" << p1 << " acc=" << acchis[Pore_Lower];

	vector<short> thresholdbin;
	thresholdbin.reserve(100);
	short halfbin(0);

	//first half
	float curacc(0.0f);
	bool firsthalfdone = false;
	for (int bin = 0; bin < 2 * ceil(1.0f / binprop); bin++) {
		curacc += binprop;
		auto i = lower_bound(acchis.begin(), acchis.end(), curacc);
		int b1 = i - acchis.begin(), b0 = b1 - 1;
		float value1 = acchis[b1], value0 = acchis[b0];

		if (b1 >= (short)Solid_Upper) {
			if (discrete_histogram_exemplar[MULTIRES - 1][Solid_Upper - 1]>0) {
				thresholdbin.push_back(Solid_Upper - 1);
				curacc = acchis[thresholdbin[bin]];
			}
			firsthalfdone = true;
			break;
		}
		else if (abs(value1 - curacc) > abs(value0 - curacc)) {
			if (bin - 1 < 0) { thresholdbin.push_back(b0); curacc = acchis[thresholdbin[bin]]; }
			else if (thresholdbin[bin - 1] != b0) { thresholdbin.push_back(b0);	curacc = acchis[thresholdbin[bin]]; }
			else bin--;
		}
		else { thresholdbin.push_back(b1); curacc = acchis[thresholdbin[bin]]; }

		//cout << " b1=" << b1 << " " << thresholdbin[bin];
		if (firsthalfdone || thresholdbin[bin] == (short)Solid_Upper - 1) break;
	}
	//push back twice Solid_Upper, in order to have a wieder gap between solid&pore
	thresholdbin.push_back(Solid_Upper); thresholdbin.push_back(Solid_Upper);
	thresholdbin.push_back(Pore_Lower);
	//second half
	bool secondhalf = false;
	curacc = acchis[Pore_Lower];
	for (int bin = thresholdbin.size(); bin < 2 * ceil(1.0f / binprop); bin++) {
		curacc += binprop;
		auto i = lower_bound(acchis.begin(), acchis.end(), curacc);
		int b1 = i - acchis.begin(), b0 = b1 - 1;
		float value1 = acchis[b1], value0 = acchis[b0];

		if (value1 >= 0.99f || value0 >= 0.99f) {
			thresholdbin.push_back(acchis.size() - 1);
			curacc = acchis[thresholdbin[bin]];
			secondhalf = true;
		}
		else if (abs(value1 - curacc) > abs(value0 - curacc)) {
			if (bin - 1 < 0) { thresholdbin.push_back(b0); curacc = acchis[thresholdbin[bin]]; }
			else if (thresholdbin[bin - 1] != b0) { thresholdbin.push_back(b0);	curacc = acchis[thresholdbin[bin]]; }
			else bin--;
		}
		else { thresholdbin.push_back(b1); curacc = acchis[thresholdbin[bin]]; }

		if (secondhalf) break;
	}
	//find half
	for (int bin = 0; bin < thresholdbin.size(); bin++)
		if (acchis[thresholdbin[bin]] >= 0.5f) { halfbin = bin; break; }

	// thresholdbin is done. now decide bingap & peakgap
	short peakgap;
	float bingap;
	//bool DMnogap = false;
	//if (!DMnogap) {
	//	bingap = 6;
	//	peakgap = 255 - bingap*(thresholdbin.size() - 2);
	//}
	//else {
	//	bingap = 255 / (thresholdbin.size() - 1);
	//	peakgap = bingap;
	//}

	bingap = 8.0;
	peakgap = 255.0 - bingap*(thresholdbin.size() - 2);
	while (peakgap < bingap * 15) {
		bingap -= 2;
		peakgap = 255.0 - bingap*(thresholdbin.size() - 2);
	}

	short Redis_Solid_Upper(0), Redis_Pore_Lower(0);

	//========= redistribute DM	=============
	for (ANNidx m = 0; m < Res.size(); m++) {
		auto i = lower_bound(thresholdbin.begin(), thresholdbin.end(), Res[m]);
		int num = i - thresholdbin.begin();

		if (num <= halfbin) {
			//if (thresholdbin[num] == Solid_Upper) Redis_Solid_Upper = num*bingap;
			//else if (thresholdbin[num] == Pore_Lower) Redis_Pore_Lower = num*bingap;
			Res[m] = num*bingap;
		}
		else {
			//if (thresholdbin[num] == Solid_Upper) Redis_Solid_Upper = (num - 1)*bingap + peakgap;
			//else if (thresholdbin[num] == Pore_Lower) Redis_Pore_Lower = (num - 1)*bingap + peakgap;
			Res[m] = (num - 1)*bingap + peakgap;
		}
	}
	auto s = lower_bound(thresholdbin.begin(), thresholdbin.end(), (short)Solid_Upper);
	short rs = s - thresholdbin.begin();
	if (rs <= halfbin) Redis_Solid_Upper = round(rs*bingap);
	else Redis_Solid_Upper = round((rs - 1)*bingap + peakgap);

	auto p = lower_bound(thresholdbin.begin(), thresholdbin.end(), (short)Pore_Lower);
	short rp = p - thresholdbin.begin();
	if (rp <= halfbin) Redis_Pore_Lower = round(rp*bingap);
	else Redis_Pore_Lower = round((rp - 1)*bingap + peakgap);

	cout << endl << "bingap = " << bingap << " peakgap = " << peakgap;
	cout << endl << "Redis_Solid_Upper= " << Redis_Solid_Upper << "  Redis_Pore_Lower= " << Redis_Pore_Lower << "  Halfbin= " << thresholdbin[halfbin];
	cout << endl << "\nThresholded bins: ";
	for (int i = 0; i < thresholdbin.size(); i++) {
		cout << " " << thresholdbin[i];
	}
	cout << endl << "Redistributed bins: ";
	for (int i = 0; i < thresholdbin.size(); i++) {
		if (i <= halfbin) 	cout << " " << i*bingap;
		else cout << " " << (i - 1)*bingap + peakgap;
	}

	// Write Bimodal 3D model
	vector<uchar> tempmodel = vector<uchar>(Res.begin(), Res.end());
	string tempoutputfilename = filename;
	tempoutputfilename = filename.substr(0, filename.find('.')) + "_S" + to_string(Redis_Solid_Upper) + "P" + to_string(Redis_Pore_Lower) + ".RAW";
	Write(outputpath + tempoutputfilename, tempmodel);
}

void DoPAR::testBimodalRedistribution(vector<float>& Res, string filename) {
	//Redistribute DM, so that it has double peak distribution, each side has similar proportion
	//Requires discrete_histogram_exemplar and accumulation function
	//So it should be called after initHistogram_exemplar()

	//!!!Level = MULTIRES-1!!!!

	float s1 = discrete_histogram_exemplar[MULTIRES - 1][Solid_Upper];
	float p1 = discrete_histogram_exemplar[MULTIRES - 1][Pore_Lower];
	float binprop = min(s1, p1);

	// calc accumulate histogram (discrete)
	vector<float> acchis(discrete_histogram_exemplar[MULTIRES - 1].size(), 0.0f);
	calcaccHistogram(discrete_histogram_exemplar[MULTIRES - 1], acchis);

	cout << endl << "Solid_Upper=" << Solid_Upper << " prop=" << s1 << " acc=" << acchis[Solid_Upper]
		<< endl << " Pore_Lower=" << Pore_Lower << " prop=" << p1 << " acc=" << acchis[Pore_Lower];

	vector<short> thresholdbin;
	thresholdbin.reserve(100);
	short halfbin(0);

	//first half
	float curacc(0.0f);
	bool firsthalfdone = false;
	for (int bin = 0; bin < 2 * ceil(1.0f / binprop); bin++) {
		curacc += binprop;
		auto i = lower_bound(acchis.begin(), acchis.end(), curacc);
		int b1 = i - acchis.begin(), b0 = b1 - 1;
		float value1 = acchis[b1], value0 = acchis[b0];

		if (b1 >= (short)Solid_Upper) {
			if (discrete_histogram_exemplar[MULTIRES - 1][Solid_Upper - 1]>0) {
				thresholdbin.push_back(Solid_Upper - 1);
				curacc = acchis[thresholdbin[bin]];
			}
			firsthalfdone = true;
			break;
		}
		else if (abs(value1 - curacc) > abs(value0 - curacc)) {
			if (bin - 1 < 0) { thresholdbin.push_back(b0); curacc = acchis[thresholdbin[bin]]; }
			else if (thresholdbin[bin - 1] != b0) { thresholdbin.push_back(b0);	curacc = acchis[thresholdbin[bin]]; }
			else bin--;
		}
		else { thresholdbin.push_back(b1); curacc = acchis[thresholdbin[bin]]; }

		//cout << " b1=" << b1 << " " << thresholdbin[bin];
		if (firsthalfdone || thresholdbin[bin] == (short)Solid_Upper - 1) break;
	}
	//push back twice Solid_Upper, in order to have a wieder gap between solid&pore
	thresholdbin.push_back(Solid_Upper); thresholdbin.push_back(Solid_Upper);
	thresholdbin.push_back(Pore_Lower);
	//second half
	bool secondhalf = false;
	curacc = acchis[Pore_Lower];
	for (int bin = thresholdbin.size(); bin < 2 * ceil(1.0f / binprop); bin++) {
		curacc += binprop;
		auto i = lower_bound(acchis.begin(), acchis.end(), curacc);
		int b1 = i - acchis.begin(), b0 = b1 - 1;
		float value1 = acchis[b1], value0 = acchis[b0];

		if (value1 >= 0.99f || value0 >= 0.99f) {
			thresholdbin.push_back(acchis.size() - 1);
			curacc = acchis[thresholdbin[bin]];
			secondhalf = true;
		}
		else if (abs(value1 - curacc) > abs(value0 - curacc)) {
			if (bin - 1 < 0) { thresholdbin.push_back(b0); curacc = acchis[thresholdbin[bin]]; }
			else if (thresholdbin[bin - 1] != b0) { thresholdbin.push_back(b0);	curacc = acchis[thresholdbin[bin]]; }
			else bin--;
		}
		else { thresholdbin.push_back(b1); curacc = acchis[thresholdbin[bin]]; }

		if (secondhalf) break;
	}
	//find half
	for (int bin = 0; bin < thresholdbin.size(); bin++)
		if (acchis[thresholdbin[bin]] >= 0.5f) { halfbin = bin; break; }

	// thresholdbin is done. now decide bingap & peakgap
	short peakgap;
	float bingap;
	//bool DMnogap = false;
	//if (!DMnogap) {
	//	bingap = 6;
	//	peakgap = 255 - bingap*(thresholdbin.size() - 2);
	//}
	//else {
	//	bingap = 255 / (thresholdbin.size() - 1);
	//	peakgap = bingap;
	//}

	bingap = 8.0;
	peakgap = 255.0 - bingap*(thresholdbin.size() - 2);
	while (peakgap < bingap * 15) {
		bingap -= 2;
		peakgap = 255.0 - bingap*(thresholdbin.size() - 2);
	}

	short Redis_Solid_Upper(0), Redis_Pore_Lower(0);

	//========= redistribute DM	=============
	for (ANNidx m = 0; m < Res.size(); m++) {
		auto i = lower_bound(thresholdbin.begin(), thresholdbin.end(), Res[m]);
		int num = i - thresholdbin.begin();

		if (num <= halfbin) {
			//if (thresholdbin[num] == Solid_Upper) Redis_Solid_Upper = num*bingap;
			//else if (thresholdbin[num] == Pore_Lower) Redis_Pore_Lower = num*bingap;
			Res[m] = num*bingap;
		}
		else {
			//if (thresholdbin[num] == Solid_Upper) Redis_Solid_Upper = (num - 1)*bingap + peakgap;
			//else if (thresholdbin[num] == Pore_Lower) Redis_Pore_Lower = (num - 1)*bingap + peakgap;
			Res[m] = (num - 1)*bingap + peakgap;
		}
	}
	auto s = lower_bound(thresholdbin.begin(), thresholdbin.end(), (short)Solid_Upper);
	short rs = s - thresholdbin.begin();
	if (rs <= halfbin) Redis_Solid_Upper = round(rs*bingap);
	else Redis_Solid_Upper = round((rs - 1)*bingap + peakgap);

	auto p = lower_bound(thresholdbin.begin(), thresholdbin.end(), (short)Pore_Lower);
	short rp = p - thresholdbin.begin();
	if (rp <= halfbin) Redis_Pore_Lower = round(rp*bingap);
	else Redis_Pore_Lower = round((rp - 1)*bingap + peakgap);

	cout << endl << "bingap = " << bingap << " peakgap = " << peakgap;
	cout << endl << "Redis_Solid_Upper= " << Redis_Solid_Upper << "  Redis_Pore_Lower= " << Redis_Pore_Lower << "  Halfbin= " << thresholdbin[halfbin];
	cout << endl << "\nThresholded bins: ";
	for (int i = 0; i < thresholdbin.size(); i++) {
		cout << " " << thresholdbin[i];
	}
	cout << endl << "Redistributed bins: ";
	for (int i = 0; i < thresholdbin.size(); i++) {
		if (i <= halfbin) 	cout << " " << i*bingap;
		else cout << " " << (i - 1)*bingap + peakgap;
	}

	// Write Bimodal image
	Mat BimodalMat = Mat(TEXSIZE[MULTIRES - 1], TEXSIZE[MULTIRES - 1], CV_8UC1);
	BimodalMat = Mat(Res, true).reshape(1, BimodalMat.rows);

	string tempoutputfilename = filename;
	tempoutputfilename = filename.substr(0, filename.find('.')) + "_S" + to_string(Redis_Solid_Upper) + "P" + to_string(Redis_Pore_Lower) + ".png";
	imwrite(tempoutputfilename, BimodalMat);
}
