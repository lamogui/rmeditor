
#version 120
#define iTime sequence_time*0.1

uniform sampler2D notes_velocity;
uniform sampler2D max_notes_velocity;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float cam_fov;

#define PI 3.14159265359


vec4 hsv_to_rgb(float h)
{
	float c = 1.;
	h = mod((h * 6.0), 6.0);
	float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
	vec4 color;
 
	if (0.0 <= h && h < 1.0) {
		color = vec4(c, x, 0.0, 1.);
	} else if (1.0 <= h && h < 2.0) {
		color = vec4(x, c, 0.0, 1.);
	} else if (2.0 <= h && h < 3.0) {
		color = vec4(0.0, c, x, 1.);
	} else if (3.0 <= h && h < 4.0) {
		color = vec4(0.0, x, c, 1.);
	} else if (4.0 <= h && h < 5.0) {
		color = vec4(x, 0.0, c, 1.);
	} else if (5.0 <= h && h < 6.0) {
		color = vec4(c, 0.0, x, 1.);
	} else {
		color = vec4(0.0, 0.0, 0.0, 1.);
	}
 
	return color;
}

vec3 rotate_dir(vec4 q, vec3 dir)
{
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

mat2 rot( float a ) {
	float c = cos(a);
  
	float s = sin(a);
	return mat2(c,-s,s,c);
}

float map ( vec3 pos ) {
	float scene = 1.;
	const float count = 4.;
vec3 p = pos;
float a = 1.;
float falloff = 1.3;
float d = length(p);
float  wave = .5+.5*sin(d*3.+iTime);
for (float i = count; i > 0.; --i) {
	p.xy = abs(p.xy)-(.2+.3*wave+.2*sin(iTime*4.+i*.3))*a;
p.xy *= rot(iTime+sin(iTime/4.));
p.zy *= rot(iTime*.5+sin(iTime*2.));
	scene = min(scene, length(p.xy)-.05*a);
	a /= falloff;
}
	return scene;
}

void main()
{
	
    	vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);
	vec3 from = cam_position;
	vec3 dir = rotate_dir(cam_rotation, normalize(vec3(uv, 2.0 * cam_fov)));
vec3 p = from;
const float count = 50.;
float shade = 0.;
	for (float i = 0; i < count; ++i) {
	float d = map(p);
	if (abs(d) < .001) {
	shade = i / count;		
break;
	}
	p += dir * d;
}
    vec3 c = vec3(exp(-.1*distance(from, p)));
c = (vec3(0.5) + hsv_to_rgb(iTime+p.x+p.z).rgb )* shade + 1.5* shade;
c *= 1.-shade;   
gl_FragColor = vec4(c, 1.);
}