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
	string workpath, outputpath, outputfilename;
	vector<string> FNameAddition;  //file path names of the training images
	//vector<uchar> Model;
	//double PorosityXY, PorosityXZ, PorosityYZ, PorosityM; //Original porosities from 3 training images

	//void ReadPBMImage(string FName, char DirID, double UpPro); //Read each training image
	////DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
	////       otherwise a single image for three direction 
	////UpPro: upper porosity, 0.5 for default. 
	
	

	//vector<uchar> XY2DImg, XZ2DImg, YZ2DImg; // 1 - pore, 0 - grain
	////@@@@@@@10/05/2016@@@@@@@   multiple TI  
	//vector<vector<uchar>> TIs;
	//int XYSx, XYSy, XZSx, XZSz, YZSy, YZSz;
	//int PARx, PARy, PARz; //Resultant model
	//int TIx, TIy; 
	//long PARxy;
	//bool Identical3DYN;  //wether or not three training images are identical
	

	std::random_device randomseed;
	std::mt19937 mersennetwistergenerator;
	std::uniform_real_distribution<double> probabilitydistribution;

	void showMat(const cv::String& winname, const cv::Mat& mat);
	///========================== 190217 Kopf. optimization based =====================

	string modelFilename3D;							//load 3D model as initial
	vector<uchar> load3Dmodel(const char* filename);
	bool loadVolume();
	// synthesized volume
	std::vector<std::vector<double > > m_volume;	// [M] size: NUM_CHANNEL * TEXSIZE^3
	void InitRandomVolume(int level);
	void upsampleVolume(int level);
	void outputmodel(int level);

	static const int MULTIRES = 1;			// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	static const int N[MULTIRES];
	static int TEXSIZE[MULTIRES];			// size of input exemplar
	static int D_NEIGHBOR[MULTIRES];		// dimension of neighborhood (:= 3 * (2 * N + 1)^2)
	static int NEIGHBORSIZE[MULTIRES];		// size: (2 * N + 1) * (2 * N + 1)
	static int NUM_CHANNEL;						// # of channels (RGB, feature dist., RTF)

	static const bool INDEXHIS_ON = true;				// Index Histogram in search step
	static const bool COLORHIS_ON = false;				// Colour Histogram in optimize step
	
	static const bool DISTANCEMAP_ON = false;			// convert to distance map model
	static const bool PROPORTIONTHRESHOLD_ON = true;	// ProportionThreshold() 

	static const bool DISCRETETHRESHOLD_ON = false;		// dynamic thresholding in optimize step
	static const bool POSITIONHIS_ON = false;			// Position Histogram	in optimize step
	static const bool DISCRETE_ON = false;				// discrete solver in optimize step
	static const bool GAUSSIANFALLOFF_ON = false;		// gaussian fall off weight in optimize step
	


	void DoANNOptimization();
	void init();
	void initthreshold();

	inline ANNidx trimIndex(int level, ANNidx index, bool isToroidal = FALSE) {
		if (isToroidal) {
			while (index < 0) index += TEXSIZE[level];
			return index % TEXSIZE[level];
		}
		else {//mirror
			while (true) {
				if (index < 0) index = -index;
				if (TEXSIZE[level] <= index) {
					index = 2 * (TEXSIZE[level] - 1) - index;
					continue;
				}
				break;
			}
			return index;
		}
	}
	inline ANNidx convertIndexANN(int level, ANNidx index){
		//convert ANNSearch m_volume_nearest_x_index to m_volume index
		int x, y, size;
		size = TEXSIZE[level] - 2 * N[level];
		x = index%size;
		y = index / size;
		return ((y + N[level])*TEXSIZE[level] + (x + N[level]));
	}
	inline double gaussian_pdf(double x, double mean, double stddev)
	{
		static const double inv_sqrt_2pi = 0.398942280401432677939946;
		double a = (x - mean) / stddev;
		return inv_sqrt_2pi / stddev * std::exp(-0.5 * a * a);
	}

	std::vector<std::vector<double> >  m_exemplar_x;		// ========== need to change to short ==================
	std::vector<std::vector<double> >  m_exemplar_y;		
	std::vector<std::vector<double> >  m_exemplar_z;		// [M] exemplar RGB image, size: NUM_CHANNEL * TEXSIZE^2
	bool loadExemplar();
	void calcNeighbor();
	void initabsoluteneigh();
	vector<vector<ANNidx>> absoluteneigh;

	// PCA-projected neighborhood vector
	std::vector<std::vector<double> > m_neighbor_x;				// [M] original neighborhood vector required for texture optimization
	std::vector<std::vector<double> > m_neighbor_y;
	std::vector<std::vector<double> > m_neighbor_z;
	std::vector<CvMat*> mp_neighbor_pca_average_x;						// [M] average of neighborhood vector
	std::vector<CvMat*> mp_neighbor_pca_average_y;
	std::vector<CvMat*> mp_neighbor_pca_average_z;
	std::vector<CvMat*> mp_neighbor_pca_projected_x;						// [M] PCA-projected neighborhood data
	std::vector<CvMat*> mp_neighbor_pca_projected_y;
	std::vector<CvMat*> mp_neighbor_pca_projected_z;
	std::vector<CvMat*> mp_neighbor_pca_eigenvec_x;						// [M] eigenvectors for covariant matrix
	std::vector<CvMat*> mp_neighbor_pca_eigenvec_y;
	std::vector<CvMat*> mp_neighbor_pca_eigenvec_z;
	std::vector<std::vector<double*> > m_neighbor_kdTree_ptr_x;			// [M] array of pointers to vectors required for ANNkd_tree
	std::vector<std::vector<double*> > m_neighbor_kdTree_ptr_y;			//ANNPoint*, 3 level
	std::vector<std::vector<double*> > m_neighbor_kdTree_ptr_z;
	std::vector<ANNkd_tree*          > mp_neighbor_kdTree_x;	// [M] ANN kdTree
	std::vector<ANNkd_tree*          > mp_neighbor_kdTree_y;	// ANNkd_tree, 3 level
	std::vector<ANNkd_tree*          > mp_neighbor_kdTree_z;

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
	double globalenergy_new, globalenergy_old;
	std::vector<int> MAXITERATION;				//max iteration time

	//=========== phase 1: search ===========================
	static const double PCA_RATIO_VARIANCE;		//0.95
	static const double ErrorBound;				//Kopf used 2.0
	static const int ANNsearchk;			//search k nearest index
	std::vector<std::vector<ANNidx> > m_volume_nearest_x_index;		// [M] size: TEXSIZE^3
	std::vector<std::vector<ANNidx> > m_volume_nearest_y_index;		// [M] size: TEXSIZE^3
	std::vector<std::vector<ANNidx> > m_volume_nearest_z_index;		// [M] size: TEXSIZE^3
	std::vector<std::vector<double> > m_volume_nearest_x_dist;		// [M] size: TEXSIZE^3
	std::vector<std::vector<double> > m_volume_nearest_y_dist;		// [M] size: TEXSIZE^3
	std::vector<std::vector<double> > m_volume_nearest_z_dist;		// [M] size: TEXSIZE^3
	void searchVolume(int level);

	// ----------- index histogram -------------
	std::vector<std::vector<double> >  m_indexhistogram_exemplar;
	std::vector<std::vector<double> >  m_indexhistogram_synthesis;
	void initIndexHistogram(int level);
	void updateIndexHistogram(int level, int orientation, const ANNidx oldannidx, const ANNidx newannidx);
	ANNidx indexhistmatching_ann_index(int level, int orientation, ANNidxArray idxarray, ANNdistArray distarry);
	bool FIRSTRUN = true;					// dont use random initial histogram. start counting from 0 for the first run.


	//========== phase 2: optimization ======================
	void optimizeVolume(int level);
	// random permutation (precomputed)
	std::vector<std::vector<ANNidx> > m_permutation_xyz;				// [M] size: TEXSIZE^3
	void initPermutation(int level);
	// Gaussian fall-off function
	static const double gaussiansigma;
	std::vector<std::vector<double> > gaussiankernel;					// m level gaussian kernel
	void InitGaussianKernel();

	//---------- color histogram ---------------	
	static const int NUM_HISTOGRAM_BIN;			// # of histogram bins
	static std::vector<int> CHANNEL_MAXVALUE;	// for color histogram	
	//double WEIGHT_HISTOGRAM;					// linear weight for histogram
	vector<vector<vector<double> > > dimensional_histogram_exemplar;				// [level][ori][bin]	16
	//std::vector<std::vector<std::vector<double> > > m_histogram_exemplar;			// [level][ch][bin]		16
	vector<vector<vector<double> > > m_histogram_synthesis;							// [level][ch][bin]		16

	void calcHistogram_exemplar(int level);
	void calcHistogram_synthesis(int level);
	void updateHistogram_synthesis(int level, const std::vector<double>& color_old, const std::vector<double>& color_new);
	
	//---------- position histogram ------------
	vector<double> WEIGHT_POSITIONHISTOGRAM;
	std::vector<std::vector<double> >  m_positionhistogram_exemplar;				//  multires * binsize(exemplar area)
	std::vector<std::vector<double> >  m_positionhistogram_synthesis;				//  multires * binsize(exmeplar area)
	std::vector<std::vector<ANNidx> > m_volume_position;		// volume_position record // [M] size: TEXSIZE^3
	
	void initPositionHistogram_exemplar(int level);
	void initPositionHistogram_synthesis(int level);
	void updatePositionHistogram_synthesis(int level, const ANNidx position_old, const ANNidx position_new);
	void writeHistogram(int level, vector<double> &histogram, int rows, int cols, const string filename);
	//discrete solver
	int FindClosestColorIndex(int level, vector<double>& colorset, vector<double>& weightset, double referencecolor);	// return the index in colorset of the most similar color	
	
	//----------- Dynamic thresholding ----------
	static const int DISCRETE_HISTOGRAM_BIN;						// for thresholding, discrete values. e.g. default256
	vector<vector<double> >  discrete_histogram_exemplar;			// [level][discretebin]	256
	vector<vector<double> >  discrete_histogram_synthesis;			// [level][discretebin]	256
	vector<vector<short> > existedbin_exemplar;						//[level][<=max bin size]
	vector<vector<double>> existed_histogram_examplar;				//[level][<=max bin size]
	vector<vector<double>> discrete_acchis_exemplar;							//[level][bin]

	void DynamicThresholding(int level);//reassign values based on TI colorhis after optimize step
	void calcaccHistogram(vector<double> &inputhis, vector<double> &acchis);
	void ProportionThreshold(vector<short>& Model, vector<short>& BinNum, vector<double>& Prob);

	//Non-linear solver
	void PolynomialInterpolation(vector<double>& Xv, vector<double>& Yv, vector<double>& X);

	//=============== distance map ===================
	double porosityTI, porosityModel;
	short DistanceThreshold;
	vector<unsigned short> BarDMap(short tSx, short tSy, short tSz, vector<char>& OImg);
	vector<short> GetDMap(short Sx, short Sy, short Sz, vector<char>& OImg, char DM_Type, bool DisValYN);
	vector<char> BinariseImg(vector<short>& DMap, double TPorosity);
	void BinariseThreshold(vector<short>& DMap, vector<char>& Binarised, short threshold);
	void RedistributeDMap(vector<short>& DMap);

	//release data
	void cleardata(int level);
};

