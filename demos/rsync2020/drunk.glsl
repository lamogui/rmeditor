#version 120
#define iTime sequence_time

uniform sampler2D notes_velocity;
uniform sampler2D max_notes_velocity;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float cam_fov;
uniform sampler2D hangover; 
uniform sampler2D credits; 
uniform sampler2D labo; 
uniform sampler2D deadline; 

float sampleNotes(vec3 id) {
	float instCount = 10.;
	float nbNotes = 128.0;
	vec2 tex;
	tex.x = floor(id.x + id.y + id.z)/nbNotes + 0.5/nbNotes;
	tex.y = floor(id.x - id.y + id.z)/instCount + 0.5/instCount;
return texture2D(max_notes_velocity,tex).r + texture2D(max_notes_velocity,1.0-tex).r;
}

float bass;
float cyl(vec3 p, float r, float h)
{
  return max(length(p.xz) - r, abs(p.y) - h);
}

float smin( float a, float b, float k )
{
    float h = max( k-abs(a-b), 0.0 )/k;
    return min( a, b ) - h*h*k*(1.0/4.0);
}

float bou(vec3 p, ivec3 id)
{
p.y += .8*sampleNotes(vec3(id));
  float d = cyl(p, 0.5, 0.5);
  return smin(d, cyl(p - vec3(0.0, 0.8, 0.0), 0.2, 0.3),0.5);
}

mat2 rot(float a)
{
  float c = cos(a);
  float s = sin(a);
  return mat2(c, s, -s, c);
}

int prout;

float map(vec3 p)
{
  vec3 per = vec3(3.0);
  ivec3 id = ivec3(p/per);
  vec3 q = mod(p, per) - 0.5 * per;
  q .y += 0.5 * sin(float(p.z));
  q.xy *= rot(float(id.x) * 0.2561 + float(id.y + id.z) + bass);
  q.yz *= rot(float(id.x) * 0.2561 + float(id.y + id.z) + bass);

  prout = id.x + id.y +id.z;
  float d = bou(q, id); 

  return d;
}

vec3 grad(vec3 p)
{
  vec2 e = vec2(0.001, 0.0);
  return normalize(vec3(map(p+e.xyy) - map(p-e.xyy), map(p+e.yxy) - map(p-e.yxy), map(p+e.yyx) - map(p-e.yyx)));
}

vec3 rm(vec3 ro, vec3 rd, out float st)
{
     st = 1.0;
  vec3 p = ro;
   for (int i = 0; i < 64; ++i)
    {
    float d = map(p);
    if (abs(d) < 0.01)
    {
      st = float(i) / 64.0;
      break;
    }
    p += rd * 0.7 * d;
  }
  return p;
}

vec3 shade(vec3 p,  vec3 ro, vec3 n, float st)
{
  return vec3(exp(-distance(ro, p) * 0.1)) * vec3(0.5 + 0.5 * cos(float(prout) + bass * 0.1), 0.5 + 0.5 * sin(float(prout) + bass * 0.1), 1.0) * (1.0 - st); //* (n * 0.5 + 0.5);
}


void main( )
{
  bass = iTime;//megabass();
  vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);

  vec3 finalColor = vec3(0.0);
  for (int i = 0; i < 4; ++i)
  {
    uv.y += 0.01 * cos(bass);
    
  float st;
  vec3 ro = vec3(0.0, 0.0, bass * 5.0);
  vec3 rd = normalize(vec3(uv, normalize(length(uv)) - 0.7));

  rd.xz *= rot(0.1 * bass);

  vec3 p = rm(ro, rd, st);
  vec3 n = grad(p);
  vec3 color = shade(p, ro, n, st);

  vec3 rd2 = reflect(rd, n);
  vec3 ro2 = p + rd2 * 0.01;

  float st2;
  vec3 p2 = rm(ro2, rd2, st2);
  vec3 n2 = grad(p2); 

  color = mix(color, shade(p2, ro, n2, st2), 0.2);
  color = pow(color, vec3(1.0 / 2.2));
   finalColor += color;
  
  }

vec2 puv = uv * 1.5;
puv.y += .1*sin(track_time * 2. + uv.x * 2.);
if ( abs(puv.x) < 1.25 && abs(puv.y) < 0.5) {
vec2 fuv = puv * vec2(.4,1.) + vec2(.50,0.5);
 if ( track_time > 102.1) 
finalColor += 4.0*texture2D( credits, fuv).rbg;
else if ( track_time > 93.8) 
finalColor += 4.0*texture2D( hangover, fuv).rbg;
else if ( track_time > 85.5) 
finalColor += 4.0*texture2D( deadline, fuv).rbg;
else {
finalColor += 4.0*texture2D( labo, fuv).rbg; 
}
}

   gl_FragColor = vec4(finalColor / 4.0, 1.0);
}