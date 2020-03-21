#include "openrt.h"
#include "core/timer.h"

using namespace rt;

int main(int argc, char* argv[]) {
	int samples = 10;
	if(argc == 2){
		std::istringstream iss(argv[1]);
		iss >> samples;
	}

	CScene scene(Vec3f::all(0.0f));
	
	// Shaders
	auto pShaderWhite = std::make_shared<CShaderPhong>(scene, RGB(1, 1, 1), 0.1f, 0.9f, 0.0f, 40.0f);

	// Floor
	float s = 3;
	// scene.add(CSolidQuad(pShaderWhite, Vec3f(-s, 0, -s), Vec3f(-s, 0, s), Vec3f(s, 0, s), Vec3f(s, 0, -s)));
	
	// Ball
	scene.add(CSolid(pShaderWhite, "C:\\Users\\GoraT\\Documents\\models\\eaton.obj"));
	
	// Camera
	scene.add(std::make_shared<CCameraPerspectiveTarget>(Vec3f(0.201876 , -20.9304, -3.60727), Vec3f(1, 0, 0), Vec3f(0, 0, 1), 45.0f, Size(720, 720)));
	
	// Light 
//	scene.add(std::make_shared<CLightPoint>(RGB(10, 10, 10), Vec3f(4, 4, 0)));
	scene.add(std::make_shared<CLightSky>(RGB(1, 1, 1),1.0f,std::make_shared<CSamplerStratified>(1, true, true)));

//	auto pSkyLight = std::make_shared<CShaderAmbientOccluson>(scene, Vec3f(1, -1, 1), 0.7, samples);
//	scene.add(pSkyLight);

	#ifdef ENABLE_BSP
		scene.buildAccelStructure();
	#endif
	
	Timer::start("Rendering... ");
	Mat img = scene.render(std::make_shared<CSamplerStratified>(2));
	Timer::stop();

	imshow("image", img);
	waitKey();
	imwrite( "out.png", img);

	return 0;
}