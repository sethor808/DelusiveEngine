#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex;
uniform vec4 uColor;

void main() {
    float alpha = texture(tex, TexCoord).r;
    FragColor = vec4(uColor.rgb, uColor.a * alpha);
}