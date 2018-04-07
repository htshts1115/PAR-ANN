#pragma once

#include "stdafx.h"

typedef struct { long x, y, z; } _XyzStrType;		//BarDMap()

typedef float size_color;
typedef long size_idx;
typedef float size_dist;
typedef long size_hiscount;

const unsigned long iSTEPLENGTH = 524288L;
static void iCGetDirFileName(string OStr, string& Dir, string & FName)
{
	FName = "";		Dir = "";
	char ch1 = '\\';	char ch2 = '/';
	unsigned long No, i;
	for (No = OStr.size() - 1; No >= 0; --No) {
		if (OStr[No] == ch1 || OStr[No] == ch2) break;
		FName = OStr[No] + FName;
	}
	for (i = 0; i <= No; ++i) Dir += OStr[i];
};

class DoPAR
{
public:
	DoPAR();
	~DoPAR();
	void GetStarted(string CurWorkExeFile, int TIseries);
private:

	void ReadRunPar_series(string CurExeFile, int TIseries);

	//=============================================================
	vector<string> FNameXY, FNameXZ, FNameYZ;
	string workpath, outputpath, outputfilename, parameterstring;
	
	random_device randomseed;
	mt19937 mersennetwistergenerator;
	uniform_real_distribution<double> probabilitydistribution;

	///========================== optimization based =====================

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int MULTIRES;										// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	vector<int> blockSize;								// template size	//tested: coarse level big for quality, fine level small for speed
	vector<int> TIsize;									// size of input exemplar
	vector<int> OUTsize;								// size of output 
	int outputsizeatlastlevel = 0;

	vector<int> MAXITERATION;							// max iteration time	//tested: fine level does not need many iterations
	int NumRealization;
	
	bool MultipleTIsYN;									//! if true, will try to use TIs with same prefix filename
	int MultipleTIsNum=1;								// how many TIs found(use)

	int COHERENCENUM = 9;								// K-coherence 11
	vector<double> ANNerror;
	bool useRandomSeed;									// Use random seed or fixed (0) for test (false)
	
	const int GRID = 2;									// sparse grid
	const size_dist min_dist = 0.1f;	
	
	bool HisEqYN = false;								// apply histogram equalization
	bool DMtransformYN = true;							// use DM transformation
	bool GenerateTI = false;							// generate DM transformed TI
	bool PrintHisYN = false;							// generate Histogram
	bool PatternEntropyAnalysisYN = true;				// analyse pattern size entropy


	bool testNoDiscrete = false;
	size_dist factorIndex = 0.1f;//0; // 0.5;				//!! change to factor * (MULTIRES-level)
	size_dist factorPos = 0.1f;//0; // 0.5;
	size_dist IndexHisManualControl = 1.0f, PosHisManualControl = 1.0f;

	int FixedLayerDir = -1;
	size_dist DirectionalWeight = 0.66; 

	bool ColorHis_ON = true;
	size_dist factorC = 0.1;
	size_hiscount poretotal_synthesis, poretotal_required;
	size_dist porosityX, porosityY, porosityZ;
	vector<size_dist> porosity_required;

	vector<size_dist> avgIndexHis;						// default average value of IndexHis
	vector<size_dist> avgPosHis;						// default average value of PosHis
	
	int MaxThread = 1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

	inline size_idx trimIndex(int level, size_idx index) {
		//!!for 3d output

		//if (isToroidal) {
			if (index < 0) index += OUTsize[level];
			return index % OUTsize[level];
		//}
		//else {//mirror
		//	while (true) {
		//		if (index < 0) index = -index;
		//		if (TEXSIZE[level] <= index) {
		//			index = 2 * (TEXSIZE[level] - 1) - index;
		//			continue;
		//		}
		//		break;
		//	}
		//	return index;
		//}
		//if (index < 0) return -index;
		//if (TEXSIZE[level] <= index) return 2 * (TEXSIZE[level] - 1) - index;
		//return index;
	}
	inline size_idx trim(size_idx SIZE, size_idx index) {
		//Toroidal
		if (index < 0) index += SIZE;
		return index % SIZE;

		//if (index < 0) return -index;
		//if (index >= SIZE) return 2 * (SIZE - 1) - index;
		//return index;
	}

	inline size_idx convertIndexANN(int level, size_idx index){
		//convert ANNSearch_nearest_x_index to TI index
		size_idx i, j, height, bias;
		height = TIsize[level] - blockSize[level] + 1;
		bias = static_cast<size_idx>(blockSize[level] * 0.5);
		i = index / height + bias;
		j = index % height + bias;
		return (i*TIsize[level] + j);
	}
	inline size_idx sparseIdx(int level, size_idx index) {
		//index is 2d. IndexHis
		//convert idx to sparsed grid --> width/2!
		size_idx i, j, height, sheight;
		height = TIsize[level];
		sheight = height / GRID;
		i = (index / height) / GRID;
		j = (index % height) / GRID;
		return (i*sheight + j);
	}


	void DoANNOptimization(int TIseries);
	
	void init(int TIseries);

	void allocateVectors();

	// 2D Exemplar 
	bool SIM2D_YN = false;											//2d reconstruction
	vector<vector<size_color> >  m_exemplar_x;									//[level][idx2d] = color
	vector<vector<size_color> >  m_exemplar_y;
	vector<vector<size_color> >  m_exemplar_z;
	bool loadExemplar();
	void gaussImage(int level, vector<vector<size_color>>& exemplar);

	void testPCA();		//compare PCA TI

	void equalizeHistogram(int level, vector<size_color>& exemplarX, vector<size_color>& exemplarY, vector<size_color>& exemplarZ);
	int Solid_Upper_noeq, Pore_Lower_noeq;


	//=============== Pattern entropy analysis ====
	void patternentropyanalysis(int templatesize, Mat &exemplar, double &entropy);


	//=============== distance map ===============
	vector<size_color> Solid_Upper, Pore_Upper, Pore_Lower;						//Redistribute DMap. Use same Solid_Upper,Pore_Lower for 3TIs and loaded model
	void binaryChar(vector<short>& DMap, vector<char>& Binarised, short threshold);
	void binaryUchar(vector<short>& DMap, vector<uchar>& Binarised, short threshold);
	void binaryUchar(vector<uchar>& model, short threshold);
	vector<unsigned short> BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg);
	vector<short> GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN);		//calculate Distance Map
	//redistribute TI based on DM, no need to resize to 0-255
	void transformDM(int level, vector<size_color>& exemplar1, vector<size_color>& exemplar2, vector<size_color>& exemplar3);
	void transformDM(int level, vector<short>& exemplar1, vector<short>& exemplar2, vector<short>& exemplar3);


	// 3D Model
	string modelFilename3D;
	//vector<uchar> load3Dmodel(const char* filename);
	bool loadVolume();
	vector<vector<size_color>> m_volume;		// synthesized volume				//[level][idx3d] = color	//can be short, others can also use unsignedint_16
	void outputmodel(int level);

	void cropmodel(int level, int cropl, vector<uchar>&model);

	void InitRandomVolume(int level);

	// assign fixed layer

	//void AssignFixedLayer(int level, int dir);
	
	// random permutation (precomputed)
	vector<size_idx>	 m_permutation;												//[idx3d] = idx3d
	void initPermutation(int level);

	// upsample
	void upsampleVolume(int level);

	//release data
	void cleardata(int level);
	void cleardata();

	// =========== K-coherence search =============
	vector<vector<vector<size_idx>>> KCoherence_x, KCoherence_y, KCoherence_z;		//[level][idx2d][k] = TIindex2d
	void computeKCoherence();


	//=========== phase 1: search ================================
	size_dist TotalDis;
	bool searchVolume(int level, int loop);

	size_dist getFullDistance(int level, vector<size_color>& exemplar, size_idx idx2d, CvMat* dataMat);

	vector<vector<bool>> isUnchanged_x, isUnchanged_y, isUnchanged_z;				//[level][idx3d]=isUnchanged	bool
	bool isUnchangedBlock(int level, int direction, size_idx i, size_idx j, size_idx k);

	size_idx DoPAR::getRandomNearestIndex(int level, vector<size_hiscount>& IndexHis);	//for bad points

	//========== phase 2: optimization ===========================
	void optimizeVolume(int level, int loop);

	bool FIRSTRUN = true;
	
	//============== index histogram ============
	vector<vector<size_hiscount>> IndexHis_x, IndexHis_y, IndexHis_z;//sparse grid!	//[level][idx2d/4]=IndexHis		 //3TI different IndexHis
	vector<vector<size_idx>> nearestIdx_x, nearestIdx_y, nearestIdx_z;				//[level][idx3d]=nearestIdx2d
	vector<vector<size_dist>> nearestWeight_x, nearestWeight_y, nearestWeight_z;	//[level][idx3d]=nearestWeight	eudis^-0.6 or eudis^-1

	bool setNearestIndex(int level, vector<size_idx>& nearestIdx, vector<size_dist>& nearestWeight, vector<size_hiscount>&IndexHis,
		size_idx idx3d, size_idx newNearestIdx, size_dist dis);


	//=========== position histogram =============
	vector<vector<size_hiscount>> PosHis;											//[level][idx2d*3]=IndexHis		// no sparse grid
	vector<vector<size_idx>> SelectedPos;											//[level][idx3d]=idx2d (TIsize*3)
	vector<vector<size_idx>> Origin_x, Origin_y, Origin_z;							//[level][idx3d]=OriginTIidx2d

	//void updatePosHis(int level, vector<size_hiscount>& PosHis, vector<size_idx>& selectedPos, size_idx idx3d, size_idx newPos);

	void writeHistogram(int level);

	//============ Color Histogram ===============
	int ColorHis_BinNum = 256;
	vector<vector<size_hiscount>> ColorHis_exemplar;								//[level][BinNum], BinNum is the same for all level
	vector<vector<size_hiscount>> ColorHis_synthesis;

	void initColorHis_synthesis(int level);
};

//for now the total size of last level is: 4*(1+1+0.75+3+6) = 4*12 times volumesize! (e.g. 400^3 raw size = 61Mb, then needs > 61*48 = 2928 Mb)