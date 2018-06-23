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
	vec4 fragPosLightSpace;
} vertexData;

uniform sampler2D tex;

uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7); 
uniform vec3 specular_albedo = vec3(0.7);          
uniform float specular_power = 200.0;     

uniform vec3 Ia = vec3(0.2, 0.2, 0.2);
uniform vec3 Id = vec3(1.0, 1.0, 1.0);
uniform vec3 Is = vec3(1.0, 1.0, 1.0);
uniform vec3 Ks = vec3(0.2, 0.2, 0.2);
uniform int shinness = 8;

uniform sampler2D depthMap;
float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{
    /*vec3 texColor = texture(tex,vertexData.texcoord).rgb;
    fragColor = vec4(texColor, 1.0);*/
	//Debug Only for U,V
    //fragColor = vec4(vertexData.texcoord, 0.0, 1.0);


	vec3 N = normalize(vertexData.N); 
	vec3 L = normalize(vertexData.L); 
	vec3 V = normalize(vertexData.V); 
	vec3 H = normalize(L + V);   
	
	vec3 texColor = texture(tex, vertexData.texcoord).rgb;
	vec3 ambient = texColor * Ia;
	vec3 diffuse = texColor * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), shinness);
	//fragColor = vec4(ambient + diffuse + specular, 1.0);
	float shadow = ShadowCalculation(vertexData.fragPosLightSpace);
	fragColor = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0);
	//fragColor = vec4(vec3(1.0-shadow), 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
	float bias = 0.0025;
    float shadow = currentDepth -bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}