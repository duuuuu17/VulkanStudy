//// ��ˮ��--��Ⱦͨ������
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
//	// ������ɫ����
//	VkAttachmentDescription colorAttachment{
//		.format = swapChainImageFormat, // ָ����ɫ��ʽ�뽻����ͼ�θ�ʽ��ͬ
//		.samples = VK_SAMPLE_COUNT_1_BIT, // �������ز�������
//		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // ָ���������ط�ʽ������ǰ���������
//		.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // ָ�������洢��ʽ���������洢���������ڴ���
//		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // ָ��ģ�建����ͼ�εļ��ز�������
//		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // ָ��ģ�建����ͼ�εĴ洢��������
//		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // ָ����Ⱦǰ��������ʽ
//		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // ָ����Ⱦ�󸽼�����ʽ
//	};
//	// ������ɫ�������ã�ָ��������������,ָ���ø�����ʲô���͵Ļ�����
//	VkAttachmentReference colorAttachementRef{
//		.attachment = 0, // �������������
//		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // ָ��������Ϊ��ɫ������
//	};
//	// ������ͨ��,ָ����ͨ���еĸ��������Ҹ���ͨ������
//	VkSubpassDescription subpass{
//		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, // ����ͨ��Ϊͼ����ͨ��
//		.colorAttachmentCount = 1,
//		.pColorAttachments = &colorAttachementRef // ָ�����ǸղŴ�������ɫ��������
//	};
//	// ������Ⱦͨ����note��һ����Ⱦͨ�����ж����ͨ��
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
//	// ��ɫ��ģ�鹹��
//	// ��ȡ�����ļ�
//	auto vertCode = readFile("src/VulkanTest/Shaders/vert.spv");
//	auto fragCode = readFile("src/VulkanTest/Shaders/frag.spv");
//	// ������ɫ��ģ��
//	auto vertexShaderModule = createShaderModule(vertCode);
//	auto fragShaderModule = createShaderModule(fragCode);
//	// ����ͼ�ι�������ɫ���׶εĽṹ��
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
//	// ��ͼ����ɫ���׶νṹ������������
//	VkPipelineShaderStageCreateInfo stages[] = { vertexShaderModuleCreateInfo, fragShaderModuleCreateInfo };
//	// ��̬״̬�������ӿںͲü�����Զ�̬�޸�
//	std::vector<VkDynamicState> dynamicState{
//	VK_DYNAMIC_STATE_VIEWPORT,
//	VK_DYNAMIC_STATE_SCISSOR
//	};
//	VkPipelineDynamicStateCreateInfo dynamicCreateInfo;
//	dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//	dynamicCreateInfo.dynamicStateCount = 2;
//	dynamicCreateInfo.pDynamicStates = dynamicState.data();
//	// ��������
//	VkPipelineVertexInputStateCreateInfo inputCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//		.vertexBindingDescriptionCount = 0,
//		.pVertexBindingDescriptions = nullptr,
//		.vertexAttributeDescriptionCount = 0,
//		.pVertexAttributeDescriptions = nullptr
//	};
//	// ������װ
//	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
//		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
//		.primitiveRestartEnable = VK_TRUE // ͼԪ��դ���������ڹ�դ�������л�ü�ͼ��
//	};
//	// �ӿںͲü�����
//	VkPipelineViewportStateCreateInfo viewportCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//		.viewportCount = 1,
//		.scissorCount = 1
//	};
//	// ��դ�׶���Ҫ����
//	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//		.depthClampEnable = VK_FALSE, // ������ƣ������ã��򳬹���ƽ���Զƽ�棬��ͼ�񲻱�ɾ��
//		.rasterizerDiscardEnable = VK_FALSE, // ��դ�������������ü���ͼ�񲻾�����դ���׶�
//		.polygonMode = VK_POLYGON_MODE_FILL, //������䷽ʽ
//		.cullMode = VK_CULL_MODE_BACK_BIT, // �����޳���ʽ
//		.frontFace = VK_FRONT_FACE_CLOCKWISE, // ���涥���˳��
//		.depthBiasEnable = VK_FALSE, // б��ƫִ������
//		.lineWidth = 1.0f // �߶ο�ȣ���Ƭ��������������������
//	};
//	// ���ز����׶�
//	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT, // �������ز�������
//		.sampleShadingEnable = VK_TRUE, // �Ƿ����ò�����ɫ
//		.minSampleShading = 1.0f, // ����������С������ɫ����
//		.pSampleMask = nullptr, // ѡ����Щ���ر�����
//		.alphaToCoverageEnable = VK_FALSE,
//		.alphaToOneEnable = VK_FALSE
//	};
//	// �����ģ�����, �˴�δʹ�ã��ڴ���ͼ����߶��󴫵ݲ���Ϊnullptr
//	// ��ɫ���
//	// �ȴ�����ɫ����,ָ����ɫ�Ĳ�����ʽ
//	VkPipelineColorBlendAttachmentState colorAttachement{
//		.blendEnable = VK_FALSE,
//		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
//		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,// ָ����Щ��ɫ������д�뻺����
//	};
//	// �ٴ�����ɫ��Ͻ׶Σ�����ɫ�������ݸý׶ζ���
//	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//		.logicOpEnable = VK_FALSE, // �Ƿ������߼�����
//		.logicOp = VK_LOGIC_OP_COPY, // �߼���������
//		.attachmentCount = 1, // ��������
//		.pAttachments = &colorAttachement, // ָ�򸽼�����
//		.blendConstants = 0.0f // ��ϳ�����ָ��ɫ�ĸ������Ļ��ֵ
//	};
//	// �������߲���
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
//	// ��ʹ������ɫ��ģ�����Ҫ���������
//	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
//	vkDestroyShaderModule(device, fragShaderModule, nullptr);
//}
