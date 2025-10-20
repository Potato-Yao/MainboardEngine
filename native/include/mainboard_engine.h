// for communicate with Java, this header file is C style
// files of this project follows Google naming standard

#ifndef MainboardEngine_H
#define MainboardEngine_H

#if defined(_WIN32)
#ifdef MAINBOARD_NATIVE_EXPORTS
#define ME_API __declspec(dllexport)
#else
#define ME_API __declspec(dllimport)
#endif
#else
#define ME_API
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define ME_BOOL int
#define ME_TRUE 1
#define ME_FALSE 0

typedef void *ME_HANDLE;

ME_API ME_BOOL ME_Initialize();

ME_API ME_HANDLE ME_CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title);

ME_API ME_BOOL ME_ProcessEvents(ME_HANDLE handle);

ME_API ME_BOOL ME_RenderFrame(ME_HANDLE handle);

ME_API ME_BOOL ME_DestroyWindow(ME_HANDLE handle);

ME_API ME_HANDLE ME_GetMEWindowHandle(ME_HANDLE handle);

ME_API ME_BOOL ME_SetWindowSize(ME_HANDLE handle, int width, int height);

ME_API ME_BOOL ME_SetWindowTitle(ME_HANDLE handle, const char *title);

#ifdef __cplusplus
}
#endif


#endif //MainboardEngine_H
