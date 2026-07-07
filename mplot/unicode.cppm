/*!
 * \file
 *
 * Unicode text manipulation utility code
 *
 * \author Seb James
 * \date January 2022
 */
module;

#include <cstdint>
#include <string>

export module mplot.unicode;

export namespace mplot::unicode
{
    /*
     * These constants are defined to make program code that uses unicode::toUtf8()
     * and unicode::append easier to read. You can also pass the raw codes.
     */

    // Greek lower case letters
    constexpr char32_t alpha = 0x03b1;
    constexpr char32_t beta = 0x03b2;
    constexpr char32_t gamma = 0x03b3;
    constexpr char32_t delta = 0x03b4;
    constexpr char32_t epsilon = 0x03b5;
    constexpr char32_t zeta = 0x03b6;
    constexpr char32_t eta = 0x03b7;
    constexpr char32_t theta = 0x03b8;
    constexpr char32_t iota = 0x03b9;
    constexpr char32_t kappa = 0x03ba;
    constexpr char32_t lambda = 0x03bb;
    constexpr char32_t mu = 0x03bc;
    constexpr char32_t nu = 0x03bd;
    constexpr char32_t xi = 0x03be;
    constexpr char32_t omicron = 0x03bf;
    constexpr char32_t pi = 0x03c0;
    constexpr char32_t rho = 0x03c1;
    constexpr char32_t finalsigma = 0x03c2;
    constexpr char32_t sigma = 0x03c3;
    constexpr char32_t tau = 0x03c4;
    constexpr char32_t upsilon = 0x03c5;
    constexpr char32_t phi = 0x03c6;
    constexpr char32_t chi = 0x03c7;
    constexpr char32_t psi = 0x03c8;
    constexpr char32_t omega = 0x03c9;

    // Greek upper case letters
    constexpr char32_t Alpha = 0x0391;
    constexpr char32_t Beta = 0x0392;
    constexpr char32_t Gamma = 0x0393;
    constexpr char32_t Delta = 0x0394;
    constexpr char32_t Epsilon = 0x0395;
    constexpr char32_t Zeta = 0x0396;
    constexpr char32_t Eta = 0x0397;
    constexpr char32_t Theta = 0x0398;
    constexpr char32_t Iota = 0x0399;
    constexpr char32_t Kappa = 0x039a;
    constexpr char32_t Lambda = 0x039b;
    constexpr char32_t Mu = 0x039c;
    constexpr char32_t Nu = 0x039d;
    constexpr char32_t Xi = 0x039e;
    constexpr char32_t Omicron = 0x039f;
    constexpr char32_t Pi = 0x03a0;
    constexpr char32_t Rho = 0x03a1;
    constexpr char32_t Sigma = 0x03a3;
    constexpr char32_t Tau = 0x03a4;
    constexpr char32_t Upsilon = 0x03a5;
    constexpr char32_t Phi = 0x03a6;
    constexpr char32_t Chi = 0x03a7;
    constexpr char32_t Psi = 0x03a8;
    constexpr char32_t Omega = 0x03a9;

    // Math symbols
    constexpr char32_t plusminus = 0x00b1;
    constexpr char32_t minusplus = 0x2213;
    constexpr char32_t divides = 0x00f7;
    constexpr char32_t multiplies = 0x00d7;
    constexpr char32_t forall = 0x2200;
    constexpr char32_t exists = 0x2203;
    constexpr char32_t nabla = 0x2207;
    constexpr char32_t piproduct = 0x220f;
    constexpr char32_t sigmasum = 0x2211;
    constexpr char32_t sqrt = 0x221a;
    constexpr char32_t cubert = 0x221b;
    constexpr char32_t infinity = 0x221e;
    constexpr char32_t notequal = 0x2260;
    constexpr char32_t almostequal = 0x2248;
    constexpr char32_t asympequal = 0x2243;
    constexpr char32_t approxequal = 0x2245;
    constexpr char32_t degreesign = 0x00b0;
    constexpr char32_t perpendicular = 0x27c2;
    constexpr char32_t parrallelto = 0x2225;
    constexpr char32_t proportionalto = 0x221d;
    constexpr char32_t integral = 0x222b;
    constexpr char32_t doubleintegral = 0x222c;
    constexpr char32_t tripleintegral = 0x222d;
    constexpr char32_t contourintegral = 0x222e;
    constexpr char32_t surfaceintegral = 0x222f;
    constexpr char32_t volumeintegral = 0x2230;

    // Arrers
    constexpr char32_t leftarrow = 0x2190;
    constexpr char32_t uparrow = 0x2191;
    constexpr char32_t rightarrow = 0x2192;
    constexpr char32_t downarrow = 0x2193;
    constexpr char32_t rightarrow2 = 0x1f812;
    constexpr char32_t longrightarrow = 0x27f6;
    constexpr char32_t longleftarrow = 0x27f5;
    constexpr char32_t longleftrightarrow = 0x27f7;
    constexpr char32_t line_emdash = 0x2014;
    constexpr char32_t line_horzbar = 0x2015;

    // Superscripts
    constexpr char32_t ss0 = 0x2070;
    constexpr char32_t ss1 = 0x00b9;
    constexpr char32_t ss2 = 0x00b2;
    constexpr char32_t ss3 = 0x00b3;
    constexpr char32_t ss4 = 0x2074;
    constexpr char32_t ss5 = 0x2075;
    constexpr char32_t ss6 = 0x2076;
    constexpr char32_t ss7 = 0x2077;
    constexpr char32_t ss8 = 0x2078;
    constexpr char32_t ss9 = 0x2079;
    constexpr char32_t ssplus = 0x207a;
    constexpr char32_t ssminus = 0x207b;
    constexpr char32_t ssequals = 0x207c;
    constexpr char32_t ssleftbracket = 0x207d;
    constexpr char32_t ssrightbracket = 0x207e;

    // Subscripts
    constexpr char32_t subs0 = 0x2080;
    constexpr char32_t subs1 = 0x2081;
    constexpr char32_t subs2 = 0x2082;
    constexpr char32_t subs3 = 0x2083;
    constexpr char32_t subs4 = 0x2084;
    constexpr char32_t subs5 = 0x2085;
    constexpr char32_t subs6 = 0x2086;
    constexpr char32_t subs7 = 0x2087;
    constexpr char32_t subs8 = 0x2088;
    constexpr char32_t subs9 = 0x2089;

    constexpr char32_t subsplus = 0x208a;
    constexpr char32_t subsminus = 0x208b;
    constexpr char32_t subsequals = 0x208c;
    constexpr char32_t subsleftbracket = 0x208d;
    constexpr char32_t subsrightbracket = 0x208e;

    // Comparison
    constexpr char32_t lessthaneq = 0x2264;
    constexpr char32_t greaterthaneq = 0x2265;
    constexpr char32_t notlessthan = 0x226e;
    constexpr char32_t notgreaterthan = 0x226f;
    constexpr char32_t lessthanapproxeq = 0x2272;
    constexpr char32_t greaterthanapproxeq = 0x2273;

    //! Convert an input 8 bit string encoded in UTF-8 (or ASCII) format into an
    //! output string of unicode characters.
    std::basic_string<char32_t> fromUtf8 (const std::string& input)
    {
        std::basic_string<char32_t> utxt;

        // Fix this so that it *interprets* _txt as unicode and builds up utxt
        // accordingly. Then I only have to have char32_t here.
        char32_t uc = 0;
        std::int32_t pos = 0; // byte position in uc
        for (std::string::size_type i = 0; i < input.size(); ++i) {
            // Decode UTF-8
            char c = input[i];
            if (c & 0x80) {
                //std::cout << "UTF-8 code. char c is " << static_cast<std::uint64_t>(c) << "\n";
                // Top bit is 1, so interpret and add to c.
                if ((c & 0xc0) == 0xc0) {
                    // Two top bits are 1 so that means start a new unicode character
                    uc = 0;
                    if ((c & 0xf0) == 0xf0) {
                        // 21 bits of data
                        // Place 3 lowest bits of c into uc in the right place
                        uc |= ((c & 0x7) << 18);
                        pos = 2;

                    } else if ((c & 0xe0) == 0xe0) {
                        // 16 bits of data
                        uc = 0;
                        // Place 4 lowest bits of c into uc in the right place
                        uc |= ((c & 0xf) << 12);
                        pos = 1;

                    } else {
                        // 11 bits of data
                        uc = 0;
                        // Place 5 lowest bits of c into uc in the right place
                        uc |= ((c & 0x1f) << 6);
                        pos = 0;
                    }
                } else {
                    // leading code is 10b. Place 6 lowest bits of c into uc in the right place
                    if (pos >= 0) {
                        char32_t code = ( (c & 0x3f) << (pos * 6) );
                        uc |= code;
                        if (pos == 0) {
                            // uc is finished!
                            utxt.push_back (uc);
                        }
                        pos -= 1;
                    } // else error in UTF-8
                }
            } else {
                //std::cout << "1 ASCII char\n";
                pos = 0;
                utxt.push_back (static_cast<char32_t>(input[i]));
            }
        }

        return utxt;
    }

    // Convert the Unicode char32_t c into a std::string containing the
    // corrresponding UTF-8 character code sequence.
    std::string toUtf8 (const char32_t c)
    {
        std::string rtn("");
        if (c < 0x80) {
            rtn.resize (1);
            rtn[0] = (c>>0 & 0x7f)  | 0x00;
        } else if (c < 0x800) {
            rtn.resize (2);
            rtn[0] = (c>>6 & 0x1f)  | 0xc0;
            rtn[1] = (c>>0 & 0x3f)  | 0x80;
        } else if (c < 0x10000) {
            rtn.resize (3);
            rtn[0] = (c>>12 & 0x0f) | 0xe0;
            rtn[1] = (c>>6  & 0x3f) | 0x80;
            rtn[2] = (c>>0  & 0x3f) | 0x80;
        } else if (c < 0x110000) {
            rtn.resize (4);
            rtn[0] = (c>>18 & 0x07) | 0xf0;
            rtn[1] = (c>>12 & 0x3f) | 0x80;
            rtn[2] = (c>>6  & 0x3f) | 0x80;
            rtn[3] = (c>>0  & 0x3f) | 0x80;
        }
        return rtn;
    }

    // Append a Unicode character to the end of string s as a UTF-8 code sequence
    void append (std::string& s, const char32_t c)
    {
        std::string s1 = mplot::unicode::toUtf8(c);
        s += s1;
    }

    // Helper to return the subscript for the given num (expected to be in range 0-9)
    std::string subs (std::int32_t num)
    {
        std::string rtn ("x");
        if (num < 0 || num > 9) { return rtn; }
        rtn = unicode::toUtf8 (unicode::subs0 + (static_cast<char32_t>(num)));
        return rtn;
    }

    // Helper to return the superscript for the given num (expected to be in range 0-9)
    std::string ss (std::int32_t num)
    {
        std::string rtn ("x");
        if (num < 0 || num > 9) { return rtn; }
        // Can't just add num to ss0, like for subscripts, because they're not in order
        if (num == 1) {
            rtn = unicode::toUtf8 (unicode::ss1);
        } else if (num == 2) {
            rtn = unicode::toUtf8 (unicode::ss2);
        } else if (num == 3) {
            rtn = unicode::toUtf8 (unicode::ss3);
        } else { // From ss4 to ss9 and for ss0, we can:
            rtn = unicode::toUtf8 (unicode::ss0 + (static_cast<char32_t>(num)));
        }
        return rtn;
    }
} // namespace mplot::unicode
