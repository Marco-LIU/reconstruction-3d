#ifndef _STEREO_RECONSTRUCTOR_H_
#define _STEREO_RECONSTRUCTOR_H_

#include "Utilities.h"
#include "CameraCalibration.h"
#include "VirtualCamera.h"
//#include "Reconstructor.h"

//ִ��˫Ŀ�ؽ�
class StereoReconstructor
{
public:
  //���������ؽ���һ����ά��
  struct RestructPoint
  {
    cv::Point3f point;	//�ؽ���ά��
    float distance;		//2������ֱ��֮��ľ���

    RestructPoint() {}
    RestructPoint(cv::Point3f p, float d) : point(p), distance(d) {}
  };

  StereoReconstructor(void);
  ~StereoReconstructor(void);

  //1��˫Ŀ���꣨ʹ��ǰ�ᣬ��Ӧ��ƥ��ǵ㣬������ǰ�Һã�
  //����R��T,����2����������ڲκ�ƥ���(>20)���궨R��T
  void computeRT(std::string cam1Folder = "camCalib/cam1/",
                 std::string cam2Folder = "camCalib/cam2/");
  void computeRTRandom(std::string cam1Folder = "camCalib/cam1/",
                       std::string cam2Folder = "camCalib/cam2/");

  //2���ؽ�	
  //��left.jpg��right.jpgͼ���м���ǵ㣬�ؽ��궨��
  void RestructCalibrationPointsFromImage(std::string leftFilename,
                                          std::string rightFilename,
                                          int x = 11, int y = 8,
                                          int squareLength = 20);

  //�ؽ����ƣ���left.txt��right.txt�ļ��ж�ȡƥ��㣬�ؽ�����,���浽Points.txt�ļ���
  void RestructPointCloud(bool objSpace = false,
                          bool undistort = true,
                          std::string pcfilename = "Points.ply");

  //3���ؽ���ʵ��ʵ�ֺ���
  static std::vector<RestructPoint> triangulation(VirtualCamera& camera1,
                                                  VirtualCamera& camera2,
                                                  bool undistortPoints);

  //��ȡ����ϵ���ѵ�ǰ����ת�����������ϵ��
  static void translateCoordinate(std::string cordsys,
                                  std::string points,
                                  std::string newPoints = "calPoints_ObjSpace.txt");
  //ת������һ������ϵ������
  static cv::Point3f coordinateChange(cv::Point3f p1,				//��������
                                      std::vector<cv::Point3f> cord);							//���꣬ԭ�㣬x����y����z����
private:
  //�궨
  cv::Mat camMatrix1;		//��������ڲ�
  cv::Mat camMatrix2;		//��������ڲ�
  cv::Mat distCoeffs1;	//�����������ϵ��
  cv::Mat distCoeffs2;	//�����������ϵ��
  cv::Size camImageSize;	//ͼ���С
  cv::Mat R;				//�������������ϵ������任���������
  cv::Mat T;				//�������������ϵ������任���������
  std::vector<std::vector<cv::Point3f>> objPoints;	//�ؽ���3d�㣬�����������ϵ��
  std::vector<std::vector<cv::Point2f>> imgPoints1;	//�������ƥ���
  std::vector<std::vector<cv::Point2f>> imgPoints2;	//�������ƥ���

  //�����������꣬�����������ͶӰ���꣬������ʹ��
  bool loadImgAndObjPoints(
    std::string cam,									//������ļ���
    std::vector<std::vector<cv::Point2f>> &imgPoints,	//���̸�ͶӰ����
    std::vector<std::vector<cv::Point3f>> &objPoints);	//���̸���������

  //�����ڲ�matrix.txt��distortion.txt
  bool loadMatrixAndCoe(
    std::string cam,	//������ļ���
    cv::Mat &mat1,		//�ڲ�
    cv::Mat &mat2);		//����ϵ��

  //����2d��
  std::vector<cv::Point2f> load2DPoints(std::string file);
  //����3d��
  std::vector<cv::Point3f> load3DPoints(std::string file);
  //�ؽ���������
  //��88�����̸�㣬�������̸������ϵ��Ҫ�����Ľǵ������Ͻǵ����½ǵ�˳��
  //todo �Ż�����
  void computeCoordinate(std::string corners,					//��ȡ���̸�����
                         std::string coord = "calPoints_Coordinate.txt",			//������������ϵ
                         int x = 8, int y = 11);										//���̸���Ϣ

  //���ɵ��ƣ�ply��ʽ��txt��ʽ��������txt��ʽ
  void plyFileGenerate(std::vector<RestructPoint> pointCloud,	//3d����
                       std::string ply = "pointcloud.ply",				//ply�ļ�
                       std::string txtFile = "pointsDistance.txt",		//������txt�ļ�
                       std::string points = "points.txt");				//txt�ļ�

  //����߾�ĳ��ȷֲ�
  void resultEvaluate(std::vector<RestructPoint> result,		//�ؽ������̸��
                      std::string rlt = "_calLineLength.txt",					//�߶ε����
                      std::string dist = "_calLineLengthDist.txt",				//�߶����ֲ�
                      int x = 8, int y = 11, int sl = 30);							//���̸���Ϣ
};

#endif