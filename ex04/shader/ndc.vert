#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
out vec3 vertexColor;
out float gl_ClipDistance[1];

uniform  mat4 modelview;
uniform  mat4 projection;
uniform  mat4 cv_transform;
uniform  float clip_plane_distance;

vec4 pos;
void main() {
  vertexColor = (modelview * vec4(normal, 0)).xyz;
  gl_Position = cv_transform * projection * modelview * vec4(vertex, 1.0);

  // Ax + By + Cz + D = 0
  vec4 clipPlaneXp = vec4( 1,  0,  0, clip_plane_distance*clip_plane_distance);
  vec4 clipPlaneXn = vec4(-1,  0,  0, clip_plane_distance*clip_plane_distance);  
  vec4 clipPlaneYp = vec4( 0,  1,  0, clip_plane_distance*clip_plane_distance);
  vec4 clipPlaneYn = vec4( 0, -1,  0, clip_plane_distance*clip_plane_distance);
  vec4 clipPlaneZp = vec4( 0,  0,  1, clip_plane_distance*clip_plane_distance);
  vec4 clipPlaneZn = vec4( 0,  0, -1, clip_plane_distance*clip_plane_distance);
  
  float clipZp = dot(gl_Position, cv_transform * clipPlaneZp);
  float clipZm = dot(gl_Position, cv_transform * clipPlaneZn);
  float clipYp = dot(gl_Position, cv_transform * clipPlaneYp);
  float clipYm = dot(gl_Position, cv_transform * clipPlaneYn);  
  float clipXp = dot(gl_Position, cv_transform * clipPlaneXp);
  float clipXm = dot(gl_Position, cv_transform * clipPlaneXn);
  float clipZ = min((clipZp), (clipZm));
  float clipY = min((clipYp), (clipYm));
  float clipX = min((clipXp), (clipXm));
  float clipZY = min((clipZ), (clipY));
  float clipXYZ = min((clipX), (clipZY));
  
  gl_ClipDistance[0] = clipXYZ;
}
