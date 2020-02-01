

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
uniform sampler2D rbbs;
uniform sampler2D prout;
uniform sampler2D frites;

#define PI 3.14159265359
#define rot(a) mat2(cos(a + PI*0.5*vec4(0,1,3,0)))


//#define frozen


#ifdef frozen

const vec3 sunCol = vec3(0.1, 0.55, 0.9)*9.0;
const vec3 subCol = vec3(0.5, 0.8, 0.9)*5.5;
const float sunRadius = 4.0;
const float sunExponent = -80.0;
const float timeScale = 0.003;
const float timeOffset = 150.0;


#else

const vec3 sunCol = vec3(0.9, 0.55, 0.1)*9.0;
const vec3 subCol = vec3(0.5, 0.8, 0.9)*0.5;
const float sunRadius = 5.0;
const float sunExponent = -100.0;
const float timeScale = 1.0;
const float timeOffset = 0.0;

#endif



vec4 hsv_to_rgb(float h)
{
	float c = 1.;
	h = mod((h * 6.0), 6.0);
	float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
	vec4 color;
 
	if (0.0 <= h && h < 1.0) {
		color = vec4(c, x, 0.0, 1.);
	} else if (1.0 <= h && h < 2.0) {
		color = vec4(x, c, 0.0, 1.);
	} else if (2.0 <= h && h < 3.0) {
		color = vec4(0.0, c, x, 1.);
	} else if (3.0 <= h && h < 4.0) {
		color = vec4(0.0, x, c, 1.);
	} else if (4.0 <= h && h < 5.0) {
		color = vec4(x, 0.0, c, 1.);
	} else if (5.0 <= h && h < 6.0) {
		color = vec4(c, 0.0, x, 1.);
	} else {
		color = vec4(0.0, 0.0, 0.0, 1.);
	}
 
	return color;
}

float time;

vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy + p3.yxx)*p3.zyx);
}

vec3 hash( in vec3 center ) {
    return center + (hash33(center)-0.5) * 0.5;
}

float B2( vec2 _P ) {
    return mod( 2.0*_P.y + _P.x + 1.0, 4.0 );
}

float B8( vec2 _P ) {
    vec2	P1 = mod( _P, 2.0 );					// (P >> 0) & 1
    vec2	P2 = floor( 0.5 * mod( _P, 4.0 ) );		// (P >> 1) & 1
    vec2	P4 = floor( 0.25 * mod( _P, 8.0 ) );	// (P >> 2) & 1
    return 4.0*(4.0*B2(P1) + B2(P2)) + B2(P4);
}

vec3 voronoi( in vec3 uv, in vec3 no, inout float rough ) {
    
    vec3 center = floor(uv) + 0.5;
    vec3 bestCenterOffset = vec3(0);
    float bestDist = 9e9;
    vec3 bestCenterOffset2 = vec3(0);
    float bestDist2 = 9e9;
    
    for (float x = -0.5 ; x < 1.0 ; x+=1.0)
    for (float y = -0.5 ; y < 1.0 ; y+=1.0)
    for (float z = -0.5 ; z < 1.0 ; z+=1.0) {
		vec3 offset = vec3(x, y, z);
        vec3 newCenter = center + offset;
        vec3 newCenterOffset = hash(newCenter);
        vec3 temp = newCenterOffset - uv;
        float distSq = dot(temp, temp);
        if (distSq < bestDist) {
    		bestCenterOffset2 = bestCenterOffset;
    		bestDist2 = bestDist;
            bestCenterOffset = newCenterOffset;
            bestDist = distSq;
        } else if (distSq < bestDist2) {
            bestCenterOffset2 = newCenterOffset;
            bestDist2 = distSq;
        }
    }
    
    vec3 dx = abs(dFdx(uv));
    vec3 dy = abs(dFdy(uv));
    float aa = max(length(dx), length(dy));
    float distFact = max(0.0, 1.0 - aa / 2.0);
    distFact *= distFact;
    vec3 n1 = normalize(no + (hash33(bestCenterOffset)-0.5)*1.5*distFact);
    vec3 n2 = normalize(no + (hash33(bestCenterOffset2)-0.5)*1.5*distFact);
    float d = (sqrt(bestDist)-sqrt(bestDist2));
    float aad = fwidth(d);
    return mix(n1, n2, smoothstep(-aad, +aad, d*2.0));
    
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdEllipsoid( in vec3 p, in vec3 r ) {
    float k0 = length(p/r);
    float k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
}

float smin( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

float smax( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h); 
}

float cross4D(vec4 p) {

    p.yz *= rot(0.258*time);
    
    float a = sdBox(p.xyz, vec3(0.03, 0.03, 0.2));
    a += sin(10.0*p.x)*sin(14.0*p.y)*sin(12.0*p.z)*sin(18.0*p.w)*0.03;
    p.xy *= rot(0.4782*time);
    float b = sdBox(p.wyx, vec3(0.03, 0.03, 0.3));
    b += sin(10.0*p.x)*sin(14.0*p.y)*sin(12.0*p.z)*sin(18.0*p.w)*0.03;
    p.wz *= rot(0.3478*time);
    float c = sdBox(p.zwy, vec3(0.03, 0.03, 0.4));
    c += sin(10.0*p.x)*sin(14.0*p.y)*sin(12.0*p.z)*sin(18.0*p.w)*0.03;
    
    return smin(smin(a, b, 0.05), c, 0.05);
}

float de4d(vec4 p) {
    
    p.zw *= rot(time*0.148);
    p.xz *= rot(time*0.241);
    p.wy *= rot(time*0.187);
    p.x += time*3.687;
   	p.y -= time*0.37454;
    vec4 inG = (fract(p) - 0.5) * (mod(floor(p), vec4(2)) * 2.0 - 1.0);
    return cross4D(inG);
}

float de(vec3 p) {
    
    //vec4 pp = vec4(p, length(p));
    //float d = dot(pp, pp);
    //d = de4d(pp/d)*d;
    
    float d = dot(p, p);
    d = de4d(vec4(p/d, d))*d;
    float skyDome = 1.5-length(p);
    d = min(d, skyDome);
    return d;
    
}

vec3 getNormal(vec3 p) {
	vec3 e = vec3(0.0, 0.001, 0.0);
	return normalize(vec3(
		de(p+e.yxx)-de(p-e.yxx),
		de(p+e.xyx)-de(p-e.xyx),
		de(p+e.xxy)-de(p-e.xxy)));	
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
    vec3 halfwayDir = normalize(viewDir + lightDir);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    float NDF = DistributionGGX(normal, halfwayDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);
    float NdotL = max(dot(normal, lightDir), 0.0);
    result += (kD * albedo / PI + specular) * radiance * NdotL;
    return result;
}

// PBR WORKFLOW ABOVE

// fake subsurface scattering
vec3 computeSSS(in vec3 normal, in vec3 viewDir, 
                in vec3 albedo, in float trans, in float index,
                in vec3 lightDir, in vec3 radiance) {
    float add = 1.0 - index;
    add *= add;
    add *= add;
    add *= add;
    add *= add;
    float fr = dot(viewDir, normal)*0.5+0.5;
    float lu = dot(viewDir, lightDir)*-0.5+0.5;
    add *= fr*fr;
    add *= lu;
    return radiance*add*1.0*trans*albedo;
}

// approximation of the error function
float erf( in float x ) {
    //return tanh(1.202760580 * x);
	float sign_x = sign(x);
	float t = 1.0/(1.0 + 0.47047*abs(x));
	float result = 1.0 - t*(0.3480242 + t*(-0.0958798 + t*0.7478556))*exp(-(x*x));
	return result * sign_x;
}

float getIntegral(vec3 start, vec3 dir, float dist) {
    
    const float a = sunExponent;
	const float b = sunRadius;
    
    float k = start.x;
    float l = dir.x;
    float m = start.y;
    float n = dir.y;
    float o = start.z;
    float p = dir.z;

    float res = sqrt(PI);
    res *= exp(b+a*(+k*k*(n*n+p*p)
                    -m*m*(-1.0+n*n)
                    -o*o*(-1.0+p*p)
                    -2.0*k*l*o*p
                    -2.0*m*n*(k*l+o*p) ));
    res *= - erf(sqrt(-a)*dot(start, dir)) + erf(sqrt(-a)*(dot(start, dir)+dist));
    res /= 2.0 * sqrt(-a);
    
    res *= 500.0;
    
    return res;
    
}

vec3 rotate_dir(vec4 q, vec3 dir)
{
	vec3 v = dir.xyz;
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}
void main(){
    
    float bay = B8(floor((coords * 2.0 - 1.0) * vec2(1920.,1080.)))/64.0;
    time = ((iTime-timeOffset/timeScale) + (1.0/120.0)*bay)*timeScale;
    	vec2 uv = vec2(coords.x*xy_scale_factor,coords.y);
	
    
    
	vec3 from = cam_position;
	vec3 dir = rotate_dir(cam_rotation, normalize(vec3(uv, 2.0 * cam_fov)));
	
    
    
    
    float totdist = 0.0;
    totdist += 0.1+pow(bay, 0.2)*0.4;
	totdist += de(from+dir*totdist)*bay;
    
    float ao = 0.0;
	for (int steps = 0 ; steps < 150 ; steps++) {
		vec3 p = from + totdist * dir;
        float dist = de(p);
		totdist += dist*0.7;
		if (dist < 0.001 || length(p) > 1.4) {
            ao = float(steps)/150.0;
            break;
		}
	}
    
    vec3 result = vec3(0);
    vec3 p = from + totdist * dir;
    
    if (length(p) < 1.4) {
        vec3 n = -getNormal(p);

        vec3 sunDir = normalize(p);
        const vec3 subDir = normalize(vec3(2, -7, 3));

        float rough = 0.0;
        vec3 vor = voronoi(p*600.0, n, rough);
        n = normalize(n+vor*0.5);

        vec4 albedo = vec4(0.98, 0.8, 0.5, 0.4);

        result += computeLighting(n, dir, albedo.rgb, 0.01, albedo.a, sunDir, sunCol);
        result += computeLighting(n, dir, albedo.rgb, 0.01, albedo.a, subDir, subCol);
        result += computeSSS(n, dir, albedo.rgb, albedo.a, ao, sunDir, sunCol);
        result += computeSSS(n, dir, albedo.rgb, albedo.a, ao, subDir, subCol);
    
    } else {
        
        // background
        float rough = 0.0;
        vec3 vor = voronoi(dir*800.0, vec3(0), rough);
        result = vec3(pow(abs(vor.x), 500.0))*3.0;
        
    }
    vec4 fragColor;
    fragColor.rgb = result;
    float fog = getIntegral(from, dir, totdist);
    fragColor.rgb += fog*0.01*sunCol;
    fragColor.rgb = pow(fragColor.rgb*0.6, vec3(1.0/2.2));
vec2 puv = uv;
puv.y += sin(track_time + uv.x);
if ( track_time > 120. && abs(puv.x) < 1.25 && abs(puv.y) < 0.5) {
vec2 fuv = puv * vec2(.4,1.) + vec2(.50,0.5);

fragColor.rgb = mix(fragColor.rgb, 1. - fragColor.rgb, texture2D( frites, fuv).r * smoothstep(144.,142.,track_time));
}
gl_FragColor = fragColor;

}