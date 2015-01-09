#version 330
in vec2 vertexTexCoord;

// this defines the fragment output //
out vec3 color;

void main() {
  //color = vec3(0, vertexTexCoord.y/4.f, vertexTexCoord.y/2.f) + vec3(0.2);
  color = mix(vec3(0, 0, .2), vec3(0, 0, .75), vertexTexCoord.y);
  //color = vec3(1);
}
