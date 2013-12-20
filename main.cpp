/***********************************************************************
	Sort a file arcording to orderBy function

************************************************************************/
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
using namespace std;

#include "mergesort.h"


// define the sorting rule
inline bool orderBy(const string &first, const string &second) 
{ 
	return first.compare(second) < 0;
}
// main program
int main(int argc, char* argv[]) {
	if ( argc != 4 ){
		cout << "Usage: "<<argv[0]<<" [input data] [outputdata] [buffersize in MB] "<<endl;
		return 0;
	} else {
		string inFileName       = argv[1];							// filename in
		string outFileName 		= argv[2];							// filename out
		ofstream output(outFileName.c_str());
		unsigned long  bufferSize   = atoi(argv[3])*(1<<20);	 // convert from MB to bytes   
		string tempPath     = "./";     
		MergeSort<string> *sorter = new MergeSort<string> (inFileName,  &output,  orderBy, bufferSize, tempPath);
		sorter->Sort();
	}
}
