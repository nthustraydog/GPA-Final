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
} vertexData;

uniform sampler2D tex;

uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7); 
uniform vec3 specular_albedo = vec3(0.7);          
uniform float specular_power = 200.0;     

uniform vec3 Ia = vec3(0.15, 0.15, 0.15);
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
	vec4 lightingColor = vec4(ambient + diffuse + specular, 1.0);

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
}