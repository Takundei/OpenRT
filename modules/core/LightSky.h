#pragma once

#include "ILight.h"
#include "CosineSampler.h"
namespace rt {
	/**
	 * @brief Point light source class
	 */
	class CLightSky : public ILight
	{
	public:
		/**
		 * @brief Constructor
		 * @param intensity The emission color and strength of the light source
		 * @param position The position (origin) of the light source
		 * @param castShadow Flag indicatin whether the light source casts shadow
		 */
		DllExport CLightSky(Vec3f direction, float radius,size_t nSamples, bool castShadow = false)
			: ILight(castShadow)
			, m_direction(normalize(direction))
			, m_radius(radius)
		{
			m_pSampler = std::make_shared <CosineSampler>(nSamples, true);
		}
		DllExport virtual ~CLightSky(void) = default;

		DllExport virtual std::optional<Vec3f> illuminate(Ray& ray)
		{
		//Set the params for ray.hit->getnormal(ray), reset properties t,hit and hit modified by illuminate
		ray.dir = Vec3f::all(0);
		ray.t = 0;
		Vec3f normal = ray.hit->getNormal(ray);
		ray.hit = nullptr;
		Vec3f rotationAxis = m_pSampler->getRotationAxis(normal);
		ray.t = m_radius;
		auto S = m_pSampler->getNextSample(rotationAxis);
		ray.dir = S.sample;
		return Vec3f::all(1.0f/(1.0f-S.pdf));
		//1/(1.0f-S.pdf)
			
		}
		DllExport virtual size_t getNumberOfSamples(void) const { return m_pSampler->getNumSamples(); }


	private:
		Vec3f m_direction; 
		float m_radius;  
		std::shared_ptr<CosineSampler>	m_pSampler;
	};
}

