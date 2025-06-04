#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creates window instance, takes in 5 values but values 1-3 most important 
	GLFWwindow* window = glfwCreateWindow(800,600,"Tokyo Manji Gang",NULL,NULL);

	// error checking
	if(window == NULL){
		printf("FATAL ERROR BOBO KA");
		glfwTerminate();
		return -1;

	}
	
	glfwMakeContextCurrent(window);
	

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

	}
	glfwTerminate();
	return 0;
}