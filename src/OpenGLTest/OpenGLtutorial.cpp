//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include <memory>
//
//class GLFWWindowDeleter {
//public:
//	void operator()(GLFWwindow* window) {
//		if (window) glfwDestroyWindow(window);
//	}
//};
//using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWWindowDeleter>;
//
//void glfwFramebuffersizeCallback(GLFWwindow* window, int width, int height) {
//	glViewport(0, 0, width, height);
//}
//// ������������esc���������˳��ô���
//void processInput(GLFWwindow* window) {
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//}
//// ������ɫ��
//const char* vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec3 aPos;\n"
//"void main()\n"
//"{\n"
//"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
//"}\0";
//// Ƭ����ɫ��1
//const char* fragmentShaderSource = "#version 330 core\n"
//"out vec4 FragColor;\n"
//"void main()\n"
//"{\n"
//"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
//"}\0";
//
//// ������ɫ��
//const char* vertexShaderSource1 = "#version 330 core\n"
//"layout (location = 0) in vec3 aPos;\n"
//"layout (location = 1) in vec3 aColor;\n"
//"out vec4 ourColor;\n"
//"void main()\n"
//"{\n"
//"	gl_Position = vec4(aPos, 1.0f);\n"
//"	ourColor = vec4(aColor, 1.0f);\n"
//"}\0";
//
//// Ƭ����ɫ��2
//const char* fragmentShaderSource1 = "#version 330 core\n"
//"in vec4 ourColor;\n"
//"out vec4 FragColor;\n"
//"void main()\n"
//"{\n"
//"	FragColor = ourColor;\n"
//"}\0";
//
//// ��������
//float vertices[] = {
//-0.9f, 0.5f, 0.0f,   // ���Ͻ�
//-0.9f, -0.5f, 0.0f,  // ���½�
//0.0f, 0.5f, 0.0f, // ���½�
//0.5f, 0.5f, 0.0f   // ���Ͻ�
//};
//unsigned int indeices[] = {
//	0, 1, 3, // ��һ��������
//	0, 1, 2 // �ڶ���������
//};
//
//float vertices1[] = {
//	// positions         // colors
//	 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
//	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
//	 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
//
//};
//void bindBufferSetting(unsigned &VAO, unsigned& VBO, unsigned& EBO) {
	//// �󶨶��㻺�建��
	//glBindVertexArray(VAO);
	//// �󶨶��㻺�����
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//// �������������
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeices), indeices, GL_STATIC_DRAW);
	//// ���ö�������
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//// ���ö�������
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
//		
//}
//
//void bindBufferSetting1(unsigned& VAO, unsigned& VBO) {
//	// �󶨶��㻺�建��
//	glBindVertexArray(VAO);
//	// �󶨶��㻺�����
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
//	// ���ö�������
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	// ���ö�������
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
//	glEnableVertexAttribArray(0);
//
//}
//
//int main() {
//
//	// ����OpenGL��Ҫ��Ϣ�����汾�š��ΰ汾�š������ļ�ģʽ
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//
//	// ����GL����
//	UniqueGLFWwindow window(glfwCreateWindow(800, 600, "First Create Test!", nullptr, nullptr));
//	if (!window.get()) {
//		std::cerr << "Fail to created window !" << std::endl;
//		return -1;
//	}
//	// ָ����ǰGL��Ⱦ�����Ķ���
//	glfwMakeContextCurrent(window.get());
//
//	// ����GLAD��̬����GLFW����ָ��
//	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
//		std::cerr << "Fail to created glad loader !" << std::endl;
//		return -1;
//	}
//	glViewport(0, 0, 800, 600);
//	// �趨�޸Ĵ������ԵĻص�����
//	glfwSetFramebufferSizeCallback(window.get(), glfwFramebuffersizeCallback);
//	
//	/*
//		title: ��ɫ��ѧϰ
//		date: 6/8
//	*/
//	// RTT����������ɫ��
//	unsigned int vertexShader[2];
//	vertexShader[0] = glCreateShader(GL_VERTEX_SHADER);
//	// ����ɫ��Դ�븽������ɫ��������
//	glShaderSource(vertexShader[0], 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader[0]);
//
//	// ����������ɫ��ʱ��ȡ���������Ϣ
//	int success;
//	char infoLog[512];
//	// ʹ��glGetShaderiv��ȡ������ɫ������ʱ��Ϣ
//	glGetShaderiv(vertexShader[0], GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(vertexShader[0], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//
//	// ����Ƭ����ɫ���������������ɫ
//	unsigned int fragmentShader[2];
//	fragmentShader[0] = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader[0], 1, &fragmentShaderSource, NULL);
//	glCompileShader(fragmentShader[0]); 
//
//	// ������ɫ�����򣬲���������ɫ����Ƭ����ɫ���������ó���
//	unsigned int shaderProgram[2];
//	shaderProgram[0] = glCreateProgram();
//	shaderProgram[1] = glCreateProgram();
//	// ��֮ǰ����õ���ɫ�����ӵ����������
//	glAttachShader(shaderProgram[0], vertexShader[0]);
//	glAttachShader(shaderProgram[0], fragmentShader[0]);
//	glLinkProgram(shaderProgram[0]);
//	// ��ȡ��ɫ������״̬
//	glGetProgramiv(shaderProgram[0], GL_LINK_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(shaderProgram[0], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
//	}
//
//
//	vertexShader[1] = glCreateShader(GL_VERTEX_SHADER);
//	// ����ɫ��Դ�븽������ɫ��������
//	glShaderSource(vertexShader[1], 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader[1]);
//
//	// ����������ɫ��ʱ��ȡ���������Ϣ
//
//	// ʹ��glGetShaderiv��ȡ������ɫ������ʱ��Ϣ
//	glGetShaderiv(vertexShader[1], GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(vertexShader[1], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//
//
//	fragmentShader[1] = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader[1], 1, &fragmentShaderSource1, NULL);
//	glCompileShader(fragmentShader[1]);
//
//	// ��֮ǰ����õ���ɫ�����ӵ����������
//	glAttachShader(shaderProgram[1], vertexShader[1]);
//	glAttachShader(shaderProgram[1], fragmentShader[1]);
//	glLinkProgram(shaderProgram[1]);
//	// ��ȡ��ɫ������״̬
//	glGetProgramiv(shaderProgram[1], GL_LINK_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(shaderProgram[1], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
//	}
//	// �����ӵ�����������Ϊ������Ҫ��ɫ��������ɾ��
//	glDeleteShader(vertexShader[0]);
//	glDeleteShader(vertexShader[1]);
//	glDeleteShader(fragmentShader[0]);
//	glDeleteShader(fragmentShader[1]);
//
//
//	/*
//		�����ж��ͼԪ����ʱ��������Ҫʹ��EBOԪ�ػ������(���߽������������)���б��涥�����ݡ�
//	*/
//	// ���ö���������󡢶��㻺����������������
//	unsigned int VAOs[2], VBOs[2], EBOs[2];
//	// ���ɶ��㻺�����
//	glGenBuffers(2, EBOs);
//	glGenBuffers(2, VBOs);
//	// ���ö����������
//	glGenVertexArrays(2, VAOs); // �������ɶ����������
//	
//	bindBufferSetting(VAOs[0], VBOs[0], EBOs[0]);
//	bindBufferSetting1(VAOs[1], VBOs[1]);
//	//// �󶨶��㻺�建��
//	//glBindVertexArray(VAOs[0]);
//	//glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
//	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
//	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeices), indeices, GL_STATIC_DRAW);
//	//// ���ö�������
//	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	//glEnableVertexAttribArray(0);
//
//	//// �󶨶��㻺�建��
//	//glBindVertexArray(VAOs[1]);
//	//// �󶨶��㻺�����
//	//glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
//	//glBufferData(GL_ARRAY_BUFFER, sizeof(secvertices), secvertices, GL_STATIC_DRAW);
//	//// �������������
//	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
//	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(secindeices), secindeices, GL_STATIC_DRAW);
//	//// ���ö�������
//	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	//glEnableVertexAttribArray(0);
//
//
//	/*
//	* ��Ҫע�������GL_ELEMENT_ARRAY_BUFFERʱ�������Խ��EBO��
//	* ��Ϊ��ʱ��VAO������glBindBuffer�ĺ�������
//	* ��Ҳ��ζ����Ҳ�ᴢ������ã�
//	* ����ȷ����û���ڽ��VAO֮ǰ����������黺�壬
//	* ��������û�����EBO�����ˡ�
//	*/
//	glBindBuffer(GL_ARRAY_BUFFER, 0); // ��� 
//	
//
//
//
//	// �������ڹرյ��¼�ѭ��
//	while (!glfwWindowShouldClose(window.get())) {
//		// ���밴������
//		processInput(window.get());
//
//		// ��Ⱦ����
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // �����ɫ������ʹ�õ���ɫ
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		// ��������
//		glUseProgram(shaderProgram[0]);
//		// ����������
//		glBindVertexArray(VAOs[0]);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//		
//		// �������
		//glUseProgram(shaderProgram[1]);
		//// ��ȡglfw��������ʱ��
		//float timeValue = glfwGetTime();
		//// ������ʱ��仯����ɫ��ɫ�仯
		//float greenValue = sin(timeValue) / 2.0f + 0.3f;
		//float redValue = sin(timeValue) / 2.0f - 0.2f;
		//float yellowValue = sin(timeValue) / 2.0f ;
		//// ��ȡuniform����
		//int vertexColorLocation = glGetUniformLocation(shaderProgram[1], "ourColor");
		//glUniform4f(vertexColorLocation, redValue, greenValue, yellowValue, 1.0f);
//
//
//		// ����������
//		glBindVertexArray(VAOs[1]);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//
//		// ˫����������
//		glfwSwapBuffers(window.get());
//		// �����¼�����
//		glfwPollEvents();
//	}
//	// �ر�ǰ����Ҫ�������������ݡ����㻺��������ɫ���������
//	glDeleteVertexArrays(2, VAOs);
//	glDeleteBuffers(2, EBOs);
//	glDeleteBuffers(2, VBOs);
//	glDeleteProgram(shaderProgram[0]);
//	glDeleteProgram(shaderProgram[1]);
//	glfwTerminate();
//	return 0;
//}