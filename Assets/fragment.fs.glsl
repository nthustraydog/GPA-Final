#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform sampler2DShadow shadow_tex;
uniform sampler2D tex;
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;

in VertexData
{
	vec4 shadow_coord;
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
	vec3 V;
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform vec3 Ia = vec3(0.15, 0.15, 0.15);
uniform vec3 Id = vec3(1.0, 1.0, 1.0);
uniform vec3 Is = vec3(1.0, 1.0, 1.0);
uniform vec3 Ks = vec3(0.2, 0.2, 0.2);
uniform int shinness = 8;

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
	
	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord, 0.005);

	vec3 texColor = texture(texture_diffuse0, vertexData.texcoord).rgb;
	vec3 ambient = texColor * Ia;
	vec3 diffuse = texColor * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), shinness);
	fragColor = vec4(ambient, 1.0) + shadow_factor * vec4(diffuse + specular, 1.0);
	//fragColor = vec4(shadow_factor, shadow_factor, shadow_factor, 1.0);
	float a = texture(shadow_tex, vec3(gl_FragCoord.xy/600.0,0));
	fragColor = vec4(a,a,a,1);
}