#pragma once

#include "stdafx.h"

typedef struct{
	int x, y, z;
} _ThreeCorr;

typedef struct{
	int x, y;
} _TwoCorr;

typedef struct{ long idx; double weight; } _Filter;

typedef struct{ long x, y, z; } _XyzStrType;

//typedef class Graph<int, int, int> GraphType;

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
	void GetStarted(string CurWorkExeFile);
private:
	bool ReadTxtFiles(const string PFName, vector<string>& ResLines);
	bool GetNextRowParameters(short Cno, vector<string>& ValidParStr, vector<string>& ParV);
	long FileLength(const string& FName);
	//bool Read(const string FPathName, vector<uchar>& Data);
	bool Write(const string FPathName, vector<uchar> Data);
	bool iFileExistYN(const string& PFileName);		
	void ReadRunPar(string CurExeFile); //Read running parameters
	//=============================================================
	string FNameXY, FNameXZ, FNameYZ;
	string workpath, outputpath, outputfilename, parameterstring;
	vector<string> FNameAddition;  //file path names of the training images
	//vector<uchar> Model;
	double PorosityX, PorosityY, PorosityZ, PorosityM; //Original porosities from 3 training images

	//void ReadPBMImage(string FName, char DirID, double UpPro); //Read each training image
	////DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
	////       otherwise a single image for three direction 
	////UpPro: upper porosity, 0.5 for default. 
	
	random_device randomseed;
	mt19937 mersennetwistergenerator;
	uniform_real_distribution<double> probabilitydistribution;

	void showMat(const cv::String& winname, const cv::Mat& mat);
	///========================== 190217 Kopf. optimization based =====================

	string modelFilename3D;							//load 3D model as initial
	vector<uchar> load3Dmodel(const char* filename);
	bool loadVolume();
	// synthesized volume
	vector<vector<ANNcoord > > m_volume;			// [M] size: TEXSIZE^3
	void InitRandomVolume(int level);
	void upsampleVolume(int level);
	void outputmodel(int level);
	void writeHistogram(bool scaling, int level, vector<float> &histogram, int rows, int cols, const string filename);

	//release data
	void cleardata(int level);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static const int MULTIRES = 3;						// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	static const int blockSize[MULTIRES];				// template size
	static ANNidx TEXSIZE[MULTIRES];				// size of input exemplar
	static const ANNdist factor[MULTIRES];				// linear weighting factor
	static const short MAXITERATION[MULTIRES];			// max iteration time
	const ANNidx GRID = 2;								// sparse grid
	const int COHERENCENUM = 9;							// K-coherence

	static const bool INDEXHIS_ON = true;				// Index Histogram in search step	
	static const bool POSITIONHIS_ON = true;			// Position Histogram	in optimize step

	//static const bool BIMODAL_ON = false;				// Using bimodal TI
	//static const bool DISTANCEMAP_ON = false;			// convert to distance map model
	//static const bool DISCRETETHRESHOLD_ON = false;	// dynamic thresholding in optimize step. 	will slightly affect quality. dont use in double peak distribution

////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void DoANNOptimization();
	void init();

	inline ANNidx trimIndex(int level, ANNidx index, bool isToroidal = true) {
		//if (isToroidal) {
			if (index < 0) index += TEXSIZE[level];
			return index % TEXSIZE[level];
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
	}
	inline ANNidx convertIndexANN(int level, ANNidx index){
		//convert ANNSearch m_volume_nearest_x_index to TI index
		ANNidx x, y, height, bias;
		height = TEXSIZE[level] - blockSize[level] + 1;
		bias = static_cast<ANNidx>(blockSize[level] / 2);
		x = index / height;
		y = index % height;
		return ((x + bias)*TEXSIZE[level] + (y + bias));
	}
	static const float inv_sqrt_2pi;
	inline float gaussian_pdf(float x, float mean, float stddev)
	{		
		float a = (x - mean) / stddev;
		return inv_sqrt_2pi / stddev * exp(-0.5 * a * a);
	}


	// exemplar
	vector<vector<ANNcoord> >  m_exemplar_x;
	vector<vector<ANNcoord> >  m_exemplar_y;
	vector<vector<ANNcoord> >  m_exemplar_z;						// [M] exemplar RGB image, size: TEXSIZE^2
	bool loadExemplar();

	// random permutation (precomputed)
	vector<ANNidx>	 m_permutation_xyz;								// [M] size: TEXSIZE[level]^3
	void initPermutation(int level);

	// =========== K-coherence search ======================
	vector<vector<vector<ANNidx>>> KCoherence_x;					//[level][TIindex][k] = TIindex
	vector<vector<vector<ANNidx>>> KCoherence_y;
	vector<vector<vector<ANNidx>>> KCoherence_z;	
	void computeKCoherence();


	//=========== phase 1: search ===========================
	bool searchVolume(int level);

	ANNdist getFullDistance(int level, int direction, ANNidx idx2d, CvMat* dataMat);

	bool isUnchangedBlock(int level, int direction, ANNidx i, ANNidx j, ANNidx k);


	//========== phase 2: optimization ======================
	void optimizeVolume(int level);


	//============== index histogram ============
	vector<vector<ANNdist>> IndexHis_x, IndexHis_y, IndexHis_z;
	
	void initIndexHis();

	bool setNearestIndex(int level, int direction, ANNidx idx3d, ANNidx nearestTIIdx, ANNdist devDis);

	void getNearestIndex(int level, int direction, ANNidx idx3d, ANNidx& nearestTIIdx, ANNdist& devDis);

	//=========== position histogram =============
	vector<vector<ANNdist>> PosHis_x, PosHis_y, PosHis_z;

	void initPosHis();

	void getOrigin(int level, int direction, ANNidx idx3d, ANNidx& originx, ANNidx& originy);

	void setOrigin(int level, int direction, ANNidx idx3d, ANNidx tiIdx);

	void updatePosHis(int level, int direction, ANNidx idx3d, ANNcoord color, ANNidx closestTIidx);


};

