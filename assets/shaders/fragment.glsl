#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex;
uniform float uAlpha; // new global alpha multiplier

void main() {
    vec4 texColor = texture(tex, TexCoord);
    FragColor = vec4(texColor.rgb, texColor.a * uAlpha);
}