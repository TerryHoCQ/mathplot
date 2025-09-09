#pragma once

/*!
 * This header defines a simple major/minor version for mathplot.
 *
 * \author Seb James
 * \date April 2024
 */

#include <string>

namespace mplot {

    //! A version definition for the whole of mathplot
    static constexpr unsigned int version_major = 1;
    static constexpr unsigned int version_minor = 0;
    //! Returns a string for the version of the mathplot library
    std::string version_string()
    {
        std::string vers = std::to_string (mplot::version_major) + std::string(".") + std::to_string (mplot::version_minor);
        return vers;
    }

} // namespace mplot
