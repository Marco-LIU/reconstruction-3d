#include "CameraCalibration.h"
#include "windows.h"

#define CAMCALIB_OUT_MATRIX 1
#define CAMCALIB_OUT_DISTORTION 2

CameraCalibration::CameraCalibration(std::string fd, int _XCorners, int _YCorners,int sl)
{
	//ɨ������jpgͼ���ļ�
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
//-----------------------------------�������̸�ǵ�����------------------
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
	//��ȡ��ת��Ϊ�Ҷ�ͼ,todo ����Ǻڰ�ͼ���п��ܽ��ͶԱȶ�
	std::string imgfile = filename+".jpg";
	cv::Mat img = cv::imread(imgfile);
	cv::vector<cv::Point2f> cCam;
	cv::Mat img_grey;
	cv::cvtColor(img, img_grey, CV_RGB2GRAY);

	//�ҽǵ�
	found=cv::findChessboardCorners(img_grey, cvSize(X,Y), cCam, CV_CALIB_CB_ADAPTIVE_THRESH );
	if(!found)
		return;
	if(found)
	{
		//����������������
		cv::cvtColor( img, img_grey, CV_RGB2GRAY );
		cv::cornerSubPix(img_grey, cCam, cvSize(20,20), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
	}
	
	//ȷ���ǵ��˳�򣬴��ϵ��£�������
	cv::vector<cv::Point2f> ncorners = Utilities::normalizeCorners(cCam,X,Y);

	//��¼�ǵ㵽�ļ�
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
		//���Ƶ���ͼ�����
		cv::Mat imgCopy = img.clone();
		cv::drawChessboardCorners(imgCopy,cvSize(X,Y),ncorners,found);
		cv::circle(imgCopy,ncorners[0],15,cv::Scalar(255,255,255));
		cv::imwrite(filename+"_Corner.bmp",imgCopy);
	}
}
void CameraCalibration::findCorners(bool debug)
{
	//����ÿ��ͼ�����ҵ��ǵ�
	std::vector<std::string> todelete;

	//����ÿһ��ͼ������ɹ��ҵ����еĽǵ㣬�ѽǵ������¼����
	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); ++i)
	{
		static int n=1;
		std::cout<<n++<<"	"<<imgFiles[i]<<std::endl;
		
		//1.jpg����1������size-4
		std::string curPicName = folder+imgFiles[i].substr(0, imgFiles[i].size()-4);
		//����ǵ��Ѿ��ҹ��ˣ���������ǵ�Ĳ�ѯ
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

	//ɾ��������ʲô̫�鷳,ֱ��������һ��
	std::vector<std::string> temp;
	for(int i=0;i<imgFiles.size();i++)
	{
		std::string filename = imgFiles[i];
		bool add= true;
		for(int j=0;j<todelete.size();j++)
		{
			if(filename==todelete[j])
			{
				//���Ҫɾ���������
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
	//���ļ�ɾ��
	for(int j=0;j<todelete.size();j++)
	{
		std::string filename = todelete[j];
		std::string cmd = "del \"" + folder+filename +"\"";
		system(cmd.c_str());
	}
}
//----------------------------------����ǵ�����--------------------------
void CameraCalibration::loadCorrespondCorners()
{
	//Ĭ�ϵĽǵ��ļ���
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
	//��������ռ�����
	std::ifstream obj(objCornersFile.c_str());
	if(!obj)
	{
		Utilities::generateObjCorners(objCornersFile,XCorners,YCorners,squareLength);
		obj.open(objCornersFile);
		if(!obj)
		{
			//һ�㲻�����е�����
			std::cout<<"�������̸������ļ�ObjCorners.txt����"<<std::endl;
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
	
	//�����ҵ������̸�ǵ�
	std::ifstream cam(imgCornersFile.c_str());
	if(!cam)
	{
		std::cout << imgCornersFile << "�ǵ������ļ��������" << std::endl;
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
	//Ĭ�ϵĽǵ��ļ���
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
	//��������ռ�����
	std::ifstream obj(objCornersFile.c_str());
	if(!obj)
	{
		Utilities::generateObjCorners(objCornersFile,XCorners,YCorners,squareLength);
		obj.open(objCornersFile);
		if(!obj)
		{
			//һ�㲻�����е�����
			std::cout<<"�������̸������ļ�ObjCorners.txt����"<<std::endl;
			return 0;
		}
	}
	std::vector<cv::Point3f> cObj;
	
	cv::Point3f point;
	while(obj >> point.x >> point.y >> point.z){
		cObj.push_back(point);
	}
	
	//�����ҵ������̸�ǵ�
	std::ifstream cam(imgCornersFile.c_str());
	if(!cam)
	{
		std::cout << imgCornersFile << "�ǵ������ļ��������" << std::endl;
		return 0;
	}
	std::vector<cv::Vec4f> pts;
	cv::Vec4f pt;
	while(cam >> pt(0) >> pt(1) >> pt(2)>>pt(3)){
		pts.push_back(pt);
	}

	std::vector<cv::Point3f> tObj;
	std::vector<cv::Point2f> tCam;
	//��������С��ƥ���
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
	//��������̫�࣬��������ͼ
	if(count < pts.size()*0.33)
		return 0;

	
	static int ct = 0;
	ct++;
	std::cout<<"����"<<ct<<"��ƥ���"<<std::endl;
	*/
	
	ObjCorners.push_back(tObj);
	CamCorners.push_back(tCam);

	return count;
}
void CameraCalibration::loadIdealCorrespondCorners()
{
	//Ĭ�ϵĽǵ��ļ���
	std::string CORNERS="ObjCorners.txt";

	for(std::vector<std::string>::size_type i=0; i!=imgFiles.size(); ++i)
	{
		std::string imgCornersFile = folder+imgFiles[i].substr(0, imgFiles[i].size()-4) + "_imgCornersIdeal.txt";
		std::string objCornersFile = folder+CORNERS;
		
		loadCorrespondCorners(imgCornersFile,objCornersFile);
	}
}

//-----------------------------------�ڲα궨-----------------------
void CameraCalibration::cameraCalibration(bool saveExtr)
{
	cv::vector<cv::Mat> camRotationVectors;
  	cv::vector<cv::Mat> camTranslationVectors;
	
	//Ŀǰ�ľ�ͷ���ʹ��k1,k2,p1��p2
	double error = cv::calibrateCamera(
		ObjCorners, 
		CamCorners, 
		camImageSize, 
		camMatrix, 
		distortion, 
		camRotationVectors,camTranslationVectors,
		CV_CALIB_FIX_ASPECT_RATIO+CV_CALIB_FIX_K3, 
		cv::TermCriteria( (cv::TermCriteria::COUNT)+(cv::TermCriteria::EPS), 100, DBL_EPSILON) );
	std::cout<<"�ܱ궨���:"<<error<<std::endl;

	//�������
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
		info<<error<<" ";						//ƽ����ͶӰ���,��λ����
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
	//k=0Ϊtrue��ʾ�����ǻ���
	if(k==0)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST+CV_CALIB_FIX_K1+CV_CALIB_FIX_K2+CV_CALIB_FIX_K3;
	}
	//k=1Ϊtrue��ʾֻʹ��k1
	if(k==1)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST+CV_CALIB_FIX_K2+CV_CALIB_FIX_K3;
	}
	//k=2Ϊtrue��ʾʹ��k1,k2
	if(k==2)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST+CV_CALIB_FIX_K3;
	}
	//k=3Ϊtrue��ʾʹ��k1,k2,k3
	if(k==3)
	{
		flag+=CV_CALIB_ZERO_TANGENT_DIST;
	}
	//k=3Ϊtrue��ʾʹ��k1,k2��p1,p2
	if(k==4)
	{
		flag+=CV_CALIB_FIX_K3;
	}
	//k=5Ϊtrue��ʾʹ��k1,k2,k3��p1,p2
	if(k==5)
	{
		//Ĭ��
	}
	//k=8Ϊtrue��ʾʹ��k1,k2,k3,k4,k5,k6��p1,p2
	if(k==8)
	{
		flag+=CV_CALIB_RATIONAL_MODEL;
	}

	//���ʹ�ö���ľ����ʼ��
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
	std::cout<<"�ܱ궨���:"<<error<<std::endl;
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
		std::cout <<"��"<< k<<"�α궨" << std::endl;
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
	//����ֲ�����
	paraDistribute(fx, folder+"Distribute_fx.txt");
	paraDistribute(fy, folder+"Distribute_fy.txt");
	paraDistribute(cx, folder+"Distribute_cx.txt");
	paraDistribute(cy, folder+"Distribute_cy.txt");
	paraDistribute(k1, folder+"Distribute_k1.txt");
	paraDistribute(k2, folder+"Distribute_k2.txt");
	paraDistribute(p1, folder+"Distribute_p1.txt");
	paraDistribute(p2, folder+"Distribute_p2.txt");
	//paraDistribute(k3, folder+"Distribute_k3.txt");
	//�������
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
	std::ofstream samples(folder+"sp.txt");	//��������Щͼ��
	std::ofstream all(folder+"all.txt");
	all<<"����"<<" "<<"fx"<<" "<<"fy"<<" "<<"cx"<<" "<<"cy"<<" "<<"k1"<<" "<<"k2"<<" "<<"k3"<<" "<<"p1"<<" "<<"p2"<<std::endl;
	for(int i=start;i<end;i++)
	{
		std::cout<<"�궨ͼƬ��"<<i;
		double tbegin = cv::getTickCount();
		float fx=0, fy=0, cx=0, cy=0, k1=0, k2=0, p1=0, p2=0, k3=0;
		std::vector<int> nums;
		float error;
		for(int k=0; k<N; k++)
		{
			//�����������
			while(nums.size()<i){
				int random = cv::theRNG().uniform(0,totalN);
				if(nums.empty() || std::find(nums.begin(), nums.end(), random)==nums.end())
				{
					nums.push_back(random);
				}
			}
			samples<<"��"<<i<<"�β�����ͼ�� ";
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

			//�궨
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
		//��¼����
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
		std::cout<<",ʱ��"<<span;
		std::cout<<" f:"<<fx;
		std::cout<<" cx:"<<cx;
		std::cout<<" cy:"<<cy;
		std::cout.precision(4);
		std::cout<<" k1:"<<k1;
		std::cout<<" k2:"<<k2;
		std::cout<<" ���:"<<error;
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
//---------------------------------�궨���-------------------------
void CameraCalibration::calCameraExtra(std::string imgCorners, std::string objCorners)
{
	//���ļ�matrix.txt���ļ�distortion.txt�������ڲ�
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

	//���ļ�������ǵ��������������
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

	//�������
	cv::Mat rVec;
	cv::Mat rotationMatrix;
	cv::Mat translationVector;
	cv::solvePnP(objCorner, imgCorner,cam_matrix, cam_distortion,rVec,translationVector);
	cv::Rodrigues(rVec,rotationMatrix);

	//�����������
	int pos=imgCorners.size()-1;
	for(; pos<imgCorners.size() && imgCorners[pos]!='_'; --pos)
		;
	std::string rotationFile = imgCorners.substr(0, pos) + "_rotation.txt";
	Utilities::exportMat(rotationFile.c_str(), rotationMatrix);

	std::string translationFile = imgCorners.substr(0, pos) + "_translation.txt";
	Utilities::exportMat(translationFile.c_str(), translationVector);

	std::string projFile = imgCorners.substr(0, pos) + "_projMatrix.txt";
	std::ofstream proj(projFile.c_str());
	//����3*4ͶӰ����
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

//��������ֲ�
void CameraCalibration::paraDistribute(std::vector<double> &para, std::string filename)
{
	std::vector<float> tempData;
	for(int i=0;i<para.size();i++)
	{
		tempData.push_back(para[i]);
	}

	Utilities::paraDistribute(tempData,filename);
}