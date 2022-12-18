﻿// SPDX-FileCopyrightText: 2020-2023 Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: MIT

#include "PerceptualColor/chromahuediagram.h"
#include "PerceptualColor/colordialog.h"
#include "PerceptualColor/colorpatch.h"
#include "PerceptualColor/colorwheel.h"
#include "PerceptualColor/constpropagatinguniquepointer.h"
#include "PerceptualColor/gradientslider.h"
#include "PerceptualColor/lchdouble.h"
#include "PerceptualColor/multispinbox.h"
#include "PerceptualColor/multispinboxsectionconfiguration.h"
#include "PerceptualColor/rgbcolorspacefactory.h"
#include "PerceptualColor/wheelcolorpicker.h"
#include "chromalightnessdiagram.h"
#include "palettewidget.h"
#include "refreshiconengine.h"
#include "rgbcolorspace.h"
#include <PerceptualColor/settranslation.h>
#include <QtCore/qsharedpointer.h>
#include <helper.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcolor.h>
#include <qcoreapplication.h>
#include <qfont.h>
#include <qglobal.h>
#include <qicon.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qlocale.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qscopedpointer.h>
#include <qstring.h>
#include <qstringbuilder.h>
#include <qstringlist.h>
#include <qstringliteral.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <stdlib.h>

using namespace PerceptualColor;

static void screenshot(QWidget *widget, const QString &comment = QString())
{
    // Get fully qualified class name
    QString className = QString::fromUtf8(widget->metaObject()->className());
    // Strip all the qualifiers
    className = className.split(QStringLiteral("::")).last();
    widget->grab().save(
        // File name:
        className + comment + QStringLiteral(".png"),
        // File format: nullptr means: The file format will be chosen
        // from file name’s suffix.
        nullptr,
        // Compression:
        // 0 means: The compression is slow and results in a small file size.
        // 100 means: The compression is fast and results in a big file size.
        0);
}

// Screenshots of widgets with asynchronous image processing.
//
// This function is not deterministic! If the delays are enough to
// get the full-quality screenshot depends on the speed of your
// hardware and on how much other applications are running on
// your system!
static void screenshotDelayed(QWidget *widget, const QString &comment = QString())
{
    widget->show(); // Necessary to receive and process events like paintEvent()
    delayedEventProcessing<>();
    screenshot(widget, comment);
    widget->hide();
}

// A message handler that does not print any messages.
static void voidMessageHandler(QtMsgType, const QMessageLogContext &, const QString &)
{
    // dummy message handler that does not print messages
}

// Sets many settings that have visual effects on hard-coded values.
//
// This function tries to set as many settings as possible to hard-coded
// values: The widget style, the translation, the icon set and many more.
// This makes it more likely to get the same screenshots on different
// computers with different settings.
static void initializeHardCodeWidgetAppearance(QApplication *app)
{
    QStyle *style = nullptr;
    // We prefer the Fusion style because he is the most cross-platform
    // style, so generating the screenshots does not depend on the
    // current system. Furthermore, it has support for fraction
    // scale factors such as 1.25 or 1.5.
    //
    // Possible styles (not all available in all setups):
    // "Breeze", "dsemilight", "dsemidark", "dlight", "ddark", "kvantum-dark",
    // "kvantum", "cleanlooks", "gtk2", "cde", "motif", "plastique", "Oxygen",
    // "QtCurve", "Windows", "Fusion"
    if (style == nullptr) {
        style = QStyleFactory::create(QStringLiteral("Fusion"));
    }
    if (style == nullptr) {
        style = QStyleFactory::create(QStringLiteral("Breeze"));
    }
    if (style == nullptr) {
        style = QStyleFactory::create(QStringLiteral("Oxygen"));
    }
    QApplication::setStyle(style); // This call is safe even if style==nullptr.

    // Fusion uses by default the system’s palette, but
    // we want something system-independent to make the screenshots
    // look always the same. Therefore, we explicitly set Fusion’s
    // standard palette.
    {
        QScopedPointer<QStyle> tempStyle( //
            QStyleFactory::create(QStringLiteral("Fusion")));
        QPalette tempPalette = tempStyle->standardPalette();
        // The following colors are missing in Fusion’s standard palette:
        // They appear in Qt’s documentation of QPalette::ColorRole,
        // but do not appear when passing Fusion’s standard palette to
        // qDebug(). Therefore, we set them explicitly to the default values
        // that are mentioned in the documentation of QPalette::ColorRole.
        tempPalette.setColor(QPalette::Link, Qt::blue);
        tempPalette.setColor(QPalette::Link, Qt::magenta);
        QApplication::setPalette(tempPalette);
    }

    // By default, the icons of the system are made available by
    // QPlatformTheme. However, we want to make screenshots that
    // are independent of the currently selected icon theme on
    // the computer that produces the screenshots. Therefore, we
    // choose an invalid search path for icon themes to avoid
    // that missing icons are found in other themes that are
    // available on the current computer:
    QIcon::setThemeSearchPaths(QStringList(QStringLiteral("invalid")));
    // Now, we change the standard icon theme to an invalid value.
    // As the search path has also been set to an invalid, missing
    // icons cannot be replaced by fallback icons.
    QIcon::setThemeName(QStringLiteral("invalid"));
    qInstallMessageHandler(voidMessageHandler); // Suppress warnings
    {
        // Trigger a call to the new, invalid icon theme. This call
        // will produce a message on the console:
        //     “Icon theme "invalid" not found.”
        // Here, we trigger it intentionally while having the message
        // suppressed. The message appears only at the first call
        // to the invalid icon theme, but not on subsequent calls.
        // Therefore, we will not get more messages for this in the
        // rest of this program.
        QWidget widget;
        widget.repaint();
        QCoreApplication::processEvents();
    }
    qInstallMessageHandler(nullptr); // Do not suppress warnings anymore

    { // Try to be as explicit as possible about the fonts.
        // TODO It might even be possible to bundle a font as Qt resource
        //      to become completely independent from the fonts that are
        //      installed on the system.
        QFont myFont = QFont(QStringLiteral("Noto Sans"), //
                             10, //
                             QFont::Weight::Normal, //
                             QFont::Style::StyleNormal);
        // NOTE The font size is defined in “point”, whatever “point” is.
        // Actually, the size of a “point” depends on the scale factor,
        // which is set elsewhere yet. So, when the scale factor is
        // correct, than using a fixed “point” size should give us
        // identical results also on different systems.
        myFont.setStyleHint(QFont::SansSerif, //
                            QFont::StyleStrategy::PreferDefault);
        myFont.setFamilies( //
            QStringList() //
            << QStringLiteral("Noto Sans") //
            << QStringLiteral("Noto Sans Symbols2"));
        app->setFont(myFont);
    }

    // Other initializations
    app->setApplicationName(QStringLiteral("Perceptual color picker"));
    app->setLayoutDirection(Qt::LeftToRight);
    QLocale::setDefault(QLocale::English);
    PerceptualColor::setTranslation(app, //
                                    QLocale(QLocale::English).uiLanguages());
}

// Creates a set of screenshots of the library and saves these
// screenshots as .png files in the working directory.
int main(int argc, char *argv[])
{
    // Adjust the scale factor before constructing our real QApplication
    // object:
    {
        // See https://doc.qt.io/qt-6/highdpi.html for documentation
        // about QT_SCALE_FACTOR. In short: For testing purpose, it
        // can be used to adjust the current system-default scale
        // factor. This affects both, widget painting and font
        // rendering (font DPI).
        //
        // We choose a small factor, because the actual default size
        // of dialog and top-level widgets in Qt is: smaller or
        // than ⅔ of the screen. This affects our color dialog, which
        // allows small sizes, but recommends bigger ones. As the
        // screen size of the computer running this program is not
        // known in advance, we try to minimize the effects be choosing
        // the smallest possible scale factor, which is 1. (Values
        // smaller than 1 are working: They break the layout.)
        constexpr qreal screenshotScaleFactor = 1;
        // Create a temporary QApplication object within this block scope.
        // Necessary to get the system’s scale factor.
        QApplication app(argc, argv);
        const qreal systemScaleFactor = QWidget().devicePixelRatioF();
        bool conversionOkay;
        double qtScaleFactor = //
            qEnvironmentVariable("QT_SCALE_FACTOR").toDouble(&conversionOkay);
        if (!conversionOkay) {
            qtScaleFactor = 1;
        }
        qtScaleFactor = //
            qtScaleFactor / systemScaleFactor * screenshotScaleFactor;
        // Set QT_SCALE_FACTOR to a corrected factor. This will only
        // take effect when the current QApplication object has been
        // destroyed and a new one has been created.
        qputenv("QT_SCALE_FACTOR", QString::number(qtScaleFactor).toUtf8());
    }

    // Prepare configuration before instantiating the application object
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // Instantiate the application object
    QApplication app(argc, argv);
    initializeHardCodeWidgetAppearance(&app);

    // Variables
    QSharedPointer<RgbColorSpace> m_colorSpace = //
        RgbColorSpaceFactory::createSrgb();
    // Chose a default color:
    // — that is present in the palette (to show the selection mark)
    // — is quite chromatic (which looks nice on screenshots)
    // — has nevertheless a little bit of distance to the outer
    //   hull (which  puts the marker somewhere in the inner of
    //   the gamut, which makes the screenshots easier to understand).
    const QColor defaultColorRgb = QColor::fromRgb(245, 194, 17);
    const LchDouble defaultColorLch = //
        m_colorSpace->toCielchDouble(defaultColorRgb.rgba64());
    QColor myColor;

    {
        ChromaHueDiagram m_chromaHueDiagram(m_colorSpace);
        m_chromaHueDiagram.setCurrentColor(defaultColorLch);
        screenshotDelayed(&m_chromaHueDiagram);
    }

    {
        ChromaLightnessDiagram m_chromaLightnessDiagram(m_colorSpace);
        m_chromaLightnessDiagram.setCurrentColor(defaultColorLch);
        screenshotDelayed(&m_chromaLightnessDiagram);
    }

    {
        ColorDialog m_colorDialog(m_colorSpace);
        m_colorDialog.setLayoutDimensions( //
            ColorDialog::DialogLayoutDimensions::Expanded);
        m_colorDialog.setCurrentColor(defaultColorRgb);
        screenshotDelayed(&m_colorDialog);
    }

    {
        ColorDialog m_colorDialog(m_colorSpace);
        m_colorDialog.setLayoutDimensions( //
            ColorDialog::DialogLayoutDimensions::Expanded);
        m_colorDialog.setCurrentColor(defaultColorRgb);
        QTabWidget *myTabWidget = m_colorDialog.findChild<QTabWidget *>();
        myTabWidget->setCurrentIndex(1);
        screenshotDelayed(&m_colorDialog, QStringLiteral("Tab1"));
    }

    {
        ColorDialog m_colorDialog(m_colorSpace);
        m_colorDialog.setLayoutDimensions( //
            ColorDialog::DialogLayoutDimensions::Expanded);
        m_colorDialog.setCurrentColor(defaultColorRgb);
        QTabWidget *myTabWidget = m_colorDialog.findChild<QTabWidget *>();
        myTabWidget->setCurrentIndex(2);
        screenshotDelayed(&m_colorDialog, QStringLiteral("Tab2"));
    }

    {
        ColorDialog m_colorDialog(m_colorSpace);
        m_colorDialog.setLayoutDimensions( //
            ColorDialog::DialogLayoutDimensions::Expanded);
        m_colorDialog.setCurrentColor(defaultColorRgb);
        QTabWidget *myTabWidget = m_colorDialog.findChild<QTabWidget *>();
        myTabWidget->setCurrentIndex(1);
        m_colorDialog.setOption( //
            ColorDialog::ColorDialogOption::ShowAlphaChannel);
        myColor = m_colorDialog.currentColor();
        myColor.setAlphaF(0.5);
        m_colorDialog.setCurrentColor(myColor);
        screenshotDelayed(&m_colorDialog, QStringLiteral("Alpha"));
    }

    {
        ColorDialog m_colorDialog(m_colorSpace);
        m_colorDialog.setLayoutDimensions( //
            ColorDialog::DialogLayoutDimensions::Expanded);
        m_colorDialog.setCurrentColor(defaultColorRgb);
        QTabWidget *myTabWidget = m_colorDialog.findChild<QTabWidget *>();
        myTabWidget->setCurrentIndex(1);
        m_colorDialog.setOption( //
            ColorDialog::ColorDialogOption::ShowAlphaChannel);
        myColor = m_colorDialog.currentColor();
        myColor.setAlphaF(0.5);
        m_colorDialog.setCurrentColor(myColor);
        screenshotDelayed(&m_colorDialog, QStringLiteral("Expanded"));
    }

    {
        ColorDialog m_colorDialog(m_colorSpace);
        m_colorDialog.setLayoutDimensions( //
            ColorDialog::DialogLayoutDimensions::Collapsed);
        m_colorDialog.setCurrentColor(defaultColorRgb);
        QTabWidget *myTabWidget = m_colorDialog.findChild<QTabWidget *>();
        myTabWidget->setCurrentIndex(1);
        m_colorDialog.setOption( //
            ColorDialog::ColorDialogOption::ShowAlphaChannel);
        myColor = m_colorDialog.currentColor();
        myColor.setAlphaF(0.5);
        m_colorDialog.setCurrentColor(myColor);
        screenshotDelayed(&m_colorDialog, QStringLiteral("Collapsed"));
    }

    {
        ColorPatch m_colorPatch;
        myColor = defaultColorRgb;
        m_colorPatch.setColor(myColor);
        screenshot(&m_colorPatch);
        myColor.setAlphaF(0.5);
        m_colorPatch.setColor(myColor);
        screenshot(&m_colorPatch, QStringLiteral("SemiTransparent"));
        m_colorPatch.setColor(QColor());
        screenshot(&m_colorPatch, QStringLiteral("Invalid"));
    }

    {
        ColorWheel m_colorWheel(m_colorSpace);
        m_colorWheel.setHue(defaultColorLch.h);
        screenshot(&m_colorWheel);
    }

    {
        GradientSlider m_gradientSlider(m_colorSpace);
        m_gradientSlider.setOrientation(Qt::Horizontal);
        screenshot(&m_gradientSlider);
    }

    {
        MultiSpinBox m_multiSpinBox;
        PerceptualColor::MultiSpinBoxSectionConfiguration mySection;
        QList<PerceptualColor::MultiSpinBoxSectionConfiguration> hsvSectionConfigurations;
        QList<double> values;
        mySection.setDecimals(1);
        mySection.setPrefix(QString());
        mySection.setMinimum(0);
        mySection.setWrapping(true);
        mySection.setMaximum(360);
        mySection.setSuffix(QStringLiteral(u"° "));
        hsvSectionConfigurations.append(mySection);
        values.append(310);
        mySection.setPrefix(QStringLiteral(u" "));
        mySection.setMinimum(0);
        mySection.setMaximum(255);
        mySection.setWrapping(false);
        mySection.setSuffix(QStringLiteral(u" "));
        hsvSectionConfigurations.append(mySection);
        values.append(200);
        mySection.setSuffix(QString());
        hsvSectionConfigurations.append(mySection);
        values.append(100);
        m_multiSpinBox.setSectionConfigurations(hsvSectionConfigurations);
        m_multiSpinBox.setSectionValues(values);
        screenshot(&m_multiSpinBox);

        // Refresh button for the HLC spin box
        RefreshIconEngine *myIconEngine = new RefreshIconEngine;
        myIconEngine->setReferenceWidget(&m_multiSpinBox);
        // myIcon takes ownership of myIconEngine, therefore we won’t
        // delete myIconEngine manually.
        QIcon myIcon = QIcon(myIconEngine);
        QAction *myAction = new QAction(
            // Icon:
            myIcon,
            // Text:
            QString(),
            // Parent object:
            &m_multiSpinBox);
        MultiSpinBox m_multiSpinBoxWithButton;
        m_multiSpinBoxWithButton.setSectionConfigurations( //
            hsvSectionConfigurations);
        m_multiSpinBoxWithButton.setSectionValues(values);
        m_multiSpinBoxWithButton.addActionButton( //
            myAction, //
            QLineEdit::ActionPosition::TrailingPosition);
        screenshot(&m_multiSpinBoxWithButton, QStringLiteral("WithButton"));
    }

    {
        WheelColorPicker m_wheelColorPicker(m_colorSpace);
        m_wheelColorPicker.setCurrentColor(defaultColorLch);
        screenshotDelayed(&m_wheelColorPicker);
    }

    {
        PaletteWidget m_paletteWidget(m_colorSpace);
        m_paletteWidget.setCurrentColor(defaultColorRgb);
        screenshot(&m_paletteWidget);
    }

    return EXIT_SUCCESS;
}
