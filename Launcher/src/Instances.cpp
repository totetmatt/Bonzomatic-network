
#include "Instances.h"
#include "ControlWindow.h"

#include <stdio.h>
#include <string>
#include <assert.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

bool MosaicFixed = false;

struct ScreenArea {
  int Monitor = 0;
  int PosX = 0;
  int PosY = 0;
  int Width = 1;
  int Height = 1;
  float PercentStartX;
  float PercentStartY;
  float PercentWidth;
  float PercentHeight;
  int BlankSpaceX = 10;
  int BlankSpaceY = 10;
  bool ForceRatio = true;
  float WantedRatio = 1.7777f;
  ScreenArea(int _Monitor, float _PercentStartX, float _PercentStartY, float _PercentWidth, float _PercentHeight) {
    Monitor = _Monitor;
    PercentStartX = _PercentStartX;
    PercentStartY = _PercentStartY;
    PercentWidth = _PercentWidth;
    PercentHeight = _PercentHeight;
  }
};

bool UseSecondaryScreen = true;
ScreenArea ScreenMain(0, 0.0f, 0.0f, 1.0f, 1.0f);
ScreenArea ScreenSecondary(1, 0.3f, 0.3f, 0.4f, 0.4f);
ScreenArea ScreenFull(0, 0.05f, 0.05f, 0.9f, 0.9f);

bool GlobalIsFullscreen = false;
int LastFullScreenIndex = -1;

int MonitorCount = 0;
GLFWmonitor** Monitors;

int DelayInitWindows = 500;
std::string ConfigCommandLine = "skipdialog networkMode=grabber";

std::string ExecutableName = "Bonzomatic.exe";
std::string ServerURL = "ws://127.0.0.1:8000/roomy/";

std::vector<class Instance*> Instances;


float DiapoBPM = 60.0f;
bool DiapoInfiniteLoop = false;
int DiapoLoops = 1;
int DiapoCurrentIndex = 0;
float DiapoCurrentTime = 0;
int DiapoCurrentLoop = 0;
bool DiapoLaunched = false;
bool WantTextEditor = true;

bool IsDiapoLaunched() { return DiapoLaunched; }
int GetCurrentDiapo() { return DiapoCurrentIndex; }

void FullscreenInstance(Instance* Target) {
  GlobalIsFullscreen = true;
  for (int i = 0; i < Instances.size(); ++i) {
    auto const& Cur = Instances[i];

    bool IsTarget = Target && Cur == Target;
    if (IsTarget) {
      Cur->IsFullScreen = true;
      LastFullScreenIndex = i;
    }
    else {
      Cur->IsFullScreen = false;
    }
  }
  RefreshDisplay();
}

void ShowMosaic() {
  GlobalIsFullscreen = false;
  for (int i = 0; i < Instances.size(); ++i) {
    auto const& Cur = Instances[i];
    Cur->IsFullScreen = false;
  }
  RefreshDisplay();
}

void FullscreenIndex(int index) {
  if (index<0 || index >= Instances.size()) return;
  FullscreenInstance(Instances[index]);
}

void FullscreenPrev() {
  int NewIndex = LastFullScreenIndex - 1;
  while(NewIndex >=0 && NewIndex <Instances.size() && Instances[NewIndex]->IsHidden) {
    --NewIndex;
  }
  FullscreenIndex(NewIndex);
}

void FullscreenNext() {
  int NewIndex = LastFullScreenIndex + 1;
  while (NewIndex >= 0 && NewIndex < Instances.size() && Instances[NewIndex]->IsHidden) {
    ++NewIndex;
  }
  FullscreenIndex(NewIndex);
}

bool CoderToggleMosaic = false;
void ToggleFullscreen(Instance* Cur) {
  if (Cur->IsFullScreen) {
    if (CoderToggleMosaic) {
      ShowMosaic();
    }
  }
  else {
    StopDiaporama();
    FullscreenInstance(Cur);
  }
}

void ToggleFullscreen(int index) {
  if (index >= Instances.size()) return;
  StopDiaporama();
  ToggleFullscreen(Instances[index]);
}

void RandomFullscreen() {
  std::vector<Instance*> List;
  for (int i = 0; i < Instances.size(); ++i) {
    Instance* Cur = Instances[i];
    if (!Cur->IsHidden && !Cur->IsFullScreen) {
      List.push_back(Cur);
    }
  }
  if (List.size() > 0) {
    ToggleFullscreen(List[rand() % List.size()]);
  }
}

void ToggleTextEditorInWindow(HWND hwnd) {
  //PostMessage(Cur->hwnd, WM_KEYDOWN, VK_F11, 0);
    //PostMessage(Cur->hwnd, WM_KEYUP, VK_F11, 0);
    //SendMessage(Cur->hwnd, WM_KEYDOWN, VK_F11, 1);
    //SendMessage(Cur->hwnd, WM_KEYUP, VK_F11, 1);

  BringWindowToTop(hwnd);
  SetForegroundWindow(hwnd);
  SetFocus(hwnd);

  /*
  LPARAM lParam = (1 | (57 << 16)); // OEM Code and Repeat for WM_KEYDOWN
  WPARAM wParam = VK_F11;
  //PostMessage(HWND_BROADCAST, WM_KEYDOWN, wParam, lParam); // Works
  //PostMessage(Cur->hwnd, WM_KEYDOWN, wParam, lParam); // Doesn't Work
  SendMessage(Cur->hwnd, WM_KEYDOWN, wParam, lParam); // Works, but I need Post
  */

  //*

  //Sleep(100);

  INPUT ip;

  ip.type = INPUT_KEYBOARD;
  ip.ki.wScan = 0x057; // bonzomatic use a weird scancode for F11, not sure what it is
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;

  ip.ki.wVk = VK_F11;
  ip.ki.dwFlags = 0; // 0 for key press
  SendInput(1, &ip, sizeof(INPUT));

  ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
  SendInput(1, &ip, sizeof(INPUT));
}

void ToggleTextEditor() {
  WantTextEditor = !WantTextEditor;
  for (auto const& Cur : Instances) {
    ToggleTextEditorInWindow(Cur->hwnd);
  }
  FocusControlWindow();
}

void StartDiaporama() {
  DiapoLaunched = true;
  DiapoCurrentIndex = -1;
  DiapoCurrentTime = 0;
  DiapoCurrentLoop = 0;
}

void StopDiaporama() {
  DiapoLaunched = false;
}

void UpdateDiaporama(float ElapsedTime) {
  if (DiapoLaunched) {
    DiapoCurrentTime += ElapsedTime;
    if (DiapoCurrentTime >= (60.0f/max(1.0f, DiapoBPM)) || DiapoCurrentIndex < 0) {
      DiapoCurrentTime = 0;
      // Go to next diapo
      
      bool CurDiapoHidden = false;
      do {
        DiapoCurrentIndex += 1;
        if (DiapoCurrentIndex >= Instances.size()) {
          DiapoCurrentIndex = 0;
          DiapoCurrentLoop += 1;
          if (DiapoCurrentLoop >= DiapoLoops && !DiapoInfiniteLoop) {
            // end of diaporama
            StopDiaporama();
            // staap
          }
        }
        // skip hidden diapo
        CurDiapoHidden = DiapoCurrentIndex < Instances.size() ? Instances[DiapoCurrentIndex]->IsHidden : true;
      } while (CurDiapoHidden && DiapoLaunched);

      if (DiapoLaunched) {
        FullscreenIndex(DiapoCurrentIndex);
      }
      else {
        ShowMosaic();
      }
    }
  }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

Instance::Instance()
{
}

bool Instance::Init(std::string InCoderName)
{
  CoderName = InCoderName;

  memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  memset(&piProcessInfo, 0, sizeof(piProcessInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  //std::string ShaderName = " shader=netshad_" + std::to_string(InstanceIndex) + ".glsl";
  std::string ServerCommand = " serverURL=" + ServerURL + CoderName;

  std::string CommandLine = ServerCommand + std::string(" ") + ConfigCommandLine;
  char* CommandString = new char[CommandLine.size() + 1];
  strncpy_s(CommandString, (CommandLine.size() + 1), CommandLine.c_str(), (CommandLine.size() + 1));

  DWORD dwExitCode = 0;
  if (CreateProcessA(ExecutableName.c_str(),
    CommandString, 0, 0, false,
    CREATE_DEFAULT_ERROR_MODE, 0, 0,
    &siStartupInfo, &piProcessInfo) != false)
  {
    //printf("[LAUNCHER] Created succesfull.\n");

    /* Watch the process. */
    dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, DelayInitWindows);

    //printf("[LAUNCHER] Process initialized.\n");

    EnumWindows(EnumWindowsProc, (LPARAM)(this));

    LONG WindowStyle = GetWindowLong(hwnd, GWL_STYLE);
    WindowStyle &= ~WS_CAPTION;
    WindowStyle &= ~WS_MAXIMIZEBOX;
    WindowStyle &= ~WS_THICKFRAME;
    SetWindowLong(hwnd, GWL_STYLE, WindowStyle);
  }
  else
  {
    /* CreateProcess failed */
    size_t iReturnVal = GetLastError();
    printf("[LAUNCHER] Error with creating process: %d\n", iReturnVal);
    return false;
  }

  return true;
}

void Instance::Release()
{
  SendMessage(hwnd, WM_CLOSE, NULL, NULL);
  /* Release handles */
  CloseHandle(piProcessInfo.hProcess);
  CloseHandle(piProcessInfo.hThread);
}

void Instance::Restart()
{
  Release();
  Init(CoderName);
  RefreshDisplay();
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  DWORD dwPID;

  GetWindowThreadProcessId(hwnd, &dwPID);

  Instance* CurrentInstance = (Instance*)(lParam);
  if (dwPID == CurrentInstance->piProcessInfo.dwProcessId) {
    //printf("[LAUNCHER] PID found %d \n", dwPID);
    CurrentInstance->hwnd = hwnd;
    return FALSE;
  }

  return TRUE;
}

void InitScreenAreaMonitor(ScreenArea* Area) {
  if (Area->Monitor >= 0 && Area->Monitor < MonitorCount) {
    glfwGetMonitorWorkarea(Monitors[Area->Monitor], &Area->PosX, &Area->PosY, &Area->Width, &Area->Height);
  }
}

void UpdateMonitors() {
  // init monitors
  Monitors = glfwGetMonitors(&MonitorCount);
  InitScreenAreaMonitor(&ScreenMain);
  InitScreenAreaMonitor(&ScreenSecondary);
  InitScreenAreaMonitor(&ScreenFull);
}

void ToggleHidden(Instance* Target) {
  Target->IsHidden = !Target->IsHidden;
  //ShowWindow(Target->hwnd, Target->IsHidden ? SW_HIDE : SW_SHOW);
  if (!GlobalIsFullscreen) {
    ShowMosaic();
  }
}

void SetInstancePositionRatio(Instance* Cur, int PosX, int PosY, int Width, int Height, bool UseRatio, float WantedRatio) {

  if (UseRatio && Height>0) {
    float Ratio = float(Width) / Height;
    if (Ratio > WantedRatio) {
      int NewWidth = Height * WantedRatio;
      PosX += (Width - NewWidth) / 2;
      Width = NewWidth;
    } else {
      int NewHeight = Width / WantedRatio;
      PosY += (Height - NewHeight) / 2;
      Height = NewHeight;
    }
  }

  SetWindowPos(Cur->hwnd, HWND_TOP, PosX, PosY, Width, Height, SWP_ASYNCWINDOWPOS);
  ShowWindow(Cur->hwnd, SW_SHOWNOACTIVATE);
}

void SetMinimalPosition(Instance* Cur) {
  SetWindowPos(Cur->hwnd, HWND_TOP, 0, 0, 1, 1, SWP_ASYNCWINDOWPOS);
  ShowWindow(Cur->hwnd, SW_HIDE);
}

void RefreshDisplay() {

  bool IsFullScreen = GlobalIsFullscreen;
  
  ScreenArea* Mosaic = &ScreenMain;
  if (IsFullScreen && UseSecondaryScreen) {
    Mosaic = &ScreenSecondary;
  }
  
  int FullWidth = ScreenFull.Width * ScreenFull.PercentWidth;
  int FullHeight = ScreenFull.Height * ScreenFull.PercentHeight;
  int PixelFullScreenOffsetX = ScreenFull.Width * ScreenFull.PercentStartX + ScreenFull.PosX;
  int PixelFullScreenOffsetY = ScreenFull.Height * ScreenFull.PercentStartY + ScreenFull.PosY;

  int NumberOfInstances = 0;
  for (int i = 0; i < Instances.size(); ++i) {
    if (MosaicFixed || !Instances[i]->IsHidden) {
      ++NumberOfInstances;
    }
  }
  
  int NumColumn = NumberOfInstances < 1 ? 1 : ceil(sqrt(NumberOfInstances));
  int NumRow = NumberOfInstances < 1 ? 1 : ceil(float(NumberOfInstances) / NumColumn);
  int TotalBlankX = max(0, NumColumn - 1) * Mosaic->BlankSpaceX;
  int TotalBlankY = max(0, NumRow - 1) * Mosaic->BlankSpaceY;
  int ColumnSize = (Mosaic->Width * Mosaic->PercentWidth - TotalBlankX)/ NumColumn;
  int RowSize = (Mosaic->Height * Mosaic->PercentHeight - TotalBlankX) / NumRow;
  int PixelScreenOffsetX = Mosaic->Width * Mosaic->PercentStartX + Mosaic->PosX;
  int PixelScreenOffsetY = Mosaic->Height * Mosaic->PercentStartY + Mosaic->PosY;
  
  int CurIndex = 0;
  for (int i = 0; i < Instances.size(); ++i) {
    auto const& Cur = Instances[i];

    int PosX = (CurIndex % NumColumn) * (ColumnSize + Mosaic->BlankSpaceX) + PixelScreenOffsetX;
    int PosY = floor(CurIndex / NumColumn) * (RowSize + Mosaic->BlankSpaceY) + PixelScreenOffsetY;

    if (IsFullScreen) {
      if (Cur->IsFullScreen) {
        SetInstancePositionRatio(Cur, PixelFullScreenOffsetX, PixelFullScreenOffsetY, FullWidth, FullHeight, ScreenFull.ForceRatio, ScreenFull.WantedRatio);
      }
      else {
        if (UseSecondaryScreen) {
          SetInstancePositionRatio(Cur, PosX, PosY, ColumnSize, RowSize, Mosaic->ForceRatio, Mosaic->WantedRatio);
        }
        else {
          SetMinimalPosition(Cur);
        }
      }
    } else {
      if (Cur->IsHidden) {
        SetMinimalPosition(Cur);
      }
      else {
        SetInstancePositionRatio(Cur, PosX, PosY, ColumnSize, RowSize, Mosaic->ForceRatio, Mosaic->WantedRatio);
      }
    }
    
    if (MosaicFixed || !Cur->IsHidden) {
      ++CurIndex;
    }
  }
}

Instance* AddInstance(std::string CoderName) {
  Instance* NewInstance = new Instance();
  if (NewInstance->Init(CoderName)) {
    Instances.push_back(NewInstance);
    if (!WantTextEditor) {
      ToggleTextEditorInWindow(NewInstance->hwnd);
    }
  } else {
    delete NewInstance;
  }
  return NewInstance;
}

void RemoveInstance(Instance* instance) {
  if (!instance) return;
  instance->Release();
  auto FoundElement = std::find(Instances.begin(), Instances.end(), instance);
  if(FoundElement != Instances.end()) {
    Instances.erase(FoundElement);
  }
}

void LoadScreenOptions(jsonxx::Object& winjson, ScreenArea* Area) {
  if (winjson.has<jsonxx::Number>("monitor")) Area->Monitor = max(0, int(winjson.get<jsonxx::Number>("monitor")));
  if (winjson.has<jsonxx::Number>("startpercent_x")) Area->PercentStartX = winjson.get<jsonxx::Number>("startpercent_x");
  if (winjson.has<jsonxx::Number>("startpercent_y")) Area->PercentStartY = winjson.get<jsonxx::Number>("startpercent_y");
  if (winjson.has<jsonxx::Number>("sizepercent_x")) Area->PercentWidth = winjson.get<jsonxx::Number>("sizepercent_x");
  if (winjson.has<jsonxx::Number>("sizepercent_y")) Area->PercentHeight = winjson.get<jsonxx::Number>("sizepercent_y");
  if (winjson.has<jsonxx::Number>("border_x")) Area->BlankSpaceX = winjson.get<jsonxx::Number>("border_x");
  if (winjson.has<jsonxx::Number>("border_y")) Area->BlankSpaceY = winjson.get<jsonxx::Number>("border_y");
  if (winjson.has<jsonxx::Boolean>("forceratio")) Area->ForceRatio = winjson.get<jsonxx::Boolean>("forceratio");
  if (winjson.has<jsonxx::Number>("wantedratio")) Area->WantedRatio = winjson.get<jsonxx::Number>("wantedratio");
}

bool LaunchInstances(jsonxx::Object options)
{
  if (options.has<jsonxx::Object>("mosaic"))
  {
    jsonxx::Object winjson = options.get<jsonxx::Object>("mosaic");
    LoadScreenOptions(winjson, &ScreenMain);
    if (winjson.has<jsonxx::Boolean>("MosaicFixed")) MosaicFixed = winjson.get<jsonxx::Boolean>("MosaicFixed");
  }

  if (options.has<jsonxx::Object>("fullscreen"))
  {
    jsonxx::Object winjson = options.get<jsonxx::Object>("fullscreen");
    LoadScreenOptions(winjson, &ScreenFull);
    if (winjson.has<jsonxx::Boolean>("codertogglemosaic")) CoderToggleMosaic = winjson.get<jsonxx::Boolean>("codertogglemosaic");
  }

  if (options.has<jsonxx::Object>("secondary"))
  {
    jsonxx::Object winjson = options.get<jsonxx::Object>("secondary");
    if (winjson.has<jsonxx::Boolean>("use")) UseSecondaryScreen = winjson.get<jsonxx::Boolean>("use");
    LoadScreenOptions(winjson, &ScreenSecondary);
  }

  UpdateMonitors();

  if (options.has<jsonxx::Object>("bonzo"))
  {
    jsonxx::Object bonzojson = options.get<jsonxx::Object>("bonzo");
    if (bonzojson.has<jsonxx::Number>("delay_between_spawn")) DelayInitWindows = bonzojson.get<jsonxx::Number>("delay_between_spawn");
    if (bonzojson.has<jsonxx::String>("commandline")) ConfigCommandLine = bonzojson.get<jsonxx::String>("commandline");
    if (bonzojson.has<jsonxx::String>("exename")) ExecutableName = bonzojson.get<jsonxx::String>("exename");
  }

  if (options.has<jsonxx::Object>("diaporama"))
  {
    jsonxx::Object diapojson = options.get<jsonxx::Object>("diaporama");
    if (diapojson.has<jsonxx::Number>("bpm")) DiapoBPM = diapojson.get<jsonxx::Number>("bpm");
    if (diapojson.has<jsonxx::Number>("loops")) DiapoLoops = diapojson.get<jsonxx::Number>("loops");
    if (diapojson.has<jsonxx::Boolean>("infiniteloop"))  DiapoInfiniteLoop = diapojson.get<jsonxx::Boolean>("infiniteloop");
  }

  bool UseNetwork = false;
  if (options.has<jsonxx::Object>("network"))
  {
    jsonxx::Object netjson = options.get<jsonxx::Object>("network");
    if (netjson.has<jsonxx::Boolean>("receiveuserlist")) UseNetwork = netjson.get<jsonxx::Boolean>("receiveuserlist");
    if (netjson.has<jsonxx::String>("serverURL")) ServerURL = netjson.get<jsonxx::String>("serverURL");
  }
  
  if (UseNetwork) {
    // TODO
  } else {
    if (options.has<jsonxx::Array>("coders")) {
      jsonxx::Array coderjson = options.get<jsonxx::Array>("coders");
      for (int i = 0; i < coderjson.size(); ++i) {
        std::string codername = coderjson.get<jsonxx::String>(i);
        printf("[LAUNCHER] Add coder %s \n", codername);
        AddInstance(codername);
      }
    } else {
      printf("[LAUNCHER] Missing coders section in config file \n");
    }
  }

  RefreshDisplay();

  return true;
}

void ReleaseInstances() {
  for (auto const& Cur : Instances) {
    Cur->Release();
    delete Cur;
  }
  Instances.clear();
}

std::vector<class Instance*>& GetInstances()
{
  return Instances;
}