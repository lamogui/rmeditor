
#version 120
#define PI 3.141592
#define iTime track_time

uniform sampler2D notes_velocity;
uniform sampler2D max_notes_velocity;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float cam_fov;
uniform sampler2D rbbs;
uniform sampler2D prout;


float sampleNotes(vec3 id) {
	float instCount = 10.;
	float nbNotes = 128.0;
	vec2 tex;
	tex.x = floor(id.x + id.y + id.z)/nbNotes + 0.5/nbNotes;
	tex.y = floor(id.x - id.y + id.z)/instCount + 0.5/instCount;
return texture2D(max_notes_velocity,tex).r + texture2D(max_notes_velocity,1.0-tex).r;
}

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
float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

mat2 rot(float a) 
{
	float c = cos(a);
	float s = sin(a);
	return mat2( c, - s, s, c);
}

float cube(vec3 p, vec3 off) {
bool f = track_time > 26.8;
//if (!f) p.y -= sampleNotes(off);
p.xz *= rot( sequence_time + off.x);
p.xy *= rot( .5*sequence_time +off.x);
if (f)
p += sampleNotes(off);
return sdBox(p, vec3(1.));
}

float map(vec3 p)
{
vec3 off = vec3(0);
if ( track_time > 26.8) {
off = floor((p-2.5)/5.0) ;
p = mod(p+2.5, 5.) - 2.5;

}
return cube(p, off);
}

void main()
{
	vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);

vec3 ro = cam_position;
vec3 rd = rotate_dir(cam_rotation, normalize(vec3(uv, 2.0 * cam_fov)));
vec3 p = ro;
vec3 color =vec3(0);
float st = 1.;
for (float i = 0;i < 128.0; ++i) {
	float d = map(p);
if (abs(d) < .001) {
st = i/128.;
break;
}
p += d * rd;
}

vec3 c = vec3(.0);

if (abs(map(p)) <.002) {
vec3 q = p + rd * .1;
for (float i = 0;i < 128.0; ++i) {
	float d = map(q);
if (abs(d) < .001) {
c+=.5*vec3(1.0-i/128.);
break;
}
q+= 0.01 * rd;
c+=.01*i*hsv_to_rgb(q.x +q.y +q.z+ sequence_time*2.).rgb/128.;
}
}

vec3 q = p;

if (abs(q.y) < 1.0 && abs(q.x) < 1.0 && track_time < 26.8) {
//q.y -= sampleNotes(vec3(0));
c+=texture2D(prout, q.xy*0.5+.5).rgb;
}
color = (3.0*st + 1.)* (c+ +.0*hsv_to_rgb(uv.x +uv.y + sequence_time*2.).rgb)* (  vec3(exp(-.1*distance(ro, p))));
	gl_FragColor = vec4(pow(color.rgb,vec3(1./1.5)), 1. );
}