#include<iostream>
#include<opencv2/opencv.hpp>
#include <sstream>

using namespace std;
using namespace cv;

string int2str(const int &int_temp)
{
	stringstream stream;
	string string_temp;
	stream << int_temp;
	string_temp = stream.str();  //此处也可以用 stream>>string_temp
	return string_temp;
}

int main(int argc, const char** argv)
{
	VideoCapture cap(1);
	bool update_bg_model = true;

	//cap.open(0);

	if (!cap.isOpened())
	{
		printf("can not open camera or video file\n");
		return -1;
	}

	namedWindow("监视器", WINDOW_AUTOSIZE);
	namedWindow("前景模板", WINDOW_AUTOSIZE);
	namedWindow("前景图像", WINDOW_AUTOSIZE);
	namedWindow("背景图像", WINDOW_AUTOSIZE);

	Ptr<BackgroundSubtractorMOG2> bg_model = createBackgroundSubtractorMOG2();//(100, 3, 0.3, 5);建立背景模型

	Mat img, fgmask, fgimg;
	int i = 0;

	while(true)
	{
		i++;
		cap >> img;

		if (img.empty())
			break;

		img = img(Rect(40, 0,600, img.rows));

		if (fgimg.empty())
			fgimg.create(img.size(), img.type());

		//更新模型
		bg_model->apply(img, fgmask, update_bg_model ? -1 : 0);
		medianBlur(fgmask, fgmask, 13);     //中值滤波
		threshold(fgmask, fgmask, 150, 255, THRESH_BINARY);   //二值化

		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

		/*erode(fgmask, fgmask, element, Point(0, 0), 3);
		dilate(fgmask, fgmask, element, Point(0, 0), 3);*/

		Mat srcGrayImage = fgmask.clone();
		vector<vector<Point>> vContours;
		vector<Vec4i> vHierarchy;
		findContours(srcGrayImage, vContours, vHierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));

		int count = 0;
		RNG rng(12345);

		for (int i = 0; i < vContours.size(); i++)
		{
			double area = contourArea(vContours[i], false);

			RotatedRect smallRect = minAreaRect(vContours[i]);
			Point2f smallRect_center = smallRect.center;
			float smallRect_width = smallRect.size.width;
			float smallRect_height = smallRect.size.height;
			float smallRect_angle = 0;

			smallRect = RotatedRect(smallRect_center, Size2f(smallRect_height, smallRect_width), smallRect_angle);
			Point2f P[4];
			smallRect.points(P);

			if (area>1000 && area < 4200)
			{
				count++;
				for (int j = 0; j <= 3; j++)
				{
					line(img, P[j], P[(j + 1) % 4], Scalar(255, 0, 0), 2);
				}
			}
			if (area>4200 && area < 6000)
			{
				count += 2;
				for (int j = 0; j <= 3; j++)
				{
					line(img, P[j], P[(j + 1) % 4], Scalar(255, 0, 0), 2);
				}
			}

		}

		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));//任意值
		putText(img, int2str(count), Point(220, 40), FONT_HERSHEY_TRIPLEX, 1, color, 2);

		fgimg = Scalar::all(0);
		img.copyTo(fgimg, fgmask);

		Mat bgimg;
		bg_model->getBackgroundImage(bgimg);

		imshow("监视器", img);

		/*string windows_name = "Video/image_" + int2str(i);
		string windows_name_ext = windows_name + ".jpg";
		imwrite(windows_name_ext, img);*/

		imshow("前景模板", fgmask);


		imshow("前景图像", fgimg);
		if (!bgimg.empty())
			imshow("背景图像", bgimg);

		char k = (char)waitKey(1);
		if (k == 27) break;
		if (k == ' ')
		{
			update_bg_model = !update_bg_model;
			if (update_bg_model)
				printf("\t>背景更新(Background update)已打开\n");
			else
				printf("\t>背景更新(Background update)已关闭\n");
		}
	}

	return 0;
}