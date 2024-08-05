#version 450

layout(binding = 0) uniform VPbuffer {
    mat4 view;
    mat4 proj;
    mat4 projView;
} vp;
layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
} ubo;
layout(binding = 2) uniform BoneBuffer {
    mat4 bone[256];
} bones;
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inBinormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in uvec4 inBlendIndices;
layout(location = 6) in vec4 inBlendWeights;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    gl_Position = vp.proj * vp.view * ubo.model * bones.bone[inBlendIndices[0]] * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
