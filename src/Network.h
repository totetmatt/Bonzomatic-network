#include "NetworkSettings.h"

namespace Network
{
  struct ShaderMessage {
	std::string Code;
	int CaretPosition;
	int AnchorPosition;
  int FirstVisibleLine;
	bool NeedRecompile;
  };
  
  void LoadSettings(jsonxx::Object & o, NETWORK_SETTINGS* DialogSettings);
  void CommandLine(int argc, const char *argv[], NETWORK_SETTINGS* DialogSettings);
  void PrepareConnection();
  void OpenConnection();
  void BroadcastMessage(const char* msg);
  void SendShader(ShaderMessage NewMessage);
  bool IsShaderNeedUpdate();
  bool IsNetworkEnabled();
  bool IsConnected();
  bool IsGrabber();
  bool IsLive();
  std::string GetNickName();
  std::string GetModeString();
  bool HasNewShader();
  bool GetNewShader(ShaderMessage& OutShader);
  void Tick(float time);
  void Release();
}