#include "autoLabel.h"


int main() {
	AutoLabel* al = new AutoLabel;

	char imagePathArray[] = "E:/Users/raychiu/Desktop/Industrial defect detection data/autoLabel"; //图像路径
	char* imagePath = imagePathArray;

	vector<string> files;
	////获取该路径下的所有文件
	al->getFiles(imagePath, files);

	char str[30];
	int size = files.size();
	for (int i = 0; i < size; i++)
	{
		cout << files[i].c_str() << endl;
		al->autoLabelMethod(files[i].c_str());   // 自动标注
	}
}