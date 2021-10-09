#include <iostream>
#include <fstream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.h"
#include "cloth.h"

// global variables for window control
bool pause = true;

// global variables for camera control
glm::vec3 camera_pos;
glm::vec3 camera_fwd;
glm::vec3 camera_up;

// global variables for OpenGL matrices
glm::mat4 proj_mat;
glm::mat4 view_mat;
glm::mat4 normal_mat;

GLuint proj_uniform_loc;
GLuint view_uniform_loc;
GLuint normal_uniform_loc;
GLuint cam_eye_uniform_loc;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        pause = !pause;
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        camera_pos.y += 0.5;
        view_mat = glm::lookAt(camera_pos,
                               camera_pos + camera_fwd,
                               camera_up);
        glUniformMatrix4fv(view_uniform_loc, 1, GL_FALSE, glm::value_ptr(view_mat));
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        camera_pos.y -= 0.5;
        view_mat = glm::lookAt(camera_pos,
                               camera_pos + camera_fwd,
                               camera_up);
        glUniformMatrix4fv(view_uniform_loc, 1, GL_FALSE, glm::value_ptr(view_mat));
    }
}

static void windowSizeCallback(GLFWwindow* window, int width, int height) {
    // Window was resized, recalculate the Projection matrix and resize the OpenGL viewport
    glViewport(0, 0, width, height);
    proj_mat = glm::infinitePerspective(3.14f/8.0f, width / (float)height, 1.0f);
    glUniformMatrix4fv(proj_uniform_loc, 1, GL_FALSE, glm::value_ptr(proj_mat));
}

std::string loadShaderSource(std::string file_name) {
    // try both the debug and install directories to find the file
    // start with debug
    std::ifstream shader_file(DEBUG_DIR + std::string("/") + file_name);
    if(!shader_file.good()) {
        shader_file.open(INSTALL_DIR + std::string("/") + file_name);
        if(!shader_file.good()) {
            exit(1);
        }
    };
	return std::string((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());
}

GLuint initShader(const char* v_src, const char* f_src) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	glShaderSource(vertexShader, 1, &v_src, NULL); 
	glCompileShader(vertexShader);
	
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (!status){
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
	}
	
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &f_src, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (!status){
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		std::cout << "Fragment Shader Compile Failed. Info:\n\n" << buffer << std::endl;
	}
	
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertexShader);
	glAttachShader(shader_program, fragmentShader);
	glBindFragDataLocation(shader_program, 0, "color");
	glLinkProgram(shader_program);
    // These aren't necessary anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shader_program;
}

int main(int argc, char* argv[]) {
    if(!glfwInit()) {
        return 1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_SAMPLES, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1080, 720, "Cloth Sim", NULL, NULL);
    if (!window) {
        // Window or OpenGL context creation failed
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSwapInterval(1);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Could not initalize OpenGL" << std::endl;
        return 1;
    };
    
    // Initalize OpenGL stuff here
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    std::string vertex_src = loadShaderSource(std::string("oren_nayar_vert.glsl"));
    std::string frag_src = loadShaderSource(std::string("oren_nayar_frag.glsl"));
    GLuint cloth_shader = initShader(vertex_src.c_str(), frag_src.c_str());
    Cloth cloth(40, 40, 100.0f, 20.0f, 1.0f, 0.25f, 1.5f, -4.5f, 32.0f, 0.5f);
    // Clamp the top of the cloth so we get a nice swing
    int val = 0;
	for (int j = 0; j < 40; ++j) {
		cloth.LockNode(j, 39, val++ % 1 == 0);
	}
    cloth.initGL(cloth_shader);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(cloth_shader);

    camera_pos = glm::vec3(20, 37.5, 25);
    camera_fwd = glm::vec3(-23.375, -11, -24.5);
    camera_up = glm::vec3(0, 1, 0);
    view_mat = glm::lookAt(camera_pos,
                           camera_pos + camera_fwd,
                           camera_up);
    proj_mat = glm::infinitePerspective(3.14f/8.0f, width / (float)height, 1.0f);
    normal_mat = glm::inverse(glm::transpose(view_mat));
    glm::vec3 light = glm::vec3(1.0f, -1.0f, -1.0f);
    
    view_uniform_loc = glGetUniformLocation(cloth_shader, "view_matrix");
    proj_uniform_loc = glGetUniformLocation(cloth_shader, "proj_matrix");
    normal_uniform_loc = glGetUniformLocation(cloth_shader, "normal_matrix");
    cam_eye_uniform_loc = glGetUniformLocation(cloth_shader, "camera_eye");
    
    glUniformMatrix4fv(view_uniform_loc, 1, GL_FALSE, glm::value_ptr(view_mat));
    glUniformMatrix4fv(proj_uniform_loc, 1, GL_FALSE, glm::value_ptr(proj_mat));
    glUniformMatrix4fv(normal_uniform_loc, 1, GL_FALSE, glm::value_ptr(normal_mat));
    glUniform3fv(glGetUniformLocation(cloth_shader, "light_dir"), 1, glm::value_ptr(light));
    glUniform3fv(cam_eye_uniform_loc, 1, glm::value_ptr(camera_pos));
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if(!pause)
            cloth.Update(0.02f);
        cloth.Draw();
        glfwSwapBuffers(window);
    }

    glDeleteProgram(cloth_shader);
    glfwTerminate();
    return 0;
}