
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
uniform sampler2D prout;


void main()
{
	vec2 uv = coords*0.5 +0.5;
uv.x *= 2.0;
uv *= 5.0;
uv -= vec2(4.5, 2.0);
	vec4 color = vec4(0.0,0.0,0.,0.);
if (uv.x > 0.0 && uv.x < 1.0 && uv.y > 0.0 && uv.y < 1.0) {
color=texture2D(prout,uv ) * (1.0 - smoothstep(110.0, 115.0, track_time));
}
	gl_FragColor = vec4(color.rgb * color.a, color.a );
}