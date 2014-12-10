#version 330

// TODO: again define your variables passed from vertex to fragment program here
// - use 'in' as qualifier now
// - make sure to use the exakt same names

in vec3 frag_normal;
in vec3 frag_light_dir;
in vec3 frag_eye_dir;

in vec3 frag_ambient_term;
in vec3 frag_specular_term;
in vec3 frag_diffuse_term;
in float frag_shininess;

// this defines the fragment output //
out vec3 color;

void main() {
    color = frag_ambient_term;
    
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(frag_light_dir);
    
    float NdotL = max(0.0, dot(N,L));
    
    // TODO: normalize the vectors passed from your vertex program here //
    // - this needs to be done, because the interpolation of these vectors is linear //
    
    // TODO: compute the half-way-vector for our specular component //
    
    // TODO: compute the ambient, diffuse and specular color terms as presented in the lecture //
    
    // TODO: assign the final color to the fragment output variable //
    color += frag_diffuse_term*NdotL;
    
    vec3 V = normalize(frag_eye_dir);
    vec3 H = normalize(V+L);
    float cosBeta = max(0.0, dot(N,H));
    float cosBetak = pow(cosBeta, frag_shininess);
    color += frag_specular_term * cosBetak;
}
