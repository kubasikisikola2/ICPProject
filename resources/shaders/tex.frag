in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D tex0;

void main() {
    FragColor = texture(tex0, TexCoords);

}