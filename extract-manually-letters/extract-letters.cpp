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

cv::vector<Point> leftClick;

typedef struct {
    Mat img;
    int indexX;
    int indexY;
} mapStruct;

typedef struct {
    CvRect rect;
    char letter;
} charStruct;

/************************* PROCESS PARAMETERS *************************/

string getParams (int argc, char *argv[])
{
    if (argc < 3){
        cout << "./extract-letters -d <directory with LP>" << endl;
        exit(1);
    }

    // get values from input
    string dir;
    int c;
    extern char * optarg;
    while ((c = getopt(argc, argv, "d:h")) != -1){
        switch(c) {
            case 'd':
                dir = optarg;
                break;
            case 'h':
                cout << "./extract-letters -d <directory with LP>" << endl;
                exit(0);
            default:
                cout << "./extract-letters -d <directory with LP>" << endl;
                exit(1);
        }       
    }

    // kontrola zadani parametru
    if(dir.empty()) {
        cout << "./extract-letters -d <directory with LP>" << endl;
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

/************************* DRAW BOUNDING BOX --************************/

Mat drawBoundingBox(Mat src, CvRect boundRect)
{
    Mat boundImg;
    src.copyTo(boundImg);
    rectangle(boundImg, Point(boundRect.x,boundRect.y), 
              Point(boundRect.x+boundRect.width,boundRect.y+boundRect.height), 
              Scalar(120, rand() % 255, rand() % 255), 
              1);
    return boundImg;
}

/************************** GET MOUSE CLICKS **************************/

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if(event == CV_EVENT_LBUTTONDOWN)
    {
        leftClick.push_back(Point(x,y));
    }
}

/*********************** FILL STRUCT OF LETTERS ***********************/

vector<charStruct> fillStructOfLetters(Mat src, string spz)
{
    charStruct letter;
    vector<charStruct> lettersVector;
    
    int pos = 0;
    int distance;

    if (leftClick.size() >= 2) 
        distance = leftClick[1].x - leftClick[0].x;

    for (int j=0; j < leftClick.size(); j++){
        
        // coordinate of x and width
        letter.rect.x = leftClick[j].x - (distance/2) * 1.3; 
        letter.rect.width  = distance * 1.3;

        // coordinate of y and height
        letter.rect.y = leftClick[j].y - distance;   
        letter.rect.height = 2 * distance;

        // compute differ for WIDTH/HEIGHT
        float H_ = (float)WIDTH*letter.rect.height/letter.rect.width;
        float delta = (float)HEIGHT / H_;

        // change coordinate of y and height
        letter.rect.y = leftClick[j].y - distance*delta;   
        letter.rect.height = 2 * distance*delta;

        // correction
        if (letter.rect.x < 0) letter.rect.x = 0;
        if (letter.rect.y < 0) letter.rect.y = 0;
        if ((letter.rect.x + letter.rect.width ) > src.cols) letter.rect.width  = src.cols -1 - letter.rect.x;
        if ((letter.rect.y + letter.rect.height) > src.rows) letter.rect.height = src.rows -1 - letter.rect.y;

        // set letter from spz
        letter.letter = spz[pos];
        pos++;

        // push all spz
        lettersVector.push_back(letter);
    }

    return lettersVector;
}

/**************************** PROCESS IMAGE ***************************/

void processImage ()
{
    
}

/****************************** FILL MAPS *****************************/

/* Map contains names of images of each characters A-Z, 0-9 */

map<char, mapStruct> fillMap()
{
    map<char, mapStruct> mapa;
    
    ifstream indexList("letter-images/list.dat");
    string line;

    // loop across all images from input file
    while(getline(indexList, line))
    {
        char znak; int inX, inY;
        istringstream iss(line);
        iss >> znak >> inX >> inY;      

        string n = "letter-images/img_";
        n += znak;
        n += ".png";

        Mat img = imread(n);
        if(!img.data) {
            cout << "Missing file " << n << endl ;
            exit(1);
        }
    
        mapStruct item;
        img.copyTo(item.img);
        item.indexX = inX;
        item.indexY = inY;

        mapa[znak] = item;
    }

    return mapa;
}

/*************** RESIZE LETTER TO WIDTH X HEIGHT SIZE ****************/

Mat resizeWIDTHxHEIGHT (Mat letterImg)
{
    if ((letterImg.cols == WIDTH) && (letterImg.rows == HEIGHT))
        return letterImg;
 
    Mat outImg (HEIGHT, WIDTH, CV_8UC3);
    outImg.setTo(0);
    letterImg.copyTo( outImg(Rect(0, 0, letterImg.cols, letterImg.rows)) );
 
    for (int i=letterImg.rows; i < HEIGHT; i++)
        for (int j=0; j < WIDTH; j++)
            outImg.at<int>(i,j) = outImg.at<int>(i-1,j);

    return outImg;
}

/****************************** SAVE MAP ******************************/

void saveMap (map<char, mapStruct> mapa)
{
    ofstream indexList;
    indexList.open("letter-images/list.dat");

    for (map<char, mapStruct>::iterator it=mapa.begin(); it!=mapa.end(); ++it){
        if (((it->second).img).empty()) {
            continue;
        }
        else {
            string cesta = "";
            cesta += "letter-images/img_";
            cesta += it->first;
            cesta += ".png";
            imwrite(cesta, (it->second).img);

            indexList << it->first << " " << (it->second).indexX << " " << (it->second).indexY << endl;
        }
    }

    indexList.close();
}

/**********************************************************************/

map<char, int> loadImgNumber()
{
    map<char, int> mapa;

    ifstream numList("negative-LP/list.dat");
    string line;

    while(getline(numList, line))
    {
        char znak; int num;
        istringstream iss(line);
        iss >> znak >> num;      

        mapa[znak] = num;
    }

    return mapa;
}

/**********************************************************************/
 
void saveNumList (map<char,int> mapa)
{
    ofstream out("negative-LP/list.dat");

    for (map<char,int>::iterator it=mapa.begin(); it!=mapa.end(); ++it){
        out << it->first << " " << it->second << endl;     
    }
}

/******************************** MAIN ********************************/

int main (int argc, char ** argv)
{
    string lpDir = getParams(argc, argv);
    vector<string> directoryLP = getFilesFromDir(lpDir);

    // create map and fill it with empty images for each numbers
    map<char, mapStruct> mapa = fillMap();

    // create map and fill it with number of next image with negative samples
    map<char, int> imgNumMap = loadImgNumber();

    // go throw all images
    for (int i = 0; i < directoryLP.size(); i++)
    {
        string imgName = directoryLP[i];
        cout << imgName << "    " << endl;

        /******************/
        /*   LOAD IMAGE   */

        Mat src, gray, outImg;
        src = imread(imgName);
        if(!src.data) {
            cout << "Could not open or find the image" << endl ;
            return 1;
        }

        resize(src,src,cv::Size(src.cols*3, src.rows*3));
        imshow("image", src);

        src.copyTo(outImg);

        /****************************/
        /*   SAVE CLICKS OF MOUSE   */

        setMouseCallback("image", CallBackFunc, NULL);

        /**************************/
        /*   GET KEYBOARD INPUT   */

        string spz = "";

        while(true)
        {
            int key = waitKey(0) % 0x100;

            if (key == 32)  break; // mezernik
            if (key == 10)  break; // enter
            if (key == 141) break; // enter na numericke klavesnici

            if ((key >= 176) && (key <= 185)) key -= 128; // numericka klavesnice
            if ((key >=  97) && (key <= 122)) key -= 32;  // mala pismena

            // pokud je to znak, ktery potrebuju, zapis do stringu spz
            if (((key >= 65) && (key <= 90)) || ((key >= 48) && (key <= 57))) 
                spz += (char)key;
            else if (key == 255) // delete - vymazani celeho stringu, znovu
                spz = "";
            else if (key == 27){ // esc - exit script
                saveMap(mapa);
                exit(0);
            }
        }

        /******************************/
        /*   FILL STRUCT OF LETTERS   */

        vector<charStruct> lettersVector = fillStructOfLetters(src, spz);

        /********************************************************/
        /*   DRAW BLACK RECTANGLES ACROSS LETTERS AND SAVE IT   */      

        map<char, Mat> imgMap;

        for (map<char,int>::iterator it=imgNumMap.begin(); it!=imgNumMap.end(); ++it){
            char z = it->first;
            src.copyTo(imgMap[z]);
        }

        for (int j=0; j < lettersVector.size(); j++)  
        {
            char z = lettersVector[j].letter;
            Mat pImg (lettersVector[j].rect.height, lettersVector[j].rect.width, CV_8UC3, 0.0);
            Mat roi (imgMap[z], lettersVector[j].rect);
            pImg.copyTo(roi);
        }

        if (lettersVector.size() != 0)
        {
            for (map<char,Mat>::iterator it=imgMap.begin(); it!=imgMap.end(); ++it){
               char z = it->first;

                stringstream ss;
                ss << imgNumMap[z];
                string s = ss.str();

                string p = "negative-LP/img_";
                p += z;
                p += "/";
                p += s;
                p += ".png";
                imwrite(p, it->second);
                imgNumMap[z]++;
            }
        
            saveNumList(imgNumMap);
        }

        /*************************/
        /*   DRAW BOUNDING BOX   */

        Mat boundImg;
        src.copyTo(boundImg);

        Mat letterImg;

        for (int j=0; j < lettersVector.size(); j++)  
        {
            // region of interest
            letterImg = src(lettersVector[j].rect);
            
            float pomer = (float)WIDTH/lettersVector[j].rect.width;
            resize(letterImg,letterImg,cv::Size(letterImg.cols*pomer, letterImg.rows*pomer));

            // resize if not WIDTH x HEIGHT
            letterImg = resizeWIDTHxHEIGHT(letterImg);

            /********************************************************************/
            /*** connect new letter to existing image with other same letters ***/

            mapStruct item = mapa[lettersVector[j].letter];
            
            Mat finalImg;
            (item.img).copyTo(finalImg);

            if (item.indexX >= MAX_IMG_ON_ROW){
                item.indexX = 0;
                item.indexY++;

                Mat pImg (finalImg.rows+HEIGHT, finalImg.cols, CV_8UC3, 0.0);
                Mat roi (pImg, Rect(0, 0, finalImg.cols, finalImg.rows));  
                finalImg.copyTo(roi);

                //Mat roiLetter (pImg, Rect(0, finalImg.rows, WIDTH, HEIGHT));
                Mat roiLetter (pImg, Rect(0, item.indexY*HEIGHT, WIDTH, HEIGHT));
                letterImg.copyTo(roiLetter);

                pImg.copyTo(finalImg);
                item.indexX++;
            }
            else {
                Mat roi (finalImg, Rect(item.indexX*WIDTH, item.indexY*HEIGHT, WIDTH, HEIGHT));
                letterImg.copyTo(roi);                
                item.indexX++;    
            }

            finalImg.copyTo(item.img);
            mapa[lettersVector[j].letter] = item;
  
            saveMap(mapa);
        }

        /*************/
        /*   CLEAN   */

        while (leftClick.size() != 0)     
            leftClick.erase(leftClick.begin());

        while (lettersVector.size() != 0) 
            lettersVector.erase(lettersVector.begin());

        // delete licence plate image from directory
        remove(imgName.c_str());
    }

    // saveMap(mapa);

	return 0;
}
