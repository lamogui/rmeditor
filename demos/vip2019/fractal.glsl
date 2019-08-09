#version 120
#define PI 3.14159265
#define RES vec2(1920, 1080)

uniform sampler2D max_notes_velocity;
varying vec2 coords;
uniform float xy_scale_factor;
uniform float sequence_time;
uniform float track_time;
uniform vec3 cam_position;
uniform vec4 cam_rotation;

int sequenceID;
void setSequenceID();
float bunnyRot = 0.0;
float bunnyRot2 = 0.0;
float beat = 1.0;
float glitch = 0.0;
float boost = 0.0;

vec3 sort(vec3 n) {
  float a = min(min(n.x, n.y), n.z);
  float b = max(max(n.x, n.y), n.z);
  return vec3(a, n.x + n.y + n.z - a - b, b);
}

float smin( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

float smax( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h);
}

mat2 rot(float t) {
	float c = cos(t);
	float s = sin(t);
	return mat2(c, s, -s, c);
}


vec3 rotate_dir(vec4 q, vec3 dir) {
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

float fOpUnionStairs(float a, float b, float r, float n) {
	float s = r/n;
	float u = b-r;
	return min(min(a,b), 0.5 * (u + a + abs ((mod (u - a + s, 2. * s)) - s)));
}

float sdbox(vec2 p, vec2 b) {
	vec2 d = abs(p) - b;
	return max(d.x, d.y);
}

float sdbox( vec3 p, vec3 b ) {
  vec3 d = abs(p) - b;
  return length(max(d,0.0)) + min(max(d.x,max(d.y,d.z)),0.0);
}

float sdEllipsoid( in vec3 p, in vec3 r ) {
    float k0 = length(p/r);
    float k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
}

float dist(vec2 a, vec2 b) {
   	vec2 d = a-b;
    return dot(d,d);
}

vec2 noise(in vec2 c, in float seed) {
    return c + 0.7*
        sin(seed+
            c.xy*vec2(13.2,18.3) +
            c.yx*vec2(52.6,25.6));
}

float worley(in vec2 uv, float seed) {
    vec2 center = floor(uv)+0.5;
    vec2 bestCenter = noise(center, seed);
    float bestLen = dist(uv, bestCenter);
    for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++) {
        vec2 offset = vec2(x, y);
        vec2 newCenter = noise(center+offset, seed);
        float d = dist(uv, newCenter);
        if (d < bestLen) {
            bestCenter = newCenter;
            bestLen = d;
        }
    }
    
    return bestLen;
}

vec4 myTexture(in vec2 uv) {
    
    float d = 0.0;
    
    vec2 a = uv;
   	a.x *= 3.0;
    a *= rot(0.3);
    
    d += worley(a, 0.3);
    
    vec2 b = a;
    b.x += b.y*0.1;
    b*= rot(0.4);
    
    float dd =  worley(b, 12.3332);
    d *=dd*6.0;
    d+= dd;
    
    vec2 z = b;
    z += b.x*0.3;
    	
    float pls = 0.0;
    pls+=length(z);
    pls+=b.x;
    pls+=sin(a.y*3.0);
    
    float c = sin(pls*1.0)*0.5+0.5;
    float cc = (c+d)*0.3;

    vec4 color = vec4(mix(vec3(0.62, 0.6, 0.65), vec3(0.5, 0.5, 0.56), cc), cc);
    
    color.rgb += d*0.1;
    
    return color;
    
}

// PBR WORKFLOW BELOW

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 computeLighting(in vec3 normal, in vec3 viewDir,
                     in vec3 albedo, in float metallic, in float roughness,
                     in vec3 lightDir, in vec3 radiance) {
    
    vec3 result = vec3(0);
    
    // find half way vector
    vec3 halfwayDir = normalize(viewDir + lightDir);
    
    // figure out surface reflection
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // find the PBR terms
    float NDF = DistributionGGX(normal, halfwayDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    // Cook Torrance BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);  
    
    // add light contribution
    float NdotL = max(dot(normal, lightDir), 0.0);
		
		if (sequenceID >= 5) {
			NdotL = dot(normal, lightDir)*0.5+0.5;
			NdotL *= NdotL;
		}
		

    result += (kD * albedo / PI + specular) * radiance * NdotL;
    
    return result;
}

// PBR WORKFLOW ABOVE

#define BS 2.5

vec2 mapBunny(vec3 p, inout vec3 glow, inout vec3 lightColor) {
	
	p /= BS;

	for (int i = 0 ; i < 4 ; i++) {
		p.xy *= rot(bunnyRot*0.43);
		p.yz *= rot(bunnyRot*0.32);
		p.xz *= rot(bunnyRot*0.24);
		p.x = abs(p.x);
	}

	p.x = abs(p.x);
	vec3 inTee = p;
	p.xy *= rot(0.1);

	vec3 inEye = p - vec3(0.3, 0.3, -0.1);

	float eye = length(inEye)-0.2;

	vec3 color = vec3(1, 0.2, 0.2);
	glow += exp(-eye*40.0)*color;
	lightColor += exp(-eye*0.3)*color;
	//p = mod(p, vec3(4.0))-2.0;

	float head = sdEllipsoid(p, vec3(0.6, 0.6, 0.8));
	float nose = sdEllipsoid(p-vec3(0.1, 0, -0.3), vec3(0.2, 0.3, 0.6));

	vec3 pNose = p;
	pNose.y = abs(pNose.y)-1.0;
	pNose.yz *= rot(-0.3);
	float mNose = sdEllipsoid(pNose-vec3(0, -0.5, -0.65), vec3(0.3, 0.2, 0.4));

	head = smin(head, nose, 0.2);
	head = smin(head, mNose, 0.05);
	head = smin(head, eye, 0.1);
	head = smax(head, -eye, 0.08);

	inTee = inTee + vec3(-0.04, 0.1, 0.88);
	inTee.yz *= rot(0.3);
	float teeth = sdbox(inTee, vec3(0.02, 0.1, 0.0))-0.02;

	head = min(head, teeth);

	vec3 inOre = p;
	inOre.yz *= rot(0.4);
	inOre.xy *= rot(-0.2);
	inOre.xz *= rot(0.3);
	vec3 dim = vec3(0.1, 0.8, 0.0);
	inOre = inOre + vec3(-0.35, -0.9, -0.3);
	dim.x += cos(inOre.y*3.0)*0.1;
	float ore = sdbox(inOre, dim)-0.1;
	float cre = sdEllipsoid(inOre + vec3(0.0, 0.0, 0.15), vec3(0.2, 0.7, 0.1));

	ore = smax(ore, -cre, 0.01);
	head = smin(head, ore, 0.2);

	return vec2(head, eye)*BS;
}

vec2 map(vec3 p, inout vec3 glow, inout vec3 lightColor,
		out vec3 inside, out float bunnyF) {

	if (sequenceID == 0) {
		p.xz *= rot(track_time*0.2);
	}

	vec3 tempP = p;

	float skybox = 200.0-length(p);
	float len = skybox;

	vec2 bunny = vec2(9e9);
	if (sequenceID > 4) {
		bunny = mapBunny(p, glow, lightColor);
	}

	int seqMod = int(mod(float(sequenceID), 2.0));

	float center = 3.0 - length(p);

	float q = sin(float(sequenceID)*8.0)*1.0;
	if (sequenceID == 6) {
		q += sequence_time*0.03;
		len += sequence_time*10.0;	
	}
	float t = length(p.xy)-0.03;

	float factor = 1.1;



	if (sequenceID == 3) {
		q = 3.8;
		factor = 0.8;
	} else if (sequenceID == 5) {
		q = 4.0 + bunnyRot2*0.01;
		factor = 1.1;
		
	} else if (sequenceID == 2) {
		q -= sequence_time*0.2 - 1.0;
	} else if (sequenceID == 4) {
		q += sequence_time*0.02;
	}
	
	float dglow = 9e9;
	float s = 1.0;
	int levels = 1 + (sequenceID+1)/2;
	for (int i = 0 ; i < levels ; i++) {

		float m = dot(p, p)*factor;
		p = abs(fract(p/m)-0.5);

		p.xy *= rot(q);
		s *= m;

		// add glow

		vec3 pp = p.xzy;
		if (seqMod==1) pp = p.xyz;

		float dd = (length(pp.xy)-0.02)*s;
			
		float xf = float(i+1)*0.2+len*1.0+float(sequenceID)+sequence_time*0.1+pp.z*0.1;
		xf += (pp.z+track_time)*boost;	

float be = texture2D(max_notes_velocity, vec2(0.5, xf)).r;
		be *= be;
		be = be*0.75+0.25;

		dd = max(dd, -bunny.x+2.5);
		vec3 color = sin(boost*sequence_time+be*0.3+float(i+1)*vec3(6, 2, 3)+beat+len*0.1+float(sequenceID))*0.5+0.5;
	
		glow += exp(-dd*30.0+0.5)*color*s*(beat)*be;
		lightColor += exp(-dd*0.1)*color*beat;
		dglow = min(dglow, dd);
	}

	inside = p;

	// add structure
	float d = sdbox(p.yz, vec2(0.03, 0.2))*s;

	vec3 inBox = p;
	inBox.x = mod(inBox.x, 0.2)-0.1;
	
	float sph = length(inBox)-0.05;

	d = min(d, sph);
	
	d = max(d, -bunny.x+3.5);
	
	d = min(d, skybox);

	if (bunny.x < 1.0) {
		inside = tempP*0.1;
	}

bunnyF = smoothstep(1.0, 0.0, bunny.x);

	return min(bunny, vec2(d, dglow));

}

vec2 map(vec3 p) {
	vec3 dum = vec3(0);
	float f = 0.0;
	return map(p, dum, dum, dum, f);
}

vec3 normal( vec3 p, const int n ) {
	vec3 dum = vec3(0);
	vec3 eps = vec3(0.01, 0.0, 0.0);
	return normalize( vec3(
		map(p+eps.xyy)[n]-map(p-eps.xyy)[n],
		map(p+eps.yxy)[n]-map(p-eps.yxy)[n],
		map(p+eps.yyx)[n]-map(p-eps.yyx)[n]
	) );
}

vec3 trace(in vec3 from, in vec3 dir, out vec3 glow, const int n) {
	vec3 dum = vec3(0);
	float f = 0.0;
	float totdist = 0.0;
	for (int i = 0 ; i < n ; i++) {
		vec3 p = from + dir*totdist;
		vec2 here = map(p, glow, dum, dum, f);
		float d = min(here.x, here.y);
		totdist += d*0.6;
		if (d < 0.00001) {
			break;
		}
	}
	return from + dir*totdist;
}

vec3 shade(in vec3 p, in vec3 dir) {
	vec3 dum = vec3(0);
	vec3 lightColor = vec3(0);
	vec3 inside = vec3(0);
	float bunnyf = 0.0;
	vec2 here = map(p, dum, lightColor, inside, bunnyf);
	vec4 tex = myTexture(inside.zx*10.0);
	vec3 emi = vec3(0);
	emi += step(here.y, 0.0001)*20.0;
	vec3 n = normal(p, 0);
	vec3 lightDir = -normal(p, 1);
	
	float po = 3.0+tex.a*9.0;

	float d = pow(max(0.0, dot(reflect(dir, n), lightDir)), po);
	
	vec3 pbr = computeLighting(n, -dir, tex.rgb, 0.3, tex.a, lightDir, lightColor*20.0);

	return pbr + emi;


}

vec3 applyFog(in vec3 color, in float totdist) {
	return mix(vec3(0), color, exp(-totdist*0.5));
}

vec3 shadeRefl(in vec3 p, in vec3 dir) {
	
	float bunnyF = 0.0;
	vec3 dum = vec3(0);
	vec3 inside = vec3(0);
	map(p, dum, dum, inside, bunnyF);
	vec4 tex = myTexture(inside.zx*10.0);




	vec3 n = normal(p, 0);
	
	vec3 dirRefl = reflect(dir, n);
	vec3 fromRefl = p+dirRefl*0.001;
	vec3 glow = vec3(0);
	vec3 pRefl = trace(fromRefl, dirRefl, glow, 100);

	vec3 here = shade(p, dir);
	vec3 refl = shade(pRefl, dirRefl);
	
	refl = applyFog(refl, distance(fromRefl, pRefl));
	refl += glow;
	float fres = max(0.0, dot(dir, -n)*(1.0-tex.a));

	return mix(refl, here, mix(fres*fres, 0.75, bunnyF));

}

vec3 hash33(vec3 p3) {
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+19.19);
    return fract((p3.xxy + p3.yxx)*p3.zyx);
}

vec3 postProcess(vec3 col, vec2 fragCoord) {
    
    vec2 del = coords.xy;
    col /= exp(dot(del, del)*2.0);

		col *= mix(1.0, sin(coords.y*400.0)*0.5+0.5, glitch);
    
    col *= 1.0;
    col = col/(1.0+col);
    col = pow(col, vec3(1.0/2.2));
    
    vec3 rnd = hash33(vec3(fragCoord, track_time*60.0))-0.5;
    col += rnd*0.01;
    
    return col;
}

void main() {
	
	setSequenceID();

	float bpm = 60.0/85.0;

	glitch = pow(1.0-fract((sequence_time+bpm*0.5)/bpm), 4.0);

	int seqMod = int(mod(float(sequenceID), 2.0));
if ((seqMod == 0 || sequenceID > 4) || track_time < bpm * 16.0) {
	glitch = 0.0;
}

	beat = texture2D(max_notes_velocity, vec2(0.5, 0.4)).x;
	beat *= beat;
	beat = beat*0.5+0.5;

	boost = smoothstep(67.60, 69.9, track_time);


	vec2 fragCoord = coords.xy*0.5+0.5*RES;

	bunnyRot = smax(0.0, -track_time+67.- 4., 5.0);
	bunnyRot2 = smax(0.0, track_time-76.6, 3.0);
	
	vec2 co = coords.xy;

	co.x += sin(co.y*70.0+track_time*20.0)*glitch*0.01;
	co.x += sin(co.y*20.0+track_time*10.0)*glitch*0.02;

	gl_FragColor = vec4(0.0);
	vec2 uv = co;
	uv.x *= xy_scale_factor;

	vec3 from = cam_position;
	vec3 dir = normalize(vec3(uv, 1));
	dir = rotate_dir(cam_rotation, dir);

	vec3 dum = vec3(0);
	vec3 glow = vec3(0);
	vec3 p = trace(from, dir, glow, 100);
	float totdist = distance(from, p);
	
	gl_FragColor.rgb = shadeRefl(p, dir);

	gl_FragColor.rgb = applyFog(gl_FragColor.rgb, totdist);
	gl_FragColor.rgb += glow;
	
	
	gl_FragColor.rgb = postProcess(gl_FragColor.rgb, fragCoord);

	gl_FragColor.rgb *= smoothstep(0.0, 3.0, track_time);
	gl_FragColor.rgb *= smoothstep(140.0, 130.0, track_time);
  gl_FragColor.a = 1.0;

}