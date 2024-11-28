#include "AudioMidiSettings.h"

//#include "[Pb]Audio/[Pb]Audio.h"

/******************************************************************
*                                                                 *
*  AudioMidiSettings Enums                                        *
*                                                                 *
*                                                                 *
*                                                                 *
******************************************************************/

enum
{
    //Labels
    OUTPUT_DEVICE_LABEL = 10,
    OUTPUT_CHANNELS_LABEL = 20,
    SAMPLE_RATE_LABEL = 30,
    BUFFER_SIZE_LABEL = 40,
    MIDI_INPUT_LABEL = 50,

    //Content
    OUTPUT_DEVICE_POPUP_BUTTON = 60,
    OUTPUT_CHANNELS_LIST_VIEW = 70,
    SAMPLE_RATE_POPUP_BUTTON = 80,
    BUFFER_SIZE_POPUP_BUTTON = 90,
    MIDI_INPUT_LIST_VIEW = 100,

    //Buttons
    TEST_BUTTON = 110,

};



/******************************************************************
*                                                                 *
*  AudioMidiSettings Globals                                      *
*                                                                 *
*                                                                 *
*                                                                 *
******************************************************************/

//WinMM Device List + Observer
PBAudioDeviceList           _deviceList;
PBADeviceNotificationClient _deviceNotificationObserver;// = { &PBADeviceNotificationClient_Vtbl, NULL };

//ASIO Driver List + Observer
PBAudioDriverList           _driverList;


struct CMClientDriver* m_midi;

//UI Resources...
HFONT hDefaultFont;

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

//Windows...
HWND _hwnd;

//Driver Mode
HWND driverModeLabel;
HWND driverModePopupButton;


//Output [Audio] Device
HWND outputDeviceLabel;
HWND testButton;
HWND outputDevicePopupButton;


//Output [Audio] Channels
HWND outputChannelsLabel;
HWND channelFilterView;

//Sample Rate
HWND sampleRateLabel;
HWND sampleRatePopupButton;

//Buffer Size
HWND bufferSizeLabel;
HWND bufferSizePopupButton;

//Midi Inputs
HWND midiInputLabel;
HWND midiInputFilterView;

//Window Frames...
RECT driverModeLabelFrame;
RECT outputDeviceLabelFrame;
RECT outputChannelsLabelFrame;
RECT channelFilterViewFrame;
RECT sampleRateLabelFrame;
RECT bufferSizeLabelFrame;
RECT midiInputLabelFrame;
RECT midiInputFilterViewFrame;


/******************************************************************
*                                                                 *
*  AudioMidiSettings statics                                       *
*                                                                 *
*  Callback methods for device-event notifications.               *
*                                                                 *
******************************************************************/

static HRESULT STDMETHODCALLTYPE AMSDeviceStateChanged(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    //fprintf(stdout, "  -->AMSDeviceStateChanged\n");

    char* pszState = "?????";

    //_PrintDeviceName(pwstrDeviceId);

    switch (dwNewState)
    {
    case DEVICE_STATE_ACTIVE:
        pszState = "ACTIVE";
        break;
    case DEVICE_STATE_DISABLED:
        pszState = "DISABLED";
        break;
    case DEVICE_STATE_NOTPRESENT:
        pszState = "NOTPRESENT";
        break;
    case DEVICE_STATE_UNPLUGGED:
        pszState = "UNPLUGGED";
        break;
    }

    fprintf(stdout, "  -->New device state is DEVICE_STATE_%s (0x%8.8x)\n", pszState, dwNewState);

    AudioMidiSettings* pAudioMidiSettings = (AudioMidiSettings*)This->context; // reinterpret_cast<AudioMidiSettings*>(static_cast<LONG_PTR>(

    //Update UI will reflect if selected device has changed
    //always update the device list due to a device state change
    pAudioMidiSettings->UpdateOutputDeviceList(); //repopulate the output device list for the newly active device

    //TO DO:  filter based on pwstrDeviceID to only update when the active device has changed
    //pAudioMidiSettings->UpdateDriverModeList();   //repopulate the driver mode list to reflect the active mode
    //pAudioMidiSettings->UpdateSampleRateList();   //repopulate the sample rate list for the newly active device
    //pAudioMidiSettings->UpdateBufferSizeList();   //repopulate the buffer size list for the newly active device
    //pAudioMidiSettings->UpdateChannelFilterView();

    return S_OK;
}


static HRESULT STDMETHODCALLTYPE AMSDeviceAdded(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId)
{
    fprintf(stdout, "  -->AMSDeviceAdded\n");

    return S_OK;
};

static HRESULT STDMETHODCALLTYPE AMSDeviceRemoved(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId)
{
    fprintf(stdout, "  -->AMSDeviceRemoved\n");

    return S_OK;
}


static HRESULT STDMETHODCALLTYPE AMSDeviceDefaultOutputChanged(PBADeviceNotificationClient* This, EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    //fprintf(stdout, "  -->AMSDeviceDefaultOutputChanged\n");

    char* pszFlow = "?????";
    char* pszRole = "?????";

    //_PrintDeviceName(pwstrDeviceId);

    switch (flow)
    {
    case eRender:
        pszFlow = "eRender";
        break;
    case eCapture:
        pszFlow = "eCapture";
        break;
    }

    switch (role)
    {
    case eConsole:
        pszRole = "eConsole";
        break;
    case eMultimedia:
        pszRole = "eMultimedia";
        break;
    case eCommunications:
        pszRole = "eCommunications";
        break;
    }

    fprintf(stdout, "  -->New default device: flow = %s, role = %s\n", pszFlow, pszRole);

    return S_OK;
}


static HRESULT STDMETHODCALLTYPE AMSDevicePropertyValueChanged(PBADeviceNotificationClient* This, LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    //fprintf(stdout, "  -->AMSDevicePropertyValueChanged\n");

    /*
    fprintf(stdout, "  -->Changed device property "
        "{%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x}#%d\n",
        key.fmtid.Data1, key.fmtid.Data2, key.fmtid.Data3,
        key.fmtid.Data4[0], key.fmtid.Data4[1],
        key.fmtid.Data4[2], key.fmtid.Data4[3],
        key.fmtid.Data4[4], key.fmtid.Data4[5],
        key.fmtid.Data4[6], key.fmtid.Data4[7],
        key.pid);
    */

    //TO DO:  I have no idea how to decipher the notification properties...
    //TO DO:  filter based on pwstrDeviceID to only update when the active device has changed
    
    /*
    long   buffSize = PBAudioDriverGetBufferSize(PBAudio.OutputStreams[0].driver, NULL, NULL, NULL, NULL);
    if (PBAudio.OutputStreams[0].bufferFrameCount != buffSize)
    {

    }
    */

    AudioMidiSettings* pAudioMidiSettings = (AudioMidiSettings*)This->context;
    pAudioMidiSettings->UpdateDriverModeList();   //repopulate the driver mode list to reflect the active mode
    pAudioMidiSettings->UpdateOutputDeviceList(); //repopulate the output device list for the newly active device
    pAudioMidiSettings->UpdateSampleRateList();   //repopulate the sample rate list for the newly active device
    pAudioMidiSettings->UpdateBufferSizeList();   //repopulate the buffer size list for the newly active device

    pAudioMidiSettings->UpdateChannelFilterView();
   
    return S_OK;
}

PBADeviceNotificationCallbacks AMSDeviceNotificationCallbacks = { AMSDeviceStateChanged, AMSDeviceAdded, AMSDeviceRemoved, AMSDeviceDefaultOutputChanged, AMSDevicePropertyValueChanged };

/******************************************************************
*                                                                 *
*  AudioMidiSettings::AudioMidiSettings constructor               *
*                                                                 *
*  Initialize member data.                                        *
*                                                                 *
******************************************************************/

AudioMidiSettings::AudioMidiSettings() :
    m_hwnd(NULL)
    //m_midi(NULL)
{
}

AudioMidiSettings::AudioMidiSettings(struct CMClientDriver* midi) :
    m_hwnd(NULL)
    //m_midi(midi)
{
    m_midi = midi;
}



/******************************************************************
*                                                                 *
*  Release resources.                                             *
*                                                                 *
******************************************************************/

AudioMidiSettings::~AudioMidiSettings()
{
    // TODO: Release app resource here.
}


/*******************************************************************
*                                                                  *
*  Create the application window and the combobox.                 *
*                                                                  *
*******************************************************************/


#include "Commctrl.h"

LRESULT CALLBACK ListViewSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    //...

    if (uMsg == WM_NOTIFY)
    {

        int idFrom = (int)wParam;
        NMHDR* pnm = (LPNMHDR)lParam;

        //assert(pnm->hwndFrom == midiInputFilterView);
        fprintf(stdout, "List View Subclass WM_NOTIFY code = %d\n", pnm->code);

        if (pnm->idFrom == LVN_ITEMCHANGED) assert(1 == 0);
        if (pnm->idFrom == LVN_ITEMCHANGING) assert(1 == 0);

        switch (pnm->code)
        {

        case LVM_SETITEMSTATE:
            assert(1 == 0);

        case LVN_ITEMCHANGED:
            assert(1 == 0);

        }
    }
    else if (uMsg == LVN_ITEMCHANGED)
        fprintf(stdout, "List View Subclass uMsg = %d\n", uMsg);


    if (uMsg == WM_NCDESTROY)
    {
        // NOTE: this requirement is NOT stated in the documentation,
        // but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, ListViewSubClass, uIdSubclass);
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void AudioMidiSettings::CreateDriverModeLabel()
{
    driverModeLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            15, 25, 50, 50,
            m_hwnd, (HMENU)OUTPUT_DEVICE_LABEL, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(driverModeLabel);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc

    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(driverModeLabel, WM_SETFONT, (WPARAM)font, FALSE);

    //Set Text
    SetWindowText(driverModeLabel, L"Driver Mode:"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    //Resize window
    SetWindowPos(driverModeLabel, 0, 20, 20, size.cx + 2, size.cy, SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(driverModeLabel, odlDC);

    driverModeLabelFrame.left = 20; driverModeLabelFrame.right = driverModeLabelFrame.left + size.cx + 2;
    driverModeLabelFrame.top = 0;  driverModeLabelFrame.bottom = driverModeLabelFrame.top + size.cy;

}


int AudioMidiSettings::UpdateDriverModeList()
{
    //Ask [Pb]Audio for the list of connected Audio Interface Devices
    //_deviceList = PBAudioAvailableDevices();

    wchar_t* driverModeStrings[PBA_NUM_DRIVER_MODES] = { L"WASAPI Shared", /*L"WASAPI Exclusive",*/ L"Vendor (ASIO)"};// , L"Vendor"

    int selectedIndex = 0;

    //TCHAR deviceName[128]; memset(&deviceName, 0, sizeof(deviceName));

    //Remove all items from the combox box
    SendMessage(driverModePopupButton, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

    for (int i = 0; i < PBA_NUM_DRIVER_MODES; i++)
    {
        //uint32_t nameLen = 128; char deviceName[128] = "\0"; PBAudioDeviceName(_deviceList.devices[i], deviceName, &nameLen);

        // Add string to combobox.
        SendMessage(driverModePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)driverModeStrings[i]);

        //[self.outputPopupButton addItemWithTitle : [NSString stringWithUTF8String : deviceName] ] ;
        if (PBAudio.OutputStreams[0].shareMode == i) { selectedIndex = i; }

    }

    //select the index of active audio device
    // Send the CB_SETCURSEL message to display an initial item in the selection field  
    SendMessage(driverModePopupButton, CB_SETCURSEL, (WPARAM)selectedIndex, (LPARAM)0);

    //int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    //(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);

    return selectedIndex;
}

void AudioMidiSettings::CreateDriverModePopupButton()
{
    RECT windowFrame;
    RECT labelFrame;
    //RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(driverModeLabel, &labelFrame);
    //GetWindowRect(testButton, &buttonFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = labelFrame.right;
    LONG  y = 20;// labelFrame.top - windowFrame.top;

    LONG  buttonRight = windowWidth - buttonWidth;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    driverModePopupButton = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        x, y, buttonRight - x, nheight,
        hwndParent, (HMENU)OUTPUT_DEVICE_POPUP_BUTTON, HINST_THISCOMPONENT, NULL);

    // load the combobox with item list.  
    // Send a CB_ADDSTRING message to load each item
    UpdateDriverModeList();

    HDC odlDC = GetDC(driverModeLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    LONG lPadding = 6;
    //Resize window
    x = 20 + size.cx + 2 + lPadding;
    SetWindowPos(driverModePopupButton, 0, x, y, buttonRight - x /* - 10 - lPadding */, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(driverModeLabel, odlDC);
}


void AudioMidiSettings::CreateOutputDeviceLabel()
{

    RECT windowFrame;
    RECT labelFrame;
    //RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(driverModeLabel, &labelFrame);
    //GetWindowRect(testButton, &buttonFrame);


    LONG  labelWidth = labelFrame.right - labelFrame.left;
    LONG  labelHeight = labelFrame.bottom - labelFrame.top;
    LONG  x = driverModeLabelFrame.left;// labelFrame.left;
    LONG  y = labelFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    outputDeviceLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            x, 20 + labelHeight, labelWidth, labelHeight,
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
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    //Resize window
    //SetWindowPos(outputDeviceLabel, 0, 20, 20, size.cx+2, size.cy, SWP_FRAMECHANGED);
    SetWindowPos(outputDeviceLabel, 0, x, 20 + labelHeight + 20, size.cx + 2, size.cy, SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(outputDeviceLabel, odlDC);

    outputDeviceLabelFrame.left = x;                     outputDeviceLabelFrame.right = outputDeviceLabelFrame.left + size.cx + 2;
    outputDeviceLabelFrame.top = 20 + labelHeight + 20; outputDeviceLabelFrame.bottom = outputDeviceLabelFrame.top + size.cy;

}

void AudioMidiSettings::CreateTestButton()
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

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = windowWidth - buttonWidth - 26;
    LONG  y = outputDeviceLabelFrame.top;// labelFrame.top - windowFrame.top;

    LONG buttonRight = windowWidth - buttonWidth;
    x = buttonRight - 40;

    testButton =
        CreateWindow(L"button", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_CENTER | SS_CENTERIMAGE,
            x, y, buttonRight - x, buttonHeight,
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



int AudioMidiSettings::UpdateOutputDeviceList()
{
    int outputIndex = 0;

    //WinMM Device List
    if (PBAudio.OutputStreams[0].shareMode < PBA_DRIVER_VENDOR)
    {
        //uint32_t idLen2 = 128; char id2[128] = "\0";
        //if (PBAudio.OutputStreams[0].audioDevice) PBAudioDeviceID(PBAudio.OutputStreams[0].audioDevice, id2, &idLen2);

        //Ask [Pb]Audio for the list of connected Audio Interface Devices
        //Update: PBAudioAvailableDevices now processes the list to ensure that any exiisting AudioDevice pointers are not overwritten
        _deviceList = PBAudioAvailableDevices(kAudioObjectPropertyScopeOutput);

        //Remove all items from the combox box
        SendMessage(outputDevicePopupButton, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

        for (int i = 0; i < _deviceList.count; i++)
        {
            uint32_t nameLen = 128; char deviceName[128] = "\0"; PBAudioDeviceName(_deviceList.devices[i], deviceName, &nameLen);

            // Add string to combobox.
            SendMessage(outputDevicePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)deviceName);

            //API Note: IMMDevice names are UNICODE, but ASIO driver names are UTF8
            fprintf(stdout, "\nAudioMidiSettings::UpdateOutputDeviceList() Endpoint: %S\n", (LPWSTR)deviceName);
            //fprintf(stdout, "\nAudioMidiSettings::UpdateOutputDeviceList() %p : %p\n", PBAudio.OutputStreams[0].audioDevice, _deviceList.devices[i]);

            //Find selected device index
            //Can't rely on comparing ptr values, must compare device id strings
            if (PBAudio.OutputStreams[0].audioDevice)
            {
                uint32_t idLen1 = 128; char id1[128] = "\0"; PBAudioDeviceID(_deviceList.devices[i], id1, &idLen1);
                uint32_t idLen2 = 128; char id2[128] = "\0"; PBAudioDeviceID(PBAudio.OutputStreams[0].audioDevice, id2, &idLen2);
                if (memcmp(id1, id2, idLen1) == 0) { outputIndex = i; }
            }
        }
    }
    //ASIO Driver List
    else 
    {
        //_driverList = PBAudioAvailableDrivers(kAudioObjectPropertyScopeOutput);

        //Remove all items from the combox box
        SendMessage(outputDevicePopupButton, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

        //char dname[64];
        char curName[64] = "\0";
        uint32_t curNameLen = 0;

        //get the stream's active [vendor] driver name
        if (PBAudio.OutputStreams[0].driverID > -1) PBAudioGetDriverName(PBAudio.OutputStreams[0].driverID, curName, 32);
        outputIndex = PBAudio.OutputStreams[0].driver ? PBAudio.OutputStreams[0].driverID : PBAudio.OutputStreams[0].driverID + 1;

        // Add string to combobox.

        if(!PBAudio.OutputStreams[0].driver) SendMessage(outputDevicePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"No Driver Selected");

        for (long i = 0; i < PBAudioAvailableDriverCount(); i++)
        {
            uint32_t nameLen = 32; char deviceName[64] = "\0"; PBAudioGetDriverName(i, deviceName, nameLen);

            //Convert driver name from UTF8 to UNICODE
            wchar_t  wDeviceName[128];
            swprintf(wDeviceName, 128, L"%hs", deviceName);

            //API Note: IMMDevice names are UNICODE, but ASIO driver names are UTF8
            //fprintf(stdout, "\nAudioMidiSettings::UpdateVendorDriverList() Endpoint: %s\n", deviceName);
            //fprintf(stdout, "\nAudioMidiSettings::UpdateOutputDriverList() Endpoint: %S\n", (LPWSTR)wDeviceName);

            // Add string to combobox.
            SendMessage(outputDevicePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)wDeviceName);

            //Find selected device index
            //Can't rely on comparing ptr values, must compare device id strings (ASIO has no device ids?)
            //if( curNameLen > 0 && memcmp(curName, deviceName, MIN(curNameLen, strlen(deviceName))) == 0) { outputIndex = i + 1; }
        }

    }

    //select the index of active audio device
    // Send the CB_SETCURSEL message to display an initial item in the selection field  
    SendMessage(outputDevicePopupButton, CB_SETCURSEL, (WPARAM)outputIndex, (LPARAM)0);

    //int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    //(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);

    return outputIndex;
}



void AudioMidiSettings::CreateOutputPopupButton()
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
    LONG  y = outputDeviceLabelFrame.top;// 20;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    outputDevicePopupButton = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        x, y, buttonFrame.left - x, nheight,
        hwndParent, (HMENU)OUTPUT_DEVICE_POPUP_BUTTON, HINST_THISCOMPONENT, NULL);

    // load the combobox with item list.  
    // Send a CB_ADDSTRING message to load each item
    UpdateOutputDeviceList();

    HDC odlDC = GetDC(outputDeviceLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    LONG lPadding = 6;
    //Resize window
    SetWindowPos(outputDevicePopupButton, 0, 20 + size.cx + 2 + lPadding, y, buttonFrame.left - x - 10 - lPadding, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(outputDeviceLabel, odlDC);
}

void AudioMidiSettings::CreateOutputChannelsLabel()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(outputDeviceLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    LONG  labelWidth = labelFrame.right - labelFrame.left;
    LONG  labelHeight = labelFrame.bottom - labelFrame.top;
    LONG  x = outputDeviceLabelFrame.left;// labelFrame.left;
    LONG  y = outputDeviceLabelFrame.bottom + 20;// labelFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    outputChannelsLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            x, y, labelWidth, labelHeight,
            m_hwnd, (HMENU)OUTPUT_CHANNELS_LABEL, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(outputChannelsLabel);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc

    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(outputChannelsLabel, WM_SETFONT, (WPARAM)font, FALSE);

    //Set Text
    SetWindowText(outputChannelsLabel, L"Active Output Channels:"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    //Resize window
    SetWindowPos(outputChannelsLabel, 0, x, y, size.cx + 2, size.cy, SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(outputChannelsLabel, odlDC);


    outputChannelsLabelFrame.left = x; outputChannelsLabelFrame.right = outputChannelsLabelFrame.left + size.cx + 2;
    outputChannelsLabelFrame.top = y; outputChannelsLabelFrame.bottom = outputChannelsLabelFrame.top + size.cy;

}

int CreateTextColumn(HWND hwndLV, int iCol, wchar_t* text, int iWidth)
{
    LVCOLUMN lvc;

    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = iWidth;
    lvc.pszText = text;
    lvc.iSubItem = iCol;
    return ListView_InsertColumn(hwndLV, iCol, &lvc);
}

int CreateCheckboxColumn(HWND hwndLV, int iCol, wchar_t* text, int iWidth)
{
    LVCOLUMN lvc;

    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = iWidth;
    lvc.pszText = text;
    lvc.iSubItem = iCol;
    return ListView_InsertColumn(hwndLV, iCol, &lvc);
}


int CreateColItem(HWND hwndList, wchar_t* Text1, bool checkstate)
{
    LVITEMW lvi = { 0 };
    int Ret;
    // Initialize LVITEMW members that are common to all items.
    lvi.mask = LVIF_TEXT | LVIF_STATE;
    lvi.pszText = Text1;
    Ret = ListView_InsertItem(hwndList, &lvi);
    if (Ret >= 0)
    {
        ListView_SetCheckState(hwndList, 0, checkstate);
    }
    return Ret;
}

int CreateItem(HWND hwndList, wchar_t* text)
{
    LVITEMW lvi = { 0 };
    lvi.mask = LVIF_TEXT;
    lvi.pszText = text;
    return ListView_InsertItem(hwndList, &lvi);
}

void AudioMidiSettings::UpdateChannelFilterView()
{
    ListView_DeleteAllItems(channelFilterView);

    int nMonoChannels = PBAudio.OutputStreams[0].nOutputChannels;

    for (int i = nMonoChannels - 2; i >= 0; i-=2)
    {
        uint32_t nameLen = 128; wchar_t channelPairName[128] = L"\0"; wsprintf(channelPairName, L"Output %d + %d", i+1, i+2);
        CreateColItem(channelFilterView, channelPairName, true);
    }

}

void AudioMidiSettings::CreateOutputChannelFilterView()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(outputChannelsLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = labelFrame.right;
    LONG  y = outputChannelsLabelFrame.top;// 20 + buttonHeight + 20;// labelFrame.top - windowFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 100;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    channelFilterView = CreateWindow(L"SysListView32", L"",
        WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT,
        x, y, buttonFrame.right - x, nheight,
        hwndParent, (HMENU)OUTPUT_CHANNELS_LIST_VIEW, NULL, NULL);
    ListView_SetExtendedListViewStyle(channelFilterView, LVS_EX_CHECKBOXES);


    HDC odlDC = GetDC(outputChannelsLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    LONG lPadding = 6;


    int iColSize1 = nwidth / 2; int iColSize2 = nwidth / 2;
    wchar_t* ptrColHeaderString1 = L"Channels";
    //wchar_t* ptrColHeaderString2 = L"Enabled";


    CreateTextColumn(channelFilterView, 0, ptrColHeaderString1, buttonFrame.right - x - 10);
    //CreateCheckboxColumn(channelFilterView, 1, ptrColHeaderString2, iColSize2);

    UpdateChannelFilterView();

    //Resize window
    SetWindowPos(channelFilterView, 0, 20 + size.cx + 2 + lPadding, y, buttonFrame.right - x - 10, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(outputChannelsLabel, odlDC);

    channelFilterViewFrame.left = 20 + size.cx + 2; channelFilterViewFrame.right = channelFilterViewFrame.left + buttonFrame.right - x - 10;
    channelFilterViewFrame.top = y;                channelFilterViewFrame.bottom = channelFilterViewFrame.top + nheight;
}


void AudioMidiSettings::CreateSampleRateLabel()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(outputChannelsLabel, &labelFrame);
    //GetWindowRect(testButton, &buttonFrame);

    LONG  labelWidth = labelFrame.right - labelFrame.left;
    LONG  labelHeight = labelFrame.bottom - labelFrame.top;
    LONG  x = outputDeviceLabelFrame.left;// labelFrame.left;
    LONG  y = channelFilterViewFrame.bottom;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    sampleRateLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            x, y + 20, labelWidth, labelHeight,
            m_hwnd, (HMENU)SAMPLE_RATE_LABEL, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(sampleRateLabel);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc

    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(sampleRateLabel, WM_SETFONT, (WPARAM)font, FALSE);

    //Set Text
    SetWindowText(sampleRateLabel, L"Sample Rate:"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    //Resize window
    SetWindowPos(sampleRateLabel, 0, x, y + 20, size.cx + 2, size.cy, SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(sampleRateLabel, odlDC);

    sampleRateLabelFrame.left = 20;     sampleRateLabelFrame.right = sampleRateLabelFrame.left + size.cx + 2;
    sampleRateLabelFrame.top = y + 20; sampleRateLabelFrame.bottom = sampleRateLabelFrame.top + size.cy;

}



int AudioMidiSettings::UpdateSampleRateList()
{
    //Ask [Pb]Audio for the list of connected Audio Interface Devices
    //_deviceList = PBAudioAvailableDevices();

    int outputIndex = 0;

    //Remove all items from the combox box
    SendMessage(sampleRatePopupButton, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

    if (PBAudio.OutputStreams[0].shareMode < PBA_DRIVER_VENDOR)
    {
        uint32_t nameLen = 128; wchar_t sampleRate[128] = L"\0"; wsprintf(sampleRate, L"%d Hz", PBAudio.OutputStreams[0].format.nSamplesPerSec);
        //fprintf(stdout, "AudioMidiSettings::UpdateOutputDeviceList() Endpoint: %.*s", nameLen, deviceName);

        // Add string to combobox.
        SendMessage(sampleRatePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)sampleRate);
    }
    else if( PBAudio.OutputStreams[0].driver)
    {
        double sampleRates[] = { 44100., 48000., 88200., 96000., 176400., 192000., 384000., 768000. };
        double currentRate = PBAudio.OutputStreams[0].sampleRate;// PBAudioDriverGetSampleRate(PBAudio.OutputStreams[0].driver);
        int nRates = 0;

        for (int i = 0; i < sizeof(sampleRates) / sizeof(double); i++)
        {
            double sampRate = sampleRates[i];

            if (PBAudioDriverCanSampleRate(PBAudio.OutputStreams[0].driver, sampRate))
            {
                uint32_t nameLen = 128; wchar_t sampleRate[128] = L"\0"; wsprintf(sampleRate, L"%d Hz", (DWORD)sampRate);

                // Add string to combobox.
                SendMessage(sampleRatePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)sampleRate);

                if (memcmp(&sampRate, &currentRate, sizeof(double)) == 0) outputIndex = nRates;
                    
                nRates++;
            }
        }

    }

    //select the index of active audio device
    // Send the CB_SETCURSEL message to display an initial item in the selection field  
    SendMessage(sampleRatePopupButton, CB_SETCURSEL, (WPARAM)outputIndex, (LPARAM)0);

    //int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    //(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);

    return outputIndex;
}


void AudioMidiSettings::CreateSampleRatePopupButton()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(sampleRateLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = labelFrame.right;
    LONG  y = sampleRateLabelFrame.top;// labelFrame.top - windowFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    sampleRatePopupButton = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        x, y, buttonFrame.right - x, nheight,
        hwndParent, (HMENU)SAMPLE_RATE_POPUP_BUTTON, HINST_THISCOMPONENT, NULL);


    // load the combobox with item list.  
    // Send a CB_ADDSTRING message to load each item
    UpdateSampleRateList();

    HDC odlDC = GetDC(sampleRateLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    LONG lPadding = 6;

    //Resize window
    SetWindowPos(sampleRatePopupButton, 0, 20 + size.cx + 2 + lPadding, y, buttonFrame.right - x - 10, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(sampleRateLabel, odlDC);

}

void AudioMidiSettings::CreateBufferSizeLabel()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(sampleRateLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    LONG  labelWidth = labelFrame.right - labelFrame.left;
    LONG  labelHeight = labelFrame.bottom - labelFrame.top;
    LONG  x = sampleRateLabelFrame.left;// labelFrame.left;
    LONG  y = sampleRateLabelFrame.bottom;// labelFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    bufferSizeLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            x, 20 + y, labelWidth, labelHeight,
            m_hwnd, (HMENU)BUFFER_SIZE_LABEL, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(bufferSizeLabel);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc

    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(bufferSizeLabel, WM_SETFONT, (WPARAM)font, FALSE);

    //Set Text
    SetWindowText(bufferSizeLabel, L"Buffer Size:"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    //Resize window
    SetWindowPos(bufferSizeLabel, 0, x, 20 + y, size.cx + 2, size.cy, SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(bufferSizeLabel, odlDC);


    bufferSizeLabelFrame.left = 20;     bufferSizeLabelFrame.right = bufferSizeLabelFrame.left + size.cx + 2;
    bufferSizeLabelFrame.top = 20 + y; bufferSizeLabelFrame.bottom = bufferSizeLabelFrame.top + size.cy;

}


int AudioMidiSettings::UpdateBufferSizeList()
{
    //Ask [Pb]Audio for the list of connected Audio Interface Devices
    //_deviceList = PBAudioAvailableDevices();

    int outputIndex = 0;

    //Remove all items from the combox box
    SendMessage(bufferSizePopupButton, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

    if (PBAudio.OutputStreams[0].shareMode < PBA_DRIVER_VENDOR)
    {
        double duration = (double)PBAudio.OutputStreams[0].bufferFrameCount / (double)PBAudio.OutputStreams[0].format.nSamplesPerSec * 1000.;
        uint32_t nameLen = 128; wchar_t sampleRate[128] = L"\0"; swprintf(sampleRate, L"%d samples (%.1f ms)", PBAudio.OutputStreams[0].bufferFrameCount, (float)duration);
        //fprintf(stdout, "AudioMidiSettings::UpdateOutputDeviceList() Endpoint: %.*s", nameLen, deviceName);

        // Add string to combobox.
        SendMessage(bufferSizePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)sampleRate);
    }
    else if( PBAudio.OutputStreams[0].driver )
    {

        long _minBufferSizeStep;
        long _minBufferSize, _maxBufferSize;
        long     bufferSize, granularity;

        //TO DO:  These IASIO calls fail when called directly from release mode (but not in debug) so the stream properties are observed instead
        double sampRate = PBAudio.OutputStreams[0].sampleRate;       // PBAudioDriverGetSampleRate(PBAudio.OutputStreams[0].driver);
        long   buffSize = PBAudio.OutputStreams[0].bufferFrameCount; // PBAudioDriverGetBufferSize(PBAudio.OutputStreams[0].driver, &_minBufferSize, &_maxBufferSize, &bufferSize, &granularity);
        
        double duration = ((double)buffSize) / sampRate * 1000.;
        uint32_t nameLen = 128; wchar_t bufferSizeStr[128] = L"\0"; swprintf(bufferSizeStr, L"%d samples (%.1f ms)", (int)buffSize, (float)duration);

        // Add string to combobox.
        SendMessage(bufferSizePopupButton, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)bufferSizeStr);

        //API Note: 
        // WASAPI device sample rate/buffer size *MUST* be set through system panel
        // and IASIO::getBufferSize returns _minBufferSize == _maxBufferSize, 
        // So there is no reason to enumerate/allow selection of buffer size through the app
        // Just open the Vendor's ASIO control panel to change buffer size instead.        
    }

    //select the index of active audio device
    // Send the CB_SETCURSEL message to display an initial item in the selection field  
    SendMessage(bufferSizePopupButton, CB_SETCURSEL, (WPARAM)outputIndex, (LPARAM)0);

    //int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    //(TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);

    return outputIndex;
}


void AudioMidiSettings::CreateBufferSizePopupButton()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(bufferSizeLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = labelFrame.right;
    LONG  y = bufferSizeLabelFrame.top;// labelFrame.top - windowFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    bufferSizePopupButton = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        x, y, buttonFrame.right - x, nheight,
        hwndParent, (HMENU)BUFFER_SIZE_POPUP_BUTTON, HINST_THISCOMPONENT, NULL);


    // load the combobox with item list.  
    // Send a CB_ADDSTRING message to load each item
    UpdateBufferSizeList();

    HDC odlDC = GetDC(bufferSizeLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    LONG lPadding = 6;

    //Resize window
    SetWindowPos(bufferSizePopupButton, 0, 20 + size.cx + 2 + lPadding, y, buttonFrame.right - x - 10, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(bufferSizeLabel, odlDC);
}


void AudioMidiSettings::CreateMidiInputLabel()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(bufferSizeLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    //float windowWidth = (float)(windowFrame.right - windowFrame.left);
    //float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  labelWidth = labelFrame.right - labelFrame.left;
    LONG  labelHeight = labelFrame.bottom - labelFrame.top;
    LONG  x = bufferSizeLabelFrame.left;// labelFrame.left;
    LONG  y = bufferSizeLabelFrame.bottom;// labelFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 200;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    midiInputLabel =
        CreateWindow(L"static", L"+",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP |
            SS_NOTIFY | SS_RIGHT | SS_CENTERIMAGE,
            x, 20 + y, labelWidth, labelHeight,
            m_hwnd, (HMENU)MIDI_INPUT_LABEL, NULL, NULL);


    //Get DC
    HDC odlDC = GetDC(midiInputLabel);

    //Use GetWinDowText to get the text of static window
    //Use GetDC to get the dc for the window
    //Use WM_GETFONT to get the font for the windowand select the font into the dc
    //Use one of the text size calculation function to calculate the text size
    //Restore the the original dc font
    //Release dc

    //Set Default Font

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SendMessage(midiInputLabel, WM_SETFONT, (WPARAM)font, FALSE);

    //Set Text
    SetWindowText(midiInputLabel, L"Active MIDI Inputs:"); //SendMessage(outputDeviceLabel, WM_SETTEXT, (WPARAM)getSystemDefaultFont(), FALSE);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    //Resize window
    SetWindowPos(midiInputLabel, 0, x, 20 + y, size.cx + 2, size.cy, SWP_FRAMECHANGED);

    //Set Window Proc
    //oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

    //Release DC
    ReleaseDC(midiInputLabel, odlDC);

    midiInputLabelFrame.left = 20;     midiInputLabelFrame.right = midiInputLabelFrame.left + size.cx + 2;
    midiInputLabelFrame.top = 20 + y; midiInputLabelFrame.bottom = midiInputLabelFrame.top + size.cy;
}


void AudioMidiSettings::CreateMidiInputFilterView()
{
    RECT windowFrame;
    RECT labelFrame;
    RECT buttonFrame;

    GetWindowRect(m_hwnd, &windowFrame);
    GetWindowRect(midiInputLabel, &labelFrame);
    GetWindowRect(testButton, &buttonFrame);

    float windowWidth = (float)(windowFrame.right - windowFrame.left);
    float labelHeight = (float)(labelFrame.bottom - labelFrame.top);

    LONG  buttonWidth = (LONG)(((float)windowWidth) / 12.f);
    LONG  buttonHeight = labelHeight;
    LONG  x = labelFrame.right;
    LONG  y = midiInputLabelFrame.top;

    int nwidth = 200;         // Width of the window
    int nheight = 100;         // Height of the window
    HWND hwndParent = m_hwnd;  // Handle to the parent window

    midiInputFilterView = CreateWindow(L"SysListView32", L"",
        WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT,
        x, y, buttonFrame.right - x, nheight,
        hwndParent, (HMENU)MIDI_INPUT_LIST_VIEW, NULL, NULL);
    ListView_SetExtendedListViewStyle(midiInputFilterView, LVS_EX_CHECKBOXES);


    HDC odlDC = GetDC(midiInputLabel);

    HFONT font = getSystemDefaultFont();
    HFONT dcFont = (HFONT)SelectObject(odlDC, (HGDIOBJ)font); //SelectFont(odlDC, font);

    SIZE size = { 0,0 };
    GetTextExtentPoint32(odlDC, L"Active Output Channels:", lstrlen(L"Active Output Channels:"), &size);

    LONG lPadding = 6;



    int iColSize1 = nwidth / 2; int iColSize2 = nwidth / 2;
    wchar_t* ptrColHeaderString1 = L"Device";
    //wchar_t* ptrColHeaderString2 = L"Enabled";

    //Create Columns
    CreateTextColumn(midiInputFilterView, 0, ptrColHeaderString1, buttonFrame.right - x - 10);
    //CreateTextColumn(midiInputFilterView, 1, ptrColHeaderString2, iColSize2);

    int numSources = 0;
    //If CMClientDriver was passed to constructor from the same process
    if (m_midi) numSources = m_midi->UpdateInputDevices();
    else
    {
        //TO DO:  Remotely message CMidi or user an internal CMClientDriver to maintain the list

    }

    //Create Rows for each midi input device

    //NSMutableArray* inputDeviceNames = [[NSMutableArray alloc]initWithCapacity:CMClient.numSources];
    //NSDictionary* sourceIDMap = [[NSMutableDictionary alloc]initWithCapacity:CMClient.numSources];

    //insert in reverse order because columns are inserted at index 0
    int srcIndex = 0;
    for (int i = numSources-1; i >=0; i--)
    {
        const CMSource* source = m_midi->Source(i);
        CreateColItem(midiInputFilterView, (wchar_t*)source->name, false);

    }


    //self.midiInputFilterView = [[NSCheckboxView alloc]initWithIdentifiers:inputDeviceNames Title : nil Justification : NSTextAlignmentVertical];
    //self.midiInputFilterView.delegate = self;


    //Resize window
    SetWindowPos(midiInputFilterView, 0, 20 + size.cx + 2 + lPadding, y, buttonFrame.right - x - 10, nheight, SWP_FRAMECHANGED);

    //Release DC
    ReleaseDC(midiInputLabel, odlDC);


    midiInputFilterViewFrame.left = 20 + size.cx + 2; midiInputFilterViewFrame.right = midiInputFilterViewFrame.left + buttonFrame.right - x - 10;
    midiInputFilterViewFrame.top = y;                midiInputFilterViewFrame.bottom = midiInputFilterViewFrame.top + nheight;

    /*
    if (!SetWindowSubclass(midiInputFilterView, ListViewSubClass, 1, 0)) {
        fprintf(stderr, "Failed to subclass list\n"); assert(1 == 0);
        //DestroyWindow(handle);
        //return NULL;
    }
    */


    /*
    WNDPROC prevWndProc = (WNDPROC)SetWindowLongPtr(midiInputFilterView, GWLP_WNDPROC, (LONG_PTR)ListViewWndProc);
    if (!prevWndProc) {
        fprintf(stderr, "Failed to subclass list\n");
        assert(1 == 0);
        //DestroyWindow(handle);
        //return NULL;
    }
    */
}



void AudioMidiSettings::buttonToggledAtIndex(int index, HWND sender, bool state, struct CMClientDriver* midi)
{
    fprintf(stdout, "AudioMidiSettingsView::NSCheckBoxView::buttonToggledAtIndex(%d)\n", index);

    //if( self.modalViewMode == ModalView_Modify ) self.submitButton.enabled = YES;

    if (sender == channelFilterView)
    {
        //if( ((NSButton*)[self.channelFilterView.buttons objectAtIndex:index]).state == NSControlStateValueOn )
        {
            //Enable Audio Device Stream
            //PBAudio.Start(&PBAudio.OutputStreams[index]);
        }
        //else
        {
            //Disable Audio Device Stream
            //PBAudio.Stop(&PBAudio.OutputStreams[index]);
        }
    }
    else if (sender == midiInputFilterView)
    {
        bool enabled = state;// ((NSButton*)[self.midiInputFilterView.buttons objectAtIndex : index]).state;

        //Enable\Disable Midi Input Connection
        //fprintf("%s Toggled: %d", ((NSButton*)[self.midiInputFilterView.buttons objectAtIndex:index]).title, enabled);

        //Populate thruParam Inputs/Outputs
        //MIDIThruConnectionParams* thruParams = &(CMClient.thruConnections[CMClient.numThruConnections].params);
        //CMInitThruParams(thruParams, inputIDs, 1, outputIDs, 1);

        //struct CMDeviceDescription * deviceDescription = &(CMClient.hardwareDevices[CMClient.numHardwareDevices]);
        //CMDeviceType deviceType = surfaceType[0];

        //Use the CoreMIDI unique id of the input source endpoint for our DOM primary key
        //NSString * sourceKey = [NSString stringWithFormat:@"%d", CMClient.sources[index].uniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'

        //CMSource* source = this->m_midi->Source(index);

        //Send message to enable/disable a midi input connection
        CMUniversalMessage* message = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;
        message->type = CMMessageTypeSystem;
        message->group = pba_midi_input_connection;
        message->system.status = enabled ? CMStatusStart : CMStatusStop;
        message->system.uniqueID = midi->Source(index)->uniqueID; //one would typically use a unique id here but in WMS that is a WinRT hstring so we'll just use the index :(

        //uint32_t messageType = state ? pba_midi_create_input_connection : pba_midi_remove_input_connection;

        /*
        //TO DO:  introduce some error checking to this function pipeline
        CMInputConnection * inputConnection = [CMInputConnection createInputConnection:sourceKey];// Params:thruParams];
        //CMHardwareDevice* device = [CMHardwareDevice createHardwareDevice:sourceKey Type:deviceType];
        //assert(device);
        assert(inputConnection);
        assert(inputConnection.connection);
        //[device setConnection:thruConnection.connection];
        */

        //struct kevent kev;
        //EV_SET(&kev, message->type, EVFILT_USER, 0, NOTE_TRIGGER, 0, message);
        //kevent(PBAudio.eventQueue.kq, &kev, 1, NULL, 0, NULL);

        //TO DO: This debug statement causes crash sometimes...
        //fprintf(stdout, "buttonToggledAtIndex(%d)::uniqueID = \n\n%S\n\n", index, (wchar_t*)midi->Source(index)->uniqueID);

        //Send the message to the PbAudio Event Queue Thread
        PostThreadMessage((DWORD)PBAudio.eventQueue.kq, PBA_EVENT_UMP_CONTROL, message->type, (LPARAM)message);

    }
}


/*
void AudioMidiSettings::devicePopupButtonClicked(HWND sender)
{
    if (sender == self.outputPopupButton)
    {
        int selectedIndex = (int)self.outputPopupButton.indexOfSelectedItem;
        PBAudio.SetOutputDevice(&PBAudio.OutputStreams[0], _deviceList.devices[selectedIndex]);
        [self createChannelFilterView] ; //repopulate the output channel list for the newly active device
        [self updateSampleRateList] ;    //repopulate the sample rate list for the newly active device
        [self updateBufferSizeList] ;    //repopulate the buffer size list for the newly active device

    }
}
*/

void AudioMidiSettings::comboBoxClicked(HWND sender)
{
    PBAStreamContext* stream = &PBAudio.OutputStreams[0];
    
    /*
    if (sender == sampleRatePopupButton)
    {
        if (stream->shareMode == PBA_DRIVER_VENDOR && stream->driver) PBAudioDriverControlPanel(stream->driver);
    }
    else */if (sender == bufferSizePopupButton)
    {
        if (stream->shareMode == PBA_DRIVER_VENDOR && stream->driver) PBAudioDriverControlPanel(stream->driver);
    }
}

void AudioMidiSettings::comboBoxSelectedAtIndex(HWND sender, int selectedIndex, TCHAR* name)
{
    //Output Device Popup Button Clicked
    if (sender == outputDevicePopupButton)
    {
        if (PBAudio.OutputStreams[0].shareMode < PBA_DRIVER_VENDOR)
        {
            //PBAudio.SetOutputDevice(&PBAudio.OutputStreams[0], _deviceList.devices[selectedIndex]);
            //UpdateSampleRateList();    //repopulate the sample rate list for the newly active device
            //UpdateBufferSizeList();    //repopulate the buffer size list for the newly active device

            CMUniversalMessage* message = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;

            message->type = CMMessageTypeData128;
            //message->group = TestOutputPassID;

            message->data128.status = CMSysExStatusComplete;
            message->data128.sysex8.streamID = 0;                             //additional stream id property

            //SYSEX: [F7, manufacturer id, channel id, device id, command id, param id, param value, F7]
            //message->data128.sysex8.data[0]  = 0x42;                         //eg Korg
            message->data128.sysex8.data[1] = 0;                               //midi 1.0 sysex stream id
            message->data128.sysex8.data[2] = selectedIndex;                   //midi 1.0 systex audio device [list] id
            message->data128.sysex8.data[3] = pba_stream_change_output_device; //midi 1.0 sysex command id
            message->data128.sysex8.data[4] = 0;                               //midi 1.0 sysex param id
            message->data128.sysex8.data[5] = 0;                               //midi 1.0 sysex param value

            PostThreadMessage((DWORD)PBAudio.eventQueue.kq, PBA_EVENT_UMP_CONTROL, message->type, (LPARAM)message);
        }
        else
        {
            //translate combo box list index to a driver list index
            int8_t driverID = PBAudio.OutputStreams[0].driver ? selectedIndex : (selectedIndex - 1);
            if(PBAudio.OutputStreams[0].driverID == driverID) return; //don't issue message if this is already the active driver

            //API Note: Because IASIO COM driver does not support proxy for marshalling in MULTI_THREADED apartments
            //SetOutputDriver *MUST* be called on a thread intialized with Coinitialize(0) (SINGLE_THREADED)
             
            //PBAudioStreamSetOutputDriver(&PBAudio.OutputStreams[0], selectedIndex - 1);
            //PBAudio.SetOutputDriver(&PBAudio.OutputStreams[0], selectedIndex-1)
            //UpdateSampleRateList();    //repopulate the sample rate list for the newly active device
            //UpdateBufferSizeList();    //repopulate the buffer size list for the newly active device

            CMUniversalMessage* message = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;

            message->type = CMMessageTypeData128;
            //message->group = TestOutputPassID;

            message->data128.status = CMSysExStatusComplete;
            message->data128.sysex8.streamID = 0;                              //additional stream id property

            //SYSEX: [F7, manufacturer id, channel id, device id, command id, param id, param value, F7]
            //message->data128.sysex8.data[0]  = 0x42;                         //eg Korg
            message->data128.sysex8.data[1] = 0;                               //midi 1.0 sysex stream id
            message->data128.sysex8.data[2] = driverID;               //midi 1.0 sysex audio driver [list] id
            message->data128.sysex8.data[3] = pba_stream_change_output_driver; //midi 1.0 sysex command id
            message->data128.sysex8.data[4] = 0;               //midi 1.0 sysex param id
            message->data128.sysex8.data[5] = 0;                               //midi 1.0 sysex param value

            PostThreadMessage((DWORD)PBAudio.eventQueue.kq, PBA_EVENT_UMP_CONTROL, message->type, (LPARAM)message);
        }
    }
    else if (sender == driverModePopupButton)
    {
        //When the driver mode is changed, modify the device list but do not modify the stream until a device is selected
        //PBAudio.SetDriverMode(&PBAudio.OutputStreams[0], selectedIndex);
        PBAudio.OutputStreams[0].shareMode = (AUDCLNT_SHAREMODE)selectedIndex;
        UpdateOutputDeviceList();
    }
    else if (sender == sampleRatePopupButton)
    {
        if (PBAudio.OutputStreams[0].shareMode == PBA_DRIVER_VENDOR && PBAudio.OutputStreams[0].driver)
        {
            //use the selected index to find the sample rate
            double sampleRates[] = { 44100., 48000., 88200., 96000., 176400., 192000., 384000., 768000. };
            int nRates = 0;

            double sampRate = 0.0;// sampleRates[0];

            for (int i = 0; i < sizeof(sampleRates) / sizeof(double); i++)
            {
                sampRate = sampleRates[i];

                if (PBAudioDriverCanSampleRate(PBAudio.OutputStreams[0].driver, sampRate))
                {
                    if (selectedIndex == nRates) break;

                    nRates++;
                }
            }

            //API Note:  PBAudioDriverSetSampleRate is called through PBAudioDeviceSetSampleRate to properly stop/restart the driver
            //TO DO:  Why doesn't IASIO:set_sample_rate require being called on single threaded apartment?
            OSStatus status = PBAudioDeviceSetSampleRate(PBAudio.OutputStreams[0].audioDevice, kAudioObjectPropertyScopeOutput, sampRate);
            assert(status == 0);
        }
    }

}





HRESULT AudioMidiSettings::Initialize()
{
    HRESULT hr;

    // Register the window class.
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = AudioMidiSettings::WndProc;
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
        static_cast<UINT>(ceil(500 * dpiX / 96.f)),
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

    // Create the Output Device Windows
    CreateDriverModeLabel();
    CreateDriverModePopupButton();

    // Create the Output Device Windows
    CreateOutputDeviceLabel();
    CreateTestButton();
    CreateOutputPopupButton();

    // Ceate the Output Channel Windows
    CreateOutputChannelsLabel();
    CreateOutputChannelFilterView();

    // Ceate the Sample Rate Windows
    CreateSampleRateLabel();
    CreateSampleRatePopupButton();

    // Ceate the Buffer Size Windows
    CreateBufferSizeLabel();
    CreateBufferSizePopupButton();

    // Create the MIDI Input Windows
    CreateMidiInputLabel();
    CreateMidiInputFilterView();


    //Initialize + Register an implementation of IMMNotificationClient w/ callbacks for receiving device notifications
    _deviceNotificationObserver = PbAudioDeviceNotificationClient(&AMSDeviceNotificationCallbacks);
    PBAudioRegisterDeviceListeners(&_deviceNotificationObserver, this);

    return hr;
}


/******************************************************************
*                                                                 *
*  The main window's message loop.                                *
*                                                                 *
******************************************************************/

void AudioMidiSettings::RunMessageLoop()
{
    MSG msg;
    bool appIsRunning = true;

    //will just idle for now (in a multiwindow scenario this is best)
    //_idleEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    while (appIsRunning)
    {
        //filter for events we would like to be notified of in the message queue, much like a kqueue or [NSApplication nextEventWith:]
        //Use GetMessage to wait until the message arrives, or PeekMessage to return immediately
        if (GetMessage(&msg, NULL, 0, 0) && msg.message != WM_QUIT)  //run the event loop until it spits out error or quit
        //if(PeekMessage(&msg, NULL, CR_PLATFORM_EVENT_MSG_LOOP_QUIT, CR_PLATFORM_EVENT_MSG_LOOP_QUIT, PM_REMOVE))
        {
            //observe our custom defined quit message
            if (msg.message == PBA_EVENT_MSG_LOOP_QUIT)
            {
                //TO DO:  when/where to call WM_DESTROY in Sample Project(s) -- it won't matter in a CoreRender application


                fprintf(stdout, "\nCR_PLATFORM_EVENT_MSG_LOOP_QUIT\n");
                appIsRunning = false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            memset(&msg, 0, sizeof(MSG));
        }
    }
}


void AudioMidiSettings::testButtonClicked()
{
    fprintf(stdout, "\nAudioMidiSettings::testButtonClicked\n");

    CMUniversalMessage* message = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;

    message->type = CMMessageTypeData128;
    //message->group                 = TestOutputPassID;

    message->data128.status = CMSysExStatusComplete;
    message->data128.sysex8.streamID = 0;                                //additional stream id property

    //SYSEX: [F7, manufacturer id, channel id, device id, command id, param id, param value, F7]
    //message->data128.sysex8.data[0]  = 0x42;                           //eg Korg
    message->data128.sysex8.data[1] = 0;                              //midi 1.0 sysex stream id
    //message->data128.sysex8.data[2]  = 0;                              //midi 1.0 systex audio device id
    message->data128.sysex8.data[3] = pba_stream_change_output_pass;   //midi 1.0 sysex command id
    message->data128.sysex8.data[4] = TestOutputPassID;               //midi 1.0 sysex param id
    message->data128.sysex8.data[5] = 0;                              //midi 1.0 sysex param value

    PostThreadMessage((DWORD)PBAudio.eventQueue.kq, PBA_EVENT_UMP_CONTROL, message->type, (LPARAM)message);
}


/******************************************************************
*                                                                 *
*  The window's message handler.                                  *
*                                                                 *
******************************************************************/


LRESULT CALLBACK AudioMidiSettings::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        AudioMidiSettings* pAudioMidiSettings = (AudioMidiSettings*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pAudioMidiSettings)
        );

        _hwnd = hwnd;

        result = 1;
    }
    else
    {
        AudioMidiSettings* pAudioMidiSettings = reinterpret_cast<AudioMidiSettings*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                _hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (pAudioMidiSettings)
        {
            switch (message)
            {

                //if (hwnd == midiInputFilterView)  fprintf(stdout, "List View Wnd Proc\n");

            case WM_NOTIFY:
            {
                NMHDR* pnm = (LPNMHDR)lParam;

                //assert(pnm->hwndFrom == midiInputFilterView);
                //fprintf(stdout, "List View Subclass WM_NOTIFY code = %d\n", pnm->code);

                if (pnm->idFrom == MIDI_INPUT_LIST_VIEW)
                {
                    LPNMLISTVIEW pnmlistview = (LPNMLISTVIEW)lParam;

                    if (pnm->code == LVN_ITEMCHANGED)
                    {
                        UINT oldCheckboxState = (pnmlistview->uOldState >> 12) - 1;
                        UINT checkboxState = (pnmlistview->uNewState >> 12) - 1;

                        fprintf(stdout, "Midi Input List View LVN_ITEMCHANGED[%d] = %d\n", pnmlistview->iItem, checkboxState);

                        if (checkboxState != oldCheckboxState) //disregard clicks on the item that didn't actually toggle the checkbox
                            pAudioMidiSettings->buttonToggledAtIndex(pnmlistview->iItem, midiInputFilterView, checkboxState, m_midi);
                    }

                    //if (pnm->code == LVN_ITEMCHANGING) fprintf(stdout, "Midi Input List View LVN_ITEMCHANGING\n");
                    //if (pnm->code == LVM_SETITEMSTATE) fprintf(stdout, "Midi Input List View LVM_SETITEMSTATE\n");

                }


                wasHandled = true;
                result = 0;
                break;
            }

            case WM_COMMAND:
            {
                //prevent this message on static [text] controls
                if ((HWND)lParam == outputDeviceLabel) break;


                //If the user makes a selection from the list:
                //   Send CB_GETCURSEL message to get the index of the selected list item.
                //   Send CB_GETLBTEXT message to get the item.
                //   Display the item in a messagebox.
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    TCHAR  ListItem[256];

                    int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    (TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);
                    //MessageBox(hwnd, (LPCWSTR)ListItem, TEXT("Item Selected"), MB_OK);
                    pAudioMidiSettings->comboBoxSelectedAtIndex((HWND)lParam, ItemIndex, ListItem);
                }
                else if (HIWORD(wParam) == CBN_DROPDOWN)
                {
                    //TCHAR  ListItem[256];
                    pAudioMidiSettings->comboBoxClicked((HWND)lParam);
                }
                else if (HIWORD(wParam) == BN_CLICKED)
                {
                    if ((HWND)lParam == testButton) pAudioMidiSettings->testButtonClicked();
                }

                wasHandled = true;
                result = 0;
                break;
            }

            case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, NULL, FALSE);
                wasHandled = true;
                result = 0;
                break;
            }

            //Make Static Controls Transparent
            case WM_CTLCOLORSTATIC:
            {
                HDC hdcStatic = (HDC)wParam;
                SetBkMode(hdcStatic, TRANSPARENT);
                return (LRESULT)GetStockObject(NULL_BRUSH);
            }

            case WM_CLOSE:	//when user clicks 'close' button on a win32 window, this is the first method that will handle it
            {
                fprintf(stdout, "\nWM_CLOSE\n");

                CMUniversalMessage* message = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;
                message->type = CMMessageTypeSystem;
                message->group = pba_shutdown;

                //message->system.status = enabled ? CMStatusStart : CMStatusStop;
                //message->system.uniqueID = midi->Source(index)->uniqueID; //one would typically use a unique id here but in WMS that is a WinRT hstring so we'll just use the index :(

                //send a message to the corresponding render thread to shutdown the platform 
                //if (MessageBox(hwnd,"Are you sure you want to close the window?", viewNode->view->title, MB_OKCANCEL) == IDOK)
                //{
                    //delete the view node, destroy the hglrc context
                    //crgc_delete_view_node(&hwnd);
                    //destroy the platform window, which will result in a WM_DESTROY message
                    //DestroyWindow(hwnd);
                //}
                // Indicate to Win32 that we processed this messasge ourselves

                //Send the shutdown message to the PbAudio Event Queue Thread

                //post quit via UMP message on PBA Event Thread's queue
                PostThreadMessage((DWORD)PBAudio.eventQueue.kq, PBA_EVENT_UMP_CONTROL, message->type, (LPARAM)message);

                //post directly to PBAudio event loop thread
                //PostThreadMessage(PBAudio.eventThreadID, PBA_EVENT_MSG_LOOP_QUIT, NULL, NULL);


                return 0;
            }

            case WM_DESTROY:
            {
                PostQuitMessage(0);
                wasHandled = true;
                result = 1;
                break;
            }
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}
