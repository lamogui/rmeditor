float hole_r = 0.;
float cur_i = 6.;
float pp_y = .9;

void globals()
{
	if (track_time > 118.)
	{
		hole_r = 3.;
	}
	else if (track_time > 94.5)
	{
		hole_r = 1.5 + 1.5*( sin(track_time-58.5) + track_time - 94.5)/(118.-94.5); 
	}
	else if (track_time > 87.5)
	{
		hole_r = 1.5;
	}
	else if (track_time > 79.)
	{
		hole_r = 1.5*(track_time - 79.)/(87.5-79.); 
	}

	if (track_time > 117.5)
	{
		cur_i = 6.;
	}
	else if (track_time > 99.)
	{
		cur_i = 6.;
	}
	
	if  (track_time > 93. && track_time < 99.)
	{
		rd_m = 0.3;
	}
	else if (track_time > 113.)
	{
		rd_m = 0.4;
	}
	if  (track_time > 120.)
 {
		pp_y = (-track_time+120.9)*2.;; 
	}
	

}

float pyramid( vec3 p, float h) {
	vec3 q=abs(p);
	return max(-p.y, (q.x+q.y+q.z-h)/3.0 );
}

float helice(vec3 p)
{
	vec3 pr = p;
	pr *= -1.;
	pr -= vec3(.0,-1.4,.0);
  pR(pr.xz,3.*(track_time + sin(track_time-58.5)));
	pModPolar(pr.xz,5.);
	vec3 prt = pr-vec3(.0,-.25,.0);
	float d = fBoxCheap(pr,vec3(0.3));
	d = max(d, length(prt) - .4);
	d = min(d,pale(prt,0.2,2.));
	return d;
}

float noteV(vec2 p)
{
	float a = mod(floor(0.5+ (((PI + atan(p.y, p.x))/(2.*PI)) * (64.))),12.) + 32.;
	float v = snare*0.4;
	for (int i =0; i < 4; i++)
	{
		v = max(v, getNoteVelocity(a,cur_i));
		a = a+12.;
	}
	return v;
}

vec2 map(vec3 p)
{
  p.y += 3.0;
  vec2 s = vec2(1.) - 0.3*p.xz;//pR(s,rotate_sand_time);
	float b_d = length(p) - hole_r;
  if (b_d > 0.) b_d = 0.;
	vec2 dmat = vec2(p.y - b_d - 0.3 * terrainSand(s,2), MAT_SAND);
	vec3 pp = p; pp.y += pp_y;
  vec3 ppp = pp; ppp.y -= 1.2; pR(ppp.xz,0.78); ppp.x += 0.8; //MAT_PYRAMID
	dmat = mmin(dmat,MAT_PYRAMID,max(max(pyramid(pp,2.), -fBoxCheap(pp,vec3(.98,.98,.98))), -fBoxCheap(ppp,vec3(0.5,.12,.1))));
	dmat = mmin(dmat, MAT_BLACKMIRROR, helice(pp*2.5));

	vec3 c = pp; pModPolar(c.xz,64.); c.x -= 3.; c.y -=0.5+pp_y-0.9;
	dmat = mmin(dmat, MAT_MIRROR, fBoxCheap(c,vec3(0.02,1. + 0.2*noteV(pp.xz),0.1)));


	return dmat;
}

vec4 shade(vec3 p, vec3 n, vec3 ro, vec3 rd, float mat)
{
	float fog = pow(min( length(p-ro)/(dFar*0.5), 1.),200.);
	float r = 0.;
	vec3 col = vec3(0.);

	vec3 ldir1 = normalize(vec3(0., 15.,0.)-p);	

	vec3 diff1 = vec3(1.,1.,1.) * (max(dot(n,ldir1),0.) );

  float da = 1.;
	float ao = 1.;
	ao = ambientOcclusion(p,n);
	if (mat==MAT_MIRROR)
	{
		r = 1.;
		col= vec3(0.7, 0.6, 0.3);
		da = 3. + 5.*noteV(p.xz);
	}
	else if (mat == MAT_PYRAMID)
	{
		col= vec3(1.,1.,0.5);
		
	} 
	else if (mat== MAT_SAND)
	{
		col= mix( vec3(0.3, 0.2, 0.0), vec3(0.7, 0.6, 0.3), n.y );
		//r = 1.;
	}
	else if (mat== MAT_BLACKMIRROR)
	{
		r = 1.;
		col= vec3(1.,1.,0.5);
		da = .3;
	}

  col =  col * (diff1) * ao *da;

	vec3 skyCol = vec3(.7,0.7,1.);
	
	
	col = mix(col, skyCol, fog );
	
	return vec4(col,r);
}
