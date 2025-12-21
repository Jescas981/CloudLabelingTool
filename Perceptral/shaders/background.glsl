#type fragment
#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform int uMode;               // 0=solid, 1=gradient, 2=skybox, 3=image
uniform vec4 uSolidColor;
uniform vec4 uGradientTop;
uniform vec4 uGradientBottom;

void main() {
    if (uMode == 0) {
        // Solid color
        FragColor = uSolidColor;
    } 
    else if (uMode == 1) {
        // Vertical gradient
        float t = vTexCoord.y;  // 0 at bottom, 1 at top
        FragColor = mix(uGradientBottom, uGradientTop, t);
    }
    else if (uMode == 2) {
        // Skybox (placeholder - would need cubemap texture)
        FragColor = vec4(0.1, 0.1, 0.2, 1.0);
    }
    else {
        // Image (placeholder)
        FragColor = vec4(vTexCoord, 0.5, 1.0);
    }
}

#type vertex
#version 330 core
layout(location = 0) in vec3 aPosition;
out vec2 vTexCoord;

void main() {
    // Transform [0,1] to clip space [-1,1]
    gl_Position = vec4(aPosition.xy * 2.0 - 1.0, 0.0, 1.0);
    vTexCoord = aPosition.xy;
}