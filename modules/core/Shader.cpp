#include "Shader.h"
#include "Scene.h"
#include "ray.h"

namespace rt {
	const static Vec3f exitColor	 = RGB(0.4f, 0.4f, 0.4f);	// errors area
	const static Vec3f ambientColor	 = RGB(1, 1, 1);			// ambient radiance
	const static Vec3f specularColor = RGB(1, 1, 1);			// white highlight;
	
	namespace {
		std::pair<Vec3f, Vec3f> getRandomTangents(Vec3f normal)
		{
			std::pair<Vec3f, Vec3f> res;
			float s1 = random::U<float>();
			float s2 = random::U<float>();

			if (fabs(normal.val[0]) > 0.1f)
				res.first = Vec3f(-(s1 * normal.val[1] + s2 * normal.val[2]) / normal.val[0], s1, s2);
			else if (fabs(normal.val[1]) > 0.1f)
				res.first = Vec3f(s1, -(s1 * normal.val[0] + s2 * normal.val[2] / normal.val[1]), s2);
			else
				res.first = Vec3f(s1, s2, -(s1 * normal.val[0] + s2 * normal.val[1]) / normal.val[2]);

			res.second = normal.cross(res.first);
			res.first = normalize(res.first);
			res.second = normalize(res.second);
			return res;
		}
	}	
	
	Vec3f CShader::shade(const Ray& ray) const
	{
		Vec3f res(0, 0, 0);

		Vec3f color = CShaderFlat::shade(ray);

		Vec3f normal = ray.hit->getNormal(ray);									// shading normal
		bool inside = false;
		if (normal.dot(ray.dir) > 0) {
			normal = -normal;													// turn normal to front
			inside = true;
		}
		const auto tangents = getRandomTangents(normal);						// two random tangent vectors

#ifdef DEBUG_MODE
		color = inside ? RGB(1, 0, 0) : RGB(0, 0, 1);
#endif

		size_t nNormalSamples = m_pSampler ? m_pSampler->getNumSamples() : 1;
		for (size_t ns = 0; ns < nNormalSamples; ns++) {

			// Distort the normal vector
			Vec3f n = normal;
			if (m_pSampler) {
				Vec2f sample = m_pSampler->getNextSample();
				n = normalize(n + sample.val[0] * tangents.first + sample.val[1] * tangents.second);
			}

			// Needed by   ks, km, kt
			Ray reflected = (m_ks > 0 || m_km > 0 || m_kt > 0) ? ray.reflected(n) : ray;	// reflection vector

			// ------ ambient ------
			if (m_ka > 0)
				res += m_ka * ambientColor.mul(color);

			// ------ diffuse and/or specular ------
			if (m_kd > 0 || m_ke > 0) {
				Ray I(ray.hitPoint());

				for (auto& pLight : m_scene.getLights()) {
					Vec3f L = Vec3f::all(0);
					const size_t nSamples = pLight->getNumberOfSamples();
					for (size_t s = 0; s < nSamples; s++) {
						// get direction to light, and intensity
						//Set the params for ray.hit->getnormal(ray), reset properties t,hit and hit modified by illuminate
						I.hit = ray.hit;
						//or set ray.property = normal ?
						auto radiance = pLight->illuminate(I);
						bool occluded = !m_scene.occluded(I);
						if (radiance && (!pLight->shadow() || occluded)) {
							// ------ diffuse ------
							if (m_kd > 0) {
								float d = occluded ? 1.0f : 0.0f; //use to view the image with occlusion as white values 0.0f : 1.0f;
								float cosLightNormal = I.dir.dot(n);
								if (cosLightNormal > 0)
									L += m_kd * cosLightNormal * color.mul(radiance.value())*d;
							}
							// ------ specular ------
							if (m_ks > 0) {
								float cosLightReflect = I.dir.dot(reflected.dir);
								if (cosLightReflect > 0)
									L += m_ks * powf(cosLightReflect, m_ke) * specularColor.mul(radiance.value());
							}
						}
					} // s
					res += (1.0f / nSamples) * L;
				} // pLight
			}

			// ------ reflection ------
			if (m_km > 0) 
				res += m_km * reTrace(reflected);

			// ------ refraction ------
			if (m_kt > 0) {
				Ray refracted = ray.refracted(n, inside ? m_refractiveIndex : 1.0f / m_refractiveIndex).value_or(reflected);
				res += m_kt * reTrace(refracted);
			}
		} // ns
		
		res = (1.0f / nNormalSamples) * res;
		return res;
	}

	Vec3f CShader::reTrace(const Ray& ray) const
	{
		return ray.counter >= maxRayCounter ? exitColor : m_scene.rayTrace(lvalue_cast(Ray(ray.org, ray.dir, ray.counter)));
	}
}

