namespace Capture
{
  void LoadSettings(jsonxx::Object & o);
  bool Open(RENDERER_SETTINGS & settings);
  void CaptureResize(int width, int height);
  void CaptureFrame();
  void Close();
}