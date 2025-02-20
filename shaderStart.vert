#version 410 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

out vec3 fNormal;
out vec3 fPos;
out vec2 fTexCoords;
out vec4 fragmentPosEyeSpace; // Output for fragment position in eye space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

//for shadow mapping
uniform mat4 lightSpaceTrMatrix;
out vec4 fragPosLightSpace;

void main() {
    // Compute eye space coordinates
    
    fPos = vec3(model * vec4(vPosition,1.0f));

    fNormal = normalize(normalMatrix * vNormal);
    fTexCoords = vTexCoords;

    // Pass fragment position in eye space
    fragmentPosEyeSpace = view * model * vec4(vPosition, 1.0f);


    // Compute final position
    gl_Position = projection * view * model * vec4(vPosition, 1.0f);

    //compute shadow coordinates
      fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
}