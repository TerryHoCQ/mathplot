// See also VisualFaceAsm.cpp

#ifdef _MSC_VER
# include <mplot/fonts/verafonts.h> // Includes vera fonts AND DejaVu fonts.
#endif

// These external pointers are set up by the inline assembly above
#ifndef _MSC_VER
extern const char __start_verabd_ttf[];
extern const char __stop_verabd_ttf[];
extern const char __start_verabi_ttf[];
extern const char __stop_verabi_ttf[];
extern const char __start_verait_ttf[];
extern const char __stop_verait_ttf[];
extern const char __start_veramobd_ttf[];
extern const char __stop_veramobd_ttf[];
extern const char __start_veramobi_ttf[];
extern const char __stop_veramobi_ttf[];
extern const char __start_veramoit_ttf[];
extern const char __stop_veramoit_ttf[];
extern const char __start_veramono_ttf[];
extern const char __stop_veramono_ttf[];
extern const char __start_verasebd_ttf[];
extern const char __stop_verasebd_ttf[];
extern const char __start_verase_ttf[];
extern const char __stop_verase_ttf[];
extern const char __start_vera_ttf[];
extern const char __stop_vera_ttf[];

extern const char __start_dvsans_ttf[];
extern const char __stop_dvsans_ttf[];
extern const char __start_dvsansit_ttf[];
extern const char __stop_dvsansit_ttf[];
extern const char __start_dvsansbd_ttf[];
extern const char __stop_dvsansbd_ttf[];
extern const char __start_dvsansbi_ttf[];
extern const char __stop_dvsansbi_ttf[];
#endif

namespace meaningless { int function(); }
