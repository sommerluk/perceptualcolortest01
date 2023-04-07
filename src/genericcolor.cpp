// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

// Own header
#include "genericcolor.h"

#include <lcms2.h>

namespace PerceptualColor
{

/** @brief Constructor.
 *
 * @param init Initial value. @ref fourth is set to <tt>0</tt>. */
GenericColor::GenericColor(const Trio &init)
    : first(init(0, 0))
    , second(init(1, 0))
    , third(init(2, 0))
    , fourth(0)
{
}

/** @brief Constructor.
 *
 * @param init Initial value. @ref fourth is set to <tt>0</tt>. */
GenericColor::GenericColor(const cmsCIELab &init)
    : first(init.L)
    , second(init.a)
    , third(init.b)
    , fourth(0)
{
}

/** @brief Constructor.
 *
 * @param init Initial value. @ref fourth is set to <tt>0</tt>. */
GenericColor::GenericColor(const cmsCIEXYZ &init)
    : first(init.X)
    , second(init.Y)
    , third(init.Z)
    , fourth(0)
{
}

/** @brief Constructor.
 *
 * @param v1 Initial value for @ref first
 * @param v2 Initial value for @ref second
 * @param v3 Initial value for @ref third
 *
 * @ref fourth is set to <tt>0</tt>. */
GenericColor::GenericColor(const double v1, const double v2, const double v3)
    : first(v1)
    , second(v2)
    , third(v3)
    , fourth(0)
{
}

/** @brief Constructor.
 *
 * @param v1 Initial value for @ref first
 * @param v2 Initial value for @ref second
 * @param v3 Initial value for @ref third
 * @param v4 Initial value for @ref fourth */
GenericColor::GenericColor(const double v1, const double v2, const double v3, const double v4)
    : first(v1)
    , second(v2)
    , third(v3)
    , fourth(v4)
{
}

/** @brief The values @ref first, @ref second, @ref third as @ref Trio.
 *
 * @returns The values @ref first, @ref second, @ref third as @ref Trio. */
Trio GenericColor::toTrio() const
{
    return createMatrix<1, 3, double>(first, second, third);
}

/** @brief Type conversion.
 *
 * @warning Interprets the current data members as XZY.
 *
 * @returns Type conversion. */
cmsCIEXYZ GenericColor::reinterpretAsXyzToCmsciexyz() const
{
    return cmsCIEXYZ{first, second, third};
}

/** @brief Type conversion.
 *
 * @warning Interprets the current data members as Lab.
 *
 * @returns Type conversion. */
cmsCIELab GenericColor::reinterpretAsLabToCmscielab() const
{
    return cmsCIELab{first, second, third};
}

/** @internal
 *
 * @brief Adds QDebug() support for data type
 * @ref PerceptualColor::GenericColor
 *
 * @param dbg Existing debug object
 * @param value Value to stream into the debug object
 * @returns Debug object with value streamed in */
QDebug operator<<(QDebug dbg, const PerceptualColor::GenericColor &value)
{
    dbg.nospace() //
        << "GenericColor(" << //
        value.first << ", " //
        << value.second << ", " //
        << value.third << ", " //
        << value.fourth << ")";
    return dbg.maybeSpace();
}

} // namespace PerceptualColor
