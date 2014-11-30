#ifndef _STEREO_RECONSTRUCTOR_H_
#define _STEREO_RECONSTRUCTOR_H_

#include "Utilities.h"
#include "CameraCalibration.h"
#include "VirtualCamera.h"
//#include "Reconstructor.h"

//执行双目重建
class StereoReconstructor
{
public:
  //用于描述重建的一个三维点
  struct RestructPoint
  {
    cv::Point3f point;	//重建三维点
    float distance;		//2条异面直线之间的距离

    RestructPoint() {}
    RestructPoint(cv::Point3f p, float d) : point(p), distance(d) {}
  };

  StereoReconstructor(void);
  ~StereoReconstructor(void);

  //1、双目定标（使用前提，对应的匹配角点，必须提前找好）
  //计算R和T,载入2个摄像机的内参和匹配点(>20)，标定R和T
  void computeRT(std::string cam1Folder = "camCalib/cam1/",
                 std::string cam2Folder = "camCalib/cam2/");
  void computeRTRandom(std::string cam1Folder = "camCalib/cam1/",
                       std::string cam2Folder = "camCalib/cam2/");

  //2、重建	
  //从left.jpg和right.jpg图像中计算角点，重建标定点
  void RestructCalibrationPointsFromImage(std::string leftFilename,
                                          std::string rightFilename,
                                          int x = 11, int y = 8,
                                          int squareLength = 20);

  //重建点云，从left.txt和right.txt文件中读取匹配点，重建点云,保存到Points.txt文件中
  void RestructPointCloud(bool objSpace = false,
                          bool undistort = true,
                          std::string pcfilename = "Points.ply");

  //3、重建的实际实现函数
  static std::vector<RestructPoint> triangulation(VirtualCamera& camera1,
                                                  VirtualCamera& camera2,
                                                  bool undistortPoints);

  //读取坐标系，把当前坐标转换到这个坐标系下
  static void translateCoordinate(std::string cordsys,
                                  std::string points,
                                  std::string newPoints = "calPoints_ObjSpace.txt");
  //转换到另一个坐标系的坐标
  static cv::Point3f coordinateChange(cv::Point3f p1,				//输入坐标
                                      std::vector<cv::Point3f> cord);							//坐标，原点，x方向，y方向，z方向
private:
  //标定
  cv::Mat camMatrix1;		//左摄像机内参
  cv::Mat camMatrix2;		//右摄像机内参
  cv::Mat distCoeffs1;	//左摄像机畸变系数
  cv::Mat distCoeffs2;	//右摄像机畸变系数
  cv::Size camImageSize;	//图像大小
  cv::Mat R;				//把左摄像机坐标系的坐标变换到右摄像机
  cv::Mat T;				//把左摄像机坐标系的坐标变换到右摄像机
  std::vector<std::vector<cv::Point3f>> objPoints;	//重建的3d点，左摄像机坐标系下
  std::vector<std::vector<cv::Point2f>> imgPoints1;	//左摄像机匹配点
  std::vector<std::vector<cv::Point2f>> imgPoints2;	//右摄像机匹配点

  //载入物体坐标，左右摄像机的投影坐标，供定标使用
  bool loadImgAndObjPoints(
    std::string cam,									//摄像机文件夹
    std::vector<std::vector<cv::Point2f>> &imgPoints,	//棋盘格投影坐标
    std::vector<std::vector<cv::Point3f>> &objPoints);	//棋盘格物体坐标

  //载入内参matrix.txt和distortion.txt
  bool loadMatrixAndCoe(
    std::string cam,	//摄像机文件夹
    cv::Mat &mat1,		//内参
    cv::Mat &mat2);		//畸变系数

  //载入2d点
  std::vector<cv::Point2f> load2DPoints(std::string file);
  //载入3d点
  std::vector<cv::Point3f> load3DPoints(std::string file);
  //重建精度评估
  //从88个棋盘格点，计算棋盘格的坐标系。要求读入的角点是左上角到右下角的顺序
  //todo 优化精度
  void computeCoordinate(std::string corners,					//读取棋盘格坐标
                         std::string coord = "calPoints_Coordinate.txt",			//保存计算的坐标系
                         int x = 8, int y = 11);										//棋盘格信息

  //生成点云，ply格式，txt格式，带误差的txt格式
  void plyFileGenerate(std::vector<RestructPoint> pointCloud,	//3d点云
                       std::string ply = "pointcloud.ply",				//ply文件
                       std::string txtFile = "pointsDistance.txt",		//带误差的txt文件
                       std::string points = "points.txt");				//txt文件

  //计算边距的长度分布
  void resultEvaluate(std::vector<RestructPoint> result,		//重建的棋盘格点
                      std::string rlt = "_calLineLength.txt",					//线段的误差
                      std::string dist = "_calLineLengthDist.txt",				//线段误差分布
                      int x = 8, int y = 11, int sl = 30);							//棋盘格信息
};

#endif