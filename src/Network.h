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

  struct ShaderParamCache {
    float lastValue;
    float currentValue;
    float goalValue;
    float duration;
  };
  
  void LoadSettings(jsonxx::Object & o, NETWORK_SETTINGS* DialogSettings);
  void CommandLine(int argc, const char *argv[], NETWORK_SETTINGS* DialogSettings);
  void PrepareConnection();
  void OpenConnection();
  void BroadcastMessage(const char* msg);
  void SendShader(ShaderMessage NewMessage, float shaderOffset, const jsonxx::Object& shaderParameters);
  bool IsShaderNeedUpdate();
  bool IsNetworkEnabled();
  bool IsConnected();
  bool IsGrabber();
  bool IsLive();
  std::string GetNickName();
  std::string GetModeString();
  bool CanSendMidiControls();
  bool CanGrabMidiControls();
  bool HasNewShader();
  bool GetNewShader(ShaderMessage& OutShader, std::map<std::string, ShaderParamCache>& networkParamCache);
  bool AdjustShaderTimeOffset(float time, float& timeOffset);
  void Tick(float time);
  void Release();
}