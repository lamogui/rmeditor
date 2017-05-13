
vec3 lpos1 = vec3(.0,-1.5,.0);
vec3 lpos2 = vec3(2.,-0.5,2.);
vec3 lpos3 = vec3(-2.,-.5,-2.);
/*
float hel_l = 0.;
float rot_d;
float ball_r;
float pale_l = 0.;
float rot_h = PI/2.;
float rotate_sand_time=0.;
*/

bool lightOn = false;

void globals()
{
	if (track_time > 13.)
		lightOn = true;
}

vec2 map(vec3 p)
{
	vec3 pr = p;
	pr -= vec3(.0,-1.,.0);
  //pR(pr.xz,rot_h);
	//pModPolar(pr.xz,1. + rot_d);
	vec3 prt = pr-vec3(.0,-.25,.0);
	//vec2 dmat = vec2(fBoxCheap(pr,vec3(0.3)),MAT_MIRROR);
	//
	vec2 dmat = vec2(fPlane(p,vec3(0.,-0,-1.0),2.), MAT_MIRROR);
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(0.,0.,1.),5.));
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(-1.,-0.,0.),5.));
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(1.,-0.,0.),5.));
  dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(0.,-1.,0.),5.));
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(0.,1.,0.),2.));
	//Table
	dmat = mmin(dmat,MAT_BLACKMIRROR,
		fOpUnionStairs(
			fBoxCheap(p - vec3(.0,-1.5,.0),vec3(1.,0.05,.8)),
			fBoxCheap(p - vec3(.0,-1.8,.0),vec3(0.8,0.15,.5)),.2,3.));


	//float fake_d = fPlane(p,vec3(0.,1.,0.),2.);
	//vec2 s = 0.8*p.xz;pR(s,rotate_sand_time);
	//dmat = fmin(dmat,MAT_SAND,fake_d, p.y + 2.0 +  0.2 * terrainSand(s,2));

	return dmat;
}

vec4 shade(vec3 p, vec3 n, vec3 ro, vec3 rd, float mat)
{
	float fog = pow(min( length(p-ro)/(dFar*0.9), 1.),200.);
	float r = 0.;
	vec3 col = vec3(0.);

	vec3 ldir1 = normalize(lpos1-p);	
	vec3 ldir2 =  normalize(lpos2-p);	
	vec3 ldir3 =  normalize(lpos3-p);	
	if (!lightOn)
	ldir1 = vec3(0.,0.,1.);
	float latt1 = pow( length(lpos1.xy-p.xy)*(.7-piano*.3), 1.5 );
	float latt2 = pow( length(lpos2-p)*.75, 1. );
	float latt3 = pow( length(lpos3-p)*.75, 2.5 );

	

	vec3 diff1 = vec3(1.,1.,1.) * (max(dot(n,ldir1),0.) ) / latt1;
	vec3 diff2 = vec3(1.,1.,1.) * (max(dot(n,ldir2),0.) ) / latt2;
	vec3 diff3 = vec3(1.,1.,1.) * (max(dot(n,ldir3),0.) ) / latt3;

  float da = 1.;
	float ao = 1.;

	if (mat==MAT_MIRROR)
	{
		r = 1.;
		col= vec3(0.8);
		da = 0.9;
	}
	else if (mat== MAT_BLACKMIRROR)
	{
		col= vec3(0.3,0.3,0.3);
		r = .9;
		da = .5;
	}

  col =  col * (diff1 + diff2 + diff3) * ao *da;

	vec3 skyCol = vec3(1.);
	
	
	col = mix(col, skyCol, fog );
	
	return vec4(col,r);
}
