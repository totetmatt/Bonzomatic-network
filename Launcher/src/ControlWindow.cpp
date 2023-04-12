
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

#include "icons.h"

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

bool bModeNewCoder = false;
std::string sNewCoderName = "";
bool bModeOptions = false;

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


void PressMosaic() {
  StopDiaporama();
}

void ToggleDiaporama() {
  if (IsDiapoLaunched()) {
    StopDiaporama();
  }
  else {
    StartDiaporama();
  }
}

void EnterNewCoderMode() {
  sNewCoderName = "";
  bModeNewCoder = true;
}

void ValidNewCoder() {
  if (sNewCoderName.size() > 0) {
    printf("[LAUNCHER] Add coder %s \n", sNewCoderName.c_str());
    AddInstance(sNewCoderName);
    ChangeDisplay(DisplayAction::FirstDisplay);
  }
  bModeNewCoder = false;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  int FullIndexOffset = 0;
  if (mods & GLFW_MOD_CONTROL) FullIndexOffset = 10;
  if (mods & GLFW_MOD_SHIFT) FullIndexOffset = 20;
  if (mods & GLFW_MOD_ALT) FullIndexOffset = 30;
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (bModeNewCoder) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          bModeNewCoder = false;
          break;
        case GLFW_KEY_BACKSPACE:
          if (sNewCoderName.length() > 0) sNewCoderName.pop_back();
          break;
        case GLFW_KEY_ENTER:
          ValidNewCoder();
          break;
        default:
          break;
      }
    } else {
      switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_M:
          PressMosaic(); break;
        case GLFW_KEY_D: ToggleDiaporama(); break;
        case GLFW_KEY_A: EnterNewCoderMode(); break;
        case GLFW_KEY_O: bModeOptions=!bModeOptions; break;
        case GLFW_KEY_F11: ToggleTextEditor(); break;
        case GLFW_KEY_0: ToggleFullscreen(FullIndexOffset + 9); break;
        case GLFW_KEY_LEFT:
        case GLFW_KEY_UP:
          FullscreenPrev(); break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_DOWN:
          FullscreenNext(); break;
        default:
          if ((key >= GLFW_KEY_A) && (key <= GLFW_KEY_Z)) {

          }
          if ((key >= GLFW_KEY_1) && (key <= GLFW_KEY_9)) {
            ToggleFullscreen(FullIndexOffset + key - GLFW_KEY_1);
          }
          break;
      }
    }
  }
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
  // space are not allowed in names
  if (codepoint == GLFW_KEY_SPACE) return;
  // TODO: will only work for ascii characters
  if (codepoint >= 256) {
    sNewCoderName += unsigned char(codepoint % 256);
    sNewCoderName += unsigned char(codepoint / 256);
  } else {
    sNewCoderName += unsigned char(codepoint);
  }
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
bool mousebtn_pressrepeat_left = false;

float mousebtn_lastpress_left = 1000.0f;
float mousebtn_repeatdelay_left = 0.3f;

void mouse_tick(float ElapsedTime) {
  mousebtn_press_left = false;
  mousebtn_press_mid = false;
  mousebtn_press_right = false;

  // repeat
  if (mousebtn_left) {
    mousebtn_lastpress_left += ElapsedTime;
    if (mousebtn_lastpress_left >= mousebtn_repeatdelay_left) {
      mousebtn_lastpress_left -= mousebtn_repeatdelay_left;
      mousebtn_pressrepeat_left = true;
      mousebtn_repeatdelay_left = max(0.01f, mousebtn_repeatdelay_left * 0.8f);
    } else {
      mousebtn_pressrepeat_left = false;
    }
  }
  else {
    mousebtn_repeatdelay_left = 0.3f;
    mousebtn_pressrepeat_left = false;
  }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) { mousebtn_left = true; mousebtn_press_left = true; mousebtn_lastpress_left = 0.0f;  }
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

  unsigned char* buf = new unsigned char[len];
  fread(buf, 1, len, f);
  fclose(f);

  stbtt_BakeFontBitmap(buf, 0, FontSize, temp_bitmap, 512, 512, 0, 512, cdata); // no guarantee this fits!
  // can free ttf_buffer at this point
  glGenTextures(1, &ftex);
  glBindTexture(GL_TEXTURE_2D, ftex);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // can free buf at this point
  delete[] buf;
}

GLuint itex;
void InitIcons()
{ 
  glGenTextures(1, &itex);
  glBindTexture(GL_TEXTURE_2D, itex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, icons_width, icons_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, icons_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

float DrawText(float x, float y, const char *text)
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
  return x;
}

void DrawIcon(int x, int y, int icon_x, int icon_y)
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, itex);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
  int x1 = x;
  int y1 = y;
  int x2 = x + 32;
  int y2 = y + 32;
  float uv_dec_x = 1.0f / icons_cols;
  float uv_dec_y = 1.0f / icons_rows;
  float uv_x1 = icon_x * uv_dec_x;
  float uv_y1 = icon_y * uv_dec_y;
  float uv_x2 = (icon_x + 1) * uv_dec_x;
  float uv_y2 = (icon_y + 1) * uv_dec_y;
  glTexCoord2f(uv_x1, uv_y1); glVertex2f(x1, y1);
  glTexCoord2f(uv_x2, uv_y1); glVertex2f(x2, y1);
  glTexCoord2f(uv_x2, uv_y2); glVertex2f(x2, y2);
  glTexCoord2f(uv_x1, uv_y2); glVertex2f(x1, y2);
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

void FocusControlWindow() {
  glfwFocusWindow(mWindow);
}

struct ThemeColor {
  float R = 1.0f;
  float G = 1.0f;
  float B = 1.0f;
  float A = 1.0f;
};
ThemeColor ColorBackground = { 0.1,0.1,0.1,1 };
ThemeColor ColorText = { 1,1,1,1 };
ThemeColor ColorButton = { 0.2,0.2,0.2,1 };
ThemeColor ColorButtonUncheck = { 0.8,0.2,0.2,1 };
ThemeColor ColorButtonUncheckHover = { 1.0,0.5,0.5,1 };
ThemeColor ColorButtonBorder = { 0.5,0.5,0.5,1 };
ThemeColor ColorButtonBorderHover = { 0.7,0.7,0.7,1 };
ThemeColor ColorButtonBorderPress = { 1,1,1,1 };

ThemeColor ParseColor(const std::string& color) {
  if (color.size() < 6 || color.size() > 8) return { 1,1,1 };
  if (color.size() == 6)
  {
    std::string text = "0x" + color;
    unsigned int v = std::stoul(text, 0, 16);
    return { ((v & 0xFF0000) >> 16) / 255.0f, ((v & 0x00FF00) >> 8) / 255.0f, (v & 0x0000FF) / 255.0f, 1 };
  }
  else
  {
    std::string text = "0x" + color;
    unsigned int v = std::stoul(text, 0, 16);
    return { ((v & 0xFF0000) >> 16) / 255.0f, ((v & 0x00FF00) >> 8) / 255.0f, (v & 0x0000FF) / 255.0f, ((v & 0xFF000000) >> 24) / 255.0f };
  }
}

void SetColor(ThemeColor Col) {
  glColor4d(Col.R, Col.G, Col.B, Col.A);
}

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
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);  

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
  InitIcons();

  if (options.has<jsonxx::Object>("theme"))
  {
    const auto& theme = options.get<jsonxx::Object>("theme");
    if (theme.has<jsonxx::String>("background"))
      ColorBackground = ParseColor(theme.get<jsonxx::String>("background"));
    if (theme.has<jsonxx::String>("text"))
      ColorText = ParseColor(theme.get<jsonxx::String>("text"));
    if (theme.has<jsonxx::String>("button"))
      ColorButton = ParseColor(theme.get<jsonxx::String>("button"));
    if (theme.has<jsonxx::String>("buttonUncheck"))
      ColorButtonUncheck = ParseColor(theme.get<jsonxx::String>("buttonUncheck"));
    if (theme.has<jsonxx::String>("buttonBorder"))
      ColorButtonBorder = ParseColor(theme.get<jsonxx::String>("buttonBorder"));
    if (theme.has<jsonxx::String>("buttonBorderHover"))
      ColorButtonBorderHover = ParseColor(theme.get<jsonxx::String>("buttonBorderHover"));
    if (theme.has<jsonxx::String>("buttonBorderPress"))
      ColorButtonBorderPress = ParseColor(theme.get<jsonxx::String>("buttonBorderPress"));
  }

  glViewport(0, 0, nWidth, nHeight);

  return true;
}

bool CheckInside(int x, int y, int w, int h) {
  return mousepos_x > x && mousepos_x < (x + w)
      && mousepos_y > y && mousepos_y < (y + h);
}

bool Button(int x, int y, int w, int h, const char* Text, bool repeat=false) {

  bool IsInside = CheckInside(x, y, w, h);

  bool Action = mousebtn_press_left || (repeat && mousebtn_pressrepeat_left);

  if (IsInside) {
    if (Action) {
      SetColor(ColorButtonBorderPress);
    } else {
      SetColor(ColorButtonBorderHover);
    }
  } else {
    SetColor(ColorButtonBorder);
  }
  DrawQuad(x, y, w, h);

  SetColor(ColorButton);
  DrawQuad(x+5, y+5, w-10, h-10);

  SetColor(ColorText);
  DrawText(x+10, y + FontSize/4 + h/2, Text);

  return Action && IsInside;
}

bool ButtonCheck(int x, int y, int w, int h, const char* Text, bool Status) {

  bool IsInside = CheckInside(x, y, w, h);

  if (IsInside) {
    if (mousebtn_press_left) {
      SetColor(ColorButtonBorderPress);
    }
    else {
      SetColor(ColorButtonBorderHover);
    }
  }
  else {
    SetColor(ColorButtonBorder);
  }

  DrawQuad(x, y, w, h);
  if (Status) {
    SetColor(ColorButton);
  } else {
    SetColor(ColorButtonUncheck);
  }
  DrawQuad(x + 5, y + 5, w - 10, h - 10);

  SetColor(ColorText);
  DrawText(x + 10, y + FontSize / 4 + h / 2, Text);

  return mousebtn_press_left && IsInside;
}

bool ButtonIcon(int x, int y, int icon_x, int icon_y, bool repeat = false) {
  y += 4; // align with regular buttons
  int w = 22;
  int h = 25;
  bool IsInside = CheckInside(x, y, w, h);
  
  bool Action = mousebtn_press_left || (repeat && mousebtn_pressrepeat_left);
  
  if (IsInside) {
    if (Action) {
      SetColor(ColorButtonBorderPress);
    }
    else {
      SetColor(ColorButtonBorderHover);
    }
  }
  else {
    SetColor(ColorButtonBorder);
  }
  
  DrawIcon(x - 2, y - 2, icon_x, icon_y);

  return Action && IsInside;
}

bool ButtonCheckIcon(int x, int y, int icon_x, int icon_y, bool Status) {
  y += 4; // align with regular buttons
  int w = 22;
  int h = 25;
  bool IsInside = CheckInside(x, y, w, h);
  
  if (IsInside) {
    if (mousebtn_press_left) {
      SetColor(ColorButtonBorderPress);
    }
    else {
      if (Status) {
        SetColor(ColorButtonBorderHover);
      }
      else {
        SetColor(ColorButtonUncheckHover);
      }
    }
  }
  else {
    if (Status) {
      SetColor(ColorButtonBorder);
    }
    else {
      SetColor(ColorButtonUncheck);
    }
  }

  DrawIcon(x - 2, y - 2, icon_x, icon_y);

  return mousebtn_press_left && IsInside;
}

float TimeSinceStart = 0.0;
void InputText(int x, int y, int w, int h, const char* Text) {

  SetColor(ColorButtonBorderHover);
  DrawQuad(x, y, w, h);
  SetColor(ColorButton);
  DrawQuad(x + 5, y + 5, w - 10, h - 10);

  SetColor(ColorText);
  float TextEndPos = DrawText(x + 10, y + FontSize / 4 + h / 2, Text);
  // carret
  SetColor(((int)(TimeSinceStart*2))%2 == 0 ? ColorButtonBorder : ColorButtonBorderHover);
  DrawQuad(TextEndPos + 2, y - FontSize / 2 + h / 2, 4, FontSize);
}

template <typename T> std::string tostr(const T& t) {
  std::ostringstream os;
  os << t;
  return os.str();
}

float LastUIHeight = 0.0;
bool ScroolStartDrag = false;
int ScroolLastMouseY = 0;
void UpdateControlWindow(float ElapsedTime) {

  const float ar = (float)nWidth / (float)nHeight;

  glViewport(0, 0, nWidth, nHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
  glOrtho(0, nWidth, nHeight, 0, -100, 100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(ColorBackground.R, ColorBackground.G, ColorBackground.B, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Start drawing UI

  std::vector<class Instance*>& Instances = GetInstances();

  ScroolPositionY = max(0, min(ScroolPositionY, LastUIHeight - nHeight));
    
  int StartY = -ScroolPositionY;
  int PosY = StartY + 20;

  if (bModeNewCoder) {

    ///////////////
    // New coder dialog
    ///////////////

    InputText(20, PosY, nWidth - 40, 35, sNewCoderName.c_str());
    
    PosY += 50;
    if (Button(20, PosY, nWidth * 0.5 - 25, 35, "Cancel")) {
      bModeNewCoder = false;
    }
    if (Button(20 + nWidth * 0.5 - 15, PosY, nWidth * 0.5 - 25, 35, "Add Coder")) {
      ValidNewCoder();
    }
  } else {

    ///////////////
    // Buttons: add coder, save, options
    ///////////////

    int MenuWidth = nWidth - 40;
    if (Button(20, PosY, MenuWidth * 0.4, 35, "Add Coder")) {
      EnterNewCoderMode();
    }
    if (Button(20 + MenuWidth * 0.4, PosY, MenuWidth * 0.3, 35, "Save")) {
      extern void SaveConfigFile();
      SaveConfigFile();
    }
    if (ButtonCheck(20 + MenuWidth * 0.7, PosY, MenuWidth * 0.3, 35, "Options", !bModeOptions)) {
      bModeOptions = !bModeOptions;
    }
    PosY += 50;

    ///////////////
    // Diaporama buttons
    ///////////////

    extern float DiapoBPM;
    std::string DelayText = tostr(DiapoBPM);
    std::string DiapoTitle = "Diapo (" + DelayText + " bpm)";
    if (Button(20, PosY, nWidth - 117, 35, DiapoTitle.c_str())) {
      ToggleDiaporama();
    }
    if (ButtonIcon(nWidth - 95, PosY, 1, 0, true)) { // minus
      DiapoBPM = max(1, DiapoBPM - 1);
    }
    if (ButtonIcon(nWidth - 70, PosY, 0, 0, true)) { // plus
      DiapoBPM = DiapoBPM + 1;
    }
    extern bool DiapoInfiniteLoop;
    if (ButtonCheckIcon(nWidth - 45, PosY, 2, 0, !DiapoInfiniteLoop)) {
      DiapoInfiniteLoop = !DiapoInfiniteLoop;
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

    ///////////////
    // Mosaic button
    ///////////////

    int VisibleInstances = 0;
    for (int i = 0; i < Instances.size(); ++i) {
      Instance* Cur = Instances[i];
      if (Cur) {
        if(!Cur->IsHidden) ++VisibleInstances;
      }
    }
    std::string VisibleCountText = tostr(VisibleInstances);
    std::string InstanceCountText = tostr(Instances.size());
    std::string MosaicTitle = "Mosaic (" + VisibleCountText + "/" + InstanceCountText + ")";
    if (Button(20, PosY, nWidth - 40, 35, MosaicTitle.c_str())) {
      PressMosaic();
    }
    PosY += 50;
    
    ///////////////
    // Coders buttons
    ///////////////

    for (int i = 0; i < Instances.size(); ++i) {
      Instance* Cur = Instances[i];
      int NameRightSize = bModeOptions ? 170 : 70;
      if (Button(20, PosY, nWidth - NameRightSize, 35, Cur->CoderName.c_str())) {
        ToggleFullscreen(Cur);
      }
      if (ButtonCheckIcon(nWidth - NameRightSize + 24, PosY, 0, 2, !Cur->IsHidden)) { // show/hidden coder
        ToggleHidden(Cur);
      }
      if (bModeOptions) {
        bool UpdateDisplay = false;
        if (i > 0 && ButtonIcon(nWidth - 120, PosY, 1, 1)) { // move up coder
          int other = i - 1;
          if (other >= 0) {
            Instance* tmp = Instances[other];
            Instances[other] = Instances[i];
            Instances[i] = tmp;
            UpdateDisplay = true;
          }
        }
        if (i < Instances.size()-1 && ButtonIcon(nWidth - 95, PosY, 2, 1)) { // move down coder
          int other = i + 1;
          if (other < Instances.size()) {
            Instance* tmp = Instances[other];
            Instances[other] = Instances[i];
            Instances[i] = tmp;
            UpdateDisplay = true;
          }
        }
        if (UpdateDisplay) {
          extern bool GlobalIsFullscreen;
          if(!GlobalIsFullscreen) ChangeDisplay(DisplayAction::ShowMosaic);
        }
        if (ButtonIcon(nWidth - 70, PosY, 0, 1)) { // restart coder
          if (Cur) Cur->Restart();
        }
        if (ButtonIcon(nWidth - 45, PosY, 3, 1)) { // delete coder
          if (Cur) {
            RemoveInstance(Cur);
            ChangeDisplay(DisplayAction::FirstDisplay);
            break; // exit the loop so we don't mess up and delete several things
          }
        }
      }
      if (Cur->IsFullScreen) {
        glColor3d(0, 1, 0);
      }
      else {
        glColor3d(1, 0, 0);
      }
      DrawQuad(15, PosY, 5, 35);

      PosY += 40;
    }
    PosY += 20;

    ///////////////
    // Scrollbar
    ///////////////

    if (LastUIHeight > nHeight) {
      SetColor(ColorButton);
      DrawQuad(nWidth - 10, 0, 10, nHeight);

      float ScroolFactor = 1.0f / ((float)(LastUIHeight - nHeight));
      int ScroolButtonHeight = 35;
      int ScroolButtonPosY = ScroolPositionY * ScroolFactor * (nHeight - ScroolButtonHeight);
      if (Button(nWidth - 10, ScroolButtonPosY, 10, ScroolButtonHeight, "")) {
        ScroolStartDrag = true;
        ScroolLastMouseY = mousepos_y;
      }
      if (ScroolStartDrag) {
        int offset = mousepos_y - ScroolLastMouseY;
        ScroolPositionY += offset * (LastUIHeight - nHeight) / ((float)(nHeight - ScroolButtonHeight));
        ScroolLastMouseY = mousepos_y;
      }
    }

    if (!mousebtn_left) ScroolStartDrag = false;

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
  }

  LastUIHeight = PosY - StartY;

  TimeSinceStart += ElapsedTime;

  mouse_tick(ElapsedTime);
  
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