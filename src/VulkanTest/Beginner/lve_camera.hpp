#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {
	class LveCamera
	{
	public:
		// ����ͶӰ
		void setOrthographicProjection(float left, float right, 
			float top, float bottom, float near, float far);
		// ͸��ͶӰ
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		// ���ݸ�����Camera�������
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f,0.f });
		// ����̶�Ŀ�����ʱʹ��
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f,0.f });
		// �������ת���ӽ�
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& getProjcetion() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }
		const glm::mat4& getInverseViewMatrixw() const { return inverseViewMatrix; }
		const glm::vec3& getPosition() const { return glm::vec3(inverseViewMatrix[3]); }
	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };
	};
}


