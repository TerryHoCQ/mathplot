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
// We may have 100 instances but only 1 set of params to apply to all instances
uniform int instparam_count = 0;

layout(location = 0) in vec4 position; // Attrib location 0
layout(location = 1) in vec4 normalin; // Attrib location 1
layout(location = 2) in vec3 color;    // Attrib location 2

layout (std430, binding = 1) buffer InstPos { float ipos[]; };
layout (std430, binding = 2) buffer InstParam { float iparam[]; };
//later: layout (std430, binding = 3) buffer InstRotn { float irotn[]; };

out VERTEX
{
    vec4 normal;
    vec4 color;   // Could make vec4 and incorporate alpha
    vec3 fragpos; // fragment position
} vertex;

void main (void)
{
    if (instance_count > 0) {
        vec4 iposv = { ipos[gl_InstanceID * 3], ipos[gl_InstanceID * 3 + 1], ipos[gl_InstanceID * 3 + 2], 0 };
        if (instparam_count > 0) {

            int idx = gl_InstanceID % instparam_count;
            float s = iparam[idx * 5 + 4] * 0.5f;
            vec3 p_three = vec3(position) * s;
            vec4 p_scaled = vec4(p_three, 1.0); // Note 1.0 in last element here, and 0.0 in last element of iposv adds to 1
            gl_Position = (p_matrix * v_matrix * m_matrix * (p_scaled + iposv));
            vertex.color = vec4(iparam[idx * 5], iparam[idx * 5 + 1], iparam[idx * 5 + 2], iparam[idx * 5 + 3]);
            vertex.fragpos = vec3(m_matrix * p_scaled); // Required for correct lighting

        } else {
            gl_Position = (p_matrix * v_matrix * m_matrix * (position + iposv));
            vertex.color = vec4(color, alpha);
            vertex.fragpos = vec3(m_matrix * position); // Required for correct lighting
        }
    } else {
        gl_Position = (p_matrix * v_matrix * m_matrix * position);
        vertex.color = vec4(color, alpha);
        vertex.fragpos = vec3(m_matrix * position); // Required for correct lighting
    }


    // Required for correct lighting
    vertex.normal = normalin;
}
