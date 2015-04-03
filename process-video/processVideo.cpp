/**
 * Title: License Plate Detection and Recognition for Traffic Analysis (Master Thesis)
 * Author: Bc. Tereza Cerna
 * Faculty of Information Technology, Brno University of Technology
 */

#define VISUAL_OUTPUT 1

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <iostream>
#include <sstream>

using namespace std; 
using namespace cv;


template <typename T>
string NumberToString ( T Number )
{
    ostringstream ss;
    ss << Number;
    return ss.str();
}


int main( int argc, char* argv[])
{
	// jmeno souboru pro zpracovani
	string inputVideoName;
	
	// zpracovani parametru prikazove radky
	for( int i = 1; i < argc; i++){
		if( string(argv[ i]) == "-i" && i + 1 < argc){
			inputVideoName = argv[ ++i];
		} else if( string(argv[ i]) == "-h"){
			cout << "Use: " << argv[0] << "  -i inputVideoName" << endl;
			return 0;
		} else {
			cerr << "Use: " << argv[0] << "  -i inputVideoName" << endl;
		}
	}

	// kontrola zadani parametru
	if( inputVideoName.empty() ){
		cerr << "Use: " << argv[0] << "  -i inputVideoName" << endl;
		return -1;
	}

	// Otevreni videa
	VideoCapture capture( inputVideoName);

	if( !capture.isOpened()){
		cerr << "Error: Unable to open input video file \"" << inputVideoName << "\"." << endl;
        return -1;
	}

	bool finished = false;
    Mat frame;

	int pocitadlo = 0;
	int poc = 0;

	while( !finished)
	{ 
		// Doplnte nacteni dalsiho snimku.
		bool success = capture.read(frame);

		finished |= !success;
		if( success)
		{
			if (pocitadlo == 25){
				string name = "data/"+NumberToString(poc)+".png";
				imwrite(name, frame);
				poc++;
				pocitadlo = 0;
			}
			else
				pocitadlo++;
			}

		}
	}
}
