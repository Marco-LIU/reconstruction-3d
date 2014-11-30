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
		//��һ������
		static void		normalize(cv::Vec3f &vec);
		static void		normalize3dtable(double vec[3]);
		static float	distance(cv::Point3f p1, cv::Point3f p2);
		//���ɷֲ����ʣ�д���ļ�
		static void		paraDistribute(std::vector<float> &para, std::string filename);

	//��д����
		//���ض�ά��ͨ������ĵ�
		static double		matGet2D(cv::Mat m, int x, int y);
		static cv::Vec3d	matGet3D(cv::Mat m, int x, int y);
		//���ö�ά��ͨ������ĵ�
		static void			matSet2D(cv::Mat m, int x, int y, double val);
		static void			matSet3D(cv::Mat m, int x, int y, cv::Vec3d);

		//ɨ�����е��ļ����ļ��У����ظ�����path=""��ʾɨ�赱ǰ���ļ���,path="cam\\"��ʾɨ��cam�ļ��е��ļ�
		static int						folderFilesScan(const char *path);
		static std::vector<std::string>	folderTxtScan(const char *path);		//ɨ������txt�ļ�
		static std::vector<std::string>	folderImagesScan(const char *path);		//ɨ������jpg�ļ�
		static std::vector<std::string> folderBmpImagesScan(const char *path);	//ɨ������bmp�ļ�

		//��д����
		static std::vector<cv::Vec4f>	load4dPoints(std::string filename);		//����4d����
		static std::vector<cv::Point3f>	load3dPoints(std::string filename);		//����3d����
		static std::vector<cv::Point2f>	load2dPoints(std::string filename);		//����2d�ǵ�
		static void						save2dPoints(std::string filename,std::vector<cv::Point2f> pts);
		//����2d�ǵ�����
		static void										save2dPointsArray(std::string filename,std::vector<std::vector<cv::Point2f>> pts);
		static std::vector<std::vector<cv::Point2f>>	load2dPointsArray(std::string filename);
		//�������Ϊply�ļ�
		static void						savePly(std::vector<cv::Point3f>& pointCloud,std::string filename);

		//��д����
		static void						exportMat(const char *path, cv::Mat m);
		static bool						importMat(const char *path, cv::Mat m);	

	//���̸�ǵ�
		//�������̸����������,д���ļ�
		static void	generateObjCorners(std::string filename ="ObjCorners.txt",int XCorners=8, int YCorners=11,int size = 30);
		//�������̸�ͼ��Ļ��Ƶ㣨��λ���ף�,ǰ��4����Ϊ�߿򣬺���ÿ4����Ϊһ����ɫ����
		static std::vector<cv::Point3f>	generateChessboardCorners(std::string filename ="ObjCorners.txt",int XCorners=8, int YCorners=11,int size = 30);
		//�������̸�ͼ��
		static cv::Mat drawChessboardImg(std::string filename,int width, int height,std::vector<cv::Point2f>& corners,float scale = 25);
		//����ĳ���ļ��У��������Ľǵ㣬������
		//static void drawImageCorners(std::string folder,int X,int Y);
		//���򻯽ǵ������ʽ�������������ң����ϵ��µ�˳�����У�ֻ����˫Ŀ�궨��ʱ����õ���
		static cv::vector<cv::Point2f> normalizeCorners(cv::vector<cv::Point2f> corners,int X,int Y);

	//�����ؽ�
		//�����ص�ӳ�䵽z=1.0��ƽ���ϣ�������ռ䣩
		static void				pixelToImageSpace(double p[3], CvScalar fc, CvScalar cc);
		//static cv::Point3f		pixelToImageSpace(cv::Point2f p, VirtualCamera cam);
		//�ѻ����ӳ��Ϊ�ǻ����
		//static cv::Point2f		undistortPoints( cv::Point2f p, VirtualCamera cam);
		//����������ƽ��Ľ���
		static CvScalar		planeRayInter(CvScalar planeNormal,CvScalar planePoint, CvScalar rayVector, CvScalar rayPoint );
		//����2������ֱ�ߵĽ���,����֮��ľ���
		static float		line_lineIntersection(cv::Point3f p1, cv::Vec3f v1, cv::Point3f p2,cv::Vec3f v2,cv::Point3f &p);
		//���ֱ��,����ֱ�߷���
		static cv::Point3f	lineFit(std::vector<cv::Point3f>& pts);
		static std::pair<cv::Point2f,cv::Point2f> lineFit(std::vector<cv::Point2f>& pts);
	//��δʹ��
		static double			matGet3D(cv::Mat m, int x, int y, int i);
		static void				matSet3D(cv::Mat m, int x, int y,int i, double val);
		static void				autoContrast(cv::Mat img_in, cv::Mat &img_out);
		static void				autoContrast(IplImage *img_in, IplImage *img_out);
		static int				accessMat(cv::Mat m, int x, int y, int i);
		static int				accessMat(cv::Mat m, int x, int y);
		static void				folderScan(const char *path);
};

#endif