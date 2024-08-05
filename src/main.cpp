#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb/stb_image.h>

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

#include <termios.h>
#include <unistd.h>

#include "Vulkan engine.h"
#include "algebra.h"
#include "model.h"
extern bool framebufferResized;
void loader_dds(char *name_texture, VkFormat *format,uint32_t *Width, uint32_t *Height, uint32_t *ImageSize, uint8_t **texture_buffer);
void decompressor_DXT(char *name_texture, uint32_t *Width, uint32_t *Height, uint32_t *ImageSize, uint8_t **texture_buffer);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void SupremeCommander_model_import(Model *model, char *modelName, size_t modelNameLength);
void SupremeCommander_model_export(Model *model, char *directoryName, size_t directoryNameLength);
void Modelset_import(char *modelNames, size_t fileSize, Modelset *modelset);
void SpaceMarine_model_import(Model *model, char *modelName, size_t modelNameLength);
int main()
{
    //инициализация GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(600, 600, "Vulkan", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);
    Modelset modelset;
    Model SpaceMarine_model;
    //packages/boltpistol/boltpistol.dcm
    //packages/weirdboy_staff/weirdboy_staff.dcm
    //SpaceMarine_model_import(&SpaceMarine_model, "packages/weirdboy_staff/weirdboy_staff.dcm", std::size("packages/weirdboy_staff/weirdboy_staff.dcm"));
    //art/race_chaos/structures/chaos_generator/chaos_generator.model

    /*
    art/race_chaos/troops_wargear/armour/chaos_marine_common/chaos_marine_common.model
    art/race_chaos/troops_wargear/heads/chaos_marine_common/chaos_marine_common_head.model
    art/race_chaos/troops_wargear/weapons_ranged/combi_flamer_common/combiflamer_common.model
    */

    /*
    art/race_chaos/troops/chaos_lord/chaos_lord.model
    art/race_chaos/troops_wargear/armour/chaos_lord_common/chaos_lord_common.model
    art/race_chaos/troops_wargear/weapons_melee/power_sword_chaos_lord/power_sword_chaos_lord.model
    */
    /*
    art/race_chaos/troops/chaos_sorceror/chaos_sorceror.model
    art/race_chaos/troops_wargear/armour/chaos_sorcerer_common/chaos_sorcerer_common.model
    art/race_chaos/troops_wargear/heads/chaos_sorcerer_common/chaos_sorcerer_head_common.model
    art/race_chaos/troops_wargear/weapons_melee/sorceror_staff_last_stand/sorceror_staff_last_stand.model
    */

    //art/race_chaos/troops_wargear/armour/heretic_common/heretic.model

    //art/race_eldar/structures/warp_generator/warp_generator.model
    //art/race_necron/troops_wargear/armour/necron_overlord_common/necron_overlord_common.model
    //art/race_ork/structures/orky_turret/orky_turret.model
    //art/race_ork/troops_wargear/heads/boy/boy.model
    FILE *file = fopen("NameModels.txt", "r");
    if(file == NULL)
    {
        printf("Не удалось открыть файл конфигурации импорта\n");
    }
    char *fileBuffer;size_t fileSize;
    fseek(file, 0, 2);
    fileSize = ftell(file);
    fseek(file, 0, 0);
    fileBuffer = new char[fileSize];
    fread(fileBuffer, 1, fileSize, file);
    fclose(file);
    Modelset_import(fileBuffer, fileSize, &modelset);
    delete[] fileBuffer;
    Vulkan_struct vulkan_program;
    vulkan_program.init(window, &modelset);
    short rotate = 0;
    uint8_t currentFrame = 0;
    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            rotate+=10;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            rotate-=10;
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            currentFrame+=1;
            if(currentFrame == modelset.models[0].animation.numFrames)
                currentFrame=0;
        }
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            currentFrame-=1;
            if(currentFrame > modelset.models[0].animation.numFrames)
                currentFrame=modelset.models[0].animation.numFrames-1;
        }
        glfwPollEvents();
        ModelMatrix model{};
        model.model.matrix_rotate(180,'x');
        mat4 matrix;
        matrix.matrix_rotate(rotate, 'y');
        model.model = matrix_multiplication(matrix, model.model);
        //matrix.matrix_scale(0.25, 0.25, 0.25);
        //model.model = matrix_multiplication(matrix, model.model);
        void* data;
        vkMapMemory(vulkan_program.device, vulkan_program.framesVkVariables[0].uniformBufferMemory, 0, sizeof(model), 0, &data);
        memcpy(data, &model, sizeof(model));
        vkUnmapMemory(vulkan_program.device, vulkan_program.framesVkVariables[0].uniformBufferMemory);
        //теперь для VPmatrices
        VPmatrices vp{};
        vp.view.matrix_translate(0,0,8);
        vp.proj.matrix_perspective(45.0f, (float)vulkan_program.swapChainExtent.width /(float)vulkan_program.swapChainExtent.height, 0.1f, 10.0f);
        vp.viewProj = vp.proj * vp.view;
        //void* _data;
        vkMapMemory(vulkan_program.device, vulkan_program.framesVkVariables[0].VPmatricesUniformBufferMemory, 0, sizeof(vp), 0, &data);
        memcpy(data, &vp, sizeof(vp));
        vkUnmapMemory(vulkan_program.device, vulkan_program.framesVkVariables[0].VPmatricesUniformBufferMemory);
        //анимация
        mat4 bones[256];
        for(size_t i = 0; i < 256; i++)
            bones[i].matrix_identity();
        for(long i = 0; i < modelset.models[0].numBones-1; i++)//modelset.models[0].numBones-1
        {
            //bones[i+1] = modelset.models[0].animation.bones[i].worldMatrix * modelset.models[0].bones[i+1].invWorldMatrix;
            //bones[i+1] = matrix_multiplication(modelset.models[0].animation.frames[0].worldBones[i], modelset.models[0].bones[i+1].invWorldMatrix);
            //bones[i+1] = bones[i+1] * modelset.models[0].animation.frames[0].relBones[i];
        }
        //bones[5].matrix_translate(0, 2, 0);
        vkMapMemory(vulkan_program.device, modelset.models[0].uniformBonesBufferMemory, 0, sizeof(mat4)*256, 0, &data);
        memcpy(data, bones, sizeof(mat4)*256);
        vkUnmapMemory(vulkan_program.device, modelset.models[0].uniformBonesBufferMemory);
        vulkan_program.drawFrame(window, &modelset);
        //usleep(1000);
        //currentFrame+=1;
        //if(currentFrame == modelset.models[0].animation.numFrameBones)
            //currentFrame=0;
    }
    vkDeviceWaitIdle(vulkan_program.device);
    vulkan_program.cleanup(&modelset);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
// всякий раз, когда изменяются размеры окна (пользователем или операционной системой), вызывается данная callback-функция
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    framebufferResized = true;
}
