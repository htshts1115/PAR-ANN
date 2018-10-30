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

	void cleardata();
	//=============================================================
	vector<string> FNameXY, FNameXZ, FNameYZ;
	string workpath, outputpath, outputfilename, parameterstring;
	
	mt19937 mersennetwistergenerator;
	///===============================================


////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int MULTIRES;										// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	vector<size_idx> blockSize;								// template size	//tested: coarse level big for quality, fine level small for speed
	vector<size_idx> TIsize;									// size of input exemplar
	vector<size_idx> OUTsize;								// size of output 
	int outputsizeatlastlevel = 0;

	vector<int> MAXITERATION;							// max iteration time	//tested: fine level does not need many iterations
	int NumRealization;
	
	bool blockSizeconfigYN = false;
	vector<int> tempblockSize;
	bool MAXITERATIONconfigYN = false;
	vector<int> tempMAXITERATION;

	bool outputmultilevelYN = false;
	bool outputiterationYN = false;

	bool MultipleTIsYN = false;									//! if true, will try to use TIs with same prefix filename
	int MultiTIsNum=1;									// how many TIs found(use)

	int COHERENCENUM = 9;								// K-coherence 11
	vector<double> ANNerror;
	bool useRandomSeed;									// Use random seed or fixed (0) for test (false)
	
	const int GRID = 2;									// sparse grid
	const size_dist min_dist = 1e-3f;	
	const size_dist max_dist = 10e7f;
	bool SIM2D_YN = false;								//2d reconstruction

	bool cropYN = true;									// first enlarge model to avoid toroidal, when output crop

	bool HisEqYN = false;								// apply histogram equalization
	bool DMtransformYN = true;							// use DM transformation
	bool PrintDMYN = false;							// generate DM transformed TI
	bool PrintHisYN = false;							// generate Histogram
	bool PatternEntropyAnalysisYN = true;				// analyse pattern size entropy
	bool KeepParameterNameYN = true;

	bool testNoDiscrete = false;
	size_dist factorIndex;
	vector<size_dist> indexweight;
	size_dist factorPos;
	vector<size_dist> posweight;
	//size_dist IndexHisManualControl = 1.0f, PosHisManualControl = 1.0f;

	int FixedLayerDir = -1;
	size_dist DirectionalWeight = 0.66; 

	bool ColorHis_ON = true;
	int ColorHis_BinNum = 128;	//max 256
	float colorhis_compressratio = 1.0*ColorHis_BinNum / 256;
	size_dist factorC;
	vector<size_dist> colorweight;

	//size_hiscount poretotal_synthesis, poretotal_required;
	size_dist porosityX, porosityY, porosityZ;
	vector<size_dist> porosity_required;

	vector<size_hiscount> avgIndexHis;						// default average value of IndexHis
	vector<size_hiscount> avgPosHis;						// default average value of PosHis
	

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ================ load 2D TIs ==========
	void initTIbasedparameters(vector<Mat>& XY, vector<Mat>& XZ, vector<Mat>& YZ);
	bool loadExemplar();

	bool batchYN = false;
	int batchsequenceNo, batchsequenceMax;
	vector<string> batchFileList_XY, batchFileList_XZ, batchFileList_YZ;

	vector<vector<vector<size_color> > > TIs_XY, TIs_XZ, TIs_YZ;					//[level][TInum][idx2d] = 0-255 
	
	//vector<vector<size_color> >  m_exemplar_x;								
	//vector<vector<size_color> >  m_exemplar_y;
	//vector<vector<size_color> >  m_exemplar_z;

	//!!control maps
	size_idx trimIndex(int level, size_idx index) {
		//!!for 3d output
		if (index < 0) index += OUTsize[level];
		return index % OUTsize[level];
	}

	//template<typename T>
	//void TIsToRegular(T idx_TIs, T TIsize2d_, T &idx_regular, T &TInum);

// ================ allocation ===========
	void allocateVectors();

	string modelFilename3D;
	bool loadVolume();
	void InitRandomVolume(int level);
	vector<vector<size_color>> m_volume;		// synthesized volume						//[level][idx3d] = uchar
	
	//vector<uchar> load3Dmodel(const char* filename);

	// assign fixed layer
	//void AssignFixedLayer(int level, int dir);

// =============== distance map ===============
	vector<short> Solid_Upper, Pore_Upper, Pore_Lower;						//Redistribute DMap. Use same Solid_Upper,Pore_Lower for 3TIs and loaded model
	//redistribute TI based on DM, no need to resize to 0-255
	void transformDMs(vector<vector<size_color> >& listXY, vector<vector<size_color> >& listXZ, vector<vector<size_color> >& listYZ);
	vector<short> DoPAR::GetDMap_Euclidean(vector<float>& vect, short dimension);

	void invertpaddingDMtransform(vector<Mat>& XY, vector<Mat>& XZ, vector<Mat>& YZ, vector<vector<size_color> >& TIsXY, vector<vector<size_color> >& TIsXZ, vector<vector<size_color> >& TIsYZ);

	void equalizeHistograms(int level, vector<vector<size_color>>& TIsXY, vector<vector<size_color>>& TIsXZ, vector<vector<size_color>>& TIsYZ);
	short Solid_Upper_noeq, Pore_Lower_noeq;

// ================ analysis ===========
	void analyze();

	void DoPAR::computeporosityrequired();
	void patternentropyanalysis(int templatesize, Mat &exemplar, double &entropy);
	void testPCA();		//compare PCA TI


// =========== main procedures =============
	vector<size_idx>	 m_permutation;// random permutation (precomputed)			//[idx3d] = idx3d

	void init();

	void DoANNOptimization(int TIseries);
	
	void upsampleVolume(int level);

// =========== K-coherence search =============
	vector<vector<vector<size_idx>>> KCoherence_x, KCoherence_y, KCoherence_z;		//[level][idx2d_TIs][k] = idx2d_TIs
	void computeKCoherence_MultipleTIs();

	//size_idx convertIndexANN(int level, size_idx index) {
	//	//convert ANNSearch_nearest_x_index to TI index
	//	size_idx i, j, height, bias;
	//	height = TIsize[level] - blockSize[level] + 1;
	//	bias = static_cast<size_idx>(blockSize[level] * 0.5);
	//	i = index / height + bias;
	//	j = index % height + bias;
	//	return (i*TIsize[level] + j);
	//}

// =========== phase 1: search ================================
	bool searchVolume(int level, int loop);

	template<typename T>
	size_dist getFullDistance_TIs(int level, vector<vector<T>>& TIs, size_idx idx2d_TIs, vector<T>& pattern);
	template<typename T>
	size_dist getFullDistance(int level, vector<T>& TI, size_idx idx2d, vector<T>& pattern);
	size_dist getFullDistance(int level, vector<size_color>& exemplar, size_idx idx2d, CvMat * dataMat);

	vector<vector<bool>> isUnchanged_x, isUnchanged_y, isUnchanged_z;				//[level][idx3d]=isUnchanged	bool
	bool isUnchangedBlock(int level, int direction, size_idx i, size_idx j, size_idx k);

	

	size_idx DoPAR::getRandomNearestIndex(int level, vector<size_hiscount>& IndexHis);	//for bad points

// ========== phase 2: optimization ===========================
	void optimizeVolume(int level, int loop);

	bool FIRSTRUN = true;

// ============== index histogram ============
	vector<vector<size_hiscount>> IndexHis_x, IndexHis_y, IndexHis_z;//sparse grid!	//[level][idx2d/4]=IndexHis		 //3TI different IndexHis
	vector<vector<size_idx>> nearestIdx_x, nearestIdx_y, nearestIdx_z;				//[level][idx3d]=nearestIdx2d
	vector<vector<size_dist>> nearestWeight_x, nearestWeight_y, nearestWeight_z;	//[level][idx3d]=nearestWeight	eudis^-0.6 or eudis^-1

	bool setNearestIndex(int level, vector<size_idx>& nearestIdx, vector<size_dist>& nearestWeight, vector<size_hiscount>&IndexHis,
		size_idx idx3d, size_idx newNearestIdx, size_dist dis);

	size_idx sparseIdx_TIs(int level, size_idx index) {
		//index is 2d. IndexHis
		//convert idx to sparsed grid --> width/2!
		size_idx i, j, height, sheight;
		height = TIsize[level];
		sheight = height / GRID;

		index %= (height*height);	//for TIs
		i = (index / height) / GRID;
		j = (index % height) / GRID;
		return (i*sheight + j);
	}

// =========== position histogram =============
	vector<vector<size_hiscount>> PosHis;											//[level][idx2d*3]=IndexHis		// no sparse grid
	vector<vector<size_idx>> SelectedPos;											//[level][idx3d]=idx2d (TIsize*3)
	vector<vector<size_idx>> Origin_x, Origin_y, Origin_z;							//[level][idx3d]=OriginTIidx2d

	//void updatePosHis(int level, vector<size_hiscount>& PosHis, vector<size_idx>& selectedPos, size_idx idx3d, size_idx newPos);

	void writeHistogram(int level);

// ============ Color Histogram ===============

	vector<vector<size_hiscount>> ColorHis_exemplar;								//[level][BinNum], BinNum is the same for all level
	vector<vector<size_hiscount>> ColorHis_synthesis;

	void initColorHis_exemplar();
	void initColorHis_synthesis(int level);


// ================ output ====================

	void outputmodel(int level);

	void crop3Dmodel(int level, int cropl, vector<uchar>&model);

};

//for now the total size of last level is: 4*(1+1+0.75+3+6) = 4*12 times volumesize! (e.g. 400^3 raw size = 61Mb, then needs > 61*48 = 2928 Mb)