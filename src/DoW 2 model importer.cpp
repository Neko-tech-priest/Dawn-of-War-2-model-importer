#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <ctime>
#include "algebra.h"
#include "model.h"
struct optionsStruct
{
    bool smoothing = false;
    bool merge = false;
    bool groupMeshes = false;
    bool weldVertices = false;
    bool importBiped = false;
};
struct VertexElement
{
    uint32_t type;
    uint32_t version;
    uint32_t dataType = 0;
};
struct AnimationStruct
{
    struct Bone
    {
        char *name = 0;
        mat4 pose;
        ~Bone()
        {
            delete[] name;
        }
    };
    struct Frame
    {
        mat4 matrix;
    };
    char *name = 0;
    Bone *bones = 0;
    uint32_t *tracks = 0;
    Frame *frames = 0;
    uint32_t animationNameLength = 0;
    uint32_t numBones = 0;
    uint32_t numTracks = 0;
    uint32_t numFrames = 0;
    ~AnimationStruct()
    {
        delete[] name;
        delete[] bones;
        delete[] tracks;
        delete[] frames;
    }
};
void loader_dds(char *name_texture, VkFormat *format,uint32_t *Width, uint32_t *Height, uint32_t *ImageSize, uint8_t **texture_buffer);
void decompressor_DXT(char *name_texture, uint32_t *Width, uint32_t *Height, uint32_t *ImageSize, uint8_t **texture_buffer);
void DoW2_model_import(Model *model, char *modelName, size_t modelNameLength)
{
    uint8_t *file_buffer;uint64_t index_file_buffer = 0;uint64_t file_size = 0;
    //uint64_t additional_index = 0;
    // файл модели
    FILE *file = fopen(modelName, "rb");
    if(file == NULL)
    {
        printf("Не удалось открыть файл модели\n");
        return;
    }
    fseek(file, 0, 2);
    file_size = ftell(file);
    fseek(file, 0, 0);
    file_buffer = new uint8_t[file_size];
    fread(file_buffer, 1, file_size, file);
    fclose(file);
    //skip file header
    index_file_buffer+=36;
    //read chunks
    uint64_t chunk_position = 0;
    //uint32_t FOLD = 0x444c4f46;
    const uint32_t MTRL = 0x4c52544d;
    const uint32_t  VAR = 0x52415600;
    const uint32_t SKEL = 0x4c454b53;
    const uint32_t MRKS = 0x534b524d;
    const uint32_t MESH = 0x4853454d;
    /*uint32_t DATA = 0x41544144;
    uint32_t INFO = 0x4f464e49;
    uint32_t MGRP = 0x5052474d;
    uint32_t SKEL = 0x4c454b53;
    uint32_t BONE = 0x454e4f42;
    uint32_t MSBP = 0x5042534d;
    uint32_t DTBP = 0x50425444;
    uint32_t BVOL = 0x4c4f5642;
    uint32_t NODE = 0x45444f4e;*/
    //FOLDMODL
    uint64_t size;
    uint64_t len;
    len = *(uint32_t*)(file_buffer+index_file_buffer+16);
    index_file_buffer += 28;
    model->modelName = new char[len];
    model->modelNameLength = len;
    for(size_t i = 0; i < len; i++)
    {
        model->modelName[i] = file_buffer[index_file_buffer+i];
        printf("%c", model->modelName[i]);
    }
    printf("\n");
    index_file_buffer += len;
    //сохранение адресов для произвольного порядка чтения
    uint64_t adress_FOLDMTRL = 0;
    uint64_t number_FOLDMTRL = 0;
    uint64_t adress_FOLDSKEL = 0;
    uint64_t adress_FOLDMRKS = 0;
    uint64_t adress_FOLDMESH = 0;
    uint64_t number_FOLDMESH = 0;
    chunk_position = index_file_buffer;
    while(chunk_position < file_size)
    {
        switch(*(uint32_t*)(file_buffer+chunk_position+4))
        {
        case MTRL:
            if(adress_FOLDMTRL == 0)
                adress_FOLDMTRL = chunk_position;
            number_FOLDMTRL+=1;
            break;
        case SKEL:
            if(adress_FOLDSKEL == 0)
                adress_FOLDSKEL = chunk_position;
            break;
        case MRKS:
            if(adress_FOLDMRKS == 0)
                adress_FOLDMRKS = chunk_position;
            break;
        case MESH:
            if(adress_FOLDMESH == 0)
                adress_FOLDMESH = chunk_position;
            number_FOLDMESH+=1;
            break;
        }
        chunk_position += 28+*(uint32_t*)(file_buffer+chunk_position+16)+*(uint32_t*)(file_buffer+chunk_position+12);
    }
    if(adress_FOLDMTRL)
    {
        printf("%s%ld\n", "numMaterials: ", number_FOLDMTRL);
        model->numMaterials = number_FOLDMTRL;
        model->materials = new Model::Material[number_FOLDMTRL];
        Model::Material *currentMaterial;
        for(uint64_t index_FOLDMTRL = 0; index_FOLDMTRL < number_FOLDMTRL; index_FOLDMTRL++)
        {
            //FOLDMTRL
            size = *(uint32_t*)(file_buffer+index_file_buffer+12);
            len = *(uint32_t*)(file_buffer+index_file_buffer+16);
            index_file_buffer += 28;
            currentMaterial = &(model->materials[index_FOLDMTRL]);
            currentMaterial->materialName = new char[len];
            currentMaterial->materialNameLength = len;
            for(size_t i = 0; i < len; i++)
            {
                currentMaterial->materialName[i] = file_buffer[index_file_buffer+i];
                printf("%c", currentMaterial->materialName[i]);
            }
            printf("\n");
            index_file_buffer += len;
            //DATAINFO
            size = *(uint32_t*)(file_buffer+index_file_buffer+12);
            len = *(uint32_t*)(file_buffer+index_file_buffer+16);
            index_file_buffer += 28 + len + size;
            //DATAVAR
            chunk_position = index_file_buffer;
            const char *textureTypes[] = {"diffuseTex",};//"normalMap", "teamTex", "specularTex"
            while(*(uint32_t*)(file_buffer+index_file_buffer+4) == VAR)
            {
                size = *(uint32_t*)(file_buffer+index_file_buffer+12);
                len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                index_file_buffer += 28 + len;
                uint32_t varNameLength = *(uint32_t*)(file_buffer+index_file_buffer);
                index_file_buffer +=4;
                char *varName = new char[varNameLength+1];//под нуль-терминал
                for(size_t i = 0; i < varNameLength; i++)
                    varName[i] = file_buffer[index_file_buffer+i];
                varName[varNameLength] = 0;
                index_file_buffer+=varNameLength;
                uint32_t varType = *(uint32_t*)(file_buffer+index_file_buffer);
                index_file_buffer+=4;
                uint32_t varSize = *(uint32_t*)(file_buffer+index_file_buffer);
                index_file_buffer+=4;
                switch(varType)
                {
                case 0:
                    index_file_buffer+=varSize;
                    break;
                case 1:
                    index_file_buffer+=varSize;
                    break;
                case 3:
                    index_file_buffer+=varSize;
                    break;
                case 4:
                    index_file_buffer+=varSize;
                    break;
                case 5:
                    index_file_buffer+=varSize;
                    break;
                case 8:
                    index_file_buffer+=varSize;
                    break;
                case 9:
                {
                    char *textureName = new char[varSize+4];//.dds
                    for(uint64_t i = 0; i < varSize; i++)
                        textureName[i] = file_buffer[index_file_buffer+i];
                    index_file_buffer+=varSize;
                    //пока что учет только текстур из каталога расы(с '_' в имени)
                    if(textureName[varSize-5] == '_')
                    {
                        Model::Material::Texture *currentTexture;
                        for(size_t i = 0; i < 1; i++)
                        {
                            if(strcmp(textureTypes[i], varName) == 0)
                            {
                                //переворот слешей
                                for(size_t j = 0; j < varSize; j++)
                                {
                                    if(textureName[j] == 0x5c)
                                        textureName[j] = '/';
                                }
                                textureName[varSize-1] = '.';
                                textureName[varSize+0] = 'd';
                                textureName[varSize+1] = 'd';
                                textureName[varSize+2] = 's';
                                textureName[varSize+3] = 0;
                                currentTexture = &(currentMaterial->textures[i]);
                                currentTexture->textureName = textureName;
                                currentTexture->textureNameLength = varSize+4;
                                printf("%s\n", currentTexture->textureName);
                                loader_dds(currentTexture->textureName, &(currentTexture->texFormat),&(currentTexture->texWidth), &(currentTexture->texHeight), &(currentTexture->texturePixelsSize), &(currentTexture->texturePixels));
                            }
                        }
                    }
                    else
                    {
                        delete []textureName;
                        continue;
                    }
                    break;
                }
                case 10:
                    index_file_buffer+=varSize;
                    break;
                }
                delete []varName;
            }
        }
    }
    else
    {
        printf("FOLDMTRL не найден\n");
    }
    if(adress_FOLDSKEL)
    {
        index_file_buffer = adress_FOLDSKEL;
        //FOLDSKEL
        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
        index_file_buffer += 28 + len;
        //DATAINFO
        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
        index_file_buffer += 28 + len;
        uint32_t numBones = *(uint32_t*)(file_buffer+index_file_buffer);
        index_file_buffer+=4;
        if(numBones == 0)
        {
            printf("No bones found\n");
        }
        model->bones = new Model::Bone[numBones];
        model->numBones = numBones;
        printf("%s%d\n", "numBones: ", numBones);
        printf("%lx\n", index_file_buffer);
        for(long index_bones = 0; index_bones < numBones; index_bones++)
        {
            //DATABONE
            len = *(uint32_t*)(file_buffer+index_file_buffer+16);
            index_file_buffer += 28;
            Model::Bone *currentBone = &(model->bones[index_bones]);
            currentBone->boneName = new char[len-1];
            currentBone->boneNameLength = len-1;
            for(size_t i = 0; i < len-1; i++)
            {
                currentBone->boneName[i] = file_buffer[index_file_buffer+i];
            }
            index_file_buffer += len;
            currentBone->parent = *(uint32_t*)(file_buffer+index_file_buffer);index_file_buffer+=4;
            index_file_buffer+=4;//unknown
            for(size_t i = 0; i < 4; i++)
            {
                for(size_t j = 0; j < 3; j++)
                {
                    currentBone->relMatrix[4*i+j] = *(float*)(file_buffer+index_file_buffer);
                    index_file_buffer+=4;
                }
                currentBone->relMatrix[3] = 0;
                currentBone->relMatrix[7] = 0;
                currentBone->relMatrix[11] = 0;
                currentBone->relMatrix[15] = 1;
            }
            if(currentBone->parent == 0xffffffff)
            {
                currentBone->worldMatrix.matrix_identity();
                currentBone->invWorldMatrix.matrix_identity();
            }
            else
            {
                //currentBone->relMatrix * model->bones[currentBone->parent].worldMatrix
                //model->bones[currentBone->parent].worldMatrix * currentBone->relMatrix
                currentBone->worldMatrix = model->bones[currentBone->parent].worldMatrix * currentBone->relMatrix;
                //currentBone->worldMatrix = gemm_v0(model->bones[currentBone->parent].worldMatrix, currentBone->relMatrix);
                currentBone->invWorldMatrix = Invert2(currentBone->worldMatrix);
            }
            index_file_buffer+=8;//unknown
        }
        //printf("%lx\n", index_file_buffer);
    }
    else
    {
        printf("FOLDSKEL не найден\n");
    }
    if(adress_FOLDMESH)
    {
        index_file_buffer = adress_FOLDMESH;
        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
        index_file_buffer += 28 + len;
        //FOLDMGRP
        uint32_t size = *(uint32_t*)(file_buffer+index_file_buffer+12);
        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
        index_file_buffer += 28 + len;
        uint32_t offset = index_file_buffer;
        //for i=1 to mgrp.children.count do
        while(index_file_buffer < offset + size)
        {
            if(*(uint32_t*)(file_buffer+index_file_buffer+4) == MESH)
            {
                len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                index_file_buffer += 28 + len;
                //FOLDIMDG
                uint32_t size = *(uint32_t*)(file_buffer+index_file_buffer+12);
                len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                index_file_buffer += 28 + len;
                uint32_t offset = index_file_buffer;
                //подсчет количества lod0 мешей
                while(index_file_buffer < offset + size)
                {
                    if(*(uint32_t*)(file_buffer+index_file_buffer+4) == MESH)
                    {
                        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                        index_file_buffer += 28 + len;
                        //FOLDIMOD
                        uint32_t size = *(uint32_t*)(file_buffer+index_file_buffer+12);
                        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                        index_file_buffer += 28 + len;
                        uint32_t offset = index_file_buffer;
                        //DATADATA
                        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                        index_file_buffer += 28 + len;
                        uint32_t lodlevel = *(uint32_t*)(file_buffer+index_file_buffer);
                        index_file_buffer += 4;
                        while(index_file_buffer < offset + size)
                        {
                            if(*(uint32_t*)(file_buffer+index_file_buffer+4) == MESH && lodlevel == 0)
                                model->numMeshes+=1;
                            index_file_buffer = index_file_buffer+28+*(uint32_t*)(file_buffer+index_file_buffer+16)+*(uint32_t*)(file_buffer+index_file_buffer+12);
                        }
                    }
                    else
                    {
                        index_file_buffer = index_file_buffer+28+*(uint32_t*)(file_buffer+index_file_buffer+16)+*(uint32_t*)(file_buffer+index_file_buffer+12);
                    }
                }
                model->meshes = new Model::Mesh[model->numMeshes];
                printf("%s%d\n", "numMeshes: ", model->numMeshes);
                index_file_buffer = offset;
                uint8_t index_mesh = 0;
                //for j=1 to imdg.children.count do
                while(index_file_buffer < offset + size)
                {
                    if(*(uint32_t*)(file_buffer+index_file_buffer+4) == MESH)
                    {
                        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                        index_file_buffer += 28 + len;
                        //FOLDIMOD
                        uint32_t size = *(uint32_t*)(file_buffer+index_file_buffer+12);
                        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                        index_file_buffer += 28 + len;
                        uint32_t offset = index_file_buffer;
                        //DATADATA
                        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                        index_file_buffer += 28 + len;
                        uint32_t lodlevel = *(uint32_t*)(file_buffer+index_file_buffer);
                        index_file_buffer += 4;
                        //загрузка только наиболее полигональных моделей
                        //for k=2 to imod.children.count do
                        while(index_file_buffer < offset + size)
                        {
                            if(*(uint32_t*)(file_buffer+index_file_buffer+4) == MESH && lodlevel == 0)
                            {
                                len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                                index_file_buffer += 28 + len;
                                //FOLDTRIM
                                len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                                index_file_buffer += 28 + len;
                                //DATADATA
                                len = *(uint32_t*)(file_buffer+index_file_buffer+16);
                                index_file_buffer += 28 + len;
                                //импорт меша
                                //Read vertex elements
                                uint32_t numVertexElements = *(uint32_t*)(file_buffer+index_file_buffer);
                                index_file_buffer += 4;
                                VertexElement vertexElements[10];
                                for(uint64_t i = 0; i < numVertexElements; i++)
                                {
                                    uint32_t data = *(uint32_t*)(file_buffer+index_file_buffer);
                                    vertexElements[data].type = *(uint32_t*)(file_buffer+index_file_buffer);
                                    vertexElements[data].version = *(uint32_t*)(file_buffer+index_file_buffer+4);
                                    vertexElements[data].dataType = *(uint32_t*)(file_buffer+index_file_buffer+8);
                                    index_file_buffer+=12;
                                }
                                //Read vertices
                                uint32_t numVertices = *(uint32_t*)(file_buffer+index_file_buffer);
                                index_file_buffer += 4;
                                //uint32_t vertSize = *(uint32_t*)(file_buffer+index_file_buffer);
                                index_file_buffer += 4;
                                Model::Mesh *currentMesh = &(model->meshes[index_mesh]);
                                currentMesh->vertices = new Model::Mesh::Vertex[numVertices];
                                currentMesh->numVertices = numVertices;
                                Model::Mesh::Vertex *vertices = currentMesh->vertices;
                                printf("%s%d\n", "normals: ", vertexElements[3].dataType);
                                printf("%s%d\n", "binormals: ", vertexElements[4].dataType);
                                printf("%s%d\n", "tangents: ", vertexElements[5].dataType);
                                printf("%lx\n", index_file_buffer);
                                uint32_t blendWeightsError = 0;
                                for(size_t i = 0; i < numVertices; i++)
                                {
                                    if(vertexElements[0].dataType != 0)
                                    {
                                        vertices[i].position[0] = *(float*)(file_buffer+index_file_buffer+0);
                                        vertices[i].position[1] = *(float*)(file_buffer+index_file_buffer+4);
                                        vertices[i].position[2] = *(float*)(file_buffer+index_file_buffer+8);
                                        index_file_buffer+=12;
                                    }
                                    //indices
                                    if(vertexElements[1].dataType != 0)
                                    {
                                        vertices[i].blendIndices[0] = file_buffer[index_file_buffer+0];
                                        vertices[i].blendIndices[1] = file_buffer[index_file_buffer+1];
                                        vertices[i].blendIndices[2] = file_buffer[index_file_buffer+2];
                                        vertices[i].blendIndices[3] = file_buffer[index_file_buffer+3];
                                        index_file_buffer+=4;
                                    }
                                    /*
                                    function BytesToWeights bytes =
                                    (
                                        local weights = #()
                                        weights[1] = ceil (bytes[3] / 255.0 * 1000 - 0.5) / 1000
                                        weights[2] = ceil (bytes[2] / 255.0 * 1000 - 0.5) / 1000
                                        weights[3] = ceil (bytes[1] / 255.0 * 1000 - 0.5) / 1000
                                        weights[4] = ceil (bytes[4] / 255.0 * 1000 - 0.5) / 1000
                                        local sum = weights[1] + weights[2] + weights[3] + weights[4]
                                        local delta = sum - 1.0
                                        weights[1] -= delta
                                        return weights
                                    ),
                                    */
                                    //weights
                                    if(vertexElements[2].dataType != 0)
                                    {
                                        vertices[i].blendWeights[0] = file_buffer[index_file_buffer+2]/255;
                                        vertices[i].blendWeights[1] = file_buffer[index_file_buffer+1]/255;
                                        vertices[i].blendWeights[2] = file_buffer[index_file_buffer+0]/255;
                                        vertices[i].blendWeights[3] = file_buffer[index_file_buffer+3]/255;
                                        //vertices[i].blendWeights.normalize();
                                        if(vertices[i].blendWeights[0]+
                                           vertices[i].blendWeights[1]+
                                           vertices[i].blendWeights[2]+
                                           vertices[i].blendWeights[3] != 1.0)
                                            blendWeightsError+=1;
                                        index_file_buffer+=4;
                                    }
                                    //нормали
                                    if(vertexElements[3].dataType != 0)
                                    {
                                        switch(vertexElements[3].dataType)
                                        {
                                        case 2:
                                        {
                                            uint8_t bytes[4];
                                            bytes[0] = file_buffer[index_file_buffer+0];
                                            bytes[1] = file_buffer[index_file_buffer+1];
                                            bytes[2] = file_buffer[index_file_buffer+2];
                                            bytes[3] = file_buffer[index_file_buffer+3];
                                            for(size_t j = 0; j < 2; j++)
                                            {
                                                uint8_t sign = (bytes[j]>>7);//если 1, то число положительное
                                                int32_t value = (~(sign)<<7) + ((bytes[j]<<1)>>1);
                                                if(!sign)
                                                {
                                                    value = ((~value)<<24)>>24;
                                                    value *= -1;
                                                }
                                                vertices[i].normal[j] = (float)value / 127.0;
                                            }
                                            index_file_buffer+=4;
                                            break;
                                        }
                                        case 4:
                                        {
                                            vertices[i].normal[0] = *(float*)(file_buffer+index_file_buffer+0);
                                            vertices[i].normal[1] = *(float*)(file_buffer+index_file_buffer+4);
                                            vertices[i].normal[1] = *(float*)(file_buffer+index_file_buffer+8);
                                            index_file_buffer+=12;
                                            break;
                                        }
                                        }
                                    }
                                    //бинормали
                                    if(vertexElements[4].dataType != 0)
                                    {
                                        switch(vertexElements[4].dataType)
                                        {
                                        case 2:
                                        {
                                            uint8_t bytes[4];
                                            bytes[0] = file_buffer[index_file_buffer+0];
                                            bytes[1] = file_buffer[index_file_buffer+1];
                                            bytes[2] = file_buffer[index_file_buffer+2];
                                            bytes[3] = file_buffer[index_file_buffer+3];
                                            for(size_t j = 0; j < 2; j++)
                                            {
                                                uint8_t sign = (bytes[j]>>7);//если 1, то число положительное
                                                int32_t value = (~(sign)<<7) + ((bytes[j]<<1)>>1);
                                                if(!sign)
                                                {
                                                    value = ((~value)<<24)>>24;
                                                    value *= -1;
                                                }
                                                vertices[i].binormal[j] = (float)value / 127.0;
                                            }
                                            index_file_buffer+=4;
                                            break;
                                        }
                                        case 4:
                                        {
                                            vertices[i].binormal[0] = *(float*)(file_buffer+index_file_buffer+0);
                                            vertices[i].binormal[1] = *(float*)(file_buffer+index_file_buffer+4);
                                            vertices[i].binormal[1] = *(float*)(file_buffer+index_file_buffer+8);
                                            index_file_buffer+=12;
                                            break;
                                        }
                                        }
                                    }
                                    //тангент
                                    if(vertexElements[5].dataType != 0)
                                    {
                                        switch(vertexElements[5].dataType)
                                        {
                                        case 2:
                                        {
                                            uint8_t bytes[4];
                                            bytes[0] = file_buffer[index_file_buffer+0];
                                            bytes[1] = file_buffer[index_file_buffer+1];
                                            bytes[2] = file_buffer[index_file_buffer+2];
                                            bytes[3] = file_buffer[index_file_buffer+3];
                                            for(size_t j = 0; j < 2; j++)
                                            {
                                                uint8_t sign = (bytes[j]>>7);//если 1, то число положительное
                                                int32_t value = (~(sign)<<7) + ((bytes[j]<<1)>>1);
                                                if(!sign)
                                                {
                                                    value = ((~value)<<24)>>24;
                                                    value *= -1;
                                                }
                                                vertices[i].tangent[j] = (float)value / 127.0;
                                            }
                                            index_file_buffer+=4;
                                            break;
                                        }
                                        case 4:
                                        {
                                            vertices[i].tangent[0] = *(float*)(file_buffer+index_file_buffer+0);
                                            vertices[i].tangent[1] = *(float*)(file_buffer+index_file_buffer+4);
                                            vertices[i].tangent[1] = *(float*)(file_buffer+index_file_buffer+8);
                                            index_file_buffer+=12;
                                            break;
                                        }
                                        }
                                    }
                                    //цвет
                                    if(vertexElements[6].dataType != 0)
                                    {
                                        index_file_buffer+=4;
                                    }
                                    vertices[i].uv1[0] = *(float*)(file_buffer+index_file_buffer);
                                    index_file_buffer+=4;
                                    vertices[i].uv1[1] = *(float*)(file_buffer+index_file_buffer);
                                    index_file_buffer+=4;
                                }
                                printf("%s%d\n", "blendWeightsError: ", blendWeightsError);
                                //read faces
                                index_file_buffer+=8;
                                uint32_t vertPerFace = *(uint32_t*)(file_buffer+index_file_buffer);
                                if(vertPerFace != 3)
                                    exit(0);
                                index_file_buffer+=4;
                                uint32_t numFaces = *(uint32_t*)(file_buffer+index_file_buffer);
                                index_file_buffer+=4;
                                currentMesh->indices = new uint16_t[numFaces];
                                currentMesh->numIndices = numFaces;

                                printf("vertices: ");
                                printf("%d\n", numVertices);
                                printf("indices: ");
                                printf("%d\n", numFaces);
                                numFaces /= 3;
                                uint16_t *indices = currentMesh->indices;
                                for(size_t i = 0; i < numFaces; i++)
                                {
                                    indices[3*i+0] = *(uint32_t*)(file_buffer+index_file_buffer);
                                    index_file_buffer+=2;
                                    indices[3*i+1] = *(uint32_t*)(file_buffer+index_file_buffer);
                                    index_file_buffer+=2;
                                    indices[3*i+2] = *(uint32_t*)(file_buffer+index_file_buffer);
                                    index_file_buffer+=2;
                                }
                                //чтение имени материала, к которому привязан меш
                                uint32_t materialNameLength = *(uint32_t*)(file_buffer+index_file_buffer);
                                index_file_buffer+=4;
                                currentMesh->materialName = new char[materialNameLength];
                                currentMesh->materialNameLength = materialNameLength;
                                for(size_t i = 0; i < materialNameLength; i++)
                                {
                                    currentMesh->materialName[i] = file_buffer[index_file_buffer+i];
                                    printf("%c", currentMesh->materialName[i]);
                                }
                                printf("\n");
                                //currentMesh->materialName[materialNameLength] = 0;
                                index_file_buffer+=materialNameLength;
                                // Read skin
                                uint32_t numSkinBones = *(uint32_t*)(file_buffer+index_file_buffer);
                                index_file_buffer+=4;
                                printf("%s%d\n", "numSkinBones: ", numSkinBones);
                                currentMesh->numBones = numSkinBones;
                                currentMesh->bones = new Model::Mesh::Bone[currentMesh->numBones];
                                Model::Mesh::Bone *currentBone;
                                for(size_t indexBone = 0; indexBone < currentMesh->numBones; indexBone++)
                                {
                                    currentBone = &(currentMesh->bones[indexBone]);
                                    for(size_t i = 0; i < 4; i++)
                                    {
                                        for(size_t j = 0; j < 3; j++)
                                        {
                                            currentBone->worldMatrix[4*i+j] = *(float*)(file_buffer+index_file_buffer);
                                            index_file_buffer+=4;
                                        }
                                        currentBone->worldMatrix[3] = 0;
                                        currentBone->worldMatrix[7] = 0;
                                        currentBone->worldMatrix[11] = 0;
                                        currentBone->worldMatrix[15] = 1;
                                    }
                                    for(size_t k = 0; k < 4; k++)
                                    {
                                        for(size_t j = 0; j < 3; j++)
                                        {
                                            currentBone->invWorldMatrix[4*k+j] = *(float*)(file_buffer+index_file_buffer);
                                            index_file_buffer+=4;
                                        }
                                        currentBone->invWorldMatrix[3] = 0;
                                        currentBone->invWorldMatrix[7] = 0;
                                        currentBone->invWorldMatrix[11] = 0;
                                        currentBone->invWorldMatrix[15] = 1;
                                    }
                                    //read bone name
                                    uint32_t size_skinBoneName = *(uint32_t*)(file_buffer+index_file_buffer);
                                    index_file_buffer+=4;
                                    currentBone->boneName = new char[size_skinBoneName];
                                    currentBone->boneNameLength = size_skinBoneName;
                                    for(size_t j = 0; j < size_skinBoneName; j++)
                                        currentBone->boneName[j] = file_buffer[index_file_buffer+j];
                                    index_file_buffer+=size_skinBoneName;
                                }
                                // Read unknowns
                                index_file_buffer+=8;
                                //сортировка индексов костей у вершин в соответствии с костями модели
                                uint32_t *blendIndices = new uint32_t[numSkinBones];
                                for(size_t indexMeshBone = 0; indexMeshBone < currentMesh->numBones; indexMeshBone++)
                                {
                                    //bool findBone = false;
                                    for(size_t indexModelBone = 0; indexModelBone < model->numBones; indexModelBone++)
                                    {
                                        //поиск одинаковых имен костей
                                        if(currentMesh->bones[indexMeshBone].boneNameLength != model->bones[indexModelBone].boneNameLength)
                                        {
                                            continue;
                                        }
                                        if(memcmp(currentMesh->bones[indexMeshBone].boneName, model->bones[indexModelBone].boneName, currentMesh->bones[indexMeshBone].boneNameLength) != 0)
                                        {
                                            continue;
                                        }
                                        blendIndices[indexMeshBone] = indexModelBone;
                                        break;
                                    }
                                }
                                /*printf("%d ", blendIndices[0]);
                                printf("%d ", blendIndices[1]);
                                printf("%d ", blendIndices[2]);
                                printf("%d ", blendIndices[3]);
                                printf("%d ", blendIndices[4]);
                                printf("%d ", blendIndices[5]);
                                printf("\n");*/
                                printf("%d\n", vertices[0].blendIndices[0]);
                                for(size_t i = 0; i < numVertices; i++)
                                {
                                    for(size_t j = 0; j < 4; j++)
                                    {
                                        vertices[i].blendIndices[j] = blendIndices[vertices[i].blendIndices[j]];
                                    }
                                }
                                printf("%d\n", vertices[0].blendIndices[0]);
                                /*for(size_t indexMeshBone = 0; indexMeshBone < currentMesh->numBones; indexMeshBone++)
                                {
                                    currentMesh->bones[indexMeshBone].boneName = model->bones[indexMeshBone].boneName;
                                    currentMesh->bones[indexMeshBone].boneName = new char[model->bones[indexMeshBone].boneNameLength];
                                    memcpy(currentMesh->bones[indexMeshBone].boneName, model->bones[indexMeshBone].boneName, model->bones[indexMeshBone].boneNameLength);
                                    currentMesh->bones[indexMeshBone].boneNameLength = model->bones[indexMeshBone].boneNameLength;
                                }*/
                                delete[] blendIndices;
                                index_mesh+=1;
                            }
                            else
                            {
                                //break;
                                index_file_buffer = index_file_buffer+28+*(uint32_t*)(file_buffer+index_file_buffer+16)+*(uint32_t*)(file_buffer+index_file_buffer+12);
                            }
                        }
                    }
                    else
                    {
                        //break;
                        index_file_buffer = index_file_buffer+28+*(uint32_t*)(file_buffer+index_file_buffer+16)+*(uint32_t*)(file_buffer+index_file_buffer+12);
                    }
                }
            }
            else
            {
                break;
                index_file_buffer = index_file_buffer+28+*(uint32_t*)(file_buffer+index_file_buffer+16)+*(uint32_t*)(file_buffer+index_file_buffer+12);
            }
            break;
        }
    }
    else
    {
        printf("FOLDMESH не найден\n");
    }
    if(adress_FOLDMRKS)
    {
        index_file_buffer = adress_FOLDMRKS;
        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
        index_file_buffer += 28 + len;
        uint32_t numMarkers = *(uint32_t*)(file_buffer+index_file_buffer);
        index_file_buffer+=4;
        printf("%s%d\n", "numMarkers: ", numMarkers);
        model->markers = new Model::Marker[numMarkers];
        model->numMarkers = numMarkers;
        if(numMarkers == 0)
        {
            printf("No markers found\n");
        }
        Model::Marker *currentMarker;
        for(size_t index_marker = 0; index_marker < numMarkers; index_marker++)
        {
            currentMarker = &(model->markers[index_marker]);
            uint32_t nameLength = *(uint32_t*)(file_buffer+index_file_buffer);
            index_file_buffer+=4;
            currentMarker->markerName = new char[nameLength];
            currentMarker->markerNameLength = nameLength;
            for(size_t i = 0; i < nameLength; i++)
            {
                currentMarker->markerName[i] = file_buffer[index_file_buffer+i];
            }
            index_file_buffer+=nameLength;
            currentMarker->parentLength = *(uint32_t*)(file_buffer+index_file_buffer);
            index_file_buffer+=4;
            currentMarker->parent = new char[currentMarker->parentLength];
            for(size_t i = 0; i < currentMarker->parentLength; i++)
            {
                currentMarker->parent[i] = file_buffer[index_file_buffer+i];
            }
            index_file_buffer+=currentMarker->parentLength;
            for(size_t k = 0; k < 4; k++)
            {
                for(size_t j = 0; j < 3; j++)
                {
                    currentMarker->matrix[4*k+j] = *(float*)(file_buffer+index_file_buffer);
                    index_file_buffer+=4;
                }
                currentMarker->matrix[3] = 0;
                currentMarker->matrix[7] = 0;
                currentMarker->matrix[11] = 0;
                currentMarker->matrix[15] = 1;
            }
            uint32_t numParams = *(uint32_t*)(file_buffer+index_file_buffer);
            index_file_buffer+=4;
            currentMarker->numParams = numParams;
            currentMarker->params = new Model::Marker::Param[numParams];
            for(size_t index_param = 0; index_param < numParams; index_param++)
            {
                currentMarker->params[index_param].keyLength = *(uint32_t*)(file_buffer+index_file_buffer);
                index_file_buffer+=4;
                currentMarker->params[index_param].key = new char[currentMarker->params[index_param].keyLength];
                for(size_t i = 0; i < currentMarker->params[index_param].keyLength; i++)
                {
                    currentMarker->params[index_param].key[i] = file_buffer[index_file_buffer+i];
                }
                index_file_buffer+=currentMarker->params[index_param].keyLength;
                index_file_buffer+=4;//unknown
                currentMarker->params[index_param].valueLength = *(uint32_t*)(file_buffer+index_file_buffer);
                index_file_buffer+=4;
                currentMarker->params[index_param].value = new char[currentMarker->params[index_param].valueLength];
                for(size_t i = 0; i < currentMarker->params[index_param].valueLength; i++)
                {
                    currentMarker->params[index_param].value[i] = file_buffer[index_file_buffer+i];
                }
                index_file_buffer+=currentMarker->params[index_param].valueLength;
            }
            //break;
        }
    }
    else
    {
        printf("FOLDMRKS не найден\n");
    }
    /*const  size_t data = 10;
    printf("%s\n", model->meshes[0].bones[data].boneName);
    mat4 matrix;
    matrix = matrix_multiplication(model->meshes[0].bones[data].relMatrix, model->meshes[0].bones[data].invRelMatrix);
    for(size_t i = 0; i < 4; i++)
    {
        for(size_t j = 0; j < 4; j++)
        {
            printf("%f ", matrix[i*4+j]);
        }
        printf("\n");
    }
    printf("\n");
    for(size_t i = 0; i < 4; i++)
    {
        for(size_t j = 0; j < 4; j++)
        {
            printf("%f ", model->meshes[0].bones[data].worldMatrix[i*4+j]);
        }
        printf("\n");
    }
    printf("\n");
    for(size_t i = 0; i < 4; i++)
    {
        for(size_t j = 0; j < 4; j++)
        {
            printf("%f ", model->meshes[0].bones[data].invWorldMatrix[i*4+j]);
        }
        printf("\n");
    }
    printf("\n");*/
    delete []file_buffer;
    //анимации
    /*const char *animations_base = "animations/base";
    const char *idle_01 = "/idle_01.anim";
    char *animationFileName = new char[modelNameLength-model->modelNameLength-6+std::size("animations/base")+std::size("/idle_01.anim")-1];
    for(size_t i = 0; i < modelNameLength-model->modelNameLength-6; i++)
        animationFileName[i] = modelName[i];
    memcpy(animationFileName+modelNameLength-model->modelNameLength-6, animations_base, std::size("animations/base"));
    memcpy(animationFileName+modelNameLength-model->modelNameLength-6+std::size("animations/base")-1, idle_01, std::size("/idle_01.anim"));

    char *hkanimName = new char[modelNameLength+1];
    for(size_t i = 0; i < modelNameLength-6; i++)
        hkanimName[i] = modelName[i];
    hkanimName[modelNameLength-6] = 'h';
    hkanimName[modelNameLength-5] = 'k';
    hkanimName[modelNameLength-4] = 'a';
    hkanimName[modelNameLength-3] = 'n';
    hkanimName[modelNameLength-2] = 'i';
    hkanimName[modelNameLength-1] = 'm';
    hkanimName[modelNameLength-0] = 0;
    index_file_buffer = 0;
    file = fopen(hkanimName, "rb");
    if(file == NULL)
    {
        printf("Не удалось открыть файл анимаций\n");
        return;
    }
    fseek(file, 0, 2);
    file_size = ftell(file);
    fseek(file, 0, 0);
    file_buffer = new uint8_t[file_size];
    fread(file_buffer, 1, file_size, file);
    fclose(file);
    //skip file header
    index_file_buffer+=36;
    //const uint32_t HAAS = 0x53414148;
    const uint32_t HAWS = 0x53574148;
    size_t numberHAWS = 0;
    //FOLDHAAS
    size = *(uint32_t*)(file_buffer+index_file_buffer+12);
    len = *(uint32_t*)(file_buffer+index_file_buffer+16);
    index_file_buffer += 28 + len;
    chunk_position = index_file_buffer;
    while(chunk_position < file_size)
    {
        switch(*(uint32_t*)(file_buffer+chunk_position+4))
        {
        case HAWS:
            numberHAWS+=1;
            break;
        }
        for(size_t i = 0; i < 8; i++)
            printf("%c", file_buffer[chunk_position+i]);
        printf("\n");
        chunk_position += 28+*(uint32_t*)(file_buffer+chunk_position+16)+*(uint32_t*)(file_buffer+chunk_position+12);
    }
    for(size_t indexHAWS = 0; indexHAWS < numberHAWS; indexHAWS++)
    {
        //DATAHAWS
        size = *(uint32_t*)(file_buffer+index_file_buffer+12);
        len = *(uint32_t*)(file_buffer+index_file_buffer+16);
        index_file_buffer += 28 + len;
        uint32_t numAnimations = *(uint32_t*)(file_buffer+index_file_buffer);
        index_file_buffer+=4;
        for(size_t indexAnimation = 0; indexAnimation < numAnimations; indexAnimation++)
        {
            uint32_t animationFileNameLength = *(uint32_t*)(file_buffer+index_file_buffer);
            index_file_buffer+=4;
            index_file_buffer+=animationFileNameLength;
        }
        uint32_t sizeHavokBlock = *(uint32_t*)(file_buffer+index_file_buffer);
        index_file_buffer+=4;
        //index_file_buffer+=sizeHavokBlock;
        break;
    }*/
    //delete []file_buffer;
}
void Modelset_import(char *modelNames, size_t fileSize, Modelset *modelset)
{
    for(size_t i = 0; i < fileSize; i++)
        if(modelNames[i] == '\n')
        {
            modelset->numModels+=1;
            modelNames[i] = 0;
        }
    modelset->models = new Model[modelset->numModels];
    size_t indexStartName = 0;
    for(size_t i = 0; i < modelset->numModels; i++)
    {
        size_t nameLength = 0;
        while(modelNames[indexStartName+nameLength] != 0)
            nameLength+=1;
        nameLength+=1;
        DoW2_model_import(&(modelset->models[i]), modelNames+indexStartName, nameLength);
        indexStartName+=nameLength;
        modelset->numMeshes+=modelset->models[i].numMeshes;
    }
    printf("%s%d\n", "numModels: ", modelset->numModels);
    printf("%s%d\n", "numMeshes: ", modelset->numMeshes);
    //printf("Проверка на наличие костей, не привязанных ни к одной вершине...\n");
    for(size_t i = 0; i < modelset->models[0].numBones; i++)
    {
        for(size_t j = 0; j < modelset->models[0].bones[i].boneNameLength; j++)
            printf("%c", modelset->models[0].bones[i].boneName[j]);
        printf("\n");
    }
    printf("\n");
    //импорт анимации
    size_t file_size, index_file_buffer;
    uint8_t *file_buffer;
    FILE *file;
    //art/race_marine/vehicles/drop_pod/animations/default/drop_pod_onground.anim
    //art/race_ork/structures/orky_turret/animations/base/aim_horiz.anim
    //art/race_chaos/structures/chaos_generator/animations/base/spawn.anim
    //art/race_eldar/structures/warp_generator/animations/base/idle_01.anim
    const char *animationFileName = "art/race_marine/vehicles/drop_pod/animations/default/drop_pod_onground.hkx";
    printf("%s\n", animationFileName);
    index_file_buffer = 0;
    file = fopen(animationFileName, "rb");
    if(file == NULL)
    {
        printf("Не удалось открыть файл анимации\n");
        return;
    }
    fseek(file, 0, 2);
    file_size = ftell(file);
    fseek(file, 0, 0);
    file_buffer = new uint8_t[file_size];
    fread(file_buffer, 1, file_size, file);
    fclose(file);
    const uint32_t signature_57e0e057 = 0x57e0e057, signature_10c0c010 = 0x10c0c010;
    uint32_t userTag, version;
    uint8_t layout[4];
    //uint32_t layout;
    int32_t numSections, contentsSectionIndex, contentsSectionOffset, contentsClassNameSectionIndex, contentsClassNameSectionOffset;
    char contentsVersion[16];
    uint32_t flags;
    int16_t maxpredicate, predicateArraySizePlusPadding;
    if(*(uint32_t*)(file_buffer+0) != signature_57e0e057 || *(uint32_t*)(file_buffer+4) != signature_10c0c010)
    {
        printf("Неправильные сигнатуры!\n");
    }
    //index_file_buffer = 8;
    userTag = *(uint32_t*)(file_buffer+8);
    version = *(uint32_t*)(file_buffer+12);
    *(uint32_t*)layout = *(uint32_t*)(file_buffer+16);
    numSections = *(int32_t*)(file_buffer+20);
    contentsSectionIndex = *(int32_t*)(file_buffer+24);
    contentsSectionOffset = *(int32_t*)(file_buffer+28);
    contentsClassNameSectionIndex = *(int32_t*)(file_buffer+32);
    contentsClassNameSectionOffset = *(int32_t*)(file_buffer+36);
    memcpy(contentsVersion, file_buffer+40, 16);
    flags = *(uint32_t*)(file_buffer+56);
    maxpredicate = *(int16_t*)(file_buffer+60);
    predicateArraySizePlusPadding = *(int16_t*)(file_buffer+62);
    if(!layout[1]/*littleEndian*/)
    {
        printf("LittleEndian!\n");
        exit(-1);
    }
    if (maxpredicate != -1)
    {
        printf("maxpredicate != -1");
        exit(-1);
    }
    //GenerateToolset
    delete[] file_buffer;
    /*uint32_t signature_version = *(uint32_t*)(file_buffer+index_file_buffer);
    if(signature_version != 0x04415453)
        printf("Wrong file signature or version!\n");
    index_file_buffer+=4;
    //Read animation name
    modelset->models[0].animation.animationNameLength = *(uint32_t*)(file_buffer+index_file_buffer);index_file_buffer+=4;
    modelset->models[0].animation.animationName = new char[modelset->models[0].animation.animationNameLength];
    for(size_t i = 0; i < modelset->models[0].animation.animationNameLength; i++)
        modelset->models[0].animation.animationName[i] = file_buffer[index_file_buffer+i];
    index_file_buffer+=modelset->models[0].animation.animationNameLength;
    //Read skeleton
    uint32_t numBones = *(uint32_t*)(file_buffer+index_file_buffer);index_file_buffer+=4;
    printf("%s%d\n", "numBones: ", numBones);
    modelset->models[0].animation.bones = new Model::Animation::Bone[numBones];
    modelset->models[0].animation.numBones = numBones;
    Model::Animation::Bone *currentBone;
    for(size_t indexBone = 0; indexBone < numBones; indexBone++)
    {
        currentBone = &(modelset->models[0].animation.bones[indexBone]);
        uint32_t boneNameLength = *(uint32_t*)(file_buffer+index_file_buffer);
        index_file_buffer+=4;
        currentBone->boneName = new char[boneNameLength];
        for(size_t i = 0; i < boneNameLength; i++)
        {
            currentBone->boneName[i] = file_buffer[index_file_buffer+i];
            printf("%c", file_buffer[index_file_buffer+i]);
        }
        printf("\n");
        index_file_buffer+=boneNameLength;
        for(size_t k = 0; k < 4; k++)
        {
            for(size_t j = 0; j < 3; j++)
            {
                currentBone->relMatrix[4*k+j] = *(float*)(file_buffer+index_file_buffer);
                index_file_buffer+=4;
            }
            currentBone->relMatrix[3] = 0;
            currentBone->relMatrix[7] = 0;
            currentBone->relMatrix[11] = 0;
            currentBone->relMatrix[15] = 1;
        }
    }
    printf("\n");
    modelset->models[0].animation.bones[0].worldMatrix = modelset->models[0].animation.bones[0].relMatrix;
    for(size_t indexBone = 1; indexBone < numBones; indexBone++)
    {
        currentBone = &(modelset->models[0].animation.bones[indexBone]);
        currentBone->parent = modelset->models[0].bones[indexBone+1].parent - 1;
        currentBone->worldMatrix = modelset->models[0].animation.bones[currentBone->parent].worldMatrix * currentBone->relMatrix;
    }
    //Read tracks
    uint32_t numTracks = *(uint32_t*)(file_buffer+index_file_buffer);index_file_buffer+=4;
    //printf("%s%d\n", "numTracks: ", numTracks);
    //bufferAnimation.tracks = new uint32_t[numTracks];
    //bufferAnimation.numTracks = numTracks;
    for(size_t i = 0; i < numTracks; i++)
    {
        index_file_buffer+=4;
    }
    //Read frames
    uint32_t numFrames = *(uint32_t*)(file_buffer+index_file_buffer);index_file_buffer+=4;
    printf("%s%d\n", "numFrames: ", numFrames);
    modelset->models[0].animation.numFrames = numFrames;
    modelset->models[0].animation.frames = new Model::Animation::Frame[numFrames];
    modelset->models[0].animation.framesData = new mat4[numBones*numFrames];
    for(size_t indexFrame = 0; indexFrame < numFrames; indexFrame++)
    {
        modelset->models[0].animation.frames[indexFrame].relBones = new mat4[numBones];
        modelset->models[0].animation.frames[indexFrame].worldBones = new mat4[numBones];
        for(size_t indexBone = 0; indexBone < numBones; indexBone++)
        {
            for(size_t i = 0; i < 4; i++)
            {
                for(size_t j = 0; j < 3; j++)
                {
                    modelset->models[0].animation.framesData[indexFrame*numBones+indexBone][i*4+j] = *(float*)(file_buffer+index_file_buffer);
                    //modelset->models[0].animation.frames[indexFrame].relBones[indexBone][i*4+j] = *(float*)(file_buffer+index_file_buffer);
                    index_file_buffer+=4;
                    //printf("%f ", modelset->models[0].animation.frames[indexFrame].relBones[indexBone][i*4+j]);
                }
                //printf("\n");
            }
            //printf("\n");
            modelset->models[0].animation.framesData[indexFrame*numBones+indexBone][3] = 0;
            modelset->models[0].animation.framesData[indexFrame*numBones+indexBone][7] = 0;
            modelset->models[0].animation.framesData[indexFrame*numBones+indexBone][11] = 0;
            modelset->models[0].animation.framesData[indexFrame*numBones+indexBone][15] = 1;
        }
    }
    printf("%lx\n", index_file_buffer);*/
    /*for(size_t indexBone = 0; indexBone < numBones; indexBone++)
    {
        for(size_t i = 0; i < 4; i++)
        {
            for(size_t j = 0; j < 4; j++)
            {
                printf("%f ", modelset->models[0].animation.bones[indexBone].relMatrix[4*i+j]);
            }
            printf("\n");
        }
        for(size_t i = 0; i < 4; i++)
        {
            for(size_t j = 0; j < 4; j++)
            {
                printf("%f ", modelset->models[0].animation.frames[0].relBones[indexBone][i*4+j]);
            }
            printf("\n");
        }
        printf("\n");
        for(size_t i = 0; i < 4; i++)
        {
            for(size_t j = 0; j < 4; j++)
            {
                printf("%f ", modelset->models[0].animation.bones[indexBone].worldMatrix[4*i+j]);
            }
            printf("\n");
        }
        printf("\n");
    }*/
}
