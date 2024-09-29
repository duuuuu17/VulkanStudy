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

        // ����UBOs
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

        // ��������������
        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
        // ����������������
        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        // �ֱ𴴽���������������UBO����д�뵽����������
        for (size_t i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *gobalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        // �������ͳ�������
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

        // �������������
        LveCamera lveCamera{};
        // ���������ģ�Ͷ���
        auto viewObject = LveGameObject::createGameObject();
        viewObject.transform.translation.z = -2.5f;
        // ���������������
        KeyboardMovementController cameraController{};

        // ��ȡ��ǰʱ�䣬׼������frameTime
        auto currentTime = std::chrono::high_resolution_clock::now();

        // mainLoop
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            
            // ���㵱ǰframeTime
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            
            // ��������������ƶ�xzƽ�洦��������
            cameraController.moveInPlaneXZ(lveWindow.getWindow(), frameTime, viewObject);
            // ���ö�Ӧ�����ת������
            lveCamera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);
            // ����͸��ͶӰ����
            float aspect = lveRenderer.getAspectRatio();
            lveCamera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            
            // prepare begin-commandbuffer & begin-RenderPass
            if (auto commandbuffer = lveRenderer.beginFrames())
            {   
                // ����ÿ֡�������Ϣ
                int frameIndex = lveRenderer.getCurrentFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandbuffer,
                    lveCamera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };
                // ����ͳһ����������
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

        // ��Ҫע���gameObjects ������Ⱦmodel������Ҫ��鵱ǰ������obj.model�Ƿ�Ϊ��(��������)��
        // ͬ���ڴ���pointLightʱ������obj.pointLightʱҲ��Ҫ����Ƿ�Ϊ��(��������)
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
                (i * glm::two_pi<float>()) / lightColors.size(), // ��Բ����lightColors.size����
                { 0.f, -1.f, 0.f }
            );
            pointLight.transform.translation =
                glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
        
    }
    

}  // namespace lve