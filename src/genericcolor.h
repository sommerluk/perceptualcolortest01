// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

#ifndef GENERICCOLOR_H
#define GENERICCOLOR_H

#include "helpermath.h"
#include <lcms2.h>
#include <qdebug.h>

namespace PerceptualColor
{

/** @internal
 *
 * @brief Numeric representation of a color without specifying the color
 * space. */
struct GenericColor {
public:
    /** @brief Default constructor. */
    GenericColor() = default;
    explicit GenericColor(const Trio &init);
    explicit GenericColor(const cmsCIELab &init);
    explicit GenericColor(const cmsCIEXYZ &init);
    GenericColor(const double first, const double second, const double third);
    GenericColor(const double first, const double second, const double third, const double fourth);
    /** @brief First value. */
    double first = 0;
    /** @brief Second value. */
    double second = 0;
    /** @brief Third value. */
    double third = 0;
    /** @brief Forth value. */
    double fourth = 0;

    [[nodiscard]] cmsCIELab reinterpretAsLabToCmscielab() const;
    [[nodiscard]] cmsCIEXYZ reinterpretAsXyzToCmsciexyz() const;
    [[nodiscard]] Trio toTrio() const;
};

QDebug operator<<(QDebug dbg, const PerceptualColor::GenericColor &value);

} // namespace PerceptualColor

#endif // GENERICCOLOR_H
