#include "Utilities.h"

bool Utilities::XOR(bool val1, bool val2)
{
  if(val1==val2)
    return 0;
  else
    return 1;
}

void Utilities::normalize(cv::Vec3f &vec)	
{
  double mag = sqrt( vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
  
  vec[0] /= (float) max(0.000001, mag);
  vec[1] /= (float) max(0.000001, mag);
  vec[2] /= (float) max(0.000001, mag);
  
  return;
}

void Utilities::normalize3dtable(double vec[3])
{
  double mag = sqrt( vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
  
  vec[0] /= max(0.000001, mag);
  vec[1] /= max(0.000001, mag);
  vec[2] /= max(0.000001, mag);
}

void Utilities::pixelToImageSpace(double p[3], CvScalar fc, CvScalar cc)
{

  p[0]=(p[0]-cc.val[0])/fc.val[0];
  p[1]=(p[1]-cc.val[1])/fc.val[1];
  p[2]=1;

}
/*
cv::Point3f Utilities::pixelToImageSpace(cv::Point2f p,  VirtualCamera cam)
{
  cv::Point3f point;

  point.x = (p.x-cam.cc.x) / cam.fc.x;
  point.y = (p.y-cam.cc.y) / cam.fc.y;
  point.z = 1;
  
  return point;
}

cv::Point2f Utilities::undistortPoints( cv::Point2f p,  VirtualCamera cam)
{

    double  k[5]={0,0,0,0,0}, fx, fy, ifx, ify, cx, cy;
  
    int iters = 1;
  
  k[0] = cam.distortion.at<float>(0);
  k[1] = cam.distortion.at<float>(1);
  k[2] = cam.distortion.at<float>(2);
  k[3] = cam.distortion.at<float>(3);
  k[4]=0;

    iters = 5;

  fx = cam.fc.x; 
    fy = cam.fc.y; 
  
    ifx = 1./fx;
    ify = 1./fy;
    cx = cam.cc.x; 
    cy = cam.cc.y; 


  double x, y, x0, y0;

  x=p.x;
  y=p.y;

  x0 = x = (x - cx)*ifx;
  y0 = y = (y - cy)*ify;
      
  for(int jj = 0; jj < iters; jj++ )
  {
    double r2 = x*x + y*y;
    double icdist = 1./(1 + ((k[4]*r2 + k[1])*r2 + k[0])*r2);
    double deltaX = 2*k[2]*x*y + k[3]*(r2 + 2*x*x);
    double deltaY = k[2]*(r2 + 2*y*y) + 2*k[3]*x*y;
    x = (x0 - deltaX)*icdist;
    y = (y0 - deltaY)*icdist;
  }
  
  return cv::Point2f((float)(x*fx)+cx,(float)(y*fy)+cy);
}
*/
CvScalar Utilities::planeRayInter(CvScalar planeNormal,CvScalar planePoint, CvScalar rayVector, CvScalar rayPoint )
{
  double l;
  CvScalar point;

  CvScalar pSub;

  pSub.val[0] = - rayPoint.val[0] + planePoint.val[0];
  pSub.val[1] = - rayPoint.val[1] + planePoint.val[1];
  pSub.val[2] = - rayPoint.val[2] + planePoint.val[2];

  double dotProd1 = pSub.val[0] * planeNormal.val[0] + pSub.val[1] * planeNormal.val[1] + pSub.val[2] * planeNormal.val[2];
  double dotProd2 = rayVector.val[0] * planeNormal.val[0] + rayVector.val[1] * planeNormal.val[1] + rayVector.val[2] * planeNormal.val[2];
  
  if(fabs(dotProd2)<0.00001)
  {
    std::cout<<"Error 10\n";
    //getch();
    point.val[0]=0;
    point.val[1]=0;
    point.val[2]=0;
    return point;
  }

  l = dotProd1 / dotProd2;

  point.val[0] = rayPoint.val[0] + l * rayVector.val[0]; 
  point.val[1] = rayPoint.val[1] + l * rayVector.val[1]; 
  point.val[2] = rayPoint.val[2] + l * rayVector.val[2]; 

  return point;
}

double Utilities::matGet2D(cv::Mat m, int x, int y)
{
  int type = m.type();

  switch(type)
  {
    case CV_8U:
      return m.at<uchar>(y,x);
      break;
    case CV_8S:
      return m.at<schar>(y,x);
      break;
    case CV_16U:
      return m.at<ushort>(y,x);
      break;
    case CV_16S:
      return m.at<short>(y,x);
      break;
    case CV_32S:
      return m.at<int>(y,x);
      break;
    case CV_32F:
      return m.at<float>(y,x);
      break;
    case CV_64F:
      return m.at<double>(y,x);
      break;
  }
  return 0.0;
}

double Utilities::matGet3D(cv::Mat m, int x, int y, int i)
{
  int type = m.type();

  switch(type)
  {
    case CV_8U:
    case CV_MAKETYPE(CV_8U,3):
      return m.at<uchar>(y,x,i);
      break;
    case CV_8S:
    case CV_MAKETYPE(CV_8S,3):
      return m.at<schar>(y,x,i);
      break;
    case CV_16U:
    case CV_MAKETYPE(CV_16U,3):
      return m.at<ushort>(y,x,i);
      break;
    case CV_16S:
    case CV_MAKETYPE(CV_16S,3):
      return m.at<short>(y,x,i);
      break;
    case CV_32S:
    case CV_MAKETYPE(CV_32S,3):
      return m.at<int>(y,x,i);
      break;
    case CV_32F:
    case CV_MAKETYPE(CV_32F,3):
      return m.at<float>(y,x,i);
      break;
    case CV_64F:
    case CV_MAKETYPE(CV_64F,3):
      return m.at<double>(y,x,i);
      break;
  }
  return 0.0;
}

cv::Vec3d Utilities::matGet3D(cv::Mat m, int x, int y)
{
  int type = m.type();

  switch(type)
  {
    case CV_8U:
    case CV_MAKETYPE(CV_8U,3):
      return m.at<cv::Vec3b>(y,x);
      break;
    case CV_8S:
    case CV_MAKETYPE(CV_8S,3):
      return m.at<cv::Vec3b>(y,x);
      break;
    case CV_16U:
    case CV_MAKETYPE(CV_16U,3):
      return m.at<cv::Vec3w>(y,x);
      break;
    case CV_16S:
    case CV_MAKETYPE(CV_16S,3):
      return m.at<cv::Vec3s>(y,x);
      break;
    case CV_32S:
    case CV_MAKETYPE(CV_32S,3):
      return m.at<cv::Vec3i>(y,x);
      break;
    case CV_32F:
    case CV_MAKETYPE(CV_32F,3):
      return m.at<cv::Vec3f>(y,x);
      break;
    case CV_64F:
    case CV_MAKETYPE(CV_64F,3):
      return m.at<cv::Vec3d>(y,x);
      break;
  }
  return 0.0;
}

void Utilities::matSet2D(cv::Mat m, int x, int y, double val)
{
  int type = m.type();

  switch(type)
  {
    case CV_8U:
      m.at<uchar>(y,x)  = (uchar) val;
      break;
    case CV_8S:
      m.at<schar>(y,x)  = (schar) val;
      break;
    case CV_16U:
      m.at<ushort>(y,x) = (ushort) val;
      break;
    case CV_16S:
      m.at<short>(y,x)  = (short) val;
      break;
    case CV_32S:
      m.at<int>(y,x)	  = (int) val;
      break;
    case CV_32F:
      m.at<float>(y,x)  = (float) val;
      break;
    case CV_64F:
      m.at<double>(y,x) = (double) val;
      break;
  }

}

void Utilities::matSet3D(cv::Mat m, int x, int y,int i, double val)
{
  int type = m.type();

  switch(type)
  {
    case CV_8U:
    case CV_MAKETYPE(CV_8U,3):
      m.at<uchar>(y,x,i) = (uchar) val;
      break;
    case CV_8S:
    case CV_MAKETYPE(CV_8S,3):
      m.at<schar>(y,x,i) = (schar) val;
      break;
    case CV_16U:
    case CV_MAKETYPE(CV_16U,3):
      m.at<ushort>(y,x,i) = (ushort) val;
      break;
    case CV_16S:
    case CV_MAKETYPE(CV_16S,3):
      m.at<short>(y,x,i) = (short) val;
      break;
    case CV_32S:
    case CV_MAKETYPE(CV_32S,3):
      m.at<int>(y,x,i) = (int) val;
      break;
    case CV_32F:
    case CV_MAKETYPE(CV_32F,3):
      m.at<float>(y,x,i) = (float) val;
      break;
    case CV_64F:
    case CV_MAKETYPE(CV_64F,3):
      m.at<double>(y,x) = (double) val;
      break;
  }

}

void Utilities::matSet3D(cv::Mat m, int x, int y, cv::Vec3d val)
{
  int type = m.type();

  switch(type)
  {
    case CV_8U:
    case CV_MAKETYPE(CV_8U,3):
      m.at<cv::Vec3b>(y,x) =  val;
      break;
    case CV_8S:
    case CV_MAKETYPE(CV_8S,3):
      m.at<cv::Vec3b>(y,x) =  val;
      break;
    case CV_16U:
    case CV_MAKETYPE(CV_16U,3):
      m.at<cv::Vec3w>(y,x) = val;
      break;
    case CV_16S:
    case CV_MAKETYPE(CV_16S,3):
      m.at<cv::Vec3s>(y,x) = val;
      break;
    case CV_32S:
    case CV_MAKETYPE(CV_32S,3):
      m.at<cv::Vec3i>(y,x) = val;
      break;
    case CV_32F:
    case CV_MAKETYPE(CV_32F,3):
      m.at<cv::Vec3f>(y,x) = val;
      break;
    case CV_64F:
    case CV_MAKETYPE(CV_64F,3):
      m.at<cv::Vec3d>(y,x) = val;
      break;
  }

}

bool direction(cv::Point p1, cv::Point p2,cv::Point p3)
{
  int p = -p2.x*p1.y + p3.x*p1.y + p1.x*p2.y - p3.x*p2.y - p1.x*p3.y + p2.x*p3.y; 

  if(p<0)
    return false;
  else
    return true;
}


void Utilities::autoContrast(cv::Mat img_in, cv::Mat &img_out)
{

  double min=0,max=0;

  std::vector<cv::Mat> bgr;
  cv::split(img_in,bgr);

  for(int i=0; i<3; i++)
  {
    cv::minMaxIdx(bgr[i],&min,&max);
    min += 255*0.05;
      
    double a = 255/(max-min);
    bgr[i]-=min;
    bgr[i]*=a;
  }
  
  cv::merge(bgr,img_out);
}

void Utilities::autoContrast(IplImage *img_in, IplImage *img_out)
{
  
  cv::Mat tmp_in = img_in;
  cv::Mat tmp_out = img_out;

  autoContrast(tmp_in,tmp_out);
  
}

void Utilities::exportMat(const char *path, cv::Mat m)
{

  std:: ofstream out; 
  out.open(path);
  
  for(int i =0; i < m.rows; i++)
  {

    for(int j = 0; j < m.cols; j++)
    {

      out<< Utilities::matGet2D(m,j,i)<<"\t";

    }
    out<<"\n";
  }
  out.close();
}

bool Utilities::importMat(const char *path, cv::Mat m)
{
  std::ifstream in;
  in.open(path);

  for(int i=0; i<m.rows; i++)
  {
    for(int j=0; j<m.cols; j++)
    {
      double temp;
      in >> temp;
      Utilities::matSet2D(m, j, i, temp);
    }
  }
  in.close();
  return true;
}

float Utilities::line_lineIntersection(cv::Point3f p1, cv::Vec3f v1, cv::Point3f p2,cv::Vec3f v2,cv::Point3f &p)
{
  //test:
  //test end
  cv::Vec3f v12;
  v12 = p1 - p2;

  float v1_dot_v1 = v1.dot(v1);
  float v2_dot_v2 = v2.dot(v2);
  float v1_dot_v2 = v1.dot(v2); 
  float v12_dot_v1 = v12.dot(v1);
  float v12_dot_v2 = v12.dot(v2);

  float s, t, denom;

  //denom=sin(a)^2,a表示v1和v2夹角
  denom = v1_dot_v1 * v2_dot_v2 - v1_dot_v2 * v1_dot_v2;


  //s表示v1方向的距离
  s =  (v1_dot_v2/denom) * v12_dot_v2 - (v2_dot_v2/denom) * v12_dot_v1;
  //t表示v2方向的距离
  t = -(v1_dot_v2/denom) * v12_dot_v1 + (v1_dot_v1/denom) * v12_dot_v2;

  //p点为到2条异面直线的最近点
  p = (p1 + s*(cv::Point3f)v1 ) + (p2 + t*(cv::Point3f) v2);
  p = 0.5*p;

  //计算2条异面直线的距离
  cv::Point3f vp1 = p1 + s*(cv::Point3f)v1;
  cv::Point3f vp2 = p2 + t*(cv::Point3f) v2;
  float distance = std::sqrt( (vp1.x-vp2.x)*(vp1.x-vp2.x)+(vp1.y-vp2.y)*(vp1.y-vp2.y)+(vp1.z-vp2.z)*(vp1.z-vp2.z) );

  return distance;
}

int Utilities::accessMat(cv::Mat m, int x, int y, int i)
{
  
  return y*m.cols*m.channels() + x*m.channels() + i;

}

int Utilities::accessMat(cv::Mat m, int x, int y)
{
  
  return y*m.cols*m.channels() + x*m.channels();

}


void Utilities::folderScan(const char *path)
{	
  _chdir(path);

  WIN32_FIND_DATA data;
  HANDLE h;

  h = FindFirstFile( "*.*", &data);

  int numOfFiles=0;
  if( h!=INVALID_HANDLE_VALUE ) 
  {
    do
    {
      numOfFiles ++;
      //
      char*  nPtr = new char [lstrlen( data.cFileName ) + 1];

      for( int i = 0; i < lstrlen( data.cFileName ); i++ )
        nPtr[i] = char( data.cFileName[i] );

      nPtr[lstrlen( data.cFileName )] = '\0';

      std::cout<<nPtr<<"\n";

    } 
    while(FindNextFile(h,&data));

  }

  for(int i=0; path[i]!=NULL; i++)
  {
    if(path[i] == '/')
      _chdir("../");
  }
}

int Utilities::folderFilesScan(const char *path)
{	

  _chdir(path);

  WIN32_FIND_DATA data;
  HANDLE h;

  h = FindFirstFile( "*.*", &data);

  int numOfFiles=0;
  if( h!=INVALID_HANDLE_VALUE ) 
  {
    do
    {
      numOfFiles ++;
      //
      char*  nPtr = new char [lstrlen( data.cFileName ) + 1];

      for( int i = 0; i < lstrlen( data.cFileName ); i++ )
        nPtr[i] = char( data.cFileName[i] );

      nPtr[lstrlen( data.cFileName )] = '\0';

      std::cout<<nPtr<<"\n";

    } 
    while(FindNextFile(h,&data));

  }

  for(int i=0; path[i]!=NULL; i++)
  {
    if(path[i] == '/' || path[i] == '\\')
      _chdir("../");
  }
  return numOfFiles-2;
}

std::vector<std::string> Utilities::folderImagesScan(const char *path)
{	
  _chdir(path);

  WIN32_FIND_DATA data;
  HANDLE h;

  h = FindFirstFile( "*.jpg", &data);

  int numOfFiles=0;
  std::vector<std::string> ret;
  if( h!=INVALID_HANDLE_VALUE ) 
  {
    do
    {
      //by legendtkl
      numOfFiles ++;
      //
      char*  nPtr = new char [lstrlen( data.cFileName ) + 1];

      for( int i = 0; i < lstrlen( data.cFileName ); i++ )
        nPtr[i] = char( data.cFileName[i] );

      nPtr[lstrlen( data.cFileName )] = '\0';

      std::string img(nPtr);
      ret.push_back(img);
    } 
    while(FindNextFile(h,&data));

  }

  //返回到初始目录
  for(int i=0; path[i]!='\0'; i++)
  {
    if(path[i] == '/' || path[i] == '\\')
      _chdir("../");
  }
  return ret;
}
std::vector<std::string> Utilities::folderBmpImagesScan(const char *path)
{	
  _chdir(path);

  WIN32_FIND_DATA data;
  HANDLE h;

  h = FindFirstFile( "*.bmp", &data);

  int numOfFiles=0;
  std::vector<std::string> ret;
  if( h!=INVALID_HANDLE_VALUE ) 
  {
    do
    {
      //by legendtkl
      numOfFiles ++;
      //
      char*  nPtr = new char [lstrlen( data.cFileName ) + 1];

      for( int i = 0; i < lstrlen( data.cFileName ); i++ )
        nPtr[i] = char( data.cFileName[i] );

      nPtr[lstrlen( data.cFileName )] = '\0';

      std::string img(nPtr);
      ret.push_back(img);
    } 
    while(FindNextFile(h,&data));

  }

  //返回到初始目录
  for(int i=0; path[i]!='\0'; i++)
  {
    if(path[i] == '/' || path[i] == '\\')
      _chdir("../");
  }
  return ret;
}
std::vector<std::string> Utilities::folderTxtScan(const char *path)
{
  _chdir(path);

  WIN32_FIND_DATA data;
  HANDLE h;

  h = FindFirstFile( "*.txt", &data);

  int numOfFiles=0;
  std::vector<std::string> ret;
  if( h!=INVALID_HANDLE_VALUE ) 
  {
    do
    {
      //by legendtkl
      numOfFiles ++;
      //
      char*  nPtr = new char [lstrlen( data.cFileName ) + 1];

      for( int i = 0; i < lstrlen( data.cFileName ); i++ )
        nPtr[i] = char( data.cFileName[i] );

      nPtr[lstrlen( data.cFileName )] = '\0';

      std::string img(nPtr);
      ret.push_back(img);
    } 
    while(FindNextFile(h,&data));

  }

  //返回到初始目录
  for(int i=0; path[i]!='\0'; i++)
  {
    if(path[i] == '/' || path[i] == '\\')
      _chdir("../");
  }
  return ret;
}

float Utilities::distance(cv::Point3f p1, cv::Point3f p2)
{
  float x = p1.x - p2.x;
  float y = p1.y - p2.y;
  float z = p1.z - p2.z;

  return sqrt(x*x+y*y+z*z);
}

void Utilities::generateObjCorners(std::string filename,int XCorners, int YCorners,int size)
{
  std::ofstream obj(filename);

  int z = 0;
  for(int i=0; i<YCorners; i++)
  {
    int y = i*size;
    for(int j=0; j<XCorners; j++)
    {
      int x = j*size;
      obj << x << '\t' << y << '\t' << z << std::endl;
    }
  }
  obj.close();
}
std::vector<cv::Point3f> Utilities::generateChessboardCorners(std::string filename,int XCorners, int YCorners,int size)
{
  std::vector<cv::Point3f> ret;

  //目前只能支持XCorners为奇数,YCorners为奇数。
  generateObjCorners(filename,XCorners,YCorners,size);
  //载入生成的坐标
  std::vector<cv::Point3f> objCorners = load3dPoints(filename);

  //棋盘格边框的四个角，左上，右上，右下，左下
  cv::Point3f lu = objCorners[0] + cv::Point3f(-size*2,-size*2,0);
  cv::Point3f ru = objCorners[XCorners-1] + cv::Point3f(size*2,-size*2,0);
  cv::Point3f rd = objCorners[XCorners*YCorners-1] + cv::Point3f(size*2,size*2,0);
  cv::Point3f ld = objCorners[XCorners*YCorners-XCorners] + cv::Point3f(-size*2,size*2,0);
  ret.push_back(lu);
  ret.push_back(ru);
  ret.push_back(rd);
  ret.push_back(ld);

  //每2行生成一对角黑色块，生成所有的黑色方块
  for(int i=0;i<YCorners;i=i+2) 
  {
    for(int j=0;j<XCorners;j=j+2)
    {
      cv::Point3f cp = objCorners[i*XCorners+j];

      //左上块
      cv::Point3f lup1 = cp-cv::Point3f(size,size,0);
      cv::Point3f lup2 = cp-cv::Point3f(0,size,0);
      cv::Point3f lup3 = cp;
      cv::Point3f lup4 = cp-cv::Point3f(size,0,0);
      ret.push_back(lup1);
      ret.push_back(lup2);
      ret.push_back(lup3);
      ret.push_back(lup4);

      //右下块
      cv::Point3f rdp1 = cp;
      cv::Point3f rdp2 = cp+cv::Point3f(size,0,0);
      cv::Point3f rdp3 = cp+cv::Point3f(size,size,0);
      cv::Point3f rdp4 = cp+cv::Point3f(0,size,0);
      ret.push_back(rdp1);
      ret.push_back(rdp2);
      ret.push_back(rdp3);
      ret.push_back(rdp4);
    }
  }

  return ret;
}
cv::Mat Utilities::drawChessboardImg(std::string filename,int width, int height,std::vector<cv::Point2f>& corners,float scale)
{
  cv::Mat img(height*scale,width*scale,CV_8UC3,cv::Scalar(0,0,0));

  //绘制白色的边框
  std::vector<cv::Point> pts;
  for(int i=0;i<4;i++)
  {
    pts.push_back(corners[i]*scale);
  }
  cv::fillConvexPoly(img,pts,cv::Scalar(255,255,255));	
  
  //依次绘制每个黑色的方块
  for(int i=4;i<corners.size();i=i+4)
  {
    pts.clear();
    pts.push_back(corners[i]*scale);
    pts.push_back((corners[i+1]-cv::Point2f(1,0))*scale);
    pts.push_back((corners[i+2]-cv::Point2f(1,1))*scale);
    pts.push_back((corners[i+3]-cv::Point2f(0,1))*scale);

    cv::fillConvexPoly(img,pts,cv::Scalar(0,0,0));
  }

  cv::Mat dst(height,width,CV_8UC3);
  cv::resize(img,dst,dst.size(),0,0,CV_INTER_AREA);
  cv::imwrite(filename,dst);

  //返回绘制的图像
  return dst;
}
void Utilities::save2dPoints(std::string filename,std::vector<cv::Point2f> pts)
{
  std::ofstream obj(filename);

  for(int i=0; i<pts.size(); i++)
  {
    obj << pts[i].x << '\t' << pts[i].y << std::endl;
  }
  obj.close();
}
void Utilities::save2dPointsArray(std::string filename,std::vector<std::vector<cv::Point2f>> pts)
{
  std::ofstream obj(filename);
  //第一个数据记录共有多少组数据
  int N = pts.size();
  obj<<N<<std::endl;

  //第1到N记录每组数据的个数
  for(int i=0;i<N;i++)
  {
    obj<<pts[i].size()<<std::endl;
  }

  //依次记录所有的数据
  for(int i=0; i<N; i++)
  {
    for(int j=0;j<pts[i].size();j++)
    {
      obj << pts[i][j].x << '\t' << pts[i][j].y << std::endl;
    }
  }
  obj.close();
}

void Utilities::paraDistribute(std::vector<float> &para, std::string filename)
{
  int N = para.size();
  if(N==0)
  {
    std::cout << "Error: empty vector!\n" << std::endl;
    return;
  }

  std::ofstream out(filename.c_str());
  if(!out){
    std::cout << filename << " file open failed!\n" << std::endl;
    return;
  }
  for(int i=0; i<N; i++)
    out << para[i] << '\t';
  out << std::endl;

  std::sort(para.begin(), para.end());
  const int M = 50;
  double step = (para[N-1] - para[0])/M;
  double threshold = para[0];
  int pos=0;

  for(double ss = threshold;ss<=para[N-1];ss=ss+step)
  {
    int Count = 0;
    for(int i=0;i<N;i++)
    {
      if(para[i]>=ss && para[i]<ss+step)
      {
        Count++;
      }
    }
    out << ss << '\t' << Count << std::endl;
  }
  out.flush();
  out.close();
}

cv::Point3f Utilities::lineFit(std::vector<cv::Point3f>& pts)
{
  std::vector<float> line(6);
  cv::fitLine(
    pts,			
    line,			
    CV_DIST_L2,		
    0,				
    0.01,			
    0.01);			

  cv::Point3f dir(line[0],line[1],line[2]);
  
  return dir;
}
std::pair<cv::Point2f,cv::Point2f> Utilities::lineFit(std::vector<cv::Point2f>& pts)
{
  std::vector<float> line(4);
  cv::fitLine(
    pts,			
    line,			
    CV_DIST_L2,		
    0,				
    0.01,			
    0.01);		
  return std::make_pair(cv::Point2f(line[2],line[3]),cv::Point2f(line[0],line[1]));
}
std::vector<cv::Vec4f>	Utilities::load4dPoints(std::string filename)
{
  std::vector<cv::Vec4f> pts;
  cv::Vec4f point;

  std::ifstream obj(filename.c_str());
  while(obj >> point(0) >> point(1) >> point(2)>>point(3)){
    pts.push_back(point);
  }
  obj.close();

  return pts;
}
std::vector<cv::Point3f> Utilities::load3dPoints(std::string filename)
{
  std::vector<cv::Point3f> pts;
  cv::Point3f point;

  std::ifstream obj(filename.c_str());
  while(obj >> point.x >> point.y >> point.z){
    pts.push_back(point);
  }
  obj.close();

  return pts;
}
std::vector<cv::Point2f> Utilities::load2dPoints(std::string filename)
{
  std::vector<cv::Point2f> pts;
  cv::Point2f point;

  std::ifstream obj(filename.c_str());
  while(obj >> point.x >> point.y )
  {
    pts.push_back(point);
  }
  obj.close();

  return pts;
}
std::vector<std::vector<cv::Point2f>> Utilities::load2dPointsArray(std::string filename)
{
  std::vector<std::vector<cv::Point2f>> pts;
  cv::Point2f point;

  std::ifstream obj(filename.c_str());
  //载入总的数据总数
  int N;
  obj>>N;
  pts.resize(N);
  //载入每组的数据个数
  std::vector<int> arraySize;
  for(int i=0;i<N;i++)
  {
    int temp;
    obj>>temp;
    arraySize.push_back(temp);
  }
  //依次载入所有数据
  for(int i=0;i<N;i++)
  {
    for(int j=0;j<arraySize[i];j++)
    {
      obj >> point.x >> point.y;
      pts[i].push_back(point);
    }
  }
  obj.close();

  return pts;
}

void Utilities::savePly(std::vector<cv::Point3f>& pointCloud,std::string filename)
{
  std::ofstream cloud(filename);

  //头
  cloud << "ply" << std::endl;
  cloud << "format ascii 1.0"<<std::endl;
  cloud << "element vertex " << pointCloud.size() << std::endl;
  cloud << "property float x" << std::endl;
  cloud << "property float y" << std::endl;
  cloud << "property float z" << std::endl;
  cloud << "end_header" << std::endl;

  //顶点
  for(int i=0; i<pointCloud.size(); i++)
  {
    cloud << pointCloud[i].x << " " << pointCloud[i].y << " " << pointCloud[i].z << std::endl;
  }

  cloud.close();
}

cv::vector<cv::Point2f> Utilities::normalizeCorners(cv::vector<cv::Point2f> corners,int X,int Y)
{

  cv::Point2f p0 = corners[0];
  cv::Point2f p1 = corners[1];
  cv::Point2f center;
  for(int i=0;i<corners.size();i++)
  {
    center +=corners[i];
  }
  center = center*(1/(float)(X*Y));

  //处理非对称的情况，只存在左上或右下
  if(X != Y)
  {
    //p0在左上角
    if(p0.x<center.x)
    {
      return corners;
    }

    //p0在右下角
    if(p0.x>center.x)
    {
      //翻转即可
      cv::vector<cv::Point2f> ret(X*Y);
      for(int i=0;i<ret.size();i++)
      {
        ret[i] = corners[ret.size()-1-i];
      }
      return ret;
    }
  }
  else
  {
    //p0在左上角
    if(p0.x<center.x && p0.y<center.y)
    {
      return corners;
    }

    //p0在右上角
    if(p0.x>center.x && p0.y<center.y)
    {
      //只有X==Y的时候才能有这种情况
      cv::vector<cv::Point2f> ret(X*Y);
      int cur = 0;
      for(int col=X-1;col>=0;col--)
      {
        for(int row=0;row<Y;row++)
        {
          ret[row*X+col]= corners[cur];
          cur++;
        }
      }
      return ret;
    }

    //p0在左下角
    if(p0.x<center.x && p0.y>center.y)
    {
      //只有X==Y的时候才能有这种情况
      cv::vector<cv::Point2f> ret(X*Y);
      int cur = 0;
      for(int col=0;col<X;col++)
      {
        for(int row=Y-1;row>=0;row--)
        {
          ret[row*X+col]= corners[cur];
          cur++;
        }
      }

      return ret;
    }

    //p0在右下角
    if(p0.x>center.x && p0.y>center.y)
    {
      //翻转即可
      cv::vector<cv::Point2f> ret(X*Y);
      for(int i=0;i<ret.size();i++)
      {
        ret[i] = corners[ret.size()-1-i];
      }
      return ret;
    }
  }
  //不可能运行到这里
  std::cout<<"error"<<std::endl;
  return corners;
}
/*
void Utilities::drawImageCorners(std::string folder,int X,int Y)
{
  std::vector<std::string> images = folderImagesScan(folder.c_str());
  for(int i=0;i<images.size();i++)
  {
    std::string filename = images[i];
    filename = filename.substr(0,filename.size()-4);
    bool b=false;
    CameraCalibration::findCorners(filename,b,X,Y,true);
  }
}
*/