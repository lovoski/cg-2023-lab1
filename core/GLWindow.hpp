#pragma once
// GLAD
#include <glad/glad.h>
// GLFW
#include <GLFW/glfw3.h>

#include <iostream>
namespace dym {
class GUI {
public:
  template <typename fb_cb, typename mouse_cb, typename scroll_cb>
  GUI(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT,
      fb_cb framebuffer_size_callback, mouse_cb mouse_callback,
      scroll_cb scroll_callback);
  ~GUI();

  template <typename func> void update(func F);

  // for ease, i define it in [public]
  GLFWwindow *window;

private:
  unsigned int scr_width, scr_height;
};

template <typename fb_cb, typename mouse_cb, typename scroll_cb>
GUI::GUI(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT,
         fb_cb framebuffer_size_callback, mouse_cb mouse_callback,
         scroll_cb scroll_callback)
    : scr_width(SCR_WIDTH), scr_height(SCR_HEIGHT) {
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  window = glfwCreateWindow(scr_width, scr_height, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR_NORMAL, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
  }
}

GUI::~GUI() {
  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
}

template <typename func> void GUI::update(func f) {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    f();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
  }
}
} // namespace dym