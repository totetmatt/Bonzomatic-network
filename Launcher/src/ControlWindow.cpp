
#include "ControlWindow.h"

#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

#define GLEW_NO_GLU
#include "GL/glew.h"
#ifdef _WIN32
#include <GL/wGLew.h>
#endif

#include <string.h>
#include "Instances.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

static void error_callback(int error, const char *description) {
  switch (error) {
  case GLFW_API_UNAVAILABLE:
    printf("OpenGL is unavailable: ");
    break;
  case GLFW_VERSION_UNAVAILABLE:
    printf("OpenGL 4.1 (the minimum requirement) is not available: ");
    break;
  }
  printf("%s\n", description);
}

int nWidth = 300;
int nHeight = 800;
bool nSizeChanged = false;
void window_size_callback(GLFWwindow* window, int width, int height)
{
  // Avoid possible division by 0
  if (width < 1) width = 1;
  if (height < 1) height = 1;
  nWidth = width;
  nHeight = height;
  nSizeChanged = true;

  UpdateControlWindow(0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
}
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
}

int mousepos_x;
int mousepos_y;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  mousepos_x = xpos;
  mousepos_y = ypos;
}
bool mousebtn_left = false;
bool mousebtn_mid = false;
bool mousebtn_right = false;
bool mousebtn_press_left = false;
bool mousebtn_press_mid = false;
bool mousebtn_press_right = false;

void mouse_tick() {
  mousebtn_press_left = false;
  mousebtn_press_mid = false;
  mousebtn_press_right = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) { mousebtn_left = true; mousebtn_press_left = true; }
    if (action == GLFW_RELEASE) mousebtn_left = false;
  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (action == GLFW_PRESS) { mousebtn_mid = true; mousebtn_press_mid = true; }
    if (action == GLFW_RELEASE) mousebtn_mid = false;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS) { mousebtn_right = true; mousebtn_press_right = true; }
    if (action == GLFW_RELEASE) mousebtn_right = false;
  }
}

float ScroolPositionY = 0.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  ScroolPositionY -= yoffset * 10.0f;
}

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

stbtt_bakedchar cdata[512];
GLuint ftex;
int FontSize = 16;

void InitFont(std::string FontPath)
{
  unsigned char temp_bitmap[512 * 512];
  
  FILE* f = fopen(FontPath.c_str(), "rb");
  
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);

  unsigned char* buf = (unsigned char*)malloc(len);
  fread(buf, 1, len, f);
  fclose(f);

  stbtt_BakeFontBitmap(buf, 0, FontSize, temp_bitmap, 512, 512, 0, 512, cdata); // no guarantee this fits!
  // can free ttf_buffer at this point
  glGenTextures(1, &ftex);
  glBindTexture(GL_TEXTURE_2D, ftex);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

  // can free temp_bitmap at this point
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void DrawText(float x, float y, const char *text)
{
  // assume orthographic projection with units = screen pixels, origin at top left
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, ftex);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
  while (*text) {
    if (*text < 512) {
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, 512, 512, *text, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
      glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
      glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
      glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
      glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
    }
    ++text;
  }
  glEnd();
}

void DrawQuad(int x, int y, int w, int h) {
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glVertex3f(x, y, -10);
  glVertex3f(x+w, y, -10);
  glVertex3f(x+w, y+h, -10);
  glVertex3f(x, y+h, -10);
  glEnd();
}

GLFWwindow* mWindow;

bool InitControlWindow(jsonxx::Object options) {
  

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
  {
    printf("[Renderer] GLFW init failed\n");
    return false;
  }
  printf("[GLFW] Version String: %s\n", glfwGetVersionString());
  
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

  // Prevent fullscreen window minimize on focus loss
  glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
  
  //GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  //const GLFWvidmode* mode = glfwGetVideoMode(monitor);

  mWindow = glfwCreateWindow(nWidth, nHeight, "BONZO Control", NULL, NULL);
  if (!mWindow)
  {
    printf("[GLFW] Window creation failed\n");
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(mWindow);

  glfwSetKeyCallback(mWindow, key_callback);
  glfwSetCharCallback(mWindow, character_callback);
  glfwSetCursorPosCallback(mWindow, cursor_position_callback);
  glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
  glfwSetScrollCallback(mWindow, scroll_callback);
  glfwSetWindowSizeCallback(mWindow, window_size_callback);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    printf("[GLFW] glewInit failed: %s\n", glewGetErrorString(err));
    glfwTerminate();
    return false;
  }
  printf("[GLFW] Using GLEW %s\n", glewGetString(GLEW_VERSION));
  glGetError(); // reset glew error

  glfwSwapInterval(1);

#ifdef _WIN32
  wglSwapIntervalEXT(1);
#endif

  std::string FontPath = "ProFontWindows.ttf";
  if (options.has<jsonxx::Object>("font"))
  {
    if (options.get<jsonxx::Object>("font").has<jsonxx::Number>("size"))
      FontSize = options.get<jsonxx::Object>("font").get<jsonxx::Number>("size");
    if (options.get<jsonxx::Object>("font").has<jsonxx::String>("file"))
    {
      FontPath = options.get<jsonxx::Object>("font").get<jsonxx::String>("file");
    }
  }
  InitFont(FontPath);

  glViewport(0, 0, nWidth, nHeight);

  return true;
}

bool CheckInside(int x, int y, int w, int h) {
  return mousepos_x > x && mousepos_x < (x + w)
      && mousepos_y > y && mousepos_y < (y + h);
}

bool Button(int x, int y, int w, int h, const char* Text) {

  bool IsInside = CheckInside(x, y, w, h);

  if (IsInside) {
    if (mousebtn_press_left) {
      glColor3d(1.0, 1.0, 1.0);
    } else {
      glColor3d(1.0, 0.5, 0.5);
    }
  } else {
    glColor3d(0.5, 0.5, 0.5);
  }
  DrawQuad(x, y, w, h);
  glColor3d(0.2, 0.2, 0.2);
  DrawQuad(x+5, y+5, w-10, h-10);

  glColor3d(1, 1, 1);
  DrawText(x+10, y + FontSize/4 + h/2, Text);

  return mousebtn_press_left && IsInside;
}

bool ButtonCheck(int x, int y, int w, int h, const char* Text, bool Status) {

  bool IsInside = CheckInside(x, y, w, h);

  if (IsInside) {
    if (mousebtn_press_left) {
      glColor3d(1.0, 1.0, 1.0);
    }
    else {
      glColor3d(1.0, 0.5, 0.5);
    }
  }
  else {
    glColor3d(0.5, 0.5, 0.5);
  }
  DrawQuad(x, y, w, h);
  if (Status) {
    glColor3d(0.2, 0.2, 0.2);
  } else {
    glColor3d(0.8, 0.2, 0.2);
  }
  DrawQuad(x + 5, y + 5, w - 10, h - 10);

  glColor3d(1, 1, 1);
  DrawText(x + 10, y + FontSize / 4 + h / 2, Text);

  return mousebtn_press_left && IsInside;
}

template <typename T> std::string tostr(const T& t) {
  std::ostringstream os;
  os << t;
  return os.str();
}

float LastUIHeight = 0.0;
float tmptime = 0.0;
void UpdateControlWindow(float ElapsedTime) {

  const float ar = (float)nWidth / (float)nHeight;

  glViewport(0, 0, nWidth, nHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
  glOrtho(0, nWidth, nHeight, 0, -100, 100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.1, 0.1, 0.1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /*
  glColor3d(0.5, 0, 0.5);
  DrawQuad(0, 0, 400, 400);

  glColor3d(0, 0, 0.5);
  DrawQuad(20, 20, 360, 20);

  glColor3d(1,1,1);
  my_stbtt_print(20, 40, "Hello World");
  glColor3d(1, 0, 1);
  my_stbtt_print(20, 60, "Hello World");
  */

  std::vector<class Instance*>& Instances = GetInstances();

  ScroolPositionY = max(0, min(ScroolPositionY, LastUIHeight - nHeight));
    
  int StartY = -ScroolPositionY;
  int PosY = StartY + 20;
  if (Button(20, PosY, nWidth - 40, 35, "Mosaic")) {
    StopDiaporama();
  }
  PosY += 50;
  if (Button(20, PosY, nWidth - 40, 35, "Diaporama")) {
    if (IsDiapoLaunched()) {
      StopDiaporama();
    } else {
      StartDiaporama();
    }
  }
  UpdateDiaporama(ElapsedTime);
  if (IsDiapoLaunched()) {
    glColor3d(0, 1, 0);
  }
  else {
    glColor3d(1, 0, 0);
  }
  DrawQuad(15, PosY, 5, 35);
  PosY += 50;

  for(int i=0; i< Instances.size(); ++i) {
    Instance* Cur = Instances[i];
    if (Button(20, PosY, nWidth-60, 35, Cur->CoderName.c_str())) {
      if (Cur->IsFullScreen) {
        ChangeDisplay(DisplayAction::ShowMosaic, Cur);
      } else {
        ChangeDisplay(DisplayAction::SetFullscreen, Cur);
      }
    }
    if (ButtonCheck(nWidth-40, PosY, 30, 35, "X", !Cur->IsHidden)) {
      ToggleHidden(Cur);
    }
    if (Cur->IsFullScreen) {
      glColor3d(0, 1, 0);
    } else {
      glColor3d(1, 0, 0);
    }
    DrawQuad(15, PosY, 5, 35);
    
    PosY += 40;
  }
  PosY += 40;
  /*
  PosY += 20;
  tmptime += ElapsedTime;
  std::string TimeStr = std::string("Time: ") + tostr(tmptime);
  glColor3d(1, 1, 1);
  DrawText(20, PosY, TimeStr.c_str());
  PosY += 20;
  extern float DiapoCurrentTime;
  TimeStr = std::string("Cur: ") + tostr(DiapoCurrentTime);
  glColor3d(1, 1, 1);
  DrawText(20, PosY, TimeStr.c_str());
  */

  // cursor
  //glColor3d(1, 1, 1);
  //DrawQuad(mousepos_x, mousepos_y, 20, 20);

  LastUIHeight = PosY - StartY;

  mouse_tick();
  
  glfwSwapBuffers(mWindow);
  glfwPollEvents();
}

bool WantsToQuit()
{
  return glfwWindowShouldClose(mWindow);
}

void CloseControlWindow() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}