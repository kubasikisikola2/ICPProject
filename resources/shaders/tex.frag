in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D tex0;

void main() {
    vec4 color = texture(tex0, TexCoords);
  //  if (color.a < 0.1)
     //   discard;
    FragColor = texture(tex0, TexCoords);

}