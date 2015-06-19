#undef UNICODE
#include <vector>
#include <string>
#include <windows.h>
#include <Tlhelp32.h>
using std::vector;
using std::string;

int main(void)
{
	vector<string>processNames; //Hold every process available
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL); //Create the snapshot
	BOOL bProcess = Process32First(hTool32, &pe32); //Call this and see if its valid
	if(bProcess == TRUE)
	{
		while((Process32Next(hTool32, &pe32)) == TRUE) //While processes left to be enumerated
		{
			processNames.push_back(pe32.szExeFile); //Save process name
			if(strcmp(pe32.szExeFile, "QQMusic.exe") == 0 || strcmp(pe32.szExeFile, "splayer.exe") == 0) //Process we want to inject to
			{
				printf("%d\n", pe32.th32ProcessID);

				char* FullPath = "D:\\Audio Cap and Play\\TestCaptureAudio\\Debug\\AudioHook2.dll";
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				//printf("%x\n", hProcess);
				LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle("Kernel32.dll"),
					"LoadLibraryA"); //Get LoadLibraryA address

				int cb = (1 + lstrlenA(FullPath)) * sizeof(char);

				LPVOID LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, cb,
					MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); //Allocate some memory for DLL string

				if (LLParam == NULL) printf("Allocate failed.\n");
				else printf("Allocate success.\n");

				if (!WriteProcessMemory(hProcess, LLParam, FullPath, cb, NULL)) //Write it
					printf("Write failed.\n");
				else printf("Write success.\n");
				
				HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr,
					LLParam, 0, NULL); //New thread with LoadLibrary as start and our string as param
				if (hRemoteThread == NULL) printf("Create Thread failed.\n");
				else printf("Create Thread success.\n");

				CloseHandle(hProcess);
				//delete [] DirPath;
				//delete [] FullPath; 
			}
		}
	}
	CloseHandle(hTool32);
	system("pause");
	return 0;
}