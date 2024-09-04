//#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <iostream>
//
//int main() {
//	//glm::mat4  identityMatrix = glm::mat4(1.0f);
//	//// Æ½ÒÆ¾ØÕó
//	//float tx = 1.0f, ty = 2.0f, tz = 3.0f;
//	//glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
//
//	//// Ðý×ª¾ØÕó
//	//constexpr float angle = glm::radians(90.f); // ½Ç¶È×ª»»Îª»¡¶È
//	//glm::vec3 axis(0.0f, 0.0f, 1.0f);
//	//glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
//	//// Ëõ·Å¾ØÕó
//	//glm::vec3 scaleFactors(2.0f, 2.0f, 2.0f);
//	//glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleFactors);
//	//// ¹Û²ì¾ØÕó
//	//glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
//	//glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
//	//glm::vec3 upVector(0.0f, 1.0f, 0.0f);
//	//glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
//
//	//// ¾ØÕó×ª»»
//	//glm::mat4 modelMatrix = glm::mat4(1.0f);
//	//modelMatrix = glm::translate(modelMatrix, translationMatrix);
//	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
//	glm::mat4 trans = glm::mat4(1.0f);
//	glm::vec3 identityVector = glm::vec3(1.0f, 1.0f, 1.0f);
//	trans = glm::translate(trans, identityVector);
//	vec = trans * vec;
//	std::cout << vec.x << vec.y << vec.z << std::endl;
//
//	glm::mat4 trans;
//	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
//	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
//	return 0;
//}