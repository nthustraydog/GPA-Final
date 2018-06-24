#version 410 core

uniform sampler2D color_map;
uniform sampler2D normal_map;
uniform sampler2D depth_map;
uniform sampler2D noise_map;
uniform vec2 noise_scale;
uniform mat4 proj;
uniform float zFar;
uniform float zNear;

in VS_OUT
{
	vec2 texcoord;
} fs_in;

layout(std140) uniform Kernals
{
	vec4 kernals[32];
};

out vec4 fragAO;

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // 回到NDC
	return (2.0 * zNear) / (zFar + zNear - z * (zFar - zNear));    
}

void main()
{
	float depth = texture(depth_map, fs_in.texcoord).r;
	if(depth == 1.0) { discard; }
	mat4 invproj = inverse(proj);
	vec4 position = invproj * vec4(vec3(fs_in.texcoord, depth) * 2.0 - 1.0, 1.0);
	position /= position.w;
	//
	fragAO = vec4(vec3(position.z), 1.0);
	//
	vec3 N = texture(normal_map, fs_in.texcoord).xyz;
	vec3 randvec = normalize(texture(noise_map, fs_in.texcoord * noise_scale).xyz * 2.0 - 1.0);
	vec3 T = normalize(randvec - N * dot(randvec, N));
	vec3 B = cross(N, T);
	mat3 tbn = mat3(T, B, N); // tangent to eye matrix
	const float radius = 2.0;
	float ao = 0.0;
	for(int i = 0; i < 32; ++i)
	{
		vec4 sampleEye = position + vec4(tbn * kernals[i].xyz * radius, 0.0);
		vec4 sampleP = proj * sampleEye;
		sampleP /= sampleP.w;
		sampleP = sampleP * 0.5 + 0.5;
		float sampleDepth = texture(depth_map, sampleP.xy).r;
		vec4 invP = invproj * vec4(vec3(sampleP.xy, sampleDepth) * 2.0 - 1.0, 1.0);
		invP /= invP.w;
		if(sampleDepth > sampleP.z || length(invP - position) > radius)
		{
			ao += 1.0;
		}
	}
	fragAO = vec4(vec3(ao / 32.0), 1.0);
}
