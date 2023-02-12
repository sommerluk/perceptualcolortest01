﻿// SPDX-FileCopyrightText: 2020-2023 Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

// Own headers
// First the interface, which forces the header to be self-contained.
#include "colorpatch.h"
// Second, the private implementation.
#include "colorpatch_p.h" // IWYU pragma: associated

#include "constpropagatinguniquepointer.h"
#include "helper.h"
#include <memory>
#include <qbrush.h>
#include <qevent.h>
#include <qfont.h>
#include <qframe.h>
#include <qimage.h>
#include <qlabel.h>
#include <qmath.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsizepolicy.h>
#include <qstringliteral.h>
#include <qstyle.h>
#include <qstyleoption.h>
class QWidget;

namespace PerceptualColor
{
/** @brief Constructor
 * @param parent The parent of the widget, if any */
ColorPatch::ColorPatch(QWidget *parent)
    : AbstractDiagram(parent)
    , d_pointer(new ColorPatchPrivate(this))
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    d_pointer->updatePixmap();
}

/** @brief Destructor */
ColorPatch::~ColorPatch() noexcept
{
}

/** @brief Constructor
 *
 * @param backLink Pointer to the object from which <em>this</em> object
 * is the private implementation. */
ColorPatchPrivate::ColorPatchPrivate(ColorPatch *backLink)
    : m_label(new QLabel(backLink))
    , q_pointer(backLink)
{
    m_label->setText(QStringLiteral("sdlfj"));
    m_label->setFrameShape(QFrame::StyledPanel);
    m_label->setFrameShadow(QFrame::Sunken);
    m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_label->setGeometry(0, 0, backLink->width(), backLink->height());
    // The following alignment is mirrored by Qt on right-to-left layouts:
    constexpr Qt::Alignment myAlignment{Qt::AlignLeading, Qt::AlignTop};
    m_label->setAlignment(myAlignment);
}

/** @brief Provide the size hint.
 *
 * Reimplemented from base class.
 *
 * @returns the size hint
 *
 * @sa @ref minimumSizeHint() */
QSize ColorPatch::sizeHint() const
{
    return minimumSizeHint();
}

/** @brief Provide the minimum size hint.
 *
 * Reimplemented from base class.
 *
 * @returns the minimum size hint
 *
 * @sa @ref sizeHint() */
QSize ColorPatch::minimumSizeHint() const
{
    // Use a size similar to a QToolButton with an icon (and without text)
    ensurePolished();
    QStyleOptionToolButton option;
    option.initFrom(this);
    option.font = font();
    const int iconSize = style()->pixelMetric( //
        QStyle::PM_ButtonIconSize,
        nullptr,
        this);
    option.iconSize = QSize(iconSize, iconSize);
    return style()->sizeFromContents( //
        QStyle::CT_ToolButton,
        &option,
        option.iconSize,
        this);
}

/** @brief Updates the pixmap in @ref m_label and its alignment. */
void ColorPatchPrivate::updatePixmap()
{
    QPixmap temp = QPixmap::fromImage(renderImage());
    temp.setDevicePixelRatio(q_pointer->devicePixelRatioF());
    m_label->setPixmap(temp);
    // NOTE Kvantum was mistakenly scaling the pixmap (even though
    // QLabel::hasScaledContents() == false) for versions ≤ 1.0.2. This bug
    // has been fixed: https://github.com/tsujan/Kvantum/issues/804.
}

/** @brief Handle resize events.
 *
 * Reimplemented from base class.
 *
 * @param event The corresponding event */
void ColorPatch::resizeEvent(QResizeEvent *event)
{
    d_pointer->m_label->resize(event->size());

    // NOTE It would be more efficient not to always update the pixmap,
    // but only when either the height or the width of the new pixmap to
    // be calculated are larger than those of the current pixmap available
    // under d_pointer->updatePixmap(). After all, a pixmap that is too
    // large does not disturb the drawing, while one that is too small does.
    // Unfortunately, however, resizing QLabel (at least with high-DPI and
    // RTL layout at the same time) causes the correct alignment (here
    // Qt::AlignLeading and Qt::AlignTop) to be lost and the image to be
    // shifted. This error can be worked around by actually each time a new
    // pixmap is assigned, which is not identical to the old one:
    d_pointer->updatePixmap();
}

// No documentation here (documentation of properties
// and its getters are in the header)
QColor ColorPatch::color() const
{
    return d_pointer->m_color;
}

/** @brief Setter for the @ref color property.
 * @param newColor the new color */
void ColorPatch::setColor(const QColor &newColor)
{
    if (newColor != d_pointer->m_color) {
        d_pointer->m_color = newColor;
        d_pointer->updatePixmap();
        Q_EMIT colorChanged(newColor);
    }
}

/** @brief Renders the image to be displayed.
 *
 * @returns An image containing the color of @ref m_color. If the color is
 * transparent or semi-transparent, background with small gray squares is
 * visible. If @ref ColorPatch has RTL layout, the image is mirrored. The
 * device-pixel-ratio is set accordingly to @ref ColorPatch. The size of
 * the image is equal or slightly bigger than necessary to paint the whole
 * @ref ColorPatch surface at the given device-pixel-ratio. As @ref m_label
 * does <em>not</em> scale the image by default, it will be displayed with
 * the correct aspect ratio, while guaranteeing to be big enough whatever
 * QLabel’s frame size is with the currently used QStyle.*/
QImage ColorPatchPrivate::renderImage()
{
    // Initialization
    // Round up to the next integer to be sure to have a big-enough image:
    const auto qLabelContentsRect = m_label->contentsRect();
    const qreal imageWidthF = //
        qLabelContentsRect.width() * q_pointer->devicePixelRatioF();
    const int imageWidth = qCeil(imageWidthF);
    const qreal imageHeightF = //
        qLabelContentsRect.height() * q_pointer->devicePixelRatioF();
    const int imageHeight = qCeil(imageHeightF);
    QImage myImage(imageWidth, //
                   imageHeight, //
                   QImage::Format_ARGB32_Premultiplied);
    if ((imageWidth <= 0) || (imageHeight <= 0)) {
        // Initializing a QPainter on an image of zero size would print
        // errors. Therefore, returning immediately:
        myImage.setDevicePixelRatio(q_pointer->devicePixelRatioF());
        return QImage();
    }
    QStyleOptionFrame opt;
    opt.initFrom(q_pointer); // Sets also QStyle::State_MouseOver if appropriate

    // Draw content of an invalid color (and return)
    if (!m_color.isValid()) {
        const QPalette::ColorGroup myColorGroup = //
            (q_pointer->isEnabled()) //
            ? QPalette::ColorGroup::Normal //
            : QPalette::ColorGroup::Disabled;
        myImage.fill( //
            Qt::transparent
            // An alternative value might be:
            // q_pointer->palette().color(myColorGroup, QPalette::Window)
            // but this integrates less nice with styles like QtCurve who
            // might have background decorations that cover all widgets.
            // Ultimately, however, it is a matter of taste.
        );
        QPen pen( //
            q_pointer->palette().color(myColorGroup, QPalette::WindowText));

        const int defaultFrameWidth = qMax( //
            q_pointer->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt),
            1);
        const auto lineWidthF = //
            defaultFrameWidth * q_pointer->devicePixelRatioF();
        pen.setWidthF(lineWidthF);
        pen.setCapStyle(Qt::PenCapStyle::SquareCap);
        {
            QPainter painter{&myImage};
            // Because Qt::PenCapStyle::SquareCap will extends beyond the line
            // end  by half the line width, we can use an offset and the line
            // will still touch the corner pixels of the image. It is a good
            // idea to do so, because on widgets with an extreme aspect ratio
            // (for example width 400, height 40, which is a realistic value in
            // ColorDialog), the lines seem to “shift out of the image”. Using
            // an offset, it looks nicer. How big should the offset be? To keep
            // it simple, we use the same offset for both, x and y. The
            // distance from the offset point to the point where the line
            // touches the border depends on the angle of the line. The worst
            // case (that means, the biggest distance) is for 45°. With
            // Pythagoras, we have, for the offset “a” (identical for x and y):
            // a² + a² = (½ linewidth)²
            //    2 a² = ¼ linewidth²
            //      a² = ⅛ linewidth²
            //       a = 1 ÷ (√8) linewidth
            //       a ≈ 0.35 linewidth              (Rounding down to be safe)
            const qreal offset = static_cast<qreal>(lineWidthF * 0.35);
            const qreal &left = offset; // alias for “offset”
            const qreal &top = offset; // alias for “offset”
            const qreal bottom = imageHeightF - offset;
            const qreal right = imageWidthF - offset;
            painter.setPen(pen);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.drawLine(QPointF(left, top), //
                             QPointF(right, bottom));
            painter.drawLine(QPointF(left, bottom), //
                             QPointF(right, top));
        }
        myImage.setDevicePixelRatio(q_pointer->devicePixelRatioF());
        return myImage;
    }

    // Draw content of a valid color
    if (m_color.alphaF() < 1) {
        // Prepare the image with (semi-)transparent color
        // Background for colors that are not fully opaque
        QImage tempBackground = transparencyBackground( //
            q_pointer->devicePixelRatioF());
        // Paint the color above
        QPainter(&tempBackground).fillRect(tempBackground.rect(), m_color);
        {
            // Fill a given rectangle with tiles. (QBrush will ignore
            // the devicePixelRatioF of the image of the tile.)
            QPainter painter{&myImage};
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.fillRect(myImage.rect(), QBrush(tempBackground));
        }
        if (q_pointer->layoutDirection() == Qt::RightToLeft) {
            // Horizontally mirrored image for right-to-left layout,
            // so that the “nice” part is the first you see in reading
            // direction.
            myImage = myImage.mirrored(true, // horizontally mirrored
                                       false // vertically mirrored
            );
        }
    } else {
        // Prepare the image with plain color
        myImage.fill(m_color);
    }
    myImage.setDevicePixelRatio(q_pointer->devicePixelRatioF());
    return myImage;
}

} // namespace PerceptualColor
