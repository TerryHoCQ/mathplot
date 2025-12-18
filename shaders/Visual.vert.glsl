#version 410
uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;
uniform float alpha;
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normalin;
layout(location = 2) in vec3 color;
out VERTEX
{
    vec4 normal;
    vec4 color;
    vec3 fragpos;
} vertex;
void main()
{
    gl_Position = (p_matrix * v_matrix * m_matrix * position);
    vertex.color = vec4(color, alpha);
    vertex.fragpos = vec3(m_matrix * position);
    vertex.normal = normalin;
}
