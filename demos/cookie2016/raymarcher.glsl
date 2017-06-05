#version 120
#define PI 3.14159265

uniform sampler2D notes_velocity;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;

const float instCount = 7.0;
const float nbNotes = 128.0;
const float dLimit = 0.001;
const float dFar = 60.;
float rd_m = .9;

const vec3 forward = vec3(0.0,0.0,1.0);

float snare = 0.0;
float piano = 0.0;

#define MAT_SKY          0.
#define MAT_MIRROR  		 1.
#define MAT_PYRAMID 		 2.
#define MAT_SAND    		 3.
#define MAT_BLACKMIRROR  4.

#define mmin(v,mat,d) (v.x>d ? vec2(d,mat) : v);
#define fmin(v,mat,f,d) (v.x>f ? vec2(d,mat) : v);

//To use the framework implement the following functions

vec2 map(vec3 p); //x = distance, y = material
vec3 shade(vec3 p, vec3 n, vec3 ro, float mat, vec3 inputColor);
void globals(); //COmpute your globals variables

/* BEGIN STOLEN */ 

//mercury
float vmax(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float fBoxCheap(vec3 p, vec3 b) { //cheap box
	return vmax(abs(p) - b);
}

void pR(inout vec2 p, float a) {
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

float pModPolar(inout vec2 p, float repetitions) {
	float angle = 2.*PI/repetitions;
	float a = atan(p.y, p.x) + angle/2.;
	float r = length(p);
	float c = floor(a/angle);
	a = mod(a,angle) - angle/2.;
	p = vec2(cos(a), sin(a))*r;
	if (abs(c) >= (repetitions/2.)) c = abs(c);
	return c;
}

float fOpUnionStairs(float a, float b, float r, float n) {
	float s = r/n;
	float u = b-r;
	return min(min(a,b), 0.5 * (u + a + abs ((mod (u - a + s, 2. * s)) - s)));
}

float fPlane(vec3 p, vec3 n, float distanceFromOrigin) {
	return dot(p, n) + distanceFromOrigin;
}


//XT95
vec3 normal( in vec3 p )
{
	vec3 eps = vec3(0.01, 0.0, 0.0);
	return normalize( vec3(
		map(p+eps.xyy).x-map(p-eps.xyy).x,
		map(p+eps.yxy).x-map(p-eps.yxy).x,
		map(p+eps.yyx).x-map(p-eps.yyx).x
	) );
}

//iq
float ambientOcclusion( in vec3 pos, in vec3 nor )
{
    float ao = 0.0;
    float sca = 15.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 1.5*(0.01 + 0.015*float(i*i));
        float dd = map( pos + hr*nor ).x;
        ao += (hr-dd);
        sca *= 0.5;
    }
    return 1.0 - clamp( ao, 0.0, 1.0 );
}

/* END STOLEN */

//Fonction naming inspirated by alien cocoons by xt95

float pale(vec3 p, float r, float height) {
	float d = length(p.yz) - r;
	d = max(d, abs(p.x) - height);
	vec3 ps = p;
	ps.y *= 3.;
	float h = length(ps - vec3(height/2.,-height/1.2,0.)) - height/1.;
	return max(-h,d);
}

float getNoteVelocity(float n, float inst)
{
	vec2 tex;
	tex.x = n/nbNotes + 0.5/nbNotes;
	tex.y = inst/instCount + 0.5/instCount;	
	return texture2D(notes_velocity,tex).x;
}



vec3 rotate_dir(vec4 q, vec3 dir)
{
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}


vec4 rm(vec3 ro, vec3 rd)
{
	vec3 p = ro;
	float d;
	vec2 dmat;
	for (int i = 0; i < 96; i++)
	{
		dmat = map(p);
		d = distance(ro, p);
		if (abs(dmat.x) < dLimit || d > dFar)
		{
			break;
		}
		p += rd * dmat.x * rd_m;
	}
	return vec4(p, dmat.y);
}

void main()
{
	vec3 color = vec3(1.);
	//vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);
  //vec3 rd = normalize(vec3(uv,2.0));
	vec3 ro = cam_position;
  vec2 uv = vec2(coords.x,coords.y) * 0.5 + vec2(0.5);
  vec2 c = vec2((uv.x + 0.5) * 6.28318530718 , uv.y * 3.14159265359);
  vec3 rd = vec3(sin(c.x) * sin(uv.y), -cos(c.y), cos(c.x) * sin(uv.y));
	rd = rotate_dir(cam_rotation, rd);

	/*snare = max(
					 max(
						max(getNoteVelocity(81.,4.),getNoteVelocity(74.,4.)),
					 getNoteVelocity(62.,4.)),
				getNoteVelocity(86.,4.));*/
  piano = getNoteVelocity(84.0, 3.); 
  
  globals();

  const int numReflexions = 6;
  vec4 pmats[numReflexions]; // everywhere we hit (+ material ID)
  vec3 ns[numReflexions];    // all our normals !

  // First trace the light path
	for (int i = 0; i < numReflexions; i++)
	{
		pmats[i] = rm(ro,rd);
    ns[i] = normal(pmats[i].xyz);

		// reflect ! (put here a condition to break before)
		ro = pmats[i].xyz + ns[i]*4.*dLimit;
		rd = reflect(rd, ns[i]);
  }
 
  for (int i = numReflexions-1; i >=0; i--)
	{
    ro = i > 0 ? pmats[i-1].xyz : cam_position;
    color = shade(pmats[i].xyz, ns[i], ro, pmats[i].w, color).xyz;
  }


	color = pow(color, vec3(1.1/2.2));
	
  //Render_normals
  /*vec3 n = vec3(0.0);
  for (int i = 0; i < numReflexions; ++i)
	  n += ns[i] * 0.5 + vec3(0.5);
  n /= vec3(numReflexions);*/
	//color = color + n * 0.2;

	//Render ambient Occlusion
	float ao = ambientOcclusion(pmats[0].xyz, ns[0]);
	color *= vec3(ao);
  //color += vec3(piano);
	
	//Render distance
	//color = vec3(1.-distance(pmat.xyz,cam_position)*2./dFar);

	gl_FragColor = vec4(color, 1.0);
}
