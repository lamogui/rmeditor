
vec3 lpos1 = vec3(.0,-1.5,.0);
vec3 lpos2 = vec3(2.,-0.5,2.);
vec3 lpos3 = vec3(-2.,-.5,-2.);


bool lightOn = false;

void globals()
{

	if (track_time > 13.)
		lightOn = true;
}

float table(vec3 p)
{
  return fOpUnionStairs(
			fBoxCheap(p - vec3(.0,-1.5,.0),vec3(1.,0.05,.8)),
			fBoxCheap(p - vec3(.0,-1.8,.0),vec3(0.8,0.15,.5)),.2,3.);
}

vec3 light_rm(vec3 ro, vec3 rd)
{
	vec3 p = ro;
	float dFromRO;
  float d;
	for (int i = 0; i < 96; i++)
	{
		d = table(p);
		dFromRO = distance(ro, p);
		if (abs(d) < dLimit || dFromRO > dFar)
		{
			break;
		}
		p += rd * d * rd_m;
	}
	return p;
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
 	vec2 dmat = vec2(fPlane(p,vec3(0.,-0.,-1.0),2.), MAT_MIRROR);
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(0.,0.,1.),5.));
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(-1.,-0.,0.),5.));
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(1.,-0.,0.),5.));
  dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(0.,-1.,0.),5.));
	dmat = mmin(dmat,MAT_MIRROR,fPlane(p,vec3(0.,1.,0.),2.));
  //vec2 dmat = vec2(fPlane(p,vec3(0.,1.,0.),2.),MAT_MIRROR);
	//Table
	dmat = mmin(dmat,MAT_BLACKMIRROR, table(p));


	//float fake_d = fPlane(p,vec3(0.,1.,0.),2.);
	//vec2 s = 0.8*p.xz;pR(s,rotate_sand_time);
	//dmat = fmin(dmat,MAT_SAND,fake_d, p.y + 2.0 +  0.2 * terrainSand(s,2));

	return dmat;
}

vec3 shade(vec3 p, vec3 n, vec3 ro, float mat, vec3 inputColor)
{
	float fog = pow(min( length(p-ro)/(dFar*0.9), 1.),200.);
	float r = 0.;
	vec3 col = vec3(0.);

  float l2Shadow = 1.0;

	vec3 ldir1 = normalize(lpos1-p);	
	vec3 ldir2 = normalize(lpos2-p);	
	vec3 ldir3 =  normalize(lpos3-p);	
	if (!lightOn)
	  ldir1 = vec3(0.,0.,1.);

  vec3 lp = light_rm(lpos2, -ldir2);
  float distanceFromP = distance(p, lpos2);
  float distanceFromObstacle = distance(lp, lpos2);
  if (distanceFromP + dLimit> distanceFromObstacle)
		l2Shadow = 0.1;

	float latt1 = pow( length(lpos1.xy-p.xy)*(.7-piano*.3), 1.5 );
	float latt2 = pow( length(lpos2-p)*.75, 1. );
	float latt3 = pow( length(lpos3-p)*.75, 2.5 );


	vec3 diff1 = (0.5 * l2Shadow + 0.5) * vec3(1.,1.,1.) * (max(dot(n,ldir1),0.) ) / latt1;
	vec3 diff2 = l2Shadow * vec3(1.,1.,1.) * (max(dot(n,ldir2),0.) ) / latt2;
	vec3 diff3 = vec3(1.,1.,1.) * (max(dot(n,ldir3),0.) ) / latt3;

	float ao = 1.;
	if (mat==MAT_MIRROR)
	{
		r = 0.98;
		col= vec3(0.8);
	}
	else if (mat== MAT_BLACKMIRROR)
	{
		col= vec3(0.3,0.3,0.3);
		r = .87;
	}

  col =  mix(col, inputColor, r) * (diff1 + diff2 + diff3) * ao;

	vec3 skyCol = vec3(0.0, 0.3, 0.3);
	
	col = mix(col, skyCol, fog );
	return col;//vec4(col,r);
}
