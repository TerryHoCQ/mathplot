/*
 * The following inline assembly incorporates Vera.ttf and friends *into the binary*. We
 * have different code for Linux and Mac. Both tested only on Intel CPUs.
 */

#ifdef __linux__

# ifdef __aarch64__

// "a", @progbits isn't liked by pi/arm, but "a", %progbits DOES seem to be necessary
asm("\n.pushsection vera_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf\"\n.popsection\n");
asm("\n.pushsection verait_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraIt.ttf\"\n.popsection\n");
asm("\n.pushsection verabd_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraBd.ttf\"\n.popsection\n");
asm("\n.pushsection verabi_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraBI.ttf\"\n.popsection\n");
asm("\n.pushsection veramono_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMono.ttf\"\n.popsection\n");
asm("\n.pushsection veramoit_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoIt.ttf\"\n.popsection\n");
asm("\n.pushsection veramobd_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoBd.ttf\"\n.popsection\n");
asm("\n.pushsection veramobi_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoBI.ttf\"\n.popsection\n");
asm("\n.pushsection verase_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraSe.ttf\"\n.popsection\n");
asm("\n.pushsection verasebd_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraSeBd.ttf\"\n.popsection\n");

// DejaVu Sans allows for Greek symbols and will be the default
asm("\n.pushsection dvsans_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansit_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-Oblique.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbd_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-Bold.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbi_ttf, \"a\", %progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-BoldOblique.ttf\"\n.popsection\n");

# else

// "a", @progbits means 'allocatable section containing type data'. It seems not to be strictly necessary.
asm("\n.pushsection vera_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf\"\n.popsection\n");
asm("\n.pushsection verait_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraIt.ttf\"\n.popsection\n");
asm("\n.pushsection verabd_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraBd.ttf\"\n.popsection\n");
asm("\n.pushsection verabi_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraBI.ttf\"\n.popsection\n");
asm("\n.pushsection veramono_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMono.ttf\"\n.popsection\n");
asm("\n.pushsection veramoit_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoIt.ttf\"\n.popsection\n");
asm("\n.pushsection veramobd_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoBd.ttf\"\n.popsection\n");
asm("\n.pushsection veramobi_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoBI.ttf\"\n.popsection\n");
asm("\n.pushsection verase_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraSe.ttf\"\n.popsection\n");
asm("\n.pushsection verasebd_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraSeBd.ttf\"\n.popsection\n");

// DejaVu Sans allows for Greek symbols and will be the default
asm("\n.pushsection dvsans_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansit_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-Oblique.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbd_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-Bold.ttf\"\n.popsection\n");
asm("\n.pushsection dvsansbi_ttf, \"a\", @progbits\n.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-BoldOblique.ttf\"\n.popsection\n");

#endif

#elif defined __APPLE__

// On Mac, we need a different incantation to use .incbin
asm("\t.global ___start_vera_ttf\n\t.global ___stop_vera_ttf\n___start_vera_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf\"\n___stop_vera_ttf:\n");
asm("\t.global ___start_verait_ttf\n\t.global ___stop_verait_ttf\n___start_verait_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraIt.ttf\"\n___stop_verait_ttf:\n");
asm("\t.global ___start_verabd_ttf\n\t.global ___stop_verabd_ttf\n___start_verabd_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraBd.ttf\"\n___stop_verabd_ttf:\n");
asm("\t.global ___start_verabi_ttf\n\t.global ___stop_verabi_ttf\n___start_verabi_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraBI.ttf\"\n___stop_verabi_ttf:\n");
asm("\t.global ___start_veramono_ttf\n\t.global ___stop_veramono_ttf\n___start_veramono_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMono.ttf\"\n___stop_veramono_ttf:\n");
asm("\t.global ___start_veramoit_ttf\n\t.global ___stop_veramoit_ttf\n___start_veramoit_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoIt.ttf\"\n___stop_veramoit_ttf:\n");
asm("\t.global ___start_veramobd_ttf\n\t.global ___stop_veramobd_ttf\n___start_veramobd_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoBd.ttf\"\n___stop_veramobd_ttf:\n");
asm("\t.global ___start_veramobi_ttf\n\t.global ___stop_veramobi_ttf\n___start_veramobi_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraMoBI.ttf\"\n___stop_veramobi_ttf:\n");
asm("\t.global ___start_verase_ttf\n\t.global ___stop_verase_ttf\n___start_verase_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraSe.ttf\"\n___stop_verase_ttf:\n");
asm("\t.global ___start_verasebd_ttf\n\t.global ___stop_verasebd_ttf\n___start_verasebd_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/ttf-bitstream-vera/VeraSeBd.ttf\"\n___stop_verasebd_ttf:\n");

asm("\t.global ___start_dvsans_ttf\n\t.global ___stop_dvsans_ttf\n___start_dvsans_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans.ttf\"\n___stop_dvsans_ttf:\n");
asm("\t.global ___start_dvsansit_ttf\n\t.global ___stop_dvsansit_ttf\n___start_dvsansit_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-Oblique.ttf\"\n___stop_dvsansit_ttf:\n");
asm("\t.global ___start_dvsansbd_ttf\n\t.global ___stop_dvsansbd_ttf\n___start_dvsansbd_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-Bold.ttf\"\n___stop_dvsansbd_ttf:\n");
asm("\t.global ___start_dvsansbi_ttf\n\t.global ___stop_dvsansbi_ttf\n___start_dvsansbi_ttf:\n\t.incbin \"" MPLOT_FONTS_DIR "/dejavu/DejaVuSans-BoldOblique.ttf\"\n___stop_dvsansbi_ttf:\n");

#elif defined _MSC_VER

# include <mplot/fonts/verafonts.h> // Includes vera fonts AND DejaVu fonts.
# include <cstdlib>

#elif defined _mplot_WIN__INCBIN // Define this only for parsing this file with the incbin executable to create verafonts.h

// Visual Studio doesn't allow __asm{} calls in C__ code anymore, so try Dale Weiler's incbin.h
#define INCBIN_PREFIX vf_
#include <mplot/fonts/incbin.h>
INCBIN(vera, "./fonts/ttf-bitstream-vera/Vera.ttf");
INCBIN(verait, "./fonts/ttf-bitstream-vera/VeraIt.ttf");
INCBIN(verabd, "./fonts/ttf-bitstream-vera/VeraBd.ttf");
INCBIN(verabi, "./fonts/ttf-bitstream-vera/VeraBI.ttf");
INCBIN(veramono, "./fonts/ttf-bitstream-vera/VeraMono.ttf");
INCBIN(veramoit, "./fonts/ttf-bitstream-vera/VeraMoIt.ttf");
INCBIN(veramobd, "./fonts/ttf-bitstream-vera/VeraMoBd.ttf");
INCBIN(veramobi, "./fonts/ttf-bitstream-vera/VeraMoBI.ttf");
INCBIN(verase, "./fonts/ttf-bitstream-vera/VeraSe.ttf");
INCBIN(verasebd, "./fonts/ttf-bitstream-vera/VeraSeBd.ttf");
// These translation units now have three symbols, eg:
// extern const unsigned char vf_veraData[];
// extern const unsigned char *const vf_veraEnd;
// extern const unsigned int vf_veraSize;

INCBIN(dvsans, "./fonts/dejavu/DejaVuSans.ttf");
INCBIN(dvsansit, "./fonts/dejavu/DejaVuSans-Oblique.ttf");
INCBIN(dvsansbd, "./fonts/dejavu/DejaVuSans-Bold.ttf");
INCBIN(dvsansbi, "./fonts/dejavu/DejaVuSans-BoldOblique.ttf");

#else
# error "Inline assembly code for including truetype fonts in the binary only work on Linux/MacOS (and then, probably only on Intel compatible compilers. Sorry about that!"
#endif

#include <mplot/VisualFaceAsm.h>

// Dummy function
int meaningless::function() { return 42; }
