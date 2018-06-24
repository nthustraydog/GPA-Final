#version 410

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 ambientColor;

uniform mat4 um4mv;
uniform mat4 um4p;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
	vec3 V;
    vec3 H; // eye space halfway vector
    vec2 texcoord;
	vec4 fragPosLightSpace;
	vec3 FragPos;
    vec3 TangentFragPos;
	vec3 viewN;
} vertexData;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;

uniform vec3 Ia = vec3(0.2, 0.2, 0.2);
uniform vec3 Id = vec3(1.0, 1.0, 1.0);
uniform vec3 Is = vec3(1.0, 1.0, 1.0);
uniform vec3 Ks = vec3(0.2, 0.2, 0.2);
uniform int shinness = 8;

uniform int fogEffect_switch;
in vec4 viewSpace_coord;
const vec4 fogColor = vec4(0.933, 0.910, 0.667, 1.0);
float fogFactor = 0;
float fog_start = 1;
float fog_end = 500;

uniform sampler2D depthMap;
float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{
	vec3 N = texture(texture_normal0, vertexData.texcoord).rgb;

	N = normalize(N * 2.0 - 1.0);

	vec3 L = normalize(vertexData.L); 
	vec3 V = normalize(vertexData.V); 
	vec3 H = normalize(L + V);   
	
	vec3 texColor = texture(texture_diffuse0, vertexData.texcoord).rgb;
	vec3 ambient = texColor * Ia;
	vec3 diffuse = texColor * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), shinness);

	float shadow = ShadowCalculation(vertexData.fragPosLightSpace);
	vec4 lightingColor = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0);

	if(fogEffect_switch == 1)
	{
		//Turn Fog Effect On (Recommended)
		float dist = length(viewSpace_coord);
		fogFactor = (dist-fog_start)/(fog_end-fog_start);
		fogFactor = clamp( fogFactor, 0.0, 1.0 );
		fragColor = mix(lightingColor, fogColor, fogFactor);
	}
	else if(fogEffect_switch == 0)
	{
		//Turn Fog Effect Off (Use At Your OWN RISK)
		fragColor = lightingColor;
	}
	fragNormal = vertexData.viewN;
	ambientColor = ambient;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float bias = max(0.05 * (1.0 - dot(vertexData.N, vertexData.L)), 0.002);
	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    float currentDepth = projCoords.z;

	float shadow = 0.0;
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float closestDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth -bias > closestDepth  ? 1.0 : 0.0;
			}
	}
	shadow /= 9.0;
    return shadow;
}