/*!
 * \file
 *
 * A single-int OpenGL versioning scheme.
 *
 * \author Seb James
 * \date January 2024
 */
module;
#include <string>
#include <cstdint>

export module mplot.gl.version;

export namespace mplot::gl
{
    //!@{
    /*!
     * I encode the OpenGL version in a single int value, which can be passed as a template
     * argument to mplot::Visual and friends. These are the human-readable definitions. You can
     * pass, for example `mplot::gl::version_4_3` as the argument to your template.
     */
    constexpr std::int32_t version_4_1        = 0x00040001;
    constexpr std::int32_t version_4_1_compat = 0x20040001;
    constexpr std::int32_t version_4_2        = 0x00040002;
    constexpr std::int32_t version_4_2_compat = 0x20040002;
    constexpr std::int32_t version_4_3        = 0x00040003;
    constexpr std::int32_t version_4_3_compat = 0x20040003;
    constexpr std::int32_t version_4_4        = 0x00040004;
    constexpr std::int32_t version_4_4_compat = 0x20040004;
    constexpr std::int32_t version_4_5        = 0x00040005;
    constexpr std::int32_t version_4_5_compat = 0x20040005;
    constexpr std::int32_t version_4_6        = 0x00040006;
    constexpr std::int32_t version_4_6_compat = 0x20040006;
    constexpr std::int32_t version_3_0_es     = 0x40030000; // OpenGL 3.0 ES is a subset of OpenGL 3.3
    constexpr std::int32_t version_3_1_es     = 0x40030001; // OpenGL 3.1 ES is a subset of OpenGL 4.3
    constexpr std::int32_t version_3_2_es     = 0x40030002;
    //!@{

    /*
     * The mplot::gl::version namespace contains static and constexpr methods to decode the
     * single OpenGL version integer into minor, major, compat, gles and to generate strings
     * which describe the version. The bottom 16 bits encode the minor version number. The next
     * 13 bits encode the major version number. bit 29 encodes the 'compatibility' flag and bit
     * 30 encodes the OpenGL ES flag. Note that outdated versions with a 3rd number such as
     * OpenGL 1.2.1 are NOT supported here.
     */
    namespace version
    {
        // Open GL minor version number
        std::int32_t constexpr minor (const std::int32_t gl_version_number)
        {
            return (gl_version_number & 0xffff);
        }
        // Open GL major version number
        std::int32_t constexpr major (const std::int32_t gl_version_number)
        {
            return (gl_version_number >> 16 & 0x1fff);
        }
        // True if this is the compatibility profile (by default it's the core profile)
        bool constexpr compat (const std::int32_t gl_version_number)
        {
            return (((gl_version_number >> 29) & 0x1) > 0x0) ? true : false;
        }
        // True if this is an OpenGL ES version
        bool constexpr gles (const std::int32_t gl_version_number)
        {
            return (((gl_version_number >> 30) & 0x1) > 0x0) ? true : false;
        }
        // True if this version suports shader storage buffer objects
        bool constexpr has_ssbo (const std::int32_t gl_version_number)
        {
            if (mplot::gl::version::gles (gl_version_number) == true) {
                // OpenGL ES 3.1 and up supports SSBO
                return (mplot::gl::version::major (gl_version_number) > 3
                        || (mplot::gl::version::major (gl_version_number) == 3
                            && mplot::gl::version::minor (gl_version_number)  >= 1));
            } else {
                // OpenGL 4.3 and up supports SSBO
                return (mplot::gl::version::major (gl_version_number) > 4
                        || (mplot::gl::version::major (gl_version_number) == 4
                            && mplot::gl::version::minor (gl_version_number)  >= 3));
            }
        }
        // Output a string describing the version number
        inline std::string vstring (const std::int32_t gl_version_number)
        {
            std::string v = std::to_string (version::major(gl_version_number)) + std::string(".")
            + std::to_string (version::minor(gl_version_number));
            if (version::compat(gl_version_number)) {
                v += " compat";
            }
            if (version::gles(gl_version_number)) {
                v += " ES";
            }
            return v;
        }
        // Return the version-specific shader preamble as a const char* from a constexpr function
        constexpr const char* shaderpreamble (const std::int32_t gl_version_number)
        {
            const char* preamble = "#version unknown\n";

            switch (gl_version_number) {
            case mplot::gl::version_3_0_es:
                preamble = "#version 300 es\n#extension GL_EXT_shader_io_blocks : enable\nprecision mediump float;\n";
                break;
            case mplot::gl::version_3_1_es:
                preamble = "#version 310 es\n#extension GL_EXT_shader_io_blocks : enable\nprecision mediump float;\n";
                break;
            case mplot::gl::version_3_2_es:
                preamble = "#version 320 es\n#extension GL_EXT_shader_io_blocks : enable\nprecision mediump float;\n";
                break;
            case mplot::gl::version_4_1:
            case mplot::gl::version_4_1_compat:
                preamble = "#version 410\n";
                break;
            case mplot::gl::version_4_2:
            case mplot::gl::version_4_2_compat:
                preamble = "#version 420\n";
                break;
            case mplot::gl::version_4_3:
            case mplot::gl::version_4_3_compat:
                preamble = "#version 430\n";
                break;
            case mplot::gl::version_4_4:
            case mplot::gl::version_4_4_compat:
                preamble = "#version 440\n";
                break;
            case mplot::gl::version_4_5:
            case mplot::gl::version_4_5_compat:
                preamble = "#version 450\n";
                break;
            case mplot::gl::version_4_6:
            case mplot::gl::version_4_6_compat:
                preamble = "#version 460\n";
                break;
            default:
                break;
            }
            return preamble;
        }
    } // namespace version
} // namespace mplot::gl
