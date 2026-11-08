#include "../include/mylib.h"

cv::Mat detectionContour(cv::Mat img_in)
{
    cv::Mat img_grey;

    cv::cvtColor(img_in, img_grey, cv::COLOR_BGR2GRAY);

    int kernel[3][3] = {
        {-1, 0, 1}, 
        {-1, 0, 1}, 
        {-1, 0, 1}
    };

    cv::Mat img_out = cv::Mat::zeros(img_grey.size(), img_grey.type());

    for (int i = 0; i<img_grey.rows; i++)
    {
        for (int j = 0; j < img_grey.cols; j++)
        {
            int somme = 0;
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    somme += kernel[x][y]*img_grey.at<uchar>(i+x-1, j+y-1);
                }
            }

            somme = std::abs(somme);
            img_out.at<uchar>(i, j) = static_cast<uchar>(somme);
        }
    }

    return img_out;
}

cv::Mat seuillage_couleur(cv::Mat frame)
{
    float nb, nv, nr;
    uchar r, v, b;
    cv::Mat frame_out;

    if (frame.empty())
        exit(0);

    frame_out.create(frame.rows, frame.cols, CV_8UC3);

    for (int i = 0; i < frame.rows; i++)
    {
        for (int j = 0; j < frame.cols; j++)
        {
            b = frame.at<Vec3b>(i,j)[0];
			v = frame.at<Vec3b>(i,j)[1];
			r = frame.at<Vec3b>(i,j)[2];

            nb = b / sqrt(r*r + b*b + v*v);
            nv = v / sqrt(r*r + b*b + v*v);
            nr = r / sqrt(r*r + b*b + v*v);

            if ((b < 120) && (v < 120) && (r < 120))
            {
				frame_out.at<Vec3b>(i,j)[0] = 0;
				frame_out.at<Vec3b>(i,j)[1] = 0;
				frame_out.at<Vec3b>(i,j)[2] = 255;
			}
            else
            {
				frame_out.at<Vec3b>(i,j)[0] = b;
				frame_out.at<Vec3b>(i,j)[1] = v;
				frame_out.at<Vec3b>(i,j)[2] = r;
			}
        }
    }
    return frame_out;
}