// PAR.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DoPAR.h"

int main(int argc, const char* argv[])
{	

	string CurWorkExeFile = argv[0];

	DoPAR DoItNow;
	DoItNow.GetStarted(CurWorkExeFile);	

	cout << endl << "Finally: Press any key to quit...";

	_getch();

	return 0;
}
//t = omp_get_wtime();
//cout << endl << omp_get_num_procs();
//vector<int> a(1000, 1);
//vector<int> b(1000, 2);
//vector<int> c(1000);
//#pragma omp parallel for
//for (int i = 0; i < 1000; i++) {
//	int& add = b[i];
//#pragma omp atomic
//	//		b[i]+=a[i];	//wrong
//	//		add += a[i];//wrong
//	add += 1;	//right
//
//	c[i] = a[i] + b[i];
//}
//cout << endl << c[0] << " " << c[1] << " " << c[2]; _getch();
//vector<int> a(40, 0);
//vector<int> b(10000, 1);
//#pragma omp parallel for schedule(static)
//#pragma omp parallel for schedule(dynamic)	//too slow in our case
//for (int i = 0; i < a.size(); i++) {
//	for (int j = 0; j < b.size(); j++) {
//		if (i % 8 == 1) break;			//inside loop can use break
//		a[i] += b[j];
//	}
//	printf("%d = %d, from thread %d\n", i, a[i], omp_get_thread_num());		//i,j are private
//}
//_getch();

//vector<float> t1 = { 100.0f, 255.0f, 256.0f, 511.0f };
//vector<uchar> t2 = { 100, 125, 200, 255};
//cout << endl << (int)t2[0] << "," << (int)t2[1] << "," << (int)t2[2] << "," << (int)t2[3];
//transform(t1.begin(), t1.end(), t2.begin(),
//	std::bind2nd(std::multiplies<float>(), 0.5));
//cout << endl << (int)t2[0] << "," << (int)t2[1] << "," << (int)t2[2] << "," << (int)t2[3]; _getch();

//cout << endl << rand(); cin.get(); //without initial seed, rand() will give the same results!

//Toroidal
//cout << endl << (-1 & 127) << " " << (258 & 127) << " " << (20 & 127);	//!!needs to be 127,255... otherwise wrong
////cout << endl << trimIndex(2, -1) << " " << trimIndex(2, 258) << " " << trimIndex(2, 20);	//same
//cin.get();

//cout << endl <<"opencv optimization enabled? " <<useOptimized(); _getch();	//true
//cout << endl << 124999999 * (1.0f / (500 * 500));	//=500 should be 499!
//_getch();

////--------------check address

//vector<int> list = { 0,1,2,3,4,5,6,7,8,9 };
//int* testnei = &list[4];
//int testint = testnei[3];
//int testint2 = list[7];
//cout << endl << "test1=" << testint << " test2=" << testint2;
//_getch();

////---------------check memory usage-----------

//cout << endl << "double=" << sizeof(double) << " float=" << sizeof(float) << " short=" << sizeof(short) << " int=" << sizeof(int) << " long=" << sizeof(long); _getch();
//double 8, float 4, short 2, int 4, long 4;

//for (std::vector<char>::size_type sz = 1;; sz *= 2)
//{
//	std::cerr << "attempting sz = " << sz << '\n';
//	std::vector<char> v(sz);
//}	//crash at 18GB
//_getch();

//int main(int argc, char* argv[])
//{
//	double *x = new double[536870912];
//
//	cout << "memory allocated" << endl;
//
//	for (long int i = 0; i < 536870912; i++)
//	{
//		cout << i << endl;
//		x[i] = 0;
//	}
//
//	delete[] x;
//	return 0;
//}

////--------------ANN sample------------------

////#include <cstdlib>						// C standard library
////#include <cstdio>						// C I/O (for sscanf)
////#include <cstring>						// string manipulation
////#include <fstream>						// file I/O
////#include "ANN/ANN.h"					// ANN declarations
////
////using namespace std;					// make  accessible
////
//////----------------------------------------------------------------------
////// ann_sample
//////
////// This is a simple sample program for the ANN library.	 After compiling,
////// it can be run as follows.
////// 
////// ann_sample [-d dim] [-max mpts] [-nn k] [-e eps] [-df data] [-qf query]
//////
////// where
//////		dim				is the dimension of the space (default = 2)
//////		mpts			maximum number of data points (default = 1000)
//////		k				number of nearest neighbors per query (default 1)
//////		eps				is the error bound (default = 0.0)
//////		data			file containing data points
//////		query			file containing query points
//////
////// Results are sent to the standard output.
//////----------------------------------------------------------------------
////
//////----------------------------------------------------------------------
//////	Parameters that are set in getArgs()
//////----------------------------------------------------------------------
////void getArgs(int argc, char **argv);			// get command-line arguments
////
////int				k = 1;			// number of nearest neighbors
////int				dim = 2;			// dimension
////double			eps = 0;			// error bound
////int				maxPts = 1000;			// maximum number of data points
////
////istream*		dataIn = NULL;			// input for data points
////istream*		queryIn = NULL;			// input for query points
////
////bool readPt(istream &in, ANNpoint p)			// read point (false on EOF)
////{
////	for (int i = 0; i < dim; i++) {
////		if (!(in >> p[i])) return false;
////	}
////	return true;
////}
////
////void printPt(ostream &out, ANNpoint p)			// print point
////{
////	out << "(" << p[0];
////	for (int i = 1; i < dim; i++) {
////		out << ", " << p[i];
////	}
////	out << ")\n";
////}
////
////int main(int argc, char **argv)
////{
////	int					nPts;					// actual number of data points
////	ANNpointArray		dataPts;				// data points
////	ANNpoint			queryPt;				// query point
////	ANNidxArray			nnIdx;					// near neighbor indices
////	ANNdistArray		dists;					// near neighbor distances
////	ANNkd_tree*			kdTree;					// search structure
////
////	getArgs(argc, argv);						// read command-line arguments
////
////	queryPt = annAllocPt(dim);					// allocate query point
////	dataPts = annAllocPts(maxPts, dim);			// allocate data points
////	nnIdx = new size_idx[k];						// allocate near neigh indices
////	dists = new size_dist[k];						// allocate near neighbor dists
////
////	nPts = 0;									// read data points
////
////	cout << "Data Points:\n";
////	while (nPts < maxPts && readPt(*dataIn, dataPts[nPts])) {
////		printPt(cout, dataPts[nPts]);
////		nPts++;
////	}
////
////	kdTree = new ANNkd_tree(					// build search structure
////		dataPts,					// the data points
////		nPts,						// number of points
////		dim);						// dimension of space
////
////	while (readPt(*queryIn, queryPt)) {			// read query points
////		cout << "Query point: ";				// echo query point
////		printPt(cout, queryPt);
////
////		kdTree->annkSearch(						// search
////			queryPt,						// query point
////			k,								// number of near neighbors
////			nnIdx,							// nearest neighbors (returned)
////			dists,							// distance (returned)
////			eps);							// error bound
////
////		cout << "\tNN:\tIndex\tDistance\n";
////		for (int i = 0; i < k; i++) {			// print summary
////			dists[i] = sqrt(dists[i]);			// unsquare distance
////			cout << "\t" << i << "\t" << nnIdx[i] << "\t" << dists[i] << "\n";
////		}
////	}
////	delete[] nnIdx;							// clean things up
////	delete[] dists;
////	delete kdTree;
////	annClose();									// done with ANN
////
////	return EXIT_SUCCESS;
////}
////
//////----------------------------------------------------------------------
//////	getArgs - get command line arguments
//////----------------------------------------------------------------------
////
////void getArgs(int argc, char **argv)
////{
////	static ifstream dataStream;					// data file stream
////	static ifstream queryStream;				// query file stream
////
////	if (argc <= 1) {							// no arguments
////		cerr << "Usage:\n\n"
////			<< "  ann_sample [-d dim] [-max m] [-nn k] [-e eps] [-df data]"
////			" [-qf query]\n\n"
////			<< "  where:\n"
////			<< "    dim      dimension of the space (default = 2)\n"
////			<< "    m        maximum number of data points (default = 1000)\n"
////			<< "    k        number of nearest neighbors per query (default 1)\n"
////			<< "    eps      the error bound (default = 0.0)\n"
////			<< "    data     name of file containing data points\n"
////			<< "    query    name of file containing query points\n\n"
////			<< " Results are sent to the standard output.\n"
////			<< "\n"
////			<< " To run this demo use:\n"
////			<< "    ann_sample -df data.pts -qf query.pts\n";
////		exit(0);
////	}
////	int i = 1;
////	while (i < argc) {							// read arguments
////		if (!strcmp(argv[i], "-d")) {			// -d option
////			dim = atoi(argv[++i]);				// get dimension to dump
////		}
////		else if (!strcmp(argv[i], "-max")) {	// -max option
////			maxPts = atoi(argv[++i]);			// get max number of points
////		}
////		else if (!strcmp(argv[i], "-nn")) {		// -nn option
////			k = atoi(argv[++i]);				// get number of near neighbors
////		}
////		else if (!strcmp(argv[i], "-e")) {		// -e option
////			sscanf_s(argv[++i], "%lf", &eps);		// get error bound
////		}
////		else if (!strcmp(argv[i], "-df")) {		// -df option
////			dataStream.open(argv[++i], ios::in);// open data file
////			if (!dataStream) {
////				cerr << "Cannot open data file\n";
////				exit(1);
////			}
////			dataIn = &dataStream;				// make this the data stream
////		}
////		else if (!strcmp(argv[i], "-qf")) {		// -qf option
////			queryStream.open(argv[++i], ios::in);// open query file
////			if (!queryStream) {
////				cerr << "Cannot open query file\n";
////				exit(1);
////			}
////			queryIn = &queryStream;			// make this query stream
////		}
////		else {									// illegal syntax
////			cerr << "Unrecognized option.\n";
////			exit(1);
////		}
////		i++;
////	}
////	if (dataIn == NULL || queryIn == NULL) {
////		cerr << "-df and -qf options must be specified\n";
////		exit(1);
////	}
////}
