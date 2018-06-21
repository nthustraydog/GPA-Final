#version 410

layout (location = 0) in vec3 pos;

uniform vec3 eyePos;
uniform mat4 vpMatrix;

out VS_OUT
{
	vec3 texCoord;
} vs_out;

void main()
{
	gl_Position = vec4(pos, 1.0);

	vec4 posInWorld = inverse(vpMatrix) * vec4(pos, 1.0);
	vs_out.texCoord = (posInWorld.xyz / posInWorld.w) - eyePos;
}