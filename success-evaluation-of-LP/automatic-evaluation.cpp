/**
 * Title: License Plate Detection and Recognition for Traffic Analysis (Master Thesis)
 * Author: Bc. Tereza Cerna
 * Faculty of Information Technology, Brno University of Technology
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    
#include <sys/stat.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <map>
#include <iomanip>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace std;
using namespace cv;

/**********************************************************************/

typedef struct {     
    string list;
    string detector;
} param;

typedef struct {
	Point A;
	Point B;
} pointStruct;

typedef struct {
	string name;
	int count;
	vector<pointStruct> vec;
} lineStruct;

/**********************************************************************/

int total = 0;
int good = 0;

/**********************************************************************/

template <typename T>
string NumberToString ( T Number )
{
    ostringstream ss;
    ss << Number;
    return ss.str();
}

/************************* PROCESS PARAMETERS *************************/

param getParams (int argc, char *argv[])
{
    if (argc < 3){
        cout << "./automatic-evaluation -l <list with annotations> -d <detector>" << endl;
        exit(1);
    }

    // get values from input
    param par;
    par.list = "";
    par.detector = "";

    int c;
    extern char * optarg;
    while ((c = getopt(argc, argv, "l:d:h")) != -1){
        switch(c) {
            case 'l':
                par.list = optarg;
                break;
            case 'd':
                par.detector = optarg;
                break;
            case 'h':
                cout << "./automatic-evaluation -l <list with annotations> -d <detector>" << endl;
                exit(0);
            default:
                cout << "./automatic-evaluation -l <list with annotations> -d <detector>" << endl;
                exit(1);
        }       
    }

    // kontrola zadani parametru
    if(par.list.empty() || par.detector.empty()) {
        cout << "./automatic-evaluation -l <list with annotations> -d <detector>" << endl;
        exit(1);
    }

    return par;
}

/**************************** PROCESS LINE ****************************/

lineStruct getValuesFromLine (string line)
{
	lineStruct lineSt;

	istringstream iss(line); 
	string imgName; 
	iss >> lineSt.name >> lineSt.count;

    total += lineSt.count;

	for (int i=0; i < lineSt.count; i++){
		pointStruct points;
		iss >> points.A.x >> points.A.y >> points.B.x >> points.B.y;
		lineSt.vec.push_back(points);
	}

	return lineSt;
}

/************************ GET BIGGER RECTANGLE ************************/             // STACILO BY POUZE ROZSIRIT, VYSKA MUZE BYT ZACHOVANA

CvRect getBiggerRect (CvRect boundRect, Mat src)
{
    float percent = 10.0;
    float border = (float)boundRect.width / 100.0 * (float)percent;
    CvRect newRect = boundRect;

    if ((boundRect.x - border) < 0){
        newRect.x = 0;
        newRect.width += boundRect.x;
    }else{
        newRect.x -= border;
        newRect.width += border;
    }
    
    if ((boundRect.x + boundRect.width + border) > src.cols){
        newRect.width += (src.cols - boundRect.x - boundRect.width - 1);
    }else{
        newRect.width += border;
    }

    return newRect;
}

/**************************** PROCESS LIST ****************************/

void processLine(string line, CascadeClassifier cascade)
{
	lineStruct lineSt = getValuesFromLine(line);
	//cout << lineSt.name << endl;
	
	// load new image
    Mat src, grayImg;
    src = imread(lineSt.name);  
    if (src.data == NULL){
        cout << "Could not load image." << endl;
    }
    cvtColor(src, grayImg, CV_BGR2GRAY);

    // vector for founded objects
    std::vector<Rect> plates;

    // detection licence plates
    cascade.detectMultiScale(grayImg, plates, 1.1, 5, 0, Size(60, 15));
    //cascade.detectMultiScale(grayImg, plates, 1.2, 5, 1, Size(60, 15));

    // draw all bounding boxes
    Mat boundImg;
    src.copyTo(boundImg);
    for (int i = 0; i < plates.size(); i++){
        //rectangle(boundImg, Point(plates[i].x,plates[i].y), Point(plates[i].x+plates[i].width,plates[i].y+plates[i].height), Scalar(255,255,0), 1);
        plates[i] = getBiggerRect(plates[i], src);
        //rectangle(boundImg, Point(plates[i].x,plates[i].y), Point(plates[i].x+plates[i].width,plates[i].y+plates[i].height), Scalar(0,255,0), 1);
    }

    // draw all good boxes
    for (int i=0; i < lineSt.count; i++){
    	for (int j=0; j < plates.size(); j++){

    		pointStruct P = lineSt.vec[i];

            cv::line(boundImg, P.A, P.B, Scalar(255,0,255), 3, 8);

    		if ((plates[j].x > P.A.x) || (P.A.x > plates[j].x+plates[j].width))
    			continue; 

    		if ((plates[j].y > P.A.y) || (P.A.y > plates[j].y+plates[j].height))
    			continue; 
    		
			if ((plates[j].x > P.B.x) || (P.B.x > plates[j].x+plates[j].width))
    			continue; 
    	
    		if ((plates[j].y > P.B.y) || (P.B.y > plates[j].y+plates[j].height))
    			continue; 
    	
    		int width  = P.B.x - P.A.x;
    		int procento = 0;

            procento = width*100/plates[j].width;
            if (procento < 45) continue;
        //    cout << procento << endl;

	        rectangle(boundImg, Point(plates[j].x,plates[j].y), Point(plates[j].x+plates[j].width,plates[j].y+plates[j].height), Scalar(0,0,255), 2);
    	    Mat roi (src, plates[j]);  
            
            //imwrite("out/"+NumberToString(poc)+".png", roi);
            good++;
  
//  Mat outImg;
//  boundImg.copyTo(outImg);
//  resize(outImg, outImg, cv::Size(src.cols/3*2, src.rows/3*2));
//  imshow("bounded", outImg);
//  waitKey(0);
        }
    }

//cout << "=====" << endl;
//  imshow("bounded", boundImg);
//  waitKey(0);
}

/******************************** MAIN ********************************/

int main(int argc, char** argv)
{
    param par = getParams(argc, argv);

    // load cascade classifier
    CascadeClassifier cascade;
    cascade.load(par.detector);
    if (cascade.empty()){
        cout << "Could not load cascade classifier" << endl;
        exit(1);
    }

    // open list with images paths 
    ifstream inputList(par.list.c_str());   
    string line;

    // loop across all images from input file
    while(getline(inputList, line))
    {
        // process image or dataset of images
	    processLine(line, cascade);
	}

    cout << setprecision(3) << fixed;
    cout << "=====================|" << endl;
    cout << "|  total:   " << total << "       |" << endl;
    cout << "|  good:    " << good  << "       |" << endl;
    cout << "|  percent: " << good*100.0/total << "   |" << endl;
    cout << "=====================|" << endl;

    return 0;
}