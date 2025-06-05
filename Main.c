#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int main() {
	//-------------------------------------------------------------//
	//                 OpenGL initialization stuff                 //
	//-------------------------------------------------------------//

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//-------------------------------------------------------------//
	//                  Window instance generation                 //
	//-------------------------------------------------------------//

	// Window parameters (Width, Height , Title , Null, Null)
	GLFWwindow* window = glfwCreateWindow(800,600,"Tokyo Manji Gang",NULL,NULL);


	// error checking if soomething goes 
	// wrong during window creation
	if(window == NULL){
		printf("FATAL ERROR BOBO KA NU GINAGAWA MO");
		glfwTerminate();
		return -1;

	}
	// sets the current context to the created window
	glfwMakeContextCurrent(window);

	//-------------------------------------------------------------//
	//                    GlAD Shenanigans UwU                     //
	//-------------------------------------------------------------//

	// Used to load glad and makes my life literally like, 1000x easier
	gladLoadGL();

	// sets GL viewport size
	glViewport(0, 0, 800, 600);

	// sets buffer color based on values in order of RGBA 
	// and executes the color change using the command bellow

	glClearColor(0.0f,0.0f,0.0f,1.0f); // changes buffer color to black 
	glClear(GL_COLOR_BUFFER_BIT);

	// swaps front and back buffer 
	glfwSwapBuffers(window);
	

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
};