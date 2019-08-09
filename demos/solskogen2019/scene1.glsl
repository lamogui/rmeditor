#define PI 3.14159265

uniform sampler2D notes_velocity;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;
uniform float cam_fov;
uniform sampler2D tex1;

mat2 rot(float t) {
	float c = cos(t);
	float s = sin(t);
	return mat2(c, s, -s, c);
}

vec3 rotate_dir(vec4 q, vec3 dir) {
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

float fOpUnionStairs(float a, float b, float r, float n) {
	float s = r/n;
	float u = b-r;
	return min(min(a,b), 0.5 * (u + a + abs ((mod (u - a + s, 2. * s)) - s)));
}

float sdbox(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return max(max(d.x, d.y), d.z);
}

float map(vec3 p ) {
	
	float q = sin(p.y*0.1);


	float d = p.z;

	#define FS 2.0
	float scale = 1.0;
	for (int i = 0 ; i < 4 ; i++) {
		p *= FS; scale *= FS;

		p.xy = abs(fract(p.xy)-0.5);
		p.xy *= rot(q*20.0+sequence_time*0.1);

		float dd = sdbox(p, vec3(0.1, 0.2, 6.0)) / scale;
		//d = min(d, dd);
		
		d = fOpUnionStairs(d, dd, 0.1, 4.0);

	}
	
	return d;

}

vec3 normal( in vec3 p ) {
	vec3 eps = vec3(0.001, 0.0, 0.0);
	return normalize( vec3(
		map(p+eps.xyy)-map(p-eps.xyy),
		map(p+eps.yxy)-map(p-eps.yxy),
		map(p+eps.yyx)-map(p-eps.yyx)
	) );
}

vec3 rm(vec3 from, vec3 dir, inout float totdist)
{
	for (int i = 0 ; i < 100 ; i++) {
		vec3 p = from + dir*totdist;
		float d = map(p)*0.8;
		totdist += d;
		if (d < 0.0001) {
			break;
		}
	}
	
	return from+dir*totdist;
}

vec3 shade( vec3 n, float totdist, vec3 p)
{
		vec3 c = 0.7* texture2D(tex1, vec2(p.xz*10.0)).r * ( n*0.5+0.5).r;
	c /= max(1.0, totdist);
return c;
}

void main() {
	gl_FragColor = vec4(0.0);
	vec2 uv = coords.xy;
	uv.x *= xy_scale_factor;
	vec3 from = cam_position;
	vec3 dir = normalize(vec3(uv, 1/cam_fov));
	dir = rotate_dir(cam_rotation, dir);

	float totdist = 0.0;
	
	vec3 p = rm(from, dir, totdist);
	vec3 n = normal(p);
vec3 c1 = shade(n, totdist, p);

	vec3 rd2 = reflect(dir, n);
	vec3 ro2 = p + rd2 * 0.01;
	float totdist2 = 0.0;
	vec3 p2 = rm(ro2, rd2, totdist2);
	vec3 n2 = normal(p2);
vec3 c2 = shade(n2, totdist2, p);

	gl_FragColor.rgb = (c1 + c2) /2.;
gl_FragColor.a = 1.0;
}