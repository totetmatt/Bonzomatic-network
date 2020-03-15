#include <stdio.h>
#include <string>
#include <assert.h>
#include <vector>

// TMP
#include <windows.h>

int ScreenWidth = 0;
int ScreenHeight = 0;

int NumberOfInstances = 2;
std::vector<class Instance*> Instances;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

class Instance {

public:
	HWND hwnd = NULL;
	STARTUPINFOA siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;

	int Index = 0;

	Instance()
	{
	}

	bool Init(int InstanceIndex)
	{
		Index = InstanceIndex;

		memset(&siStartupInfo, 0, sizeof(siStartupInfo));
		memset(&piProcessInfo, 0, sizeof(piProcessInfo));
		siStartupInfo.cb = sizeof(siStartupInfo);

		std::string ShaderName = " shader=netshad_" + std::to_string(InstanceIndex) + ".glsl";

		std::string CommandLine = std::string(" skipdialog") + ShaderName + std::string(" networkMode=grabber");
		char* CommandString = new char[CommandLine.size() + 1];
		strncpy_s(CommandString, (CommandLine.size() + 1), CommandLine.c_str(), (CommandLine.size()+1));

		DWORD dwExitCode = 0;
		if (CreateProcessA("Bonzomatic.exe",
			CommandString, 0, 0, false,
			CREATE_DEFAULT_ERROR_MODE, 0, 0,
			&siStartupInfo, &piProcessInfo) != false)
		{
			//printf("[LAUNCHER] Created succesfull.\n");

			/* Watch the process. */
			int SecondsToWait = 1;
			dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000));

			//printf("[LAUNCHER] Process initialized.\n");

			EnumWindows(EnumWindowsProc, (LPARAM)(this));
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

	void Release()
	{
		/* Release handles */
		CloseHandle(piProcessInfo.hProcess);
		CloseHandle(piProcessInfo.hThread);
	}
};

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

void FindDesktopResolution() {
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	ScreenWidth = desktop.right - desktop.left;
	ScreenHeight = desktop.bottom - desktop.top;
}

bool LaunchInstances()
{
	for (int i = 0; i < NumberOfInstances; ++i) {
		Instance* NewInstance = new Instance();
		if (NewInstance->Init(i)) {
			Instances.push_back(NewInstance);
		}
		else {
			delete NewInstance;
		}
    Sleep(500);
	}

	int NumColumn = ceil(sqrt(NumberOfInstances));
	int NumRow = ceil(NumberOfInstances/NumColumn);
	int ColumnSize = ScreenWidth / NumColumn;
	int RowSize = ScreenHeight / NumRow;
	
	for (auto const& Cur : Instances) {
		int PosX = (Cur->Index % NumColumn) * ColumnSize;
		int PosY = floor(Cur->Index / NumColumn) * RowSize;
	  SetWindowPos(Cur->hwnd, HWND_TOP, PosX, PosY, ColumnSize, RowSize, SWP_ASYNCWINDOWPOS);
	  //SetWindowLong(Cur->hwnd, GWL_EXSTYLE, NULL);
	  LONG WindowStyle = GetWindowLong(Cur->hwnd, GWL_STYLE);
	  WindowStyle &= ~WS_CAPTION;
	  WindowStyle &= ~WS_MAXIMIZEBOX;
	  WindowStyle &= ~WS_THICKFRAME;
	  SetWindowLong(Cur->hwnd, GWL_STYLE, WindowStyle);
  }

	return true;
}

void ReleaseInstances() {
	for (auto const& Cur : Instances) {
		Cur->Release();
		delete Cur;
	}
	Instances.clear();
}

int main(int argc, const char *argv[])
{
  printf("[LAUNCHER] Started \n");
  FindDesktopResolution();

  LaunchInstances();

  // Wait indefinitely
  while (true) {
	  Sleep(30);
  }

  ReleaseInstances();
  
  return 0;
}