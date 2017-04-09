// g++ -std=c++11 -o spielplatz `pkg-config --cflags --libs glew cairo` -lglfw -lstdc++ main.cc

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cairo.h>

#include <time.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const char *vs_simple[] = { R"SHDR(
#version 330 core

in vec4 coord;
out vec2 texpos;

uniform sampler2D tex;
//uniform mat4 vp;

void main(void) {
  gl_Position = vec4(coord.xy, 0, 1);
  texpos.s =     (0.5 + coord.x);
  texpos.t = 1 - (0.5 + coord.y);
}

)SHDR" };


static const char *fs_simple[] = { R"SHDR(
#version 330 core

in vec2 texpos;
uniform sampler2D tex;

out vec4 finalColor;

void main(void) {
    finalColor = texture2D(tex, texpos.st);
}

)SHDR" };

//shamelessly borrowed from cairo examples

static void
draw_clock (cairo_t *cr)
{
  time_t t;
  struct tm *tm;
  double seconds, minutes, hours;


  t = time(NULL);
  tm = localtime(&t);


  seconds = tm->tm_sec * M_PI / 30;
  minutes = tm->tm_min * M_PI / 30;
  hours = tm->tm_hour * M_PI / 6;


  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);


  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width(cr, 0.1);


  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_translate(cr, 0.5, 0.5);
  cairo_arc(cr, 0, 0, 0.4, 0, M_PI * 2);
  cairo_stroke(cr);

  /* seconds */
  cairo_set_source_rgba(cr, 1, 1, 1, 0.6);
  cairo_arc(cr, sin(seconds) * 0.4, -cos(seconds) * 0.4,
            0.05, 0, M_PI * 2);
  cairo_fill(cr);

  /* minutes */
  cairo_set_source_rgba(cr, 0.2, 0.2, 1, 0.6);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, sin(minutes) * 0.4, -cos(minutes) * 0.4);
  cairo_stroke(cr);

  /* hours     */
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, sin(hours) * 0.2, -cos(hours) * 0.2);
  cairo_stroke(cr);
}


static void
fb_size_gl_cb(GLFWwindow *gl_win, int width, int height)
{
  glViewport(0, 0, width, height);
}

int
main(int argc, char **argv) {

  GLenum err;

  if (!glfwInit()) {
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("Profile Error...%d \n", err);
    return -2;
  }

  GLFWwindow *wnd = glfwCreateWindow(600, 600, "Test", NULL, NULL);

  if (wnd == NULL) {
    return -1;
  }

  glfwMakeContextCurrent(wnd);

  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK) {
    return -1;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_MULTISAMPLE);

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, vs_simple, NULL);
  glCompileShader(vs);
  GLint status;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    printf("vs shader error\n");
    return -1;
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, fs_simple, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    printf("fs shader error\n");
    return -1;
  }

  GLuint prg = glCreateProgram();
  glAttachShader(prg, vs);
  glAttachShader(prg, fs);
  glLinkProgram(prg);

  glGetProgramiv(prg, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    printf("Could not link shader...%d \n", status);
    return -2;
  }

  glfwSetFramebufferSizeCallback(wnd, fb_size_gl_cb);

  // lets draw something      x     y
  GLfloat box_coords[] = { -0.5f,  0.5f,
                           -0.5f, -0.5f,
                            0.5f, -0.5f,

                            0.5f,  0.5f,
                           -0.5f,  0.5f,
                            0.5f, -0.5f};

  GLuint bb;
  glGenBuffers(1, &bb);
  glBindBuffer(GL_ARRAY_BUFFER, bb);
  glBufferData(GL_ARRAY_BUFFER, sizeof(box_coords)*sizeof(GLfloat), &box_coords, GL_STATIC_DRAW);


  GLuint vb;
  glGenVertexArrays(1, &vb);
  glBindVertexArray(vb);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  glBindVertexArray(0);

  // textures
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


  int w = 1000, h = 1000;
  int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, w);
  unsigned char buffer[h * stride] = {0, };

  cairo_surface_t *surface =
    cairo_image_surface_create_for_data (buffer,
                                         CAIRO_FORMAT_ARGB32,
                                         w,
                                         h,
                                         stride);

  if (err != GL_NO_ERROR) {
    printf("Error...%d\n", err);
  }

  double tick = glfwGetTime();
  int n_frames = 0;

  setbuf(stdout, NULL);
  glfwSwapInterval(0);
  while (glfwWindowShouldClose(wnd) == 0) {
    //glm::mat4 vp;

    double now = glfwGetTime();
    n_frames++;
    double elapsed = now - tick;
    if (elapsed >= 1.0) {

      printf("\r%f ms/frame [%f fps]",
             elapsed * 1000.0/double(n_frames),
             n_frames/elapsed);
      n_frames = 0;
      tick += elapsed;
    }

    cairo_t *cr = cairo_create (surface);

    cairo_scale(cr, w, h);
    draw_clock(cr);
    cairo_destroy(cr);
    cairo_surface_flush(surface);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    float data[4] = {1.0f, 0.5f, 0.7f, 1.0f};

    glClearColor(1.f, 1.f, 1.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(prg);

    //glUniform4fv(glGetUniformLocation(prg, "plot_color"), 1, data);
    glUniform1i(glGetUniformLocation(prg, "tex"), 0);
    //glUniformMatrix4fv(glGetUniformLocation(prg, "vp"), 1, GL_FALSE, glm::value_ptr(vp));

    glBindVertexArray(vb);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);

    glfwSwapBuffers(wnd);
    glfwPollEvents();

    err = glGetError();
    if (err != GL_NO_ERROR) {
      printf("Error...%d\n", err);
    }
  }

  cairo_surface_destroy (surface);

  glfwTerminate();
  return 0;
}
