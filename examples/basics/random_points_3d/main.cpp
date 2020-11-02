#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
using namespace gl;
//
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>
//
#include <lyrahgames/delaunay/delaunay.hpp>

float width = 800;
float height = 450;
glm::vec2 fov{M_PI_4 * width / height, M_PI_4};
glm::vec3 origin{};
glm::vec3 up{0, 1, 0};
glm::vec3 camera{5, 0.0f, M_PI_2};

using namespace std;
using namespace lyrahgames;
using delaunay::experimental_3d::point;

using vertex = point;

// Generate random points in sphere.
mt19937 rng{random_device{}()};
uniform_real_distribution<float> dist{0.0f, 1.0f};
const size_t samples = 1000;
vector<vertex> vertices(samples);
vector<uint32_t> elements{};
vector<uint32_t> surface_elements{};

void generate_random_points_and_triangulate() {
  for (auto& v : vertices) {
    const auto u = 2 * dist(rng) - 1;
    const auto phi = 2 * float(M_PI) * dist(rng);
    const auto r = pow(dist(rng), 1 / 3.0f);
    // const auto r = 1.0f;
    const auto p = sqrt(1 - u * u);
    v = r * point{cos(phi) * p, sin(phi) * p, u};
    // v = point{dist(rng), dist(rng), dist(rng)};
  }

  const auto tetrahedrons = delaunay::experimental_3d::triangulation(vertices);

  elements.resize(12 * tetrahedrons.size());
  size_t i = 0;
  for (const auto& t : tetrahedrons) {
    elements[i + 0] = t[0];
    elements[i + 1] = t[1];
    elements[i + 2] = t[2];

    elements[i + 3] = t[1];
    elements[i + 4] = t[2];
    elements[i + 5] = t[3];

    elements[i + 6] = t[2];
    elements[i + 7] = t[3];
    elements[i + 8] = t[0];

    elements[i + 9] = t[3];
    elements[i + 10] = t[0];
    elements[i + 11] = t[1];

    i += 12;
  }

  unordered_map<delaunay::experimental_3d::face, int,
                delaunay::experimental_3d::face::hash>
      tmp;
  for (const auto& t : tetrahedrons) {
    ++tmp[{t[0], t[1], t[2]}];
    ++tmp[{t[1], t[2], t[3]}];
    ++tmp[{t[2], t[3], t[0]}];
    ++tmp[{t[3], t[0], t[1]}];
  }

  surface_elements.clear();
  for (const auto& [f, i] : tmp) {
    if (i == 1) {
      surface_elements.push_back(f[0]);
      surface_elements.push_back(f[1]);
      surface_elements.push_back(f[2]);
    }
  }
}

bool draw_surface_elements = false;

int main(void) {
  cout  //
      << "Space:       Regenerate points and triangulation.\n"
      << "Left Mouse:  Turn around observer.\n"
      << "Mouse Wheel: Zoom in and out.\n"
      << "Escape:      Close the application.\n"
      << "s:           Toggle interior triangles.\n"
      << flush;

  generate_random_points_and_triangulate();

  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error{"GLFW Error " + to_string(error) + ": " + description};
  });

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  auto window = glfwCreateWindow(
      width, height, "3D Delaunay Triangulation Viewer", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);

  glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
    width = w;
    height = h;
    fov.x = fov.y * width / height;
  });
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
      generate_random_points_and_triangulate();
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(),
                   vertices.data(), GL_STATIC_DRAW);
      if (draw_surface_elements)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     surface_elements.size() * sizeof(uint32_t),
                     surface_elements.data(), GL_STATIC_DRAW);
      else
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     elements.size() * sizeof(uint32_t), elements.data(),
                     GL_STATIC_DRAW);
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
      draw_surface_elements = !draw_surface_elements;
      if (draw_surface_elements)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     surface_elements.size() * sizeof(uint32_t),
                     surface_elements.data(), GL_STATIC_DRAW);
      else
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     elements.size() * sizeof(uint32_t), elements.data(),
                     GL_STATIC_DRAW);
    }
  });
  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    camera.x *= exp(-0.1f * float(y));
  });

  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  const char* vertex_shader_text =
      "#version 330\n"
      "uniform mat4 MVP;"
      "attribute vec3 vPos;"
      "out vec3 color;"
      "void main() {"
      "  gl_Position = MVP * vec4(vPos, 1.0);"
      "  color = 0.5 * (vPos + 1);"
      "}";
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fragment_shader_text =
      "#version 330\n"
      "in vec3 color;"
      "void main() {"
      "  gl_FragColor = vec4(color, 1.0);"
      "}";
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);
  auto program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  auto mvp_location = glGetUniformLocation(program, "MVP");
  auto vpos_location = glGetAttribLocation(program, "vPos");

  // Create OpenGl vertex array for point rendering;
  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void*)0);

  GLuint element_buffer;
  glGenBuffers(1, &element_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(uint32_t),
               elements.data(), GL_STATIC_DRAW);

  glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPointSize(4.0f);
  glLineWidth(1.0f);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POINT_SPRITE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glm::vec2 old_mouse_pos{};
  glm::vec2 mouse_pos{};

  while (!glfwWindowShouldClose(window)) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    old_mouse_pos = mouse_pos;
    mouse_pos = glm::vec2(xpos, ypos);

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS) {
      const auto mouse_move = mouse_pos - old_mouse_pos;
      camera.z += mouse_move.x * 0.01;
      camera.y += mouse_move.y * 0.01;
      const constexpr float eye_altitude_max_abs = M_PI_2 - 0.0001f;
      camera.y = clamp(camera.y, -eye_altitude_max_abs, eye_altitude_max_abs);
    }

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4x4 m{1.0f};
    // m = rotate(m, (float)glfwGetTime(), glm::vec3(1, 1, 1));
    const auto v = glm::lookAt(
        origin + camera.x * glm::vec3{cos(camera.y) * cos(camera.z),  //
                                      sin(camera.y),                  //
                                      cos(camera.y) * sin(camera.z)},
        origin, up);
    const auto p = glm::perspective(fov.y, width / height, 0.1f, 100.f);
    glm::mat4 mvp = p * v * m;

    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

    if (draw_surface_elements)
      glDrawElements(GL_TRIANGLES, surface_elements.size(), GL_UNSIGNED_INT,
                     nullptr);
    else
      glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_POINTS, 0, vertices.size());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}