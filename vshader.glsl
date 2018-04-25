attribute  vec4 vPosition;
attribute  vec3 vNormal;
attribute  vec2 vTexCoord;
attribute  vec4 vColor;

varying  vec4 color;
varying  vec2 texCoord;
varying  vec3 fN;
varying  vec3 fV;
varying  vec3 fL_p;
varying  vec3 fL_d;

uniform vec4 PointLightPosition, DirectionalLightPosition;
uniform mat4 ModelView;
uniform mat4 Projection;

void main()
{
    fN = (ModelView*vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates

    fV = (ModelView * vPosition).xyz; //viewer direction in camera coordinates

    fL_p = PointLightPosition.xyz-fV; // light direction (point source)
    fL_d = vec3(DirectionalLightPosition.x,-DirectionalLightPosition.y,-DirectionalLightPosition.z); // light direction (directional source)

    gl_Position = Projection * ModelView * vPosition;
    texCoord = vTexCoord;
    color = vColor;
}
