#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
	vec3 V;
    vec3 H; // eye space halfway vector
    vec2 texcoord;

	vec3 FragPos;
    vec3 TangentFragPos;
} vertexData;

uniform sampler2D tex;
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;

uniform vec3 Ia = vec3(0.15, 0.15, 0.15);
uniform vec3 Id = vec3(1.0, 1.0, 1.0);
uniform vec3 Is = vec3(1.0, 1.0, 1.0);
uniform vec3 Ks = vec3(0.2, 0.2, 0.2);
uniform int shinness = 8;

void main()
{
	vec3 N = texture(texture_normal0, vertexData.texcoord).rgb;

	N = normalize(N * 2.0 - 1.0);

	vec3 L = normalize(vertexData.L - vertexData.TangentFragPos);
	vec3 V = normalize(vertexData.V - vertexData.TangentFragPos);
	vec3 reflectDir = reflect(-L, N);
	vec3 H = normalize(L + V);  
	
	vec3 texColor = texture(tex, vertexData.texcoord).rgb;
	vec3 ambient = texColor * Ia;
	vec3 diffuse = texColor * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), shinness);
	fragColor = vec4(ambient + diffuse + specular, 1.0);
}