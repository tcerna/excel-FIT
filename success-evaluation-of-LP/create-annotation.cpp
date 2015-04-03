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

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace std;
using namespace cv;

cv::vector<Point> leftClick;

/************************* PROCESS PARAMETERS *************************/

string getParams (int argc, char *argv[])
{
    if (argc < 3){
        cout << "./create-annotation -l <list of images>" << endl;
        exit(1);
    }

    // get values from input
    string dir;
    int c;
    extern char * optarg;
    while ((c = getopt(argc, argv, "l:h")) != -1){
        switch(c) {
            case 'l':
                dir = optarg;
                break;
            case 'h':
                cout << "./create-annotation -l <list of images>" << endl;
                exit(0);
            default:
                cout << "./create-annotation -l <list of images>" << endl;
                exit(1);
        }       
    }

    // kontrola zadani parametru
    if(dir.empty()) {
        cout << "./create-annotation -l <list of images>" << endl;
        exit(1);
    }

    return dir;
}

/************************** GET MOUSE CLICKS **************************/

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if(event == CV_EVENT_LBUTTONDOWN)
    {
        leftClick.push_back(Point(x,y));
    }
}

/******************************** MAIN ********************************/

int main(int argc, char** argv)
{
    string par = getParams(argc, argv);

    // open list with images paths 
    ifstream inputList(par.c_str());   
    string line;

    // loop across all images from input file
    while(getline(inputList, line))
    {
        cout << line << " ";

        /******************/
        /*   LOAD IMAGE   */

        Mat src, gray;
        src = imread(line);
        if(!src.data) {
            cout << "Could not open or find the image" << endl ;
            return 1;
        }

        //resize(src,src,cv::Size(src.cols/1.2, src.rows/1.2));
        imshow("image", src);

        /****************************/
        /*   SAVE CLICKS OF MOUSE   */

        setMouseCallback("image", CallBackFunc, NULL);
        waitKey(0);

        cout << leftClick.size()/2 << " ";

        for (int j=0; j < leftClick.size(); j++)
            cout << leftClick[j].x << " " << leftClick[j].y << " ";
        cout << endl;

        /*************/
        /*   CLEAN   */

        while (leftClick.size() != 0)     
            leftClick.erase(leftClick.begin());
    }

    return 0;
}