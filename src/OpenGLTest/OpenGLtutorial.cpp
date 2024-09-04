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
//// 若监听到按键esc触发，则退出该窗口
//void processInput(GLFWwindow* window) {
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//}
//// 顶点着色器
//const char* vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec3 aPos;\n"
//"void main()\n"
//"{\n"
//"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
//"}\0";
//// 片段着色器1
//const char* fragmentShaderSource = "#version 330 core\n"
//"out vec4 FragColor;\n"
//"void main()\n"
//"{\n"
//"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
//"}\0";
//
//// 顶点着色器
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
//// 片段着色器2
//const char* fragmentShaderSource1 = "#version 330 core\n"
//"in vec4 ourColor;\n"
//"out vec4 FragColor;\n"
//"void main()\n"
//"{\n"
//"	FragColor = ourColor;\n"
//"}\0";
//
//// 顶点坐标
//float vertices[] = {
//-0.9f, 0.5f, 0.0f,   // 右上角
//-0.9f, -0.5f, 0.0f,  // 右下角
//0.0f, 0.5f, 0.0f, // 左下角
//0.5f, 0.5f, 0.0f   // 左上角
//};
//unsigned int indeices[] = {
//	0, 1, 3, // 第一个三角形
//	0, 1, 2 // 第二个三角形
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
	//// 绑定顶点缓冲缓冲
	//glBindVertexArray(VAO);
	//// 绑定顶点缓冲对象
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//// 绑定索引缓冲对象
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeices), indeices, GL_STATIC_DRAW);
	//// 设置顶点属性
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//// 设置顶点属性
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
//		
//}
//
//void bindBufferSetting1(unsigned& VAO, unsigned& VBO) {
//	// 绑定顶点缓冲缓冲
//	glBindVertexArray(VAO);
//	// 绑定顶点缓冲对象
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
//	// 设置顶点属性
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	// 设置顶点属性
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
//	glEnableVertexAttribArray(0);
//
//}
//
//int main() {
//
//	// 设置OpenGL必要信息：主版本号、次版本号、配置文件模式
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//
//	// 创建GL窗口
//	UniqueGLFWwindow window(glfwCreateWindow(800, 600, "First Create Test!", nullptr, nullptr));
//	if (!window.get()) {
//		std::cerr << "Fail to created window !" << std::endl;
//		return -1;
//	}
//	// 指定当前GL渲染上下文对象
//	glfwMakeContextCurrent(window.get());
//
//	// 加载GLAD动态管理GLFW窗口指针
//	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
//		std::cerr << "Fail to created glad loader !" << std::endl;
//		return -1;
//	}
//	glViewport(0, 0, 800, 600);
//	// 设定修改窗口属性的回调函数
//	glfwSetFramebufferSizeCallback(window.get(), glfwFramebuffersizeCallback);
//	
//	/*
//		title: 着色器学习
//		date: 6/8
//	*/
//	// RTT创建顶点着色器
//	unsigned int vertexShader[2];
//	vertexShader[0] = glCreateShader(GL_VERTEX_SHADER);
//	// 将着色器源码附加在着色器对象上
//	glShaderSource(vertexShader[0], 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader[0]);
//
//	// 声明编译着色器时获取编译检测的信息
//	int success;
//	char infoLog[512];
//	// 使用glGetShaderiv获取顶点着色器编译时信息
//	glGetShaderiv(vertexShader[0], GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(vertexShader[0], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//
//	// 创建片段着色器，计算出最后的颜色
//	unsigned int fragmentShader[2];
//	fragmentShader[0] = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader[0], 1, &fragmentShaderSource, NULL);
//	glCompileShader(fragmentShader[0]); 
//
//	// 创建着色器程序，并将顶点着色器和片段着色器链接至该程序
//	unsigned int shaderProgram[2];
//	shaderProgram[0] = glCreateProgram();
//	shaderProgram[1] = glCreateProgram();
//	// 将之前编译好的着色器附加到程序对象上
//	glAttachShader(shaderProgram[0], vertexShader[0]);
//	glAttachShader(shaderProgram[0], fragmentShader[0]);
//	glLinkProgram(shaderProgram[0]);
//	// 获取着色器编译状态
//	glGetProgramiv(shaderProgram[0], GL_LINK_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(shaderProgram[0], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
//	}
//
//
//	vertexShader[1] = glCreateShader(GL_VERTEX_SHADER);
//	// 将着色器源码附加在着色器对象上
//	glShaderSource(vertexShader[1], 1, &vertexShaderSource, NULL);
//	glCompileShader(vertexShader[1]);
//
//	// 声明编译着色器时获取编译检测的信息
//
//	// 使用glGetShaderiv获取顶点着色器编译时信息
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
//	// 将之前编译好的着色器附加到程序对象上
//	glAttachShader(shaderProgram[1], vertexShader[1]);
//	glAttachShader(shaderProgram[1], fragmentShader[1]);
//	glLinkProgram(shaderProgram[1]);
//	// 获取着色器编译状态
//	glGetProgramiv(shaderProgram[1], GL_LINK_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(shaderProgram[1], 512, NULL, infoLog);
//		std::cout << " ERROR:SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
//	}
//	// 在链接到程序对象后，因为不再需要着色器对象，则删除
//	glDeleteShader(vertexShader[0]);
//	glDeleteShader(vertexShader[1]);
//	glDeleteShader(fragmentShader[0]);
//	glDeleteShader(fragmentShader[1]);
//
//
//	/*
//		当具有多个图元对象时，我们需要使用EBO元素缓冲对象(或者叫索引缓冲对象)进行保存顶点数据。
//	*/
//	// 设置顶点数组对象、顶点缓冲对象、索引缓冲对象
//	unsigned int VAOs[2], VBOs[2], EBOs[2];
//	// 生成顶点缓冲对象
//	glGenBuffers(2, EBOs);
//	glGenBuffers(2, VBOs);
//	// 设置顶点数组对象
//	glGenVertexArrays(2, VAOs); // 声明生成顶点数组对象
//	
//	bindBufferSetting(VAOs[0], VBOs[0], EBOs[0]);
//	bindBufferSetting1(VAOs[1], VBOs[1]);
//	//// 绑定顶点缓冲缓冲
//	//glBindVertexArray(VAOs[0]);
//	//glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
//	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
//	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeices), indeices, GL_STATIC_DRAW);
//	//// 设置顶点属性
//	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	//glEnableVertexAttribArray(0);
//
//	//// 绑定顶点缓冲缓冲
//	//glBindVertexArray(VAOs[1]);
//	//// 绑定顶点缓冲对象
//	//glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
//	//glBufferData(GL_ARRAY_BUFFER, sizeof(secvertices), secvertices, GL_STATIC_DRAW);
//	//// 绑定索引缓冲对象
//	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
//	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(secindeices), secindeices, GL_STATIC_DRAW);
//	//// 设置顶点属性
//	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	//glEnableVertexAttribArray(0);
//
//
//	/*
//	* 需要注意的是在GL_ELEMENT_ARRAY_BUFFER时，不可以解绑定EBO，
//	* 因为此时的VAO保存有glBindBuffer的函数调用
//	* 这也意味着它也会储存解绑调用，
//	* 所以确保你没有在解绑VAO之前解绑索引数组缓冲，
//	* 否则它就没有这个EBO配置了。
//	*/
//	glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑定 
//	
//
//
//
//	// 监听窗口关闭的事件循环
//	while (!glfwWindowShouldClose(window.get())) {
//		// 输入按键处理
//		processInput(window.get());
//
//		// 渲染处理
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 清除颜色缓冲区使用的颜色
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		// 绘制物体
//		glUseProgram(shaderProgram[0]);
//		// 激活程序对象
//		glBindVertexArray(VAOs[0]);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//		
//		// 激活程序
		//glUseProgram(shaderProgram[1]);
		//// 获取glfw窗口运行时间
		//float timeValue = glfwGetTime();
		//// 设置随时间变化，绿色深色变化
		//float greenValue = sin(timeValue) / 2.0f + 0.3f;
		//float redValue = sin(timeValue) / 2.0f - 0.2f;
		//float yellowValue = sin(timeValue) / 2.0f ;
		//// 获取uniform变量
		//int vertexColorLocation = glGetUniformLocation(shaderProgram[1], "ourColor");
		//glUniform4f(vertexColorLocation, redValue, greenValue, yellowValue, 1.0f);
//
//
//		// 激活程序对象
//		glBindVertexArray(VAOs[1]);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//
//		// 双缓冲器声明
//		glfwSwapBuffers(window.get());
//		// 窗口事件监听
//		glfwPollEvents();
//	}
//	// 关闭前，需要清理顶点数组数据、顶点缓冲对象和着色器程序对象
//	glDeleteVertexArrays(2, VAOs);
//	glDeleteBuffers(2, EBOs);
//	glDeleteBuffers(2, VBOs);
//	glDeleteProgram(shaderProgram[0]);
//	glDeleteProgram(shaderProgram[1]);
//	glfwTerminate();
//	return 0;
//}