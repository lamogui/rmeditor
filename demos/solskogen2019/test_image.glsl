#version 120

varying vec2 coords;
uniform sampler2D prout;
uniform sampler2D recycle;
uniform float track_time;

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