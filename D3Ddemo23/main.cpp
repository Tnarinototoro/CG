#include"base.h"
DWORD first=0, last = 0;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wndClass = { 0 };							
	wndClass.cbSize = sizeof(WNDCLASSEX);			
	wndClass.style = CS_HREDRAW | CS_VREDRAW;	
	wndClass.lpfnWndProc = WndProc;					
	wndClass.cbClsExtra = 0;							
	wndClass.cbWndExtra = 0;							
	wndClass.hInstance = hInstance;						
	wndClass.hIcon = (HICON)::LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);  
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   
	wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);  
	wndClass.lpszMenuName = NULL;						
	wndClass.lpszClassName = L"ForTheDream";		
	
																
	if (!RegisterClassEx(&wndClass))				
		return -1;
	HWND hwnd = CreateWindowExW(WS_EX_LAYERED, L"ForTheDream", L"Window", WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, hInstance, 0);
	if (hwnd == NULL)
		return 1;
	SetLayeredWindowAttributes(hwnd, NULL, 255, LWA_ALPHA); // Tomato
	if (!(S_OK == Direct3D_Init(hwnd, hInstance)))
	{
		MessageBox(hwnd, _T("Direct3D初始化失败~！"), _T("Error"), 0);
	}			
	MoveWindow(hwnd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, true);		
	ShowWindow(hwnd, nShowCmd);   
	UpdateWindow(hwnd);

	bool bGotMsg;
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		// Use PeekMessage() so we can use idle time to render the scene. 
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		if (bGotMsg)
		{
			// Translate and dispatch the message
			if ( hwnd == NULL ||
				0 == TranslateAccelerator(hwnd,NULL, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			last = GetTickCount64();
			if (last - first > 0)
			{
				Direct3D_Update(hwnd);
				Direct3D_Render(hwnd);
				first = last;
			}
		}
	}


	
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)						
	{
	case WM_PAINT:			
		Direct3D_Render(hwnd);                
		ValidateRect(hwnd, NULL);		
		break;									

	case WM_KEYDOWN:					
	{
		if (wParam == VK_ESCAPE)
		{
			if (AllSources->m_gamestate == RenderStage_InGame)
			{
				BackgroundMusic->PlaysoundTotheend();
				AllSources->m_gamestate = RenderStage_GamePause;
				ShowCursor(true);
			}
			else if (AllSources->m_gamestate == RenderStage_GamePause)
			{
				BackgroundMusic->PauseAudio();
				AllSources->m_gamestate = RenderStage_InGame;
				ShowCursor(false);
			}
			else
				;
		}
		break;
	}

	case WM_DESTROY:					
	{
		Direct3D_CleanUp();
		PostQuitMessage(0);
		break;
	}
	default:										
		return DefWindowProc(hwnd, message, wParam, lParam);		
	}
	
	return 0;									
}










