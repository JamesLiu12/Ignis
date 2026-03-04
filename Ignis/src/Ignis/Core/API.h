#if defined(_WIN32)
    #if defined(IGNIS_BUILD_DLL)
        #define IGNIS_API __declspec(dllexport)
    #else
        #define IGNIS_API __declspec(dllimport)
    #endif
#else
    #define IGNIS_API __attribute__((visibility("default")))
#endif