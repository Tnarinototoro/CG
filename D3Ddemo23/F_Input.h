#pragma once
#include "Runtime.h"
#ifndef F_INPUT_H
#define F_INPUT_H
#endif // !Util_H
#define MouseType 0
#define KeyBoardType 1
#define LBUTTON_PRESSED 2
#define LBUTTON_RELEASED 3
#define RBUTTON_PRESSED 4
#define RBUTTON_RELEASED 5
#define MBUTTON_PRESSED 6
#define MBUTTON_RELEASED 7
class F_Input
{
	char ReadBuffer[256];
	LPDIRECTINPUT8 Input8com;
	LPDIRECTINPUTDEVICE8 MouseDevice;
	LPDIRECTINPUTDEVICE8 KeyBoardDevice;
	DIMOUSESTATE mousestate;
public:
	F_Input();
	~F_Input();
	
	HRESULT Init(HWND hwnd, HINSTANCE Hinstance);
	HRESULT StateRead(int Devicechoose);
    int IskeyPressed(int Device, int Key);
	LONG GetMouseMovement(int axis);
	int LeftButton();
	int MiddleButton();
	int RightButton();

};
F_Input::F_Input()
{
	ReadBuffer[256] = { 0 };
	Input8com = NULL;
	MouseDevice = NULL;
	KeyBoardDevice = NULL;
	mousestate = { 0 };
}
F_Input::~F_Input()
{
	SAFE_RELEASE(Input8com);
	SAFE_RELEASE(KeyBoardDevice);
	SAFE_RELEASE(MouseDevice);

}
HRESULT F_Input::Init(HWND hwnd, HINSTANCE Hinstance)
{
	if (FAILED(DirectInput8Create(Hinstance, 0x0800, IID_IDirectInput8, (void**)&Input8com, NULL)))
	{
		MessageBox(hwnd, L"initiation Failed!", L"Error", 0);
		return E_FAIL;
	}
	//keyboard init
	Input8com->CreateDevice(GUID_SysKeyboard, &KeyBoardDevice, 0);
	Input8com->CreateDevice(GUID_SysMouse, &MouseDevice, 0);
	KeyBoardDevice->SetDataFormat(&c_dfDIKeyboard);
	KeyBoardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	KeyBoardDevice->Acquire();
	//mouse init
	MouseDevice->SetDataFormat(&c_dfDIMouse);
	MouseDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	MouseDevice->Acquire();
	return S_OK;


}
int F_Input::IskeyPressed(int Device, int Key)
{
	if (Device == KeyBoardType)
	{
		return (ReadBuffer[Key] & 0x80);
	}
	if (Device == MouseType)
	{
		return (mousestate.rgbButtons[Key] & 0x80);
	}
	return 0;

}
LONG F_Input::GetMouseMovement(int axis)
{
	switch (axis)
	{
	case 0://x axis
		return mousestate.lX;
		break;
	case 1://y axis
		return mousestate.lY;
		break;
	case 2://z axis
		return mousestate.lZ;
		break;
	default:
		break;
	}
	return 0;
}
HRESULT F_Input::StateRead(int Devicechoose)
{
	//keyboard read
	if (Devicechoose == KeyBoardType)
	{
		HRESULT hr;
		KeyBoardDevice->Poll();
		KeyBoardDevice->Acquire();
		while (true)
		{
			if (SUCCEEDED(hr = KeyBoardDevice->GetDeviceState(sizeof(ReadBuffer), (LPVOID)ReadBuffer)))
			{
				break;
			}
			if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED) return E_FAIL;//处理数据获取异常
			if (FAILED(KeyBoardDevice->Acquire())) return E_FAIL; //处理取得权限异常

		}
		return hr;

	}
	if (Devicechoose == MouseType)
	{
		HRESULT hr;
		MouseDevice->Poll();
		MouseDevice->Acquire();
		while (true)
		{
			if (SUCCEEDED(hr = MouseDevice->GetDeviceState(sizeof(mousestate), (LPVOID)&mousestate)))
			{
				break;
			}
			if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED) return E_FAIL;//处理数据获取异常
			if (FAILED(MouseDevice->Acquire())) return E_FAIL; //处理取得权限异常

		}
		return hr;

	}
	return E_FAIL;

}
int F_Input::LeftButton()
{
	if (mousestate.rgbButtons[0] & 0x80)
	{
		return LBUTTON_PRESSED;
	}
	else
		return  LBUTTON_RELEASED;
}
int F_Input::RightButton()
{
	if (mousestate.rgbButtons[1] & 0x80)
	{
		return RBUTTON_PRESSED;
	}
	else
		return  RBUTTON_RELEASED;
}
int F_Input::MiddleButton()
{
	if (mousestate.rgbButtons[2] & 0x80)
	{
		return MBUTTON_PRESSED;
	}
	else
		return  MBUTTON_RELEASED;
}