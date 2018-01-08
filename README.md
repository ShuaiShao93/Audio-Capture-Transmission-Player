# Remote Audio Capture-Transmission-Play software including Server, Client, and Network Transmission
• On Windows server, captured audio by injecting DLL into processes and intercept system audio stream<br />
• Designed audio mixing, AAC encoding, TCP network transmission and AAC decoding modules<br />
• On client, implemented audio player with buffer mechanism <br />
• Supports multiple sessions for multiple clients <br />

Instructions:
1. Install detours library in C:\Program Files\Detours.

2. Locate Line 21 in InjectDLL/InjectDLL.cpp, change the if-sentence with the process names you want to inject into.

3. Locate Line 19 in TestLibPlayer/TestLibPlayer.h, change SERVER_ADD to the actual address of server. If you want to test locally, just leave it be.

4. Build 'AudioHook'.

5. Move AudioHook2.dll from AudioHook\Release to TestCaptureAudio\Release.

6. Locate Line 25 in InjectDLL/InjectDLL.cpp, change the path to AudioHook2.dll file, aka path to "TestCaptureAudio\Release\AudioHook2.dll".

7. Build all other subprojects.

8. On server, run the processes that you want to capture audio.

9. On server, run InjectDll.exe in 'InjectDLL' to inject the dll into the processes and start capturing audio.

10. On server, run TestCaptureAudio.exe in 'TestCaptureAudio' to read audio from buffer and mix.

11. On client, run TestLibPlayer.exe in 'TestLibPlayer' to play audio.
