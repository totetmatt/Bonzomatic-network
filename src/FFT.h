#define FFT_SIZE 1024

namespace FFT
{
  extern float fAmplification;

  bool Open(bool CapturePlaybackDevices, const char* CaptureDeviceSearchString);
  bool GetFFT( float * _samples );
  void Close();
}