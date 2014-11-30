#ifndef _VIRTUAL_CAMERA_H_
#define _VIRTUAL_CAMERA_H_

#include "Utilities.h"

//表示一个拍摄的摄像机
class VirtualCamera
{
public:
	VirtualCamera(void);
	~VirtualCamera(void);

	//从文件载入内外参，用于描述一个拍摄的摄像机
	void loadDistortion(std::string path);
	void loadCameraMatrix(std::string path);
	void loadRotationMatrix(std::string path);
	void loadTranslationVector(std::string path);
	//从4个文件载入摄像机参数
	void loadFromFile(std::string path);
	
	//载入特征点
	void loadKeyPoints(std::string filename);
	void setKeyPoints(std::vector<cv::Point2f> pts)
	{
		for(int i=0;i<pts.size();i++)
		{
			cv::Point2f tpt = pts[i];
			std::pair<float,float> cp;
			cp.first = tpt.x;
			cp.second = tpt.y;
			keypoints.push_back(cp);
		}
	}

	//从x,y,z轴旋转的角度生成旋转矩阵
	cv::Mat generateRotateMatrix(float xDegree,float yDegree,float zDegree);

	//把摄像机坐标转换为世界坐标(棋盘格坐标系，载入内外参后),计算公式R-1(p-T)
	cv::Point3f cam2WorldSpace(cv::Point3f p);
	//把世界坐标变换到摄像机坐标中(棋盘格坐标系，载入内外参后),计算公式R*p + T
	cv::Point3f world2CamSpace(cv::Point3f p);
	//把像素点，映射为z=1平面上的点
	cv::Point3f pixelToImageSpace(cv::Point2f p);
	//计算摄像机原点在物体坐标系中的位置
	cv::Point3f calCameraPosition();

	//把世界坐标投影为图像坐标(载入内外参后)
	std::vector<cv::Point2f> project2Image(std::vector<cv::Point3f> objPoints);
	//理想投影，不考虑畸变
	std::vector<cv::Point2f> project2ImageWithDistortion(std::vector<cv::Point3f> objPoints);
	//载入重建的3d点
	std::vector<cv::Point3f> load3dPoints(std::string filename);
	//保存重投影的2d点
	void saveproject2dPoints(std::vector<cv::Point2f> pts,std::string filename="project2dPoints.txt");
	//计算去畸变后的点（摄像机坐标系，z=1平面上的点）
	std::vector<cv::Point2f> undistortPoints(std::vector<cv::Point2f> &pts);
	//计算去畸变后的点，转换为了图像坐标
	std::vector<cv::Point2f> undistortPointsImageSpace(std::vector<cv::Point2f> &pts);
public:
	//内参
	cv::Point2f fc;				//像素焦距
	cv::Point2f cc;				//投影中心点
	cv::Mat distortion;			//畸变系数5x1矩阵
	cv::Mat matrix;				//内参
	//外参
	cv::Mat rotationMatrix;		//旋转矩阵3x3矩阵
	cv::Mat translationVector;	//平移矩阵3x1矩阵
	cv::Point3f position;		//在世界空间中摄像机的位置（即棋盘格定义的坐标系）
	//特征点
	std::vector<std::pair<float,float>> keypoints;
private:
	int loadMatrix(cv::Mat &matrix,int rows,int cols ,std::string file);
};

#endif