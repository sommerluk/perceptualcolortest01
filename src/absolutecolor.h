// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

#ifndef ABSOLUTECOLOR_H
#define ABSOLUTECOLOR_H

#include "genericcolor.h"
#include "helperconversion.h"
#include <array>
#include <qglobal.h>
#include <qhash.h>
#include <qlist.h>
#include <qobject.h>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <qtmetamacros.h>
#else
#include <qobjectdefs.h>
#include <qstring.h>
#endif

namespace PerceptualColor
{

/** @internal
 *
 * @brief Toolbox for color conversions. */
class AbsoluteColor final
{
public:
    AbsoluteColor() = delete;

    static QHash<ColorSpace, GenericColor> allConversions(const ColorSpace space, const GenericColor &value);

private:
    /** @brief Function pointer type for the conversion functions. */
    // NOTE std::function<> has nicer syntax for function pointers, but does
    // not allow constexpr.
    using ConversionFunction = GenericColor (*)(const GenericColor &);

    /** @brief Gives access to a conversion function. */
    struct Conversion {
    public:
        /** @brief The color space from which the function converts. */
        ColorSpace from;
        /** @brief The color space to which the function converts. */
        ColorSpace to;
        /** @brief The function. */
        ConversionFunction conversionFunction;
    };

    static GenericColor fromXyzD50ToXyzD65(const GenericColor &value);
    static GenericColor fromXyzD65ToXyzD50(const GenericColor &value);
    static GenericColor fromXyzD65ToOklab(const GenericColor &value);
    static GenericColor fromOklabToXyzD65(const GenericColor &value);
    static GenericColor fromXyzD50ToCielabD50(const GenericColor &value);
    static GenericColor fromCielabD50ToXyzD50(const GenericColor &value);
    static GenericColor fromPolarToCartesian(const GenericColor &value);
    static GenericColor fromCartesianToPolar(const GenericColor &value);

    /** @brief List of all conversion accesses. */
    static constexpr std::array<Conversion, 10> conversionList //
        {{{ColorSpace::XyzD50, ColorSpace::XyzD65, fromXyzD50ToXyzD65},
          {ColorSpace::XyzD65, ColorSpace::XyzD50, fromXyzD65ToXyzD50},
          {ColorSpace::OklabD65, ColorSpace::XyzD65, fromOklabToXyzD65},
          {ColorSpace::XyzD65, ColorSpace::OklabD65, fromXyzD65ToOklab},
          {ColorSpace::XyzD50, ColorSpace::CielabD50, fromXyzD50ToCielabD50},
          {ColorSpace::CielabD50, ColorSpace::XyzD50, fromCielabD50ToXyzD50},
          {ColorSpace::CielchD50, ColorSpace::CielabD50, fromPolarToCartesian},
          {ColorSpace::OklchD65, ColorSpace::OklabD65, fromPolarToCartesian},
          {ColorSpace::CielabD50, ColorSpace::CielchD50, fromCartesianToPolar},
          {ColorSpace::OklabD65, ColorSpace::OklchD65, fromCartesianToPolar}}};

    static QList<AbsoluteColor::Conversion> conversionsFrom(const ColorSpace space);

    static void addDirectConversionsRecursivly(QHash<ColorSpace, GenericColor> *values, const ColorSpace space);
};

} // namespace PerceptualColor

#endif // ABSOLUTECOLOR_H
