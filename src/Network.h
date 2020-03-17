#include "NetworkSettings.h"

namespace Network
{
  struct ShaderMessage {
	std::string Code;
	int CaretPosition;
	int AnchorPosition;
	bool NeedRecompile;
  };
  
  void LoadSettings(jsonxx::Object & o, NETWORK_SETTINGS* DialogSettings);
  void CommandLine(int argc, const char *argv[], NETWORK_SETTINGS* DialogSettings);
  void OpenConnection();
  void BroadcastMessage(const char* msg);
  void SendShader(float time, ShaderMessage NewMessage);
  bool IsShaderNeedUpdate(float Time);
  bool IsNetworkEnabled();
  bool IsConnected();
  bool IsGrabber();
  bool IsLive(float time);
  std::string GetNickName();
  std::string GetModeString();
  bool HasNewShader();
  bool GetNewShader(float time, ShaderMessage& OutShader);
  void Tick();
  void Release();
}