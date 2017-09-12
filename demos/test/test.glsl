#version 440

smooth in vec2 uv;
smooth in vec2 coords;

layout(location = 0) out vec4 color;
layout(location = 1) out vec3 normal;
layout(location = 2) out float depth;


float map(vec3 p)
{
	return length(p) - 1.0;
}

vec3 rm(vec3 ro, vec3 rd)
{
	vec3 p = ro;
	for (int i = 0 ; i < 64; i++)
	{
		float d = map(p);
		if (abs(d) < 0.01)
			break;
		p += d * rd;
	}
	return p;
}

void main()
{
	vec3 ro = vec3(0.0, 0.0, -2.0);
	vec3 rd = vec3(uv, 1.0);
	
	vec3 p = rm(ro, rd);
	depth = exp(-distance(ro, p));

  color = vec4(coords, 1.0, 1.0);
	normal = vec3(0.0, 0.0, 0.0);
}