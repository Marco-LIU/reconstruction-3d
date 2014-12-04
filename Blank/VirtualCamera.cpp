#include "VirtualCamera.h"

VirtualCamera::VirtualCamera(void)
{
	distortion=NULL;
	rotationMatrix=NULL;
	translationVector=NULL;
	fc.x=0;
	fc.y=0;
	cc.x=0;
	cc.y=0;
}

VirtualCamera::~VirtualCamera(void)
{

}

void VirtualCamera::loadDistortion(std::string path)
{
	loadMatrix(distortion,5,1,path);
}

void VirtualCamera::loadCameraMatrix(std::string path)
{
	cv::Mat camMatrix;
	loadMatrix(camMatrix,3,3,path);
	matrix = camMatrix;
	cc.x = Utilities::matGet2D(camMatrix,2,0);
	cc.y = Utilities::matGet2D(camMatrix,2,1);
	
	fc.x = Utilities::matGet2D(camMatrix,0,0);
	fc.y = Utilities::matGet2D(camMatrix,1,1);

}

void VirtualCamera::loadRotationMatrix(std::string path)
{
	loadMatrix(rotationMatrix,3,3,path);
}

void VirtualCamera::loadTranslationVector(std::string path)
{
	loadMatrix(translationVector,3,1,path);
}

//从4个文件载入摄像机参数
void VirtualCamera::loadFromFile(std::string path)
{
	std::string file_name;
	file_name = path.c_str();
	
	file_name = path+"matrix.txt";
	loadCameraMatrix(file_name);

	file_name = path+"distortion.txt";
	loadDistortion(file_name);

	file_name = path+"rotation.txt";
	loadRotationMatrix(file_name);

	file_name = path+"translation.txt";
	loadTranslationVector(file_name);
}

int VirtualCamera::loadMatrix(cv::Mat &matrix,int rows,int cols ,std::string file)
{
	std:: ifstream in1; 
	in1.open(file.c_str());
	
	if(in1==NULL)
	{
		std::cout<<"Error loading file "<<file.c_str()<<"\n";
		return -1;

	}

	if(!matrix.empty())
		matrix.release();

	matrix=cv::Mat(rows, cols, CV_32F);

	for(int i=0; i<rows; i++)
	{
		for(int j=0; j<cols; j++)
		{
			float val;
			in1>>val;

			Utilities::matSet2D(matrix,j,i,val);

		}
	}
	return 1;
}

cv::Point3f VirtualCamera::cam2WorldSpace(cv::Point3f p)
{
	cv::Mat tmp(3,1,CV_32F);
	cv::Mat tmpPoint(3,1,CV_32F);
	//cv::Point3f ret;

	tmpPoint.at<float>(0) = p.x;
	tmpPoint.at<float>(1) = p.y;
	tmpPoint.at<float>(2) = p.z;

	tmp = -rotationMatrix.t() * translationVector ;
	tmpPoint = rotationMatrix.t() * tmpPoint;

	p.x = tmp.at<float>(0) + tmpPoint.at<float>(0);
	p.y = tmp.at<float>(1) + tmpPoint.at<float>(1);
	p.z = tmp.at<float>(2) + tmpPoint.at<float>(2);

	return p;
}
cv::Mat VirtualCamera::generateRotateMatrix(float xDegree,float yDegree,float zDegree)
{
	//计算旋转矩阵
	float a = xDegree;	//x轴旋转的角度
	float b = yDegree;	//y轴旋转的角度
	float r = zDegree;	//z轴旋转的角度
	a = a*3.1415926/180;
	b = b*3.1415926/180;
	r = r*3.1415926/180;
	float rot00 = std::cos(b)*std::cos(r);
	float rot01 = std::cos(r)*std::sin(a)*std::sin(b)-std::cos(a)*std::sin(r);
	float rot02 = std::cos(a)*std::cos(r)*std::sin(b)+std::sin(a)*std::sin(r);
	float rot10 = std::cos(b)*std::sin(r);
	float rot11 = std::cos(a)*std::cos(r)+std::sin(a)*std::sin(b)*std::sin(r);
	float rot12 = -std::cos(r)*std::sin(a)+std::cos(a)*std::sin(b)*std::sin(r);
	float rot20 = -std::sin(b);
	float rot21 = std::cos(b)*std::sin(a);
	float rot22 = std::cos(a)*std::cos(b);

	rotationMatrix = cv::Mat(3,3,CV_32FC1);
	rotationMatrix.at<float>(0,0) = rot00;
	rotationMatrix.at<float>(0,1) = rot01;
	rotationMatrix.at<float>(0,2) = rot02;
	rotationMatrix.at<float>(1,0) = rot10;
	rotationMatrix.at<float>(1,1) = rot11;
	rotationMatrix.at<float>(1,2) = rot12;
	rotationMatrix.at<float>(2,0) = rot20;
	rotationMatrix.at<float>(2,1) = rot21;
	rotationMatrix.at<float>(2,2) = rot22;

	return rotationMatrix;
}
cv::Point3f VirtualCamera::world2CamSpace(cv::Point3f p)
{
	cv::Mat tmp(3,1,CV_32F);
	cv::Mat tmpPoint(3,1,CV_32F);
	//cv::Point3f ret;

	tmpPoint.at<float>(0) = p.x;
	tmpPoint.at<float>(1) = p.y;
	tmpPoint.at<float>(2) = p.z;

	tmp = rotationMatrix * tmpPoint;
	float tx = translationVector.at<float>(0);
	float ty = translationVector.at<float>(1);
	float tz = translationVector.at<float>(2);
	p.x = tmp.at<float>(0) + tx;
	p.y = tmp.at<float>(1) + ty;
	p.z = tmp.at<float>(2) + tz;

	return p;
}
std::vector<cv::Point2f> VirtualCamera::project2Image(std::vector<cv::Point3f> objPoints)
{
	cv::Mat rvec;
	cv::Rodrigues(rotationMatrix,rvec);
	cv::Matx33f CM(
	fc.x,0,cc.x,
	0,fc.y,cc.y,
	0,0,1);

	std::vector<cv::Point2f> imgPoints;
	//投影到图像坐标系
	cv::projectPoints(
		objPoints,
		rvec,
		translationVector,
		CM,
		distortion,
		imgPoints);

	return imgPoints;
}
std::vector<cv::Point2f> VirtualCamera::project2ImageWithDistortion(std::vector<cv::Point3f> objPoints)
{
	cv::Mat rvec;
	cv::Rodrigues(rotationMatrix,rvec);
	cv::Matx33f CM(
	fc.x,0,cc.x,
	0,fc.y,cc.y,
	0,0,1);
	cv::Mat Distort(5,1,CV_32FC1,cv::Scalar(0));
	std::vector<cv::Point2f> imgPoints;
	//投影到图像坐标系
	cv::projectPoints(
		objPoints,
		rvec,
		translationVector,
		CM,
		Distort,
		imgPoints);

	return imgPoints;
}

cv::Point3f VirtualCamera::calCameraPosition()
{
	position = cam2WorldSpace(cv::Point3f(0,0,0));
	return position;
}

void VirtualCamera::loadKeyPoints(std::string filename)
{
	std::ifstream ik(filename.c_str());
	std::pair<float,float> temp;
	while(ik>>temp.first && ik>>temp.second)
	{
		keypoints.push_back(temp);
	}
	ik.close();
}

//载入重建的3d点
std::vector<cv::Point3f> VirtualCamera::load3dPoints(std::string filename)
{
	std::vector<cv::Point3f> ret;
	std::ifstream ik(filename.c_str());
	float x,y,z;
	while(ik>>x && ik>>y && ik>>z)
	{
		ret.push_back(cv::Point3f(x,y,z));
	}
	ik.close();
	return ret;
}

//保存重投影的2d点
void VirtualCamera::saveproject2dPoints(std::vector<cv::Point2f> pts,std::string filename)
{
	std::ofstream of(filename);
	for(int i=0;i<pts.size();i++)
	{
		of<<pts[i].x<<"	"<<pts[i].y<<std::endl;
	}
	of.close();
}

std::vector<cv::Point2f> VirtualCamera::undistortPoints(std::vector<cv::Point2f> &pts)
{
	std::vector<cv::Point2f> ret;

	cv::Matx33f camMatrix(
	fc.x,0,cc.x,
	0,fc.y,cc.y,
	0,0,1);

	cv::undistortPoints(pts,ret,camMatrix,distortion);

	return ret;
}

std::vector<cv::Point2f> VirtualCamera::undistortPointsImageSpace(std::vector<cv::Point2f> &pts)
{
	std::vector<cv::Point2f> ret;

	cv::Matx33f camMatrix(
	fc.x,0,cc.x,
	0,fc.y,cc.y,
	0,0,1);

	cv::undistortPoints(pts,ret,camMatrix,distortion);

	for(int i=0;i<ret.size();i++)
	{
		ret[i] = ret[i]*fc.x+cc;
	}

	return ret;
}

cv::Point3f VirtualCamera::pixelToImageSpace(cv::Point2f p)
{
	cv::Point3f point;

	point.x = (p.x-cc.x) / fc.x;
	point.y = (p.y-cc.y) / fc.y;
	point.z = 1;
	
	return point;
}