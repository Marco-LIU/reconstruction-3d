#include "CameraCalibration.h"
#include "windows.h"

#define CAMCALIB_OUT_MATRIX 1
#define CAMCALIB_OUT_DISTORTION 2

CameraCalibration::CameraCalibration(std::string fd, int _XCorners, int _YCorners,int sl)
{
	//扫描所有jpg图像文件
	imgFiles = Utilities::folderImagesScan(fd.c_str());
	folder = fd;
	numOfCamImgs = imgFiles.size();
	XCorners = _XCorners;
	YCorners = _YCorners;
	squareLength = sl;
	numOfCorners = _XCorners*_YCorners;

	cv::Mat img = cv::imread(folder+imgFiles[0].c_str());
	camImageSize = img.size();
}
CameraCalibration::~CameraCalibration(void)
{

}
//-----------------------------------计算棋盘格角点坐标------------------
cv::vector<cv::Point2f> CameraCalibration::findCorners(cv::Mat img_grey, int X, int Y) {
  cv::vector<cv::Point2f> cCam;
  bool found = cv::findChessboardCorners(img_grey, cvSize(X, Y), cCam, CV_CALIB_CB_ADAPTIVE_THRESH);
  if (!found) {
    return cv::vector<cv::Point2f>();
  } else {
    cv::cornerSubPix(img_grey, cCam, cvSize(20, 20), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
    return cCam;
  }
}

void CameraCalibration::findCorners(std::string filename, bool &found, int X, int Y,bool debug)
{
	//读取并转换为灰度图,todo 如果是黑白图像，有可能降低对比度
	std::string imgfile = filename+".jpg";
	cv::Mat img = cv::imread(imgfile);
	cv::vector<cv::Point2f> cCam;
	cv::Mat img_grey;
	cv::cvtColor(img, img_grey, CV_RGB2GRAY);

	//找角点
	found=cv::findChessboardCorners(img_grey, cvSize(X,Y), cCam, CV_CALIB_CB_ADAPTIVE_THRESH );
	if(!found)
		return;
	if(found)
	{
		//继续找亚像素坐标
		cv::cvtColor( img, img_grey, CV_RGB2GRAY );
		cv::cornerSubPix(img_grey, cCam, cvSize(20,20), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
	}
	
	//确保角点的顺序，从上到下，从左到右
	cv::vector<cv::Point2f> ncorners = Utilities::normalizeCorners(cCam,X,Y);

	//记录角点到文件
	std::string cornersfile = filename+"_imgCorners.txt";
	std::ofstream out(cornersfile);
	for(int i=0; i<ncorners.size(); i++)
	{
		if(i!=ncorners.size()-1)
			out << ncorners[i].x << '\t' << ncorners[i].y << std::endl;
		else
			out << ncorners[i].x << '\t' << ncorners[i].y;
	}
	out.close();

	if(debug)
	{
		//绘制调试图像输出
		cv::Mat imgCopy = img.clone();
		cv::drawChessboardCorners(imgCopy,cvSize(X,Y),ncorners,found);
		cv::circle(imgCopy,ncorners[0],15,cv::Scalar(255,255,255));
		cv::imwrite(filename+"_Corner.bmp",imgCopy);
	}
}
void CameraCalibration::findCorners(bool debug)
{
	//不是每张图像都能找到角点
	std::vector<std::string> todelete;

	//遍历每一张图像，如果成功找到所有的角点，把角点坐标记录下来
	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); ++i)
	{
		static int n=1;
		std::cout<<n++<<"	"<<imgFiles[i]<<std::endl;
		
		//1.jpg传入1，所以size-4
		std::string curPicName = folder+imgFiles[i].substr(0, imgFiles[i].size()-4);
		//如果角点已经找过了，跳过这个角点的查询
		std::string temp = curPicName+"_imgCorners.txt";
		std::ifstream tf;
		tf.open(temp);
		bool exist = tf.is_open();
		if(exist)
		{
			tf.close();
			continue;
		}

		bool found=false;
		findCorners(curPicName, found, XCorners, YCorners,debug);
		if(!found)
		{
			todelete.push_back(imgFiles[i]);
		}
	}

	//删除操作的什么太麻烦,直接在生成一个
	std::vector<std::string> temp;
	for(int i=0;i<imgFiles.size();i++)
	{
		std::string filename = imgFiles[i];
		bool add= true;
		for(int j=0;j<todelete.size();j++)
		{
			if(filename==todelete[j])
			{
				//如果要删除，则不添加
				add =false;
				break;
			}
		}
		if(add)
		{
			temp.push_back(filename);
		}
	}
	std::sort(temp.begin(),temp.end());
	imgFiles=temp;
	//把文件删除
	for(int j=0;j<todelete.size();j++)
	{
		std::string filename = todelete[j];
		std::string cmd = "del \"" + folder+filename +"\"";
		system(cmd.c_str());
	}
}
//----------------------------------载入角点坐标--------------------------
void CameraCalibration::loadCorrespondCorners()
{
	//默认的角点文件名
	std::string CORNERS="ObjCorners.txt";

	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); ++i)
	{
		std::string imgCornersFile = folder+imgFiles[i].substr(0, imgFiles[i].size()-4) + "_imgCorners.txt";
		std::string objCornersFile = folder+CORNERS;
		
		loadCorrespondCorners(imgCornersFile,objCornersFile);
	}
}
void CameraCalibration::loadCorrespondCorners(std::string imgCornersFile, std::string objCornersFile)
{
	//载入物体空间坐标
	std::ifstream obj(objCornersFile.c_str());
	if(!obj)
	{
		Utilities::generateObjCorners(objCornersFile,XCorners,YCorners,squareLength);
		obj.open(objCornersFile);
		if(!obj)
		{
			//一般不会运行到这里
			std::cout<<"载入棋盘格坐标文件ObjCorners.txt错误"<<std::endl;
			return;
		}
	}
	std::vector<cv::Point3f> cObj;
	int count=0;
	cv::Point3f point;
	while(obj >> point.x >> point.y >> point.z){
		cObj.push_back(point);
	}
	ObjCorners.push_back(cObj);
	
	//载入找到的棋盘格角点
	std::ifstream cam(imgCornersFile.c_str());
	if(!cam)
	{
		std::cout << imgCornersFile << "角点坐标文件载入错误" << std::endl;
		return;
	}
	std::vector<cv::Point2f> cCam;
	cv::Point2f cpoint;
	while(cam >> cpoint.x >> cpoint.y){
		cCam.push_back(cpoint);
	}
	CamCorners.push_back(cCam);
}
void CameraCalibration::loadCorrectCorrespondCorners(float lf,float re)
{
	//默认的角点文件名
	std::string CORNERS="ObjCorners.txt";

	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); ++i)
	{
		std::string imgCornersFile = folder+imgFiles[i].substr(0, imgFiles[i].size()-4) + "_imgCornersLineFit.txt";
		std::string objCornersFile = folder+CORNERS;
		
		int N = loadCorrectCorrespondCorners(imgCornersFile,objCornersFile,lf,re);
		std::cout<<imgFiles[i].substr(0, imgFiles[i].size()-4)<<"	"<<N<<std::endl;
	}
}
int CameraCalibration::loadCorrectCorrespondCorners(std::string imgCornersFile, std::string objCornersFile,float lf,float re)
{
	//载入物体空间坐标
	std::ifstream obj(objCornersFile.c_str());
	if(!obj)
	{
		Utilities::generateObjCorners(objCornersFile,XCorners,YCorners,squareLength);
		obj.open(objCornersFile);
		if(!obj)
		{
			//一般不会运行到这里
			std::cout<<"载入棋盘格坐标文件ObjCorners.txt错误"<<std::endl;
			return 0;
		}
	}
	std::vector<cv::Point3f> cObj;
	
	cv::Point3f point;
	while(obj >> point.x >> point.y >> point.z){
		cObj.push_back(point);
	}
	
	//载入找到的棋盘格角点
	std::ifstream cam(imgCornersFile.c_str());
	if(!cam)
	{
		std::cout << imgCornersFile << "角点坐标文件载入错误" << std::endl;
		return 0;
	}
	std::vector<cv::Vec4f> pts;
	cv::Vec4f pt;
	while(cam >> pt(0) >> pt(1) >> pt(2)>>pt(3)){
		pts.push_back(pt);
	}

	std::vector<cv::Point3f> tObj;
	std::vector<cv::Point2f> tCam;
	//载入误差较小的匹配点
	int count=0;
	for(int i=0;i<pts.size();i++)
	{
		if(/*pts[i](2)<lf &&*/ pts[i](3)<re)
		{
			count++;
			tObj.push_back(cObj[i]);
			tCam.push_back(cv::Point2f(pts[i](0),pts[i](1)));
		}
	}
	
	/*
	//如果错误点太多，舍弃这张图
	if(count < pts.size()*0.33)
		return 0;

	
	static int ct = 0;
	ct++;
	std::cout<<"载入"<<ct<<"对匹配点"<<std::endl;
	*/
	
	ObjCorners.push_back(tObj);
	CamCorners.push_back(tCam);

	return count;
}
void CameraCalibration::loadIdealCorrespondCorners()
{
	//默认的角点文件名
	std::string CORNERS="ObjCorners.txt";

	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); ++i)
	{
		std::string imgCornersFile = folder+imgFiles[i].substr(0, imgFiles[i].size()-4) + "_imgCornersIdeal.txt";
		std::string objCornersFile = folder+CORNERS;
		
		loadCorrespondCorners(imgCornersFile,objCornersFile);
	}
}

//-----------------------------------内参标定-----------------------
void CameraCalibration::cameraCalibration(bool saveExtr)
{
	cv::vector<cv::Mat> camRotationVectors;
  	cv::vector<cv::Mat> camTranslationVectors;
	
	//目前的镜头最好使用k1,k2,p1和p2
	double error = cv::calibrateCamera(
		ObjCorners, 
		CamCorners, 
		camImageSize, 
		camMatrix, 
		distortion, 
		camRotationVectors,camTranslationVectors,
		CV_CALIB_FIX_ASPECT_RATIO+CV_CALIB_FIX_K3, 
		cv::TermCriteria( (cv::TermCriteria::COUNT)+(cv::TermCriteria::EPS), 100, DBL_EPSILON) );
	std::cout<<"总标定误差:"<<error<<std::endl;

	//保存外参
	if(saveExtr)
	{
		for(int i=0;i<imgFiles.size();i++)
		{
			std::string tFileName = folder+imgFiles[i].substr(0,imgFiles[i].size()-4);
			std::string rotFileName = tFileName+"_calRotation.txt";
			std::string transFileName = tFileName+"_calTranslation.txt";

			cv::Mat rotationMatrix;
			cv::Rodrigues(camRotationVectors[i],rotationMatrix);

			Utilities::exportMat(rotFileName.c_str(),rotationMatrix );
			Utilities::exportMat(transFileName.c_str(), camTranslationVectors[i]);
		}

		std::ofstream info(folder+"info.txt");
		info<<error<<" ";						//平均反投影误差,单位像素
		info<<camMatrix.at<double>(0,0)<<" ";	//f
		info<<camMatrix.at<double>(0,2)<<" ";	//cx
		info<<camMatrix.at<double>(1,2)<<" ";	//cy
		info<<distortion.at<double>(0,0)<<" ";	//k1
		info<<distortion.at<double>(0,1)<<" ";	//k2
		info<<distortion.at<double>(0,2)<<" ";	//p1
		info<<distortion.at<double>(0,3)<<" ";	//p2
		info.close();
	}
	exportTxtFiles(folder+"matrix.txt", CAMCALIB_OUT_MATRIX);
	exportTxtFiles(folder+"distortion.txt", CAMCALIB_OUT_DISTORTION);
}
void CameraCalibration::cameraCalibrationCustom(int k ,cv::Mat matrix)
{
	int flag = CV_CALIB_FIX_ASPECT_RATIO;
	//k=0为true表示不考虑畸变
	if(k==0)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST+CV_CALIB_FIX_K1+CV_CALIB_FIX_K2+CV_CALIB_FIX_K3;
	}
	//k=1为true表示只使用k1
	if(k==1)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST+CV_CALIB_FIX_K2+CV_CALIB_FIX_K3;
	}
	//k=2为true表示使用k1,k2
	if(k==2)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST+CV_CALIB_FIX_K3;
	}
	//k=3为true表示使用k1,k2,k3
	if(k==3)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST;
	}
	//k=3为true表示使用k1,k2和p1,p2
	if(k==4)
	{
		flag+=CV_CALIB_FIX_K3;
	}
	//k=5为true表示使用k1,k2,k3和p1,p2
	if(k==5)
	{
		//默认
	}
	//k=8为true表示使用k1,k2,k3,k4,k5,k6和p1,p2
	if(k==8)
	{
		flag+=CV_CALIB_RATIONAL_MODEL;
	}

	//如果使用定义的矩阵初始化
	if(matrix.at<float>(0,0)>0)
	{
		flag+=CV_CALIB_USE_INTRINSIC_GUESS;
		camMatrix = matrix;
		distortion = cv::Mat(5,1,CV_32FC1,cv::Scalar(0));
	}
	cv::vector<cv::Mat> camRotationVectors;
  	cv::vector<cv::Mat> camTranslationVectors;
	
	double error = cv::calibrateCamera(
		ObjCorners, 
		CamCorners, 
		camImageSize, 
		camMatrix, 
		distortion, 
		camRotationVectors,camTranslationVectors,
		flag, 
		cv::TermCriteria( (cv::TermCriteria::COUNT)+(cv::TermCriteria::EPS), 100, DBL_EPSILON) );
	std::cout<<"总标定误差:"<<error<<std::endl;
}
void CameraCalibration::cameraCalibrationRandom(int imgs, int N)
{
	int totalN = imgFiles.size();
	std::vector<int> nums;

	double zeor33[] = {0,0,0,0,0,0,0,0,0};
	double zero15[] = {0,0,0,0,0};
	cv::Mat matrixSum(3,3,CV_64FC1,zeor33);
	cv::Mat distortSum(1,5,CV_64FC1,zero15);
	
	std::vector<double> fx, fy, cx, cy, k1, k2, p1, p2, k3;
	int flag = CV_CALIB_FIX_ASPECT_RATIO+CV_CALIB_FIX_K3;
	for(int k=0; k<N; k++)
	{
		std::cout <<"第"<< k<<"次标定" << std::endl;
		while(nums.size()<imgs){
			int random = cv::theRNG().uniform(0,totalN);
			if(nums.empty() || std::find(nums.begin(), nums.end(), random)==nums.end())
			{
				nums.push_back(random);
			}
		}
		std::vector<std::vector<cv::Point2f>> imgCornersCam;
		std::vector<std::vector<cv::Point3f>> objCornersCam;

		for(std::vector<int>::size_type i=0; i!=nums.size(); ++i)
		{
			imgCornersCam.push_back(CamCorners[nums[i]]);
			objCornersCam.push_back(ObjCorners[nums[i]]);
		}
	
		cv::vector<cv::Mat> camRotationVectors;
  		cv::vector<cv::Mat> camTranslationVectors;

		cv::calibrateCamera(objCornersCam, imgCornersCam, camImageSize,camMatrix, distortion, camRotationVectors,camTranslationVectors,flag, 
		cv::TermCriteria( (cv::TermCriteria::COUNT)+(cv::TermCriteria::EPS), 30, DBL_EPSILON) );
		
		fx.push_back(Utilities::matGet2D(camMatrix, 0, 0));
		fy.push_back(Utilities::matGet2D(camMatrix, 1, 1));
		cx.push_back(Utilities::matGet2D(camMatrix, 2, 0));
		cy.push_back(Utilities::matGet2D(camMatrix, 2, 1));
		k1.push_back(Utilities::matGet2D(distortion, 0, 0));
		k2.push_back(Utilities::matGet2D(distortion, 1, 0));
		p1.push_back(Utilities::matGet2D(distortion, 2, 0));
		p2.push_back(Utilities::matGet2D(distortion, 3, 0));
		//k3.push_back(Utilities::matGet2D(distortion, 4, 0));
		
		matrixSum += camMatrix;
		distortSum += distortion;
		nums.clear();
	}
	//处理分布数据
	paraDistribute(fx, folder+"Distribute_fx.txt");
	paraDistribute(fy, folder+"Distribute_fy.txt");
	paraDistribute(cx, folder+"Distribute_cx.txt");
	paraDistribute(cy, folder+"Distribute_cy.txt");
	paraDistribute(k1, folder+"Distribute_k1.txt");
	paraDistribute(k2, folder+"Distribute_k2.txt");
	paraDistribute(p1, folder+"Distribute_p1.txt");
	paraDistribute(p2, folder+"Distribute_p2.txt");
	//paraDistribute(k3, folder+"Distribute_k3.txt");
	//处理完毕
	camMatrix = matrixSum/N;
	distortion = distortSum/N;

	exportTxtFiles(folder+"Matrix.txt", CAMCALIB_OUT_MATRIX);
	exportTxtFiles(folder+"Distortion.txt", CAMCALIB_OUT_DISTORTION);
}
void CameraCalibration::cameraCalibrationProgress(int start,int end, int N)
{
	std::cout.setf(std::ios::fixed);
	std::cout.precision(1);

	int totalN = imgFiles.size();
	if(start == -1)
	{
		start =3;
	}
	if(end == -1)
	{
		if(N==1)
		{
			end = totalN-3;
		}
		else
		{
			end = totalN-N;
		}
	}
	
	double zeor33[] = {0,0,0,0,0,0,0,0,0};
	double zero15[] = {0,0,0,0,0};
	cv::Mat matrixSum(3,3,CV_64FC1,zeor33);
	cv::Mat distortSum(1,5,CV_64FC1,zero15);
	
	std::ofstream ffx(folder+"fx.txt");
	std::ofstream ffy(folder+"fy.txt");
	std::ofstream fcx(folder+"cx.txt");
	std::ofstream fcy(folder+"cy.txt");
	std::ofstream fk1(folder+"k1.txt");
	std::ofstream fk2(folder+"k2.txt");
	std::ofstream fp1(folder+"p1.txt");
	std::ofstream fp2(folder+"p2.txt");
	std::ofstream fk3(folder+"k3.txt");
	std::ofstream samples(folder+"sp.txt");	//采样了哪些图像
	std::ofstream all(folder+"all.txt");
	all<<"张数"<<" "<<"fx"<<" "<<"fy"<<" "<<"cx"<<" "<<"cy"<<" "<<"k1"<<" "<<"k2"<<" "<<"k3"<<" "<<"p1"<<" "<<"p2"<<std::endl;
	for(int i=start;i<end;i++)
	{
		std::cout<<"标定图片数"<<i;
		double tbegin = cv::getTickCount();
		float fx=0, fy=0, cx=0, cy=0, k1=0, k2=0, p1=0, p2=0, k3=0;
		std::vector<int> nums;
		float error;
		for(int k=0; k<N; k++)
		{
			//生成随机数据
			while(nums.size()<i){
				int random = cv::theRNG().uniform(0,totalN);
				if(nums.empty() || std::find(nums.begin(), nums.end(), random)==nums.end())
				{
					nums.push_back(random);
				}
			}
			samples<<"第"<<i<<"次采样的图像 ";
			for(int ci=0;ci<nums.size();ci++)
			{
				samples<<imgFiles[nums[ci]]<<" ";
			}
			samples<<std::endl;
			samples.flush();

			std::vector<std::vector<cv::Point2f>> imgCornersCam;
			std::vector<std::vector<cv::Point3f>> objCornersCam;
			for(std::vector<int>::size_type i=0; i!=nums.size(); ++i)
			{
				imgCornersCam.push_back(CamCorners[nums[i]]);
				objCornersCam.push_back(ObjCorners[nums[i]]);
			}
			cv::vector<cv::Mat> camRotationVectors;
  			cv::vector<cv::Mat> camTranslationVectors;

			//标定
			error = cv::calibrateCamera(
				objCornersCam, 
				imgCornersCam, 
				camImageSize,
				camMatrix, 
				distortion, 
				camRotationVectors,camTranslationVectors,
				CV_CALIB_FIX_ASPECT_RATIO+CV_CALIB_FIX_K3,  
				cv::TermCriteria( (cv::TermCriteria::COUNT)+(cv::TermCriteria::EPS), 30, DBL_EPSILON) );
		
			fx+=(Utilities::matGet2D(camMatrix, 0, 0));
			fy+=(Utilities::matGet2D(camMatrix, 1, 1));
			cx+=(Utilities::matGet2D(camMatrix, 2, 0));
			cy+=(Utilities::matGet2D(camMatrix, 2, 1));
			k1+=(Utilities::matGet2D(distortion, 0, 0));
			k2+=(Utilities::matGet2D(distortion, 1, 0));
			p1+=(Utilities::matGet2D(distortion, 2, 0));
			p2+=(Utilities::matGet2D(distortion, 3, 0));
			k3+=(Utilities::matGet2D(distortion, 4, 0));
		}
		//记录数据
		fx = fx/(float)N;ffx<<i<<"	"<<fx<<std::endl;ffx.flush();
		fy = fy/(float)N;ffy<<i<<"	"<<fy<<std::endl;ffy.flush();
		cx = cx/(float)N;fcx<<i<<"	"<<cx<<std::endl;fcx.flush();
		cy = cy/(float)N;fcy<<i<<"	"<<cy<<std::endl;fcy.flush();
		k1 = k1/(float)N;fk1<<i<<"	"<<k1<<std::endl;fk1.flush();
		k2 = k2/(float)N;fk2<<i<<"	"<<k2<<std::endl;fk2.flush();
		p1 = p1/(float)N;fp1<<i<<"	"<<p1<<std::endl;fp1.flush();
		p2 = p2/(float)N;fp2<<i<<"	"<<p2<<std::endl;fp2.flush();
		k3 = k3/(float)N;fk3<<i<<"	"<<k3<<std::endl;fk3.flush();
		all<<i<<" "<<fx<<" "<<fy<<" "<<cx<<" "<<cy<<" "<<k1<<" "<<k2<<" "<<k3<<" "<<p1<<" "<<p2<<std::endl;
		all.flush();
		double tend=cv::getTickCount();
		double frequency = cv::getTickFrequency();
		int span = (tend-tbegin)/frequency;
		std::cout.precision(1);
		std::cout<<",时间"<<span;
		std::cout<<" f:"<<fx;
		std::cout<<" cx:"<<cx;
		std::cout<<" cy:"<<cy;
		std::cout.precision(4);
		std::cout<<" k1:"<<k1;
		std::cout<<" k2:"<<k2;
		std::cout<<" 误差:"<<error;
		std::cout<<std::endl;
	}
	ffx.close();
	ffy.close();
	fcx.close();
	fcy.close();
	fk1.close();
	fk2.close();
	fp1.close();
	fp2.close();
	fk3.close();
	samples.close();
	all.close();
}
//---------------------------------标定外参-------------------------
void CameraCalibration::calCameraExtra(std::string imgCorners, std::string objCorners)
{
	//从文件matrix.txt和文件distortion.txt中载入内参
	cv::Mat cam_matrix(3,3,CV_64F);
	cv::Mat cam_distortion(1,5,CV_64F);

	std::string matrixPath = "matrix.txt";
	std::ifstream matrix(matrixPath.c_str());

	std::string distortPath = "distortion.txt";
	std::ifstream distort(distortPath.c_str());

	if(!matrix)
	{
		std::cout << "file cam_matrix.txt open failed" << std::endl;
		return;
	}

	if(!distort)
	{
		std::cout << "file cam_distortion.txt open failed" << std::endl; 
		return;
	}

	for(int i=0; i<cam_matrix.rows; i++)
	{
		for(int j=0; j<cam_matrix.cols; j++)
		{
			double temp;
			matrix >> temp;
			Utilities::matSet2D(cam_matrix, j,i,temp);
		}
	}

	for(int i=0; i<cam_distortion.rows; i++)
	{
		for(int j=0; j<cam_distortion.cols; j++)
		{
			double temp;
			distort >> temp;
			Utilities::matSet2D(cam_distortion, j,i,temp);
		}
	}

	//从文件中载入角点坐标和世界坐标
	cv::vector<cv::Point3f> objCorner;
	cv::vector<cv::Point2f> imgCorner;

	std::ifstream in(objCorners.c_str());
	if(!in){
		std::cout << "Load IntriData Error: can't open file " << objCorners << std::endl;
		return;
	}
	while(in)
	{
		cv::Point3f tmp;
		in >> tmp.x >> tmp.y >> tmp.z;
		objCorner.push_back(tmp);
	}
	in.close();

	in.open(imgCorners.c_str());
	if(!in){
		std::cout << "Load IntriData Error: can't open file " << objCorners << std::endl;
		return;
	}
	while(in){
		cv::Point2f tmp;
		in >> tmp.x >> tmp.y;
		imgCorner.push_back(tmp);
	}
	in.close();

	//计算外参
	cv::Mat rVec;
	cv::Mat rotationMatrix;
	cv::Mat translationVector;
	cv::solvePnP(objCorner, imgCorner,cam_matrix, cam_distortion,rVec,translationVector);
	cv::Rodrigues(rVec,rotationMatrix);

	//保存外参数据
	int pos=imgCorners.size()-1;
	for(; pos<imgCorners.size() && imgCorners[pos]!='_'; --pos)
		;
	std::string rotationFile = imgCorners.substr(0, pos) + "_rotation.txt";
	Utilities::exportMat(rotationFile.c_str(), rotationMatrix);

	std::string translationFile = imgCorners.substr(0, pos) + "_translation.txt";
	Utilities::exportMat(translationFile.c_str(), translationVector);

	std::string projFile = imgCorners.substr(0, pos) + "_projMatrix.txt";
	std::ofstream proj(projFile.c_str());
	//保存3*4投影矩阵
	cv::Mat A = cam_matrix*rotationMatrix;
	cv::Mat B = cam_matrix*translationVector;

	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
			proj << A.at<double>(i,j) << '\t';
		proj << B.at<double>(i,0) << std::endl;
	}
	proj.close();
}
void CameraCalibration::calCameraExtra()
{
	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); i++)
	{
		std::string path = folder+imgFiles[i].substr(0, imgFiles[i].size()-4) + "_imgCorners.txt";
		calCameraExtra(path,folder+"ObjCorners.txt");
	}
}

void CameraCalibration::exportTxtFiles(std::string path, int CAMCALIB_OUT_PARAM)
{
	cv::Mat out;
	switch (CAMCALIB_OUT_PARAM)
	{
		case CAMCALIB_OUT_MATRIX:
			out = camMatrix;
			break;
		case CAMCALIB_OUT_DISTORTION:
			out = distortion;
			break;
	}

	Utilities::exportMat(path.c_str(), out);	
}

//计算参数分布
void CameraCalibration::paraDistribute(std::vector<double> &para, std::string filename)
{
	std::vector<float> tempData;
	for(int i=0;i<para.size();i++)
	{
		tempData.push_back(para[i]);
	}

	Utilities::paraDistribute(tempData,filename);
}