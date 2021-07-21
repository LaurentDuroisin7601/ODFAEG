
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

#include "../../../include/odfaeg/Math/distributions.h"

#include <cassert>


namespace odfaeg
{
    namespace math {
        namespace Distributions
        {
            namespace
            {
                template <typename T>
                Distribution<T> uniformT(T min, T max)
                {
                    assert(min <= max);

                    return [=] () -> T
                    {
                        return Math::random(min, max);
                    };
                }
            }
            // ---------------------------------------------------------------------------------------------------------------------------


            Distribution<int> uniformi(int min, int max)
            {
                return uniformT(min, max);
            }

            Distribution<unsigned int> uniformui(unsigned int min, unsigned int max)
            {
                return uniformT(min, max);
            }

            Distribution<float> uniform(float min, float max)
            {
                return uniformT(min, max);
            }

            Distribution<sf::Time> uniform(sf::Time min, sf::Time max)
            {
                assert(min <= max);

                const float floatMin = min.asSeconds();
                const float floatMax = max.asSeconds();

                return [=] () -> sf::Time
                {
                    return sf::seconds(Math::random(floatMin, floatMax));
                };
            }

            Distribution<sf::Vector3f> rect(sf::Vector3f center, sf::Vector3f halfSize)
            {
                assert(halfSize.x >= 0.f && halfSize.y >= 0.f);

                return [=] () -> sf::Vector3f
                {
                    return sf::Vector3f(
                    Math::random(center.x - halfSize.x, center.x + halfSize.x),
                    Math::random(center.y - halfSize.y, center.y + halfSize.y),
                    Math::random(center.z - halfSize.z, center.z + halfSize.z));
                };
            }

            Distribution<sf::Vector3f> circle(sf::Vector3f center, float radius)
            {
                assert(radius >= 0.f);

                return [=] () -> sf::Vector3f
                {
                    //sf::Vector3f radiusVector = sf::Vector3f(radius * std::sqrt(Math::random(0.f, 1.f)), Math::random(0.f, 360.f), 0);
                    sf::Vector3f radiusVector = sf::Vector3f(Math::random(0, radius) * Math::cosinus(Math::random(0, Math::toRadians(360))), Math::random(0, radius) * Math::sinus(Math::random(0, Math::toRadians(360))), 0);
                    return center + radiusVector;
                };
            }

            Distribution<sf::Vector3f> deflect(sf::Vector3f direction, float maxRotation)
            {
                return [=] () -> sf::Vector3f
                {
                    Vec3f v(direction.x, direction.y, direction.z);
                    graphic::TransformMatrix tm;
                    tm.setRotation(Vec3f(0, 0, 1), Math::random(0.f - maxRotation, 0.f + maxRotation));
                    Vec3f t = tm.transform(v);
                    return sf::Vector3f(t.x, t.y, t.z);
                };

            }
            Distribution<sf::Color> color(Vec3f color1, Vec3f color2) {
                return [=] () -> sf::Color {
                    sf::Color color;
                    color.r = Math::clamp(Math::random(color1.x, color2.x), 0, 255);
                    color.g = Math::clamp(Math::random(color1.y, color2.y), 0, 255);
                    color.b = Math::clamp(Math::random(color1.z, color2.z), 0, 255);
                    color.a = Math::clamp(Math::random(color1.w, color2.w), 0, 255);
                    return color;
                };
            }
        }

    } // namespace Distributions
} // namespace thor
