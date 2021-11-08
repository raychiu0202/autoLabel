#include "autoLabel.h"

AutoLabel::AutoLabel()
{

}


/*
	自动标注图片函数
	将每张单调背景中的物体自动提取物体外接矩形，并提取出yolo支持的xml标注文件。
*/
void AutoLabel::autoLabelMethod(std::string wholeName) {
	//*****************************第一步 找出目标物体的外接矩形*****************************
	cv::Mat img = cv::imread(wholeName.c_str(), -1);

	size_t index = wholeName.find_last_of("/");
	string fileName = wholeName.substr(index+1, wholeName.length() - 1);
	cout << fileName << endl;
	string pathStr = wholeName.substr(0, index);
	cout << pathStr << endl;
	index = fileName.find_last_of(".");
	string prefixName = fileName.substr(0, index);
	cout << prefixName << endl;

	Mat imgSmall = img.clone();
	//resize(img, imgSmall, Size(), 0.5, 0.5, INTER_LINEAR_EXACT);

	// 取出HSV图像
	cv::Mat HSV;
	cv::cvtColor(imgSmall, HSV, COLOR_BGR2HSV);

	Mat greenMask, noGreenImg;
	noGreenImg = imgSmall.clone();
	inRange(HSV, Scalar(30, 43, 46), Scalar(85, 255, 255), greenMask);

	//cv::medianBlur(addImage, filter, 5); //中值滤波使边缘均衡清晰
	Mat element = getStructuringElement(MORPH_RECT, Size(9, 9));
	morphologyEx(greenMask, greenMask, MORPH_OPEN, element); //开区间保证边缘连续起来
	//morphologyEx(greenMask, greenMask, MORPH_CLOSE, element);

	//因为背景为纯色绿色，只保留非绿色背景的区域(透明的物体效果不好)
	for (int r = 0; r < imgSmall.rows; r++)
	{
		for (int c = 0; c < imgSmall.cols; c++)
		{
			if (greenMask.at<uchar>(r, c) == 255)
			{
				noGreenImg.at<Vec3b>(r, c)[0] = 0;
				noGreenImg.at<Vec3b>(r, c)[1] = 0;
				noGreenImg.at<Vec3b>(r, c)[2] = 0;
			}
		}
	}

	Mat imgThresh, notGreenGry, notGreenfilter, imgCny;

	cv::cvtColor(noGreenImg, notGreenGry, cv::COLOR_BGR2GRAY);
	cv::medianBlur(notGreenGry, notGreenfilter, 5); //中值滤波使边缘均衡清晰
	cv::threshold(notGreenfilter, imgThresh, 10, 255, THRESH_BINARY);
	cv::Canny(imgThresh, imgCny, 40, 80, 3, true);

	// 提取轮廓
	vector<vector<Point> > baseContours, baseContours_temp;
	vector<Vec4i> baseHierarchy;
	cv::findContours(imgCny, baseContours_temp, baseHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//根据面积阈值筛选一下轮廓
	int maxNum = 0;
	for (int i = 0; i < baseContours_temp.size(); i++)
	{
		cout << "轮廓面积为：" << contourArea(baseContours_temp[i]) << endl;
		if (contourArea(baseContours_temp[i]) > contourArea(baseContours_temp[maxNum]))
			maxNum = i;
	}
	baseContours.push_back(baseContours_temp[maxNum]);

	Rect roiBox = cv::boundingRect(baseContours[0]);
	cv::rectangle(imgSmall, Point(roiBox.x-3, roiBox.y-3), Point(roiBox.x + roiBox.width+6, roiBox.y + roiBox.height+6), Scalar(0, 255, 0), 2);

	//初始化Mat 掩膜容器
	Mat mask = Mat::zeros(imgSmall.size(), CV_32FC1);
	// 填充轮廓
	Scalar color(rand() & 255, rand() & 255, rand() & 255);
	drawContours(mask, baseContours, 0, color, FILLED);

	// ***********************第二步转为xml标注文件*****************************
	// 新建的xml文件名字
	string filename = pathStr + "/" + prefixName + ".xml";

	//新建一个xml文件
	// 定义一个TiXmlDocument类指针
	TiXmlDocument* pWriteDoc = new TiXmlDocument();

	// xml的声明(三个属性：版本，编码格式，独立文件声明)
	TiXmlDeclaration* pDeclare = new TiXmlDeclaration("1.0", "UTF-8", "yes");
	pWriteDoc->LinkEndChild(pDeclare);			// 连接到最后

	// 根节点
	TiXmlElement* pRootElement = new TiXmlElement("annotation");
	pWriteDoc->LinkEndChild(pRootElement);		// 把根节点连接到最后

	// 二级节点
	TiXmlElement* pFolderElement = new TiXmlElement("folder");
	pRootElement->LinkEndChild(pFolderElement);	// 连接到根节点下
	TiXmlText* folderContent = new TiXmlText("JPEGImages");
	pFolderElement->LinkEndChild(folderContent);	// 给folder节点添加文本

	TiXmlElement* pFileNameElement = new TiXmlElement("filename");
	pRootElement->LinkEndChild(pFileNameElement);	// 连接到根节点下
	TiXmlText* fileNameContent = new TiXmlText(fileName.c_str());
	pFileNameElement->LinkEndChild(fileNameContent);	// 给filename节点添加文本

	TiXmlElement* pPathElement = new TiXmlElement("path");
	pRootElement->LinkEndChild(pPathElement);	// 连接到根节点下
	TiXmlText* pathContent = new TiXmlText(wholeName.c_str());
	pPathElement->LinkEndChild(pathContent);	// 给path节点添加文本

	TiXmlElement* pSizeElement = new TiXmlElement("size");
	pRootElement->LinkEndChild(pSizeElement);	// 连接到根节点下
	TiXmlElement* pWithElement = new TiXmlElement("width");
	pSizeElement->LinkEndChild(pWithElement);	// 连接到二级节点size节点下的三级节点
	TiXmlText* withContent = new TiXmlText(to_string(imgSmall.cols).c_str());		// width节点文本
	pWithElement->LinkEndChild(withContent);	// 给三级节点width添加文本
	TiXmlElement* pHeightElement = new TiXmlElement("height");
	pSizeElement->LinkEndChild(pHeightElement);	// 连接到二级节点size节点下的三级节点
	TiXmlText* heightContent = new TiXmlText(to_string(imgSmall.rows).c_str());		// height节点文本
	pHeightElement->LinkEndChild(heightContent);	// 给三级节点height添加文本
	TiXmlElement* pDepthElement = new TiXmlElement("depth"); //通道数目
	pSizeElement->LinkEndChild(pDepthElement);	// 连接到二级节点size节点下的三级节点
	TiXmlText* depthContent = new TiXmlText("3");		// depth节点文本
	pDepthElement->LinkEndChild(depthContent);	// 给三级节点depth添加文本

	//TiXmlElement* pSegmentedElement = new TiXmlElement("segmented");
	//pRootElement->LinkEndChild(pSegmentedElement);	// 连接到根节点下
	//TiXmlText* segmentedContent = new TiXmlText("0");
	//pSegmentedElement->LinkEndChild(segmentedContent);	// 给segmented节点添加文本

	TiXmlElement* pObjectElement = new TiXmlElement("object");
	pRootElement->LinkEndChild(pObjectElement);	// 连接到根节点下
	TiXmlElement* pNameElement = new TiXmlElement("name"); //*****正例负例标记节点，正例normal负例defect******
	pObjectElement->LinkEndChild(pNameElement);	// 连接到二级节点object节点下的三级节点
	TiXmlText* nameContent = new TiXmlText("defect");		// name节点文本
	pNameElement->LinkEndChild(nameContent);	// 给三级节点name添加文本
	//TiXmlElement* pPoseElement = new TiXmlElement("pose");
	//pObjectElement->LinkEndChild(pPoseElement);	// 连接到二级节点object节点下的三级节点
	//TiXmlText* poseContent = new TiXmlText("Unspecified");		// pose节点文本
	//pPoseElement->LinkEndChild(poseContent);	// 给三级节点pose添加文本
	//TiXmlElement* pTruncatedElement = new TiXmlElement("truncated");
	//pObjectElement->LinkEndChild(pTruncatedElement);	// 连接到二级节点object节点下的三级节点
	//TiXmlText* truncatedContent = new TiXmlText("0");		// truncated节点文本
	//pTruncatedElement->LinkEndChild(truncatedContent);	// 给三级节点truncated添加文本
	//TiXmlElement* pDifficultElement = new TiXmlElement("difficult");
	//pObjectElement->LinkEndChild(pDifficultElement);	// 连接到二级节点object节点下的三级节点
	//TiXmlText* difficultContent = new TiXmlText("1");		// difficult节点文本
	//pDifficultElement->LinkEndChild(difficultContent);	// 给三级节点difficult添加文本
	TiXmlElement* pBndboxElement = new TiXmlElement("bndbox");
	pObjectElement->LinkEndChild(pBndboxElement);	// 连接到二级节点object节点下的三级节点

	TiXmlElement* pXminElement = new TiXmlElement("xmin");
	pBndboxElement->LinkEndChild(pXminElement);
	TiXmlText* xminContent = new TiXmlText(to_string(roiBox.x - 3).c_str());		// xmin节点文本
	pXminElement->LinkEndChild(xminContent);	// 给四级节点xmin添加文本
	TiXmlElement* pYminElement = new TiXmlElement("ymin");
	pBndboxElement->LinkEndChild(pYminElement);
	TiXmlText* yminContent = new TiXmlText(to_string(roiBox.y - 3).c_str());		// ymin节点文本
	pYminElement->LinkEndChild(yminContent);	// 给四级节点ymin添加文本
	TiXmlElement* pXmaxElement = new TiXmlElement("xmax");
	pBndboxElement->LinkEndChild(pXmaxElement);
	TiXmlText* xmaxContent = new TiXmlText(to_string(roiBox.x + roiBox.width + 6).c_str());		// xmax节点文本
	pXmaxElement->LinkEndChild(xmaxContent);	// 给四级节点xmax添加文本
	TiXmlElement* pYmaxElement = new TiXmlElement("ymax");
	pBndboxElement->LinkEndChild(pYmaxElement);
	TiXmlText* ymaxContent = new TiXmlText(to_string(roiBox.y + roiBox.height + 6).c_str());		// ymax节点文本
	pYmaxElement->LinkEndChild(ymaxContent);	// 给四级节点ymax添加文本

	//		保存到文件	
	pWriteDoc->SaveFile(filename.c_str());
	printf("new xml success, file's name is %s\n\n", filename.c_str());


	return;
}

//**********************************************************************
// Method:   获取文件夹下所有文件
// FullName: getFiles
// Returns:  void
// Parameter: 输入和对应的模板路径
// Timer:2021.11.08
//**********************************************************************
void AutoLabel::getFiles(std::string path, std::vector<std::string>  &files) {
	struct _finddata_t filefind;
	intptr_t hfile = 0;
	std::string s;
	if ((hfile = _findfirst(s.assign(path).append("/*").c_str(), &filefind)) != -1) {
		do {
			if (filefind.attrib == _A_SUBDIR) {
				if (strcmp(filefind.name, ".") && strcmp(filefind.name, "..")) {
					getFiles(s.assign(path).append("/").append(filefind.name), files);
				}
			}
			else {
				files.push_back(s.assign(path).append("/").append(filefind.name));
				std::cout << filefind.name << std::endl;
			}
		} while (_findnext(hfile, &filefind) == 0);
	} _findclose(hfile);
}