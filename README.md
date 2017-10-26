# Remote Audio Capture-Transmission-Play software including Server, Client, and Network Transmission
• On Windows server, captured audio by injecting DLL into processes and intercept system audio stream<br />
• Designed audio mixing, AAC encoding, TCP network transmission and AAC decoding modules<br />
• On client, customized audio player with buffer mechanism to synchronize with video <br />

Instructions:
1. Locate Line 21 in InjectDLL/InjectDLL.cpp, change the if-sentence with the process names you want to inject. You can see currently it's working on "QQmusic.exe" and "splayer.exe".

2. Locate Line 19 in TestLibPlayer/TestLibPlayer.h, change SERVER_ADD to the actual address of server. If you want to test locally, just leave it be.

3. Build every subprojects except 'InjectDLL'.

4. Locate Line 25 in InjectDLL/InjectDLL.cpp, change the path to AudioHook2.dll file, which you built in project 'AudioHook'.

5. Build 'InjectDLL'.

6. On server, run the processes that you want to capture audio.

7. On server, run InjectDll.exe in 'InjectDLL' to inject the dll into the processes and start capturing audio.

8. On server, run TestCaptureAudio.exe in 'TestCaptureAudio' to read audio from buffer and mix.

9. On client, run TestLibPlayer.exe in 'TestLibPlayer' to play audio.
