#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec4 uColor;

void main() {
    float alpha = texture(text, TexCoords).r;
    FragColor = vec4(uColor.rgb, uColor.a * alpha);
}