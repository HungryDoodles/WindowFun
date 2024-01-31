#pragma once
#include "Windows.h"

// Just to isolate this motherfucker
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>


class DirectInput {
public:
    IDirectInput8* m_directInput;
    IDirectInputDevice8* m_keyboard;
    IDirectInputDevice8* m_mouse;
    DIMOUSESTATE m_mouseState;
    DIMOUSESTATE m_mouseStateOld;
    DirectInput();
    int init();
    bool ReadMouse();
    unsigned char m_keyboardState[256];
    unsigned char m_keyboardStateOld[256];
    bool ReadKeyboard();
    
    bool Update();

    bool KeyUp(unsigned char key) const;
    bool KeyDown(unsigned char key) const;
    bool Key(unsigned char key) const;

    int MouseDX() const;
    int MouseDY() const;

    // WIP
    bool MouseUp(int button) const;
    bool MouseDown(int button) const;
    bool Mouse(int button) const;
};
