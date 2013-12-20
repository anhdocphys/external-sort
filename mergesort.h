/****************************************************************************

****************************************************************************/
#ifndef MERGESORT_H
#define MERGESORT_H

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <queue>
#include <cstdio>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h> 
using namespace std;

bool isRegularFile(const string& filename);
std::string stl_basename(const std::string& path);

template <class T> class MERGE_DATA {

public:
    T data;
    istream *stream;
    bool (*compFunc)(const T &a, const T &b);

    MERGE_DATA (const T &data,
                istream *stream,
                bool (*compFunc)(const T &a, const T &b))
    :
        data(data),
        stream(stream),
        compFunc(compFunc)
    {}

    bool operator < (const MERGE_DATA &a) const
    {
        return !(compFunc(data, a.data));
    }
};


template <class T> class MergeSort {

public:
    // constructor 
    MergeSort(const string &inFile,
                 ostream *out,
                 bool (*compareFunction)(const T &a, const T &b) = NULL,
                 int  maxBufferSize = 262144,
                 string tempPath     = "./");
    
    // constructor, overloaded
    MergeSort(const string &inFile,
                 ostream *out,
                 int  maxBufferSize = 262144,
                 string tempPath     = "./");
                                               
    // destructor
    ~MergeSort(void);
     
    void Sort();            // the main function
    void SetBufferSize(int bufferSize);		// to set buffer size
    void SetComparison(bool (*compareFunction)(const T &a, const T &b));	// to set comparison operator
    
private:
    string _inFile;
    bool (*_compareFunction)(const T &a, const T &b);
    string _tempPath;
    vector<string>    _vTempFileNames;
    vector<ifstream*>  _vTempFiles;
    unsigned int _maxBufferSize;
    unsigned int _runCounter;
    bool _compressOutput;
    bool _tempFileUsed;
    ostream *_out;

    void DivideAndSort();
    void Merge();
    void WriteToTempFile(const vector<T> &lines);
    void OpenTempFiles();
    void CloseTempFiles();
};

// constructor
template <class T> MergeSort<T>::MergeSort (const string &inFile,
                               ostream *out,
                               bool (*compareFunction)(const T &a, const T &b),
                               int maxBufferSize,
                               string tempPath)
    : _inFile(inFile)
    , _out(out)
    , _compareFunction(compareFunction)
    , _tempPath(tempPath)
    , _maxBufferSize(maxBufferSize)
    , _runCounter(0)
{}

// constructor
template <class T> MergeSort<T>::MergeSort (const string &inFile,
                               ostream *out,
                               int maxBufferSize,
                               string tempPath)
    : _inFile(inFile)
    , _out(out)
    , _compareFunction(NULL)
    , _tempPath(tempPath)
    , _maxBufferSize(maxBufferSize)
    , _runCounter(0)
{}

// destructor
template <class T> MergeSort<T>::~MergeSort(void)
{}

// API for sorting.  
template <class T> void MergeSort<T>::Sort() { 
    DivideAndSort();
    Merge();
}

// change the buffer size used for sorting
template <class T> void MergeSort<T>::SetBufferSize (int bufferSize) {
    _maxBufferSize = bufferSize;
}

// change the sorting criteria
template <class T> void MergeSort<T>::SetComparison (bool (*compareFunction)(const T &a, const T &b)) {
    _compareFunction = compareFunction;
}

template <class T> void MergeSort<T>::DivideAndSort() {

    istream *input = new ifstream(_inFile.c_str(), ios::in);
    if ( input->good() == false ) {
        cerr << "Error: The requested input file (" << _inFile << ") could not be opened. Exiting!" << endl;
        exit (1);
    }
    vector<T> lineBuffer;
    lineBuffer.reserve(100000); 	// about 100K lines to be reserved
    unsigned long totalBytes = 0;  

    _tempFileUsed = false;

    T line;
    while (*input >> line) {		// getline in
        lineBuffer.push_back(line);	
        totalBytes += sizeof(line);

        if (totalBytes > _maxBufferSize) {
            if (_compareFunction != NULL)
                sort(lineBuffer.begin(), lineBuffer.end(), *_compareFunction);
            else
                sort(lineBuffer.begin(), lineBuffer.end());
            WriteToTempFile(lineBuffer);
            lineBuffer.clear();
            _tempFileUsed = true;
            totalBytes = 0;
        }
    }

    if (lineBuffer.empty() == false) {
        if (_tempFileUsed == true) {
            if (_compareFunction != NULL)
                sort(lineBuffer.begin(), lineBuffer.end(), *_compareFunction);
            else
                sort(lineBuffer.begin(), lineBuffer.end());
            WriteToTempFile(lineBuffer);
            WriteToTempFile(lineBuffer);
        }
        else {
            if (_compareFunction != NULL)
                sort(lineBuffer.begin(), lineBuffer.end(), *_compareFunction);
            else
                sort(lineBuffer.begin(), lineBuffer.end());
            for (size_t i = 0; i < lineBuffer.size(); ++i)
                *_out << lineBuffer[i] << endl;
        }
    }
}

template <class T> void MergeSort<T>::WriteToTempFile(const vector<T> &lineBuffer) {
    stringstream tempFileSS;
    if (_tempPath.size() == 0)
        tempFileSS << _inFile << "." << _runCounter;
    else
        tempFileSS << _tempPath << "/" << stl_basename(_inFile) << "." << _runCounter;
    string tempFileName = tempFileSS.str();

    ofstream *output;
    output = new ofstream(tempFileName.c_str(), ios::out);

    for (size_t i = 0; i < lineBuffer.size(); ++i) {
        *output << lineBuffer[i] << endl;
    }
    ++_runCounter;
    output->close();
    delete output;
    _vTempFileNames.push_back(tempFileName);
}


template <class T> void MergeSort<T>::Merge() {

    if (_tempFileUsed == false)
        return;

    OpenTempFiles();

    priority_queue< MERGE_DATA<T> > outQueue;

    T line;
    for (size_t i = 0; i < _vTempFiles.size(); ++i) {
        *_vTempFiles[i] >> line;
        outQueue.push( MERGE_DATA<T>(line, _vTempFiles[i], _compareFunction) );
    }

    while (outQueue.empty() == false) {
        MERGE_DATA<T> lowest = outQueue.top();
        *_out << lowest.data << endl;
        outQueue.pop();
        *(lowest.stream) >> line;
        if (*(lowest.stream))
            outQueue.push( MERGE_DATA<T>(line, lowest.stream, _compareFunction) );
    }
    CloseTempFiles();
}


template <class T> void MergeSort<T>::OpenTempFiles() {
    for (size_t i=0; i < _vTempFileNames.size(); ++i) {

        ifstream *file = new ifstream(_vTempFileNames[i].c_str(), ios::in);

        if (file->good() == true) {
            _vTempFiles.push_back(file);
        }
        else {
            cerr << "I/O Error: Unable to open temp file (" << _vTempFileNames[i] << endl;
             exit(1);
        }
    }
}


template <class T> void MergeSort<T>::CloseTempFiles() {
    for (size_t i=0; i < _vTempFiles.size(); ++i) {
        _vTempFiles[i]->close();
        delete _vTempFiles[i];
    }
    for (size_t i=0; i < _vTempFileNames.size(); ++i) {
        remove(_vTempFileNames[i].c_str());  // remove = UNIX "rm"
    }
}

string stl_basename(const string &path) {
    string result;

    char* path_dup = strdup(path.c_str());
    char* basename_part = basename(path_dup);
    result = basename_part;
    free(path_dup);

    size_t pos = result.find_last_of('.');
    if (pos != string::npos )
        result = result.substr(0,pos);

    return result;
}


#endif /* MERGESORT_H */

