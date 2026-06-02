#version 430
uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;
uniform float alpha;
uniform int instance_count;
uniform int instance_start;
uniform int instparam_count;
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normalin;
layout(location = 2) in vec3 color;
layout (std430, binding = 1) buffer InstPos { float ipos[]; };
layout (std430, binding = 2) buffer InstParam { float iparam[]; };
out VERTEX
{
    vec4 normal;
    vec4 color;
    vec3 fragpos;
} vertex;
void main()
{
    if (instance_count > 0) {
        int ipos_i = instance_start * 3 + gl_InstanceID * 3;
        vec4 iposv;
        iposv[0] = ipos[ipos_i];
        iposv[1] = ipos[ipos_i + 1];
        iposv[2] = ipos[ipos_i + 2];
        iposv[3] = 0.0;
        if (instparam_count > 0) {
            int idx = gl_InstanceID % instparam_count;
            int ippos_i = instance_start * 5;
            float s = iparam[ippos_i + idx * 5 + 4];
            vec3 p_three = vec3(position) * s;
            vec4 p_scaled = vec4(p_three, 1.0);
            gl_Position = (p_matrix * v_matrix * m_matrix * (p_scaled + iposv));
            vertex.color = vec4(iparam[ippos_i + idx * 5], iparam[ippos_i + idx * 5 + 1], iparam[ippos_i + idx * 5 + 2], iparam[ippos_i + idx * 5 + 3]);
            vertex.fragpos = vec3(m_matrix * p_scaled);
        } else {
            gl_Position = (p_matrix * v_matrix * m_matrix * (position + iposv));
            vertex.color = vec4(color, alpha);
            vertex.fragpos = vec3(m_matrix * position);
        }
    } else {
        gl_Position = (p_matrix * v_matrix * m_matrix * position);
        vertex.color = vec4(color, alpha);
        vertex.fragpos = vec3(m_matrix * position);
    }
    vertex.normal = normalin;
}
