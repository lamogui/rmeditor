//#version 120

uniform vec2 resolution;

float map(vec3 p)
{
	return cos(p.x) + cos(p.y) + cos(p.z);
}

void main()
{
	vec2 uv = vec2(gl_FragCoord.xy) / resolution;

  for (float i=0.0; i < 64.0; ++i)
{
	
}

	vec3 color = vec3(uv, 0.0);
	gl_FragColor = vec4(color, 1.0);
}