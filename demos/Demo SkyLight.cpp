#include "openrt.h"
#include "core/timer.h"

using namespace rt;
#include <chrono>
int main(int argc, char* argv[]) {
	int samples = 10;
	if(argc == 2){
		std::istringstream iss(argv[1]);
		iss >> samples;
	}
	CScene scene(Vec3f::all(0.0f));
	float ka = 0.0f;
	float kd = 1.0f;
	float ks = 0.0f;
	float ke = 0.0f;
	auto pShaderFloor = std::make_shared<CShaderPhong>(scene, RGB(1, 1, 1), ka, kd, ks, ke);
	auto pShaderBall = std::make_shared<CShaderPhong>(scene, RGB(1,1, 1), 	ka, kd, ks, ke);
	auto pShaderL = std::make_shared<CShaderPhong>(scene, RGB(1, 1, 1),		ka, kd, ks, ke);
	float s = 3;
	//Ball
	scene.add(std::make_shared<CPrimSphere>(pShaderBall, Vec3f(0, 0.5, 0), 1.5f));
	//Floor
	scene.add(CSolidQuad(pShaderFloor, Vec3f(-s, 0, -s), Vec3f(-s, 0, s), Vec3f(s, 0, s), Vec3f(s, 0, -s)));
	auto pCamera = std::make_shared<CCameraPerspectiveTarget>(Vec3f(4, 4, 4), Vec3f(0, 0.5f, 0), Vec3f(0, 1, 0), 45.0f, Size(720, 720));
	// scene.add(std::make_shared<CLightPoint>(RGB(10, 10, 10), Vec3f(4, 4, 0)));

	auto pSkyLight = std::make_shared<CLightSky>( Vec3f(1.0f, 2.0f, 1.0f), 0.5f, samples);
	scene.add(pCamera);
	scene.add(pSkyLight);

	#ifdef ENABLE_BSP
		scene.buildAccelStructure();
	#endif
	auto start = std::chrono::high_resolution_clock::now();
	Mat img = scene.render(std::make_shared<CSamplerStratified>(2));
	auto stop = std::chrono::high_resolution_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout<<"Time for "<<samples<<" samples -- "<<delta.count()<<" ms"<<std::endl;
	// imwrite( std::to_string(samples) + "time" +std::to_string(delta.count()) +".png", img);
	imwrite("out.png",img);
	return 0;
}