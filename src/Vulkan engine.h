#ifndef VULKANENGINE_H
#define VULKANENGINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include "algebra.h"
#include "model.h"
const int MAX_FRAMES_IN_FLIGHT = 1;
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats = 0;
    VkPresentModeKHR* presentModes = 0;
    ~SwapChainSupportDetails()
    {
        delete []formats;
        delete []presentModes;
    }
};
struct VPmatrices
{
    alignas(16) mat4 view;
    alignas(16) mat4 proj;
    alignas(16) mat4 viewProj;
};
struct ModelMatrix
{
    alignas(16) mat4 model;
};
struct FrameInFlight
{
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkCommandBuffer commandBuffer;
};
struct Vulkan_struct
{
    struct FrameInFlight
    {
        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        VkBuffer VPmatricesUniformBuffer;
        VkDeviceMemory VPmatricesUniformBufferMemory;

        VkDescriptorSet descriptorSet;
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;
    };
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsPresentQueue;
    uint32_t graphicsPresentQueueFamilyIndex;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    //VkImage textureImage;
    //VkDeviceMemory textureImageMemory;
    //VkImageView textureImageView;
    //сэмплер применяется 1 раз для текстур с одинаковой фильтрацией
    VkSampler textureSampler;

    VkDescriptorPool descriptorPool;
    VkDescriptorPool ModeldataDescriptorPool;
    FrameInFlight framesVkVariables[MAX_FRAMES_IN_FLIGHT];
    //std::vector<VkBuffer> uniformBuffers;
    //std::vector<VkDeviceMemory> uniformBuffersMemory;

    //std::vector<VkDescriptorSet> descriptorSets;

    //std::vector<VkCommandBuffer> commandBuffers;

    //std::vector<VkSemaphore> imageAvailableSemaphores;
    //std::vector<VkSemaphore> renderFinishedSemaphores;
    //std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0;
    void init(GLFWwindow *window, Modelset *models);
    void init_Vulkan(GLFWwindow *window){
        createInstance();
        createSurface(window);
        pickPhysicalDevice();
        createLogicalDevice();
    }
    void cleanupSwapChain();
    void cleanup(Modelset *modelset);
    void createInstance();
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(GLFWwindow *window);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    void createRenderPass();
    void createDescriptorSetLayout();
    VkShaderModule createShaderModule(unsigned char *shader_buffer, unsigned long shader_size);
    void createGraphicsPipeline();
    void createCommandPool();
    void createDescriptorPool();
    void createModeldataDescriptorPool(Modelset *modelset);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createDepthResources();
    void createFramebuffers();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createTextureImage(Model::Material::Texture *texture);
    void createTextureImageView(Model::Material::Texture *texture);
    void createTextureSampler();
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createVertexBuffer(Model::Mesh *mesh);
    void createIndexBuffer(Model::Mesh *mesh);
    void createUniformBuffers(Model *model);
    void createDescriptorSets(Model *model);
    void createModeldataDescriptorSets(Modelset *modelset);
    void createCommandBuffers();
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, Modelset *modelset);
    void recreateSwapChain(GLFWwindow *window);
    void drawFrame(GLFWwindow *window, Modelset *modelset);
};
#endif
