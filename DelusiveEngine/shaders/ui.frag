#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;

void main() {
    vec4 baseColor = uColor;
    if (uUseTexture)
        baseColor *= texture(uTexture, TexCoord);
    FragColor = baseColor;
}