#ifndef _CAMERA_CALIBRATION_H_
#define _CAMERA_CALIBRATION_H_

#include "Utilities.h"

/*
这个类用于单摄像机标定
*/
class CameraCalibration
{
public:
  cv::Mat camMatrix;			//内参,3x3矩阵
  cv::Mat distortion;			//畸变系数,1x5矩阵

  //找某张图片的角点
  //在内存中找角点,img_grey为灰度图，找不到返回空数组，找到了返回角点
  static  cv::vector<cv::Point2f> findCorners(cv::Mat img_grey, int X = 11, int Y = 9);

  static void findCorners(std::string filename, bool &found, int X = 8, int Y = 11, bool debug = false);
  //已知内参，计算外参
  static void calCameraExtra(std::string imgCorners, std::string objCorners = "ObjCorners.txt");
  //todo 增加一些同样功能的静态函数

public:
  //构造函数
  CameraCalibration(std::string fd = "Cam", int _XCorners = 11, int _YCorners = 8, int sl = 20);
  ~CameraCalibration(void);

  //1、计算棋盘格角点坐标
  //遍历folderPath文件夹中1.jpg,2.jpg,...,numOfCamImgs.jpg图像，如果成功找到所有的角点，则记录角点坐标到*_ImgCorners.txt中
  void findCorners(bool debug = false);

  //2、载入对应角点坐标
  //载入一对对应的角点坐标
  void loadCorrespondCorners(std::string imgCornersFile, std::string objCornersFile);
  void loadCorrespondCorners();

  //3、读取数据进行标定
  //内参标定，用所有的点标定
  void cameraCalibration(bool saveExtr = false);

  //4、计算外参
  //遍历每一张图片，计算外参
  void calCameraExtra();
public:
  //for debug

  //载入带误差计算的标定点
  int loadCorrectCorrespondCorners(std::string imgCornersFile, std::string objCornersFile, float lf, float re);
  void loadCorrectCorrespondCorners(float lf, float re);
  //载入理想的标定点
  void loadIdealCorrespondCorners();

  //内参标定，设置畸变系数的个数
  void cameraCalibrationCustom(int k, cv::Mat matrix = cv::Mat(3, 3, CV_32FC1, cv::Scalar(-1)));
  //随机抽取imgs张图像，重复N次标定，取平均值
  void cameraCalibrationRandom(int imgs, int N = 5);
  //从start开始，到end结束，每次平均标定N次。看看各个系数的走向
  void cameraCalibrationProgress(int start = -1, int end = -1, int N = 1);
protected:
  //把内外参矩阵写入文件
  void exportTxtFiles(std::string path, int CAMCALIB_OUT_PARAM);

  //统计参数分布
  void paraDistribute(std::vector<double> &para, std::string filename);
private:
  //图片数目和角点数目
  int			numOfCamImgs;	//棋盘格图像张数，如10张
  int			XCorners;		//X方向的角点个数,如11
  int			YCorners;		//Y方向的角点个数，如8
  int			squareLength;	//棋盘格的长度，如20表示20mm
  int			numOfCorners;	//每张棋盘格的角点个数，如11x8=88个角点
  cv::Size	camImageSize;	//棋盘格图像大小，如1280x1024

  std::string					folder;			//标定图片所在的文件夹
  std::vector<std::string>	imgFiles;		//所有的标定文件名.jpg图片

  std::vector<std::vector<cv::Point2f>> CamCorners;		//摄像机图像角点坐标
  std::vector<std::vector<cv::Point3f>> ObjCorners;		//记录定标点（世界坐标系）
};

#endif