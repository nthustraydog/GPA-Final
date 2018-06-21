#version 410

out vec4 color;

uniform samplerCube skyboxTexture;

in VS_OUT
{
	vec3 texCoord;
} fs_in;

void main()
{
	color = texture(skyboxTexture, fs_in.texCoord);
}