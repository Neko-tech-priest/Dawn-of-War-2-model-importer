#ifndef MODEL_H
#define MODEL_H
#include "algebra.h"
#include <cstdint>
#include <vulkan/vulkan.h>
struct Model
{
    struct Material
    {
        char *materialName = 0;
        uint16_t materialNameLength = 0;
        struct Texture
        {
            char *textureName = 0;
            uint16_t textureNameLength = 0;
            uint8_t *texturePixels = 0;
            uint32_t texturePixelsSize = 0;
            uint32_t texWidth;
            uint32_t texHeight;
            VkFormat texFormat;
            VkImage textureImage;
            VkImageView textureImageView;
            VkDeviceMemory textureImageMemory;
            ~Texture()
            {
                delete []textureName;
                delete []texturePixels;
            }
        };
        //Texture dif_Texture;
        VkDescriptorSet descriptorSet;
        Texture textures[6];
        ~Material()
        {
            delete []materialName;
        }
    };
    struct Bone
    {
        char *boneName = 0;
        uint16_t boneNameLength = 0;
        uint32_t parent = 0;
        mat4 relMatrix;
        mat4 invRelMatrix;
        mat4 worldMatrix;
        mat4 invWorldMatrix;
        //mat4 inverseWorldMatrix;
        ~Bone()
        {
            delete[] boneName;
        }
    };
    struct Marker
    {
        struct Param
        {
            char *key = 0;
            uint16_t keyLength = 0;
            char *value = 0;
            uint16_t valueLength = 0;
            ~Param()
            {
                delete[] key;
                delete[] value;
            }
        };
        char *markerName = 0;//без 0
        uint16_t markerNameLength = 0;
        char *parent = 0;
        uint16_t parentLength = 0;
        mat4 matrix;
        Param *params = 0;
        uint16_t numParams = 0;
        ~Marker()
        {
            delete[] markerName;
            delete[] parent;
            delete[] params;
        }
    };
    struct Mesh
    {
        struct Vertex
        {
            vec3 position;
            vec3 normal;
            vec3 binormal;
            vec3 tangent;
            vec2 uv1;
            uint32_t blendIndices[4] = {0, 0, 0, 0};
            vec4 blendWeights = {0, 0, 0, 0};
            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;// layout(binding = 0) in
                bindingDescription.stride = sizeof(Vertex);
                //переходить к следующей записи данных после каждой вершины или экземпляра
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }
            //как обрабатывать ввод вершин
            static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions()
            {
                std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};

                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(Vertex, position);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(Vertex, normal);

                attributeDescriptions[2].binding = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[2].offset = offsetof(Vertex, binormal);

                attributeDescriptions[3].binding = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[3].offset = offsetof(Vertex, tangent);

                attributeDescriptions[4].binding = 0;
                attributeDescriptions[4].location = 4;
                attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[4].offset = offsetof(Vertex, uv1);

                attributeDescriptions[5].binding = 0;
                attributeDescriptions[5].location = 5;
                attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_UINT;
                attributeDescriptions[5].offset = offsetof(Vertex, blendIndices);

                attributeDescriptions[6].binding = 0;
                attributeDescriptions[6].location = 6;
                attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attributeDescriptions[6].offset = offsetof(Vertex, blendWeights);

                return attributeDescriptions;
            }
        };
        struct Bone
        {
            char *boneName = 0;
            uint16_t boneNameLength = 0;
            uint32_t parent = 0;
            mat4 relMatrix;
            mat4 worldMatrix;
            mat4 invWorldMatrix;
            vec3 quatPosition;
            Quaternion quatRotation;
        };
        //связь с нужным материалом
        char *materialName = 0;
        Vertex *vertices = 0;
        uint16_t *indices = 0;
        Bone *bones = 0;
        uint16_t materialNameLength = 0;
        uint16_t numVertices = 0;
        uint32_t numIndices = 0;
        uint16_t numBones = 0;
        //graphics memory
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        ~Mesh()
        {
            delete[] vertices;
            delete[] indices;
            //delete[] bones;
        }
    };
    struct Animation
    {
        struct Bone
        {
            char *boneName = 0;
            uint16_t boneNameLength = 0;
            mat4 relMatrix;
            mat4 worldMatrix;
            mat4 invWorldMatrix;
            uint32_t parent = 0;
            ~Bone()
            {
                delete[] boneName;
            }
        };
        struct Frame
        {
            mat4 *relBones = 0;
            mat4 *worldBones = 0;
            ~Frame()
            {
                delete[] relBones;
                delete[] worldBones;
            }
        };
        char *animationName = 0;
        Bone *bones = 0;
        Frame *frames = 0;
        mat4 *framesData = 0;
        uint16_t animationNameLength = 0;
        uint16_t numBones = 0;
        uint16_t numFrames = 0;
        ~Animation()
        {
            delete[] animationName;
            delete[] bones;
            delete[] frames;
            delete[] framesData;
        }
    };
    char *modelName = 0;
    uint16_t modelNameLength = 0;
    Material *materials = 0;
    uint8_t numMaterials = 0;
    Bone *bones = 0;
    uint16_t numBones = 0;
    Marker *markers = 0;
    uint16_t numMarkers = 0;
    Mesh *meshes = 0;
    uint8_t numMeshes = 0;
    Animation animation;
    //graphics
    VkBuffer uniformBonesBuffer;
    VkDeviceMemory uniformBonesBufferMemory;
    ~Model()
    {
        delete[] modelName;
        delete[] materials;
        delete[] bones;
        delete[] markers;
        delete[] meshes;
    }
};
struct Modelset
{
    Model *models;
    uint8_t numModels = 0;
    uint8_t numMeshes = 0;
    //uint8_t numBones = 0;
    VkBuffer matrixUnitUniformBuffer;
    VkDeviceMemory matrixUnitUniformBufferMemory;
    ~Modelset()
    {
        delete[] models;
    }
};
#endif
