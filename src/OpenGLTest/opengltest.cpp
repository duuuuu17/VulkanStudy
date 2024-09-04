////#include "./shader_s.h"
////#include <glad/glad.h>
////#include <GLFW/glfw3.h>
////#include "./stb_image.h"
////#include <iostream>
////
////void framebufferSizeCallback(GLFWwindow* window, int width, int height);
////void processInput(GLFWwindow* window);
////
////// settings
////const unsigned int SCR_WIDTH = 800;
////const unsigned int SCR_HEIGHT = 600;
////int main() {
////	/*初始化GLFW版本信息，以及创建GLFW窗口并加载到当前OpenGL渲染的上下文中，
////	设置framebuffersizecallback函数
////	最后加载GLAD管理GLFW窗口指针*/
////	glfwInit();
////	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
////	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
////	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
////
//////#ifndef __APPLE__
//////	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//////#endif // !__APPLE__
////
////	// glfw window creation
////	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shader Class Test", nullptr, nullptr);
////	if (window == nullptr) {
////		std::cout << "Failed to craete GLFW window" << std::endl;
////		glfwTerminate();
////		return -1;
////	}
////	glfwMakeContextCurrent(window);
////	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
////	// glad: load all OpenGL function pointers
////	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
////		std::cout << "Failed to initialize GLAD" << std::endl;
////		return -1;
////	}
////	Shader shader("src\\vshader.txt", "src\\fshader.txt");
////	float vertices[] = {
////		// positions         // colors
////		0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,// bottom right
////		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,// bottom left
////		0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   0.5f, 1.0f// top 
////	};
////	
////	// 生成VAO和VBO，并绑定数据和设置顶点属性
////	unsigned int VAO, VBO;
////	glGenVertexArrays(1, &VAO); 
////	glGenBuffers(1, &VBO);
////	// 绑定
////	glBindVertexArray(VAO);
////	glBindBuffer(GL_ARRAY_BUFFER, VBO);
////	// 加载VBO数据
////	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
////
////	// 解释顶点数组中向量的属性并启用属性:指定顶点向量和颜色向量索引
////	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
////	glEnableVertexAttribArray(0);
////	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
////	glEnableVertexAttribArray(1);
////	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(6 * sizeof(float)));
////	glEnableVertexAttribArray(2);
////	
////	// 设置texture纹理
////	unsigned int texture;
////	glGenTextures(1, &texture);
////	glBindTexture(GL_TEXTURE_2D, texture);
////	// 设置环绕方式
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
////	// 设置过滤方式
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
////	// 加载并生成纹理
////	int width, height, nrChannels;
////	unsigned char* data = stbi_load("src\\boqi.jpg", &width, &height, &nrChannels, 0);
////	if (data) { //色湖之多级渐远纹理
////		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
////		glGenerateMipmap(GL_TEXTURE_2D);
////	}
////	else {
////		std::cout << "Failed to load texture" << std::endl;
////	}
////	stbi_image_free(data);
////	
////	
////	
////	
////
////
////
////	while (!glfwWindowShouldClose(window))
////	{
////		// 处理输入
////		processInput(window);
////
////		// 渲染
////		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
////		glClear(GL_COLOR_BUFFER_BIT);
////		// 参数设置
////		// 在GLSL中声明的layout in vec3 aPos，不可更改，需要使用中间变量进行水平移动
////		float timeValue = glfwGetTime();
////		float offset = (sin(timeValue) / 2.0f) ;
////		shader.setfloat("uhorizontalOffset", offset); 
////		// 画出图形
////		
////		// 绑定纹理
////		glBindTexture(GL_TEXTURE_2D, texture);
////		shader.use(); // == glUseProgram()
////		glBindVertexArray(VAO); // 绑定顶点数组对象
////		glDrawArrays(GL_TRIANGLES, 0, 3); // 指定图形，以及定点数
////		// 设置双缓冲
////		glfwSwapBuffers(window);
////		// 轮询事件
////		glfwPollEvents();
////	}
////	// 删除缓冲
////	glDeleteVertexArrays(1, & VAO);
////	glDeleteBuffers(1, &VBO);
////	glfwTerminate();
////
////	return 0;
////}
////
////void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
////	glViewport(0, 0, width, height);
////}
////
////void processInput(GLFWwindow* window) {
////	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
////		glfwSetWindowShouldClose(window, true);
////}
//#include <include/glad/glad.h>
//#include <GLFW/glfw3.h>
//#define STB_IMAGE_IMPLEMENTATION
//#include "./stb_image.h"
//
//#include "./shader_s.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <iostream>
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void processInput(GLFWwindow* window);
//// settings
//const unsigned int SCR_WIDTH = 800;
//const unsigned int SCR_HEIGHT = 600;
//
//float mixValue = 0.2f;
//int main()
//{
//    // glfw: initialize and configure
//    // ------------------------------
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//#ifdef __APPLE__
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif
//
//    // glfw window creation
//    // --------------------
//    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
//    if (window == NULL)
//    {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//    // glad: load all OpenGL function pointers
//    // ---------------------------------------
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    // build and compile our shader zprogram
//    // ------------------------------------
//    Shader ourShader("src\\vshader.txt", "src\\fshader.txt");
//
//    // set up vertex data (and buffer(s)) and configure vertex attributes
//    // ------------------------------------------------------------------
//    float vertices[] = {
//        // positions          // colors           // texture coords (note that we changed them to 'zoom in' on our texture image)
//         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   2.0f, 2.0f, // top right
//         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   2.0f, 0.0f, // bottom right
//        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
//        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 2.0f  // top left 
//    };
//    unsigned int indices[] = {
//        0, 1, 3, // first triangle
//        1, 2, 3  // second triangle
//    };
//    unsigned int VBO, VAO, EBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &EBO);
//
//    glBindVertexArray(VAO);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//
//    // position attribute
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//    // color attribute
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
//    // texture coord attribute
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//    glEnableVertexAttribArray(2);
//
//
//    // load and create a texture 
//    // -------------------------
//    unsigned int texture1, texture2;
//    // texture 1
//    // ---------
//    glGenTextures(1, &texture1);
//    glBindTexture(GL_TEXTURE_2D, texture1);
//    // set the texture wrapping parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // note that we set the container wrapping method to GL_CLAMP_TO_EDGE
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    // set texture filtering parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // set texture filtering to nearest neighbor to clearly see the texels/pixels
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    // load image, create texture and generate mipmaps
//    int width, height, nrChannels;
//    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
//    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
//    std::string path = "D:\\visualstudioproject\\opengl\\src\\container.jpg";
//    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
//    if (data)
//    {
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//    }
//    else
//    {
//        std::cout << "Failed to load texture" << std::endl;
//    }
//    stbi_image_free(data);
//    // texture 2
//    // ---------
//    glGenTextures(1, &texture2);
//    glBindTexture(GL_TEXTURE_2D, texture2);
//    // set the texture wrapping parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    // set texture filtering parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // set texture filtering to nearest neighbor to clearly see the texels/pixels
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    // load image, create texture and generate mipmaps
//    path = "D:\\visualstudioproject\\opengl\\src\\awesomeface.png";
//    data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
//    if (data)
//    {
//        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//    }
//    else
//    {
//        std::cout << "Failed to load texture" << std::endl;
//    }
//    stbi_image_free(data);
//
//    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
//    // -------------------------------------------------------------------------------------------
//    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
//    // either set it manually like so:
//    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
//    // or set it via the texture class
//    ourShader.setInt("texture2", 1);
//
//
//
//    // render loop
//    // -----------
//    while (!glfwWindowShouldClose(window))
//    {   
//        // input
//        // -----
//
//        processInput(window);
//
//        // render
//        // ------
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // bind textures on corresponding texture units
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, texture1);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, texture2);
//        glUniform1f(glGetUniformLocation(ourShader.ID, "uhorizontalOffset"), mixValue);
//    
//
//        glm::mat4 trans;
//        trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
//        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
//        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
//        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
//        ourShader.setfloat("p", sin((float)glfwGetTime()));
//        // render container
//        ourShader.use();
//        glBindVertexArray(VAO);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//
//        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//        // -------------------------------------------------------------------------------
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // optional: de-allocate all resources once they've outlived their purpose:
//    // ------------------------------------------------------------------------
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteBuffers(1, &EBO);
//
//    // glfw: terminate, clearing all previously allocated GLFW resources.
//    // ------------------------------------------------------------------
//    glfwTerminate();
//    return 0;
//}
//
//// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
//// ---------------------------------------------------------------------------------------------------------
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//    {
//        glfwSetWindowShouldClose(window, true);
//        mixValue = 0.0f;
//    }
//    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
//    {   
//        mixValue += 0.01f;
//        if (mixValue >= 1.0f)
//            mixValue =  1.0f;
//    }
//    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
//    {   
//        mixValue -= 0.01f;
//        if (mixValue <= 0.1f)
//            mixValue =  0.0f;
//    }
//    else
//        mixValue =  0.0f;
//}
//
//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    // make sure the viewport matches the new window dimensions; note that width and 
//    // height will be significantly larger than specified on retina displays.
//    glViewport(0, 0, width, height);
//}
