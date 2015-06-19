#include <Audioclient.h>
#include <mmdeviceapi.h>

extern const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
extern const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
extern const IID IID_IAudioClient = __uuidof(IAudioClient);
extern const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);