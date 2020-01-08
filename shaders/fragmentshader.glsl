#version 330 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D shadowMap;

uniform vec3 color;
//uniform mat4 model;
//uniform mat4 view;
uniform bool lightsOn;
uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;
uniform vec3 lightPos1, lightPos2, lightPos3;
uniform vec3 lightColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    projCoords = projCoords * 0.5 + 0.5;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    
    float currentDepth = projCoords.z;

    //float bias = 0.003;
    float bias = max(0.05 * (1.0 - dot(normalize(Normal), normalize(lightPos1 - FragPos))), 0.005);
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{	
    	if(lightsOn){

		vec3 result = vec3(0,0,0);

		float ambientStrength = 0.2;
	    vec3 ambient = ambientStrength * lightColor;
	    
	    //result += ambient;

	    vec3 norm = normalize(Normal);
	    vec3 lightDir1 = normalize(lightPos1 - FragPos);
	    vec3 surfaceToCamera = normalize(cameraPos-FragPos);
	    float nDotL = dot(norm, lightDir1);
	    float diff1 = max(dot(norm, lightDir1), 0.0);
	    vec3 diffuse1 = diff1*lightColor*nDotL;
	 	
	 	//result += diffuse1;
	    
	    //specular
	    vec3 materialSpecularColor = vec3(1.0,1.0,1.0);
    	float specularCoefficient = 0.0;
    	float materialShininess = 100.0;
    	if(diff1 > 0.0)
        	specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-lightDir1, norm))), materialShininess);
   		
   		vec3 specular = specularCoefficient * materialSpecularColor * lightColor;

   		float shadow = ShadowCalculation(FragPosLightSpace); 
    	
    	//result = result*color + specular;

    	result = (ambient + (1.0 - shadow)*diffuse1)*color + specular*(1.0-shadow);

	    FragColor = vec4(result, 1.0);

	    }
	    else {
        	FragColor = vec4(color, 1.0f); 
    	}
}