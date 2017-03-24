#pragma once

#include "stdafx.h"

typedef struct{
	int x, y, z;
} _ThreeCorr;

typedef struct{
	int x, y;
} _TwoCorr;

typedef struct{ long idx; double weight; } _Filter;

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
	bool Read(const string FPathName, vector<uchar>& Data);
	bool Write(const string FPathName, vector<uchar> Data);
	bool iFileExistYN(const string& PFileName);
	string workpath, outputpath, outputfilename;
	//=============================================================
	void ReadRunPar(string CurExeFile); //Read running parameters
	void ReadPBMImage(string FName, char DirID, double UpPro); //Read each training image
	//DirID: '1' - XY plane, '2' - XZ plane, '3' - YZ plane
	//       otherwise a single image for three direction 
	//UpPro: upper porosity, 0.5 for default. 
	
	double porelppercentage, grainlppercentage;
	double minEAsamplerate;
	long totalsamplenum, Egdesamplecount;
	int edgewidth = 0;
	int maxinterval = 40;
	int patchtemplate = 50, overlap = 10/*, cotemplate = 100*/;
	//double scanprop = 0.05/*, Tweight = 1.0*/;
	//int ConditionCandidate = 50;
	int patchcandidate = 20;
	//=============================================================
	bool Identical3DYN;  //wether or not three training images are identical
	vector<uchar> Model;
	vector<uchar> XY2DImg, XZ2DImg, YZ2DImg; // 1 - pore, 0 - grain
	//@@@@@@@10/05/2016@@@@@@@   multiple TI  
	string FNameXY, FNameXZ, FNameYZ;
	vector<string> FNameAddition;  //file path names of the training images
	vector<vector<uchar>> TIs;
	int XYSx, XYSy, XZSx, XZSz, YZSy, YZSz;
	int PARx, PARy, PARz; //Resultant model
	int TIx, TIy; 
	long PARxy;
	double computeporo(vector<uchar>& model);
	double PorosityXY, PorosityXZ, PorosityYZ, AverPoro; //Original porosities from 3 training images

	//=============================================================
	std::random_device randomseed;
	std::mt19937 mersennetwistergenerator;
	std::uniform_real_distribution<double> probabilitydistribution;

	//=============================================================
	
	vector<Mat> matTIs; 	
	vector<int> TIsID;


	void showMat(const cv::String& winname, const cv::Mat& mat);
	///========================== 190217 Kopf. optimization based =====================

	string modelFilename3D;					//load 3D model as initial
	vector<uchar> load3Dmodel(const char* filename);
	bool loadVolume();
	void InitRandomVolume(int level);

	static const int MULTIRES = 1;			// # of multi-resolution (0 -> MULTIRES - 1 :: coarsest -> finest)
	static const int N[MULTIRES];
	static int TEXSIZE[MULTIRES];			// size of input exemplar
	static int D_NEIGHBOR[MULTIRES];		// dimension of neighborhood (:= 3 * (2 * N + 1)^2)
	static int NEIGHBORSIZE[MULTIRES];		// size: (2 * N + 1) * (2 * N + 1)
	static const int NUM_HISTOGRAM_BIN;			// # of histogram bins
	static int NUM_CHANNEL;						// # of channels (RGB, feature dist., RTF)
	static std::vector<int> CHANNEL_MAXVALUE;	// for color histogram
	static const double PCA_RATIO_VARIANCE;		//0.95
	static const double ErrorBound;				//Kopf used 2.0
	static const int ANNsearchk;			//search k nearest index
	
	double WEIGHT_HISTOGRAM;					// weight for histogram
	vector<double> WEIGHT_POSITIONHISTOGRAM;

	static const bool POSITIONHIS_ON = true;	// Using Index/Position Histogram
	static const bool COLOURHIS_ON = true;		// Using Colour Histogram


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

	std::vector<std::vector<double> >  m_exemplar_x;		// [M] exemplar RGB image, size: NUM_CHANNEL * TEXSIZE^2
	std::vector<std::vector<double> >  m_exemplar_y;
	std::vector<std::vector<double> >  m_exemplar_z;
	bool loadExemplar();

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
	void calcNeighbor();

	void initabsoluteneigh();
	vector<vector<ANNidx>> absoluteneigh;

	// color histogram
	std::vector<std::vector<std::vector<double> > > m_histogram_exemplar;			// [M] size: NUM_CHANNEL x NUM_HISTOGRAM_BIN
	std::vector<std::vector<std::vector<double> > > m_histogram_synthesis;			// m_histogram[level][ch][bin]
	void calcHistogram_exemplar(int level);
	void calcHistogram_synthesis(int level);
	void updateHistogram_synthesis(int level, const std::vector<double>& color_old, const std::vector<double>& color_new);
	// ===========position histogram=============
	std::vector<std::vector<double> >  m_positionhistogram_exemplar;				//  multires * binsize(exemplar area)
	std::vector<std::vector<double> >  m_positionhistogram_synthesis;				//  multires * binsize(exmeplar area)
	void initPositionHistogram_exemplar(int level);
	void initPositionHistogram_synthesis(int level);
	void updatePositionHistogram_synthesis(int level, const ANNidx position_old, const ANNidx position_new);
	void writeHistogram(int level, vector<double> &histogram, int rows, int cols, const string filename);
	std::vector<std::vector<ANNidx> > m_volume_position;		// volume_position record // [M] size: TEXSIZE^3
	// ===========index histogram ==============
	std::vector<std::vector<double> >  m_indexhistogram_exemplar;
	std::vector<std::vector<double> >  m_indexhistogram_synthesis;
	void initIndexHistogram(int level);
	void updateIndexHistogram(int level, int orientation, const ANNidx oldannidx, const ANNidx newannidx);
	ANNidx indexhistmatching_ann_index(int level, int orientation, ANNidxArray idxarray, ANNdistArray distarry);
	std::vector<std::vector<ANNidx> > m_volume_index_x, m_volume_index_y, m_volume_index_z;		// [M] size: TEXSIZE^3
	// dont use random initial histogram. start counting from 0 for the first run.
	bool FIRSTRUN = true;

	// synthesized volume
	std::vector<std::vector<double > > m_volume;			// [M] size: NUM_CHANNEL * TEXSIZE^3

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
	std::vector<double> valuechange;
	std::vector<double> valuechangethreshold;	//mean value change per voxel
	double globalenergy_new, globalenergy_old;
	std::vector<int> MAXITERATION;				//max iteration time

	// phase 1: nearest neighbor search
	std::vector<std::vector<ANNidx> > m_volume_nearest_x_index;		// [M] size: TEXSIZE^3
	std::vector<std::vector<ANNidx> > m_volume_nearest_y_index;		// [M] size: TEXSIZE^3
	std::vector<std::vector<ANNidx> > m_volume_nearest_z_index;		// [M] size: TEXSIZE^3
	std::vector<std::vector<double> > m_volume_nearest_x_dist;		// [M] size: TEXSIZE^3
	std::vector<std::vector<double> > m_volume_nearest_y_dist;		// [M] size: TEXSIZE^3
	std::vector<std::vector<double> > m_volume_nearest_z_dist;		// [M] size: TEXSIZE^3
	void searchVolume(int level);

	// phase 2: optimization
	void optimizeVolume(int level);
	// random permutation (precomputed)
	std::vector<std::vector<ANNidx> > m_permutation_xyz;				// [M] size: TEXSIZE^3
	void initPermutation(int level);
	// Gaussian fall-off function
	static const double gaussiansigma;
	std::vector<std::vector<double> > gaussiankernel;					// m level gaussian kernel
	void InitGaussianKernel();

	//discrete solver
	int FindClosestColorIndex(int level, vector<double>& colorset, vector<double>& weightset, double referencecolor);	// return the index in colorset of the most similar color
	vector<int> PredefinedL0idx, PredefinedL1idx, PredefinedL2idx, PredefinedL3idx;		//[3(ori)*D_NEIGHBOR[level]]

	// upsampling
	void upsampleVolume(int level);

	void outputmodel(int level);
};

