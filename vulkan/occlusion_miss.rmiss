#version 460

#include "util.glsl"

layout(location = OCCLUSION_RAY) rayPayloadInNV bool occlusion_hit;

void main() {
    occlusion_hit = false;
}

