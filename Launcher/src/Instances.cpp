
#include "Instances.h"
#include "ControlWindow.h"
#include "Network.h"

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

bool ShuttingDown = false;

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

bool CoderHiddenAtBottom = false;
bool CoderAutoAdd = true;
bool CoderAutoLaunchBonzo = false;

bool IsDiapoLaunched() { return DiapoLaunched; }

bool UseRandomShuffle = false;
std::vector<int> RandomIndexes;
void RandomShuffle() {
  int last = RandomIndexes.size() > 0 ? RandomIndexes.back() : -1;
  RandomIndexes.clear();
  for (int i = 0; i < Instances.size(); ++i) {
    RandomIndexes.push_back(i);
  }
  // shuffle
  for (int i = RandomIndexes.size()-1; i>0; --i) {
    int a = RandomIndexes[i];
    int ri = rand() % (i + 1);
    RandomIndexes[i] = RandomIndexes[ri];
    RandomIndexes[ri] = a;
  }
}

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
  while(NewIndex >=0 && NewIndex <Instances.size() && !Instances[NewIndex]->IsShowMosaic()) {
    --NewIndex;
  }
  FullscreenIndex(NewIndex);
}

void FullscreenNext() {
  int NewIndex = LastFullScreenIndex + 1;
  while (NewIndex >= 0 && NewIndex < Instances.size() && !Instances[NewIndex]->IsShowMosaic()) {
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
    if (Cur->IsShowMosaic() && !Cur->IsFullScreen) {
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
  RandomShuffle();
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
      int HiddenDiapoCounter = 0; // check that to avoid looping if everything is hidden
      int PrevIndex = DiapoCurrentIndex;
      if (UseRandomShuffle) {
        PrevIndex = RandomIndexes[DiapoCurrentIndex % RandomIndexes.size()];
      }
      int AvoidIndex = -1;
      do {
        DiapoCurrentIndex += 1;
        if (DiapoCurrentIndex >= Instances.size()) {
          DiapoCurrentIndex = 0;
          DiapoCurrentLoop += 1;
          if (DiapoCurrentLoop >= DiapoLoops && !DiapoInfiniteLoop || HiddenDiapoCounter>= Instances.size()) {
            // end of diaporama
            StopDiaporama();
            // staap
          } else {
            HiddenDiapoCounter = 0;
            AvoidIndex = PrevIndex; // we want to avoid repeating last visible index
            RandomShuffle();
          }
        }
        // skip hidden diapo
        int CurrentIndex = DiapoCurrentIndex;
        if (UseRandomShuffle) {
          CurrentIndex = RandomIndexes[DiapoCurrentIndex % RandomIndexes.size()];
        }
        CurDiapoHidden = CurrentIndex < Instances.size() ? !Instances[CurrentIndex]->IsShowMosaic() : true;
        if (CurDiapoHidden) {
          HiddenDiapoCounter++;
        }
        if (AvoidIndex >= 0 && AvoidIndex == CurrentIndex) {
          // avoid repeating last index after a loop
          CurDiapoHidden = true;
          AvoidIndex = -1; // avoid only once so we can loop if only one visible instance
          PrevIndex = -1;
        }
      } while (CurDiapoHidden && DiapoLaunched);

      if (DiapoLaunched) {
        int CurrentIndex = DiapoCurrentIndex;
        if (UseRandomShuffle) {
          CurrentIndex = RandomIndexes[DiapoCurrentIndex % RandomIndexes.size()];
        }
        FullscreenIndex(CurrentIndex);
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
  Launched = false;
  CoderName = InCoderName;
  return true;
}

bool Instance::InitBonzo() {
  Launched = true;

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
    if (dwExitCode = 258) {
      printf("[LAUNCHER] Failed to get process handle.\n");
    }

    //printf("[LAUNCHER] Process initialized.\n");
    EnumWindows(EnumWindowsProc, (LPARAM)(this));
    if (!hwnd) {
      printf("[LAUNCHER] Failed to get window handle.\n");
    }

    LONG WindowStyle = GetWindowLong(hwnd, GWL_STYLE);
    WindowStyle &= ~WS_CAPTION;
    WindowStyle &= ~WS_MAXIMIZEBOX;
    WindowStyle &= ~WS_THICKFRAME;
    SetWindowLong(hwnd, GWL_STYLE, WindowStyle);

    if (!WantTextEditor) {
      ToggleTextEditorInWindow(hwnd);
    }
    FocusControlWindow();
    RefreshDisplay();
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
  if (Launched) {
    SendMessage(hwnd, WM_CLOSE, NULL, NULL);
    /* Release handles */
    CloseHandle(piProcessInfo.hProcess);
    CloseHandle(piProcessInfo.hThread);
  }
}

void Instance::Restart()
{
  Release();
  Init(CoderName);
  InitBonzo();
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
  //ShowWindow(Target->hwnd, Target->IsShowMosaic() ? SW_SHOW : SW_HIDE);
  if (!GlobalIsFullscreen) {
    ShowMosaic();
  }
}

void SetInstancePositionRatio(Instance* Cur, int PosX, int PosY, int Width, int Height, bool UseRatio, float WantedRatio) {
  if (!Cur->Launched) return;

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
  if (!Cur->Launched) return;
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
    Instance* Cur = Instances[i];
    if ((MosaicFixed && Cur->Launched) || Cur->IsShowMosaic()) {
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
      if (Cur->IsShowMosaic()) {
        SetInstancePositionRatio(Cur, PosX, PosY, ColumnSize, RowSize, Mosaic->ForceRatio, Mosaic->WantedRatio);
      } else {
        SetMinimalPosition(Cur);
      }
    }
    
    if ((MosaicFixed && Cur->Launched) || Cur->IsShowMosaic()) {
      ++CurIndex;
    }
  }
}

Instance* AddInstance(std::string CoderName) {
  if (ShuttingDown) return NULL;
  Instance* NewInstance = new Instance();
  if (NewInstance->Init(CoderName)) {
    Instances.push_back(NewInstance);
    if (CoderAutoLaunchBonzo) {
      NewInstance->InitBonzo();
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

void SortInstances() {
  if (Instances.size() < 2) return;
  for (int i = 0; i < Instances.size() - 1; ++i) {
    Instance* Cur = Instances[i];
    Instance* Next = Instances[i + 1];
    if (Cur && Next) {
      bool swap = !Cur->Launched && Next->Launched;
      if (CoderHiddenAtBottom) {
        swap |= Cur->IsHidden && !Next->IsHidden;
      }
      if (swap) {
        // swap
        Instances[i] = Next;
        Instances[i + 1] = Cur;
      }
    }
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

  if (options.has<jsonxx::Object>("list"))
  {
    jsonxx::Object listjson = options.get<jsonxx::Object>("list");
    if (listjson.has<jsonxx::Boolean>("codertogglemosaic")) CoderToggleMosaic = listjson.get<jsonxx::Boolean>("codertogglemosaic");
    if (listjson.has<jsonxx::Boolean>("hiddenatbottom")) CoderHiddenAtBottom = listjson.get<jsonxx::Boolean>("hiddenatbottom");
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
    if (netjson.has<jsonxx::Boolean>("autoaddcoder")) CoderAutoAdd = netjson.get<jsonxx::Boolean>("autoaddcoder");
    if (netjson.has<jsonxx::Boolean>("autolaunchbonzo")) CoderAutoLaunchBonzo = netjson.get<jsonxx::Boolean>("autolaunchbonzo");
    if (netjson.has<jsonxx::String>("serverURL")) ServerURL = netjson.get<jsonxx::String>("serverURL");
  }
  
  if (options.has<jsonxx::Array>("coders")) {
    jsonxx::Array coderjson = options.get<jsonxx::Array>("coders");
    for (int i = 0; i < coderjson.size(); ++i) {
      std::string codername = coderjson.get<jsonxx::String>(i);
      printf("[LAUNCHER] Add coder %s \n", codername);
      AddInstance(codername);
    }
  }

  if (UseNetwork) {
    Network::PrepareConnection();
    Network::OpenConnection(ServerURL);
  }

  RefreshDisplay();

  return true;
}

void ReleaseInstances() {
  ShuttingDown = true;
  for (auto const& Cur : Instances) {
    Cur->Release();
    delete Cur;
  }
  Instances.clear();

  Network::Release();
}

std::vector<class Instance*>& GetInstances()
{
  return Instances;
}

void SignalLiveUser(std::string UserName) {
  for (auto const& Cur : Instances) {
    if (Cur->CoderName == UserName) {
      Cur->TimeLastLive = 0.0f;
      return;
    }
  }
  // no coder found
  if (CoderAutoAdd && !ShuttingDown) {
    printf("[LAUNCHER] Network Add coder %s \n", UserName.c_str());
    AddInstance(UserName);
  }
}