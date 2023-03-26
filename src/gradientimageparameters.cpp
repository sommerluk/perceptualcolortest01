﻿// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

// Own headers
// First the interface, which forces the header to be self-contained.
#include "gradientimageparameters.h"

#include "asyncimagerendercallback.h"
#include "helper.h"
#include "helperqttypes.h"
#include "lchadouble.h"
#include "lchdouble.h"
#include "rgbcolorspace.h"
#include <cmath>
#include <qbrush.h>
#include <qcolor.h>
#include <qimage.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qsharedpointer.h>

namespace PerceptualColor
{
/** @brief Constructor */
GradientImageParameters::GradientImageParameters()
{
    setFirstColor(LchaDouble{0, 0, 0, 1});
    setFirstColor(LchaDouble{1000, 0, 0, 1});
}

/** @brief Normalizes the value and bounds it to the LCH color space.
 * @param color the color that should be treated.
 * @returns A normalized and bounded version. If the chroma was negative,
 * it gets positive (which implies turning the hue by 180°). The hue is
 * normalized to the range <tt>[0°, 360°[</tt>. Lightness is bounded to the
 * range <tt>[0, 100]</tt>. Alpha is bounded to the range <tt>[0, 1]</tt>. */
LchaDouble GradientImageParameters::completlyNormalizedAndBounded(const LchaDouble &color)
{
    LchaDouble result;
    if (color.c < 0) {
        result.c = color.c * (-1);
        result.h = fmod(color.h + 180, 360);
    } else {
        result.c = color.c;
        result.h = fmod(color.h, 360);
    }
    if (result.h < 0) {
        result.h += 360;
    }
    result.l = qBound<qreal>(0, color.l, 100);
    result.a = qBound<qreal>(0, color.a, 1);
    return result;
}

/** @brief Setter for the first color property.
 * @param newFirstColor The new first color.
 * @sa @ref m_firstColorCorrected */
void GradientImageParameters::setFirstColor(const LchaDouble &newFirstColor)
{
    LchaDouble correctedNewFirstColor = //
        completlyNormalizedAndBounded(newFirstColor);
    if (!m_firstColorCorrected.hasSameCoordinates(correctedNewFirstColor)) {
        m_firstColorCorrected = correctedNewFirstColor;
        updateSecondColor();
        // Free the memory used by the old image.
        m_image = QImage();
    }
}

/** @brief Setter for the second color property.
 * @param newSecondColor The new second color.
 * @sa @ref m_secondColorCorrectedAndAltered */
void GradientImageParameters::setSecondColor(const LchaDouble &newSecondColor)
{
    LchaDouble correctedNewSecondColor = //
        completlyNormalizedAndBounded(newSecondColor);
    if (!m_secondColorCorrectedAndAltered.hasSameCoordinates(correctedNewSecondColor)) {
        m_secondColorCorrectedAndAltered = correctedNewSecondColor;
        updateSecondColor();
        // Free the memory used by the old image.
        m_image = QImage();
    }
}

/** @brief Updates @ref m_secondColorCorrectedAndAltered
 *
 * This update takes into account the current values of
 * @ref m_firstColorCorrected and @ref m_secondColorCorrectedAndAltered. */
void GradientImageParameters::updateSecondColor()
{
    m_secondColorCorrectedAndAltered = //
        completlyNormalizedAndBounded(m_secondColorCorrectedAndAltered);
    if (qAbs(m_firstColorCorrected.h - m_secondColorCorrectedAndAltered.h) > 180) {
        if (m_firstColorCorrected.h > m_secondColorCorrectedAndAltered.h) {
            m_secondColorCorrectedAndAltered.h += 360;
        } else {
            m_secondColorCorrectedAndAltered.h -= 360;
        }
    }
}

/** @brief Render an image.
 *
 * The function will render the image with the given parameters,
 * and deliver the result by means of <tt>callbackObject</tt>.
 *
 * This function is thread-safe as long as each call of this function
 * uses different <tt>variantParameters</tt> and <tt>callbackObject</tt>.
 *
 * @param variantParameters A <tt>QVariant</tt> that contains the
 *        image parameters.
 * @param callbackObject Pointer to the object for the callbacks.
 *
 * @todo Could we get better performance? Even online tools like
 * https://bottosson.github.io/misc/colorpicker/#ff2a00 or
 * https://oklch.evilmartians.io/#65.4,0.136,146.7,100 get quite good
 * performance. How do they do that? */
void GradientImageParameters::render(const QVariant &variantParameters, AsyncImageRenderCallback &callbackObject)
{
    if (!variantParameters.canConvert<GradientImageParameters>()) {
        return;
    }
    const GradientImageParameters parameters = //
        variantParameters.value<GradientImageParameters>();
    if (parameters.rgbColorSpace.isNull()) {
        return;
    }

    // From Qt Example’s documentation:
    //
    //     “If we discover […] that restart has been set
    //      to true (by render()), we break out […] immediately […].
    //      Similarly, if we discover that abort has been set
    //      to true (by the […] destructor), we return from the
    //      function immediately […].”
    if (callbackObject.shouldAbort()) {
        return;
    }

    // First, create an image of the gradient with only one pixel thickness.
    // (Color management operations are expensive in CPU time; we try to
    // minimize this.)
    QImage onePixelLine(parameters.m_gradientLength, //
                        1, //
                        QImage::Format_ARGB32_Premultiplied);
    onePixelLine.fill(Qt::transparent); // Initialize image with transparency.
    LchaDouble color;
    LchDouble cielchD50;
    QColor temp;
    for (int i = 0; i < parameters.m_gradientLength; ++i) {
        color = parameters.colorFromValue( //
            (i + 0.5) / static_cast<qreal>(parameters.m_gradientLength));
        cielchD50.l = color.l;
        cielchD50.c = color.c;
        cielchD50.h = color.h;
        temp = parameters.rgbColorSpace->fromCielchD50ToQRgbBound(cielchD50);
        temp.setAlphaF(
            // Reduce floating point precision if necessary.
            static_cast<QColorFloatType>(color.a));
        onePixelLine.setPixelColor(i, 0, temp);
    }
    if (callbackObject.shouldAbort()) {
        return;
    }

    // Now, create a full image of the gradient
    QImage result = QImage(parameters.m_gradientLength, //
                           parameters.m_gradientThickness, //
                           QImage::Format_ARGB32_Premultiplied);
    if (result.isNull()) {
        // Make sure that no QPainter can be created on a null image
        // (because this would trigger warning messages on the command
        // line).
        return;
    }
    QPainter painter(&result);

    // Transparency background
    if ( //
        (parameters.m_firstColorCorrected.a != 1) //
        || (parameters.m_secondColorCorrectedAndAltered.a != 1) //
    ) {
        // Fill the image with tiles. (QBrush will ignore
        // the devicePixelRatioF of the image of the tile.)
        const auto background = transparencyBackground( //
            parameters.m_devicePixelRatioF);
        painter.fillRect(0, //
                         0, //
                         parameters.m_gradientLength, //
                         parameters.m_gradientThickness, //
                         QBrush(background));
    }

    // Paint the gradient itself.
    for (int i = 0; i < parameters.m_gradientThickness; ++i) {
        painter.drawImage(0, i, onePixelLine);
    }

    result.setDevicePixelRatio(parameters.m_devicePixelRatioF);

    if (callbackObject.shouldAbort()) {
        return;
    }

    callbackObject.deliverInterlacingPass( //
        result, //
        variantParameters, //
        AsyncImageRenderCallback::InterlacingState::Final);
}

/** @brief The color that the gradient has at a given position of the gradient.
 * @param value The position. Valid range: <tt>[0.0, 1.0]</tt>. <tt>0.0</tt>
 * means the first color, <tt>1.0</tt> means the second color, and everything
 * in between means a color in between.
 * @returns If the position is valid: The color at the given position and
 * its corresponding alpha value. If the position is out-of-range: An
 * arbitrary value. */
LchaDouble GradientImageParameters::colorFromValue(qreal value) const
{
    LchaDouble color;
    color.l = m_firstColorCorrected.l //
        + (m_secondColorCorrectedAndAltered.l - m_firstColorCorrected.l) * value;
    color.c = m_firstColorCorrected.c + //
        (m_secondColorCorrectedAndAltered.c - m_firstColorCorrected.c) * value;
    color.h = m_firstColorCorrected.h + //
        (m_secondColorCorrectedAndAltered.h - m_firstColorCorrected.h) * value;
    color.a = m_firstColorCorrected.a + //
        (m_secondColorCorrectedAndAltered.a - m_firstColorCorrected.a) * value;
    return color;
}

/** @brief Setter for the device pixel ratio (floating point).
 *
 * This value is set as device pixel ratio (floating point) in the
 * <tt>QImage</tt> that this class holds. It does <em>not</em> change
 * the <em>pixel</em> size of the image or the pixel size of wheel
 * thickness or border.
 *
 * This is for HiDPI support. You can set this to
 * <tt>QWidget::devicePixelRatioF()</tt> to get HiDPI images in the correct
 * resolution for your widgets. Within a method of a class derived
 * from <tt>QWidget</tt>, you could write:
 *
 * @snippet testgradientimageparameters.cpp GradientImage HiDPI usage
 *
 * The default value is <tt>1</tt> which means no special scaling.
 *
 * @param newDevicePixelRatioF the new device pixel ratio as a
 * floating point data type. (Values smaller than <tt>1.0</tt> will be
 * considered as <tt>1.0</tt>.) */
void GradientImageParameters::setDevicePixelRatioF(const qreal newDevicePixelRatioF)
{
    const qreal tempDevicePixelRatioF = qMax<qreal>(1, newDevicePixelRatioF);
    if (m_devicePixelRatioF != tempDevicePixelRatioF) {
        m_devicePixelRatioF = tempDevicePixelRatioF;
        // Free the memory used by the old image.
        m_image = QImage();
    }
}

/** @brief Setter for the gradient length property.
 *
 * @param newGradientLength The new gradient length, measured
 * in <em>physical pixels</em>. */
void GradientImageParameters::setGradientLength(const int newGradientLength)
{
    const int temp = qMax(0, newGradientLength);
    if (m_gradientLength != temp) {
        m_gradientLength = temp;
        // Free the memory used by the old image.
        m_image = QImage();
    }
}

/** @brief Setter for the gradient thickness property.
 *
 * @param newGradientThickness The new gradient thickness, measured
 * in <em>physical pixels</em>. */
void GradientImageParameters::setGradientThickness(const int newGradientThickness)
{
    const int temp = qMax(0, newGradientThickness);
    if (m_gradientThickness != temp) {
        m_gradientThickness = temp;
        // Free the memory used by the old image.
        m_image = QImage();
    }
}

/** @brief Equal operator
 *
 * @param other The object to compare with.
 *
 * @returns <tt>true</tt> if equal, <tt>false</tt> otherwise. */
bool GradientImageParameters::operator==(const GradientImageParameters &other) const
{
    return ( //
        (m_devicePixelRatioF == other.m_devicePixelRatioF) //
        && (m_firstColorCorrected.l == other.m_firstColorCorrected.l) //
        && (m_firstColorCorrected.c == other.m_firstColorCorrected.c) //
        && (m_firstColorCorrected.h == other.m_firstColorCorrected.h) //
        && (m_firstColorCorrected.a == other.m_firstColorCorrected.a) //
        && (m_gradientLength == other.m_gradientLength) //
        && (m_gradientThickness == other.m_gradientThickness) //
        && (rgbColorSpace == other.rgbColorSpace) //
        && (m_secondColorCorrectedAndAltered.l == other.m_secondColorCorrectedAndAltered.l) //
        && (m_secondColorCorrectedAndAltered.c == other.m_secondColorCorrectedAndAltered.c) //
        && (m_secondColorCorrectedAndAltered.h == other.m_secondColorCorrectedAndAltered.h) //
        && (m_secondColorCorrectedAndAltered.a == other.m_secondColorCorrectedAndAltered.a) //
    );
}

/** @brief Unequal operator
 *
 * @param other The object to compare with.
 *
 * @returns <tt>true</tt> if unequal, <tt>false</tt> otherwise. */
bool GradientImageParameters::operator!=(const GradientImageParameters &other) const
{
    return !(*this == other);
}

} // namespace PerceptualColor
