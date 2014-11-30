#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "cv.h"
#include "highgui.h"
#include <conio.h>
#include <direct.h>

#define STRICT
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using std::min;
using std::max;

class VirtualCamera;

class Utilities
{
	public:
		static bool		XOR(bool val1, bool val2);
		//归一化向量
		static void		normalize(cv::Vec3f &vec);
		static void		normalize3dtable(double vec[3]);
		static float	distance(cv::Point3f p1, cv::Point3f p2);
		//生成分布概率，写入文件
		static void		paraDistribute(std::vector<float> &para, std::string filename);

	//读写数据
		//返回二维单通道矩阵的点
		static double		matGet2D(cv::Mat m, int x, int y);
		static cv::Vec3d	matGet3D(cv::Mat m, int x, int y);
		//设置二维单通道矩阵的点
		static void			matSet2D(cv::Mat m, int x, int y, double val);
		static void			matSet3D(cv::Mat m, int x, int y, cv::Vec3d);

		//扫描所有的文件和文件夹，返回个数。path=""表示扫描当前的文件夹,path="cam\\"表示扫描cam文件夹的文件
		static int						folderFilesScan(const char *path);
		static std::vector<std::string>	folderTxtScan(const char *path);		//扫描所有txt文件
		static std::vector<std::string>	folderImagesScan(const char *path);		//扫描所有jpg文件
		static std::vector<std::string> folderBmpImagesScan(const char *path);	//扫描所有bmp文件

		//读写数据
		static std::vector<cv::Vec4f>	load4dPoints(std::string filename);		//载入4d数据
		static std::vector<cv::Point3f>	load3dPoints(std::string filename);		//载入3d点云
		static std::vector<cv::Point2f>	load2dPoints(std::string filename);		//载入2d角点
		static void						save2dPoints(std::string filename,std::vector<cv::Point2f> pts);
		//载入2d角点数组
		static void										save2dPointsArray(std::string filename,std::vector<std::vector<cv::Point2f>> pts);
		static std::vector<std::vector<cv::Point2f>>	load2dPointsArray(std::string filename);
		//保存点云为ply文件
		static void						savePly(std::vector<cv::Point3f>& pointCloud,std::string filename);

		//读写矩阵
		static void						exportMat(const char *path, cv::Mat m);
		static bool						importMat(const char *path, cv::Mat m);	

	//棋盘格角点
		//生成棋盘格的物体坐标,写入文件
		static void	generateObjCorners(std::string filename ="ObjCorners.txt",int XCorners=8, int YCorners=11,int size = 30);
		//生成棋盘格图像的绘制点（单位毫米）,前面4个点为边框，后面每4个点为一个黑色方块
		static std::vector<cv::Point3f>	generateChessboardCorners(std::string filename ="ObjCorners.txt",int XCorners=8, int YCorners=11,int size = 30);
		//绘制棋盘格图像
		static cv::Mat drawChessboardImg(std::string filename,int width, int height,std::vector<cv::Point2f>& corners,float scale = 25);
		//遍历某个文件夹，计算它的角点，并绘制
		//static void drawImageCorners(std::string folder,int X,int Y);
		//规则化角点的排序方式，让它从做到右，从上到下的顺序排列（只有在双目标定的时候才用到）
		static cv::vector<cv::Point2f> normalizeCorners(cv::vector<cv::Point2f> corners,int X,int Y);

	//畸变重建
		//把像素点映射到z=1.0的平面上（摄像机空间）
		static void				pixelToImageSpace(double p[3], CvScalar fc, CvScalar cc);
		//static cv::Point3f		pixelToImageSpace(cv::Point2f p, VirtualCamera cam);
		//把畸变点映射为非畸变点
		//static cv::Point2f		undistortPoints( cv::Point2f p, VirtualCamera cam);
		//返回射线与平面的交点
		static CvScalar		planeRayInter(CvScalar planeNormal,CvScalar planePoint, CvScalar rayVector, CvScalar rayPoint );
		//计算2条异面直线的交点,返回之间的距离
		static float		line_lineIntersection(cv::Point3f p1, cv::Vec3f v1, cv::Point3f p2,cv::Vec3f v2,cv::Point3f &p);
		//拟合直线,返回直线方向
		static cv::Point3f	lineFit(std::vector<cv::Point3f>& pts);
		static std::pair<cv::Point2f,cv::Point2f> lineFit(std::vector<cv::Point2f>& pts);
	//暂未使用
		static double			matGet3D(cv::Mat m, int x, int y, int i);
		static void				matSet3D(cv::Mat m, int x, int y,int i, double val);
		static void				autoContrast(cv::Mat img_in, cv::Mat &img_out);
		static void				autoContrast(IplImage *img_in, IplImage *img_out);
		static int				accessMat(cv::Mat m, int x, int y, int i);
		static int				accessMat(cv::Mat m, int x, int y);
		static void				folderScan(const char *path);
};

#endif