#include "Keyboard_Movement_Controller.hpp"

namespace lve {
	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window,
		float dt, LveGameObject& gameObject)
	{	
		// 摄像机旋转
		glm::vec3 rotate{ 0.f };
		rotate.y -= static_cast<float>(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS);
		rotate.y += static_cast<float>(glfwGetKey(window, keys.lookRight) == GLFW_PRESS);
		rotate.x += static_cast<float>(glfwGetKey(window, keys.lookUp) == GLFW_PRESS);
		rotate.x -= static_cast<float>(glfwGetKey(window, keys.lookDown) == GLFW_PRESS);
		if (glm::dot(rotate, rotate) > glm::epsilon<float>())
			gameObject.transform.rotation += lookSpeed * dt * rotate;
		
		// 限制上下倾斜 -/+85度 和左右转动
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
		// pitch(上下倾斜):x axis yaw(左右转动):y axis roll(翻滚):z axis
		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };
		// Camera Obj move
		glm::vec3 movement{ 0.f };
		glfwGetKey(window, keys.moveForward) == GLFW_PRESS ? movement += forwardDir : movement;
		glfwGetKey(window, keys.moveBackward) == GLFW_PRESS ? movement -= forwardDir : movement;
		glfwGetKey(window, keys.moveRight) == GLFW_PRESS ? movement += rightDir : movement;
		glfwGetKey(window, keys.moveLeft) == GLFW_PRESS ? movement -= rightDir: movement;
		glfwGetKey(window, keys.moveUp) == GLFW_PRESS ? movement += upDir : movement;
		glfwGetKey(window, keys.moveDown) == GLFW_PRESS ? movement -= upDir: movement;
		if (glm::dot(movement, movement) > glm::epsilon<float>())
			gameObject.transform.translation += moveSpeed * dt * movement;
	}
}
