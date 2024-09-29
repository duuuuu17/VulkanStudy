#include "first_app.hpp"
#include "lve_camera.hpp"
#include "Keyboard_Movement_Controller.hpp"
#include "RenderSysytems/PointLight.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <chrono>
namespace lve {



    FirstApp::FirstApp() {
        gobalDescriptorPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }

    FirstApp::~FirstApp() { }

    void FirstApp::run() {

        // 创建UBOs
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < uboBuffers.size(); ++i)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GLoableUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffers[i]->map();
        }

        // 创建描述符布局
        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
        // 创建描述符集数组
        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        // 分别创建描述符集，并将UBO数据写入到描述符集中
        for (size_t i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *gobalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        // 创建推送常量对象
        SimpleRenderSystem simpleRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };
        PointLight pointLightSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        // 创建摄像机对象
        LveCamera lveCamera{};
        // 创建摄像机模型对象
        auto viewObject = LveGameObject::createGameObject();
        viewObject.transform.translation.z = -2.5f;
        // 创建按键处理对象
        KeyboardMovementController cameraController{};

        // 获取当前时间，准备计算frameTime
        auto currentTime = std::chrono::high_resolution_clock::now();

        // mainLoop
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            
            // 计算当前frameTime
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            
            // 设置摄像机对象移动xz平面处理函数调用
            cameraController.moveInPlaneXZ(lveWindow.getWindow(), frameTime, viewObject);
            // 设置对应的相机转换矩阵
            lveCamera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);
            // 设置透视投影矩阵
            float aspect = lveRenderer.getAspectRatio();
            lveCamera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            
            // prepare begin-commandbuffer & begin-RenderPass
            if (auto commandbuffer = lveRenderer.beginFrames())
            {   
                // 更新每帧的相关信息
                int frameIndex = lveRenderer.getCurrentFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandbuffer,
                    lveCamera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };
                // 更新统一缓冲区数据
                GLoableUbo ubo{
                    .projectionMatrix = lveCamera.getProjcetion() ,
                    .viewMatrix =  lveCamera.getView(),
                    .inverseView = lveCamera.getInverseViewMatrixw()
                };
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // Render
                lveRenderer.beginSwapChainRenderPass(commandbuffer);
                simpleRenderSystem.renderGameObjects(frameInfo); 
                pointLightSystem.render(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandbuffer);
                lveRenderer.endFrames();
            }
        }
        vkDeviceWaitIdle(lveDevice.device());
    }


    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel = LveModel::createGameModelForFile(lveDevice, "src/VulkanTest/models/flat_vase.obj");
        auto flatVase = LveGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {- .5f, .5f, .0f};
        flatVase.transform.scale = glm::vec3{ 3.f,1.5f,3.f }; //glm::vec3{ 3.f };
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        lveModel = LveModel::createGameModelForFile(lveDevice, "src/VulkanTest/models/smooth_vase.obj");
        auto smoothObj = LveGameObject::createGameObject();
        smoothObj.model = lveModel;
        smoothObj.transform.translation = { .5f, .5f, .0f };
        smoothObj.transform.scale = glm::vec3{ 3.f,1.5f,3.f }; //glm::vec3{ 3.f };
        gameObjects.emplace(smoothObj.getId(), std::move(smoothObj));

        // 需要注意的gameObjects 中在渲染model部分需要检查当前遍历的obj.model是否为空(跳过处理)。
        // 同理：在处理pointLight时，遍历obj.pointLight时也需要检查是否为空(跳过处理)
        lveModel = LveModel::createGameModelForFile(lveDevice, "src/VulkanTest/models/quad.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = { .0f, .5f, .0f };
        floor.transform.scale = glm::vec3{ 3.f,1.f,3.f }; //glm::vec3{ 3.f };
        gameObjects.emplace(floor.getId(),std::move(floor));
        
        // add point light obj
        std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}  };
        for (size_t i = 0; i < lightColors.size(); ++i)
        {
            auto pointLight = LveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(), // 将圆均分lightColors.size个弧
                { 0.f, -1.f, 0.f }
            );
            pointLight.transform.translation =
                glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
        
    }
    

}  // namespace lve