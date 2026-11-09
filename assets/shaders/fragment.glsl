#version 330 core
in vec2 TexCoord;
in vec4 Color;
in float Flash; // 0-1 blend toward white
out vec4 FragColor;

uniform sampler2D tex;

void main() {
    vec4 texColor = texture(tex, TexCoord) * Color;
    FragColor = vec4(mix(texColor.rgb, vec3(1.0), Flash), texColor.a);
}
