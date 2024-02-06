// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


glm::vec3 origin = glm::vec3( 0, 0, 0 );
// Initial position : on x = 7, z = 7
glm::vec3 position = glm::vec3( 0, 0, 7 );

// Initial horizontal angle
float phi = 0.0f;
// Initial vertical angle
float theta = 3.14f / 4.0f;

// Initial Field of View
float initialFoV = 45.0f;

// Speed of camera moving
float speed = 0.04f;

float radius = 7.0f;



void computeMatricesFromInputs(){
    // glfwGetTime is called only once, the first time this function is called
    //static double lastTime = glfwGetTime();
    
    // Move camera UP
    if (glfwGetKey( window, GLFW_KEY_U ) == GLFW_PRESS){
        if (theta > speed) { // Preventing screen flipping
            theta -= speed;
        }
    }
    // Move camera DOWN
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
        if (theta < 3.14f - speed) {
            theta += speed;
        }
    }
    // Rotate camera RIGHT
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        phi += speed;
    }
    // Rotate camera LEFT
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        phi -= speed;
    }
    // Move camera CLOSER to the origin
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        if (radius > 0.1f) { // Preventing screen flipping
            radius -= speed;
        }
    }
    // Move camera Farther from the origin
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        radius += speed;
    }
    
    position.x = radius * sin(theta) * cos(phi);
    position.y = radius * sin(theta) * sin(phi);
    position.z = radius * cos(theta);

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
                                   position,           // Camera is here
                                   origin,             // Always look at the origin
                                   vec3( 0, 0, 1 )     // Head is up (set to 0,0,1 to look upside-down)
                                   );

}
