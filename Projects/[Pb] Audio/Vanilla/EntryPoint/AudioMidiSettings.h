// Windows Header Files:
#include <windows.h>
#include <CommCtrl.h>

// C RunTime Header Files
#include <math.h>

#include <objbase.h>

#include "../../EntryPoint/PbAudioAppInterface.h"

/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/
template<class Interface>
inline void
SafeRelease(
    Interface** ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

/******************************************************************
*                                                                 *
*  AudioMidiSettings                                                        *
*                                                                 *
******************************************************************/

class AudioMidiSettings
{
public:
    AudioMidiSettings();
    AudioMidiSettings(struct CMClientDriver *midi);
    ~AudioMidiSettings();

    HRESULT Initialize();
    void RunMessageLoop();

    int  UpdateOutputDeviceList();

    //void Midi() { return m_midi };

private:
    //HRESULT CreateResources();
    //void    DiscardResources();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListViewSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void CreateDriverModeLabel();
    void CreateDriverModePopupButton();

    //Output [Audio] Device
    void CreateOutputDeviceLabel();
    void CreateTestButton();
    void CreateOutputPopupButton();

    //Output [Audio] Channels
    void CreateOutputChannelsLabel();
    void CreateOutputChannelFilterView();

    //Sample Rate
    void CreateSampleRateLabel();
    void CreateSampleRatePopupButton();

    //Buffer Size
    void CreateBufferSizeLabel();
    void CreateBufferSizePopupButton();

    //MIDI Inputs
    void CreateMidiInputLabel();
    void CreateMidiInputFilterView();

    int  UpdateDriverModeList();
    int  UpdateSampleRateList();
    int  UpdateBufferSizeList();

    void buttonToggledAtIndex(int index, HWND sender, bool state, struct CMClientDriver * midi);
    void comboBoxSelectedAtIndex(HWND sender, int selectedIndex, TCHAR* name);


private:
    HWND                   m_hwnd;
};