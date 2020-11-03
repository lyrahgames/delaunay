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
glm::vec3 camera{20, 0.0f, M_PI_2};

struct vertex {
  float x, y, z;
  float r{}, g{}, b{};
};

using namespace std;
using namespace lyrahgames;
using delaunay::experimental_3d::point;

// Generate random points in sphere.
mt19937 rng{random_device{}()};
uniform_real_distribution<float> dist{0, 1};
const size_t samples = 100000;
array<point, 4> tetrahedron;
vector<vertex> vertices(samples);
vector<vertex> tetra_vertices(4);

void generate_tetrahedron_and_check_points() {
  const auto radius = dist(rng);
  const auto center = point{dist(rng), dist(rng), dist(rng)};

  for (auto& v : vertices) {
    const auto u = 2 * dist(rng) - 1;
    const auto phi = 2 * float(M_PI) * dist(rng);
    const auto r = radius * pow(dist(rng), 1 / 3.0f);
    // const auto r = 1.0f;
    const auto p = sqrt(1 - u * u);
    v = vertex{r * cos(phi) * p + center.x, r * sin(phi) * p + center.y,
               r * u + center.z};
  }

  tetrahedron = delaunay::experimental_3d::bounding_tetrahedron(
      {center, radius * radius});
  // Tetrahedron
  for (int i = 0; i < 4; ++i) {
    // tetrahedron[i] = point{dist(rng), dist(rng), dist(rng)};
    tetra_vertices[i] =
        vertex{tetrahedron[i].x, tetrahedron[i].y, tetrahedron[i].z,
               tetrahedron[i].x, tetrahedron[i].y, tetrahedron[i].z};
  }
}

int main(void) {
  cout  //
      << "Space:       Regenerate tetrahedron and points.\n"
      << "Left Mouse:  Turn around observer.\n"
      << "Mouse Wheel: Zoom in and out.\n"
      << "Escape:      Close the application.\n"
      << flush;

  generate_tetrahedron_and_check_points();

  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error{"GLFW Error " + to_string(error) + ": " + description};
  });

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  auto window = glfwCreateWindow(width, height, "Bounding Tetrahedron Viewer",
                                 nullptr, nullptr);
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
      generate_tetrahedron_and_check_points();
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(),
                   vertices.data(), GL_STATIC_DRAW);
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
      "attribute vec3 vCol;"
      "out vec3 color;"
      "void main() {"
      "  gl_Position = MVP * vec4(vPos, 1.0);"
      "  color = vCol;"
      "}";
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fragment_shader_text =
      "#version 330\n"
      "in vec3 color;"
      "void main() {"
      "  gl_FragColor = vec4(color, 0.5);"
      "}";
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);
  auto program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  auto mvp_location = glGetUniformLocation(program, "MVP");
  auto vpos_location = glGetAttribLocation(program, "vPos");
  auto vcol_location = glGetAttribLocation(program, "vCol");

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
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void*)12);

  GLuint element_buffer;
  glGenBuffers(1, &element_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
  vector<uint32_t> elements{0, 1, 2,  //
                            0, 1, 3,  //
                            0, 2, 3,  //
                            1, 2, 3};
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(uint32_t),
               elements.data(), GL_STATIC_DRAW);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPointSize(4.0f);
  glLineWidth(4.0f);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POINT_SPRITE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_POINTS, 0, vertices.size());

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * tetra_vertices.size(),
                 tetra_vertices.data(), GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}