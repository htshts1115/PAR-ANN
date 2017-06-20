#pragma once

#include "stdafx.h"

typedef struct { long x, y, z; } _XyzStrType;		//BarDMap()

typedef float size_color;
typedef long size_idx;
typedef float size_dist;

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
	bool Write(const string FPathName, vector<uchar> Data);
	bool iFileExistYN(const string& PFileName);		
	void ReadRunPar(string CurExeFile); //Read running parameters
	//=============================================================
	string FNameXY, FNameXZ, FNameYZ;
	string workpath, outputpath, outputfilename, parameterstring;
	vector<string> FNameAddition;  //file path names of the training images
	//double PorosityX, PorosityY, PorosityZ, PorosityM; //Original porosities from 3 training images
	
	random_device randomseed;
	mt19937 mersennetwistergenerator;
	uniform_real_distribution<double> probabilitydistribution;

	void showMat(const cv::String& winname, const cv::Mat& mat);
	///========================== optimization based =====================

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static const int MULTIRES = 4;						// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	static const int blockSize[MULTIRES];				// template size
	static size_idx TEXSIZE[MULTIRES];					// size of input exemplar
	const size_idx GRID = 2;							// sparse grid
	const int COHERENCENUM = 9;							// K-coherence
	static const int MAXITERATION[MULTIRES];			// max iteration time
	const bool DISTANCEMAP_ON = true;					// convert to distance map model

	const bool useRandomSeed = true;					// Use random seed or fixed (0) for test

	const size_dist min_dist = 0.0001f;
	size_dist factorIndex[MULTIRES];					// linear weighting factor
	size_dist factorPos[MULTIRES];
	size_dist deltaIndexHis[MULTIRES];					// update IndexHis value per operation
	size_dist deltaPosHis[MULTIRES];					// update PosHis value per operation
	size_dist avgIndexHis[MULTIRES];					// default average value of IndexHis
	size_dist avgPosHis[MULTIRES];						// default average value of PosHis
	
	const bool GenerateDMTI = false;					// generate DM transformed TI

////////////////////////////////////////////////////////////////////////////////////////////////////////////

	inline size_idx trimIndex(int level, size_idx index, bool isToroidal = true) {
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
	inline size_idx convertIndexANN(int level, size_idx index){
		//convert ANNSearch m_volume_nearest_x_index to TI index
		size_idx i, j, height, bias;
		height = TEXSIZE[level] - blockSize[level] + 1;
		bias = static_cast<size_idx>(blockSize[level] / 2);
		i = index / height + bias;
		j = index % height + bias;
		return (i*TEXSIZE[level] + j);
	}
	inline size_idx sparseIdx(int level, size_idx index) {
		//convert idx to sparsed grid --> width/2!
		size_idx i, j, height, sheight;
		height = TEXSIZE[level];
		sheight = height / 2;
		i = (index / height) / GRID;
		j = (index % height) / GRID;
		return (i*sheight + j);
	}
	static const size_dist inv_sqrt_2pi;
	const size_dist pdfdevS = 0.05f;
	const size_dist pdfdevO = 0.05f/3.0f;
	inline size_dist gaussian_pdf(size_dist x, size_dist dev /*, size_dist mean = 0.0f*/){		
		return exp(-0.5f * (x * x / dev / dev));
	}
	//const size_dist	gwdev = 1.0f / 6.0f;
	//inline size_dist normal_cdf(size_dist x, size_dist stddev, size_dist mean = 0.0f){
	//	if (x == 0.0f) return 1.0f;
	//	size_dist tx = (x - mean) / (stddev * sqrt(2.0f));
	//	size_dist y = 1.0f / (1.0f + 0.3275911f * tx);
	//	size_dist erf = 1.0f - (((((
	//		+1.061405429f  * y
	//		- 1.453152027f) * y
	//		+ 1.421413741f) * y
	//		- 0.284496736f) * y
	//		+ 0.254829592f) * y)
	//		* exp(-tx * tx);
	//	size_dist cdf = (1.0f - 0.5f * (1.0f + erf)) * 2.0f;	// =/0.5f
	//	if (cdf < FLT_MIN) cdf = FLT_MIN;
	//	return cdf;
	//}


	void DoANNOptimization();
	
	void init();

	void allocateVectors(int level);

	// 2D Exemplar 
	vector<vector<size_color> >  m_exemplar_x;									//[level][idx2d] = color
	vector<vector<size_color> >  m_exemplar_y;
	vector<vector<size_color> >  m_exemplar_z;
	bool loadExemplar();
	void gaussImage(int level, vector<vector<size_color>>& exemplar);

	//=============== distance map ===============
	size_color Solid_Upper, Pore_Upper;						//Redistribute DMap. Use same Solid_Upper,Pore_Lower for 3TIs and loaded model
	void binaryChar(vector<short>& DMap, vector<char>& Binarised, short threshold);
	void binaryUchar(vector<short>& DMap, vector<uchar>& Binarised, short threshold);
	vector<unsigned short> BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg);
	vector<short> GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN);		//calculate Distance Map
	//redistribute TI based on DM, no need to resize to 0-255
	void transformDM(vector<size_color>& exemplar1, vector<size_color>& exemplar2, vector<size_color>& exemplar3);


	// 3D Model
	string modelFilename3D;
	vector<uchar> load3Dmodel(const char* filename);
	bool loadVolume();
	vector<vector<size_color>> m_volume;		// synthesized volume				//[level][idx3d] = color	//can be short, others can also use unsignedint_16
	void outputmodel(int level);

	void InitRandomVolume(int level);
	
	// random permutation (precomputed)
	vector<size_idx>	 m_permutation;												//[idx3d] = idx3d
	void initPermutation(int level);

	// upsample
	void upsampleVolume(int level);

	//release data
	void cleardata(int level);

	// =========== K-coherence search =============
	vector<vector<vector<size_idx>>> KCoherence_x, KCoherence_y, KCoherence_z;		//[level][idx2d][k] = TIindex2d
	void computeKCoherence();


	//=========== phase 1: search ================================
	bool searchVolume(int level);

	size_dist getFullDistance(int level, vector<size_color>& exemplar, size_idx idx2d, CvMat* dataMat);

	vector<vector<bool>> isUnchanged_x, isUnchanged_y, isUnchanged_z;				//[level][idx3d]=isUnchanged	bool
	bool isUnchangedBlock(int level, int direction, size_idx i, size_idx j, size_idx k);

	size_idx DoPAR::getRandomNearestIndex(int level, vector<size_dist>& IndexHis);	//for bad points

	//========== phase 2: optimization ===========================
	void optimizeVolume(int level);


	//============== index histogram ============
	vector<vector<size_dist>> IndexHis_x, IndexHis_y, IndexHis_z;					//[level][idx2d/4]=IndexHis		// sparse grid! //3TI different IndexHis
	vector<vector<size_idx>> nearestIdx_x, nearestIdx_y, nearestIdx_z;				//[level][idx3d]=nearestIdx2d
	vector<vector<size_dist>> nearestWeight_x, nearestWeight_y, nearestWeight_z;	//[level][idx3d]=nearestWeight	eudis^-0.6 or eudis^-1

	bool setNearestIndex(int level, vector<size_idx>& nearestIdx, vector<size_dist>& nearestWeight, vector<size_dist>&IndexHis,
		size_idx idx3d, size_idx newNearestIdx, size_dist dis);


	//=========== position histogram =============
	vector<vector<size_dist>> PosHis;												//[level][idx2d*3]=IndexHis		// no sparse grid
	vector<vector<size_idx>> SelectedPos;											//[level][idx3d]=idx2d (TIsize*3)
	vector<vector<size_idx>> Origin_x, Origin_y, Origin_z;							//[level][idx3d]=OriginTIidx2d

	void updatePosHis(int level, vector<size_dist>& PosHis, vector<size_idx>& selectedPos, size_idx idx3d, size_idx newPos);

	void writeHistogram(int level);


};

//for now the total size of last level is: 4*(1+1+0.75+3+6) = 4*12 times volumesize!