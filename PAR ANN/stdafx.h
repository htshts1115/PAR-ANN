// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>
#include <stdlib.h> //for encode/decode base64, random number
#include <fstream> //File read write
#include <iostream> //cout, cin
#include <stdio.h>
#include <conio.h>  //_getch();
#include <sys/stat.h> 
#include <vector>  //for the use of vector class template
#include <string>  //string
#include <ctime> //time1.0e6
#include <iomanip>  //setprecision
#include <random> //random number generator
#include <time.h> //time seed
#include <algorithm>  
#include <math.h>
#include <list>
#include <limits>
#include <cassert>
#include <assert.h>
#include <iterator>
#include <set>
#include <functional>	//bind1st
#include <numeric>

#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include <ANN/ANN.h>

//#include <omp.h>		//OpenMP multi-thread!

using namespace std;
using namespace cv;