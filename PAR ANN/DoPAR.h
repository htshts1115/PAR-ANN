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
	///========================== optimization based =====================

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static const int MULTIRES = 3;						// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	static const int blockSize[MULTIRES];				// template size
	static ANNidx TEXSIZE[MULTIRES];					// size of input exemplar
	const ANNidx GRID = 2;								// sparse grid
	const int COHERENCENUM = 9;							// K-coherence
	static const ANNdist factor[MULTIRES];				// linear weighting factor
	static ANNdist deltaIndexHis[MULTIRES];				// update IndexHis value per operation
	static ANNdist deltaPosHis[MULTIRES];				// update PosHis value per operation
	static const short MAXITERATION[MULTIRES];			// max iteration time

	static const bool INDEXHIS_ON = true;				// Index Histogram in search step	
	static const bool POSITIONHIS_ON = true;			// Position Histogram	in optimize step

	//static const bool BIMODAL_ON = false;				// Using bimodal TI
	//static const bool DISTANCEMAP_ON = false;			// convert to distance map model
	//static const bool DISCRETETHRESHOLD_ON = false;	// dynamic thresholding in optimize step. 	will slightly affect quality. dont use in double peak distribution

////////////////////////////////////////////////////////////////////////////////////////////////////////////

	string modelFilename3D;								
	vector<uchar> load3Dmodel(const char* filename);
	bool loadVolume();
	vector<vector<ANNcoord>> m_volume;		// synthesized volume				//[level][idx3d] = color	//can be short, others can also use unsignedint_16
	void InitRandomVolume(int level);
	void upsampleVolume(int level);
	void outputmodel(int level);
	void writeHistogram(bool scaling, int level, vector<float> &histogram, int rows, int cols, const string filename);

	//release data
	void cleardata(int level);

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
		ANNidx i, j, height, bias;
		height = TEXSIZE[level] - blockSize[level] + 1;
		bias = static_cast<ANNidx>(blockSize[level] / 2);
		i = index / height;
		j = index % height;
		return ((i + bias)*TEXSIZE[level] + (j + bias));
	}
	inline ANNidx sparseIdx(int level, ANNidx index) {
		//convert idx to sparsed grid
		ANNidx i, j, height;
		height = TEXSIZE[level];
		i = (index / height) / GRID;
		j = (index % height) / GRID;
		return (i*height + j);
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
	vector<vector<ANNcoord> >  m_exemplar_z;									
	bool loadExemplar();

	// random permutation (precomputed)
	vector<ANNidx>	 m_permutation_xyz;											//[level][idx3d] = idx3d
	void initPermutation(int level);

	// =========== K-coherence search ======================
	vector<vector<vector<ANNidx>>> KCoherence_x, KCoherence_y, KCoherence_z;	//[level][idx2d][k] = TIindex2d
	void computeKCoherence();


	//=========== phase 1: search ===========================
	bool searchVolume(int level);

	ANNdist getFullDistance(int level, vector<ANNcoord>& exemplar, ANNidx idx2d, CvMat* dataMat);

	vector<vector<bool>> isUnchanged_x, isUnchanged_y, isUnchanged_z;			//[level][idx3d]=isUnchanged	bool
	bool isUnchangedBlock(int level, int direction, ANNidx i, ANNidx j, ANNidx k);


	//========== phase 2: optimization ======================
	void optimizeVolume(int level);


	//============== index histogram ============
	vector<vector<ANNdist>> IndexHis_x, IndexHis_y, IndexHis_z;					//[level][idx2d]=IndexHis		//3TI different IndexHis, sparse grid
	vector<vector<ANNidx>> nearestIdx_x, nearestIdx_y, nearestIdx_z;			//[level][idx3d]=nearestIdx2d
	vector<vector<ANNdist>> nearestWeight_x, nearestWeight_y, nearestWeight_z;	//[level][idx3d]=nearestWeight	eudis^-0.6

	void initIndexHis();

	bool setNearestIndex(int level, vector<ANNidx>& nearestIdx, vector<ANNdist>& nearestWeight, vector<ANNdist>&IndexHis,
		ANNidx idx3d, ANNidx newNearestIdx, ANNdist dis);

	//void getNearestIndex(vector<ANNidx>& nearestIdx, vector<ANNdist>& nearestWeight,
	//	ANNidx idx3d, ANNidx& nearestTIIdx, ANNdist& weight);


	//=========== position histogram =============
	vector<vector<ANNdist>> PosHis;												//[level][idx2d]=IndexHis		//3TI share same PosHis, no sparse grid
	vector<vector<ANNidx>> SelectedPos;											//[level][idx3d]=idx2d
	vector<vector<ANNidx>> Origin_x, Origin_y, Origin_z;						//[level][idx3d]=OriginTIidx2d

	void initPosHis();

	//void getOrigin(vector<ANNidx>& origin, ANNidx idx3d, ANNidx& originx, ANNidx& originy);
	//void setOrigin(vector<ANNidx>& origin, vector<bool>& isUnchanged, ANNidx idx3d, ANNidx tiIdx);

	void updatePosHis(int level, vector<ANNdist>& PosHis, vector<ANNidx>& selectedPos, ANNidx idx3d, ANNidx newPos);


};

//for now the total size of last level is: 4*(1+1+0.75+3+6) = 4*12 times volumesize!