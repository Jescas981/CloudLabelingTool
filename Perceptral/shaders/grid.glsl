#type vertex
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aLineType; // 0 = major, 1 = minor

uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform vec3 uCameraPosition;
uniform float uFadeStart;
uniform float uFadeEnd;
uniform bool uFadeEnabled;

out vec4 vColor;
out float vFade;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    gl_Position = uViewProjection * worldPos;
    
    vColor = aColor;
    
    // Distance-based fade
    if (uFadeEnabled) {
        float dist = distance(worldPos.xyz, uCameraPosition);
        vFade = 1.0 - smoothstep(uFadeStart, uFadeEnd, dist);
    } else {
        vFade = 1.0;
    }
}


#type fragment
#version 330 core
in vec4 vColor;
in float vFade;
out vec4 FragColor;

void main() {
    FragColor = vColor;
    FragColor.a *= vFade; // Apply distance fade
    
    // Optional: alpha discard for performance
    if (FragColor.a < 0.01) discard;
}