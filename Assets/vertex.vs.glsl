#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;
layout(location = 3) in vec3 iv3tangent;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform mat4 um4m;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
	vec3 V;
    vec3 H; // eye space halfway vector
    vec2 texcoord;

	vec3 FragPos;
    vec3 TangentFragPos;
} vertexData;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(um4m)));
	vec4 P = um4mv * vec4(iv3vertex, 1.0);
	vec3 T = normalize(normalMatrix * iv3tangent);
    vec3 B = normalize(normalMatrix * cross(iv3normal, iv3tangent));
    vec3 N = normalize(normalMatrix * iv3normal);
	mat3 TBN = transpose(mat3(T, B, N));  

	gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
	vertexData.texcoord = iv2tex_coord;

	vertexData.N = mat3(um4mv) * iv3normal;
	vertexData.H = vec3(1, 1, 1);

	vertexData.FragPos = vec3(um4m * vec4(iv3vertex, 1.0));   
	vertexData.L = TBN * (um4mv * vec4(0, 1, -1, 0)).xyz;
    vertexData.V  = TBN * (-P.xyz);
    vertexData.TangentFragPos  = TBN * vertexData.FragPos;
}