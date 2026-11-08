#ifndef MYLIB_H
#define MYLIB_H

#include <opencv2/opencv.hpp>

using namespace cv;

cv::Mat detectionContour(cv::Mat img_in);

cv::Mat seuillage_couleur(cv::Mat frame);

#endif