#type fragment
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}

#type vertex
#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;           // Per-vertex color (RGB or computed)
layout (location = 2) in float aSelectionMask;  // 0.0 = not selected, 1.0 = selected
layout (location = 3) in float aLabel;          // Label ID (0-255)

out vec3 vertexColor;
out vec3 worldPos;

uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform float uPointSize;

// Colors for selection purposes
uniform vec3 uSelectedColor;
uniform vec3 uUnselectedColor;
uniform vec3 uSelectionColor;

// Label colors (up to 256 labels)
uniform bool uShowLabels;
uniform vec3 uLabelColors[256];

// Rendering modes: 0=RGB, 1=FlatColor, 2=AxisX, 3=AxisY, 4=AxisZ, 5=Gradient
uniform int uColorMode;
uniform bool uSelected;  // Rendering mode for selected object

// Bounding box for axis coloring and gradients
uniform vec3 uBoundsMin;
uniform vec3 uBoundsMax;

// Generate gradient color based on position along an axis
vec3 applyAxisGradient(vec3 pos, int axis) {
    float value;
    vec3 range = uBoundsMax - uBoundsMin;

    if (axis == 2) { // X axis
        value = (pos.x - uBoundsMin.x) / range.x;
    } else if (axis == 3) { // Y axis
        value = (pos.y - uBoundsMin.y) / range.y;
    } else { // Z axis
        value = (pos.z - uBoundsMin.z) / range.z;
    }

    // Clamp to [0, 1]
    value = clamp(value, 0.0, 1.0);

    // Create a color gradient (blue -> green -> red)
    vec3 color;
    if (value < 0.5) {
        // Blue to green
        color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), value * 2.0);
    } else {
        // Green to red
        color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (value - 0.5) * 2.0);
    }

    return color;
}

// Generate full 3D gradient based on all axes
vec3 applyFullGradient(vec3 pos) {
    vec3 range = uBoundsMax - uBoundsMin;
    vec3 normalized = (pos - uBoundsMin) / range;
    normalized = clamp(normalized, 0.0, 1.0);
    return normalized; // Use XYZ as RGB
}

// Apply rendering mode to get color
vec3 applyRenderMode(int mode, vec3 pos, vec3 vertexColor, vec3 flatColor) {
    if (mode == 0) {
        // RGB mode: use vertex color
        return vertexColor;
    } else if (mode == 1) {
        // Flat color mode
        return flatColor;
    } else if (mode == 2) {
        // X axis gradient
        return applyAxisGradient(pos, 2);
    } else if (mode == 3) {
        // Y axis gradient
        return applyAxisGradient(pos, 3);
    } else if (mode == 4) {
        // Z axis gradient
        return applyAxisGradient(pos, 4);
    } else if (mode == 5) {
        // Full gradient
        return applyFullGradient(pos);
    }

    return vertexColor; // Default fallback
}

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    worldPos = worldPosition.xyz;
    gl_Position = uViewProjection * worldPosition;
    gl_PointSize = uPointSize;

    // Select the object
    if (uSelected) {
        vertexColor = uSelectionColor;
        return;
    }

    // Use selection mask to determine which rendering mode to apply
    bool isSelected = (aSelectionMask > 0.5);

    if (isSelected) {
        // Selection color always overrides everything
        vertexColor = uSelectedColor;
    } else if (uShowLabels) {
        // Show label colors
        int labelId = int(aLabel);
        vertexColor = uLabelColors[labelId];
    } else {
        // Apply unselected rendering mode
        vertexColor = applyRenderMode(uColorMode, aPosition, aColor, uUnselectedColor);
    }
}
