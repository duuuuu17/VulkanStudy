#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <string>
#include <memory>

namespace lve {
    // encapsulated Vertex data & setting InputBindingDescriptor/InputAttributeDescriptor
    // CmdBindVertexBuffer & CmdDraw command
    class LveModel {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            
            bool operator==(Vertex const& other) const
            {
                return (position == other.position) && (color == other.color)
                    && (uv == other.uv) && (normal == other.normal);
            }
        };
        struct Builder {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            void loadModelForFile(const std::string& filePath);
            
        };

        LveModel(LveDevice& device, const Builder& builder);
        ~LveModel();

        LveModel(const LveModel&) = delete;
        LveModel& operator=(const LveModel&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        static std::unique_ptr<LveModel> createGameModelForFile(LveDevice& device, const std::string& filePath);
    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);
        
        LveDevice& lveDevice;

        std::unique_ptr<LveBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<LveBuffer> indexBuffer;
        uint32_t indexCount;
    };
}  // namespace lve