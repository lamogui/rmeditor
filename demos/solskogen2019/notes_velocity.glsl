#version 120

varying vec2 coords;
uniform sampler2D notes_velocity;
uniform sampler2D max_notes_velocity;



void main()
{

	float instCount = 7.0f;//textureSize(max_notes_velocity, 0).y;
	float nbNotes = 128.0;

	vec2 uv = coords*0.5 + vec2(0.5,0.5);

	vec2 tex;
	tex.x = floor(nbNotes * uv.x)/nbNotes + 0.5/nbNotes;
	tex.y = floor(instCount * uv.y)/instCount + 0.5/instCount;
	
	float height = 1.0/instCount;
	float y = mod(uv.y,height)/height;
	
	
	//y = y < texture2D(max_notes_velocity,tex).x ? 1.0: 0.0;
	y = texture2D(notes_velocity,tex).r;
	gl_FragColor = vec4(y,y,y,1.0);
}