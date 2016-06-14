
varying vec2 coords;
uniform float sequence_time;
uniform float track_time;
uniform float noise_lvl;
uniform float bass;
uniform vec3 cam_position;
uniform vec4 cam_rotation;

vec3 rotate_dir(vec4 q, vec3 dir)
{
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

const int rays_hits=9;
const float epsilon=0.001;
vec3 vertical=vec3(0.0,1.0,0.0);

vec3 sphere1=vec3(0.0,0.0+noise_lvl,-2.0);
vec3 sphere2=vec3(0.0,sin(track_time),0.0);
vec3 sphere3=vec3(1.0,1.0-bass*0.8,1.0);

vec3 back(vec3 rd)
{
	float s=pow(dot(rd,vertical),0.68);//Sky
	float f=dot(rd,-vertical);//floor
	vec3 sc=s*vec3(66.0/255.0,111.0/255.0,169.0/255.0);
	vec3 fc=f*vec3(185./255.,122./255.,87./255.);
	return max(sc,fc);
}

float sphere(vec3 ray, vec3 dir, vec3 center, float radius, float closed_hit, out vec3 normal)
{
  vec3 rc = ray-center;
  float c = dot(rc, rc) - (radius*radius);
  float b = dot(dir, rc);
  float d = b*b - c;
  float t = -b - sqrt(abs(d));
  if (d < 0.0 || t < 0.0 || t > closed_hit) 
	return closed_hit;
  else {
    normal = center-(ray+dir*t);
	return t;
 }
 
}

float scene(vec3 ro, vec3 rd, out vec3 normal, float max_dist)
{
	float dist=max_dist;
	dist=sphere(ro,rd,sphere1,0.7,dist,normal);
	dist=sphere(ro,rd,sphere2,0.5,dist,normal);
	dist=sphere(ro,rd,sphere3,0.88,dist,normal);
	return dist;
}


mat2 rotate2D(float a)
{
	return mat2(cos(a), -sin(a),
				sin(a),  cos(a));
}

vec2 rotateY(vec2 v, float a)
{
	return v*rotate2D(a);
}

void main()
{
	vec3 light = vec3(0.0);    // How much light hits the eye through the ray.
	vec3 transmit = vec3(1.0); // How much light the ray lets through.
	vec3 ro=cam_position;
	vec3 rd=normalize(vec3(coords,1.0));    //direction 
	rd = rotate_dir(cam_rotation,rd);
	for (int r=0;r < rays_hits; r++)
	{
		vec3 normal;
		float max_dist=1000.0;
		float dist=scene(ro,rd,normal,max_dist);
		if (dist < max_dist)
		{
			//Here modify ray transmition with transmit
			transmit*=0.9;
			ro += rd*dist;
			rd =reflect(rd,normalize(normal));//Here modify ray direction (mirror exemple)
			ro += rd*epsilon; //force undo collision
		}
		else { //Hit infinite (or default object)
			light += transmit*back(rd); //here background fonction
			break;
		}
	}
	gl_FragColor = vec4(light, 1.0); // Set pixel color to the amount of light seen.
}
