#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
struct Color
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
};
struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};
void loader_dds(char *name_texture, VkFormat *format,uint32_t *Width, uint32_t *Height, uint32_t *ImageSize, uint8_t **texture_buffer)
{
    FILE *texture_file = fopen(name_texture, "r");
    if(texture_file == NULL)
    {
        printf("не может открыть текстуру\n");
        exit(-1);
    }
    fseek(texture_file, 0, 2);
    size_t compressed_data_size = ftell(texture_file) -128;
    fseek(texture_file, 0, 0);
    uint8_t dds_header[128];
    fread(dds_header, 1, 128, texture_file);
    *Height = *(uint32_t*)(dds_header+12);
    *Width = *(uint32_t*)(dds_header+16);
    uint32_t dwPitchOrLinearSize = *(uint32_t*)(dds_header+20);
    uint32_t ddspf_dwFourCC = *(uint32_t*)(dds_header+0x54);
    uint8_t *compressed_texture_buffer = new uint8_t[dwPitchOrLinearSize];
    fread(compressed_texture_buffer, 1, dwPitchOrLinearSize, texture_file);
    fclose(texture_file);
    //DDS_PIXELFORMAT pixelformat;
    const uint32_t DXT1 = 0x31545844;
    const uint32_t DXT5 = 0x35545844;
    switch(ddspf_dwFourCC)
    {
    case DXT1:
        *format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        break;
    case DXT5:
        *format = VK_FORMAT_BC3_SRGB_BLOCK;
        break;
    }
    *ImageSize = dwPitchOrLinearSize;
    *texture_buffer = compressed_texture_buffer;
    //delete []compressed_texture_buffer;
}
void decompressor_DXT(char *name_texture, uint32_t *Width, uint32_t *Height, uint32_t *ImageSize, uint8_t **texture_buffer)
{
    unsigned char *compressed_texture_buffer = 0; unsigned int index_compressed_texture_buffer = 0;
    //ôàéë òåêñòóðû
    FILE *texture_file = fopen(name_texture, "r");
    if(texture_file == NULL)
        exit(0);
    fseek(texture_file, 0, 2);
    unsigned long compressed_texture_size = ftell(texture_file);
    fseek(texture_file, 0, 0);
    compressed_texture_buffer = new unsigned char[compressed_texture_size];
    fread(compressed_texture_buffer, 1, compressed_texture_size, texture_file);
    fclose(texture_file);

    unsigned int dwSize = *(unsigned int*)(compressed_texture_buffer+4);
    unsigned int dwFlags = *(unsigned int*)(compressed_texture_buffer+8);
    unsigned int dwHeight = *(unsigned int*)(compressed_texture_buffer+12);
    unsigned int dwWidth = *(unsigned int*)(compressed_texture_buffer+16);
    unsigned int dwPitchOrLinearSize = *(unsigned int*)(compressed_texture_buffer+20);
    unsigned int dwDepth = *(unsigned int*)(compressed_texture_buffer+24);
    unsigned int dwMipMapCount = *(unsigned int*)(compressed_texture_buffer+28);
    unsigned int dwReserved1[11] = {0,0,0,0,0,0,0,0,0,0,0};

    unsigned int ddspf_dwSize = *(unsigned int*)(compressed_texture_buffer+76);
    unsigned int ddspf_dwFlags = *(unsigned int*)(compressed_texture_buffer+80);
    unsigned int ddspf_dwFourCC = *(unsigned int*)(compressed_texture_buffer+84);
    unsigned int ddspf_dwRGBBitCount = *(unsigned int*)(compressed_texture_buffer+88);
    unsigned int ddspf_dwRBitMask = *(unsigned int*)(compressed_texture_buffer+92);
    unsigned int ddspf_dwGBitMask = *(unsigned int*)(compressed_texture_buffer+96);
    unsigned int ddspf_dwBBitMask = *(unsigned int*)(compressed_texture_buffer+100);
    unsigned int ddspf_dwABitMask = *(unsigned int*)(compressed_texture_buffer+104);

    unsigned int dwCaps = *(unsigned int*)(compressed_texture_buffer+108);
    unsigned int dwCaps2 = *(unsigned int*)(compressed_texture_buffer+112);
    unsigned int dwCaps3 = *(unsigned int*)(compressed_texture_buffer+116);
    unsigned int dwCaps4 = *(unsigned int*)(compressed_texture_buffer+120);
    unsigned int dwReserved2 = 0;
    index_compressed_texture_buffer = 128;
    *Width = dwWidth; *Height = dwHeight;
    *texture_buffer = new unsigned char[8*dwPitchOrLinearSize];
    *ImageSize = 8*dwPitchOrLinearSize;
    for(unsigned int y = 0; y < dwHeight/4; y++)
    {
        for(unsigned int x = 0; x < dwWidth/4; x++)
        {
            //ðàçæàòèå öâåòîâ
            Color colors[4];
            unsigned short compressed_color_0, compressed_color_1, buffer_16;
            compressed_color_0 = *(unsigned short*)(compressed_texture_buffer+index_compressed_texture_buffer);
            colors[0].R = ((compressed_color_0>>11)+1)*8 - 1;
            buffer_16 = compressed_color_0<<5;
            colors[0].G = ((buffer_16>>10)+1)*4 - 1;
            buffer_16 = compressed_color_0<<11;
            colors[0].B = ((buffer_16>>11)+1)*8 - 1;
            index_compressed_texture_buffer += 2;
            compressed_color_1 = *(unsigned short*)(compressed_texture_buffer+index_compressed_texture_buffer);
            colors[1].R = ((compressed_color_1>>11)+1)*8 - 1;
            buffer_16 = compressed_color_1<<5;
            colors[1].G = ((buffer_16>>10)+1)*4 - 1;
            buffer_16 = compressed_color_1<<11;
            colors[1].B = ((buffer_16>>11)+1)*8 - 1;
            //colors[3].B = compressed_color & 0x1f;
            index_compressed_texture_buffer += 2;
            //ðàñïðåäåëåíèå ïèêñåëåé
            unsigned int index_table = *(unsigned int*)(compressed_texture_buffer+index_compressed_texture_buffer);
            index_compressed_texture_buffer += 4;
            unsigned int buffer_32;unsigned char bit_shift = 32;
            for(unsigned int yy = 0; yy < 4; yy++)
            {
                for(unsigned int xx = 0; xx < 4; xx++)
                {
                    bit_shift -= 2;
                    buffer_32 = index_table<<bit_shift;
                    buffer_32 = buffer_32>>30;
                    if(buffer_32 <= 1)
                    {
                        *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+0) = colors[buffer_32].R;
                        *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+1) = colors[buffer_32].G;
                        *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+2) = colors[buffer_32].B;
                    }
                    else if(compressed_color_0 > compressed_color_1)
                    {
                        if(buffer_32 == 2)
                        {
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+0) = (colors[0].R*2+colors[1].R)/3;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+1) = (colors[0].G*2+colors[1].G)/3;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+2) = (colors[0].B*2+colors[1].B)/3;
                        }
                        else
                        {
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+0) = (colors[0].R+colors[1].R*2)/3;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+1) = (colors[0].G+colors[1].G*2)/3;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+2) = (colors[0].B+colors[1].B*2)/3;
                        }
                    }
                    else if(compressed_color_0 <= compressed_color_1)
                    {
                        if(buffer_32 == 2)
                        {
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+0) = (colors[0].R+colors[1].R)/2;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+1) = (colors[0].G+colors[1].G)/2;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+2) = (colors[0].B+colors[1].B)/2;
                        }
                        else
                        {
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+0) = 0;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+1) = 0;
                            *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+2) = 0;
                        }
                    }
                    *(*texture_buffer+dwWidth*4*4*y+16*x+dwWidth*4*yy+4*xx+3) = 255;
                }
            }
        }
    }
    delete []compressed_texture_buffer;
}
