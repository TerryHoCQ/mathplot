/*
 * Helper to load PNG images into mplot::vvec<mplot::vec<float>> format and similar.
 */
module;

#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS 1
#include <mplot/lodepng.h>

#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <cstddef>
#include <stdexcept>
#include <fstream>

export module mplot.loadpng;

export import sm.vec;
import sm.vvec;

export namespace mplot
{
    std::uint32_t pnm_encode (const std::string& img_filename, const std::uint8_t* raw, std::int32_t w, std::int32_t h)
    {
        std::ofstream fout (img_filename, std::ios::out | std::ios::trunc);
        if (!fout.is_open()) {
            return 1;
        }

        fout << "P6\n"
             << "#mathplot frame size " << w << "x" << h << "\n"
             << w << " " << h << "\n255\n";

        for (std::int32_t i = 0; i < h; ++i) {
            std::int32_t for_line = i * 4 * w;
            for (std::int32_t j = 0; j < 4 * w; ++j) {
                if (j % 4 != 3) {
                    // access raw[for_line + j];
                    fout.write (reinterpret_cast<const char*>(&raw[for_line + j]), 1);
                }
            }
        }

        fout.close();

        return 0u;
    }

    std::uint32_t png_encode (const std::string& img_filename, const std::uint8_t* in, std::int32_t w, std::int32_t h)
    {
        if (w < 0 || h < 0) { return std::numeric_limits<std::uint32_t>::max(); }
        return lodepng::encode (img_filename, in, w, h);
    }

    std::string png_error_text (const std::uint32_t error) { return lodepng_error_text (error); }

    /*
     * Wrap lodepng::decode to load a PNG from file, placing the data into the
     * image_data array. Figure out based on the type of T, how to scale the numbers.
     *
     * Use with T as float, double, std::uint8_t/int32_t or mplot::vec<float, 3> etc
     *
     * If flip[0] is true, then flip the order of the rows to do a left/right flip of
     * the image during loading.
     *
     * If flip[1] is true, then flip the order of the rows to do an up/down flip of the
     * image during loading.
     *
     * Note: The default for flip is {false, true}, which means that by default,
     * image_data will be filled in a bottom-left to top-right order.
     */
    template <typename T>
    sm::vec<std::uint32_t, 2> loadpng (const std::string& filename, sm::vvec<T>& image_data,
                                       const sm::vec<bool,2> flip = {false, true})
    {
        std::vector<std::uint8_t> png;
        std::uint32_t w = 0;
        std::uint32_t h = 0;
        // Assume RGBA and bit depth of 8
        std::uint32_t lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "mplot::loadpng: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text (lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        sm::vec<std::uint32_t, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        std::uint32_t vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("mplot::loadpng: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz/4);

        for (std::uint32_t c = 0; c < dims[1]; ++c) {
            for (std::uint32_t r = 0; r < dims[0]; ++r) {
                // Offset into png
                std::uint32_t i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                std::uint32_t idx = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));

                // The above unpacks as:

                // flip[0]==false and flip[1]==false: (no flipping)
                // idx = r + dims[0]*c

                // flip[0]==false and flip[1]==true: (vertical flip)
                // idx = (r + dims[0]*(dims[1]-c-1))

                // flip[0]==true and flip[1]==false: (horizontal flip)
                // ((dims[0]-r-1) + dims[0]*c))

                // flip[0]==true and flip[1]==true:  (v-h flip)
                // ((dims[0]-r-1) + dims[0]*(dims[1]-c-1))

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    // monochrome 0-1 values
                    image_data[idx] = (static_cast<T>(png[i] + png[i+1] + png[i+2]))/T{765}; // 3*255

                } else if constexpr (std::is_same<std::decay_t<T>, std::uint32_t>::value == true
                                     || std::is_same<std::decay_t<T>, std::uint8_t>::value == true) {
                    // monochrome, 0-255 values
                    image_data[idx] = (static_cast<T>(png[i] + png[i+1] + png[i+2]))/T{3};

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("mplot::loadpng: type failure");
                }
            }
        }

        return dims;
    }

    /*
     * This overload pf loadpng reads the image into a vvec of vecs of dimension 3 or 4.
     *
     * \tparam T The type of the individual channels. Expected to be std::uint32_t,
     * std::uint8_t, float or double.
     *
     * \tparam N The number of channels (3 for RGB; 4 for RGBA, anything else will lead
     * to errors)
     */
    template <typename T, std::size_t N>
    sm::vec<std::uint32_t, 2> loadpng (const std::string& filename,
                                       sm::vvec<sm::vec<T, N>>& image_data,
                                       const sm::vec<bool,2> flip = {false, true})
    {
        std::vector<std::uint8_t> png;
        std::uint32_t w = 0;
        std::uint32_t h = 0;
        // Assume RGBA and bit depth of 8
        std::uint32_t lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "mplot::loadpng: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text (lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        sm::vec<std::uint32_t, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        std::uint32_t vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("mplot::loadpng: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz/4);

        for (std::uint32_t c = 0; c < dims[1]; ++c) {
            for (std::uint32_t r = 0; r < dims[0]; ++r) {

                // Offset into png
                std::uint32_t i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                std::uint32_t idx = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));

                if constexpr ((std::is_same<std::decay_t<T>, float>::value == true
                               || std::is_same<std::decay_t<T>, double>::value == true) && N==3) {
                    // RGB, 0-1 values
                    std::uint8_t p0 = png[i];
                    std::uint8_t p1 = png[i+1];
                    std::uint8_t p2 = png[i+2];
                    image_data[idx] = { static_cast<T>(p0), static_cast<T>(p1), static_cast<T>(p2) };
                    image_data[idx] /= T{255};

                } else if constexpr ((std::is_same<std::decay_t<T>, float>::value == true
                                      || std::is_same<std::decay_t<T>, double>::value == true) && N==4) {
                    // RGBA 0-1 values
                    image_data[idx][0] = static_cast<T>(png[i]) / T{255};
                    image_data[idx][1] = static_cast<T>(png[i+1]) / T{255};
                    image_data[idx][2] = static_cast<T>(png[i+2]) / T{255};
                    image_data[idx][3] = static_cast<T>(png[i+3]) / T{255};

                } else if constexpr ((std::is_same<std::decay_t<T>, std::uint8_t>::value == true
                                      || std::is_same<std::decay_t<T>, std::uint32_t>::value == true) && N==3) {
                    // RGB, 0-255 values
                    image_data[idx][0] = static_cast<T>(png[i]);
                    image_data[idx][1] = static_cast<T>(png[i+1]);
                    image_data[idx][2] = static_cast<T>(png[i+2]);

                } else if constexpr ((std::is_same<std::decay_t<T>, std::uint8_t>::value == true
                                      || std::is_same<std::decay_t<T>, std::uint32_t>::value == true) && N==4) {
                    // RGBA, 0-255 values
                    image_data[idx][0] = static_cast<T>(png[i]);
                    image_data[idx][1] = static_cast<T>(png[i+1]);
                    image_data[idx][2] = static_cast<T>(png[i+2]);
                    image_data[idx][3] = static_cast<T>(png[i+3]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("mplot::loadpng: type failure (or N is not 3 or 4)");
                }
            }
        }

        return dims;
    }

    // Load a colour PNG and return a vector of type T with elements ordered as RGBRGBRGB...
    template <typename T>
    sm::vec<std::uint32_t, 2> loadpng_rgb (const std::string& filename, sm::vvec<T>& image_data,
                                           const sm::vec<bool,2> flip = {false, true})
    {
        std::vector<std::uint8_t> png;
        std::uint32_t w = 0;
        std::uint32_t h = 0;
        // Assume RGBA and bit depth of 8
        std::uint32_t lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "mplot::loadpng_rgb: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text (lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        sm::vec<std::uint32_t, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then for each in RGB, convert to range 0 to 1
        // If T is of integer type, then for each in RGB encode in range 0-255

        std::uint32_t vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("mplot::loadpng_rgb: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (3*vsz/4);

        for (std::uint32_t c = 0; c < dims[1]; ++c) {
            for (std::uint32_t r = 0; r < dims[0]; ++r) {
                // Offset into png
                std::uint32_t i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                std::uint32_t idx_r = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));
                idx_r *= 3; // Because our output is rgbrgb...
                std::uint32_t idx_g = flip[0] == true ? idx_r-1 : idx_r+1;
                std::uint32_t idx_b = flip[0] == true ? idx_r-2 : idx_r+2;

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    image_data[idx_r] = static_cast<T>(png[i])/T{255};
                    image_data[idx_g] = static_cast<T>(png[i+1])/T{255};
                    image_data[idx_b] = static_cast<T>(png[i+2])/T{255};

                } else if constexpr (std::is_same<std::decay_t<T>, std::uint32_t>::value == true
                                     || std::is_same<std::decay_t<T>, std::uint8_t>::value == true) {
                    // Copy RGB, 0-255 values
                    image_data[idx_r] = static_cast<T>(png[i]);
                    image_data[idx_g] = static_cast<T>(png[i+1]);
                    image_data[idx_b] = static_cast<T>(png[i+2]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("mplot::loadpng_rgb: type failure");
                }
            }
        }

        return dims;
    }

    // Load a colour PNG and return a vector of type T with elements ordered as RGBARGBARGBA...
    template <typename T>
    sm::vec<std::uint32_t, 2> loadpng_rgba (const std::string& filename, sm::vvec<T>& image_data,
                                            const sm::vec<bool,2> flip = {false, true})
    {
        std::vector<std::uint8_t> png;
        std::uint32_t w = 0;
        std::uint32_t h = 0;
        // Assume RGBA and bit depth of 8
        std::uint32_t lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "mplot::loadpng_rgba: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text(lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        sm::vec<std::uint32_t, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        std::uint32_t vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("mplot::loadpng_rgba: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz);

        for (std::uint32_t c = 0; c < dims[1]; ++c) {
            for (std::uint32_t r = 0; r < dims[0]; ++r) {
                // Offset into png
                std::uint32_t i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                std::uint32_t idx_r = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));
                idx_r *= 4; // Because our output is rgbargba...
                std::uint32_t idx_g = flip[0] == true ? idx_r-1 : idx_r+1;
                std::uint32_t idx_b = flip[0] == true ? idx_r-2 : idx_r+2;
                std::uint32_t idx_a = flip[0] == true ? idx_r-3 : idx_r+3;

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    image_data[idx_r] = static_cast<T>(png[i])/T{255};
                    image_data[idx_g] = static_cast<T>(png[i+1])/T{255};
                    image_data[idx_b] = static_cast<T>(png[i+2])/T{255};
                    image_data[idx_a] = static_cast<T>(png[i+3])/T{255};

                } else if constexpr (std::is_same<std::decay_t<T>, std::uint32_t>::value == true
                                     || std::is_same<std::decay_t<T>, std::uint8_t>::value == true) {
                    // Copy RGB, 0-255 values
                    image_data[idx_r] = static_cast<T>(png[i]);
                    image_data[idx_g] = static_cast<T>(png[i+1]);
                    image_data[idx_b] = static_cast<T>(png[i+2]);
                    image_data[idx_a] = static_cast<T>(png[i+3]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("mplot::loadpng_rgba: type failure");
                }
            }
        }

        return dims;
    }

    // Load a colour PNG and return a vector of type T with elements ordered as RGBARGBARGBA...
    template <typename T, std::uint32_t im_w, std::uint32_t im_h>
    sm::vec<std::uint32_t, 2> loadpng_rgba (const std::string& filename, sm::vec<T, 4*im_w*im_h>& image_data,
                                            const sm::vec<bool,2> flip = {false, true})
    {
        std::vector<std::uint8_t> png;
        std::uint32_t w = 0;
        std::uint32_t h = 0;
        // Assume RGBA and bit depth of 8
        std::uint32_t lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "mplot::loadpng_rgba: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text(lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        sm::vec<std::uint32_t, 2> dims = {w, h};
        if (w != im_w || h != im_h) {
            throw std::runtime_error ("mplot::loadpng_rgba: Expect png to be the size specified in the template args.");
        }

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        std::uint32_t vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("mplot::loadpng_rgba: Expect png vector to have size divisible by 4.");
        }

        for (std::uint32_t c = 0; c < dims[1]; ++c) {
            for (std::uint32_t r = 0; r < dims[0]; ++r) {
                // Offset into png
                std::uint32_t i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                std::uint32_t idx_r = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));
                idx_r *= 4; // Because our output is rgbargba...
                std::uint32_t idx_g = flip[0] == true ? idx_r-1 : idx_r+1;
                std::uint32_t idx_b = flip[0] == true ? idx_r-2 : idx_r+2;
                std::uint32_t idx_a = flip[0] == true ? idx_r-3 : idx_r+3;

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    image_data[idx_r] = static_cast<T>(png[i])/T{255};
                    image_data[idx_g] = static_cast<T>(png[i+1])/T{255};
                    image_data[idx_b] = static_cast<T>(png[i+2])/T{255};
                    image_data[idx_a] = static_cast<T>(png[i+3])/T{255};

                } else if constexpr (std::is_same<std::decay_t<T>, std::uint32_t>::value == true
                                     || std::is_same<std::decay_t<T>, std::uint8_t>::value == true) {
                    // Copy RGB, 0-255 values
                    image_data[idx_r] = static_cast<T>(png[i]);
                    image_data[idx_g] = static_cast<T>(png[i+1]);
                    image_data[idx_b] = static_cast<T>(png[i+2]);
                    image_data[idx_a] = static_cast<T>(png[i+3]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("mplot::loadpng_rgba: type failure");
                }
            }
        }

        return dims;
    }

} // namespace
