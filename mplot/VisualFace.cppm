/*!
 * \file
 *
 * Declares a VisualFace class to hold the information about a (Freetype-managed) font
 * face and the GL-textures that will reproduce it.
 *
 * This is the non-GL base class.
 *
 * \author Seb James
 * \date November 2020
 */
module;

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# include <mplot/glad/gl.h>
#endif

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <map>
#include <iostream>
#include <utility>
#include <fstream>

#include <mplot/VisualFaceAsm.hpp>

export module mplot.visualface;

import mplot.visualcommon;
import mplot.visualfont;
import mplot.textfeatures;
import mplot.tools;

import sm.vec;

export namespace mplot::visgl
{
    struct VisualFace
    {
        VisualFace () { std::cout << "meaningless function: " << meaningless::function() << std::endl; }
        /*!
         * Construct with a mplot::VisualFont \a _font, which specifies a supported
         * font (one which we can legally include in the source code without paying any licence fees,
         * e.g. Bitstream Vera) and \a fontpixels, which is the texture size,
         * e.g. 192. This is the width, in pixels, of the texture that would be
         * applied to the letter 'm'. A larger value is required for fonts that will
         * take up a large part of the screen, but will be detrimental to the
         * appearance of a font which is rendered 'small on the screen'.
         *
         * VisualResources holds a map of VisualFace instances, to avoid many copies
         * of font textures for separate VisualTextModel instances which might have
         * the same pixel size.
         */
        VisualFace (const mplot::VisualFont _font, std::uint32_t fontpixels, FT_Library& ft_freetype,
                    GladGLContext* glfn = nullptr)
        {
            constexpr bool debug_visualface = false;

            this->init_common (_font, fontpixels, ft_freetype);

            // How far to loop. In principle, up to 21 bits worth - that's 2097151 possible characters!
            for (char32_t c = 0; c < 2097151; c++) {
                // Check glyph index first, if it's 0 it's a blank so skip.
                if (FT_Get_Char_Index (this->face, c) == 0) { continue; }

                // load character glyph
                if (FT_Load_Char (this->face, c, FT_LOAD_RENDER)) {
                    std::cout << "ERROR::FREETYPE: Failed to load Glyph for Unicode 0x"
                              << std::hex << static_cast<std::uint32_t>(c) << std::dec << std::endl;
                    continue;
                }

                // generate texture
                std::uint32_t texture = 0;

                if (glfn == nullptr) { throw std::runtime_error ("glfn problem"); }
                glfn->GenTextures (1, &texture);
                glfn->BindTexture (GL_TEXTURE_2D, texture);
                glfn->TexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    this->face->glyph->bitmap.width,
                    this->face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    this->face->glyph->bitmap.buffer
                    );
                // set texture options
                glfn->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glfn->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glfn->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glfn->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Could be GL_NEAREST, but doesn't look as good.

                // now store character for later use
                mplot::visgl::CharInfo glchar = {
                    texture,
                    {static_cast<std::int32_t>(this->face->glyph->bitmap.width),
                     static_cast<std::int32_t>(this->face->glyph->bitmap.rows)}, // size
                    {this->face->glyph->bitmap_left, this->face->glyph->bitmap_top}, // bearing
                    static_cast<std::uint32_t>(this->face->glyph->advance.x)          // advance
                };

                if constexpr (debug_visualface == true) {
                    std::cout << "Inserting character into this->glchars with info: ID:" << glchar.textureID
                              << ", Size:" << glchar.size << ", Bearing:" << glchar.bearing
                              << ", Advance:" << glchar.advance << std::endl;
                }
                this->glchars.insert (std::pair<char32_t, mplot::visgl::CharInfo>(c, glchar));
            }
            glfn->BindTexture(GL_TEXTURE_2D, 0);

            // At this point could FT_Done_Face() etc, I think. as we no longer do anything Freetypey with it.
            FT_Done_Face (this->face);
        }

        ~VisualFace () {}

        //! The FT_Face that we're managing
        FT_Face face;

        //! The OpenGL character info stuff
        std::map<char32_t, mplot::visgl::CharInfo> glchars;

    protected:

        void init_common (const mplot::VisualFont _font, std::uint32_t fontpixels, FT_Library& ft_freetype)
        {
            constexpr bool debug_visualface = false;

            std::string fontpath = "";
#ifdef _MSC_VER
            char* userprofile = getenv ("USERPROFILE");
            std::string uppath("");
            if (userprofile != nullptr) {
                uppath = std::string (userprofile);
            }

            switch (_font) {
            case VisualFont::DVSans:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_dvsansData, vf_dvsansEnd);
                break;
            }
            case VisualFont::DVSansItalic:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans-Oblique.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_dvsansitData, vf_dvsansitEnd);
                break;
            }
            case VisualFont::DVSansBold:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans-Bold.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_dvsansbdData, vf_dvsansbdEnd);
                break;
            }
            case VisualFont::DVSansBoldItalic:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\DejaVuSans-BoldOblique.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_dvsansbiData, vf_dvsansbiEnd);
                break;
            }
            case VisualFont::Vera:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\Vera.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veraData, vf_veraEnd);
                break;
            }
            case VisualFont::VeraItalic:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraIt.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veraitData, vf_veraitEnd);
                break;
            }
            case VisualFont::VeraBold:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraBd.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_verabdData, vf_verabdEnd);
                break;
            }
            case VisualFont::VeraBoldItalic:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraBI.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_verabiData, vf_verabiEnd);
                break;
            }
            case VisualFont::VeraMono:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMono.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veramonoData, vf_veramonoEnd);
                break;
            }
            case VisualFont::VeraMonoBold:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMoBd.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veramobdData, vf_veramobdEnd);
                break;
            }
            case VisualFont::VeraMonoItalic:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMoIt.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veramoitData, vf_veramoitEnd);
                break;
            }
            case VisualFont::VeraMonoBoldItalic:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraMoBI.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veramobiData, vf_veramobiEnd);
                break;
            }
            case VisualFont::VeraSerif:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraSe.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_veraseData, vf_veraseEnd);
                break;
            }
            case VisualFont::VeraSerifBold:
            {
                fontpath = uppath + "\\AppData\\Local\\Temp\\VeraSeBd.ttf";
                this->makeTempFontFile<const std::uint8_t> (fontpath, vf_verasebdData, vf_verasebdEnd);
                break;
            }
            default:
            {
                std::cout << "ERROR::Unsupported mplot font\n";
                break;
            }
            }
#else	 // Non-windows:
            switch (_font) {
            case VisualFont::DVSans:
            {
                fontpath = "/tmp/DejaVuSans.ttf";
                this->makeTempFontFile (fontpath, __start_dvsans_ttf, __stop_dvsans_ttf);
                break;
            }
            case VisualFont::DVSansItalic:
            {
                fontpath = "/tmp/DejaVuSans-Oblique.ttf";
                this->makeTempFontFile (fontpath, __start_dvsansit_ttf, __stop_dvsansit_ttf);
                break;
            }
            case VisualFont::DVSansBold:
            {
                fontpath = "/tmp/DejaVuSans-Bold.ttf";
                this->makeTempFontFile (fontpath, __start_dvsansbd_ttf, __stop_dvsansbd_ttf);
                break;
            }
            case VisualFont::DVSansBoldItalic:
            {
                fontpath = "/tmp/DejaVuSans-BoldOblique.ttf";
                this->makeTempFontFile (fontpath, __start_dvsansbi_ttf, __stop_dvsansbi_ttf);
                break;
            }
            case VisualFont::Vera:
            {
                fontpath = "/tmp/Vera.ttf";
                this->makeTempFontFile (fontpath, __start_vera_ttf, __stop_vera_ttf);
                break;
            }
            case VisualFont::VeraItalic:
            {
                fontpath = "/tmp/VeraIt.ttf";
                this->makeTempFontFile (fontpath, __start_verait_ttf, __stop_verait_ttf);
                break;
            }
            case VisualFont::VeraBold:
            {
                fontpath = "/tmp/VeraBd.ttf";
                this->makeTempFontFile (fontpath, __start_verabd_ttf, __stop_verabd_ttf);
                break;
            }
            case VisualFont::VeraBoldItalic:
            {
                fontpath = "/tmp/VeraBI.ttf";
                this->makeTempFontFile (fontpath, __start_verabi_ttf, __stop_verabi_ttf);
                break;
            }
            case VisualFont::VeraMono:
            {
                fontpath = "/tmp/VeraMono.ttf";
                this->makeTempFontFile (fontpath, __start_veramono_ttf, __stop_veramono_ttf);
                break;
            }
            case VisualFont::VeraMonoBold:
            {
                fontpath = "/tmp/VeraMoBd.ttf";
                this->makeTempFontFile (fontpath, __start_veramobd_ttf, __stop_veramobd_ttf);
                break;
            }
            case VisualFont::VeraMonoItalic:
            {
                fontpath = "/tmp/VeraMoIt.ttf";
                this->makeTempFontFile (fontpath, __start_veramoit_ttf, __stop_veramoit_ttf);
                break;
            }
            case VisualFont::VeraMonoBoldItalic:
            {
                fontpath = "/tmp/VeraMoBI.ttf";
                this->makeTempFontFile (fontpath, __start_veramobi_ttf, __stop_veramobi_ttf);
                break;
            }
            case VisualFont::VeraSerif:
            {
                fontpath = "/tmp/VeraSe.ttf";
                this->makeTempFontFile (fontpath, __start_verase_ttf, __stop_verase_ttf);
                break;
            }
            case VisualFont::VeraSerifBold:
            {
                fontpath = "/tmp/VeraSeBd.ttf";
                this->makeTempFontFile (fontpath, __start_verasebd_ttf, __stop_verasebd_ttf);
                break;
            }
            default:
            {
                std::cout << "ERROR::Unsupported mplot font\n";
                break;
            }
            }
#endif // Windows/Non-windows

            // Keep the face as a mplot::Visual owned resource, shared by VisTextModels?
            if constexpr (debug_visualface == true) {
                std::cout << "FT_New_Face (ft_freetype, " << fontpath << ", 0, &this->face);\n";
            }
            if (FT_New_Face (ft_freetype, fontpath.c_str(), 0, &this->face)) {
                std::cout << "ERROR::FREETYPE: Failed to load font (font file may be invalid)" << std::endl;
            }

            FT_Set_Pixel_Sizes (this->face, 0, fontpixels);

            // Can I check this->face for how many glyphs it has? Yes:
            // std::cout << "This face has " << this->face->num_glyphs << " glyphs.\n";
        }

        //! Create a temporary font file at fontpath, using the embedded data
        //! starting from filestart and extending to filenend
        template <typename T = const char>
        void makeTempFontFile (const std::string& fontpath, T* file_start, T* file_stop)
        {
            constexpr bool debug_visualface = false;
            T* p;
            if (!mplot::tools::fileExists (fontpath)) {
                std::ofstream fout;
                fout.open (fontpath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
                if (fout.is_open()) {
                    for (p = file_start; p < file_stop; p++) { fout << *p; }
                    fout.close();
                } else {
                    std::cout << "WARNING: Failed to open " << fontpath << "!!\n";
                }
            } else {
                if constexpr (debug_visualface == true) {
                    std::cout << "INFO: " << fontpath << " already exists, no need to re-create it\n";
                }
            }
        }
    };
} // namespace
