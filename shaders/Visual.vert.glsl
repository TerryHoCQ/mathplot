// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 430

// SSBO to be added optionally


// ProjMatrix * RotnMatrix operation can be carried out on CPU with a single matrix
//uniform mat4 mvp_matrix;
// Or, and this is important for lighting effects and possibly text, too, matrices can be passed separately
//uniform mat4 vp_matrix; // sceneview-projection matrix
uniform mat4 m_matrix; // model matrix
uniform mat4 v_matrix; // scene view matrix
uniform mat4 p_matrix; // projection matrix
// alpha - to make a model see-through
uniform float alpha;
uniform int instance_count = 0;
uniform int instance_start = -1;

layout(location = 0) in vec4 position; // Attrib location 0
layout(location = 1) in vec4 normalin; // Attrib location 1
layout(location = 2) in vec3 color;    // Attrib location 2

layout (std430, binding = 1) buffer InputBlock { float instance_data[]; };

out VERTEX
{
    vec4 normal;
    vec4 color;   // Could make vec4 and incorporate alpha
    vec3 fragpos; // fragment position
} vertex;

void main (void)
{
    if (instance_count > 0) {
        vec4 offset = { instance_data[gl_InstanceID * 3], instance_data[gl_InstanceID * 3 + 1], instance_data[gl_InstanceID * 3 + 2], 0 };
        gl_Position = (p_matrix * v_matrix * m_matrix * (position + offset));
    } else {
        gl_Position = (p_matrix * v_matrix * m_matrix * position);
    }
    vertex.color = vec4(color, alpha);
    vertex.fragpos = vec3(m_matrix * position);
    // Normals are all automatically computed, so there's no need for
    // this line and the cube program doesn't bother to pass in the
    // normals. Maybe required only for lighting?
    vertex.normal = normalin;
}
