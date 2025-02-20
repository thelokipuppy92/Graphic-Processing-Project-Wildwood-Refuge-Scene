#version 410 core

// Inputs
in vec3 fNormal;
in vec3 fPos;
in vec2 fTexCoords;
in vec4 fragmentPosEyeSpace; // Add this input, passed from the vertex shader
in vec4 fragPosLightSpace;   // Light space position for shadow mapping

out vec4 fColor;

// Lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// Point light
uniform int pointLightStart;
uniform vec3 pointLightPos1;

// Matrices
uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 model;

// Texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Fog
uniform int isFogRunning;

// Shadow map
uniform sampler2D shadowMap;


// Lighting constants
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

// Point light constants
float ambientPoint = 0.5f;
float specularStrengthPoint = 0.5f;
float shininessPoint = 25.0f;

// Point light attenuation constants
float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.20f;

// Compute directional light
vec3 computeDirLight() {
    vec3 cameraPosEye = vec3(0.0f); // Viewer position in eye space
    vec4 fPosEye = view * model * vec4(fPos, 1.0f);

    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightDir);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    // Compute ambient light
    ambient = ambientStrength * lightColor;

    // Compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    // Compute specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;

    return ambient + diffuse + specular;
}

vec3 computePointLight(vec4 lightPosEye){
    vec3 redColor = vec3(1.0f, 0.0f, 0.0f);
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightDirN = normalize(lightPosEye.xyz - fragmentPosEyeSpace.xyz);
    vec3 viewDirN = normalize(cameraPosEye - fragmentPosEyeSpace.xyz);
    
    vec3 ambient = ambientPoint * redColor;
    vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * redColor;
    vec3 halfVector = normalize(lightDirN + viewDirN);
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininessPoint);
    vec3 specular = specularStrengthPoint * specCoeff * redColor;
    
    // Calculate attenuation
    float distance = length(lightPosEye.xyz - fragmentPosEyeSpace.xyz);
    float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
    
    return (ambient + diffuse + specular) * att * vec3(2.0f, 2.0f, 2.0f);
}

// Compute shadow
float computeShadow() {
    // Perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;

    // Get the closest depth value from the shadow map
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    
    // Check if the fragment is in shadow
    float bias = max(0.05 * (1.0 - dot(fNormal, lightDir)), 0.005f);
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    
    if (normalizedCoords.z > 1.0f) {
        return 0.0f;
    }
    
    return shadow;
}

// Compute fog
float computeFog() {
   float fogDensity = 0.04f;
    float fragmentDistance = length(fragmentPosEyeSpace);
    return exp(-pow(fragmentDistance * fogDensity, 2));
}


void main() {
    // Compute directional light
    vec3 dirLight = computeDirLight();

    // Apply shadow
    float shadowFactor = computeShadow();
    
    // Apply diffuse and specular textures
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    // Apply point light if activated
    if (pointLightStart == 1) {
        vec4 lightPosEye1 = view * vec4(pointLightPos1, 1.0f);
        dirLight += computePointLight(lightPosEye1);
 vec4 diffuseColor = texture(diffuseTexture,fTexCoords);

    }

   // Final color calculation with shadow
    vec3 color = min((ambient + (1.0f - shadowFactor) * diffuse) + (1.0f - shadowFactor) * specular, 1.0f);
    vec4 colorWithShadow = vec4(color, 1.0f);
  
    // Apply fog if enabled
    if (isFogRunning == 1) {
        float fogFactor = computeFog();
        vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray fog color
        fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
    } else {
        //fColor = vec4(color, 1.0f); // Use the computed lighting result directly
	fColor = min(colorWithShadow * vec4(dirLight, 1.0f), 1.0f);

    }

	}
