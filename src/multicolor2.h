// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

#ifndef MULTICOLOR2_H
#define MULTICOLOR2_H

#include "genericcolor.h"
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
// TODO xxx Inherit from QObject: Necessary for Q_ENUM. Can be removed if the enum is outside the class.
class MultiColor2 final : QObject
{
    // TODO xxx Q_OBJECT macro: Necessary for Q_ENUM. Can be removed if the enum is outside the class.
    Q_OBJECT

public:
    MultiColor2() = delete;
    /** @brief Identifiers for color spaces */
    enum class ColorSpace {
        CielabD50, /**< Cielab color space using a D50 illuminant.
            Lightness: [0, 100].<br/>
            a: unbound.<br/>
            b: unbound. */
        CielchD50, /**< Cielch color space using a D50 illuminant.
            Lightness: [0, 100].<br/>
            Chroma: unbound.<br/>
            Hue: [0, 360[. */
        XyzD50, /**< Xyz color space using a D50 illuminant.
            X: unbound.<br/>
            Y: [0, 1].<br/>
            Z: unbound. */
        XyzD65, /**< Xzy color space using a D65 illuminant.
            X: unbound.<br/>
            Y: [0, 1].<br/>
            Z: unbound. */
        OklabD65, /**< Oklab color space, which by definition always and
            exclusively uses a D65 illuminant.

            Lightness: [0, 1].<br/>
            a: unbound.<br/>
            b: unbound. */
        OklchD65 /**< Oklch color space, which by definition always and
            exclusively uses a D65 illuminant.

            Lightness: [0, 1].<br/>
            Chroma: unbound.<br/>
            Hue: [0, 360[. */
    };
    Q_ENUM(ColorSpace)

    static QHash<MultiColor2::ColorSpace, GenericColor> allConversions(const MultiColor2::ColorSpace space, const GenericColor &value);

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

    static QList<MultiColor2::Conversion> conversionsFrom(const ColorSpace space);

    static void addDirectConversionsRecursivly(QHash<MultiColor2::ColorSpace, GenericColor> *values, const ColorSpace space);
};

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
uint qHash(const MultiColor2::ColorSpace t, uint seed = 0); // clazy:exclude=qt6-qhash-signature
#endif

} // namespace PerceptualColor

#endif // MULTICOLOR2_H
