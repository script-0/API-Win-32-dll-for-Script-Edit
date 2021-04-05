#include "color.h"

#ifdef __cplusplus
extern "C"
{
#endif

DLL_EXPORT BOOL LoadColor(APP_COLOR* mycolor) /**<Load default colors and return TRUE if successful and FALSE otherwise*/
{
    mycolor->text_color_idm_text=DEFAULT_COLOR;
    mycolor->bck_color_idm_text=DEFAULT_BKCOLOR;
    mycolor->bck_color_s_idm_text=DEFAULT_BKCOLOR_S;
    mycolor->static_bck_color=DEFAULT_BKCOLOR_STATIC_CONTROL;
    mycolor->text_color_file_name=DEFAULT_TEXT_COLOR_RICH_EDIT;
    mycolor->bck_color_file_name=DEFAULT_BCK_COLOR_RICH_EDIT;
    mycolor->static_text_color=DEFAULT_COLOR_STATIC_CONTROL;
    return TRUE;
}

DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

#ifdef __cplusplus
}
#endif // __cplusplus
