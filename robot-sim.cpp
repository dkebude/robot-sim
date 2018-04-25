#include "Angel.h"

// Timer function declaration
void timer( int p );

// PPM reader
unsigned char *readppm(char *filename, int *width, int *height)
{
    FILE *fd;
    int n, m, k, nm;
    char c;
    char b[100];
    sscanf(filename, "%s", b);
    int red, green, blue;
    fd = fopen(b, "rb");
    if ( !fd ) {
      perror(filename);
      return NULL;
    }
    fscanf(fd,"%[^\n] ", b);
    if(b[0]!='P'|| b[1] != '3')
    { 
        printf("Is not a PPM file!\n");
        exit(0);
    }
    fscanf(fd, "%c", &c);
    while(c == '#')
    {
        fscanf(fd, "%[^\n]", b);
        printf("%s\n",b);
        fscanf(fd, "%c",&c);
    }
    ungetc(c,fd);
    
    fscanf(fd, "%d %d %d", &n, &m, &k);
    printf("%d rows %d columns max value= %d\n",n,m,k);
    nm = n*m;
    unsigned char *image = (unsigned char*)malloc(3*sizeof(unsigned char)*nm);
    for(int i=nm;i>0;i--)
    {
        fscanf(fd,"%d %d %d",&red, &green, &blue );
        image[3*nm-3*i]=red;
        image[3*nm-3*i+1]=green;
        image[3*nm-3*i+2]=blue;
    }

    *width = n;
    *height = m;
    return image;
}

typedef vec4  color4;
typedef vec4  point4;

// slice and stack count for the spheres and cylinders
const int slices = 30;
const int stacks = 30;

// Number of vertices per object in the scene
const int NumVertices_table = 36;
const int NumVertices_base = slices*6; 
const int NumVertices_shoulder = slices*6; 
const int NumVertices_uArm = slices*6;
const int NumVertices_lArm = slices*6;
const int NumVertices_sJoint = 2 * (slices + 1) * stacks;
const int NumVertices_eJoint = 2 * (slices + 1) * stacks;
const int NumVertices_wJoint = 2 * (slices + 1) * stacks;

// Vertex information arrays to be placed in the buffer,
// These will be filled by functions later
point4 points[NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm+NumVertices_eJoint+NumVertices_lArm+NumVertices_wJoint];
vec3 normals[NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm+NumVertices_eJoint+NumVertices_lArm+NumVertices_wJoint];
vec2 tex_coords[NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm+NumVertices_eJoint+NumVertices_lArm+NumVertices_wJoint];
color4 colors[NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm+NumVertices_eJoint+NumVertices_lArm+NumVertices_wJoint];

// Vertices for the table
point4 vertices[8] = {
    point4( -1.0, -0.4,  1.0, 1.0 ),
    point4( -1.0,  0.0,  1.0, 1.0 ),
    point4( 11.0,  0.0,  1.0, 1.0 ),
    point4( 11.0, -0.4,  1.0, 1.0 ),
    point4( -1.0, -0.4, -4.0, 1.0 ),
    point4( -1.0,  0.0, -4.0, 1.0 ),
    point4( 11.0,  0.0, -4.0, 1.0 ),
    point4( 11.0, -0.4, -4.0, 1.0 )
};

// Several colors to be used later wherever necessary
color4 vertex_colors[10] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 ),   // cyan
    color4( 0.5, 0.5, 0.5, 1.0 ),  // gray
    color4( 51.0/256.0, 25.0/256.0, 0.0, 1.0) //brown
};

// Angle definitions. These will hold the rotation information of the members
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int  Axis = Xaxis;
GLfloat  BaseAngle[NumAxes] = { 0.0, 90.0, 0.0 };
GLfloat  UArmAngle[NumAxes] = { 0.0, 0.0, 270.0 };
GLfloat  LArmAngle[NumAxes] = { 180.0, 0.0, 0.0 };
GLfloat  Theta[NumAxes] = { 45.0, 0.0, 0.0 };

// Light positions
point4 p_light_position( -1.0, 8.0, 1.0, 1.0 );
point4 d_light_position(  1.0, 8.0, 1.0, 0.0 );

// Parameter initializations
int armID = 0;
float material_shininess = 200;
float zScale = -12.0;
float xAlign = -5.5;
float yAlign = 0.0;
bool turn_on = true;
bool turn_off = false;
bool selected = false;

// Uniform declarations
GLuint  ModelView, Projection; 
GLuint BaseOrJointFlag, Shininess, PointLightPosition, DirectionalLightPosition, PointLightDist;

// Texture buffer to hold texture images
GLuint textures[2];

int Index = 0;

// Calculates normals for shading,
// Texture coordinates for texture mapping
// And vertex positions for each face of the table
// Feeds that information to the related arrays
void
quad( int a, int b, int c, int d )
{
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];

    vec3 normal = normalize( cross(u, v) );

    normals[Index] = normal; tex_coords[Index] = vec2(1.0, 1.0); points[Index] = vertices[a]; Index++;
    normals[Index] = normal; tex_coords[Index] = vec2(0.0, 1.0); points[Index] = vertices[b]; Index++;
    normals[Index] = normal; tex_coords[Index] = vec2(0.0, 0.0); points[Index] = vertices[c]; Index++;
    normals[Index] = normal; tex_coords[Index] = vec2(1.0, 1.0); points[Index] = vertices[a]; Index++;
    normals[Index] = normal; tex_coords[Index] = vec2(0.0, 0.0); points[Index] = vertices[c]; Index++;
    normals[Index] = normal; tex_coords[Index] = vec2(1.0, 0.0); points[Index] = vertices[d]; Index++;

}

// Generates faces of the table
void
table()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

// Calculates normals for shading,
// Color for the base cylinder that holds the robot,
// Texture coordinates for texture mapping,
// And vertex positions for each cylinder
// Feeds that information to the related arrays
void cylinder(float initX, float initY, float initZ, float radius, float height, bool base) {
  float step = 2*M_PI/slices;

  for (float theta = 0.0; theta < 2*M_PI-0.0001; theta += step){

    if(base){ colors[Index] = vertex_colors[8]; } points[Index] = vec4(initX + 0.0, initY + 0.0, initZ + 0.0, 1.0)*radius;                                    normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); tex_coords[Index] = vec2( (atan2(points[Index].z, points[Index].x)/M_PI+1.0)+0.5, (points[Index].y-initY)/height); Index++; 
    if(base){ colors[Index] = vertex_colors[8]; } points[Index] = vec4(initX + cos(theta)*radius, initY + 0.0, initZ + sin(theta)*radius,1.0);                normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); tex_coords[Index] = vec2( (atan2(points[Index].z, points[Index].x)/M_PI+1.0)+0.5, (points[Index].y-initY)/height); Index++;
    if(base){ colors[Index] = vertex_colors[8]; } points[Index] = vec4(initX + cos(theta+step)*radius, initY + 0.0, initZ + sin(theta+step)*radius,1.0);      normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); tex_coords[Index] = vec2( (atan2(points[Index].z, points[Index].x)/M_PI+1.0)+0.5, (points[Index].y-initY)/height); Index++;
    if(base){ colors[Index] = vertex_colors[8]; } points[Index] = vec4(initX + cos(theta)*radius, initY + height, initZ + sin(theta)*radius,1.0);             normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); tex_coords[Index] = vec2( (atan2(points[Index].z, points[Index].x)/M_PI+1.0)+0.5, (points[Index].y-initY)/height); Index++;
    if(base){ colors[Index] = vertex_colors[8]; } points[Index] = vec4(initX + cos(theta+step)*radius, initY + height, initZ + sin(theta+step)*radius,1.0);   normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); tex_coords[Index] = vec2( (atan2(points[Index].z, points[Index].x)/M_PI+1.0)+0.5, (points[Index].y-initY)/height); Index++;
    if(base){ colors[Index] = vertex_colors[8]; } points[Index] = vec4(initX + 0.0, initY + height, initZ + 0.0,1.0);                                         normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); tex_coords[Index] = vec2( (atan2(points[Index].z, points[Index].x)/M_PI+1.0)+0.5, (points[Index].y-initY)/height); Index++;
  }
}

// Calculates normals for shading,
// Color for each of the joints,
// And vertex positions for each sphere.
// Feeds that information to the related arrays
void sphere(float initX, float initY, float initZ, float radius)
{
  float stepsk = M_PI/stacks;
  float stepsl = 2*M_PI/slices;

  for (float theta = 0.0; theta < M_PI - 0.0001; theta += stepsk) {
    for (float phi = 0.0; phi <= 2*M_PI + 0.0001; phi += stepsl) {
      colors[Index] = vertex_colors[8]; points[Index] = vec4(initX + sin(theta) * sin(phi)*radius, initY + cos(theta)*radius, initZ + sin(theta) * cos(phi)*radius, 1.0);                               normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); Index++;
      colors[Index] = vertex_colors[8]; points[Index] = vec4(initX + sin(theta + stepsk) * sin(phi)*radius, initY + cos(theta + stepsk)*radius, initZ + sin(theta + stepsk) * cos(phi)*radius, 1.0);    normals[Index] = normalize(vec3(points[Index].x, points[Index].y, points[Index].z)); Index++;
    }
}
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    // Fill info arrays
    table(); // table
    cylinder(0.0, 0.0, 0.0, 0.6f, 0.2f, true);  // base
    cylinder(0.0, 0.2, 0.0, 0.25f, 1.05f, false);    // shoulder
    sphere(0.0, 1.4, 0.0, 0.3); // shoulder joint
    cylinder(0.0, 1.55, 0.0, 0.2f, 2.6f, false); // upper arm
    sphere(0.0, 4.3, 0.0, 0.3); // elbow joint
    cylinder(0.0, 4.45, 0.0, 0.15f, 2.5f, false); // lower arm
    sphere(0.0, 7.1, 0.0, 0.3); // wrist joint
    
    // Get texture data from ppm files
    GLint w1, h1, w2, h2;
    GLubyte *image1;
    GLubyte *image2;
    glGenTextures( 2, textures );

    // Get the carbon fiber texture,
    // Use nearest neighbors filter for magnification and linear filter for minification
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    image1 = (GLubyte*) readppm( "carbon.ppm", &w1, &h1 );
    printf("%d \t %d \t %d", image1[0], image1[1], image1[2]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, image1);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Get the wooden texture
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    image2 = (GLubyte*) readppm( "wood.ppm", &w2, &h2 );
    printf("%d \t %d \t %d", image2[0], image2[1], image2[2]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w2, h2, 0, GL_RGB, GL_UNSIGNED_BYTE, image2);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(tex_coords) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(tex_coords), tex_coords );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(tex_coords), sizeof(colors), colors );
 
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    
    // Set up vertex attribute arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( sizeof(points)) );

    GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" ); 
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( sizeof(points) + sizeof(normals) ) );

    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( sizeof(points) + sizeof(normals) + sizeof(tex_coords) ) );

    // Initialize shader lighting parameters
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 ); // L_a
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 ); // L_d
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 ); // L_s

    color4 material_ambient( 1.0, 1.0, 1.0, 1.0 ); // k_a
    color4 material_diffuse( 0.5, 0.5, 0.5, 1.0 ); // k_d
    color4 material_specular( 1.0, 1.0, 1.0, 1.0 ); // k_s

    color4 ambient_product = light_ambient * material_ambient; // k_a * L_a
    color4 diffuse_product = light_diffuse * material_diffuse; // k_d * L_d
    color4 specular_product = light_specular * material_specular; // k_s * L_s

    // Retrieve uniform variable positions below for everything in the shaders
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
          1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
          1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
          1, specular_product );

    PointLightPosition = glGetUniformLocation(program, "PointLightPosition");
    glUniform4fv( PointLightPosition, 1, p_light_position );
    DirectionalLightPosition = glGetUniformLocation(program, "DirectionalLightPosition");
    glUniform4fv( DirectionalLightPosition, 1, d_light_position );

    Shininess = glGetUniformLocation(program, "Shininess");
    glUniform1f( Shininess, material_shininess );

    // This flag determines if the object is the base member or one of the joints,
    // If so, the object will be colored instead of being texturized.
    BaseOrJointFlag = glGetUniformLocation( program, "BaseOrJointFlag" );

    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    // Set current program object
    glUseProgram( program );
   
    // Enable hiddden surface removal
    glEnable( GL_DEPTH_TEST );

    // Set state variable "clear color" to clear buffer with.
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
   }

//----------------------------------------------------------------------------

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate the model-view matrix
    const vec3 displacement( xAlign, yAlign, zScale ); // xAlign, yAlign and zScale parameters can be manipulated by the user
    mat4  model_view = ( Scale( 1.0, 1.0, 1.0 ) * Translate( displacement ) *
			 RotateX( Theta[Xaxis] ) *
			 RotateY( Theta[Yaxis] ) *
			 RotateZ( Theta[Zaxis] ) ); 
    
    // Send the model-view matrix to change how the scene is viewed
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );

    // Bind the wooden texture and set the member as 'not-the-base-or-one-of-the-joints'
    glUniform1i(BaseOrJointFlag, 0);
    glBindTexture( GL_TEXTURE_2D, textures[1] );

    // Draw the table
    glDrawArrays( GL_TRIANGLES, 0, NumVertices_table);

    // manipulate the model-view matrix to rotate the base member
    model_view = model_view * RotateY(BaseAngle[Yaxis]);
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );

    // Activate stencil buffer to assign an ID to each member
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    
    glUniform1i(BaseOrJointFlag, 1); // Set the member as the base
    glStencilFunc(GL_ALWAYS, 1, -1); // assign ID = 1 to the base and shoulder members
    glBindTexture( GL_TEXTURE_2D, textures[0] ); // Bind the carbon fiber texture
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table, NumVertices_base); // Draw the base
    glUniform1i(BaseOrJointFlag, 0); // Set the member as 'not-the-base-or-one-of-the-joints'
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table+NumVertices_base, NumVertices_shoulder); // Draw the shoulder
    glUniform1i(BaseOrJointFlag, 1); // Set the object as a joint
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table+NumVertices_base+NumVertices_shoulder, NumVertices_sJoint); // Draw the shoulder joint

    // Manipulate the model-view matrix to rotate the upper arm
    model_view = model_view * Translate ( 0.0, 1.55, 0.0) *
            RotateX( UArmAngle[Xaxis] ) *
            RotateY( UArmAngle[Yaxis] ) *
            RotateZ( UArmAngle[Zaxis] ) * Translate ( 0.0, -1.55, 0.0);
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );

    glUniform1i(BaseOrJointFlag, 0); // Set the member as 'not-the-base-or-one-of-the-joints'
    glStencilFunc(GL_ALWAYS, 2, -1); // assign ID = 2 to the upper arm 
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint, NumVertices_uArm); // Draw the upper arm
    glUniform1i(BaseOrJointFlag, 1); // Set the object as a joint
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm, NumVertices_eJoint); // Draw the elbow joint

    // Manipulate the model-view matrix to rotate the lower arm
    model_view = model_view * Translate ( 0.0, 4.45, 0.0) *
            RotateX( LArmAngle[Xaxis] ) *
            RotateY( LArmAngle[Yaxis] ) *
            RotateZ( LArmAngle[Zaxis] ) * Translate ( 0.0, -4.45, 0.0);
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );

    glUniform1i(BaseOrJointFlag, 0); // Set the member as 'not-the-base-or-one-of-the-joints'
    glStencilFunc(GL_ALWAYS, 3, -1); // assign ID = 3 to the lower arm
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm+NumVertices_eJoint, NumVertices_lArm); // Draw the lower arm
    glUniform1i(BaseOrJointFlag, 1); // Set the object as a joint
    glDrawArrays( GL_TRIANGLE_STRIP, NumVertices_table+NumVertices_base+NumVertices_shoulder+NumVertices_sJoint+NumVertices_uArm+NumVertices_eJoint+NumVertices_lArm, NumVertices_wJoint); // Draw the wrist joint

    glDisable(GL_STENCIL_TEST); // Disable stencil buffer so that we do not assign any additional IDs

    glutSwapBuffers();
}

// Utilize perspective projection 
// /w 90 degree FOV
// near @ z = 0.1 and far @ z = 20.0
// Keep the aspect ratio so that the scene does not go bad when the window is reshaped
void reshape( int w, int h )
{
    glViewport( 0, 0, w, h );
    
    GLfloat aspect = GLfloat(w)/h;
    mat4  projection = Perspective( 90.0, aspect, 0.1, 20.0 );

    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

// Keep the angles between 0 and 360 degrees
// Also call the timer function when initial pos. or shut-off pos. is called
void
idle( void )
{
    if(BaseAngle[Yaxis] < 0.0)
        BaseAngle[Yaxis] += 360.0;
    else if(BaseAngle[Yaxis] >= 360.0)
        BaseAngle[Yaxis] -= 360.0;

    if(UArmAngle[Xaxis] < 0.0)
        UArmAngle[Xaxis] += 360.0;
    else if(UArmAngle[Xaxis] >= 360.0)
        UArmAngle[Xaxis] -= 360.0;

    if(UArmAngle[Yaxis] < 0.0)
        UArmAngle[Yaxis] += 360.0;
    else if(UArmAngle[Yaxis] >= 360.0)
        UArmAngle[Yaxis] -= 360.0;

    if(UArmAngle[Zaxis] < 0.0)
        UArmAngle[Zaxis] += 360.0;
    else if(UArmAngle[Zaxis] >= 360.0)
        UArmAngle[Zaxis] -= 360.0;

    if(LArmAngle[Xaxis] < 0.0)
        LArmAngle[Xaxis] += 360.0;
    else if(LArmAngle[Xaxis] >= 360.0)
        LArmAngle[Xaxis] -= 360.0;

    if(LArmAngle[Yaxis] < 0.0)
        LArmAngle[Yaxis] += 360.0;
    else if(LArmAngle[Yaxis] >= 360.0)
        LArmAngle[Yaxis] -= 360.0;

    if(LArmAngle[Zaxis] < 0.0)
        LArmAngle[Zaxis] += 360.0;
    else if(LArmAngle[Zaxis] >= 360.0)
        LArmAngle[Zaxis] -= 360.0;

    if(turn_on && BaseAngle[Yaxis] != 0 && UArmAngle[Zaxis] != 45 && LArmAngle[Zaxis] != 270)
    {        
        glutTimerFunc(50, timer, 0);
    }
    else if(turn_off)
    {
        glutTimerFunc(50, timer, 0);
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key,int x, int y )
{
    // Quit when Q is hit
    if(key == 'Q' | key == 'q')
        exit(0);

    // Go back to the initial position
    if(key == 'I' | key == 'i')
    {
        BaseAngle[Xaxis] = 0.0;
        BaseAngle[Yaxis] = 90.0;
        BaseAngle[Zaxis] = 0.0;
        UArmAngle[Xaxis] = 0.0;
        UArmAngle[Yaxis] = 0.0;
        UArmAngle[Zaxis] = 270.0;
        LArmAngle[Xaxis] = 180.0;
        LArmAngle[Yaxis] = 0.0;
        LArmAngle[Zaxis] = 0.0;
        turn_on = true;
    }

    // Go to shut-off position
    if(key == 'F' | key == 'f')
    {
        turn_off = true;
    }

    // Rotate the selected member along x-axis (CCW)
    if(key == 'W' | key == 'w')
    {
        if(selected)
        {
            if(armID == 1)
                BaseAngle[Xaxis] -= 0.0;
            else if(armID == 2)
                UArmAngle[Xaxis] -= 1.0;
            else if(armID == 3)
                LArmAngle[Xaxis] -= 1.0;
        }   
    }

    // Rotate the selected member along x-axis (CW)
    if(key == 'S' | key == 's')
    {
        if(selected)
        {
            if(armID == 1)
                BaseAngle[Xaxis] += 0.0;
            else if(armID == 2)
                UArmAngle[Xaxis] += 1.0;
            else if(armID == 3)
                LArmAngle[Xaxis] += 1.0;
        }   
    }

    // Rotate the selected member along z-axis (CW)
    if(key == 'A' | key == 'a')
    {
        if(selected)
        {
            if(armID == 1)
                BaseAngle[Zaxis] += 0.0;
            else if(armID == 2)
                UArmAngle[Zaxis] += 1.0;
            else if(armID == 3)
                LArmAngle[Zaxis] += 1.0;
        }   
    }

    // Rotate the selected member along z-axis (CCW)
    if(key == 'D' | key == 'd')
    {
        if(selected)
        {
            if(armID == 1)
                BaseAngle[Zaxis] -= 0.0;
            else if(armID == 2)
                UArmAngle[Zaxis] -= 1.0;
            else if(armID == 3)
                LArmAngle[Zaxis] -= 1.0;
        }   
    }

    // Rotate the selected member along y-axis (CW)
    if(key == 'Z' | key == 'z')
    {
        if(selected)
        {
            if(armID == 1)
                BaseAngle[Yaxis] += 1.0;
            else if(armID == 2)
                UArmAngle[Yaxis] += 1.0;
            else if(armID == 3)
                LArmAngle[Yaxis] += 1.0;
        }   
    }

    // Rotate the selected member along y-axis (CCW)
    if(key == 'X' | key == 'x')
    {
        if(selected)
        {
            if(armID == 1)
                BaseAngle[Yaxis] -= 1.0;
            else if(armID == 2)
                UArmAngle[Yaxis] -= 1.0;
            else if(armID == 3)
                LArmAngle[Yaxis] -= 1.0;
        }   
    }

    // Zoom in
    if(key == 'l')
    {
        if(zScale < -2.0)
        {
            zScale += 0.2;
        }   
    }

    // Zoom out
    if(key == 'L')
    {
        if(zScale > -18.0)
        {
            zScale -= 0.2;
        }   
    }

    // Move left
    if(key == 'j' | key == 'J')
    {
        if(xAlign > -12.0)
        {
            xAlign -= 0.2;
        }   
    }

    // Move right
    if(key == 'k' | key == 'K')
    {
        if(xAlign < 6.0)
        {
            xAlign += 0.2;
        }   
    }        

    // Move down
    if(key == 'm' | key == 'M')
    {
        if(yAlign > -10.0)
        {
            yAlign -= 0.2;
        }   
    }

    // Move up
    if(key == 'u' | key == 'U')
    {
        if(yAlign < 10.0)
        {
            yAlign += 0.2;
        }   
    }

    // Call help (print on the console)
    if(key == 'h' | key == 'H')
    {
        cout << endl << endl << "************** YOU HAVE CALLED HELP **************" << endl << endl;
        cout << "Welcome to the simulator!" << endl << endl;
        cout << "To control the robot:" << endl;
        cout << "First, select an arm member (either the base, the lower arm or the upper arm)" << endl;
        cout << "Then, use the following keys for rotation around the joints:" << endl << endl;
        cout << "* S for counter-clockwise (CCW) and W for clockwise (CW) rotation around x-axis" << endl;
        cout << "* Z for counter-clockwise (CCW) and X for clockwise (CW) rotation around y-axis" << endl;
        cout << "* A for counter-clockwise (CCW) and D for clockwise (CW) rotation around z-axis" << endl << endl;
        cout << "Use I key for going back to the initial position of the robot." << endl;
        cout << "Use F key for going to the shut-off position of the robot." << endl << endl;
        cout << "To manipulate the scene use the following keys: " << endl << endl;
        cout << "*L key to zoom in and shift+L keys to zoom out" << endl;
        cout << "*J and K keys to move along x-axis (left and right)" << endl;
        cout << "*U and M keys to move along y-axis (up and down)" << endl << endl;
        cout << "Press Q to quit the simulator."
        cout << endl << endl << "***************** HELP HAS ENDED *****************" << endl << endl;
    }
}

// Does the picking and prints which member is picked to the console
void mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        y = glutGet( GLUT_WINDOW_HEIGHT ) - y;
        
        glReadPixels(x, y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &armID);
        
        if(armID == 1)
            std::cout << "Selected Arm: Base" << std::endl;
        else if(armID == 2)
            std::cout << "Selected Arm: Upper Arm" << std::endl;
        else if(armID == 3)
            std::cout << "Selected Arm: Lower Arm" << std::endl;
        else
            armID = 0;

        if(armID != 0)
            selected = true;
        else
            selected = false;

        glutPostRedisplay(); //needed to avoid display of the content of the back buffer when some portion of the window is obscured
    }
}

void timer( int p ) // Rotation animations are handled by the timer
{
    if(turn_on)
    {
        if(BaseAngle[Yaxis] != 0)
            BaseAngle[Yaxis] -= 0.25;
        else if(UArmAngle[Xaxis] != 0)
            UArmAngle[Xaxis] -= 0.25;
        else if(UArmAngle[Yaxis] != 0)
            UArmAngle[Yaxis] -= 0.25;
        else if(UArmAngle[Zaxis] != 45)
            UArmAngle[Zaxis] += 0.25;
        else if(LArmAngle[Xaxis] != 180)
            LArmAngle[Xaxis] -= 0.25;
        else if(LArmAngle[Yaxis] != 0)
            LArmAngle[Yaxis] -= 0.25;
        else if(LArmAngle[Zaxis] != 270)
            LArmAngle[Zaxis] -= 0.25;
        else
            turn_on = false;
    }
    else if(turn_off)
    {
        if(BaseAngle[Yaxis] != 90 )
            BaseAngle[Yaxis] += 0.25;
        else if(UArmAngle[Xaxis] != 0)
            UArmAngle[Xaxis] -= 0.25;
        else if(UArmAngle[Yaxis] != 0)
            UArmAngle[Yaxis] -= 0.25;
        else if(UArmAngle[Zaxis] != 270)
            UArmAngle[Zaxis] -= 0.25;
        else if(LArmAngle[Xaxis] != 180)
            LArmAngle[Xaxis] -= 0.25;
        else if(LArmAngle[Yaxis] != 0)
            LArmAngle[Yaxis] -= 0.25;
        else if(LArmAngle[Zaxis] != 360.0)
            LArmAngle[Zaxis] += 0.25;
        else
            turn_off = false;
    }
    glutTimerFunc(50,timer,0);
}
//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Robot Arm" );

    glewExperimental = GL_TRUE;
    glewInit();

    init();
    
    glutDisplayFunc( display ); // set display callback function
    glutReshapeFunc( reshape );
    glutIdleFunc( idle );
    glutMouseFunc( mouse );
    glutKeyboardFunc(keyboard);
    glutTimerFunc(5,timer,0);
    
    glutMainLoop();
    return 0;
}