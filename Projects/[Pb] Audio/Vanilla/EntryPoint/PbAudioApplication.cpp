#include "PbAudioApplication.h"

#include "[Pb]Audio/[Pb]Audio.h"

/******************************************************************
*                                                                 *
*  DemoApp::DemoApp constructor                                   *
*                                                                 *
*  Initialize member data.                                        *
*                                                                 *
******************************************************************/

DemoApp::DemoApp() :
    m_hwnd(NULL)
{
}

/******************************************************************
*                                                                 *
*  Release resources.                                             *
*                                                                 *
******************************************************************/

DemoApp::~DemoApp()
{
    // TODO: Release app resource here.
}

/*******************************************************************
*                                                                  *
*  Create the application window and the combobox.                 *
*                                                                  *
*******************************************************************/

PBAudioDeviceList _deviceList;

WNDPROC oldButtonProc;
HINSTANCE ghInstance;
HFONT hDefaultFont;


HWND outputDeviceLabel;
HWND testButton;
HWND outputDevicePopupButton;

enum
{
    OUTPUT_DEVICE_LABEL = 10,
    TEST_BUTTON         = 20,
};


static HFONT getSystemDefaultFont(void)
{
    if (!hDefaultFont)
    {
        // get system default font
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
        hDefaultFont = CreateFontIndirect(&ncm.lfMessageFont);
    }
    return hDefaultFont;
}

void SetDefaultFont(HWND hwnd)
{
    SendMessage(hwnd, WM_SETFONT, (WPARAM)getSystemDefaultFont(), FALSE);
}

void DemoApp::CreateOutputDeviceLabel()
{
    /*
    CreateWindowEx(0,
        "STATIC",
        NULL,
        WS_CHILD | WS_VISIBLE,
        10, 10, 124, 25,
        Handle,
        NULL,
        HInstance,
        NULL);
    */

    outputDeviceLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            15, 25, 50, 50,
            m_hwnd, (HMENU)OUTPUT_DEVICE_LABEL, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(outputDeviceLabel);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc


    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(outputDeviceLabel, WM_SETFONT, (WPARAM)font, FALSE);
    
    //Set Text
    SetWindowText(outputDeviceLabel, L"Output Device:"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);
    
    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels", lstrlen(L"Active Output Channels"), &size);

    //Resize window
    SetWindowPos(outputDeviceLabel, 0, 20, 0, size.cx+2, size.cy, SWP_NOMOVE | SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(outputDeviceLabel, odlDC);
}

void DemoApp::CreateTestButton()
{
    /*
    CreateWindowEx(0,
        "STATIC",
        NULL,
        WS_CHILD | WS_VISIBLE,
        10, 10, 124, 25,
        Handle,
        NULL,
        HInstance,
        NULL);
    */

    RECT windowFrame;
    RECT labelFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(outputDeviceLabel, &labelFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG buttonWidth  = (LONG) (((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = windowWidth - buttonWidth - 26;
    LONG  y = 20;// labelFrame.top - windowFrame.top;

    testButton =
        CreateWindow(L"button", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_CENTER| SS_CENTERIMAGE,
            x, y, buttonWidth, buttonHeight,
            m_hwnd, (HMENU)TEST_BUTTON, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(testButton);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc


    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(testButton, WM_SETFONT, (WPARAM)font, FALSE);

    //Set Text
    SetWindowText(testButton, L"Test"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Test", lstrlen(L"Test"), &size);


    //Resize window
    //SetWindowPos(testButton, 0, x, y, buttonWidth, buttonHeight, SWP_NOMOVE | SWP_FRAMECHANGED);


    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(testButton, odlDC);
}



int DemoApp::UpdateOutputDeviceList()
{
    //Ask [Pb]Audio for the list of connected Audio Interface Devices
    _deviceList = PBAudioAvailableDevices();

    int outputIndex = 0;

    //TCHAR deviceName[128]; memset(&deviceName, 0, sizeof(deviceName));

    for (int i = 0; i < _deviceList.count; i++)
    {
        uint32_t nameLen = 128; char deviceName[128] = "\0"; PBAudioDeviceName(_deviceList.devices[i], deviceName, &nameLen);
        //fprintf(stdout, "AudioMidiSettings::UpdateOutputDeviceList() Endpoint: %.*s", nameLen, deviceName);

        // Add string to combobox.
        SendMessage(outputDevicePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)deviceName);

        //[self.outputPopupButton addItemWithTitle : [NSString stringWithUTF8String : deviceName] ] ;
        if (PBAudio.OutputStreams[0].audioDevice == _deviceList.devices[i]) { outputIndex = i; assert(1 == 0); }

        fprintf(stdout, "\nAudioMidiSettings::UpdateOutputDeviceList() Endpoint: %S\n", (LPWSTR)deviceName);
        //fprintf(stdout, "\nAudioMidiSettings::UpdateOutputDeviceList() %p : %p\n", PBAudio.OutputStreams[0].audioDevice, _deviceList.devices[i]);
 
        //Find selected device index
        //Can't rely on comparing ptr values, must compare device id strings
        uint32_t idLen1 = 128; char id1[128] = "\0"; PBAudioDeviceID(_deviceList.devices[i], id1, &idLen1);
        uint32_t idLen2 = 128; char id2[128] = "\0"; PBAudioDeviceID(PBAudio.OutputStreams[0].audioDevice, id2, &idLen2);
        if (memcmp(id1, id2, idLen1) == 0) { outputIndex = i; }
                   
    }

    //select the index of active audio device
    // Send the CB_SETCURSEL message to display an initial item in the selection field  
    SendMessage(outputDevicePopupButton, CB_SETCURSEL, (WPARAM)outputIndex, (LPARAM)0);

    //int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    //(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);

    return outputIndex;
}



void DemoApp::CreateOutputPopupButton()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(outputDeviceLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = labelFrame.right;
    LONG  y = 20;// labelFrame.top - windowFrame.top;

    // Create the Combobox
    //
    // Uses the CreateWindow function to create a child window of 
    // the application window. The WC_COMBOBOX window style specifies  
    // that it is a combobox.

    //int xpos    = 100;            // Horizontal position of the window.
    //int ypos    = 100;            // Vertical position of the window.
    int nwidth  = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    outputDevicePopupButton = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        x, y, buttonFrame.left - x, nheight, hwndParent, NULL, HINST_THISCOMPONENT,
        NULL);


    // load the combobox with item list.  
    // Send a CB_ADDSTRING message to load each item
    UpdateOutputDeviceList();
    
    HDC odlDC = GetDC(outputDeviceLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels", lstrlen(L"Active Output Channels"), &size);

    //Resize window
    SetWindowPos(outputDevicePopupButton, 0, 20 + size.cx + 2, y, buttonFrame.left - x - 10, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(testButton, odlDC);
}
 

HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    // Register the window class.
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DemoApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);;
    wcex.lpszMenuName = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName = TEXT("AudioMidiSettingsWindow");

    RegisterClassEx(&wcex);

    // Create the application window.
    //
    // Because the CreateWindow function takes its size in pixels, we
    // obtain the system DPI and use it to scale the window size.
    int dpiX = 0;
    int dpiY = 0;
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }

    m_hwnd = CreateWindow(
        TEXT("AudioMidiSettingsWindow"),
        TEXT("Audio MIDI Settings"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<UINT>(ceil(500   * dpiX / 96.f)),
        static_cast<UINT>(ceil(450.f * dpiY / 96.f)),
        NULL,
        NULL,
        HINST_THISCOMPONENT,
        this
    );

    hr = m_hwnd ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);
    }

    // Create teh Combobox label
    CreateOutputDeviceLabel();
    CreateTestButton();
    CreateOutputPopupButton();


    return hr;
}


/******************************************************************
*                                                                 *
*  The main window's message loop.                                *
*                                                                 *
******************************************************************/

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


/******************************************************************
*                                                                 *
*  The window's message handler.                                  *
*                                                                 *
******************************************************************/

LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pDemoApp)
        );

        result = 1;
    }
    else
    {
        DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (pDemoApp)
        {
            switch (message)
            {
            case WM_COMMAND:

                //prevent this message on static [text] controls
                if((HWND)lParam == outputDeviceLabel ) break;

                //If the user makes a selection from the list:
                //   Send CB_GETCURSEL message to get the index of the selected list item.
                //   Send CB_GETLBTEXT message to get the item.
                //   Display the item in a messagebox.
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    TCHAR  ListItem[256];
                    int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    (TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);
                    MessageBox(hwnd, (LPCWSTR)ListItem, TEXT("Item Selected"), MB_OK);
                }

                wasHandled = true;
                result = 0;
                break;

            case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            wasHandled = true;
            result = 0;
            break;

            case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            wasHandled = true;
            result = 1;
            break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}