#version 330

layout(location = 0) in vec3 vertex;
layout(location = 2) in vec2 vertex_texcoord;

out vec2 vertexTexCoord;

// modelview and projection matrix //
uniform mat4 modelview;
uniform mat4 projection;

void main() {
  vertexTexCoord = vertex_texcoord;
  
  gl_Position = projection * modelview * vec4(vertex, 1.0);
}
