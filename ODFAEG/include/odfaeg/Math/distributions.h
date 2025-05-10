/////////////////////////////////////////////////////////////////////////////////
//
// Thor C++ Library
// Copyright (c) 2011-2014 Jan Haller
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////

/// @file
/// @brief Functors to create random distributions of geometric shapes

#ifndef ODFAEG_DISTRIBUTIONS_HPP
#define ODFAEG_DISTRIBUTIONS_HPP

#include "distribution.h"
#include "export.hpp"
#include "maths.h"

#include <SFML/System/Vector3.hpp>
#include <SFML/System/Time.hpp>


namespace odfaeg
{
    namespace math {

        /// @addtogroup Math
        /// @{

        /// @brief Namespace for some predefined distribution functions
        ///
        namespace Distributions
        {

        /// @brief %Uniform random distribution in an int interval
        ///
        Distribution<int> ODFAEG_MATH_API	uniformi(int min, int max);

        /// @brief %Uniform random distribution in an unsigned int interval
        ///
        Distribution<unsigned int> ODFAEG_MATH_API	uniformui(unsigned int min, unsigned int max);

        /// @brief %Uniform random distribution in a float interval
        ///
        Distribution<float> ODFAEG_MATH_API	uniform(float min, float max);

        /// @brief %Uniform random distribution in a time interval
        ///
        Distribution<sf::Time> ODFAEG_MATH_API	uniform(sf::Time min, sf::Time max);

        /// @brief %Uniform random distribution in a rectangle
        ///
        Distribution<sf::Vector3f> ODFAEG_MATH_API	rect(sf::Vector3f center, sf::Vector3f halfSize);

        /// @brief %Uniform random distribution in a circle
        ///
        Distribution<sf::Vector3f> ODFAEG_MATH_API	circle(sf::Vector3f center, float radius);

        /// @brief Vector rotation with a random angle
        ///
        Distribution<sf::Vector3f> ODFAEG_MATH_API	deflect(sf::Vector3f direction, float maxRotation);
        Distribution<sf::Color>   ODFAEG_MATH_API    color(Vec3f color1, Vec3f color2);

        } // namespace Distributions
    }
/// @}

} // namespace thor

#endif // THOR_DISTRIBUTIONS_HPP
