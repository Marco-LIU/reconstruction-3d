#ifndef _CAMERA_CALIBRATION_H_
#define _CAMERA_CALIBRATION_H_

#include "Utilities.h"

/*
��������ڵ�������궨
*/
class CameraCalibration
{
public:
  cv::Mat camMatrix;			//�ڲ�,3x3����
  cv::Mat distortion;			//����ϵ��,1x5����

  //��ĳ��ͼƬ�Ľǵ�
  //���ڴ����ҽǵ�,img_greyΪ�Ҷ�ͼ���Ҳ������ؿ����飬�ҵ��˷��ؽǵ�
  static  cv::vector<cv::Point2f> findCorners(cv::Mat img_grey, int X = 11, int Y = 9);

  static void findCorners(std::string filename, bool &found, int X = 8, int Y = 11, bool debug = false);
  //��֪�ڲΣ��������
  static void calCameraExtra(std::string imgCorners, std::string objCorners = "ObjCorners.txt");
  //todo ����һЩͬ�����ܵľ�̬����

public:
  //���캯��
  CameraCalibration(std::string fd = "Cam", int _XCorners = 11, int _YCorners = 8, int sl = 20);
  ~CameraCalibration(void);

  //1���������̸�ǵ�����
  //����folderPath�ļ�����1.jpg,2.jpg,...,numOfCamImgs.jpgͼ������ɹ��ҵ����еĽǵ㣬���¼�ǵ����굽*_ImgCorners.txt��
  void findCorners(bool debug = false);

  //2�������Ӧ�ǵ�����
  //����һ�Զ�Ӧ�Ľǵ�����
  void loadCorrespondCorners(std::string imgCornersFile, std::string objCornersFile);
  void loadCorrespondCorners();

  //3����ȡ���ݽ��б궨
  //�ڲα궨�������еĵ�궨
  void cameraCalibration(bool saveExtr = false);

  //4���������
  //����ÿһ��ͼƬ���������
  void calCameraExtra();
public:
  //for debug

  //�����������ı궨��
  int loadCorrectCorrespondCorners(std::string imgCornersFile, std::string objCornersFile, float lf, float re);
  void loadCorrectCorrespondCorners(float lf, float re);
  //��������ı궨��
  void loadIdealCorrespondCorners();

  //�ڲα궨�����û���ϵ���ĸ���
  void cameraCalibrationCustom(int k, cv::Mat matrix = cv::Mat(3, 3, CV_32FC1, cv::Scalar(-1)));
  //�����ȡimgs��ͼ���ظ�N�α궨��ȡƽ��ֵ
  void cameraCalibrationRandom(int imgs, int N = 5);
  //��start��ʼ����end������ÿ��ƽ���궨N�Ρ���������ϵ��������
  void cameraCalibrationProgress(int start = -1, int end = -1, int N = 1);
protected:
  //������ξ���д���ļ�
  void exportTxtFiles(std::string path, int CAMCALIB_OUT_PARAM);

  //ͳ�Ʋ����ֲ�
  void paraDistribute(std::vector<double> &para, std::string filename);
private:
  //ͼƬ��Ŀ�ͽǵ���Ŀ
  int			numOfCamImgs;	//���̸�ͼ����������10��
  int			XCorners;		//X����Ľǵ����,��11
  int			YCorners;		//Y����Ľǵ��������8
  int			squareLength;	//���̸�ĳ��ȣ���20��ʾ20mm
  int			numOfCorners;	//ÿ�����̸�Ľǵ��������11x8=88���ǵ�
  cv::Size	camImageSize;	//���̸�ͼ���С����1280x1024

  std::string					folder;			//�궨ͼƬ���ڵ��ļ���
  std::vector<std::string>	imgFiles;		//���еı궨�ļ���.jpgͼƬ

  std::vector<std::vector<cv::Point2f>> CamCorners;		//�����ͼ��ǵ�����
  std::vector<std::vector<cv::Point3f>> ObjCorners;		//��¼����㣨��������ϵ��
};

#endif