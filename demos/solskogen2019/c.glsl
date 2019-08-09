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
uniform sampler2D texnoise;

uniform sampler2D tex1;



vec3 rotate_dir(vec4 q, vec3 dir)
{
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}
mat2 rot(float a)
{
  float c = cos(a);
  float s = sin(a);
  return mat2(c, s ,-s,  c);
}

float sea(vec3 p)
{
  p.xy = rot(p.z) * p.xy;
  return length(p.y -texture2D(texnoise, vec2(p.xz * 0.1 + 0.1 * sequence_time)).r -0.5);
}
float co(vec3 p )
{
  p.xy = rot(p.z) * p.xy;
  return cos(p.x) + cos(p.y) + cos(p.z);
}

float map(vec3 p)
{
   
    float d = min(sea(p), co(p));
    return d;
}

vec3 rm(vec3 ro, vec3 rd, inout float st)
{
  vec3 p = ro;
  for (float i = 0.0; i < 128.0; i++){
  float d = map(p);
  
    if(abs(d) < 0.01)
    {
      st = i / 128.0;
      break;
    }
    p += rd * d * 0.9;
    }
    return p;
}

vec3 grad(vec3 p)
{
  vec2 e = vec2(0.01, 0.0); 
  return normalize(vec3(
    map(p - e.xyy),
    map(p - e.yxy),
    map(p - e.yyx)
  
  ));
}

vec3 shade(vec3 n, float st, vec3 p, vec3 ro)
{
  vec3 c = vec3(0.1) * (n * 0.5 + 0.5);
  c = texture(tex1, p.xy).rgb;
  if (abs(sea(p)) <0.05)
     c = vec3(1.0, 1.0, 1.0);
  c *= exp(-0.1 * distance(ro, p));
  return c * (1.0 - st);
}

void main(void)
{
  vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);
	vec3 ro = cam_position;
	vec3 rd = rotate_dir(cam_rotation, normalize(vec3(uv, 2.0 * cam_fov)));

  float st = 0.0;
  vec3 p = rm(ro, rd, st);
  vec3 n = grad(p);
  
  vec3 color = shade(n, st, p, ro);
  
  vec3 rd2 = reflect(rd, n);
  vec3 ro2 = p + 0.1 * rd2;
  
  
  
  float st2 = 0.0;
  vec3 p2 = rm(ro2, rd2, st); 
  vec3 n2 = grad(p2);
  color = mix (color, shade(n2, st2, p2, ro), 0.3);
  
  gl_FragColor = vec4(color, 1);
}