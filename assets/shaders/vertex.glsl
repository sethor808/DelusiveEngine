#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aFlash;

//Positions arrive in world (or UI) space; the batch has no per-sprite model matrix
uniform mat4 viewProjection;

out vec2 TexCoord;
out vec4 Color;
out float Flash;

void main() {
    gl_Position = viewProjection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTex;
    Color = aColor;
    Flash = aFlash;
}
