#include "pch.h"
#include "hook.h"
#include "kiero.h"
#include "MinHook.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "injector.hpp"
#include "font.h"
#include "kiero.h"
#include "scriptextender.hpp"
#include "wrapper.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Configuration for Chinese font support
static bool g_EnableChineseSupport = true;

static const ImWchar* GetGlyphRanges() {
    if (g_EnableChineseSupport) {
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0980, 0x09FF, // Bengali
            0x2000, 0x206F, // General Punctuation

            // Chinese
            0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
            0x31F0, 0x31FF, // Katakana Phonetic Extensions
            0xFF00, 0xFFEF, // Half-width characters
            0x4E00, 0x9FAF, // CJK Ideograms

            // Cyrillic
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
            0x2DE0, 0x2DFF, // Cyrillic Extended-A
            0xA640, 0xA69F, // Cyrillic Extended-B

            //Turkish
            0x011E, 0x011F,
            0x015E, 0x015F,
            0x0130, 0x0131,
            0,
        };
        return &ranges[0];
    } else {
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0980, 0x09FF, // Bengali
            0x2000, 0x206F, // General Punctuation

            // Cyrillic
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
            0x2DE0, 0x2DFF, // Cyrillic Extended-A
            0xA640, 0xA69F, // Cyrillic Extended-B

            //Turkish
            0x011E, 0x011F,
            0x015E, 0x015F,
            0x0130, 0x0131,
            0,
        };
        return &ranges[0];
    }
}

bool Hook::GetMouseState() {
    return mouseVisible;
}

void Hook::SetMouseState(bool state) {
    mouseVisible = state;
}

LRESULT Hook::hkWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (m_bInitialized) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        if (ImGui::GetIO().WantTextInput || (gGameVer > eGameVer::SA && mouseVisible)) {
    #ifndef _WIN64
            if (gGameVer == eGameVer::SA) {
                reinterpret_cast<void(__cdecl*)()>(0x53F1E0)(); // CPad::ClearKeyboardHistory
            }
    #endif
            return 1;
        }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT Hook::hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();

    return oReset(pDevice, pPresentationParameters);
}

void Hook::ProcessFrame(void* ptr) {
    if (m_bInitialized) {
        ProcessMouse();

        // Scale the menu if game resolution changed
        static int height, width, RsGlobal;

#ifndef _WIN64
        if (gGameVer == eGameVer::III) {
            RsGlobal = 0x8F4360;
            width = injector::ReadMemory<int>(RsGlobal + 4, 0);      // width
            height = injector::ReadMemory<int>(RsGlobal + 8, 0);    // height
        } else if (gGameVer == eGameVer::VC) {
            RsGlobal = 0x9B48D8;
            width = injector::ReadMemory<int>(RsGlobal + 4, 0);      // width
            height = injector::ReadMemory<int>(RsGlobal + 8, 0);    // height
        } else if (gGameVer == eGameVer::SA) {
            RsGlobal = 0xC17040;
            width = injector::ReadMemory<int>(RsGlobal + 4, 0);      // width
            height = injector::ReadMemory<int>(RsGlobal + 8, 0);    // height
        } else {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
        }
#else 
        RECT rect;
        GetWindowRect(hwnd, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
#endif

        static ImVec2 fScreenSize = ImVec2(-1, -1);
        if (fScreenSize.x != width && fScreenSize.y != height) {
            if (gRenderer == eRenderer::Dx9) {
                ImGui_ImplDX9_InvalidateDeviceObjects();
            } else if (gRenderer == eRenderer::Dx11) {
                ImGui_ImplDX11_InvalidateDeviceObjects();
            }

            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();
            float fontSize = height / 48.0f;
            io.FontDefault = io.Fonts->AddFontFromMemoryCompressedBase85TTF(fontData, fontSize, NULL, GetGlyphRanges());
            io.Fonts->Build();

            ImGuiStyle* style = &ImGui::GetStyle();
            float scaleX = width / 1366.0f;
            float scaleY = height / 768.0f;

            style->TabRounding = 0.0f;
            style->ChildBorderSize = 0;
            style->WindowBorderSize = 0;
            style->FrameBorderSize = 0;
            style->TabBorderSize = 0;
            style->PopupBorderSize = 0;
            style->FramePadding = ImVec2(5 * scaleX, 3 * scaleY);
            style->ItemSpacing = ImVec2(4 * scaleY, 4 * scaleY);
            style->ScrollbarSize = 12 * scaleX;
            style->IndentSpacing = 20 * scaleX;
            style->ItemInnerSpacing = ImVec2(4 * scaleY, 4 * scaleY);
            style->Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
            style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
            style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
            fScreenSize = ImVec2((float)width, (float)height);
            ScriptExData::SetScaling({scaleX, scaleY});
        }

        ScriptExData::InitRenderStates();

        ImGui_ImplWin32_NewFrame();
        if (gRenderer == eRenderer::Dx9) {
            ImGui_ImplDX9_NewFrame();
        } else if (gRenderer == eRenderer::Dx11) {
            ImGui_ImplDX11_NewFrame();
        }

        ImGui::NewFrame();

        if (pCallbackFunc != nullptr) {
            static_cast<void(*)()>(pCallbackFunc)();
        }

        ImGui::EndFrame();
        ImGui::Render();

        if (gRenderer == eRenderer::Dx9) {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        } else if (gRenderer == eRenderer::Dx11) {
            pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
    } else {
        if (!ImGui::GetCurrentContext()) {
            ImGui::CreateContext();
        }

        if (gGameVer == eGameVer::SA) {
            injector::MakeNOP(0x531155, 5); // shift trigger fix
        }

        if (gRenderer == eRenderer::Dx9) {
            hwnd = GetForegroundWindow();
            if (!ImGui_ImplWin32_Init(hwnd)) {
                return;
            }

            if (!ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(ptr))) {
                return;
            }
            gD3DDevice = ptr;
        } else if (gRenderer == eRenderer::Dx11) {
            IDXGISwapChain* pSwapChain = reinterpret_cast<IDXGISwapChain*>(ptr);
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), &ptr))) {
                ID3D11Device *pDevice = reinterpret_cast<ID3D11Device*>(ptr);
                pDevice->GetImmediateContext(&pDeviceContext);

                DXGI_SWAP_CHAIN_DESC Desc;
                pSwapChain->GetDesc(&Desc);
                hwnd = Desc.OutputWindow;

                ID3D11Texture2D* backBuffer;
                pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
                pDevice->CreateRenderTargetView(backBuffer, NULL, &pRenderTargetView);
                backBuffer->Release();

                if (!ImGui_ImplWin32_Init(hwnd)) {
                    return;
                }
                ImGui_ImplDX11_Init(pDevice, pDeviceContext);
                ImGui_ImplDX11_CreateDeviceObjects();
            }

            gD3DDevice = ptr;
        } else {
            hwnd = GetForegroundWindow();
            if (!ImGui_ImplWin32_Init(hwnd)) {
                return;
            }
        }

        ImGui_ImplWin32_EnableDpiAwareness();

        ShowCursor(false);
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NoMouseCursorChange;

        oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, -4, (LRESULT)hkWndProc); // GWL_WNDPROC = -4
        m_bInitialized = true;
    }
}

HRESULT Hook::hkEndScene(IDirect3DDevice9* pDevice) {
    ProcessFrame(pDevice);
    return oEndScene(pDevice);
}

HRESULT Hook::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    ProcessFrame(pSwapChain);
    return oPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT Hook::hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT a, UINT b, UINT c, DXGI_FORMAT d, UINT e) {
    if (pRenderTargetView) {
        pRenderTargetView->Release();
        pRenderTargetView = nullptr;
        pDeviceContext->Flush();
    }

    HRESULT hr = oResizeBuffers(pSwapChain, a, b, c, d, e);
    ID3D11Texture2D* back_buffer{};
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
    reinterpret_cast<ID3D11Device*>(pSwapChain)->CreateRenderTargetView(back_buffer, nullptr, &pRenderTargetView);
    back_buffer->Release();
    return hr;
}

bool Hook::hkGlSwapBuffer(HDC unnamedParam1, UINT unnamedParam2) {
    ProcessFrame(nullptr);
    return oGlSwapBuffer(unnamedParam1, unnamedParam2);
}

void Hook::ProcessMouse() {
    static bool curState = false;
    if (curState != mouseVisible) {
        ImGui::GetIO().MouseDrawCursor = mouseVisible;

        /*
            Skip mouse patches on unknown host
            ImGui menus should be interactive on game menu
        */
#ifndef _WIN64
        if (gGameVer == eGameVer::SA) {
            if (ImGui::GetIO().MouseDrawCursor) {
                injector::WriteMemory<unsigned char>(0x6194A0, 0xC3, true);
                injector::MakeNOP(0x541DD7, 5, true);
            } else {
                injector::WriteMemory<unsigned char>(0x6194A0, 0xE9, true);
                injector::WriteMemoryRaw(0x541DD7, (char*)"\xE8\xE4\xD5\xFF\xFF", 5, true);
            }

            // ClearMouseStates
            injector::WriteMemory<float>(0xB73418 + 12, 0, true); // X
            injector::WriteMemory<float>(0xB73418 + 16, 0, true); // Y

            reinterpret_cast<void(__cdecl*)()>(0x541BD0)(); // CPad::ClearMouseHistory();
            reinterpret_cast<void(__cdecl*)()>(0x541DD0)(); // CPad::UpdatePads();
        } else if (gGameVer == eGameVer::VC) {
            if (ImGui::GetIO().MouseDrawCursor) {
                injector::WriteMemory<unsigned char>(0x6020A0, 0xC3, true);
                injector::MakeNOP(0x4AB6CA, 5, true);
            } else {
                injector::WriteMemory<unsigned char>(0x6020A0, 0x53, true);
                injector::WriteMemoryRaw(0x4AB6CA, (char*)"\xE8\x51\x21\x00\x00", 5, true);
            }

            // ClearMouseStates
            injector::WriteMemory<float>(0x94D788 + 8, 0, true); // X
            injector::WriteMemory<float>(0x94D788 + 12, 0, true);// Y

            reinterpret_cast<void(__cdecl*)()>(0x4ADB30)(); // CPad::ClearMouseHistory();
            reinterpret_cast<void(__cdecl*)()>(0x4AB6C0)(); // CPad::UpdatePads();
        } else if (gGameVer == eGameVer::III) {
            if (ImGui::GetIO().MouseDrawCursor) {
                injector::WriteMemory<unsigned char>(0x580D20, 0xC3, true);
                injector::MakeNOP(0x49272F, 5, true);
            } else {
                injector::WriteMemory<unsigned char>(0x580D20, 0x53, true);
                injector::WriteMemoryRaw(0x49272F, (char*)"\xE8\x6C\xF5\xFF\xFF", 5, true);
            }

            // ClearMouseStates
            injector::WriteMemory<float>(0x8809F0 + 8, 0, true); // X
            injector::WriteMemory<float>(0x8809F0 + 12, 0, true);// Y

            int pad = reinterpret_cast<int(__thiscall*)(int)>(0x492F60)(NULL); // CPad::GetPads();
            reinterpret_cast<void(__thiscall*)(int)>(0x491B50)(pad); // CPad::ClearMouseHistory();
            reinterpret_cast<void(__cdecl*)()>(0x492720)(); // CPad::UpdatePads();
        }
#endif

        curState = mouseVisible;
    }
}

static bool IsKeyPressed(int i, LPVOID data) {
    return reinterpret_cast<char*>(data)[i] & 0x80;
}

ImGuiKey VirtualKeyToImGuiKey(int vk);

HRESULT CALLBACK Hook::hkGetDeviceState(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData) {
    HRESULT result = oGetDeviceState(pThis, cbData, lpvData);

    /*
    * We're detecting it here since usual WndProc doesn't seem to work for bully
    * This probably should work for other games using dinput too..?
    */
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::GetIO().MouseDrawCursor) {
        int frameCount = ImGui::GetFrameCount();
        if (cbData == 16) { // mouse
            LPDIMOUSESTATE2 mouseState = reinterpret_cast<LPDIMOUSESTATE2>(lpvData);

            // Block camera rotation
            mouseState->lX = 0;
            mouseState->lY = 0;
            io.MouseWheel = mouseState->lZ / static_cast<float>(WHEEL_DELTA);
            mouseState->lZ = 0;

            static int mouseCount = -1;
            if (frameCount != mouseCount) {
                io.MouseDown[0] = (mouseState->rgbButtons[0] != 0);
                io.MouseDown[1] = (mouseState->rgbButtons[1] != 0);
                mouseCount = frameCount;
            }

            // Block left & right clicks
            mouseState->rgbButtons[0] = 0;
            mouseState->rgbButtons[1] = 0;
        } else if (cbData == 256) { // keyboard
            /*
            *   GetDeviceData doesn't work
            */
            static int keyCount = -1;
            if (frameCount != keyCount) {
                io.KeyAlt = IsKeyPressed(DIK_LALT, lpvData) || IsKeyPressed(DIK_RALT, lpvData);
                io.KeyCtrl = IsKeyPressed(DIK_LCONTROL, lpvData) || IsKeyPressed(DIK_RCONTROL, lpvData);
                io.KeyShift = IsKeyPressed(DIK_LSHIFT, lpvData) || IsKeyPressed(DIK_RSHIFT, lpvData);

                for (size_t i = 0; i < cbData; ++i) {
                    bool pressed = IsKeyPressed(i, lpvData);
                    UINT vk = MapVirtualKeyEx(i, MAPVK_VSC_TO_VK, GetKeyboardLayout(NULL));

                    ImGuiKey imgui_key = VirtualKeyToImGuiKey(vk); // You define this mapping

                    if (imgui_key != ImGuiKey_None) {
                        io.AddKeyEvent(imgui_key, pressed);

                        if (pressed) {
                            WCHAR c;
                            BYTE keystate[256] = {};
                            ToUnicode(vk, i, keystate, &c, 1, 0);

                            // Capital letters on shift hold
                            if (io.KeyShift && c >= 0x61 && c <= 0x7A) {
                                c -= 0x20;
                            }

                            io.AddInputCharacterUTF16(c);
                        }
                    }
                }
                keyCount = frameCount;
            }
        }
    }

    if (io.WantTextInput) {
        ZeroMemory(lpvData, 256);
        result = oGetDeviceState(pThis, cbData, lpvData);
    }
    return result;
}

bool Hook::GetDinputDevice(void** pMouse, size_t size) {
    if (!pMouse) {
        return false;
    }

    IDirectInput8* pDirectInput = NULL;

    // Create dummy device
    if (DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
        return false;
    }

    LPDIRECTINPUTDEVICE8  lpdiInput;

    /*
    * We're creating a sysMouse but it still seems to receive keyboard messages?
    */
    if (pDirectInput->CreateDevice(GUID_SysMouse, &lpdiInput, NULL) != DI_OK) {
        pDirectInput->Release();
        return false;
    }

    lpdiInput->SetDataFormat(&c_dfDIKeyboard);
    lpdiInput->SetCooperativeLevel(GetActiveWindow(), DISCL_NONEXCLUSIVE);
    memcpy(pMouse, *reinterpret_cast<void***>(lpdiInput), size);
    lpdiInput->Release();
    pDirectInput->Release();
    return true;
}

BOOL CALLBACK Hook::hkSetCursorPos(int x, int y) {
    if (ImGui::GetIO().MouseDrawCursor) {
        return true;
    }
    return oSetCursorPos(x, y);
}

BOOL CALLBACK Hook::hkShowCursor(bool flag) {
    if (ImGui::GetIO().MouseDrawCursor) {
        return oShowCursor(TRUE);
    }
    return oShowCursor(flag);
}

void Hook::InputWatcher()
{
	while (true)
	{
		ImGuiIO& io = ImGui::GetIO();
        if (io.MouseDrawCursor) {
			io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        }
		Sleep(1);
	}
}

bool Hook::Inject(void *pCallback) {
    static bool injected;
    if (injected) {
        return false;
    }

    ImGui::CreateContext();
    MH_Initialize();
    PVOID pSetCursorPos = GetProcAddress(GetModuleHandle("user32.dll"), "SetCursorPos");
    PVOID pShowCursor = GetProcAddress(GetModuleHandle("user32.dll"), "ShowCursor");
    MH_CreateHook(pSetCursorPos, hkSetCursorPos, reinterpret_cast<LPVOID*>(&oSetCursorPos));
    MH_CreateHook(pShowCursor, hkShowCursor, reinterpret_cast<LPVOID*>(&oShowCursor));
    MH_EnableHook(pSetCursorPos);
    MH_EnableHook(pShowCursor);

    /*
        Must check for d3d9 first!
        Seems to crash with nvidia geforce experience overlay
        if anything else is checked before d3d9
    */
    if (GetModuleHandle("_gtaRenderHook.asi")
            || gGameVer == eGameVer::III_DE || gGameVer == eGameVer::VC_DE || gGameVer == eGameVer::SA_DE) {
        goto dx11;
    }

    if (init(kiero::RenderType::D3D9) == kiero::Status::Success) {
        gRenderer = eRenderer::Dx9;
        injected = true;
        kiero::bind(16, reinterpret_cast<LPVOID*>(&oReset), hkReset);
        kiero::bind(42, reinterpret_cast<LPVOID*>(&oEndScene), hkEndScene);
        pCallbackFunc = pCallback;
    }
dx11:
    if (init(kiero::RenderType::D3D11) == kiero::Status::Success) {
        gRenderer = eRenderer::Dx11;
        kiero::bind(8, reinterpret_cast<LPVOID*>(&oPresent), hkPresent);
        kiero::bind(13, reinterpret_cast<LPVOID*>(&oResizeBuffers), hkResizeBuffers);
        pCallbackFunc = pCallback;
        injected = true;
    }

    if (gGameVer == eGameVer::BullySE) {
        static void *diMouse[32];
        if (GetDinputDevice(diMouse, sizeof(diMouse))) {
            MH_CreateHook(diMouse[9], hkGetDeviceState, reinterpret_cast<LPVOID*>(&oGetDeviceState));
            MH_EnableHook(diMouse[9]);
        }
    }

    if (gGameVer == eGameVer::IV) {
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(InputWatcher), nullptr, 0, nullptr);
    }

    return injected;
}

void Hook::Remove() {
    pCallbackFunc = nullptr;
    SetWindowLongPtr(hwnd, -4, (LRESULT)oWndProc); // GWL_WNDPROC = -4

    if (gRenderer == eRenderer::Dx9) {
        ImGui_ImplDX9_Shutdown();
    } else if (gRenderer == eRenderer::Dx11) {
        ImGui_ImplDX11_Shutdown();
    }

    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    kiero::shutdown();
}

ImGuiKey VirtualKeyToImGuiKey(int vk) {
    if (vk >= 'A' && vk <= 'Z') return (ImGuiKey)(ImGuiKey_A + (vk - 'A'));
    if (vk >= '0' && vk <= '9') return (ImGuiKey)(ImGuiKey_0 + (vk - '0'));

    switch (vk) {
        // Function keys
        case VK_F1:  return ImGuiKey_F1;
        case VK_F2:  return ImGuiKey_F2;
        case VK_F3:  return ImGuiKey_F3;
        case VK_F4:  return ImGuiKey_F4;
        case VK_F5:  return ImGuiKey_F5;
        case VK_F6:  return ImGuiKey_F6;
        case VK_F7:  return ImGuiKey_F7;
        case VK_F8:  return ImGuiKey_F8;
        case VK_F9:  return ImGuiKey_F9;
        case VK_F10: return ImGuiKey_F10;
        case VK_F11: return ImGuiKey_F11;
        case VK_F12: return ImGuiKey_F12;

        // Navigation
        case VK_LEFT:     return ImGuiKey_LeftArrow;
        case VK_RIGHT:    return ImGuiKey_RightArrow;
        case VK_UP:       return ImGuiKey_UpArrow;
        case VK_DOWN:     return ImGuiKey_DownArrow;
        case VK_HOME:     return ImGuiKey_Home;
        case VK_END:      return ImGuiKey_End;
        case VK_PRIOR:    return ImGuiKey_PageUp;
        case VK_NEXT:     return ImGuiKey_PageDown;
        case VK_INSERT:   return ImGuiKey_Insert;
        case VK_DELETE:   return ImGuiKey_Delete;
        case VK_BACK:     return ImGuiKey_Backspace;
        case VK_RETURN:   return ImGuiKey_Enter;
        case VK_ESCAPE:   return ImGuiKey_Escape;
        case VK_SPACE:    return ImGuiKey_Space;
        case VK_TAB:      return ImGuiKey_Tab;

        // Modifiers
        case VK_SHIFT:    return ImGuiKey_LeftShift;   // You may want to distinguish left/right
        case VK_LSHIFT:   return ImGuiKey_LeftShift;
        case VK_RSHIFT:   return ImGuiKey_RightShift;
        case VK_CONTROL:  return ImGuiKey_LeftCtrl;
        case VK_LCONTROL: return ImGuiKey_LeftCtrl;
        case VK_RCONTROL: return ImGuiKey_RightCtrl;
        case VK_MENU:     return ImGuiKey_LeftAlt;
        case VK_LMENU:    return ImGuiKey_LeftAlt;
        case VK_RMENU:    return ImGuiKey_RightAlt;
        case VK_LWIN:     return ImGuiKey_LeftSuper;
        case VK_RWIN:     return ImGuiKey_RightSuper;

        // Punctuation and symbols
        case VK_OEM_1:    return ImGuiKey_Semicolon;     // ;:
        case VK_OEM_PLUS: return ImGuiKey_Equal;         // =+
        case VK_OEM_COMMA:return ImGuiKey_Comma;         // ,<
        case VK_OEM_MINUS:return ImGuiKey_Minus;         // -_
        case VK_OEM_PERIOD:return ImGuiKey_Period;       // .>
        case VK_OEM_2:    return ImGuiKey_Slash;          // /?
        case VK_OEM_3:    return ImGuiKey_GraveAccent;    // `~
        case VK_OEM_4:    return ImGuiKey_LeftBracket;    // [{
        case VK_OEM_5:    return ImGuiKey_Backslash;      // \|
        case VK_OEM_6:    return ImGuiKey_RightBracket;   // ]}
        case VK_OEM_7:    return ImGuiKey_Apostrophe;     // '"

        // Numpad
        case VK_NUMPAD0:  return ImGuiKey_Keypad0;
        case VK_NUMPAD1:  return ImGuiKey_Keypad1;
        case VK_NUMPAD2:  return ImGuiKey_Keypad2;
        case VK_NUMPAD3:  return ImGuiKey_Keypad3;
        case VK_NUMPAD4:  return ImGuiKey_Keypad4;
        case VK_NUMPAD5:  return ImGuiKey_Keypad5;
        case VK_NUMPAD6:  return ImGuiKey_Keypad6;
        case VK_NUMPAD7:  return ImGuiKey_Keypad7;
        case VK_NUMPAD8:  return ImGuiKey_Keypad8;
        case VK_NUMPAD9:  return ImGuiKey_Keypad9;
        case VK_DIVIDE:   return ImGuiKey_KeypadDivide;
        case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case VK_ADD:      return ImGuiKey_KeypadAdd;
        case VK_DECIMAL:  return ImGuiKey_KeypadDecimal;

        default: return ImGuiKey_None;
    }
}

// Font management functions implementation
void Hook::SetChineseSupportEnabled(bool enabled) {
    g_EnableChineseSupport = enabled;
    // Force font reload on next frame
    static ImVec2 fScreenSize = ImVec2(-1, -1);
    fScreenSize = ImVec2(-1, -1);
}

bool Hook::IsChineseSupportEnabled() {
    return g_EnableChineseSupport;
}

bool Hook::LoadCustomFont(const char* fontPath, float fontSize) {
    if (!m_bInitialized) {
        return false;
    }
    
    // 检查字体路径是否为空
    if (!fontPath || strlen(fontPath) == 0) {
        return false;
    }
    
    // 检查文件是否存在
    HANDLE hFile = CreateFileA(fontPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    // 检查文件大小，避免加载过大的字体文件（超过10MB拒绝加载）
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return false;
    }
    
    const LONGLONG MAX_FONT_SIZE = 10 * 1024 * 1024; // 10MB
    if (fileSize.QuadPart > MAX_FONT_SIZE) {
        CloseHandle(hFile);
        return false;
    }
    
    CloseHandle(hFile);
    
    // 检查字体大小参数
    if (fontSize <= 0.0f || fontSize > 100.0f) {
        return false;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    
    // 使用 try-catch 包装字体加载，防止断言失败
    ImFont* font = nullptr;
    try {
        // 清除之前的字体
        io.Fonts->Clear();
        
        // 尝试加载自定义字体
        font = io.Fonts->AddFontFromFileTTF(fontPath, fontSize, NULL, GetGlyphRanges());
        
        // 如果加载失败，添加默认字体作为备选
        if (font == nullptr) {
            font = io.Fonts->AddFontDefault();
        }
        
        // 构建字体图集
        if (!io.Fonts->Build()) {
            return false;
        }
        
    } catch (...) {
        // 如果发生任何异常，使用默认字体
        io.Fonts->Clear();
        font = io.Fonts->AddFontDefault();
        io.Fonts->Build();
    }
    
    // 验证字体是否成功加载
    if (font == nullptr) {
        return false;
    }
    
    // 设置为默认字体
    io.FontDefault = font;
    
    // 使 device objects 失效以强制重建
    try {
        if (gRenderer == eRenderer::Dx9) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
        } else if (gRenderer == eRenderer::Dx11) {
            ImGui_ImplDX11_InvalidateDeviceObjects();
        }
    } catch (...) {
        // 忽略设备对象失效时的异常
    }
    
    return true;
}