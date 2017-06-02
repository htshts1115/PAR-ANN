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
	vector<vector<ANNcoord > > m_volume;	// [M] size: TEXSIZE^3
	void InitRandomVolume(int level);
	void upsampleVolume(int level);
	void outputmodel(int level);
	void DynamicThreshold(int level);
	void writeHistogram(bool scaling, int level, vector<float> &histogram, int rows, int cols, const string filename);

	//release data
	void cleardata(int level);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static const int MULTIRES = 3;				// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	static const int N[MULTIRES];
	static ANNidx TEXSIZE[MULTIRES];			// size of input exemplar
	static int D_NEIGHBOR[MULTIRES];			// (2 * N + 1) * (2 * N + 1)

	static const bool INDEXHIS_ON = true;				// Index Histogram in search step

	static const bool DISCRETE_ON = true;				// discrete solver in optimize step
	
	static const bool COLORHIS_ON = false;				// Colour Histogram in optimize step
	static const bool POSITIONHIS_ON = true;			// Position Histogram	in optimize step

	static const bool BIMODAL_ON = true;				// Using bimodal TI
	static const bool DISTANCEMAP_ON = false;			// convert to distance map model

	static const bool DISCRETETHRESHOLD_ON = false;		// dynamic thresholding in optimize step. 	will slightly affect quality. dont use in double peak distribution
	//Discarded. Wrong or poor performance
	//static const bool PROPORTIONTHRESHOLD_ON = false;	// ProportionThreshold()  not good when DM is not exact DistanceMap
	
	//static const bool ITERATIVEGETDMAP_ON = false;	//convert to 3D DMap every iteration, to correct DM values
	
	//experimental	
	//static const bool EARLYTERMINATION_ON = false;		// terminate ANN search when max points has benn visited. Careful.
	//bool COUTCH_ON = false;
	//static const bool HYBRID_DMGREY_ON = false;		//For finnest resolution use grey image not DM

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
		ANNidx x, y, size;
		size = TEXSIZE[level] - 2 * N[level];
		x = index%size;
		y = index / size;
		return ((y + N[level])*TEXSIZE[level] + (x + N[level]));
	}
	static const float inv_sqrt_2pi;
	inline float gaussian_pdf(float x, float mean, float stddev)
	{		
		float a = (x - mean) / stddev;
		return inv_sqrt_2pi / stddev * exp(-0.5 * a * a);
	}
	static const ANNdist alpha_[MULTIRES];


	// ========== need to change to short ==================
	vector<vector<ANNcoord> >  m_exemplar_x;
	vector<vector<ANNcoord> >  m_exemplar_y;
	vector<vector<ANNcoord> >  m_exemplar_z;						// [M] exemplar RGB image, size: TEXSIZE^2

	bool loadExemplar();
	//void calcNeighbor();

	// =========== K-coherence search ======================
	const int CoherenceNUM = 9;
	void computeKCoherence();
	vector<vector<vector<ANNidx>>> KCoherence_x;					//[level][TIindex][k] = TIindex
	vector<vector<vector<ANNidx>>> KCoherence_y;
	vector<vector<vector<ANNidx>>> KCoherence_z;
	



	void initabsoluteneigh();
	vector<vector<ANNidx>> absoluteneigh;

	// random permutation (precomputed)
	//vector<ANNidx>	 m_permutation_xyz;								// [M] size: TEXSIZE[level]^3
	//void initPermutation(int level);

	// PCA-projected neighborhood vector
	vector<vector<ANNcoord> > m_neighbor_x;							// [M] original neighborhood vector required for texture optimization
	vector<vector<ANNcoord> > m_neighbor_y;
	vector<vector<ANNcoord> > m_neighbor_z;
	//vector<CvMat*> mp_neighbor_pca_average_x;						// [M] average of neighborhood vector
	//vector<CvMat*> mp_neighbor_pca_average_y;
	//vector<CvMat*> mp_neighbor_pca_average_z;
	//vector<CvMat*> mp_neighbor_pca_projected_x;						// [M] PCA-projected neighborhood data
	//vector<CvMat*> mp_neighbor_pca_projected_y;
	//vector<CvMat*> mp_neighbor_pca_projected_z;
	//vector<CvMat*> mp_neighbor_pca_eigenvec_x;						// [M] eigenvectors for covariant matrix
	//vector<CvMat*> mp_neighbor_pca_eigenvec_y;
	//vector<CvMat*> mp_neighbor_pca_eigenvec_z;
	//vector<vector<ANNcoord*> > m_neighbor_kdTree_ptr_x;				// [M] array of pointers to vectors required for ANNkd_tree
	//vector<vector<ANNcoord*> > m_neighbor_kdTree_ptr_y;				//ANNPoint*, 3 level
	//vector<vector<ANNcoord*> > m_neighbor_kdTree_ptr_z;


	// pseudocode-----------------------------------------------------------------
	// volume[0] := initVolume(0);                                        % initialization
	// for level = 0 to L                                                 % coarse-to-fine synthesis
	//   repeat                                                           % several iterations for a single level
	//     nearest_neighbor := searchVolume(level);                       % phase 1: search
	//     volume[level] := optimizeVolume(level, nearest_neighbor);      % phase 2: optimization
	//   until converged
	//   volume[level + 1] = upsampleVolume(level);                       % upsampling
	// end for
	//----------------------------------------------------------------------------

	//check convergence
	ANNdist perpixel_energy_new, perpixel_energy_old;
	static const short MAXITERATION;				//max iteration time

	//=========== phase 1: search ===========================
	//const double PCA_RATIO_VARIANCE = 0.999;					//0.95
	//const double ErrorBound = 0.0;							//Kopf used 2.0
	//const int ANNsearchk = 9;								//search k nearest index
	//vector<vector<ANNidx> > m_volume_nearest_x_index;		// [M] size: TEXSIZE^3
	//vector<vector<ANNidx> > m_volume_nearest_y_index;		// [M] size: TEXSIZE^3
	//vector<vector<ANNidx> > m_volume_nearest_z_index;		// [M] size: TEXSIZE^3
	//vector<vector<ANNdist> > m_volume_nearest_x_dist;		// [M] size: TEXSIZE^3
	//vector<vector<ANNdist> > m_volume_nearest_y_dist;		// [M] size: TEXSIZE^3
	//vector<vector<ANNdist> > m_volume_nearest_z_dist;		// [M] size: TEXSIZE^3
	//vector<vector<ANNdist> > m_volume_weight_x;
	//vector<vector<ANNdist> > m_volume_weight_y;
	//vector<vector<ANNdist> > m_volume_weight_z;

	//void searchVolume(int level);


	// ----------- index histogram -------------
	static vector<float> delta_histogram_synthesis, delta_histogram_exemplar;		//store the value when first initial histogram
	static float perHisBin;													//store NUM_HISTOGRAM_BIN / CHANNEL_MAXVALUE
	vector<vector<float> >  m_indexhistogram_synthesis;
	void initIndexHistogram();
	void updateIndexHistogram(int level, const ANNidx oldannidx, const ANNidx newannidx);
	//int indexhistmatching_ann_index(int level, int orientation, ANNidxArray& idxarray);
	bool FIRSTRUN;					// dont use random initial histogram. start counting from 0 for the first run.


	//========== phase 2: optimization ======================
	//void optimizeVolume(int level);

	//static vector<float> HisStdDev;
	// ===========position histogram=============
	vector<vector<float> >  m_positionhistogram_exemplar;						//[level][bin]
	vector<vector<float> >  m_positionhistogram_synthesis;				
	vector<vector<ANNidx> > m_volume_position;		// volume_position record // [M] size: TEXSIZE^3
	void initPositionHistogram_exemplar();
	void initPositionHistogram_synthesis(int level);
	void updatePositionHistogram_synthesis(int level, const ANNidx position_old, const ANNidx position_new);


	//---------- color histogram ---------------	
	static const short NUM_HISTOGRAM_BIN;			// # of histogram bins
	static short CHANNEL_MAXVALUE;	// for color histogram	
	//vector<vector<vector<double> > > dimensional_histogram_exemplar;				// [level][ori][bin]	16
	vector<vector<float> >  m_histogram_exemplar;									// [level][bin]		16
	vector<vector<float> >  m_histogram_synthesis;									// [level][bin]		16

	void calcTempHistogram(vector<ANNcoord>& model, vector<short>& existedbin, vector<float>& existedbinHis);
	void initHistogram_exemplar();
	void initHistogram_synthesis(int level);
	void updateHistogram_synthesis(int level, const ANNcoord color_old, const ANNcoord color_new);	
	
	//discrete solver
	ANNcoord DoPAR::FindClosestColor(int level, vector<ANNcoord> &color, ANNcoord referencecolor);
	ANNidx FindClosestIndex(int level, vector<ANNcoord>& color, vector<ANNidx>& position, ANNcoord referencecolor);
	

	//----------- Dynamic thresholding ----------
	const static short DISCRETE_HISTOGRAM_BIN;						// for thresholding, discrete values. e.g. default256
	vector<vector<float> >  discrete_histogram_exemplar;			// [level][discretebin]	256
	vector<vector<float> >  discrete_histogram_synthesis;			// [level][discretebin]	256
	vector<vector<short> > existed_bin_exemplar;						//[level][<=max bin size]
	vector<vector<float>> existed_histogram_examplar;				//[level][<=max bin size]
	vector<vector<float>> discrete_acchis_exemplar;							//[level][bin]

	void DynamicThresholding(int level);//reassign values based on TI colorhis after optimize step
	void calcaccHistogram(vector<float> &inputhis, vector<float> &acchis);
	////Non-linear solver
	//void PolynomialInterpolation(vector<double>& Xv, vector<double>& Yv, vector<double>& X);
	void ProportionThreshold(vector<short>& Model, vector<short> BinNum, vector<float> Prob);
	

	//=============== distance map ===================
	double porosityTI, porosityModel;
	static float Solid_Upper;			//Redistribute DMap Model. Use same Solid_Upper,Pore_Lower for 3TIs and loaded model
	static float Pore_Lower;
	static short DistanceThreshold;		//Binarise DM, use the same threshold.
	static vector<short> ProjectDMapMaxBins;
	bool GenerateDMTI = false;

	vector<unsigned short> BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg);
	vector<short> GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN);
	vector<char> BinariseImg(vector<short>& DMap, double TPorosity);
	void BinariseThreshold(vector<short>& DMap, vector<char>& Binarised, short threshold);
	void PrepareDMapProjection(vector<short>& TI1, vector<short>& TI2, vector<short>& TI3, int level);
	void ProjectDMap(vector<short>& DMap, int level);
	void NooutputDM(vector<short>& TI1, vector<short>& TI2, vector<short>& TI3);
	
	//=========== Bimodal Transform ===============
	void BimodalRedistribution(vector<float>& Res, string filename);
	void BimodalRedistribution3D(vector<float>& Res, string filename);
	void testBimodalRedistribution(vector<float>& Res, string filename);

};

