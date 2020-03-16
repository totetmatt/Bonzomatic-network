
namespace Network
{
  struct ShaderMessage {
	std::string Code;
	int CaretPosition;
	int AnchorPosition;
	bool NeedRecompile;
  };

  void LoadSettings(jsonxx::Object & o);
  void CommandLine(int argc, const char *argv[]);
  void OpenConnection();
  void BroadcastMessage(const char* msg);
  void SendShader(float time, ShaderMessage NewMessage);
  bool IsShaderNeedUpdate(float Time);
  bool IsConnected();
  bool IsGrabber();
  bool HasNewShader();
  bool GetNewShader(ShaderMessage& OutShader);
  void Tick();
  void Release();
}