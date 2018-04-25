varying  vec2 texCoord;
varying  vec4 color;
varying  vec3 fN;
varying  vec3 fV;
varying  vec3 fL_p;
varying  vec3 fL_d;

uniform sampler2D texture;
uniform int BaseOrJointFlag;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float Shininess;

float Ks_p, Ks_d;

void main() 
{
    // Normalize the input lighting vectors
    vec3 N = normalize(fN);
    vec3 V = normalize(fV);
    vec3 L_p = normalize(fL_p);
    vec3 L_d = normalize(fL_d);

    // Calculate the halfway vectors
    vec3 H_p = normalize( L_p + V );
    vec3 H_d = normalize( L_d + V );

    // Calculate specular coefficients
    Ks_p = pow(max(dot(N, H_p), 0.0), Shininess);
    Ks_d = pow(max(dot(N, H_d), 0.0), Shininess);
    
    // Get ambient element
    vec4 ambient = AmbientProduct;

    // Calculate diffuse coefficients
    float Kd_p = max(dot(L_p, N), 0.0);
    float Kd_d = max(dot(L_d, N), 0.0);
    
    // Calculate diffuse elements
    vec4 diffuse_p = Kd_p*DiffuseProduct;
    vec4 diffuse_d = Kd_d*DiffuseProduct;

    // Calculate specular elements
    vec4 specular_p = Ks_p*SpecularProduct;
    vec4 specular_d = Ks_d*SpecularProduct;

	if(BaseOrJointFlag == 1)
	{
        // When the object is the base or one of the joints
        gl_FragColor = ambient + diffuse_p + diffuse_d + specular_p + specular_d;
	} 
	else
	{
        // When the object is 'not-the-base-or-one-of-the-joints'
    	gl_FragColor = texture2D(texture,texCoord) + 0.35*(ambient + diffuse_p + diffuse_d + specular_p + specular_d);
	}


} 

