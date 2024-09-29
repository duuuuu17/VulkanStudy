#pragma once
#include "lve_model.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
// std
#include <memory>
#include<unordered_map>
namespace lve {
	struct TransformComponent {
		// linear transform
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation;
		// translation
		glm::vec3 translation{};
		//// Matrix corresponds to translate * Ry * Rx * Rz * scale transormation
		//// Rotation convention uses tait-bryan anlges with axis other Y(1), x(2), Z(3)
		//glm::mat4 mat4()
		//{
		//	auto transform = glm::translate( glm::mat4{1.f},translation );
		//	transform = glm::rotate(transform, rotation.y, { 0.f,1.f,0.f });
		//	transform = glm::rotate(transform, rotation.x, { 1.f,.0f,0.f });
		//	transform = glm::rotate(transform, rotation.z, { 0.f,0.f,1.f });
		//	transform = glm::scale(transform, scale);
		//	return transform;
		//}
		
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		// using Euler angle rotation method: YXZ
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};
	// 设置点光源对象的剩余分量为结构体，并添加在gameObj中
	struct PointLightComponent {
		float intensity = 1.f;
	};
	class LveGameObject {
	public : 
		using id_t = unsigned int;
		using Map = std::unordered_map<uint32_t, LveGameObject>;
		static LveGameObject createGameObject()
		{
			static id_t currentId = 0;
			return LveGameObject{currentId++};
		}

		LveGameObject (const LveGameObject&) = delete;
		LveGameObject& operator=(const LveGameObject&) = delete;
		LveGameObject (LveGameObject&&) = default;
		LveGameObject& operator=(LveGameObject&&) = default;

		static LveGameObject makePointLight(float intensity = 10.f, float radius = .1f, glm::vec3 color = glm::vec3(1.f));

		id_t getId() const { return id; }

		std::shared_ptr<LveModel> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;
		glm::vec3 color{};
		TransformComponent transform{};
	private:
		LveGameObject(id_t objId) { id = objId; }
		id_t id = 0;
	};

}