#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    // Convert FragPos to world space and calculate light distance
    float lightDistance = length(FragPos.xyz - lightPos);

    // Map to [0,1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;

    // Write this as modified depth
    gl_FragDepth = lightDistance;
}
