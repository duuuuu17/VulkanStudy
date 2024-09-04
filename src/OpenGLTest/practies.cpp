//// 流水线--渲染通道构建
//std::vector<char> readFile(const std::string& filename)
//{
//	std::ifstream file(filename, std::ios::ate | std::ios::binary);
//	if (!file.is_open())
//		throw std::runtime_error("Failed to open the file:!");
//	size_t size = size_t(file.tellg());
//	std::vector<char> buffer(size);
//	file.seekg(0);
//	file.read(buffer.data(), size);
//	file.close();
//	return buffer;
//}
//VkShaderModule createShaderModule(const std::vector<char>& code)
//{
//	VkShaderModuleCreateInfo createInfo{
//	.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
//	.codeSize = code.size(),
//	.pCode = reinterpret_cast<const uint32_t*>(code.data())
//	};
//	VkShaderModule shaderModule;
//	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
//		throw std::runtime_error("Failed to create shader module!");
//	return shaderModule;
//}
//void createRenderPass()
//{
//	// 创建颜色附件
//	VkAttachmentDescription colorAttachment{
//		.format = swapChainImageFormat, // 指定颜色格式与交换链图形格式相同
//		.samples = VK_SAMPLE_COUNT_1_BIT, // 单个像素采样次数
//		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // 指定附件加载方式，加载前清除缓冲区
//		.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // 指定附件存储方式，将附件存储缓冲区于内存中
//		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // 指定模板缓冲区图形的加载操作类型
//		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // 指定模板缓冲区图形的存储操作类型
//		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // 指定渲染前附件处理方式
//		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // 指定渲染后附件处理方式
//	};
//	// 创建颜色附件引用，指定附件数组索引,指定该附件是什么类型的缓冲区
//	VkAttachmentReference colorAttachementRef{
//		.attachment = 0, // 附件数组的索引
//		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // 指定附件作为颜色缓冲区
//	};
//	// 创建子通道,指定子通道中的附件，并且该子通道类型
//	VkSubpassDescription subpass{
//		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, // 该子通道为图形子通道
//		.colorAttachmentCount = 1,
//		.pColorAttachments = &colorAttachementRef // 指定我们刚才创建的颜色附件引用
//	};
//	// 创建渲染通道。note：一个渲染通道具有多个子通道
//	VkRenderPassCreateInfo renderpassCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//		.attachmentCount = 1,
//		.pAttachments = &colorAttachment,
//		.subpassCount = 1,
//		.pSubpasses = &subpass,
//	};
//	if (vkCreateRenderPass(device, &renderpassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
//		throw std::runtime_error("Faile to create render pass!");
//}
//
//void createGraphicsPipline()
//{
//	// 着色器模块构建
//	// 读取代码文件
//	auto vertCode = readFile("src/VulkanTest/Shaders/vert.spv");
//	auto fragCode = readFile("src/VulkanTest/Shaders/frag.spv");
//	// 创建着色器模块
//	auto vertexShaderModule = createShaderModule(vertCode);
//	auto fragShaderModule = createShaderModule(fragCode);
//	// 创建图形管线中着色器阶段的结构体
//	VkPipelineShaderStageCreateInfo vertexShaderModuleCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//		.stage = VK_SHADER_STAGE_VERTEX_BIT,
//		.module = vertexShaderModule,
//		.pName = "main"
//	};
//	VkPipelineShaderStageCreateInfo fragShaderModuleCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
//		.module = fragShaderModule,
//		.pName = "main"
//	};
//	// 将图形着色器阶段结构体存放至数组中
//	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderModuleCreateInfo, fragShaderModuleCreateInfo };
//	// 动态状态，设置视口和裁剪体可以动态修改
//	std::vector<VkDynamicState> dynamicState{
//	VK_DYNAMIC_STATE_VIEWPORT,
//	VK_DYNAMIC_STATE_SCISSOR
//	};
//	VkPipelineDynamicStateCreateInfo dynamicCreateInfo;
//	dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//	dynamicCreateInfo.dynamicStateCount = 2;
//	dynamicCreateInfo.pDynamicStates = dynamicState.data();
//	// 顶点输入
//	VkPipelineVertexInputStateCreateInfo inputCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//		.vertexBindingDescriptionCount = 0,
//		.pVertexBindingDescriptions = nullptr,
//		.vertexAttributeDescriptionCount = 0,
//		.pVertexAttributeDescriptions = nullptr
//	};
//	// 输入组装
//	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
//		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
//		.primitiveRestartEnable = VK_TRUE // 图元光栅化启动，在光栅化过程中会裁剪图像
//	};
//	// 视口和裁剪矩形
//	VkPipelineViewportStateCreateInfo viewportCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//		.viewportCount = 1,
//		.scissorCount = 1
//	};
//	// 光栅阶段需要处理
//	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//		.depthClampEnable = VK_FALSE, // 深度限制，若启用，则超过近平面和远平面，器图像不被删除
//		.rasterizerDiscardEnable = VK_FALSE, // 光栅化丢弃，若启用几何图像不经过光栅化阶段
//		.polygonMode = VK_POLYGON_MODE_FILL, //几何填充方式
//		.cullMode = VK_CULL_MODE_BACK_BIT, // 背面剔除方式
//		.frontFace = VK_FRONT_FACE_CLOCKWISE, // 正面顶点的顺序
//		.depthBiasEnable = VK_FALSE, // 斜率偏执不启用
//		.lineWidth = 1.0f // 线段宽度，以片段数量来描述线条数量
//	};
//	// 多重采样阶段
//	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT, // 单个像素采样次数
//		.sampleShadingEnable = VK_TRUE, // 是否启用采样着色
//		.minSampleShading = 1.0f, // 单个像素最小采样着色次数
//		.pSampleMask = nullptr, // 选择哪些像素被采样
//		.alphaToCoverageEnable = VK_FALSE,
//		.alphaToOneEnable = VK_FALSE
//	};
//	// 深度与模板测试, 此处未使用，在创建图像管线对象传递参数为nullptr
//	// 颜色混合
//	// 先创建颜色附件,指明颜色的操作方式
//	VkPipelineColorBlendAttachmentState colorAttachement{
//		.blendEnable = VK_FALSE,
//		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
//		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,// 指明哪些颜色分量被写入缓冲区
//	};
//	// 再创建颜色混合阶段，将颜色附件传递该阶段对象
//	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//		.logicOpEnable = VK_FALSE, // 是否启用逻辑运算
//		.logicOp = VK_LOGIC_OP_COPY, // 逻辑运算类型
//		.attachmentCount = 1, // 附件个数
//		.pAttachments = &colorAttachement, // 指向附件对象
//		.blendConstants = 0.0f // 混合常量，指颜色四个分量的混合值
//	};
//	// 创建管线布局
//	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//		.setLayoutCount = 0,
//		.pSetLayouts = nullptr,
//		.pushConstantRangeCount = 0,
//		.pPushConstantRanges = nullptr
//	};
//	if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
//		throw std::runtime_error("Failed to create pipeline layout!");
//
//
//	// 在使用完着色器模块后，需要将它们清除
//	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
//	vkDestroyShaderModule(device, fragShaderModule, nullptr);
//}
