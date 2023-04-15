
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
int NewCoderMaxLength = 100;

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
  ShowMosaic();
}

void ToggleDiaporama() {
  if (IsDiapoLaunched()) {
    StopDiaporama();
    ShowMosaic();
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
    FocusControlWindow();
    RefreshDisplay();
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
        case GLFW_KEY_V:
          if (mods & GLFW_MOD_CONTROL) {
            std::string paste_string = glfwGetClipboardString(window);
            for (int i = 0; i < min(paste_string.length(), NewCoderMaxLength); ++i) {
              if (paste_string[i] != ' ') {
                sNewCoderName += paste_string[i];
              }
            }
          }
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
        case GLFW_KEY_R: RandomFullscreen(); break;
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
  if (!bModeNewCoder || sNewCoderName.length() >= NewCoderMaxLength) {
    return;
  }
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

float DrawLabel(float x, float y, std::string text)
{
  // assume orthographic projection with units = screen pixels, origin at top left
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, ftex);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
  for(int i=0; i < text.length(); ++i) {
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(cdata, 512, 512, text[i], &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
    glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
    glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
    glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
    glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
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
ThemeColor ColorButton = { 0.4,0.4,0.4,1 };
ThemeColor ColorButtonHover = { 0.6,0.6,0.6,1 };
ThemeColor ColorButtonPress = { 1,1,1,1 };
ThemeColor ColorButtonUncheck = { 0.8,0.2,0.2,1 };
ThemeColor ColorButtonUncheckHover = { 1.0,0.5,0.5,1 };
ThemeColor ColorScrollbar = { 0.25,0.25,0.25,1 };

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
    if (theme.has<jsonxx::String>("buttonHover"))
      ColorButtonHover = ParseColor(theme.get<jsonxx::String>("buttonHover"));
    if (theme.has<jsonxx::String>("buttonPress"))
      ColorButtonPress = ParseColor(theme.get<jsonxx::String>("buttonPress"));
    if (theme.has<jsonxx::String>("buttonUncheck"))
      ColorButtonUncheck = ParseColor(theme.get<jsonxx::String>("buttonUncheck"));
    if (theme.has<jsonxx::String>("buttonUncheckHover"))
      ColorButtonUncheckHover = ParseColor(theme.get<jsonxx::String>("buttonUncheckHover"));
    if (theme.has<jsonxx::String>("scrollbar"))
      ColorScrollbar = ParseColor(theme.get<jsonxx::String>("scrollbar"));
  }

  glViewport(0, 0, nWidth, nHeight);

  return true;
}

bool CheckInside(int x, int y, int w, int h) {
  return mousepos_x > x && mousepos_x < (x + w)
      && mousepos_y > y && mousepos_y < (y + h);
}

bool BlockAlignIsLeft = true;
int BlockMarginLeft = 20;
int BlockMarginRight = 10;
int BlockCurrentLeft = 0;
int BlockCurrentRight = 0;
int BlockHeight = 40;
int BlockButtonHeight = 30;
int BlockPositionY = 10;
int BlockIconOffset = 2;

void BlockInit(int x, int y, int w, int h) {
  BlockMarginLeft = x;
  BlockMarginRight = nWidth - BlockMarginLeft - w;
  BlockPositionY = y;
  BlockHeight = h;
  BlockButtonHeight = h - 10;
  BlockCurrentLeft = BlockMarginLeft;
  BlockCurrentRight = nWidth - BlockMarginRight;
}

void BlockNextLine() {
  BlockPositionY += BlockHeight;
  BlockCurrentLeft = BlockMarginLeft;
  BlockCurrentRight = nWidth - BlockMarginRight;
}

void BlockNextLine(int NewHeight) {
  BlockHeight = NewHeight;
  BlockNextLine();
}

void BlockAlignLeft() { BlockAlignIsLeft = true; }
void BlockAlignRight() { BlockAlignIsLeft = false; }

int BlockPush(int w) {
  if (BlockAlignIsLeft) {
    int BlockStart = BlockCurrentLeft;
    BlockCurrentLeft += w;
    return BlockStart;
  } else {
    BlockCurrentRight -= w;
    return BlockCurrentRight;
  }
}

void BlockVerticalSeparator(int w) {
  BlockPush(w);
}

void BlockVerticalSeparator() {
  BlockVerticalSeparator(4);
}

float DrawLabel(std::string Text)
{
  int w = BlockCurrentRight - BlockCurrentLeft;
  return DrawLabel(BlockPush(w), BlockPositionY, Text);
}

float DrawLabel(int w, std::string Text)
{
  return DrawLabel(BlockPush(w), BlockPositionY, Text);
}

float DrawLabelRight(float x, float y, std::string Text)
{
  return DrawLabel(x - (Text.length() + 1)* FontSize / 2, y, Text);
}

float DrawLabelRight(std::string Text)
{
  int w = BlockCurrentRight - BlockCurrentLeft;
  return DrawLabelRight(BlockPush(w), BlockPositionY, Text);
}

float DrawLabelRight(int w, std::string Text)
{
  return DrawLabelRight(BlockPush(w), BlockPositionY, Text);
}

bool Button(int x, int y, int w, int h, std::string Text, bool repeat=false) {

  bool IsInside = CheckInside(x, y, w, h);

  bool Action = mousebtn_press_left || (repeat && mousebtn_pressrepeat_left);

  if (IsInside) {
    if (Action) {
      SetColor(ColorButtonPress);
    } else {
      SetColor(ColorButtonHover);
    }
  } else {
    SetColor(ColorButton);
  }
  DrawQuad(x, y, w, h);

  SetColor(ColorText);
  DrawLabel(x+10, y + FontSize/4 + h/2, Text);

  return Action && IsInside;
}

bool Button(std::string Text, bool repeat = false) {
  int w = BlockCurrentRight - BlockCurrentLeft;
  return Button(BlockPush(w), BlockPositionY, w, BlockButtonHeight, Text, repeat);
}

bool Button(int w, std::string Text, bool repeat = false) {
  return Button(BlockPush(w), BlockPositionY, w, BlockButtonHeight, Text, repeat);
}

bool ButtonCheck(int x, int y, int w, int h, std::string Text, bool Status) {

  bool IsInside = CheckInside(x, y, w, h);
  
  if (IsInside) {
    if (mousebtn_press_left) {
      SetColor(ColorButtonPress);
    }
    else {
      if (Status) {
        SetColor(ColorButtonHover);
      }
      else {
        SetColor(ColorButtonUncheckHover);
      }
    }
  }
  else {
    if (Status) {
      SetColor(ColorButton);
    }
    else {
      SetColor(ColorButtonUncheck);
    }
  }

  DrawQuad(x, y, w, h);

  SetColor(ColorText);
  DrawLabel(x + 10, y + FontSize / 4 + h / 2, Text);

  return mousebtn_press_left && IsInside;
}

bool ButtonCheck(std::string Text, bool Status) {
  int w = BlockCurrentRight - BlockCurrentLeft;
  return ButtonCheck(BlockPush(w), BlockPositionY, w, BlockButtonHeight, Text, Status);
}

bool ButtonCheck(int w, std::string Text, bool Status) {
  return ButtonCheck(BlockPush(w), BlockPositionY, w, BlockButtonHeight, Text, Status);
}

bool ButtonIcon(int x, int y, int icon_x, int icon_y, bool repeat = false) {
  int w = 22;
  int h = 25;
  bool IsInside = CheckInside(x, y, w, h);
  
  bool Action = mousebtn_press_left || (repeat && mousebtn_pressrepeat_left);
  
  if (IsInside) {
    if (Action) {
      SetColor(ColorButtonPress);
    }
    else {
      SetColor(ColorButtonHover);
    }
  }
  else {
    SetColor(ColorButton);
  }
  
  DrawIcon(x - 2, y - 2, icon_x, icon_y);

  return Action && IsInside;
}

bool ButtonIcon(int icon_x, int icon_y, bool repeat = false) {
  int w = 22;
  return ButtonIcon(BlockPush(w + 3), BlockPositionY + BlockIconOffset, icon_x, icon_y, repeat);
}

bool ButtonCheckIcon(int x, int y, int icon_x, int icon_y, bool Status) {
  int w = 22;
  int h = 25;
  bool IsInside = CheckInside(x, y, w, h);
  
  if (IsInside) {
    if (mousebtn_press_left) {
      SetColor(ColorButtonPress);
    }
    else {
      if (Status) {
        SetColor(ColorButtonHover);
      }
      else {
        SetColor(ColorButtonUncheckHover);
      }
    }
  }
  else {
    if (Status) {
      SetColor(ColorButton);
    }
    else {
      SetColor(ColorButtonUncheck);
    }
  }

  DrawIcon(x - 2, y - 2, icon_x, icon_y);

  return mousebtn_press_left && IsInside;
}

bool ButtonCheckIcon(int icon_x, int icon_y, bool Status) {
  int w = 22;
  return ButtonCheckIcon(BlockPush(w + 3), BlockPositionY + BlockIconOffset, icon_x, icon_y, Status);
}

float TimeSinceStart = 0.0;
void InputText(int x, int y, int w, int h, const char* InText) {

  std::string Text = InText;
  const int MaxDisplayCharacter = max(1, w * 2 / FontSize -4);
  if (Text.length() > MaxDisplayCharacter) {
    Text = "-" + Text.substr(Text.length() - MaxDisplayCharacter, MaxDisplayCharacter);
  }

  SetColor(ColorButtonHover);
  DrawQuad(x, y, w, h);
  SetColor(ColorButton);
  DrawQuad(x + 5, y + 5, w - 10, h - 10);

  SetColor(ColorText);
  float TextEndPos = DrawLabel(x + 10, y + FontSize / 4 + h / 2, Text);
  // carret
  SetColor(((int)(TimeSinceStart*2))%2 == 0 ? ColorButton : ColorButtonHover);
  DrawQuad(TextEndPos + 2, y - FontSize / 2 + h / 2, 4, FontSize);
}

void InputText(const char* InText) {
  int w = BlockCurrentRight - BlockCurrentLeft;
  int h = BlockHeight - 10;
  InputText(BlockPush(w), BlockPositionY, w, h, InText);
}

void InputText(int w, int h, const char* InText) {
  InputText(BlockPush(w), BlockPositionY, w, h, InText);
}

template <typename T> std::string tostr(const T& t) {
  std::ostringstream os;
  os << t;
  return os.str();
}

float LastUIHeight = 0.0;
bool ScroolStartDrag = false;
int ScroolLastMouseY = 0;

int LeftMargin = 12;
int RightMargin = 12;
int MenuWidth = 0;

///////////////
// New coder dialog
///////////////

void DialogNewCoder(float ElapsedTime) {
  
  BlockAlignLeft();

  InputText(sNewCoderName.c_str());

  BlockNextLine();
  if (Button(MenuWidth * 0.5, "Cancel")) {
    bModeNewCoder = false;
  }
  BlockVerticalSeparator();
  if (Button("Add Coder")) {
    ValidNewCoder();
  }
}

void DialogCommon(float ElapsedTime) {

  ///////////////
  // Buttons: add coder, save, options
  ///////////////

  if (Button(MenuWidth * 0.4, "Add Coder")) {
    EnterNewCoderMode();
  }
  BlockVerticalSeparator();
  if (Button(MenuWidth * 0.3 - 4, "Save")) {
    extern void SaveConfigFile();
    SaveConfigFile();
  }
  BlockVerticalSeparator();
  if (ButtonCheck("Options", !bModeOptions)) {
    bModeOptions = !bModeOptions;
  }
  BlockNextLine();

  ///////////////
  // Diaporama buttons
  ///////////////

  extern float DiapoBPM;
  std::string DelayText = tostr(DiapoBPM);
  std::string DiapoTitle = "Diapo (" + DelayText + " bpm)";

  BlockAlignRight();
  extern bool UseRandomShuffle;
  if (ButtonCheckIcon(3, 0, !UseRandomShuffle)) {
    UseRandomShuffle = !UseRandomShuffle;
  }
  extern bool DiapoInfiniteLoop;
  if (ButtonCheckIcon(2, 0, !DiapoInfiniteLoop)) {
    DiapoInfiniteLoop = !DiapoInfiniteLoop;
  }
  if (ButtonIcon(0, 0, true)) { // plus
    DiapoBPM = DiapoBPM + 1;
  }
  if (ButtonIcon(1, 0, true)) { // minus
    DiapoBPM = max(1, DiapoBPM - 1);
  }
  BlockVerticalSeparator();
  BlockAlignLeft();
  if (Button(DiapoTitle.c_str())) {
    ToggleDiaporama();
  }
  if (IsDiapoLaunched()) {
    glColor3d(0, 1, 0);
  }
  else {
    glColor3d(1, 0, 0);
  }
  DrawQuad(BlockMarginLeft - 5, BlockPositionY, 5, BlockButtonHeight);
  BlockNextLine();

  ///////////////
  // Mosaic button
  ///////////////

  std::vector<class Instance*>& Instances = GetInstances();

  int VisibleInstances = 0;
  for (int i = 0; i < Instances.size(); ++i) {
    Instance* Cur = Instances[i];
    if (Cur) {
      if (!Cur->IsHidden) ++VisibleInstances;
    }
  }
  std::string VisibleCountText = tostr(VisibleInstances);
  std::string InstanceCountText = tostr(Instances.size());
  std::string MosaicTitle = "Mosaic (" + VisibleCountText + "/" + InstanceCountText + ")";
  BlockAlignLeft();
  if (Button(MenuWidth * 0.7, MosaicTitle.c_str())) {
    PressMosaic();
  }
  BlockVerticalSeparator();
  if (Button("Random")) {
    RandomFullscreen();
  }

  BlockNextLine();
}

void DialogCoderList(float ElapsedTime) {

  ///////////////
  // Coders buttons
  ///////////////

  BlockInit(LeftMargin + 18, BlockPositionY, nWidth - LeftMargin - 10 - RightMargin, 25);
  BlockButtonHeight = 25;
  BlockIconOffset = -2;

  std::vector<class Instance*>& Instances = GetInstances();
  for (int i = 0; i < Instances.size(); ++i) {
    Instance* Cur = Instances[i];

    BlockAlignRight();

    if (bModeOptions) {
      if (ButtonIcon(3, 1)) { // delete coder
        if (Cur) {
          // If we delete the fullscreen instance, go back to the mosaic
          bool SwitchMosaic = Cur->IsFullScreen;
          RemoveInstance(Cur);
          if (SwitchMosaic) {
            PressMosaic();
          }
          else {
            RefreshDisplay();
          }
          break; // exit the loop so we don't mess up and delete several things
        }
      }
      if (ButtonIcon(0, 1)) { // restart coder
        if (Cur) Cur->Restart();
      }
      bool UpdateDisplay = false;
      if (i < Instances.size() - 1) {
        if (ButtonIcon(2, 1)) { // move down coder
          int other = i + 1;
          if (other < Instances.size()) {
            Instance* tmp = Instances[other];
            Instances[other] = Instances[i];
            Instances[i] = tmp;
            UpdateDisplay = true;
          }
        }
      }
      else {
        BlockVerticalSeparator(25);
      }
      if (i > 0) {
        if (ButtonIcon(1, 1)) { // move up coder
          int other = i - 1;
          if (other >= 0) {
            Instance* tmp = Instances[other];
            Instances[other] = Instances[i];
            Instances[i] = tmp;
            UpdateDisplay = true;
          }
        }
      }
      else {
        BlockVerticalSeparator(25);
      }
      if (UpdateDisplay) {
        RefreshDisplay();
      }
    }
    if (ButtonCheckIcon(0, 2, !Cur->IsHidden)) { // show/hidden coder
      ToggleHidden(Cur);
    }

    BlockVerticalSeparator();

    SetColor(ColorButtonHover);
    DrawLabelRight(BlockMarginLeft, BlockPositionY + FontSize / 4 + 12, tostr(i + 1));

    int CoderRightSide = BlockCurrentRight;

    if (Button(Cur->CoderName.c_str())) {
      ToggleFullscreen(Cur);
    }

    // separator below the coder
    glColor3d(0, 0, 0);
    DrawQuad(BlockMarginLeft, BlockPositionY + 23, CoderRightSide - BlockMarginLeft, 2);

    if (Cur->IsFullScreen) {
      glColor3d(0, 1, 0);
    }
    else {
      glColor3d(1, 0, 0);
    }
    DrawQuad(BlockMarginLeft - 5, BlockPositionY, 5, 23);

    BlockNextLine();
  }
  BlockNextLine();
}

void UpdateControlWindow(float ElapsedTime) {

  UpdateDiaporama(ElapsedTime);
  SortInstances();

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
  
  ScroolPositionY = max(0, min(ScroolPositionY, LastUIHeight - nHeight));
    
  int StartY = -ScroolPositionY;
  int PosY = StartY + 10;

  MenuWidth = nWidth - LeftMargin - RightMargin;

  BlockInit(LeftMargin, PosY, MenuWidth, 40);
  BlockAlignLeft();
  BlockIconOffset = 2;
  
  if (bModeNewCoder) {
    DialogNewCoder(ElapsedTime);
  } else {
    DialogCommon(ElapsedTime);
 
    DialogCoderList(ElapsedTime);

    ///////////////
    // Scrollbar
    ///////////////

    if (LastUIHeight > nHeight) {
      SetColor(ColorScrollbar);
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
  }

  LastUIHeight = BlockPositionY - StartY;

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