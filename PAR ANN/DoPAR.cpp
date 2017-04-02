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
	dimensional_histogram_exemplar.resize(MULTIRES);

	discrete_histogram_exemplar.resize(MULTIRES);
	discrete_histogram_synthesis.resize(MULTIRES);

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
	
	WEIGHT_POSITIONHISTOGRAM.resize(MULTIRES,0);
	gaussiankernel.resize(MULTIRES);
	absoluteneigh.resize(MULTIRES);
	m_indexhistogram_exemplar.resize(MULTIRES);
	m_indexhistogram_synthesis.resize(MULTIRES);
	discrete_acchis_exemplar.resize(MULTIRES);
	existed_bin_exemplar.resize(MULTIRES);
	existed_histogram_examplar.resize(MULTIRES);

	Solid_Upper.resize(MULTIRES, 0);
	Pore_Lower.resize(MULTIRES, 0);
	DistanceThreshold.resize(MULTIRES, 0);
	ProjectDMapCompressRatio.resize(MULTIRES, 0);
	TIsDMmean.resize(MULTIRES, 0);
	
	ANNsearchk.resize(MULTIRES);
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

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool DoPAR::Write(const string FPathName, vector<uchar> Data)
{
	if (FPathName.size() == 0) return false;
	if (Data.size() == 0) return false;

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

	//----------read 3D model
	if (ResLines.size() > ++Row) {
		vector<string> ParV;
		GetNextRowParameters(Row, ResLines, ParV);
		if (ParV.size() > 0) {
			if (ParV.size() > 0) modelFilename3D = outputpath + ParV[0];
		}
		//////seperate filename and format!
		tempoutputformat = ".RAW";
		tempoutputfilename = ParV[0].substr(0, ParV[0].rfind('.') == std::string::npos ? ParV[0].length() : ParV[0].rfind('.'));
	}

	for (int i = 0; i < MULTIRES; i++) parameterstring += to_string(N[i]);
	if (INDEXHIS_ON) parameterstring += "ih";
	if (COLORHIS_ON) parameterstring += "ch";
	if (POSITIONHIS_ON) parameterstring += "ph";
	if (DISCRETE_ON) parameterstring += "ds";
	if (GAUSSIANFALLOFF_ON) parameterstring += "g";
	if (PROPORTIONTHRESHOLD_ON) parameterstring += "pt";
	if (DISCRETETHRESHOLD_ON) parameterstring += "dt";
	if (LINEARTRANSFORM_ON) parameterstring += "lt";
	if (DISTANCEMAP_ON) parameterstring += "DM";

	outputfilename = tempoutputfilename + "_"+ parameterstring + tempoutputformat;

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


	//vector<short> shortmodel(200*200*200)
	//ProportionThreshold(shortmodel, bin, prob);


	DoANNOptimization();
}

void VectorShortToMat(const vector<short>& in, Mat& out)
{
	vector<short>::const_iterator it = in.begin();
	MatIterator_<int> jt, end;
	jt = out.begin<int>();
	for (; it != in.end(); ++it) { *jt++ = (int)(*it); }
}
void VectorDoubleToMat(const vector<double>& in, Mat& out){
	vector<double>::const_iterator it = in.begin();
	MatIterator_<int> jt, end;
	jt = out.begin<int>();
	for (; it != in.end(); ++it) { *jt++ = (int)(*it); }
}

///========================== 190217 Kopf. optimization based =====================


//MULTIRES: larger number means finner level
const int DoPAR::N[MULTIRES] = {5};		//Kopf{ 3, 4, 4 }; Chen{3,4,4}; Turner{5,5,5}	// neighborhood size: (2 * N + 1)^2
int DoPAR::TEXSIZE[MULTIRES];
int DoPAR::D_NEIGHBOR[MULTIRES];
int DoPAR::NEIGHBORSIZE[MULTIRES];

const short DoPAR::NUM_HISTOGRAM_BIN = 64;		//for color histograms	Kopf used 16
vector<short> DoPAR::CHANNEL_MAXVALUE;			//for color histogram
int DoPAR::NUM_CHANNEL = 1;

short DoPAR::DISCRETE_HISTOGRAM_BIN = 256;		//for thresholding, discrete values. e.g. default256
vector<short> DoPAR::Solid_Upper;				//for redistribute distancemap model. e.g. 192: 0-192 solid
vector<short> DoPAR::Pore_Lower;				//for redistribute distancemap model
vector<short> DoPAR::DistanceThreshold;			//for binarise distance model.  DistanceThreshold=(Solid_Upper+Pore_Lower)/2
short DoPAR::ProjectDMapGap;
vector<double>DoPAR::ProjectDMapCompressRatio;
short DoPAR::ProjectDMapMaxBins;

double DoPAR::PCA_RATIO_VARIANCE = 0.99;		//Kopf used 0.95

const double DoPAR::ErrorBound = 2.0;			//Kopf used 2.0

vector<short> DoPAR::ANNsearchk;				//related to N[level]

const double DoPAR::gaussiansigma = 7.0;		//gaussian fall-off function in optimization phase, higher sigma means more uniform

const short DoPAR::MAXITERATION = 15;

void DoPAR::DoANNOptimization(){
	if (NUM_CHANNEL>1){ cout << endl << "Error NUM_CHANNEL>1"; _getch(); exit(1); }

	init();
	
	time_t StartTime;
	time(&StartTime);
	
	for (int curlevel = 0; curlevel < MULTIRES; curlevel++){
		cout << endl << "=============level: " << curlevel << "===============";
		globalenergy_new = 0.0; globalenergy_old = 10e-9;
		
		int convergencecount(0);
		for (int loop = 0; loop < MAXITERATION - MULTIRES; loop++){
			cout << endl << "---------iteration: " << loop+1<<"------------";
			
			if (loop == 0) FIRSTRUN = true;
			else FIRSTRUN = false;

			//if (loop>0) COUTCH_ON = true;
	
			searchVolume(curlevel);
			optimizeVolume(curlevel);

			if (LINEARTRANSFORM_ON) calcstddev(curlevel);

			if ((globalenergy_old - globalenergy_new)>0 && (globalenergy_old - globalenergy_new) / globalenergy_old < 0.02) convergencecount++;
			globalenergy_old = globalenergy_new;
			if (convergencecount > MULTIRES-curlevel) break;	//if change <1% for twice, then mark as converge
			if (LINEARTRANSFORM_ON && loop > 4) break;

			//if (ITERATIVEGETDMAP_ON) outputmodel(curlevel);
			//_getch();
		}
		if (curlevel >= MULTIRES-2) outputmodel(curlevel);
		//_getch();

		if (curlevel < MULTIRES - 1) {//level up
			upsampleVolume(curlevel);
			
			FIRSTRUN = true;
			calcHistogram_exemplar(curlevel + 1);
			if (COLORHIS_ON || DISCRETETHRESHOLD_ON) calcHistogram_synthesis(curlevel + 1);
			if (INDEXHIS_ON){
				initIndexHistogram(curlevel + 1);
			}
			if (POSITIONHIS_ON){
				initPositionHistogram_exemplar(curlevel + 1);
				initPositionHistogram_synthesis(curlevel + 1);		
			}
			//release vector
			cleardata(curlevel);
		}
	}

	time_t NewTime;
	time(&NewTime);
	cout << endl << "Total reconstruction time: " << long(NewTime - StartTime);

	//outputmodel(MULTIRES - 1);

	if (TRUE){//draw histogram graph
		int cols = TEXSIZE[MULTIRES - 1];
		int rows = 3*cols;
		if (POSITIONHIS_ON) writeHistogram(MULTIRES - 1, m_positionhistogram_synthesis[MULTIRES - 1], rows, cols, "PosHis_" + parameterstring+ ".png");
		if (INDEXHIS_ON) writeHistogram(MULTIRES - 1, m_indexhistogram_synthesis[MULTIRES - 1], 3 * (cols - 2 * N[MULTIRES - 1]), cols - 2 * N[MULTIRES - 1], "IndexHis_" + parameterstring + ".png");
	}
}

void DoPAR::init() {
	if (!loadExemplar()) return;

	if (!loadVolume()) return;
	
	calcNeighbor();

	//initthreshold();	
	InitGaussianKernel();
	initabsoluteneigh();
	
	//WEIGHT_HISTOGRAM = double(NUM_HISTOGRAM_BIN);
	calcHistogram_exemplar(0);
	if (COLORHIS_ON || DISCRETETHRESHOLD_ON) calcHistogram_synthesis(0);
	
	if (INDEXHIS_ON){
		initIndexHistogram(0);
	}
	if (POSITIONHIS_ON){		
		initPositionHistogram_exemplar(0);
		initPositionHistogram_synthesis(0);
	}
}

//void DoPAR::initthreshold(){
//}
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
	char type;
	if (DISTANCEMAP_ON){		
		do{
			cout <<endl<< "Generate converted TI of DistanceModel? [y/n]";
			cin >> type;
		} while (!cin.fail() && type != 'y' && type != 'n');
	}if (type == 'y') GenerateDMTI = true;


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
		if (POSITIONHIS_ON) m_volume_position[level].resize(TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
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
				ANNidx index = NUM_CHANNEL * (TEXSIZE[level] * v + u);
				ANNidx index2 = NUM_CHANNEL * (TEXSIZE[level] * v + u);
				for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
					m_exemplar_x[level][index + ch] = (unsigned char)img_x->imageData[index2 + ch];
					m_exemplar_y[level][index + ch] = (unsigned char)img_y->imageData[index2 + ch];
					m_exemplar_z[level][index + ch] = (unsigned char)img_z->imageData[index2 + ch];
				}
			}
		}

		//============= Convert to distance model ==========================
		if (DISTANCEMAP_ON){
			vector<char> tempchar(TEXSIZE[level] * TEXSIZE[level]);	
			vector<short> shortx(m_exemplar_x[level].begin(), m_exemplar_x[level].end());
			vector<short> shorty(m_exemplar_y[level].begin(), m_exemplar_y[level].end());
			vector<short> shortz(m_exemplar_z[level].begin(), m_exemplar_z[level].end());
		
			short autothresholdvalue(110);
			bool binaryYN(true);
			char type;
			if (binaryYN) for (int i = 0; i < shortx.size(); i++){ if (shortx[i]<255 && shortx[i]>0) { binaryYN = false; break; } }
			if (binaryYN) for (int i = 0; i < shorty.size(); i++){ if (shorty[i]<255 && shorty[i]>0) { binaryYN = false; break; } }
			if (binaryYN) for (int i = 0; i < shortz.size(); i++){ if (shortz[i]<255 && shortz[i]>0) { binaryYN = false; break; } }
			if (binaryYN == false && level == MULTIRES - 1){
				do{
					cout << endl << "Choose thresholding value for grey TIs yourself? [y/n]";
					cin >> type;
				} while (!cin.fail() && type != 'y' && type != 'n');
				if (type == 'y') {
					cout << endl << "Input autothresholdvalue for TIs (default=108): ";
					cin >> autothresholdvalue;
				}			
			}

			if (type == 'y') BinariseThreshold(shortx, tempchar, autothresholdvalue);
			else tempchar = BinariseImg(shortx, porosityxyz[0]);
			shortx = GetDMap(TEXSIZE[level], TEXSIZE[level], 1, tempchar, 2, false);

			if (type == 'y') BinariseThreshold(shorty, tempchar, autothresholdvalue);
			else tempchar = BinariseImg(shorty, porosityxyz[1]);
			shorty = GetDMap(TEXSIZE[level], TEXSIZE[level], 1, tempchar, 2, false);
			
			if (type == 'y') BinariseThreshold(shortz, tempchar, autothresholdvalue);
			else tempchar = BinariseImg(shortz, porosityxyz[2]);
			shortz = GetDMap(TEXSIZE[level], TEXSIZE[level], 1, tempchar, 2, false);
			
			//Decide solid_upper && pore_lower based on 3TIs
			PrepareDMapProjection(shortx, shorty, shortz, level);

			//redistribute distance values
			ProjectDMap(shortx, level);
			ProjectDMap(shorty, level);
			ProjectDMap(shortz, level);

			m_exemplar_x[level] = vector<double>(shortx.begin(), shortx.end());
			m_exemplar_y[level] = vector<double>(shorty.begin(), shorty.end());
			m_exemplar_z[level] = vector<double>(shortz.begin(), shortz.end());

			//LINEARTRANSFORM_ON: calculate TIsDMmean
			if (TRUE){
				TIsDMmean[level] = 0.0;	
				for (long idx = 0; idx < shortx.size(); ++idx){
					TIsDMmean[level] += shortx[idx];
				}
				for (long idx = 0; idx < shorty.size(); ++idx){
					TIsDMmean[level] += shorty[idx];
				}
				for (long idx = 0; idx < shortz.size(); ++idx){
					TIsDMmean[level] += shortz[idx];
				}
				TIsDMmean[level] /= (shortx.size() + shorty.size() + shortz.size());
				cout << endl << "TIsDMmean[" << level << "]= " << TIsDMmean[level]; //_getch();
			}

			//draw distance model
			if (GenerateDMTI){
				std::ostringstream name;

				Mat DM1 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
				VectorShortToMat(shortx, DM1);			
				name << "DM1_gap";
				name << ProjectDMapGap << "maxbin" << ProjectDMapMaxBins; if (MULTIRES > 1) name << "_L" << level;		
				name << ".png";
				imwrite(name.str(), DM1);	name.str(""); //name.clear();not necessary

				Mat DM2 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
				VectorShortToMat(shorty, DM2);
				name << "DM2_gap";
				name << ProjectDMapGap << "maxbin" << ProjectDMapMaxBins; if (MULTIRES > 1) name << "_L" << level;
				name << ".png";
				imwrite(name.str(), DM2);	name.str("");

				Mat DM3 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
				VectorShortToMat(shortz, DM3);
				name << "DM3_gap";
				name << ProjectDMapGap << "maxbin" << ProjectDMapMaxBins; if (MULTIRES > 1) name << "_L" << level;				
				name << ".png";
				imwrite(name.str(), DM3);	name.str("");
			}
		}
		
		//cout << endl;
		//for (int i = 0; i < m_exemplar_x[level].size(); i++){
		//	cout << m_exemplar_x[level][i] << " ";
		//}_getch();

		//cvShowImage("dismap", img_x);
		//cvWaitKey(0);
		if (level == 0) continue;
		// go to the coarser level
		cv::Mat tempmat(TEXSIZE[level] / 2, TEXSIZE[level] / 2, CV_8UC1);
		cv::resize(matxy, tempmat, tempmat.size(), 0, 0, INTER_AREA);
		IplImage* img_next_x = cvCloneImage(&(IplImage)tempmat);
		cv::resize(matxz, tempmat, tempmat.size(), 0, 0, INTER_AREA);
		IplImage* img_next_y = cvCloneImage(&(IplImage)tempmat);
		cv::resize(matyz, tempmat, tempmat.size(), 0, 0, INTER_AREA);
		IplImage* img_next_z = cvCloneImage(&(IplImage)tempmat);
		//IplImage *img_next_x = cvCreateImage(cvSize(TEXSIZE[level] / 2, TEXSIZE[level] / 2), img_depth, img_nChannels);
		//IplImage *img_next_y = cvCreateImage(cvSize(TEXSIZE[level] / 2, TEXSIZE[level] / 2), img_depth, img_nChannels);
		//IplImage *img_next_z = cvCreateImage(cvSize(TEXSIZE[level] / 2, TEXSIZE[level] / 2), img_depth, img_nChannels);
		//cvResize(img_x, img_next_x, CV_INTER_AREA);		//CV_INTER_AREA for shrink
		//cvResize(img_y, img_next_y, CV_INTER_AREA);		//CV_INTER_CUBIC for enlarge 
		//cvResize(img_z, img_next_z, CV_INTER_AREA);		//CV_INTER_LINEAR for speed

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
		if (DISTANCEMAP_ON) CHANNEL_MAXVALUE[ch] = DISCRETE_HISTOGRAM_BIN;
	}

	cout << endl << "load TIs done.";
	return true;
}
void DoPAR::calcNeighbor() {
	//initialize kdtree for certain template/neighbourhood
	cout << endl << "calcNeighbor...";
	for (int level = 0; level < MULTIRES; ++level) {
		//ANNsearchk now relates to neighbourhood size
		ANNsearchk[level] = N[level] * N[level];

		cout << endl << "level:" << level;
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
						ANNidx index = NUM_CHANNEL * (TEXSIZE[level] * (v + dv) + u + du);
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

		char type;
		do{
			type = 'n';

			dimPCA_x = 0;
			dimPCA_y = 0;
			dimPCA_z = 0;
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

			if (level == 0){
				do{
					cout << endl << "PCA_RATIO_VARIANCE= " << PCA_RATIO_VARIANCE << " Modify or not? [y / n]";
					cin >> type;
				} while (!cin.fail() && type != 'y' && type != 'n');
				if (type == 'y'){
					cout << endl << "new PCA_RATIO_VARIANCE(e.g.0.99)= ";
					cin >> PCA_RATIO_VARIANCE;
				}
			}
		} while (type == 'y' && PCA_RATIO_VARIANCE<1.0 && PCA_RATIO_VARIANCE>0.85);

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
			//ANNpoint* point array, row = 1, col = PCA dimension
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
		//ANNpoint* data point array = m_neighbor_kdTree_ptr_x, number of points = numData, dimension = dimPCA_x
		mp_neighbor_kdTree_x[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_x[level][0], numData, dimPCA_x); //ANNkd_tree
		mp_neighbor_kdTree_y[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_y[level][0], numData, dimPCA_y);
		mp_neighbor_kdTree_z[level] = new ANNkd_tree(&m_neighbor_kdTree_ptr_z[level][0], numData, dimPCA_z);
		
		
	
		//============ TEST TI PCA backproject result
		if (GenerateDMTI && level == 0){
			CvMat* backproject_x = cvCreateMat(numData, D_NEIGHBOR[level], CV_64F);
			cvBackProjectPCA(mp_neighbor_pca_projected_x[level], mp_neighbor_pca_average_x[level], mp_neighbor_pca_eigenvec_x[level], backproject_x);	
			Mat backprojectMat_x = cvarrToMat(backproject_x);			
			Mat PCAbackprojectDM1 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
			VectorDoubleToMat(m_exemplar_x[level], PCAbackprojectDM1);

			CvMat* backproject_y = cvCreateMat(numData, D_NEIGHBOR[level], CV_64F);
			cvBackProjectPCA(mp_neighbor_pca_projected_y[level], mp_neighbor_pca_average_y[level], mp_neighbor_pca_eigenvec_y[level], backproject_y);
			Mat backprojectMat_y = cvarrToMat(backproject_y);
			Mat PCAbackprojectDM2 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
			VectorDoubleToMat(m_exemplar_y[level], PCAbackprojectDM2);

			CvMat* backproject_z = cvCreateMat(numData, D_NEIGHBOR[level], CV_64F);
			cvBackProjectPCA(mp_neighbor_pca_projected_z[level], mp_neighbor_pca_average_z[level], mp_neighbor_pca_eigenvec_z[level], backproject_z);
			Mat backprojectMat_z = cvarrToMat(backproject_z);
			Mat PCAbackprojectDM3 = Mat(TEXSIZE[level], TEXSIZE[level], CV_8UC1);
			VectorDoubleToMat(m_exemplar_z[level], PCAbackprojectDM3);

			int row = 0; 
			int cols = 0.5*((2 * N[level] + 1)*(2 * N[level] + 1) - 1);
			for (int v = N[level]; v < TEXSIZE[level] - N[level]; ++v) {
				for (int u = N[level]; u < TEXSIZE[level] - N[level]; ++u) {
					PCAbackprojectDM1.at<uchar>(v, u) = backprojectMat_x.at<double>(row, cols);
					PCAbackprojectDM2.at<uchar>(v, u) = backprojectMat_y.at<double>(row, cols);
					PCAbackprojectDM3.at<uchar>(v, u) = backprojectMat_z.at<double>(row, cols);
					++row;
				}
			}

			imwrite("PCAbackproject_DM1.png", PCAbackprojectDM1);
			imwrite("PCAbackproject_DM2.png", PCAbackprojectDM2);
			imwrite("PCAbackproject_DM3.png", PCAbackprojectDM3);
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

	cout << endl << "calcNeighbor done.";
}

bool DoPAR::loadVolume(){
	//----------------convert Model(vector<uchar>) to m_volume (vector<vector<int>> (multires,ch*x*y*z))		
	//load from Model, later can also load from file		//level 0
	if (fileExists(modelFilename3D.c_str()) == true){	
		vector<uchar> model = load3Dmodel(modelFilename3D.c_str());	

		if (DISTANCEMAP_ON){
			//============= Convert to distance model ==========================
			long PoreNum = 0;
			short autothresholdvalue(0);
			bool binaryYN(true);
			if (binaryYN) for (int i = 0; i < model.size(); i++){ if ((model[i] != 0 && model[i]!=1) && (model[i]<255 && model[i]>0)) { binaryYN = false; break; } }
			if (binaryYN == false){
				cout << endl << "Input autothresholdvalue for TIs (default=110): ";
				cin >> autothresholdvalue;
			}
			for (ANNidx idx = 0; idx < model.size(); idx++){
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

			m_volume[0] = vector<double>(shortmodel.begin(), shortmodel.end());
			
			vector<uchar> tempmodel = vector<uchar>(shortmodel.begin(), shortmodel.end());
			string tempoutputfilename = outputfilename;
			tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "loadedDM" + ".RAW";
			Write(outputpath + tempoutputfilename, tempmodel);
		}
		else{
			m_volume[0] = vector<double>(model.begin(), model.end());
		}
		cout << endl << "load 3D model done.";
	}
	else{
		cout << endl << "3D initial model doesn't exist. Use Random initial.";
		InitRandomVolume(0);				//no need to convert again.
		cout << endl << "Init Random Volume Done.";
	}	
	return true;
}
void DoPAR::InitRandomVolume(int level) {
	vector<double>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	for (int xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
		ANNidx index2 = NUM_CHANNEL * (rand() % (TEXSIZE[level] * TEXSIZE[level]));
		int ori = rand() % 3;
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			m_volume[level][NUM_CHANNEL * xyz + ch] = p[ori]->operator[](index2 + ch);
		}
	}
}

void DoPAR::upsampleVolume(int level) {
	//============= Convert back to binary model first [leve]==========================
	vector<char> tempchar(m_volume[level].size());
	vector<short> shortmodel(m_volume[level].begin(), m_volume[level].end());
	double porosity(0.0);
	if (DISTANCEMAP_ON){
		BinariseThreshold(shortmodel, tempchar, Solid_Upper[level]);				//[0,1]	
		for_each(tempchar.rbegin(), tempchar.rend(), [&](char n) { porosity += n; });
		porosity /= tempchar.size();
		//cout << endl << "porosity=" << porosity; _getch();
	
		m_volume[level] = vector<double>(tempchar.begin(), tempchar.end());		//[0,1]
		transform(m_volume[level].begin(), m_volume[level].end(), m_volume[level].begin(), bind2nd(std::multiplies<double>(), 255)); //[0,255]
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

	//============= Convert to distance model [level+1]==========================
	if (DISTANCEMAP_ON){
		shortmodel.clear();
		tempchar.clear(); tempchar.resize(m_volume[level + 1].size());
		shortmodel = vector<short>(m_volume[level + 1].begin(), m_volume[level + 1].end());

		//BinariseThreshold(shortmodel, tempchar, 128);
		BinariseImg(shortmodel, porosity);
		shortmodel = GetDMap(TEXSIZE[level + 1], TEXSIZE[level + 1], TEXSIZE[level + 1], tempchar, 2, false);

		//redistribute distance values
		ProjectDMap(shortmodel, level+1);

		m_volume[level + 1] = vector<double>(shortmodel.begin(), shortmodel.end());		//[0,1]
	}
}

void DoPAR::outputmodel(int level){
	vector<uchar> tempmodel;
	tempmodel = vector<uchar>(m_volume[level].begin(), m_volume[level].end());
	string tempoutputfilename = outputfilename;
	if (MULTIRES>1) tempoutputfilename = outputfilename.substr(0, outputfilename.find('.')) + "_L" + to_string(level) + ".RAW";
	Write(outputpath + tempoutputfilename, tempmodel);
	//============= Convert to binary model [leve]==========================
	if (DISTANCEMAP_ON){
		vector<char> tempchar(tempmodel.size());
		//output an extra binary model
		vector<short> shortmodel(tempmodel.begin(), tempmodel.end());
		BinariseThreshold(shortmodel, tempchar, Solid_Upper[level]);
		//BinariseImg(shortmodel, porosityTI);
		tempmodel = vector<uchar>(tempchar.begin(), tempchar.end());
		tempoutputfilename = tempoutputfilename.substr(0, tempoutputfilename.find('.')) + "_binary.RAW";
		Write(outputpath + tempoutputfilename, tempmodel);
	}
	cout << endl << "output done.";
}
void DoPAR::writeHistogram(int level, vector<double> &hisvec, int rows, int cols, const string filename){
	if (filename.size() == 0) { cout << endl << "writeHistogram Error: empty filename"; return; };
	if (hisvec.size() != rows*cols) { cout << endl << "writeHistogram Error: wrong size of hisvec"; return; };

	Mat hist = Mat(rows, cols, CV_64FC1);
	memcpy(hist.data, hisvec.data(), hisvec.size()*sizeof(double));	//float 32F, double 64F
	hist *= TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level];
	hist.convertTo(hist, CV_8UC1);	//convertTo just copy the value, no scaling

	short i(0);
	string tempFPathName = filename;
	while (fileExists(tempFPathName) == true){
		tempFPathName = filename.substr(0, filename.find('.')) +  "_" + to_string(i) + ".png";
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

void DoPAR::cleardata(int level){	
	//m_exemplar_x[level].clear();
	//m_exemplar_y[level].clear();
	//m_exemplar_z[level].clear();
	m_neighbor_x[level].clear();
	m_neighbor_y[level].clear();
	m_neighbor_z[level].clear();
	//m_volume[level].resize(NUM_CHANNEL * TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	m_permutation_xyz[level].clear();
	m_volume_position[level].clear();
	m_volume_nearest_x_index[level].clear();
	m_volume_nearest_y_index[level].clear();
	m_volume_nearest_z_index[level].clear();
	m_volume_nearest_x_dist[level].clear();
	m_volume_nearest_y_dist[level].clear();
	m_volume_nearest_z_dist[level].clear();
	m_neighbor_kdTree_ptr_x[level].clear();
	m_neighbor_kdTree_ptr_y[level].clear();
	m_neighbor_kdTree_ptr_z[level].clear();
	//!!!Every use of new should be balanced by a delete, and every use of new[] should be balanced by delete[]
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

//================= phase 1: search ===========================
void DoPAR::searchVolume(int level) {
	long time_start = clock();
	const double min_dist = 0.00000000001;
	cout << endl <<"phase1:searching";
	vector<ANNidx> nearest_x_index_old = m_volume_nearest_x_index[level];	//size = texsize^3
	vector<ANNidx> nearest_y_index_old = m_volume_nearest_y_index[level];
	vector<ANNidx> nearest_z_index_old = m_volume_nearest_z_index[level];
	double global_energy_new = 0;
	long cnt_nearest_index_new = 0;
	//========For index histogram counting, shuffle the order. ========
	initPermutation(level);	//shuffle m_permutation_xyz

	int process = 0, displayprocess = -1;
	for (ANNidx i2 = 0; i2 < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i2) {
		//for each point in volume	(the sequence doesnt matter, all the points are searched and then optimize)
		if (level == MULTIRES - 1){
			process = i2 * 10 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
			if (process > displayprocess){
				displayprocess = process;
				cout << "\r" << displayprocess*10 << "%..";
			}
		}
		//========For index histogram counting, use shuffled index========
		ANNidx i = m_permutation_xyz[level][i2];	//x,y,z is random order
		int x = i % TEXSIZE[level];
		int y = (i / TEXSIZE[level]) % TEXSIZE[level];
		int z = i / (TEXSIZE[level] * TEXSIZE[level]);

		// obtain current neighborhood_x from volume
		CvMat* current_neighbor_x = cvCreateMat(1, D_NEIGHBOR[level], CV_64F); //rows = 1, cols = dimesnion
		CvMat* current_neighbor_y = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		CvMat* current_neighbor_z = cvCreateMat(1, D_NEIGHBOR[level], CV_64F);
		ANNidx index = 0;
		for (int dv = -N[level]; dv <= N[level]; ++dv) {	//N is neighbourhood size. 
			for (int du = -N[level]; du <= N[level]; ++du) {
				ANNidx index2_x = NUM_CHANNEL * (TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + dv) + TEXSIZE[level] * trimIndex(level, y + du) + x);
				ANNidx index2_y = NUM_CHANNEL * (TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + du) + TEXSIZE[level] * y + trimIndex(level, x + dv));
				ANNidx index2_z = NUM_CHANNEL * (TEXSIZE[level] * TEXSIZE[level] * z + TEXSIZE[level] * trimIndex(level, y + dv) + trimIndex(level, x + du));
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
		//==========multiple nearest index, position control=========
		ANNidxArray ann_index_x = new ANNidx[ANNsearchk[level]];
		ANNidxArray ann_index_y = new ANNidx[ANNsearchk[level]];
		ANNidxArray ann_index_z = new ANNidx[ANNsearchk[level]];
		ANNdistArray ann_dist_x = new ANNdist[ANNsearchk[level]];
		ANNdistArray ann_dist_y = new ANNdist[ANNsearchk[level]];
		ANNdistArray ann_dist_z = new ANNdist[ANNsearchk[level]];
		
		// ANN search. error bound = 2.0; Kopf used 2.0
		mp_neighbor_kdTree_x[level]->annkSearch(current_neighbor_x_projected->data.db, ANNsearchk[level], ann_index_x, ann_dist_x, ErrorBound);
		mp_neighbor_kdTree_y[level]->annkSearch(current_neighbor_y_projected->data.db, ANNsearchk[level], ann_index_y, ann_dist_y, ErrorBound);
		mp_neighbor_kdTree_z[level]->annkSearch(current_neighbor_z_projected->data.db, ANNsearchk[level], ann_index_z, ann_dist_z, ErrorBound);

		// CV release
		cvReleaseMat(&current_neighbor_x);
		cvReleaseMat(&current_neighbor_y);
		cvReleaseMat(&current_neighbor_z);
		cvReleaseMat(&current_neighbor_x_projected);
		cvReleaseMat(&current_neighbor_y_projected);
		cvReleaseMat(&current_neighbor_z_projected);
		
		int selected_index_x(0), selected_index_y(0), selected_index_z(0);
		//==========multiple nearest index, Index Histogram matching=========
		if (INDEXHIS_ON){
			selected_index_x = indexhistmatching_ann_index(level, 0, ann_index_x, ann_dist_x);
			selected_index_y = indexhistmatching_ann_index(level, 1, ann_index_y, ann_dist_y);
			selected_index_z = indexhistmatching_ann_index(level, 2, ann_index_z, ann_dist_z);
		
			//update index histogram		
			updateIndexHistogram(level, 0, m_volume_nearest_x_index[level][i], ann_index_x[selected_index_x]);	//level, ori, old, new
			updateIndexHistogram(level, 1, m_volume_nearest_y_index[level][i], ann_index_y[selected_index_y]);
			updateIndexHistogram(level, 2, m_volume_nearest_z_index[level][i], ann_index_z[selected_index_z]);
		}

		//update nearest_index, nearest_dist
		m_volume_nearest_x_index[level][i] = ann_index_x[selected_index_x];
		m_volume_nearest_y_index[level][i] = ann_index_y[selected_index_y];
		m_volume_nearest_z_index[level][i] = ann_index_z[selected_index_z];
		m_volume_nearest_x_dist[level][i] = ann_dist_x[selected_index_x] + min_dist;
		m_volume_nearest_y_dist[level][i] = ann_dist_y[selected_index_y] + min_dist;
		m_volume_nearest_z_dist[level][i] = ann_dist_z[selected_index_z] + min_dist;

		//search all points then optimize! Not search one point optimize one point!
		if (m_volume_nearest_x_index[level][i] != nearest_x_index_old[i]) ++cnt_nearest_index_new;	//just for illustration
		if (m_volume_nearest_y_index[level][i] != nearest_y_index_old[i]) ++cnt_nearest_index_new;
		if (m_volume_nearest_z_index[level][i] != nearest_z_index_old[i]) ++cnt_nearest_index_new;
		global_energy_new += m_volume_nearest_x_dist[level][i];	//just for illustration
		global_energy_new += m_volume_nearest_y_dist[level][i];
		global_energy_new += m_volume_nearest_z_dist[level][i];

		//Release ann_index
		//!!!Every use of new should be balanced by a delete, and every use of new[] should be balanced by delete[]
		delete ann_index_x;		delete ann_index_y;		delete ann_index_z;
		delete ann_dist_x;		delete ann_dist_y;		delete ann_dist_z;
	}
	long time_end = clock();
	cout << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
	cout << " updated NN: " << cnt_nearest_index_new <<", global energy: " << global_energy_new;

	globalenergy_new = global_energy_new;

}


//---------- Index Histogram for search step ---------
void DoPAR::initIndexHistogram(int level){
	if (FIRSTRUN){
		long numData = 3 * (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
		//init examplarhistogram
		m_indexhistogram_exemplar[level].resize(numData);
		m_indexhistogram_exemplar[level].assign(numData, 0.0);	
		for (ANNidx xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
			m_volume_nearest_x_index[level][xyz] = 10000000;
			m_volume_nearest_y_index[level][xyz] = 10000000;
			m_volume_nearest_z_index[level][xyz] = 10000000;
		}
		m_indexhistogram_synthesis[level].resize(numData);
		m_indexhistogram_synthesis[level].assign(numData, 0.0);

		return;
	}

	
	long numData = 3* (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	//init examplarhistogram
	m_indexhistogram_exemplar[level].resize(numData);
	m_indexhistogram_exemplar[level].assign(numData, 1.0 / ((TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level])));
				
	//initial uniform distribution
	ANNidx size = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	for (ANNidx xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
		m_volume_nearest_x_index[level][xyz] = xyz%size;
		m_volume_nearest_y_index[level][xyz] = xyz%size;
		m_volume_nearest_z_index[level][xyz] = xyz%size;
	}
	shuffle(m_volume_nearest_x_index[level].begin(), m_volume_nearest_x_index[level].end(), mersennetwistergenerator);
	shuffle(m_volume_nearest_y_index[level].begin(), m_volume_nearest_y_index[level].end(), mersennetwistergenerator);
	shuffle(m_volume_nearest_z_index[level].begin(), m_volume_nearest_z_index[level].end(), mersennetwistergenerator);
	//count each volume
	m_indexhistogram_synthesis[level].resize(numData);
	m_indexhistogram_synthesis[level].assign(numData, 0.0);
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	for (ANNidx i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; i++){
		ANNidx binx = m_volume_nearest_x_index[level][i];
		m_indexhistogram_synthesis[level][binx] += delta_histogram;
		ANNidx biny = m_volume_nearest_y_index[level][i] + size;
		m_indexhistogram_synthesis[level][biny] += delta_histogram;
		ANNidx binz = m_volume_nearest_z_index[level][i] + 2 * size;
		m_indexhistogram_synthesis[level][binz] += delta_histogram;
	}

	//writeHistogram(level, m_indexhistogram_synthesis[level], TEXSIZE[level] - 2 * N[level], 3 * (TEXSIZE[level] - 2 * N[level]), "IndexHis.png");
}
void DoPAR::updateIndexHistogram(int level, int orientation, const ANNidx oldannidx, const ANNidx newannidx){
	if (FIRSTRUN){
		ANNidx newidx = newannidx + orientation*(TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
		double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_indexhistogram_synthesis[level][newidx] += delta_histogram;

		return;
	}
	
	ANNidx newidx = newannidx + orientation*(TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	ANNidx oldidx = oldannidx + orientation*(TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	m_indexhistogram_synthesis[level][oldidx] -= delta_histogram;
	m_indexhistogram_synthesis[level][newidx] += delta_histogram;	
}
ANNidx DoPAR::indexhistmatching_ann_index(int level, int orientation, ANNidxArray idxarray, ANNdistArray distarry){
	ANNidx size = (TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]);
	double stddev = 1.0 / ((TEXSIZE[level] - 2 * N[level]) * (TEXSIZE[level] - 2 * N[level]));
	double probzero = gaussian_pdf(0.0, 0.0, stddev);
	double dist_acc(0.0), weight_acc(0.0); 
	double idx_acc(0.0);

	//simple form works better: just select minimum frequency
	vector<double> frequecyarray(ANNsearchk[level], 0.0);
	for (int i = 0; i < ANNsearchk[level]; i++){
		ANNidx idx = idxarray[i] + orientation*size;
		frequecyarray[i] = m_indexhistogram_synthesis[level][idx];
	}
	auto i = min_element(begin(frequecyarray), end(frequecyarray));
	int closestindex = distance(begin(frequecyarray), i);

	return closestindex;	
}


//================= phase 2: optimization =====================
void DoPAR::optimizeVolume(int level) {
	long time_start = clock();
	initPermutation(level);	//shuffle m_permutation_xyz
	cout <<endl<<"phase2:optimizing";
	int process = 0, displayprocess = -1;
	for (ANNidx i2 = 0; i2 < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i2) {
		ANNidx i = m_permutation_xyz[level][i2];	//x,y,z is random order
		int x = i % TEXSIZE[level];
		int y = (i / TEXSIZE[level]) % TEXSIZE[level];
		int z = i / (TEXSIZE[level] * TEXSIZE[level]);

		double weight_acc = 0.0;
		vector<double> color_acc(NUM_CHANNEL, 0.0);

		if (level == MULTIRES - 1){
			process = i2 * 10 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
			if (process > displayprocess){
				displayprocess = process;
				cout << "\r" << displayprocess*10 << "%..";
			}
		}

		//colorset and its positionset [3*NEIGHBORSIZE]	//just for NUM_CHANNEL=1
		vector<double> colorset;	if (DISCRETE_ON) colorset.resize(3 * NEIGHBORSIZE[level], 10e5);
		vector<ANNidx> positionset;	if (DISCRETE_ON&&POSITIONHIS_ON) positionset.resize(3 * NEIGHBORSIZE[level], 10000000);
		vector<double> positionfrequency; if (DISCRETE_ON&&POSITIONHIS_ON) positionfrequency.resize(3 * NEIGHBORSIZE[level], 10e2);
		//LINEAR TRANSFORM
		vector<double> transformedweight; if (LINEARTRANSFORM_ON) transformedweight.resize(3 * NEIGHBORSIZE[level], 0.0);
		

		int m = 0;	
		if (COUTCH_ON) cout << endl;
		//for every voxel's neighbourhood, in 3 orientations
		for (int dv = -N[level]; dv <= N[level]; ++dv) {
			for (int du = -N[level]; du <= N[level]; ++du) {
				//skipping boundary neighbourhood
				if ((z + dv <0 || z + dv>TEXSIZE[level] - 1) || (y + du <0 || y + du >TEXSIZE[level] - 1)
					|| (z + du <0 || z + du >TEXSIZE[level] - 1) || (x + dv <0 || x + dv >TEXSIZE[level] - 1)
					|| (y + dv <0 || y + dv >TEXSIZE[level] - 1) || (x + du <0 || x + du >TEXSIZE[level] - 1))
				{
					m++;
					continue;
				}

				//corresponding index of neighbour
				ANNidx index2_x = TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + dv) + TEXSIZE[level] * trimIndex(level, y + du) + x;
				ANNidx index2_y = TEXSIZE[level] * TEXSIZE[level] * trimIndex(level, z + du) + TEXSIZE[level] * y + trimIndex(level, x + dv);
				ANNidx index2_z = TEXSIZE[level] * TEXSIZE[level] * z + TEXSIZE[level] * trimIndex(level, y + dv) + trimIndex(level, x + du);
				ANNidx index2[3] = { index2_x, index2_y, index2_z };
				vector<ANNidx>* volume_nearest_index[3] = { &m_volume_nearest_x_index[level], &m_volume_nearest_y_index[level], &m_volume_nearest_z_index[level] };
				vector<double>* volume_nearest_dist[3] = { &m_volume_nearest_x_dist[level], &m_volume_nearest_y_dist[level], &m_volume_nearest_z_dist[level] };
				vector<double>* neighbor[3] = { &m_neighbor_x[level], &m_neighbor_y[level], &m_neighbor_z[level] };
				for (int ori = 0; ori < 3; ++ori) {//3 orientations
					ANNidx   nearest_index = (*volume_nearest_index[ori])[index2[ori]];
					double nearest_dist = (*volume_nearest_dist[ori])[index2[ori]];
					double* p_neighbor = &(*neighbor[ori])[D_NEIGHBOR[level] * nearest_index];
					// color of overlapping neighborhood pixel
					vector<double> color(NUM_CHANNEL);
					for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
						color[ch] = p_neighbor[NUM_CHANNEL * (NEIGHBORSIZE[level] - 1 - m) + ch];	
						//index2 ~ m, index ~ NEIGHBORSIZE[level] - 1 - m; the position is symmetrical!
												
						//========discrete solver=================	
						if (DISCRETE_ON) colorset[ori*NEIGHBORSIZE[level] + m] = color[ch];//record each color and its position				
						if (POSITIONHIS_ON && DISCRETE_ON){
							//relative index to centre:(NEIGHBORSIZE[level] - 1)/2 - m	absolute index:annconvert(nearest_index) + absolute[relative]
							positionset[ori*NEIGHBORSIZE[level] + m] = ori*(TEXSIZE[level] * TEXSIZE[level] )
																	+ (convertIndexANN(level, nearest_index) + absoluteneigh[level][NEIGHBORSIZE[level] - 1 - m]);				
							positionfrequency[ori*NEIGHBORSIZE[level] + m] = m_positionhistogram_synthesis[level][positionset[ori*NEIGHBORSIZE[level] + m]];;
						}
					}
					// blending weight of this color according to matching distance
					double weight = pow(nearest_dist, -0.6);	//L2norm(i.e.2)*-0.6 = -1.2 = 0.8(i.e.r)-2	

					// modify weight according to Color histogram matching
					if (COLORHIS_ON){
						double histogram_matching = 0.0;
						double difference = 0.0;
						double histogram_exemplar = 0.0;
						double histogram_synthesis = 0.0;
						for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
							int bin = (int)(color[ch] * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
							histogram_exemplar = m_histogram_exemplar[level][ch][bin];	
							//histogram_exemplar = dimensional_histogram_exemplar[level][ori][bin];
							
							histogram_synthesis = m_histogram_synthesis[level][ch][bin];	
							difference = histogram_synthesis - histogram_exemplar;
							histogram_matching = max(0.0, histogram_synthesis - histogram_exemplar);		
						}
						////changed to gaussian distribution, std = accepted error
						double ColourHisstddev = 0.01;  	// accept maximum stddev*3 error
						double ColourHisgaussianprob(0.0);
						////increase weight when (positionhistogram_synthesis - positionhistogram_exemplar)<0	
						//if (difference >= 0) ColourHisgaussianprob = gaussian_pdf(difference, 0.0, ColourHisstddev);
						//else ColourHisgaussianprob = 2 * gaussian_pdf(0.0, 0.0, ColourHisstddev) - gaussian_pdf(-difference, 0.0, ColourHisstddev);
						ColourHisgaussianprob = gaussian_pdf(histogram_matching, 0.0, ColourHisstddev);					
						weight *= ColourHisgaussianprob / gaussian_pdf(0.0, 0.0, ColourHisstddev);		//(0,1]	
						//weight *= 1.0 / (1.0 + WEIGHT_HISTOGRAM * histogram_matching);	//linear weight
					}
					//// modify weight according to Position Histogram matching
					if (POSITIONHIS_ON && DISCRETE_ON){
						double positionhistogram_exemplar = m_positionhistogram_exemplar[level][positionset[ori*NEIGHBORSIZE[level] + m]];
						double positionhistogram_synthesis = positionfrequency[ori*NEIGHBORSIZE[level] + m];
						//changed to gaussian distribution, std = averagef. for better influence
						double stddev = 1.0 / m_positionhistogram_exemplar[level].size();  	
						double gaussianprob(0.0);
						double difference = positionhistogram_synthesis - positionhistogram_exemplar;
						////increase weight when (positionhistogram_synthesis - positionhistogram_exemplar)<0	
						//if (difference >= 0) gaussianprob = gaussian_pdf(difference, 0.0, stddev);
						//else gaussianprob = 2 * gaussian_pdf(0.0, 0.0, stddev) - gaussian_pdf(-difference, 0.0, stddev);
						double positionhistogram_matching = max(0.0, difference);
						gaussianprob = gaussian_pdf(positionhistogram_matching, 0.0, stddev);
						weight *= gaussianprob / gaussian_pdf(0.0, 0.0, stddev);		//(0,1]
						//former used linear weight:
						//weight *= 1.0 / (1.0 + WEIGHT_POSITIONHISTOGRAM[level] * positionhistogram_matching);	
					}

					//Gaussian fall-off
					if (GAUSSIANFALLOFF_ON) weight *= gaussiankernel[level][m] * (gaussiankernel[level].size());	//or NEIGHBORSIZE[level] - 1 - m


					// accumulate color
					for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
						color_acc[ch] += weight * color[ch];						
					}
					weight_acc += weight;

					//======== Linear Transform from c_new to c_new' ===============
					if (LINEARTRANSFORM_ON) transformedweight[ori*NEIGHBORSIZE[level] + m] = weight;

					if (COUTCH_ON) {
						if (ori == 0) {
							cout << color[0] << " ";	
							if (m % (N[level] * 2 + 1) == N[level] * 2) cout << ";" << endl;
						}
					}
				}//3 orientations
				++m;
			}
		}//for every voxel's neighbourhood, in 3 orientations

		

		// old & new colors for this voxel
		vector<double> color_old(NUM_CHANNEL);
		vector<double> color_new(NUM_CHANNEL);
		//old & new position index for this voxel
		ANNidx position_old, position_new;
		int closestindex;
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			color_old[ch] = m_volume[level][NUM_CHANNEL * i + ch];
			//least square solver
			color_new[ch] = 1.0*color_acc[ch] / weight_acc;		
			
			if (COUTCH_ON) { cout << "---------"<< color_new[0]<<"-------"; _getch(); }

			//======== Linear Transform from c_new to c_new' ===============
			if (LINEARTRANSFORM_ON){
				double transformedweightsquare_acc(0.0);
				double check(0.0); 
				for (int i = 0; i < 3 * NEIGHBORSIZE[level]; i++){
					transformedweight[i] /= weight_acc;
					transformedweightsquare_acc += transformedweight[i] * transformedweight[i];	//sum(wi'^2)
					check += transformedweight[i];
				}
				double alpha = 1.0 / sqrt(transformedweightsquare_acc);
				double beta = (1.0 - alpha)*TIsDMmean[level];
				
				//double temp = color_new[0];
				color_new[0] = alpha* color_new[0] + beta;		//Linear transform to have same mean and dev as TIs
				if (check<0.99999 || check>1.00001) exit(1);
			}

			//========discrete solver=================	
			if (DISCRETE_ON){	
				//first calculate the weighted average color, then find the most similar color existed in exemplar, each color corresponds to a m_volume_nearest_index
				closestindex = FindClosestColorIndex(level, colorset, positionfrequency, color_new[ch]);			
				color_new[ch] = colorset[closestindex];		
				//Position histogram update
				if (POSITIONHIS_ON){
					position_new = positionset[closestindex];
					position_old = m_volume_position[level][i];
					updatePositionHistogram_synthesis(level, position_old, position_new);
					m_volume_position[level][i] = position_new;
				}
			}
		}
		//color histogram update
		if (COLORHIS_ON || DISCRETETHRESHOLD_ON){
			updateHistogram_synthesis(level, color_old, color_new);
		}			
		// voxel update
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			m_volume[level][NUM_CHANNEL * i + ch] = color_new[ch];		
		}
	}//for every voxel
	

	//========= Dynamic Thresholding based on TI histogram =========
	if (PROPORTIONTHRESHOLD_ON){
		vector<short> tempshort = vector<short>(m_volume[level].begin(), m_volume[level].end());
		ProportionThreshold(tempshort, existed_bin_exemplar[level], existed_histogram_examplar[level]);
		m_volume[level] = vector<double>(tempshort.begin(), tempshort.end());
	}
	else if (DISCRETETHRESHOLD_ON) DynamicThresholding(level);


	//========= Binarise & GetDMap ===============
	if (DISTANCEMAP_ON && ITERATIVEGETDMAP_ON){
		vector<short> tempshort(m_volume[level].size());
		vector<char> tempchar(m_volume[level].size());
		if (!PROPORTIONTHRESHOLD_ON && !DISCRETETHRESHOLD_ON) transform(m_volume[level].begin(), m_volume[level].end(), tempshort.begin(), std::bind1st(std::multiplies<short>(), 100));	//32767/256 = 128
		if (PROPORTIONTHRESHOLD_ON || DISCRETETHRESHOLD_ON) BinariseThreshold(tempshort, tempchar, Solid_Upper[level]);
		else tempchar = BinariseImg(tempshort, porosityTI);
		tempshort = GetDMap(TEXSIZE[level], TEXSIZE[level], TEXSIZE[level], tempchar, 2, false);
		ProjectDMap(tempshort, level);
		m_volume[level] = vector<double>(tempshort.begin(), tempshort.end());
	}


	long time_end = clock();
	cout << "done. clocks = " << (time_end - time_start) / CLOCKS_PER_SEC;
}


void DoPAR::initPermutation(int level) {// random permutation (precomputed)
	for (ANNidx i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i) {
		m_permutation_xyz[level][i] = i;
	}
	shuffle(m_permutation_xyz[level].begin(), m_permutation_xyz[level].end(), mersennetwistergenerator);
}

//---------- Color histogram ---------------
void DoPAR::calcHistogram_exemplar(int level) {
	m_histogram_exemplar[level].clear();
	m_histogram_exemplar[level].resize(NUM_CHANNEL, vector<double>(NUM_HISTOGRAM_BIN, 0));
	//dimensional_histogram_exemplar[level].clear();
	//if (COLORHIS_ON) dimensional_histogram_exemplar[level].resize(3, vector<double>(NUM_HISTOGRAM_BIN, 0));	//[level][ori][bin]
	discrete_histogram_exemplar[level].clear();
	if (DISCRETETHRESHOLD_ON || PROPORTIONTHRESHOLD_ON) discrete_histogram_exemplar[level].resize(DISCRETE_HISTOGRAM_BIN, 0.0);		//[level][discretebin]

	double dimensional_delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level]);
	double discrete_delta_histogram = 1.0 / (3 * TEXSIZE[level] * TEXSIZE[level]);
	vector<double>* p[3] = { &m_exemplar_x[level], &m_exemplar_y[level], &m_exemplar_z[level] };
	for (int ori = 0; ori < 3; ++ori) {
		for (int i = 0; i < TEXSIZE[level] * TEXSIZE[level]; ++i) {
			for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
				double c = (*p[ori])[NUM_CHANNEL * i + ch];
				if (COLORHIS_ON){					
					int bin = (int)(c * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
					m_histogram_exemplar[level][ch][bin] += discrete_delta_histogram;
					//dimensional_histogram_exemplar[level][ori][bin] += dimensional_delta_histogram;
				}
				if (DISCRETETHRESHOLD_ON || PROPORTIONTHRESHOLD_ON) discrete_histogram_exemplar[level][(int)c] += discrete_delta_histogram;
			}
		}
	}
	// calc accumulate histogram (discrete)
	if (DISCRETETHRESHOLD_ON){
		discrete_acchis_exemplar[level].resize(DISCRETE_HISTOGRAM_BIN, 0.0);
		calcaccHistogram(discrete_histogram_exemplar[level], discrete_acchis_exemplar[level]);
	}
	// record existed colorset (discrete)
	if (DISCRETETHRESHOLD_ON || PROPORTIONTHRESHOLD_ON){
		existed_bin_exemplar[level].clear();
		existed_histogram_examplar[level].clear();
		for (int c = 0; c < DISCRETE_HISTOGRAM_BIN; c++){
			if (discrete_histogram_exemplar[level][c] > 0.0) {
				existed_bin_exemplar[level].push_back(c);
				existed_histogram_examplar[level].push_back(discrete_histogram_exemplar[level][c]);
			}
		}
	}

	//for (int i = 0; i < existed_bin_exemplar[level].size(); i++){
	//	cout << endl << existed_bin_exemplar[level][i] << "     " << existed_histogram_examplar[level][i];
	//}_getch();
}
void DoPAR::calcHistogram_synthesis(int level) {
	m_histogram_synthesis[level].clear();
	m_histogram_synthesis[level].resize(NUM_CHANNEL, vector<double>(NUM_HISTOGRAM_BIN, 0.0));
	discrete_histogram_synthesis[level].clear();
	if (DISCRETETHRESHOLD_ON) discrete_histogram_synthesis[level].resize(DISCRETE_HISTOGRAM_BIN, 0.0);

	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	for (int i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++i) {
		for (int ch = 0; ch < NUM_CHANNEL; ++ch) {
			double c = m_volume[level][NUM_CHANNEL * i + ch];
			int bin = (int)(c * NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE[ch]);
			m_histogram_synthesis[level][ch][bin] += delta_histogram;

			int discretebin = (int)c;
			if (DISCRETETHRESHOLD_ON) discrete_histogram_synthesis[level][discretebin] += delta_histogram;
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

		int discretebin_old = (int)color_old[ch];
		int discretebin_new = (int)color_new[ch];
		if (DISCRETETHRESHOLD_ON) {
			discrete_histogram_synthesis[level][discretebin_old] -= delta_histogram;
			discrete_histogram_synthesis[level][discretebin_new] += delta_histogram;
		}
	}
}

//----------- Position Histogram -----------
void DoPAR::initPositionHistogram_exemplar(int level){
	//initial uniform distribution
	m_positionhistogram_exemplar[level].resize(3 * (TEXSIZE[level] * TEXSIZE[level]));
	m_positionhistogram_exemplar[level].assign(3 * (TEXSIZE[level] * TEXSIZE[level]), 0.0);
	for (ANNidx i = 0; i < m_positionhistogram_exemplar[level].size(); i++){
		m_positionhistogram_exemplar[level][i] = 1.0 / m_positionhistogram_exemplar[level].size();
	}

	WEIGHT_POSITIONHISTOGRAM[level] = 1.0* m_positionhistogram_exemplar[level].size();
}
void DoPAR::initPositionHistogram_synthesis(int level){
	if (FIRSTRUN){
		for (ANNidx xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
			m_volume_position[level][xyz] = 10000000;
		}
		m_positionhistogram_synthesis[level].resize(3 * (TEXSIZE[level] * TEXSIZE[level]));
		m_positionhistogram_synthesis[level].assign(3 * (TEXSIZE[level] * TEXSIZE[level]), 0.0);

		return;
	}

	//initial uniform distribution
	long maxsize = 3 * (TEXSIZE[level] * TEXSIZE[level]);
	for (ANNidx xyz = 0; xyz < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; ++xyz) {
		m_volume_position[level][xyz] = xyz%maxsize;
	}
	shuffle(m_volume_position[level].begin(), m_volume_position[level].end(), mersennetwistergenerator);
	//count each volume
	m_positionhistogram_synthesis[level].resize(3 * (TEXSIZE[level] * TEXSIZE[level]));
	m_positionhistogram_synthesis[level].assign(3 * (TEXSIZE[level] * TEXSIZE[level]), 0.0);
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	for (ANNidx i = 0; i < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; i++){
		ANNidx bin = m_volume_position[level][i];
		m_positionhistogram_synthesis[level][bin] += delta_histogram;
	}
}
void DoPAR::updatePositionHistogram_synthesis(int level, const ANNidx position_old, const ANNidx position_new){
	if (FIRSTRUN){
		double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
		m_positionhistogram_synthesis[level][position_new] += delta_histogram;

		return;
	}
	
	double delta_histogram = 1.0 / (TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]);
	m_positionhistogram_synthesis[level][position_old] -= delta_histogram;
	m_positionhistogram_synthesis[level][position_new] += delta_histogram;
}
int DoPAR::FindClosestColorIndex(int level, vector<double> &color, vector<double> &weight, double referencecolor){
	//find nearest color value, then compare weight for all closest values
	//return the final index
	
	if (DISTANCEMAP_ON) { 
		if (referencecolor < Solid_Upper[level]) referencecolor = floor(referencecolor);
		if (referencecolor > Pore_Lower[level]) referencecolor = ceil(referencecolor);
	}

	auto i = min_element(begin(color), end(color), [=](double x, double y)
	{
		return abs(x - referencecolor) < abs(y - referencecolor);
	});
	int closestindex = distance(begin(color), i);

	if (POSITIONHIS_ON){//after choosing closest color value, filter again in these value positions, choose minimum frequency		
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

	return closestindex;
}

//----------- Dynamic thresholding, worse than proportionthreshold ---------
void DoPAR::calcaccHistogram(vector<double> &inputhis, vector<double> &acchis){
	if (inputhis.size() != acchis.size()) { cout << endl << "accHistogram size " << acchis.size() << " not match " << inputhis.size(); _getch(); }
	acchis = inputhis;
	for (int i = 1; i < inputhis.size(); i++){
		acchis[i] += acchis[i-1];
	}
	if (acchis[acchis.size() - 1] > 1.0001 || acchis[acchis.size() - 1] < 0.9999) { cout << endl << "Error: acchis max=" << acchis[acchis.size() - 1]; _getch(); }
}
void DoPAR::DynamicThresholding(int level){
	if (PROPORTIONTHRESHOLD_ON) { cout << endl << "PROPORTIONTHRESHOLD_ON"; _getch(); }
	//if (DISCRETE_HISTOGRAM_BIN != CHANNEL_MAXVALUE[0]) { cout << endl << "DISCRETE_HISTOGRAM_BIN=" << DISCRETE_HISTOGRAM_BIN << " only accept " << CHANNEL_MAXVALUE[0]; _getch(); }
	//first calculate accumulate histogram
	vector<double> acchis_synthesis(DISCRETE_HISTOGRAM_BIN, 0.0);		//ch=0
	calcaccHistogram(discrete_histogram_synthesis[level], acchis_synthesis);

	//calc threshold value set
	vector<double> thresholdvalue(existed_bin_exemplar[level].size()-1, 0.0);
	//----------- Linear solver, not very accurate ----------------------
	auto vectoriterator = upper_bound(acchis_synthesis.begin(), acchis_synthesis.end(), 0.0);
	for (int i = 0; i < existed_bin_exemplar[level].size() - 1; i++){		//ignore the last threshold(should be max)
		double acc = discrete_acchis_exemplar[level][existed_bin_exemplar[level][i]];
		vectoriterator = upper_bound(acchis_synthesis.begin(), acchis_synthesis.end(), acc);
		int upper = vectoriterator - acchis_synthesis.begin();
		int lower = max(0, upper - 1);			//!should be lower_bound, but because the weighted average set is continuious, it is equal to upper-1
		if (lower == 0) {/* cout << endl << endl << "up_acc - low_acc=0" << endl;  */thresholdvalue[i] = 0.0; continue; }
		double up_acc = acchis_synthesis[upper];
		double low_acc = acchis_synthesis[lower];
		thresholdvalue[i] =  1.0*lower + (acc - low_acc) / (up_acc - low_acc);	//assume linear distribution within acchis_synthesis[lower,upper]	
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
	for (ANNidx m = 0; m < TEXSIZE[level] * TEXSIZE[level] * TEXSIZE[level]; m++){
		auto i = lower_bound(thresholdvalue.begin(), thresholdvalue.end(), m_volume[level][m]);
		int bin = i - thresholdvalue.begin();		
		m_volume[level][m] = existed_bin_exemplar[level][bin];
		if (m_volume[level][m] > DISCRETE_HISTOGRAM_BIN-1) m_volume[level][m] = DISCRETE_HISTOGRAM_BIN-1;
	}

	//update color histogram after changing value
	calcHistogram_synthesis(level);
}
//Non-linear solver
void DoPAR::PolynomialInterpolation(vector<double>& Xv, vector<double>& Yv, vector<double>& X){
	//(1) Xv, Yv are a set of points
	//   Note that no two x-values are the same, otherwise this programm could collapse 
	//(2) A set of x values as input, will get y values as output

	if (Xv.size() != Yv.size() || Xv.size() == 0 || X.size() == 0) { cout << endl << "PolynomialInterpolation:size error"; return; }

	vector<double> XX, YRes;

	XX.resize(Xv.size());

	for (long ij = 0; ij < Xv.size(); ++ij) {
		XX[ij] = 1.0;
		for (long kl = 0; kl < Xv.size(); ++kl) {
			if (kl == ij) continue;
			if (Xv[ij] == Xv[kl]) { cout << endl << "!!!Error: Xv[ij]=Xv[kl]!!!"; _getch(); return; }
			XX[ij] *= (Xv[ij] - Xv[kl]);
		}
	}

	YRes.resize(X.size());

	for (long idx = 0; idx < X.size(); ++idx) {
		YRes[idx] = 0.0;
		for (long ij = 0; ij < Xv.size(); ++ij) {
			double Val = 1.0;
			for (long kl = 0; kl < Xv.size(); ++kl) {
				if (kl == ij) continue;
				Val *= (X[idx] - Xv[kl]);
			}
			YRes[idx] += Val*Yv[ij] / XX[ij];
		}
	}

	YRes.swap(X);
}

//----------- Proportion Threshold, better ---------
void DoPAR::ProportionThreshold(vector<short>& Model, vector<short>& BinNum, vector<double>& Prob){
	//(1) Model to be thresholded according to a distribution <BinNum, Prob>
	//(2) BinNum and Prob corresponds to histogram (), BinNum must be ordered. The last BinNum is biggest.
	const bool SelNewAlgYN = false;

	if (BinNum.size() == 0 || Prob.size() != BinNum.size()) { cout << endl << "BinNum.size=" << BinNum.size() << " Prob.size=" << Prob.size(); _getch(); return; }
	if (Model.size() == 0) { cout << endl << "shortmodel.size=" << Model.size(); _getch(); return; }
	double check_sum_Prob(0.0); 
	for_each(Prob.rbegin(), Prob.rend(), [&](double n) { check_sum_Prob += n; });
	if (check_sum_Prob > 1.00001 || check_sum_Prob < 0.99999) { cout << endl << "check_sum_Prob=" << check_sum_Prob; _getch(); return; }

	if (!SelNewAlgYN) {
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
				//if (Tmp[shuffledidx[idx]] == 1) {
				//	ResModel[shuffledidx[idx]] = BinNum[ij];
				//	Model[shuffledidx[idx]] = MinVal;
				//}
			}
		}
		Model.swap(ResModel);
	}
	//else {//New algorithm. Not accurate for small frequency distribution
	//	short MaxVal, MinVal;
	//	MaxVal = MinVal = Model[0];

	//	for (long idx = 0; idx < Model.size(); ++idx) {
	//		if (Model[idx] < MinVal) MinVal = Model[idx];
	//		if (Model[idx] > MaxVal) MaxVal = Model[idx];
	//	}

	//	vector<long> CntValue(MaxVal - MinVal + 2, 0);
	//	vector<short> SelMlVal(MaxVal - MinVal + 2, BinNum[0]);

	//	for (long idx = 0; idx < Model.size(); ++idx) {
	//		CntValue[Model[idx] - MinVal]++;
	//	}

	//	long SelNo = Prob.size() - 1;
	//	long TotNum = Prob[SelNo] * Model.size();
	//	long RemNum(0);

	//	for (long ij = CntValue.size() - 1; ij >= 0; --ij) {
	//		if (CntValue[ij] == 0) continue;
	//		RemNum += CntValue[ij];
	//		if (RemNum > TotNum) {
	//			if (TotNum - RemNum + CntValue[ij] > CntValue[ij] / 2) {
	//				SelMlVal[ij] = BinNum[SelNo];
	//				RemNum = 0;
	//			}
	//			else {
	//				SelMlVal[ij] = BinNum[SelNo - 1];
	//				RemNum = SelMlVal[ij];
	//			}

	//			SelNo--;
	//			if (SelNo < 0) break;
	//			TotNum = Prob[SelNo] * Model.size();
	//		}
	//		else {
	//			SelMlVal[ij] = BinNum[SelNo];
	//		}
	//	}

	//	vector<short> ResModel(Model.size(), BinNum[0]);

	//	for (long idx = 0; idx < Model.size(); ++idx) {
	//		ResModel[idx] = SelMlVal[Model[idx] - MinVal];
	//	}

	//	Model.swap(ResModel);
	//}
}
//void DoPAR::ProportionThreshold(vector<double>& Model, vector<short>& BinNum, vector<double>& Prob){
//	//(1) Model to be thresholded according to a distribution <BinNum, Prob>
//	//(2) BinNum and Prob corresponds to histogram (), BinNum must be ordered. The last BinNum is biggest.
//	const bool SelNewAlgYN = false;
//
//	if (BinNum.size() == 0 || Prob.size() != BinNum.size()) { cout << endl << "BinNum.size=" << BinNum.size() << " Prob.size=" << Prob.size(); _getch(); return; }
//	if (Model.size() == 0) { cout << endl << "shortmodel.size=" << Model.size(); _getch(); return; }
//	double check_sum_Prob(0.0);
//	for_each(Prob.rbegin(), Prob.rend(), [&](double n) { check_sum_Prob += n; });
//	if (check_sum_Prob > 1.0001 || check_sum_Prob < 0.9999) { cout << endl << "check_sum_Prob=" << check_sum_Prob; _getch(); return; }
//
//	if (!SelNewAlgYN) {
//
//		double MinVal = Model[0];
//
//		for (long idx = 0; idx < Model.size(); ++idx) {
//			if (Model[idx] < MinVal)
//				MinVal = Model[idx];
//		}
//
//		vector<double> ResModel(Model.size(), BinNum[0]);
//
//		for (long ij = BinNum.size() - 1; ij > 0; --ij) {
//			if (long(1000 * Prob[ij]) < 0) continue;
//			vector<char> Tmp;
//			Tmp = BinariseImg(Model, Prob[ij]);
//			for (long idx = 0; idx < Model.size(); ++idx) {
//				if (Tmp[idx] == 1) {
//					ResModel[idx] = BinNum[ij];
//					Model[idx] = MinVal;
//				}
//			}
//		}
//		Model.swap(ResModel);
//	}
//	else {//New algorithm 
//		short MaxVal, MinVal;
//		MaxVal = MinVal = Model[0];
//
//		for (long idx = 0; idx < Model.size(); ++idx) {
//			if (Model[idx] < MinVal) MinVal = Model[idx];
//			if (Model[idx] > MaxVal) MaxVal = Model[idx];
//		}
//
//		vector<long> CntValue(MaxVal - MinVal + 2, 0);
//		vector<short> SelMlVal(MaxVal - MinVal + 2, BinNum[0]);
//
//		for (long idx = 0; idx < Model.size(); ++idx) {
//			CntValue[Model[idx] - MinVal]++;
//		}
//
//		long SelNo = Prob.size() - 1;
//		long TotNum = Prob[SelNo] * Model.size();
//		long RemNum(0);
//
//		for (long ij = CntValue.size() - 1; ij >= 0; --ij) {
//			if (CntValue[ij] == 0) continue;
//			RemNum += CntValue[ij];
//			if (RemNum > TotNum) {
//				if (TotNum - RemNum + CntValue[ij] > CntValue[ij] / 2) {
//					SelMlVal[ij] = BinNum[SelNo];
//					RemNum = 0;
//				}
//				else {
//					SelMlVal[ij] = BinNum[SelNo - 1];
//					RemNum = SelMlVal[ij];
//				}
//
//				SelNo--;
//				if (SelNo < 0) break;
//				TotNum = Prob[SelNo] * Model.size();
//			}
//			else {
//				SelMlVal[ij] = BinNum[SelNo];
//			}
//		}
//
//		vector<short> ResModel(Model.size(), BinNum[0]);
//
//		for (long idx = 0; idx < Model.size(); ++idx) {
//			ResModel[idx] = SelMlVal[Model[idx] - MinVal];
//		}
//
//		Model.swap(ResModel);
//	}
//}


// ========= Distance Map ===========
vector<unsigned short> DoPAR::BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg){
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
		for (long CNum = 0, k = -1; k<2; ++k) {
			for (long j = -1; j<2; ++j) {
				for (long i = -1; i<2; ++i) {
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
		for (long idx = 0; idx<Size; ++idx)
		if (OImg[idx] > 0)
			DMap[idx] = MaxDistV;
	} //Initialize DMap and clear up OImg			

	vector<_XyzStrType> TgtImg(Size);  //coordinates (x,y,z) ///@~@

	short x, y, z, i, j, k, CNum;
	long idx, NIdx, CurVal;
	_XyzStrType TVal;

	TVal.x = 0; TVal.y = 0; TVal.z = 0;

	//cout<<" scanning forwards...";
	for (idx = -1, z = 0; z<Sz; ++z) {
		//if (z % 100 == 0) cout << "*";
		for (y = 0; y<Sy; ++y) {
			for (x = 0; x<Sx; ++x) { 	//if(++JCnt >= JStepNum) {JCnt=0; cout<<".";}
				if (DMap[++idx] < 1) {
					TgtImg[idx] = TVal;
				}
				else {
					for (CNum = -1, k = z - 1; k<z + 2; ++k) {
						if (k < 0 || k >= Sz) { CNum += 9; continue; }
						for (j = y - 1; j<y + 2; ++j) {
							if (j < 0 || j >= Sy) { CNum += 3; continue; }
							for (i = x - 1; i<x + 2; ++i) {
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
				for (CNum = 27, k = z + 1; k>z - 2; --k) {
					if (k < 0 || k >= Sz) { CNum -= 9; continue; }
					for (j = y + 1; j>y - 2; --j) {
						if (j < 0 || j >= Sy) { CNum -= 3; continue; }
						for (i = x + 1; i>x - 2; --i) {
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
vector<short> DoPAR::GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN){
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

			if (tDMap[idx] > MaxDis)
				MaxDis = tDMap[idx];
		}

		if (!DisValYN) {
			vector<bool> UsedYN(MaxDis + 1, false);
			for (long idx = 0; idx < tDMap.size(); ++idx) {
				if (tDMap[idx] > 0) {
					UsedYN[tDMap[idx]] = true;
				}
			}

			tDMap.clear();

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

			if (tDMap[idx] > MaxDis)
				MaxDis = tDMap[idx];
		}

		if (!DisValYN) {
			vector<bool> UsedYN(MaxDis + 1, false);
			for (long idx = 0; idx < tDMap.size(); ++idx) {
				if (tDMap[idx] > 0) {
					UsedYN[tDMap[idx]] = true;
				}
			}

			tDMap.clear();

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
vector<char> DoPAR::BinariseImg(vector<short>& DMap, double TPorosity){
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

void DoPAR::BinariseThreshold(vector<short>& DMap, vector<char>& Binarised, short threshold){
	//input  vector<short> DMap
	//output vector<char>Binarised 
	if (Binarised.size() != DMap.size()) { cout << endl << "BinariseThreshold() size not matched"; _getch(); return; }
	for (long i = 0; i < DMap.size(); i++){
		if (DMap[i] <= threshold) Binarised[i] = 0;
		else Binarised[i] = 1;
	}
}
void DoPAR::PrepareDMapProjection(vector<short>& TI1, vector<short>& TI2, vector<short>& TI3, int level){
	//dynamically decide solid_upper, pore_lower && DistanceThreshold
	//compute RedistributeRatio for 3TI

	short minVal, maxVal, minVal1, minVal2, minVal3, maxVal1, maxVal2, maxVal3;	//total min, max for 3TIs; and separately
	minVal = maxVal = TI1[0];	minVal1 = maxVal1 = TI1[0]; minVal2 = maxVal2 = TI2[0]; minVal3 = maxVal3 = TI3[0];
	for (long idx = 0; idx < TI1.size(); ++idx) {
		if (TI1[idx] < minVal) minVal = TI1[idx];
		if (TI1[idx] > maxVal) maxVal = TI1[idx];
		if (TI1[idx] < minVal1) minVal1 = TI1[idx];
		if (TI1[idx] > maxVal1) maxVal1 = TI1[idx];
	}cout << endl << "TI1: porosity= "<< PorosityX << " solid_min= " << minVal1 << " pore_max= " << maxVal1;	
	for (long idx = 0; idx < TI2.size(); ++idx) {
		if (TI2[idx] < minVal) minVal = TI2[idx];
		if (TI2[idx] > maxVal) maxVal = TI2[idx];
		if (TI2[idx] < minVal2) minVal2 = TI2[idx];
		if (TI2[idx] > maxVal2) maxVal2 = TI2[idx];
	}cout << endl << "TI2: porosity= " << PorosityY << " solid_min= " << minVal2 << " pore_max= " << maxVal2;
	for (long idx = 0; idx < TI3.size(); ++idx) {
		if (TI3[idx] < minVal) minVal = TI3[idx];
		if (TI3[idx] > maxVal) maxVal = TI3[idx];
		if (TI3[idx] < minVal3) minVal3 = TI3[idx];
		if (TI3[idx] > maxVal3) maxVal3 = TI3[idx];
	}cout << endl << "TI3: porosity= " << PorosityZ << " solid_min= " << minVal3 << " pore_max= " << maxVal3;

	ProjectDMapMaxBins = -minVal + maxVal;
	ProjectDMapGap = ceil(1.0*(DISCRETE_HISTOGRAM_BIN - 1) / (ProjectDMapMaxBins + 1));

	//compress minVal, maxVal, reducing the dimension. e.g.-1,-2,-3-->-1; -4,-5,-6-->-2...
	//ProjectDMapCompressRatio[level] = 1.0*(maxVal - minVal) / ProjectDMapMaxBins;
	ProjectDMapCompressRatio[level] = 1.0;

	Solid_Upper[level] = (-minVal * (DISCRETE_HISTOGRAM_BIN - 1 - ProjectDMapGap)) / (maxVal - minVal);
	Pore_Lower[level] = Solid_Upper[level] + ProjectDMapGap;

	DistanceThreshold[level] = 0.5*(Solid_Upper[level] + Pore_Lower[level]);

	cout << endl << "ProjectDMapMaxBins= " << ProjectDMapMaxBins << " ProjectDMapGap= " << ProjectDMapGap /*<< " ProjectDMapCompressRatio= " << ProjectDMapCompressRatio[level]*/;
	cout << endl << "Solid_Upper= " << Solid_Upper[level] << "  Pore_Lower= " << Pore_Lower[level];
}
void DoPAR::ProjectDMap(vector<short>& DMap, int level){
	//linear projection for DMap
	//solid: [minVal,-1] to [solid_upper, 1]  ;  pore: [1,maxVal] to [DISCRETE_HISTOGRAM_BIN, pore_lower] Note the order has been reversed!
	//if requires to have same ratio for solid and pore, use the bigger compress ratio
	
	//const bool equalratio = true;
	//const bool reversemodel = false;	
	////Use same ratio for three TIs, so no need to calculate again.
	double binsize = (DISCRETE_HISTOGRAM_BIN - 1.0 - ProjectDMapGap) / ProjectDMapMaxBins;

	//cout << endl << "Solid_Upper= " << Solid_Upper[level] << "  Pore_Lower= " << Pore_Lower[level];

	for (long idx = 0; idx < DMap.size(); ++idx){
		//if (!reversemodel){
		if (DMap[idx] > 0) { 
			DMap[idx] = ceil(((DMap[idx] - 1) / ProjectDMapCompressRatio[level])*binsize) + Pore_Lower[level];
			if (DMap[idx] > DISCRETE_HISTOGRAM_BIN - 1) DMap[idx] = DISCRETE_HISTOGRAM_BIN - 1; 
		}
		else if (DMap[idx] < 0) { 
			DMap[idx] = Solid_Upper[level] - ceil(((-DMap[idx] - 1) / ProjectDMapCompressRatio[level]) *binsize);
			if (DMap[idx] < 0) DMap[idx] = 0; 
		}
		
		//cout << DMap[idx] <<",";
	}
}

void DoPAR::calcstddev(int level){
	double sum = std::accumulate(m_volume[level].begin(), m_volume[level].end(), 0.0);
	double mean = sum / m_volume[level].size();

	std::vector<double> diff(m_volume[level].size());
	std::transform(m_volume[level].begin(), m_volume[level].end(), diff.begin(), [mean](double x) { return x - mean; });
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / m_volume[level].size());

	cout << endl << "mean=" << mean << " stdev=" << stdev << " size=" << m_volume[level].size();
}