﻿// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

// Own headers
// First the interface, which forces the header to be self-contained.
#include "multispinboxsection.h"
// Second, the private implementation.
#include "multispinboxsection_p.h" // IWYU pragma: associated

#include "constpropagatinguniquepointer.h"
#include "helpermath.h"
#include <qglobal.h>

namespace PerceptualColor
{
/** @brief Constructor
 *
 * The object is initialized with default values. */
MultiSpinBoxSection::MultiSpinBoxSection()
    : d_pointer(new MultiSpinBoxSectionPrivate())
{
}

/** @brief Destructor */
MultiSpinBoxSection::~MultiSpinBoxSection() noexcept
{
}

/** @brief Copy constructor
 *
 * @param other the object to be copied */
MultiSpinBoxSection::MultiSpinBoxSection(const MultiSpinBoxSection &other)
{
    d_pointer.reset(
        // Create a copy of the private implementation object
        new MultiSpinBoxSectionPrivate(*other.d_pointer));
}

/** @brief Copy assignment operator
 *
 * @returns By convention, always returns <tt>*this</tt>.
 *
 * @param other the object to be copied */
MultiSpinBoxSection &MultiSpinBoxSection::operator=(const MultiSpinBoxSection &other)
{
    if (this != &other) { // protect against invalid self-assignment
        d_pointer.reset(
            // Create a copy of the private implementation object
            new MultiSpinBoxSectionPrivate(*other.d_pointer));
    }

    return *this; // By convention, always return *this.
}

/** @brief Move constructor
 *
 * @param other the object to move */
MultiSpinBoxSection::MultiSpinBoxSection(MultiSpinBoxSection &&other) noexcept
{
    d_pointer.swap(other.d_pointer);
}

/** @brief Move assignment operator
 *
 * @returns By convention, always returns <tt>*this</tt>.
 *
 * @param other the object to move-assign */
MultiSpinBoxSection &MultiSpinBoxSection::operator=(MultiSpinBoxSection &&other) noexcept
{
    if (this != &other) { // protect against invalid self-assignment
        d_pointer.swap(other.d_pointer);
    }

    return *this; // By convention, always return *this.
}

/** @brief The number of digits after the decimal point.
 *
 * This value can also be <tt>0</tt> to get integer-like behavior.
 *
 * @returns The property value.
 *
 * @sa @ref setDecimals */
int MultiSpinBoxSection::decimals() const
{
    return d_pointer->m_decimals;
}

/** @brief Setter for @ref decimals property.
 *
 * @param newDecimals The new decimals values. */
void MultiSpinBoxSection::setDecimals(int newDecimals)
{
    d_pointer->m_decimals = qBound(0, newDecimals, 323);
}

/** @brief Holds whether or not @ref MultiSpinBox::sectionValues wrap
 * around when they reaches @ref minimum or @ref maximum.
 *
 * The default is <tt>false</tt>.
 *
 * If <tt>false</tt>, @ref MultiSpinBox::sectionValues shall be bound
 * between @ref minimum and  @ref maximum. If <tt>true</tt>,
 * @ref MultiSpinBox::sectionValues shall be treated as a circular.
 *
 * Example: You have a section that displays a value measured in
 * degree. @ref minimum is <tt>0</tt>. @ref maximum is <tt>360</tt>.
 * The following corrections would be applied to input:
 * | Input | isWrapping == false | isWrapping == true |
 * | ----: | ------------------: | -----------------: |
 * |    -5 |                   0 |                355 |
 * |     0 |                   0 |                  0 |
 * |     5 |                   5 |                  5 |
 * |   355 |                 355 |                355 |
 * |   360 |                 360 |                  0 |
 * |   365 |                 360 |                  5 |
 * |   715 |                 360 |                355 |
 * |   720 |                 360 |                  0 |
 * |   725 |                 360 |                  5 |
 *
 * @returns The property value.
 *
 * @sa @ref setWrapping */
bool MultiSpinBoxSection::isWrapping() const
{
    return d_pointer->m_isWrapping;
}

/** @brief Setter for @ref isWrapping property.
 *
 * @param newIsWrapping The new isWrapping value. */
void MultiSpinBoxSection::setWrapping(bool newIsWrapping)
{
    d_pointer->m_isWrapping = newIsWrapping;
}

/** @brief The maximum possible value of the section.
 *
 * @returns The property value.
 *
 * @sa @ref setMaximum */
double MultiSpinBoxSection::maximum() const
{
    return roundToDigits(d_pointer->m_maximum, d_pointer->m_decimals);
}

/** @brief Setter for @ref maximum property.
 *
 * @param newMaximum The new maximum value. */
void MultiSpinBoxSection::setMaximum(double newMaximum)
{
    d_pointer->m_maximum = newMaximum;
    if (d_pointer->m_minimum > d_pointer->m_maximum) {
        d_pointer->m_minimum = d_pointer->m_maximum;
    }
}

/** @brief The minimum possible value of the section.
 *
 * @returns The property value.
 *
 * @sa @ref setMinimum */
double MultiSpinBoxSection::minimum() const
{
    return roundToDigits(d_pointer->m_minimum, d_pointer->m_decimals);
}

/** @brief Setter for @ref minimum property.
 *
 * @param newMinimum The new minimum value. */
void MultiSpinBoxSection::setMinimum(double newMinimum)
{
    d_pointer->m_minimum = newMinimum;
    if (d_pointer->m_maximum < d_pointer->m_minimum) {
        d_pointer->m_maximum = d_pointer->m_minimum;
    }
}

/** @brief A prefix to be displayed before the value.
 *
 * @returns The property value.
 *
 * @sa @ref setPrefix */
QString MultiSpinBoxSection::prefix() const
{
    return d_pointer->m_prefix;
}

/** @brief Setter for @ref prefix property.
 *
 * @param newPrefix The new prefix value. */
void MultiSpinBoxSection::setPrefix(const QString &newPrefix)
{
    d_pointer->m_prefix = newPrefix;
}

/** @brief A smaller of two natural steps.
 *
 * Valid range: >= 0
 *
 * When the user uses the arrows to change the spin box’s value
 * the value will be incremented/decremented by the amount of the
 * @ref singleStep.
 *
 * @returns The property value.
 *
 * @sa @ref setSingleStep */
double MultiSpinBoxSection::singleStep() const
{
    return d_pointer->m_singleStep;
}

/** @brief Setter for @ref singleStep property.
 *
 * @param newSingleStep The new single step value. */
void MultiSpinBoxSection::setSingleStep(double newSingleStep)
{
    d_pointer->m_singleStep = qMax<double>(0, newSingleStep);
}

/** @brief The suffix to be displayed behind the value.
 *
 * @returns The property value.
 *
 * @sa @ref setSuffix */
QString MultiSpinBoxSection::suffix() const
{
    return d_pointer->m_suffix;
}

/** @brief Setter for @ref suffix property.
 *
 * @param newSuffix The new suffix value. */
void MultiSpinBoxSection::setSuffix(const QString &newSuffix)
{
    d_pointer->m_suffix = newSuffix;
}

/** @brief Adds QDebug() support for data type
 * @ref PerceptualColor::MultiSpinBoxSection
 *
 * @param dbg Existing debug object
 * @param value Value to stream into the debug object
 * @returns Debug object with value streamed in */
QDebug operator<<(QDebug dbg, const PerceptualColor::MultiSpinBoxSection &value)
{
    dbg.nospace() << "\nMultiSpinBoxSection(" // Opening line
                  << "\n    prefix: " << value.prefix() //
                  << "\n    minimum: " << value.minimum() //
                  << "\n    decimals: " << value.decimals() //
                  << "\n    isWrapping: " << value.isWrapping() //
                  << "\n    maximum: " << value.maximum() //
                  << "\n    suffix: " << value.suffix() //
                  << "\n)" // Closing line
        ;
    return dbg.maybeSpace();
}

} // namespace PerceptualColor
