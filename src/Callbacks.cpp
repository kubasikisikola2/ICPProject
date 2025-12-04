#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "App.hpp"

void App::glfw_cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

	// ignornig first movement to prevent flicks
	if (app->firstMouse)
	{
		app->cursorLastX = xpos;
		app->cursorLastY = ypos;
		app->firstMouse = false;
		return;  
	}

	app->camera.ProcessMouseMovement(xpos - app->cursorLastX, (ypos - app->cursorLastY) * -1.0);
	app->cursorLastX = xpos;
	app->cursorLastY = ypos;
}

void App::glfw_error_callback(int error, const char* description)
{
	std::cerr << "GLFW error: " << description << std::endl;
}

void App::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			// Exit The App
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_V:
			// Vsync on/off
			this_inst->is_vsync_on = !this_inst->is_vsync_on;
			glfwSwapInterval(this_inst->is_vsync_on);
			std::cout << "VSync: " << this_inst->is_vsync_on << "\n";
			break;
		case GLFW_KEY_TAB:
			this_inst->show_imgui = !this_inst->show_imgui;
			break;
		case GLFW_KEY_P:
			this_inst->paused_by_key = !this_inst->paused_by_key;
			break;
		default:
			break;
		}
	}
}

void App::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
	this_inst->FOV_degrees += 10 * yoffset; // yoffset is mostly +1 or -1; one degree difference in fov is not visible
	this_inst->FOV_degrees = std::clamp(this_inst->FOV_degrees, 20.0f, 170.0f); // limit FOV to reasonable values...

	this_inst->update_projection_matrix();
}

void App::glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
	this_inst->viewport_width = width;
	this_inst->viewport_height = height;

	this_inst->screenshot.create(height, width, CV_8UC3);
	// set viewport
	glViewport(0, 0, width, height);
	//now your canvas has [0,0] in bottom left corner, and its size is [width x height] 

	this_inst->update_projection_matrix();
}

void App::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT: {
			int mode = glfwGetInputMode(window, GLFW_CURSOR);
			if (mode == GLFW_CURSOR_NORMAL) {
				// we are outside of application, catch the cursor
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else {
				// we are already inside our game: shoot, click, etc.
				std::cout << "Bang!\n";
			}
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
            // release the cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		default:
			break;
		}
	}
}

void GLAPIENTRY App::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		default: return "Unknown";
		}
		}();

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: return "Unknown";
		}
		}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: return "Unknown";
		}
		}();

	std::cout << "[GL CALLBACK]: " <<
		"source = " << src_str <<
		", type = " << type_str <<
		", severity = " << severity_str <<
		", ID = '" << id << '\'' <<
		", message = '" << message << '\'' << std::endl;
}
