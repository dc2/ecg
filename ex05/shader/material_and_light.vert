#version 330
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;


// these struct help to organize all the uniform parameters //
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

uniform LightSource lightSource;
uniform Material material;

// TODO: define out variables to be passed to the fragment shader //
// - define variables for vertex normal, camera vector and light vector
// - also define variables to pass color information to the fragment shader
//   (ambient, diffuse and specular color and the shininess exponent)

out vec3 frag_normal;
out vec3 frag_light_dir;
out vec3 frag_eye_dir;

out vec3 frag_ambient_term;
out vec3 frag_diffuse_term;
out vec3 frag_specular_term;
out float frag_shininess;

// view, modelview and projection matrix //
uniform mat4 view;
uniform mat4 modelview;
uniform mat4 projection;

// TODO: define uniform variables used for the pumping effect //
uniform float time;
uniform float amplitude;

void main() {
    vec4 v = vec4(vertex, 1);
    vec4 vn = vec4(vertex_normal, 0);
    vec4 lp = vec4(lightSource.position, 1);
    
    // TODO: pass the light-material color information to the fragment program
    // - as presented in the lecture, you just need to combine light and material color here
    // - assign the final values to your defined out-variables
    
    // TODO: create a normal matrix by inverting and transposing the modelview matrix //
    mat4 normalmat = transpose(inverse(modelview));
    
    // TODO: transform vertex position and the vertex normal using the appropriate matrices //
    // - assign the transformed vertex position (modelview & projection) to 'gl_Position'
    // - assign the transformed vertex normal (normal matrix) to your out-variable as defined above
    // - let the model pump by transforming the vertex
    
    
    frag_normal = normalize(vec3(normalmat*vn));
    
    vec3 pos = (modelview*v).xyz;
    pos += (frag_normal * abs(sin(.125*time))*amplitude/2.);
    
    frag_light_dir = ((view * lp).xyz - pos);
    frag_eye_dir = -pos;
    
    frag_ambient_term = material.ambient_color * lightSource.ambient_color;
    frag_diffuse_term = material.diffuse_color * lightSource.diffuse_color;
    frag_specular_term = material.specular_color * lightSource.specular_color;
    frag_shininess = material.specular_shininess;
    
    // TODO: compute the vectors from the current vertex towards the camera and towards the light source //
    gl_Position = projection * vec4(pos, 1);
}
