/*
Author: Hyeonjae Park
Class: ECE 4122 (A)
Last Date Modified: Sep 9, 2023
Description:

Using muti-threading to move the obj with collision features and internal lights

*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>
#include <mutex>
#include <cmath>
#include <atomic>
#include <thread>
#include <chrono>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

std::atomic<bool> isRunning(true);
std::atomic<bool> isGKeyPressed(false); // Variable to check the state of the G key

// Struct declaration for convinient management of variables
struct ObjectState {
    std::atomic<float> x, y, z;
    glm::vec3 direction;
    float speed;
};

// Function to calculate distance between two circle's center
float distanceBetweenPoints(glm::vec3 p1, glm::vec3 p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2) + pow(p2.z - p1.z, 2));
}

// Creating array for 4 objects
const int numberOfObjects = 4; // 물체의 수
ObjectState objects[numberOfObjects];

// Global atomic variables for object rotation
std::atomic<float> rotationAngle(0.0f);

void movementThread() {
    std::uniform_real_distribution<float> directionDistribution(-0.5, 0.5); // Direction distribution
    std::uniform_real_distribution<float> speedDistribution(0.1, 0.3); // Speed distribution
    std::default_random_engine generator;
    float radiusBoundary = 6.0f;
    float minY = 0.0f; // Preventing it goes under the floor
    
    for (int i = 0; i < numberOfObjects; ++i) {
            objects[i].x = 0.0f;
            objects[i].y = 1.0f;
            objects[i].z = 1.9f;
            objects[i].direction = glm::vec3(directionDistribution(generator),
                                             directionDistribution(generator),
                                             directionDistribution(generator));
            objects[i].speed = speedDistribution(generator);
    }
    
    float collisionRadius = 0.5f; // Collision area value
    
    while (isRunning) {
        if (isGKeyPressed) {
            for (int i = 0; i < numberOfObjects; ++i) {
                // Update position
                objects[i].x.store(objects[i].x.load() + objects[i].direction.x * objects[i].speed);
                objects[i].y.store(objects[i].y.load() + objects[i].direction.y * objects[i].speed);
                objects[i].z.store(objects[i].z.load() + objects[i].direction.z * objects[i].speed);

                // Boundary conditions
                // X axis
                if (objects[i].x.load() > radiusBoundary || objects[i].x.load() < -radiusBoundary) {
                    objects[i].direction.x = directionDistribution(generator);
                    objects[i].speed = speedDistribution(generator);
                }
                
                // Y axis
                if (objects[i].y.load() > radiusBoundary || objects[i].y.load() < minY) {
                    objects[i].direction.y = directionDistribution(generator);
                    objects[i].speed = speedDistribution(generator);
                }
                
                // Z axis
                if (objects[i].z.load() > radiusBoundary || objects[i].z.load() < -radiusBoundary) {
                    objects[i].direction.z = directionDistribution(generator);
                    objects[i].speed = speedDistribution(generator);
                }
            }
            // Detecting collision and management
            for (int i = 0; i < numberOfObjects; ++i) {
                glm::vec3 pos1(objects[i].x.load(), objects[i].y.load(), objects[i].z.load());

                for (int j = i + 1; j < numberOfObjects; ++j) {
                    glm::vec3 pos2(objects[j].x.load(), objects[j].y.load(), objects[j].z.load());

                    if (distanceBetweenPoints(pos1, pos2) < 2 * collisionRadius) {
                        glm::vec3 collisionDirection = glm::normalize(pos1 - pos2);

                        // Changing direction of object[i]
                        objects[i].direction.x = collisionDirection.x;
                        objects[i].direction.y = collisionDirection.y;
                        objects[i].direction.z = collisionDirection.z;

                        // Changing direction of object[j]
                        objects[j].direction.x = -collisionDirection.x;
                        objects[j].direction.y = -collisionDirection.y;
                        objects[j].direction.z = -collisionDirection.z;

                        // Recalculating speed
                        objects[i].speed = speedDistribution(generator);
                        objects[j].speed = speedDistribution(generator);
                    }
                }
            }
            
            // Update the global ratation angle atomically
            rotationAngle.store(rotationAngle.load() + 1.0f); // Adjustable rotation speed
            if (rotationAngle.load() >= 360.0f) {
                rotationAngle.store(0.0f); // Reset to 0 if it passes 360
            }
        }
        // Sleep for a bit to simulate time passing
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
  
int main( void )
{
    std::thread moveThread(movementThread);
    
    bool lastGKeyState = false; // State of the G key in previous frame
    
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
    
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
    
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    
    // 추가한 부분
    GLuint JustGreen = glGetUniformLocation(programID, "JustGreen");
    
	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    

    // Two Triangles
    static const GLfloat g_vertex_buffer_data[] = {
        -10.0f, -10.0f, -7.0f,
         10.0f,  10.0f, -7.0f,
         10.0f, -10.0f, -7.0f,
        
        -10.0f, -10.0f, -7.0f,
         10.0f,  10.0f, -7.0f,
        -10.0f,  10.0f, -7.0f
    };



    
	GLuint vertexbuffer[2];
	glGenBuffers(2, vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    
	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
    
	do{

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
        
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		
		// Use our shader
		glUseProgram(programID);
        
        
        ////// Start of the rendering of the first object //////
        
        
		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
        
        ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(rotationAngle.load()), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축을 중심으로 회전
        ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(objects[0].x.load(), objects[0].y.load(), objects[0].z.load()));
        
        ////// Start of the rendering of the light //////
        
        // Light position located inside of the object1
        
        glm::vec4 lightPosInModelSpace1 = glm::vec4(0.0, 0.0, 0.0, 1.0); // Create vec with original position
        glm::vec4 lightPosInWorldSpace1 = ModelMatrix1 * lightPosInModelSpace1; // Implement current position
        
        glUniform3f(LightID, lightPosInWorldSpace1.x, lightPosInWorldSpace1.y, lightPosInWorldSpace1.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
        
        ////// End of the rendering of the light //////
        
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;
        
		// Send our transformation to the currently bound shader,
        glUniform1f(JustGreen, 0);
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
		glVertexAttribPointer(
			vertexPosition_modelspaceID, // The attribute we want to configure
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                       // The attribute we want to configure
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,        // The attribute we want to configure
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        
        
		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);


		////// End of rendering of the first object //////
        
		////// Start of the rendering of the second object //////
		
		
		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
        ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(rotationAngle.load()), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축을 중심으로 회전
        ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(objects[1].x.load(), objects[1].y.load(), objects[1].z.load()));
        
        ////// Start of the rendering of the light //////
        
        // Light position located inside of the object1
        
        glm::vec4 lightPosInModelSpace2 = glm::vec4(0.0, 0.0, 0.0, 1.0); // Create vec with original position
        glm::vec4 lightPosInWorldSpace2 = ModelMatrix2 * lightPosInModelSpace2; // Implement current position
        
        glUniform3f(LightID, lightPosInWorldSpace2.x, lightPosInWorldSpace2.y, lightPosInWorldSpace2.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
        
        ////// End of the rendering of the light //////
         
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;
        
		// Send our transformation to the currently bound shader,
        glUniform1f(JustGreen, 0);
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);

		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);


		////// End of rendering of the second object //////
        
        ////// Start of the rendering of the third object //////
    
        
        glm::mat4 ModelMatrix3 = glm::mat4(1.0);
        ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(rotationAngle.load()), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축을 중심으로 회전
        ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(objects[2].x.load(), objects[2].y.load(), objects[2].z.load()));
        
        ////// Start of the rendering of the light //////
        
        // Light position located inside of the object1
        
        glm::vec4 lightPosInModelSpace3 = glm::vec4(0.0, 0.0, 0.0, 1.0); // Create vec with original position
        glm::vec4 lightPosInWorldSpace3 = ModelMatrix3 * lightPosInModelSpace3; // Implement current position
        
        glUniform3f(LightID, lightPosInWorldSpace3.x, lightPosInWorldSpace3.y, lightPosInWorldSpace3.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
        
        ////// End of the rendering of the light //////
        
        glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;
        
        glUniform1f(JustGreen, 0);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);
        
        // The rest is exactly the same as the first object
        
        // 1rst attribute buffer : vertices
        //glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
        glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 2nd attribute buffer : UVs
        //glEnableVertexAttribArray(vertexUVID); // Already enabled
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 3rd attribute buffer : normals
        //glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);


        ////// End of rendering of the third object //////
        
        ////// Start of the rendering of the fourth object //////
        
        
        glm::mat4 ModelMatrix4 = glm::mat4(1.0);
        ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(rotationAngle.load()), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축을 중심으로 회전
        ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(objects[3].x.load(), objects[3].y.load(), objects[3].z.load()));
        
        ////// Start of the rendering of the light //////
        
        // Light position located inside of the object1
        
        glm::vec4 lightPosInModelSpace4 = glm::vec4(0.0, 0.0, 0.0, 1.0); // Create vec with original position
        glm::vec4 lightPosInWorldSpace4 = ModelMatrix4 * lightPosInModelSpace4; // Implement current position
        
        glUniform3f(LightID, lightPosInWorldSpace4.x, lightPosInWorldSpace4.y, lightPosInWorldSpace4.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
        
        ////// End of the rendering of the light //////
        
        glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;
        
        glUniform1f(JustGreen, 0);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix4[0][0]);
        
        // The rest is exactly the same as the first object
        
        // 1rst attribute buffer : vertices
        //glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
        glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 2nd attribute buffer : UVs
        //glEnableVertexAttribArray(vertexUVID); // Already enabled
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 3rd attribute buffer : normals
        //glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

        
        ////// End of rendering of the fourth object //////
        
        // Rectangle
        glUniform1f(JustGreen, 1);
        
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 Model_12      = glm::mat4(1.0f);
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP_12        = ProjectionMatrix * ViewMatrix * Model_12; // Remember, matrix multiplication is the other way around
        
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_12[0][0]);
        //glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model_12[0][0]);
        
        // 추가한거
        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
        glVertexAttribPointer(
            vertexPosition_modelspaceID, // The attribute we want to configure
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
        glEnableVertexAttribArray(vertexUVID);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            vertexUVID,                       // The attribute we want to configure
            2,                                // size : U+V => 2
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );
        
        // 추가한거
        glDrawArrays(GL_TRIANGLES, 0, 3 * 2); // 3 indices starting at 0 -> 1 triangle
        
		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
        
        // Check if the G key was pressed with toggle
        bool currentGKeyState = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
        if (currentGKeyState && !lastGKeyState) {
            // Toggle when the G key was pressed
            isGKeyPressed = !isGKeyPressed.load();
        }
        lastGKeyState = currentGKeyState;
        
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );
    
    isRunning = false;
    if (moveThread.joinable()) {
            moveThread.join();
        }
    
    // Cleanup VBO and shader
	glDeleteBuffers(1, vertexbuffer);
    //glDeleteBuffers(1, &vertexbuffer1);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
    
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

