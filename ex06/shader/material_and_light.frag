#version 330

struct LightSource {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 position;
};

struct Material {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  float specular_shininess;
};

// TODO: set up uniforms for multiple light sources //
uniform LightSource lights[10];

// uniform for the used material //
uniform Material material;

// fragment normal //
in vec3 vertexNormal;
// vector from fragment to camera //
in vec3 eyeDir;

// TODO: vector from fragment to light per light source //
in vec3 lightDir[10];

// this defines the fragment output //
out vec4 color;

void main() {
  // normalize the vectors passed from your vertex program here //
  vec3 E = normalize(eyeDir);
  vec3 N = normalize(vertexNormal);
  
  // init the ambient, diffuse and specular color terms //
  vec3 ambientTerm = vec3(0);
  vec3 diffuseTerm = vec3(0);
  vec3 specularTerm = vec3(0);
  
  // TODO: compute the ambient, diffuse and specular color terms for every used light source //
  
  for(int i=0; i<10; ++i)
  {
    ambientTerm += lights[i].ambient_color * material.ambient_color;
    
    vec3 L = normalize(lightDir[i]);
    vec3 H = normalize(E + L);
    
    diffuseTerm  += (lights[i].diffuse_color* material.diffuse_color) * max(dot(L, N), 0);
    specularTerm += (lights[i].specular_color* material.specular_color) * pow(max(dot(H, N), 0), material.specular_shininess);
  }
  
  // assign the final color to the fragment output variable //
  color = vec4(ambientTerm + diffuseTerm + specularTerm, 1);
  //color = vec4(lightDir[9], 1);
}
