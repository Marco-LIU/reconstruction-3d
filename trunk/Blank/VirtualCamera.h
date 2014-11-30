#ifndef _VIRTUAL_CAMERA_H_
#define _VIRTUAL_CAMERA_H_

#include "Utilities.h"

//��ʾһ������������
class VirtualCamera
{
public:
	VirtualCamera(void);
	~VirtualCamera(void);

	//���ļ���������Σ���������һ������������
	void loadDistortion(std::string path);
	void loadCameraMatrix(std::string path);
	void loadRotationMatrix(std::string path);
	void loadTranslationVector(std::string path);
	//��4���ļ��������������
	void loadFromFile(std::string path);
	
	//����������
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

	//��x,y,z����ת�ĽǶ�������ת����
	cv::Mat generateRotateMatrix(float xDegree,float yDegree,float zDegree);

	//�����������ת��Ϊ��������(���̸�����ϵ����������κ�),���㹫ʽR-1(p-T)
	cv::Point3f cam2WorldSpace(cv::Point3f p);
	//����������任�������������(���̸�����ϵ����������κ�),���㹫ʽR*p + T
	cv::Point3f world2CamSpace(cv::Point3f p);
	//�����ص㣬ӳ��Ϊz=1ƽ���ϵĵ�
	cv::Point3f pixelToImageSpace(cv::Point2f p);
	//���������ԭ������������ϵ�е�λ��
	cv::Point3f calCameraPosition();

	//����������ͶӰΪͼ������(��������κ�)
	std::vector<cv::Point2f> project2Image(std::vector<cv::Point3f> objPoints);
	//����ͶӰ�������ǻ���
	std::vector<cv::Point2f> project2ImageWithDistortion(std::vector<cv::Point3f> objPoints);
	//�����ؽ���3d��
	std::vector<cv::Point3f> load3dPoints(std::string filename);
	//������ͶӰ��2d��
	void saveproject2dPoints(std::vector<cv::Point2f> pts,std::string filename="project2dPoints.txt");
	//����ȥ�����ĵ㣨���������ϵ��z=1ƽ���ϵĵ㣩
	std::vector<cv::Point2f> undistortPoints(std::vector<cv::Point2f> &pts);
	//����ȥ�����ĵ㣬ת��Ϊ��ͼ������
	std::vector<cv::Point2f> undistortPointsImageSpace(std::vector<cv::Point2f> &pts);
public:
	//�ڲ�
	cv::Point2f fc;				//���ؽ���
	cv::Point2f cc;				//ͶӰ���ĵ�
	cv::Mat distortion;			//����ϵ��5x1����
	cv::Mat matrix;				//�ڲ�
	//���
	cv::Mat rotationMatrix;		//��ת����3x3����
	cv::Mat translationVector;	//ƽ�ƾ���3x1����
	cv::Point3f position;		//������ռ����������λ�ã������̸��������ϵ��
	//������
	std::vector<std::pair<float,float>> keypoints;
private:
	int loadMatrix(cv::Mat &matrix,int rows,int cols ,std::string file);
};

#endif