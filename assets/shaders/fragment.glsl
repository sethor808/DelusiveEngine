#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex;
uniform float uAlpha; // new global alpha multiplier
uniform float uFlash; // 0-1 blend toward white

void main() {
    vec4 texColor = texture(tex, TexCoord);
    FragColor = vec4(mix(texColor.rgb, vec3(1.0), uFlash), texColor.a * uAlpha);
}