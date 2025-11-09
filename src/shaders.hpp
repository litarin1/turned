constexpr static const char* VERTEX_SHADER_2D = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform mat4 MVP;
uniform vec2 dimensions;
out vec2 TexCoord;
void main(){
    gl_Position = MVP * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

constexpr static const char* FRAGMENT_SHADER_2D = R"(
#version 330 core
out vec4 Color;
in vec2 TexCoord;
uniform sampler2D Sprite;
void main(){
    Color = texture(Sprite, TexCoord);
    if (Color.a != 1.0) discard;
}
)";

constexpr static const char* FRAGMENT_SHADER_2D_SINGLE_COLOR = R"(
#version 330 core
out vec4 Color;
in vec2 TexCoord;
void main(){
    Color = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
