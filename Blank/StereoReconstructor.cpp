#include "StereoReconstructor.h"
#include "VirtualCamera.h"

StereoReconstructor::StereoReconstructor() {}

StereoReconstructor::~StereoReconstructor() {}

void StereoReconstructor::computeRT(std::string cam1Folder, std::string cam2Folder) {
  bool load1 = loadMatrixAndCoe(cam1Folder, camMatrix1, distCoeffs1);
  bool load2 = loadMatrixAndCoe(cam2Folder, camMatrix2, distCoeffs2);

  if (load1 == false || load2 == false) {
    std::cout << "Load matrix and distortion failed!" << std::endl;
    return;
  }

  load1 = loadImgAndObjPoints(cam1Folder, imgPoints1, objPoints);
  load2 = loadImgAndObjPoints(cam2Folder, imgPoints2, objPoints);

  if (load1 == false || load2 == false) {
    std::cout << "Load Points failed!" << std::endl;
    return;
  }

  //std::cout << camImageSize.width << '\t' << camImageSize.height << std::endl;
  cv::Mat E, F;
  cv::stereoCalibrate(
    objPoints,
    imgPoints1,
    imgPoints2,
    camMatrix1,
    distCoeffs1,
    camMatrix2,
    distCoeffs2,
    camImageSize,

    R, T,
    E, F,

    cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5), CV_CALIB_FIX_INTRINSIC);

  Utilities::exportMat("reconstruct\\R.txt", R);
  Utilities::exportMat("reconstruct\\T.txt", T);
  Utilities::exportMat("reconstruct\\F.txt", F);

  Utilities::exportMat("reconstruct\\LeftMatrix.txt", camMatrix1);
  Utilities::exportMat("reconstruct\\LeftDistortion.txt", distCoeffs1);
  Utilities::exportMat("reconstruct\\RightMatrix.txt", camMatrix2);
  Utilities::exportMat("reconstruct\\RightDistortion.txt", distCoeffs2);

  cv::Mat angle;
  cv::Rodrigues(R, angle);
  double x = angle.at<double>(0, 0);
  double y = angle.at<double>(1, 0);
  double z = angle.at<double>(2, 0);
  double length = std::sqrt(x*x + y*y + z*z);
  float degree = length * 180 / 3.1415926;
  std::ofstream A("reconstruct\\Rv.txt");
  A << degree << " " << x / length << " " << y / length << " " << z / length << std::endl;
  A.close();
}

void StereoReconstructor::computeRTRandom(std::string cam1Folder, std::string cam2Folder) {
  bool load1 = loadMatrixAndCoe(cam1Folder, camMatrix1, distCoeffs1);
  bool load2 = loadMatrixAndCoe(cam2Folder, camMatrix2, distCoeffs2);

  if (load1 == false || load2 == false) {
    std::cout << "Load matrix and distortion failed!" << std::endl;
    return;
  }

  std::vector<std::string> imgFiles1 = Utilities::folderImagesScan(cam1Folder.c_str());
  std::vector<std::string> imgFiles2 = Utilities::folderImagesScan(cam2Folder.c_str());

  cv::Mat img = cv::imread(cam1Folder + imgFiles1[0].c_str(), 1);
  camImageSize = img.size();

  int totalN = imgFiles1.size();

  std::ofstream Tx("����_T_x.txt");	//x����λ��
  std::ofstream Ty("����_T_y.txt");	//y����λ��
  std::ofstream Tz("����_T_z.txt");	//z����λ��
  std::ofstream Rx("����_R_x.txt");	//��ת��x����
  std::ofstream Ry("����_R_y.txt");	//��ת��y����
  std::ofstream Rz("����_R_z.txt");	//��ת��z����
  std::ofstream Ra("����_R_a.txt");	//��ת�Ƕ�
  std::ofstream All("����_All.txt");	//��������
  All << "Tx " << "Ty " << "Tz " << "T " << "Rx " << "Ry " << "Rz " << "Ra " << std::endl;
  for (int k = 1; k <= totalN; k++) {
    std::cout << k;
    double tbegin = cv::getTickCount();
    std::vector<int> nums;
    while (nums.size() < k) {
      srand(time(NULL));
      int random = rand() % totalN;
      if (nums.empty() || std::find(nums.begin(), nums.end(), random) == nums.end()) {
        nums.push_back(random);
      }
    }

    for (size_t i = 0; i < nums.size(); ++i) {
      imgPoints1.push_back(load2DPoints(cam1Folder + imgFiles1[nums[i]].substr(0, imgFiles1[nums[i]].size() - 4) + "_imgCorners.txt"));
      imgPoints2.push_back(load2DPoints(cam2Folder + imgFiles2[nums[i]].substr(0, imgFiles2[nums[i]].size() - 4) + "_imgCorners.txt"));
      objPoints.push_back(load3DPoints(cam1Folder + "ObjCorners.txt"));
    }
    cv::Mat E, F;
    cv::stereoCalibrate(objPoints, imgPoints1, imgPoints2, camMatrix1, distCoeffs1, camMatrix2, distCoeffs2, camImageSize, R, T, E, F,
                        cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5), CV_CALIB_FIX_INTRINSIC);

    //�޵����˹��Rodrigues���任
    cv::Mat R2T(3, 1, CV_64F);
    cv::Rodrigues(R, R2T);

    float tx = Utilities::matGet2D(T, 0, 0);
    float ty = Utilities::matGet2D(T, 0, 1);
    float tz = Utilities::matGet2D(T, 0, 2);
    float T = std::sqrt(tx*tx + ty*ty + tz*tz);
    Tx << tx << std::endl;
    Ty << ty << std::endl;
    Tz << tz << std::endl;
    double dx = Utilities::matGet2D(R2T, 0, 0);
    double dy = Utilities::matGet2D(R2T, 0, 1);
    double dz = Utilities::matGet2D(R2T, 0, 2);
    double dl = std::sqrt(dx*dx + dy*dy + dz*dz);
    float angle = dl * 180 / 3.1415926535897932;
    float fdx = dx / dl;
    float fdy = dy / dl;
    float fdz = dz / dl;
    Rx << fdx << std::endl;
    Ry << fdy << std::endl;
    Rz << fdz << std::endl;
    Ra << angle << std::endl;

    All << tx << " " << ty << " " << tz << " " << T << " " << fdx << " " << fdy << " " << fdz << " " << angle << std::endl;

    nums.clear();
    imgPoints1.clear();
    imgPoints2.clear();
    objPoints.clear();
    double tend = cv::getTickCount();
    double frequency = cv::getTickFrequency();
    int span = (tend - tbegin) / frequency;
    std::cout << "��ͼ��,�궨ʱ��	" << span << ",���߳���" << T << std::endl;
  }

  Tx.close();
  Ty.close();
  Tz.close();
  Rx.close();
  Ry.close();
  Rz.close();
  Ra.close();
  All.close();
}

bool StereoReconstructor::loadImgAndObjPoints(std::string cam, std::vector<std::vector<cv::Point2f>> &imgPoints, std::vector<std::vector<cv::Point3f>> &objPoints) {
  bool flag = false;
  if (objPoints.empty())
    flag = true;

  std::vector<std::string> imgFiles = Utilities::folderImagesScan(cam.c_str());

  cv::Mat img = cv::imread(cam + imgFiles[0].c_str(), 1);
  camImageSize = img.size();

  for (size_t i = 0; i != imgFiles.size(); ++i) {
    imgPoints.push_back(load2DPoints(cam + imgFiles[i].substr(0, imgFiles[i].size() - 4) + "_imgCorners.txt"));
    if (flag)
      objPoints.push_back(load3DPoints(cam + "ObjCorners.txt"));
  }
  if (imgPoints.empty() || objPoints.empty())
    return false;
  else
    return true;
}

std::vector<cv::Point2f> StereoReconstructor::load2DPoints(std::string file) {
  std::vector<cv::Point2f> ret;
  std::ifstream out(file.c_str());
  if (!out) {
    std::cout << file << " open failed!" << std::endl;
    return ret;
  }

  cv::Point2f p;
  while (out >> p.x >> p.y)
    ret.push_back(p);
  out.close();
  return ret;
}

std::vector<cv::Point3f> StereoReconstructor::load3DPoints(std::string file) {
  std::vector<cv::Point3f> ret;
  std::ifstream out(file.c_str());
  if (!out) {
    std::cout << file << " open failed!" << std::endl;
    return ret;
  }

  cv::Point3f p;
  while (out >> p.x >> p.y >> p.z)
    ret.push_back(p);
  out.close();
  return ret;
}

void StereoReconstructor::RestructCalibrationPointsFromImage(std::string leftFilename, std::string rightFilename, int x, int y, int squareLength) {
  //����ǵ�
  bool f1, f2;
  CameraCalibration::findCorners(leftFilename, f1, x, y);
  CameraCalibration::findCorners(rightFilename, f2, x, y);
  
  if (!f1 || !f2) {
    std::cout << "�ǵ�û�ҵ�" << std::endl;
    return;
  }

  //����2�����������
  VirtualCamera vc1;
  VirtualCamera vc2;
  //�����ڲ�
  vc1.loadCameraMatrix("reconstruct\\LeftMatrix.txt");
  vc1.loadDistortion("reconstruct\\LeftDistortion.txt");
  vc1.rotationMatrix = cv::Mat::eye(3, 3, CV_32FC1);
  vc1.translationVector = cv::Mat::zeros(3, 1, CV_32FC1);
  vc2.loadCameraMatrix("reconstruct\\RightMatrix.txt");
  vc2.loadDistortion("reconstruct\\RightDistortion.txt");
  vc2.loadRotationMatrix("reconstruct\\R.txt");
  vc2.loadTranslationVector("reconstruct\\T.txt");

  //����ǵ�	
  vc1.loadKeyPoints(leftFilename + "_imgCorners.txt");
  vc2.loadKeyPoints(rightFilename + "_imgCorners.txt");

  //�ؽ�3d��
  std::vector<RestructPoint> ret;
  ret = triangulation(vc1, vc2, true);	//���л������,false��ʾ������

  //������
  //���ɵ����ļ�
  plyFileGenerate(ret,
                  leftFilename + ".ply",		//ply�ļ�
                  "_calPointsDis.txt",		//3d��+�ཻ����
                  "_calPoints.txt");			//3d��
  //�����������
  resultEvaluate(ret,
                 "_calLineLength.txt",		//157���߶ε����calLineLength.txt
                 "_calLineLengthDist.txt",	//�߶εĳ��ȵĸ��ʷֲ�
                 x, y, squareLength);
  //�������̸�����ϵ
  computeCoordinate(
    "_calPoints.txt",			//��ȡ���̸�����
    "_coordinate.txt",			//��������ϵ�����浽Coordinate.txt
    x, y);						//ˮƽ����ʹ�ֱ����Ľǵ����
  //ת�������̸�����ϵ
  translateCoordinate(
    "_coordinate.txt",			//���̸�����ϵ
    "_calPoints.txt",			//Ҫת����3D��
    "_calPoints_ObjSpace.txt");	//�����̸�����ϵ�µ�����calPoints_ObjSpace.txt

  //������������ϵ�µ�ply�ļ�
  std::vector<cv::Point3f> pointcloud = Utilities::load3dPoints("_calPoints_ObjSpace.txt");
  Utilities::savePly(pointcloud, "ObjSpace" + leftFilename + ".ply");

  //�������̸�����ռ����꣬������
  Utilities::generateObjCorners("_chessbordCorner.txt", x, y, squareLength);
  std::vector<cv::Point3f> ideaPts = Utilities::load3dPoints("_chessbordCorner.txt");
  std::vector<cv::Point3f> realPts = Utilities::load3dPoints("_calPoints_ObjSpace.txt");
  //���㵽�ǵ�ľ��Ծ��룬������
  std::ofstream absDis("_absCornerDistance.txt");
  for (int i = 0; i < ideaPts.size(); i++) {
    cv::Point3f idea = ideaPts[i];
    cv::Point3f real = realPts[i];
    float dis = cv::norm((idea - real));
    absDis << dis << std::endl;
  }
  absDis.close();

}
void StereoReconstructor::RestructPointCloud(bool objSpace, bool undistort, std::string pcfilename) {
  //����2�����������
  VirtualCamera vc1;
  VirtualCamera vc2;
  //�����ڲ�
  vc1.loadCameraMatrix("reconstruct\\LeftMatrix.txt");
  vc1.loadDistortion("reconstruct\\LeftDistortion.txt");
  vc1.rotationMatrix = cv::Mat::eye(3, 3, CV_32FC1);
  vc1.translationVector = cv::Mat::zeros(3, 1, CV_32FC1);
  vc2.loadCameraMatrix("reconstruct\\RightMatrix.txt");
  vc2.loadDistortion("reconstruct\\RightDistortion.txt");
  vc2.loadRotationMatrix("reconstruct\\R.txt");
  vc2.loadTranslationVector("reconstruct\\T.txt");

  vc1.loadKeyPoints("left.txt");					//����ǵ�
  vc2.loadKeyPoints("right.txt");

  //�ؽ�3d��
  std::vector<RestructPoint> ret;
  ret = triangulation(vc1, vc2, undistort);	//true���л������,false��ʾ������

  //������
  std::vector<StereoReconstructor::RestructPoint> ans;
  for (size_t i = 0; i != ret.size(); ++i)
    ans.push_back(StereoReconstructor::RestructPoint(ret[i].point, ret[i].distance));

  plyFileGenerate(ans,
                  pcfilename,					//ply�ļ�
                  "_calPointsDis.txt",			//3d��+�ཻ����
                  "_calPoints.txt");				//3d��

  //ת������������ϵ
  if (objSpace) {
    translateCoordinate(
      "Coordinate.txt",				//���̸�����ϵ
      "_calPoints.txt",				//Ҫת����3D��
      "Points_obj.txt");				//�����̸�����ϵ�µ�����calPoints_ObjSpace.txt

    //������������ϵ�µ�ply�ļ�
    std::vector<cv::Point3f> pointcloud = Utilities::load3dPoints("Points_obj.txt");
    Utilities::savePly(pointcloud, "PointsObj.ply");
  }
}

std::vector<StereoReconstructor::RestructPoint>
StereoReconstructor::triangulation(
    VirtualCamera& camera1, VirtualCamera& camera2, bool undistortPoints) {
  if (camera1.keypoints.size() != camera2.keypoints.size()) {
    std::cout << "keypoints not match!" << std::endl;
    std::cout << camera1.keypoints.size() << std::endl;
    std::cout << camera2.keypoints.size() << std::endl;
  }

  //��д��ĵ�������
  std::vector<RestructPoint> pointCloud;

  //����2���������ԭ��
  camera1.calCameraPosition();
  camera2.calCameraPosition();

  //��ȥ����
  if (undistortPoints) {
    std::vector<cv::Point2f> kp1, kp2, ukp1, ukp2;
    for (int i = 0; i < camera1.keypoints.size(); i++) {
      kp1.push_back(cv::Point2f(camera1.keypoints[i].first, camera1.keypoints[i].second));
      kp2.push_back(cv::Point2f(camera2.keypoints[i].first, camera2.keypoints[i].second));
    }
    ukp1 = camera1.undistortPointsImageSpace(kp1);
    ukp2 = camera2.undistortPointsImageSpace(kp2);
    for (int i = 0; i < camera1.keypoints.size(); i++) {
      camera1.keypoints[i].first = ukp1[i].x;
      camera1.keypoints[i].second = ukp1[i].y;

      camera2.keypoints[i].first = ukp2[i].x;
      camera2.keypoints[i].second = ukp2[i].y;
    }
  }

  //����ƥ��Ľ���
  for (int i = 0; i < camera1.keypoints.size(); i++) {
    cv::Point2f camPixelUD;
    camPixelUD = cv::Point2f(camera1.keypoints[i].first, camera1.keypoints[i].second);

    //�任����ƽ��=1�����������ϵ�����������꣬�任����3d���꣩
    cv::Point3f cam1Point = camera1.pixelToImageSpace(camPixelUD);
    //�任����������ϵ
    cam1Point = camera1.cam2WorldSpace(cam1Point);
    //�õ�1������
    cv::Vec3f ray1Vector = (cv::Vec3f) (camera1.position - cam1Point);
    Utilities::normalize(ray1Vector);

    //���Ӧ��ĵ�2������
    camPixelUD = cv::Point2f(camera2.keypoints[i].first, camera2.keypoints[i].second);
    cv::Point3f cam2Point = camera2.pixelToImageSpace(camPixelUD);
    cam2Point = camera2.cam2WorldSpace(cam2Point);
    cv::Vec3f ray2Vector = (cv::Vec3f) (camera2.position - cam2Point);
    Utilities::normalize(ray2Vector);

    cv::Point3f interPoint;
    float dis = Utilities::line_lineIntersection(camera1.position, ray1Vector, camera2.position, ray2Vector, interPoint);

    RestructPoint temp;
    temp.point = interPoint;
    temp.distance = dis;
    pointCloud.push_back(temp);
  }

  return pointCloud;
}

void StereoReconstructor::resultEvaluate(std::vector<StereoReconstructor::RestructPoint> result, std::string rlt, std::string dist, int x, int y, int sl) {
  std::vector<float> distance;
  //�������ľ���
  for (int i = 0; i < y; i++) {
    for (int j = 0; j < x - 1; j++) {
      cv::Point3f p1 = result[i*x + j].point;
      cv::Point3f p2 = result[i*x + j + 1].point;
      float d = cv::norm(p1 - p2);
      distance.push_back(d);
    }
  }

  //��������ľ���
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y - 1; j++) {
      cv::Point3f p1 = result[j*x + i].point;
      cv::Point3f p2 = result[(j + 1)*x + i].point;
      float d = cv::norm(p1 - p2);
      distance.push_back(d);
    }
  }

  //����������ֵ��30mm
  std::ofstream ret(rlt);
  for (std::vector<float>::size_type ix = 0; ix != distance.size(); ++ix)
    ret << distance[ix] - (float)sl << std::endl;
  ret.close();

  //���ɸ��ʷֲ�
  Utilities::paraDistribute(distance, dist);
}

void StereoReconstructor::computeCoordinate(std::string corners, std::string coord, int x, int y) {
  //��ȡ���еĽǵ�
  std::ifstream file(corners.c_str());
  std::vector<cv::Point3f> corner;
  corner.reserve(x*y);
  cv::Point3f point;
  while (file >> point.x >> point.y >> point.z) {
    corner.push_back(point);
  }
  file.close();

  //����x����
  cv::Point3f pd = corner[x - 1] - corner[0];
  cv::Point3f xdir(0, 0, 0);
  for (int i = 0; i < y; i++) {
    std::vector<cv::Point3f> tps;
    //��ȡx����ĵ�
    for (int j = 0; j < x; j++) {
      cv::Point3f tp = corner[i*x + j];
      tps.push_back(tp);
    }
    cv::Point3f td = Utilities::lineFit(tps);
    xdir += td;
  }
  xdir.x = xdir.x / y;
  xdir.y = xdir.y / y;
  xdir.z = xdir.z / y;
  if (pd.dot(xdir) < 0) {
    xdir = -xdir;
  }
  double xv[] = {xdir.x, xdir.y, xdir.z};
  double normal = sqrt(xv[0] * xv[0] + xv[1] * xv[1] + xv[2] * xv[2]);
  xv[0] /= normal;
  xv[1] /= normal;
  xv[2] /= normal;

  //����y����
  cv::Point3f pyd = corner[x] - corner[0];
  cv::Point3f ydir(0, 0, 0);
  for (int i = 0; i < x; i++) {
    std::vector<cv::Point3f> tps;
    //��ȡy����ĵ�
    for (int j = 0; j < y; j++) {
      cv::Point3f tp = corner[j*x + i];
      tps.push_back(tp);
    }
    cv::Point3f td = Utilities::lineFit(tps);
    ydir += td;
  }
  ydir.x = ydir.x / x;
  ydir.y = ydir.y / x;
  ydir.z = ydir.z / x;
  if (pyd.dot(ydir) < 0) {
    ydir = -ydir;
  }
  double yv[] = {ydir.x, ydir.y, ydir.z};
  normal = sqrt(yv[0] * yv[0] + yv[1] * yv[1] + yv[2] * yv[2]);
  yv[0] /= normal;
  yv[1] /= normal;
  yv[2] /= normal;

  //��������ϵ
  double zv[3];
  zv[0] = xv[1] * yv[2] - xv[2] * yv[1];
  zv[1] = xv[2] * yv[0] - xv[0] * yv[2];
  zv[2] = xv[0] * yv[1] - xv[1] * yv[0];

  //���浽����ϵ�ļ�
  std::ofstream coordinate(coord);
  coordinate << corner[0].x << '\t' << corner[0].y << '\t' << corner[0].z << std::endl;
  coordinate << xv[0] << '\t' << xv[1] << '\t' << xv[2] << std::endl;
  coordinate << yv[0] << '\t' << yv[1] << '\t' << yv[2] << std::endl;
  coordinate << zv[0] << '\t' << zv[1] << '\t' << zv[2] << std::endl;
  coordinate.close();
}

void StereoReconstructor::translateCoordinate(std::string cordsys, std::string points, std::string newPoints) {
  std::ifstream cord(cordsys.c_str());
  if (!cord) {
    std::cout << cordsys << " file open failed!" << std::endl;
    return;
  }
  std::vector<cv::Point3f> coord;
  cv::Point3f p;
  while (cord >> p.x >> p.y >> p.z) {
    coord.push_back(p);
  }

  std::ifstream point(points.c_str());
  if (!point) {
    std::cout << points << " file open failed!" << std::endl;
    return;
  }

  std::vector<cv::Point3f> pointsv;
  while (point >> p.x >> p.y >> p.z) {
    pointsv.push_back(p);
  }

  std::ofstream newpoint(newPoints);
  if (!newpoint) {
    std::cout << "new_points.txt open failed" << std::endl;
    return;
  }
  for (size_t i = 0; i != pointsv.size(); i++) {
    cv::Point3f p = coordinateChange(pointsv[i], coord);
    newpoint << p.x << '\t' << p.y << '\t' << p.z << std::endl;
  }
  newpoint.close();
}

bool StereoReconstructor::loadMatrixAndCoe(
  std::string cam,	//������ļ���
  cv::Mat &mat1,		//�ڲ�
  cv::Mat &mat2)		//����ϵ��
{
  cv::Mat matrix(3, 3, CV_64FC1);
  cv::Mat distort(1, 5, CV_64FC1);


  if (Utilities::importMat((cam + "matrix.txt").c_str(), matrix) &&
      Utilities::importMat((cam + "distortion.txt").c_str(), distort)) {
    mat1 = matrix;
    mat2 = distort;
    return true;
  } else
    return false;
}

//ת������һ������ϵ������
cv::Point3f StereoReconstructor::coordinateChange(cv::Point3f p1, //��������
                                                  std::vector<cv::Point3f> cord)					//���꣬ԭ�㣬x����y����z����
{
  cv::Point3f ret;
  p1.x -= cord[0].x;
  p1.y -= cord[0].y;
  p1.z -= cord[0].z;

  ret.x = p1.x*cord[1].x + p1.y*cord[1].y + p1.z*cord[1].z;
  ret.y = p1.x*cord[2].x + p1.y*cord[2].y + p1.z*cord[2].z;
  ret.z = p1.x*cord[3].x + p1.y*cord[3].y + p1.z*cord[3].z;

  return ret;
}

//���ɵ��ƣ�ply��ʽ��txt��ʽ��������txt��ʽ
void StereoReconstructor::plyFileGenerate(std::vector<RestructPoint> pointCloud,	//3d����
                                          std::string ply,				//ply�ļ�
                                          std::string txtFile,		//������txt�ļ�
                                          std::string points)				//txt�ļ�
{
  std::ofstream cloud(ply);
  std::ofstream txtfile(txtFile);
  std::ofstream d3file(points);

  //ͷ
  cloud << "ply" << std::endl;
  cloud << "format ascii 1.0" << std::endl;
  cloud << "element vertex " << pointCloud.size() << std::endl;
  cloud << "property float x" << std::endl;
  cloud << "property float y" << std::endl;
  cloud << "property float z" << std::endl;
  cloud << "end_header" << std::endl;

  //����
  for (int i = 0; i < pointCloud.size(); i++) {
    cloud << pointCloud[i].point.x << " " << pointCloud[i].point.y << " " << pointCloud[i].point.z << std::endl;
    txtfile << pointCloud[i].point.x << " " << pointCloud[i].point.y << " " << pointCloud[i].point.z << " " << pointCloud[i].distance << std::endl;
    d3file << pointCloud[i].point.x << " " << pointCloud[i].point.y << " " << pointCloud[i].point.z << std::endl;
  }

  cloud.close();
  txtfile.close();
  d3file.close();
}