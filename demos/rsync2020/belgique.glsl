

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
uniform sampler2D prout;

float wave_amp=0.03;
float wave_freq=3.0;
float wave_speed=5.0;

vec2 rot2D(vec2 p, float a)
{
    return vec2(p.x*cos(a)  -p.y*sin(a),
                p.x*sin(a) + p.y*cos(a));
}

bool on_2d_donut(vec2 p, vec2 c, float r, float e)
{
    float d=distance(p,c);
    float e2=e*.5;
    return d > r - e2 && d < r + e2;
    
}

vec4 earth_flag(vec2 uv)
{
   if (uv.x < -.666) {
return vec4(.10,.10,.10,1);
}else if (uv.x < .6666) {
return vec4( 	1., 233./255., 54./255., 1);
} else 
{
return vec4( 	1., 15./255., 33./255., 1);
}
}

void main()
{
    	vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);

    float d=-uv.x+0.3*uv.y /*+ texture(iChannel0,uv).x*0.03 */- exp(-1.0+uv.x) +  exp(-uv.y);
    float a=mod(wave_speed*iTime*1.05+d*wave_freq,4.0*3.14159265359);
    uv.x += cos(a)*wave_amp;
    uv.y += sin(a)*wave_amp;
	vec4 color = earth_flag(uv);
    float shadow = .75 - 0.25*(sin(a-0.3));
    gl_FragColor = color * shadow;
}