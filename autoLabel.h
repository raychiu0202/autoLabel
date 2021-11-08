#pragma once

#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <atlbase.h>
#include "tinyxml.h"
#include <io.h>

#define DEBUGDISPLAY 1 //调试开关
using namespace std;
using namespace cv;

class AutoLabel {
public:
	AutoLabel();
	void autoLabelMethod(std::string wholeName); //自动标注函数
	void getFiles(std::string path, std::vector<std::string>  &files);
//private:
};