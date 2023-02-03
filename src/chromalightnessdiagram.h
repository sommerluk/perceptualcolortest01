﻿// SPDX-FileCopyrightText: 2020-2023 Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef CHROMALIGHTNESSDIAGRAM_H
#define CHROMALIGHTNESSDIAGRAM_H

#include "abstractdiagram.h"
#include "constpropagatinguniquepointer.h"
#include "lchdouble.h"
#include <QtCore/qsharedpointer.h>
#include <qglobal.h>
#include <qsize.h>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <qtmetamacros.h>
#else
#include <qobjectdefs.h>
#include <qstring.h>
class QObject;
#endif

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWidget;

namespace PerceptualColor
{
class ChromaLightnessDiagramPrivate;

class RgbColorSpace;

/** @internal
 *
 * @brief A widget that displays a chroma-lightness diagram.
 *
 * This widget displays a chroma-lightness diagram for a given hue.
 *
 * @image html ChromaLightnessDiagram.png "ChromaLightnessDiagram" width=250
 *
 * The widget shows the chroma-lightness diagram at the whole widget extend.
 * - Vertically the lightness from 0 (bottom) to 100 (top).
 * - Horizontally the chroma from 0 (left) to a higher value (right). The same
 *   scale is used like for the vertical axis: So if the widget size is a
 *   square, both chroma and lightness range from 0 to 100. If the widget
 *   width is twice the height, the lightness ranges from 0 to 100
 *   and the chroma ranges from 0 to 200.
 *
 * @internal
 *
 * @note This class is not part of the public API because its interface
 * is not polished enough. Notably it does not automatically scale the
 * diagram to fit a given gamut (means: to fit up to a given maximum
 * chroma). Even if we would fix this: We would need a public API
 * that is widthForHeight-dependent to allow the library user to
 * comfortably make use of this!
 *
 * @todo What to do if a gamut allows lightness < 0 or lightness > 100 ???
 * What if a part of the gamut at the right is not displayed? (Thought
 * this means that @ref RgbColorSpace has a bug.) Shouldn’t this be
 * controlled?) Maybe it would be better to control this
 * within @ref RgbColorSpace … */
class ChromaLightnessDiagram : public AbstractDiagram
{
    Q_OBJECT

    /** @brief Currently selected color
     *
     * The widget allows the user to change the LCH chroma and the
     * LCH lightness values. However, the LCH hue value cannot be
     * changed by the user, but only by the programmer through this property.
     *
     * @sa READ @ref currentColor() const
     * @sa WRITE @ref setCurrentColor()
     * @sa NOTIFY @ref currentColorChanged() */
    Q_PROPERTY(PerceptualColor::LchDouble currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged)

public:
    Q_INVOKABLE explicit ChromaLightnessDiagram(const QSharedPointer<PerceptualColor::RgbColorSpace> &colorSpace, QWidget *parent = nullptr);
    virtual ~ChromaLightnessDiagram() noexcept override;
    /** @brief Getter for property @ref currentColor
     *  @returns the property @ref currentColor */
    [[nodiscard]] PerceptualColor::LchDouble currentColor() const;
    [[nodiscard]] virtual QSize minimumSizeHint() const override;
    [[nodiscard]] virtual QSize sizeHint() const override;

public Q_SLOTS:
    void setCurrentColor(const PerceptualColor::LchDouble &newCurrentColor);

Q_SIGNALS:
    /** @brief Notify signal for property @ref currentColor.
     *  @param newCurrentColor the new current color */
    void currentColorChanged(const PerceptualColor::LchDouble &newCurrentColor);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(ChromaLightnessDiagram)

    /** @internal
     *
     * @brief Declare the private implementation as friend class.
     *
     * This allows the private class to access the protected members and
     * functions of instances of <em>this</em> class. */
    friend class ChromaLightnessDiagramPrivate;
    /** @brief Pointer to implementation (pimpl) */
    ConstPropagatingUniquePointer<ChromaLightnessDiagramPrivate> d_pointer;

    /** @internal @brief Only for unit tests. */
    friend class TestChromaLightnessDiagram;

    /** @internal
     * @brief Internal friend declaration.
     *
     * This class is used as child class in @ref WheelColorPicker.
     * There is a tight collaboration. */
    friend class WheelColorPicker;
    /** @internal
     * @brief Internal friend declaration.
     *
     * This class is used as child class in @ref WheelColorPicker.
     * There is a tight collaboration. */
    friend class WheelColorPickerPrivate;
};

} // namespace PerceptualColor

#endif // CHROMALIGHTNESSDIAGRAM_H
