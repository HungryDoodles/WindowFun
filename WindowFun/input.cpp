#include "input.h"

#include <Windows.h>
#include <winnt.h>
#include <stdio.h>

//https://blogs.msdn.microsoft.com/oldnewthing/20041025-00/?p=37483
//http://stackoverflow.com/questions/1749972/determine-the-current-hinstance
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)


DirectInput::DirectInput() {

}

int DirectInput::init() {
    // Initialize the main direct input interface.
    int result = DirectInput8Create(HINST_THISCOMPONENT, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
    if (FAILED(result)) {
        return 0;
    }

    //The first input device we will initialize will be the keyboard.

    // Initialize the direct input interface for the keyboard.
    result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format.  In this case since it is a keyboard we can use the predefined data format.
    result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result))
    {
        return false;
    }
    //Setting the cooperative level of the keyboard is important in both what it does and how you use the device from that point forward. In this case we will set it to not share with other programs (DISCL_EXCLUSIVE). This way if you press a key only your application can see that input and no other application will have access to it. However if you want other applications to have access to keyboard input while your program is running you can set it to non-exclusive (DISCL_NONEXCLUSIVE). Now the print screen key works again and other running applications can be controlled by the keyboard and so forth. Just remember that if it is non-exclusive and you are running in a windowed mode then you will need to check for when the device loses focus and when it re-gains that focus so it can re-acquire the device for use again. This focus loss generally happens when other windows become the main focus over your window or your window is minimized.

    //Set the cooperative level of the keyboard to not share with other programs.
    //HWND hwnd = GetActiveWindow();
    //result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    ////result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    //if(FAILED(result))
    //{
    //    return false;
    //}
    //Once they keyboard is setup we then call Acquire to finally get access to the keyboard for use from this point forward.

    // Now acquire the keyboard.
    result = m_keyboard->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the mouse.
    result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
    if (FAILED(result))
    {
        return 0;
    }

    // Set the data format for the mouse using the pre-defined mouse data format.
    result = m_mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result)) {
        return 0;
    }

    // MOUSE
    //We use non-exclusive cooperative settings for the mouse. We will have to check for when it goes in and out of focus and re-acquire it each time.
    // Set the cooperative level of the mouse to share with other programs.
    //HWND hwnd = GetActiveWindow();
    //result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    //if(FAILED(result))
    //{
    //    return 0;
    //}

    result = m_mouse->Acquire();
    if (FAILED(result)) {
        return 0;
    }
    return 1;
    //Sys_Printf("got the mouse!\n");
}
bool DirectInput::ReadKeyboard() {
    HRESULT result;
    memcpy(m_keyboardStateOld, m_keyboardState, sizeof(unsigned char) * 256);
    memset(m_keyboardState, 0, 256); // probably not needed, but who knows
    result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)m_keyboardState);
    if (FAILED(result)) {
        printf("failed ReadKeyboard()\n");
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            m_keyboard->Acquire();
        }
        else {
            return false;
        }
    }
    return true;
}

bool DirectInput::Update()
{
    return ReadMouse() && ReadKeyboard();
}

bool DirectInput::KeyUp(unsigned char key) const
{
    return (m_keyboardState[key] & 0x00) && (m_keyboardStateOld[key] & 0x80);
}

bool DirectInput::KeyDown(unsigned char key) const
{
    return (m_keyboardState[key] & 0x80) && (m_keyboardStateOld[key] & 0x00);
}

bool DirectInput::Key(unsigned char key) const
{
    return (m_keyboardState[key] & 0x80);
}

int DirectInput::MouseDX() const 
{
    return m_mouseState.lX;
}

int DirectInput::MouseDY() const 
{
    return m_mouseState.lY;
}

bool DirectInput::MouseUp(int button) const
{
    return (m_mouseState.rgbButtons[button] & 0x00) && (m_mouseStateOld.rgbButtons[button] & 0x80);
}

bool DirectInput::MouseDown(int button) const
{
    return (m_mouseState.rgbButtons[button] & 0x80) && (m_mouseStateOld.rgbButtons[button] & 0x00);
}

bool DirectInput::Mouse(int button) const
{
    return (m_mouseState.rgbButtons[button] & 0x80);
}

bool DirectInput::ReadMouse() {
    HRESULT result;
    m_mouseStateOld = m_mouseState;
    result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
    if (FAILED(result)) {
        // If the mouse lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            m_mouse->Acquire();
        }
        else {
            return false;
        }
    }
    return true;
}
/*
DirectInput* directinput = NULL;

int main() {
    directinput = new DirectInput();
    directinput->init();
    while (1) {
        int ret = directinput->ReadKeyboard();
        if (!ret)
            continue;

        int keys_pressed = 0;
        for (int i = 0; i < 256; i++) {
            if ((directinput->m_keyboardState[i] & 128) == 0)
                continue;
            unsigned char scancode = i;
            UINT key = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
            //UINT key = MapVirtualKeyEx(scancode, MAPVK_VSC_TO_VK, GetKeyboardLayout(0)); // same as MapVirtualKey

            switch (scancode) {
            case 203: key = VK_LEFT; break;
            case 205: key = VK_RIGHT; break;
            case 200: key = VK_UP; break;
            case 208: key = VK_DOWN; break;
            case 211: key = VK_DELETE; break;
            case 207: key = VK_END; break;
            case 199: key = VK_HOME; break; // pos1
            case 201: key = VK_PRIOR; break; // page up
            case 209: key = VK_NEXT; break;  // page down
            case 210: key = VK_INSERT; break;
            case 184: key = VK_RMENU; break; // right alt
            case 157: key = VK_RCONTROL; break; // right control
            case 219: key = VK_LWIN; break; // left win
            case 220: key = VK_RWIN; break; // right win
            case 156: key = VK_RETURN; break; // right enter
            case 181: key = VK_DIVIDE; break; // numpad divide
            case 221: key = VK_APPS; break; // menu key
            }

            printf("keys_pressed=%d scancode=%d/0x%x key=%d char=%c hex=0x%x\n", keys_pressed, scancode, scancode, key, key, key);
            keys_pressed++;
        }

        Sleep((int)(1000.0 / 60.0));
    }
    return 0;
}*/