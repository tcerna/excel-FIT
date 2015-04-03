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

/******************** GLOBAL VARIABLES AND STRUCTS ********************/

#define WIDTH  20  
#define HEIGHT 40  
#define MAX_IMG_ON_ROW 40

int maxY = 0;
int minY = 0;

typedef struct {
    char znak;
    int indexX;
    int indexY;
} indexStruct;

typedef struct {
    CvRect rect;
    char letter;
} charStruct;

/************************* PROCESS PARAMETERS *************************/

string getParams (int argc, char *argv[])
{
    if (argc < 3){
        cout << "./sort-to-categories -d <directory with positive letter samples>" << endl;
        exit(1);
    }

    // get values from input
    string dir;
    int c;
    extern char * optarg;
    while ((c = getopt(argc, argv, "d:i:h")) != -1){
        switch(c) {
            case 'd':
                dir = optarg;
                break;
            case 'i':
                dir = optarg;
                break;
            case 'h':
                cout << "./sort-to-categories -d <directory with positive letter samples>" << endl;
                exit(0);
            default:
                cout << "./sort-to-categories -d <directory with positive letter samples>" << endl;
                exit(1);
        }       
    }

    // kontrola zadani parametru
    if(dir.empty()) {
        cout << "./sort-to-categories -d <directory with positive letter samples>" << endl;
        exit(1);
    }

    return dir;
}

/********************** GET CONTENT OF DIRECTORY **********************/

cv::vector<string> getFilesFromDir (string lpDir)
{
    cv::vector<string> vect;

    DIR *dir;
    class dirent *ent;
    
    dir = opendir(lpDir.c_str());
    while ((ent = readdir(dir)) != NULL) {
        string file_name = ent->d_name;
        
        if (file_name[0] == '.')
            continue;
        
        string file_path = lpDir + "/" + file_name;
        vect.push_back(file_path);
    }

    closedir(dir);
    return vect;
}

/**********************************************************************/

map<char, indexStruct> loadImgIndexes(string lpDir)
{
    map<char, indexStruct> mapa;

    string path = lpDir+"/list.dat";
    ifstream numList(path.c_str());
    string line;

    while(getline(numList, line))
    {
        char znak; 
        int indexX; 
        int indexY;

        istringstream iss(line);
        iss >> znak >> indexX >> indexY;      

        mapa[znak].znak = znak;
        mapa[znak].indexX = indexX;
        mapa[znak].indexY = indexY;        
    }

    return mapa;
}

/************************ GET OUTPUT IMAGE NAME ***********************/

string getOutputImgName (string imgName)
{
    string outName;

    int pos = imgName.find(".png");
    outName = imgName.substr(0,pos);

    return outName;
}

/*********************** ADD TO IMAGE CATEGORY ************************/

Mat addToCategoryImg(Mat letterImg, Mat matCat, int countCat)
{
    int inX = countCat % MAX_IMG_ON_ROW;
    int inY = countCat / MAX_IMG_ON_ROW;

    if ((inX == 0) && (inY != 0))
    {
        Mat newImg(matCat.rows+HEIGHT, matCat.cols, CV_8UC3, 0.0);
        Mat roiRow = newImg(Rect(0, 0, matCat.cols, matCat.rows));
        matCat.copyTo(roiRow);
        newImg.copyTo(matCat);    
    }    

    Mat roi = matCat(Rect(inX*WIDTH, inY*HEIGHT, WIDTH, HEIGHT));
    letterImg.copyTo(roi);

    return matCat;
}

/*************** DRAW BOUNDING BOX ********************/

Mat drawBoundingBox(Mat src, CvRect boundRect)
{
    Mat boundImg;
    src.copyTo(boundImg);
    rectangle(boundImg, Point(boundRect.x,boundRect.y), Point(boundRect.x+boundRect.width,boundRect.y+boundRect.height), Scalar(0,255,0), 1);
    return boundImg;
}

/**************************** PROCESS IMAGE ***************************/

void processImage (Mat src, indexStruct item, string outName)
{
    int count = item.indexY * MAX_IMG_ON_ROW + item.indexX;

    // create matrix for all categories
    Mat matCat1(HEIGHT, WIDTH*MAX_IMG_ON_ROW, CV_8UC3, 0.0);
    Mat matCat2(HEIGHT, WIDTH*MAX_IMG_ON_ROW, CV_8UC3, 0.0);
    Mat matCat3(HEIGHT, WIDTH*MAX_IMG_ON_ROW, CV_8UC3, 0.0);

    int countCat1 = 0;
    int countCat2 = 0;
    int countCat3 = 0;    

    // go across all letters in one image
    for (int i=0; i < count; i++)
    {
        /************************/
        /*   GET LETTER IMAGE   */   
 

        int inX = i % MAX_IMG_ON_ROW;
        int inY = i / MAX_IMG_ON_ROW;

        CvRect boundRect;
        boundRect.x = inX*WIDTH;
        boundRect.y = inY*HEIGHT;
        boundRect.width  = WIDTH;
        boundRect.height = HEIGHT;

        // get letter
        Mat letterImg (HEIGHT, WIDTH, CV_8UC3, 0.0);
        Mat roi (src, boundRect);
        roi.copyTo(letterImg);

        //imshow("letter", letterImg);

        // show source image with bounding box around actual letter
        Mat boundSrc = drawBoundingBox(src, boundRect);

        resize(boundSrc,boundSrc,cv::Size(boundSrc.cols*0.8, boundSrc.rows*0.8));
        imshow("bound", boundSrc);

        /**************************/
        /*   SORT TO CATEGORIES   */

        int category;

        while(true)
        {
            int key = waitKey(0) % 0x100;
            if (key == 49) category = 1;  // klavesa 1 - good
            if (key == 50) category = 2;  // klavesa 2 - middle
            if (key == 51) category = 3;  // klavesa 3 - bad
            if (key == 52) category = 4;  // klavesa 4 - extremally bad
            break;
        }
       
        /******************************/
        /*   SAVE TO CATEGORY IMAGE   */

        switch(category) {
            case 1:
                    matCat1 = addToCategoryImg(letterImg, matCat1, countCat1);
                    countCat1++;                   
                    break;
            case 2: 
                    matCat2 = addToCategoryImg(letterImg, matCat2, countCat2);
                    countCat2++;
                    break;
            case 3:
                    matCat3 = addToCategoryImg(letterImg, matCat3, countCat3);
                    countCat3++;
                    break;
            case 4:
                    break;
        }
    }    

    /***********************/
    /*   SAVE CATEGORIES   */

    cout << outName << "_1.png" << endl;

    imwrite(outName+"_1.png", matCat1);
    imwrite(outName+"_2.png", matCat2);
    imwrite(outName+"_3.png", matCat3);

    /********************/
    /*   SAVE INDEXES   */

    //saveIndexes();
}

/******************************** MAIN ********************************/

int main (int argc, char ** argv)
{
    string lpDir = getParams(argc, argv);
    vector<string> directoryLP = getFilesFromDir(lpDir);

    // create map and fill it with number of next image with negative samples
    map<char, indexStruct> letterIndexes = loadImgIndexes(lpDir);

    // go throw all images
    for (int i = 0; i < directoryLP.size(); i++)
    {
        string imgName = directoryLP[i];
        cout << imgName << endl;

        string outName = getOutputImgName(imgName);
        char znak = outName[outName.size()-1];

        outName = "categories/img_";
        outName += znak;
        cout << outName << endl;

        /******************/
        /*   LOAD IMAGE   */

        Mat src, outImg;
        src = imread(imgName);
        if(!src.data) {
            cout << "Could not open or find the image" << endl ;
            return 1;
        }

        /*********************/
        /*   PROCESS IMAGE   */

        indexStruct item = letterIndexes[znak];
        processImage(src, item, outName);
        exit(0);
    }

	return 0;
}
