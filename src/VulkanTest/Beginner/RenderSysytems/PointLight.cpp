#include "PointLight.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <map>
#include <cassert>
#include <stdexcept>

namespace lve {

    struct PointLightPushConstants {
        glm::vec4 lightPosition;
        glm::vec4 lightColor;
        float radius ;
    };

    PointLight::PointLight(LveDevice& device, VkRenderPass renderPass,
        VkDescriptorSetLayout descriptorSetLayout)
        : lveDevice(device) {
        createPipelineLayout(descriptorSetLayout);
        createPipeline(renderPass);
    }

    PointLight::~PointLight() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void PointLight::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout )
    {
        VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = sizeof(PointLightPushConstants)
        };
        std::vector<VkDescriptorSetLayout> setLayouts{ descriptorSetLayout };
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }



    void PointLight::createPipeline(VkRenderPass& renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "src/VulkanTest/Shaders/point_light.vert.spv",
            "src/VulkanTest/Shaders/point_light.frag.spv",
            pipelineConfig);
    }


    void PointLight::update(FrameInfo& frameInfo, GLoableUbo& ubo)
    {
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
        int lightIndex = 0;
        for (auto& key : frameInfo.globalGameObjects)
        {
            auto& obj = key.second;
            if (obj.pointLight == nullptr) continue;
            assert(lightIndex < MAX_POINT_LIGHTS && "over the clamp light numbers");
            // update light position
            obj.transform.translation = 
             glm::vec3(rotateLight * glm::vec4(obj.transform.translation,1.f));
            // copy light from gameobj to ubo
            ubo.pointLights[lightIndex].lightPosition = 
                glm::vec4(obj.transform.translation, 1.f);
            ubo.pointLights[lightIndex].lightColor = 
                glm::vec4(obj.color, obj.pointLight->intensity);
            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
    }

    // call transform matrix that transform object coordinate  to relative to camera basis
    // finally using command push constant value to vertex shader
    void PointLight::render(FrameInfo& frameInfo) {
        lvePipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 
            1,&frameInfo.globalDescriptorSet,
            0,nullptr
        );
        std::map<float, uint32_t> disSorted;
        for (auto& key : frameInfo.globalGameObjects)
        {
            auto& obj = key.second;
            if (obj.pointLight == nullptr) continue;
            auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
            float distanceSquared = glm::dot(offset, offset);
            disSorted[distanceSquared] = obj.getId();
        }
        for(auto it = disSorted.rbegin(); it != disSorted.rend(); ++it)
        {   
            auto& obj = frameInfo.globalGameObjects.at(it->second);
            PointLightPushConstants push{
                .lightPosition = glm::vec4(obj.transform.translation,1.f),
                .lightColor = glm::vec4(obj.color, obj.pointLight->intensity),
                .radius = obj.transform.scale.x
            };
            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants), &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }

    }

}  // namespace lve