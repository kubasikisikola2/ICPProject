#version 460 core
out vec4 FragColor;
uniform vec4 my_color;
void main() {
    FragColor = my_color;//vec4(1.0, 0.5, 0.2, 1.0); // orange
}
