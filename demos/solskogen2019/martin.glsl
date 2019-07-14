

#define mmin(v, d, i) (v.x > d ? vec2(d,i) : v)
#define ID_SPHERE 1.0
#define ID_LIGHT 2.0
#define ID_BLACK 3.0
#define PI 3.14159265

uniform sampler2D notes_velocity;
uniform sampler2D max_notes_velocity;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float cam_fov;
uniform sampler2D recycle;
uniform sampler2D prout;
uniform sampler2D texnoise;

float lc;

vec3 rotate_dir(vec4 q, vec3 dir)
{
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}


float sphere(vec3 p, vec3 pos, float id)
{
  vec3 q = normalize(p - pos);
  vec2 uv = vec2(atan(q.z, q.x) / (2.0 * PI), acos(q.y) / PI);
  float d = 0.15; //* texture2D(iChannel0, uv + sin(2.0 * PI * id) * iTime * 0.1 + vec2(id * 0.4), 0.0).x;
  return length(p - pos) - (0.35)* (1.0 + d * 0.15);
}

float cylinder(vec3 p)
{
float dis = (2.0-4.0*texture2D(texnoise, vec2(p.y*0.05 + 0.2*floor(sequence_time *20.0))).r) * smoothstep(73.5, 73.7, track_time);
  return length(p.xz + dis) - 0.2 - 0.1 * lc;
}

mat3 rotateY(float a)
{
  float c = cos(a);
  float s = sin(a);
  return mat3(c, 0.0, -s, 
            0.0, 1.0, 0.0, 
              s, 0.0, c);
}

mat3 rotateZ(float a)
{
  float c = cos(a);
  float s = sin(a);
  return mat3(c, -s , 0.0, s, c, 0.0 , 0.0 ,0.0 ,1.0);
}

mat3 rotateX(float a)
{
  float c = cos(a);
  float s = sin(a);
  return mat3(1.0, 0.0 ,0.0,
              0.0 , c, -s,
              0.0, s, c);
}

float modA(inout vec2 v, float n, float len)
{
  float l = length(v);
  float an = PI * 2.0 / n; 
  
  float a = atan(v.y, v.x);
  float id = floor(a / an);
  a = mod(a, an) - an * 0.5;
  //l = mod(l, len) - len * 0.5;
  v = vec2(cos(a), sin(a)) * l;
  return id;
}

vec2 map(vec3 p)
{
  float d = 100000.0;
  for (int i = 0; i < 8; ++i)
  {
    int j = i + int(sequence_time * .1);
    vec3 q = rotateY(sequence_time * (0.1 + 0.3 * fract(0.3 + 0.1451 * float(i)))) * p;
    float id = modA(q.xz, 10.0 + 4.0 * float(i), 4.0);
    float x = 1.0 + (float(j) - sequence_time * .1) * .5 + 0.3 * fract(id * 0.534);
    d = min(sphere(q, vec3(x, sqrt((x - 1.0) * 0.9) - 0.3 + 0.2 * texture2D(notes_velocity, vec2(fract(id * 0.784), fract(id * 0.1554)), 0.0).x, 0.0), fract(id * 0.15614 * float(i))),d);
  }
  vec2 v = vec2(d, ID_SPHERE);
  v = mmin(v, cylinder(p), ID_LIGHT);
  return v;
}

vec4 rm(vec3 ro, vec3 rd)
{
  vec3 p = ro;
  vec2 d;
  float id = ID_BLACK;
  for (int i = 0; i< 64; ++i)
  {
    d = map(p);
    if (abs(d.x) < 0.001)
    {
      id = d.y;
      break;
    }
    else if (d.x > 1000.0)
      break;
    p += rd * d.x * 0.9;
  }
  return vec4(p, id);
}

vec3 grad(vec3 p)
{
  vec2 e = vec2(0.01, 0.0);
  return normalize(vec3(map(p + e.xyy).x - map(p - e.xyy).x, map(p + e.yxy).x - map(p - e.yxy).x, map(p + e.yyx).x - map(p - e.yyx).x));
}

float f(float a)
{
  a *= PI;
  return pow(abs(cos(a)) + abs(sin(a)), -1.0 / 0.05);
}

void main( )
{
  vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);
  float a = -PI / 2.0;
  vec3 ro = cam_position;//vec3(3.5 * cos(a), 3.0, -2.0 + 3.5 *sin(a));
  vec3 rd = rotate_dir(cam_rotation, normalize(vec3(uv, 2.0 * cam_fov)));
  //rd = rotateY(-PI / 2.0 + a) * rd;
  rd = rotateX(-0.2) * rd;
  rd = rotateZ(0.2) * rd;
  
  lc = clamp(1.0-f(.05*sequence_time) - f(sequence_time + sin(sequence_time * 3.0)), 0.0, 1.0);
  vec4 q = rm(ro, rd);
  vec3 p = q.xyz;
  float id = q.w;
  float d = distance(ro, p);
  
  vec3 n = grad(p);
  
  vec3 color;
  if (id == ID_LIGHT)
    color = vec3(0.8 + 0.2 * lc);
  else if (id == ID_SPHERE)
  {
    color = vec3(0.05);
//if (p.y > 1.0)
//color += texture2D(recycle, p.xz  *10.0).ggg;
    //color *= n * 0.5 + 0.5;
    vec3 pp = p;
    q = rm(p + 0.1 * n, reflect(rd, n));
    p = q.xyz;
    id = q.w;
    n = grad(p);
    if (id == ID_LIGHT)
      color =  mix(color, vec3(lc) * exp(-0.2*distance(vec2(0.0), pp.xz)), 0.7);
    else if (id == ID_SPHERE) {

      color = mix(color, 0.15 * (n * 0.5 + 0.5), 0.1);
}
  }
    
  if (id != ID_LIGHT)
    color *= exp(-d * 0.1);
  color = pow(color, vec3(1.0 / 1.1));
  gl_FragColor = vec4(color, 1.0);
}
