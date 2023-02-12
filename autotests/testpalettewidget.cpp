﻿// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

// First included header is the public header of the class we are testing;
// this forces the header to be self-contained.
#include "palettewidget.h"
// Second, the private implementation.
#include "palettewidget_p.h" // IWYU pragma: keep

#include "constpropagatinguniquepointer.h"
#include "helperqttypes.h"
#include "rgbcolorspacefactory.h"
#include <QtCore/qsharedpointer.h>
#include <memory>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qglobal.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpoint.h>
#include <qsize.h>
#include <qstring.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qstyleoption.h>
#include <qtest.h>
#include <qtestcase.h>
#include <qtestdata.h>
#include <qtestkeyboard.h>
#include <type_traits>
#include <utility>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <qtmetamacros.h>
#else
#include <qobjectdefs.h>
#include <qstringlist.h>
#endif

namespace PerceptualColor
{
class RgbColorSpace;

class TestPaletteWidget : public QObject
{
    Q_OBJECT

public:
    explicit TestPaletteWidget(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private:
    QSharedPointer<PerceptualColor::RgbColorSpace> m_rgbColorSpace = RgbColorSpaceFactory::createSrgb();

    void provideStyleNamesAsData()
    {
        QTest::addColumn<QString>("styleName");
        const auto container = QStyleFactory::keys();
        for (const QString &currentStyleName : std::as_const(container)) {
            QTest::newRow(currentStyleName.toUtf8().constData()) << currentStyleName;
        }
    }

private Q_SLOTS:
    void initTestCase()
    {
        // Called before the first test function is executed
    }

    void cleanupTestCase()
    {
        // Called after the last test function was executed
    }

    void init()
    {
        // Called before each test function is executed
    }

    void cleanup()
    {
        // Called after every test function
    }

    void testConstructorDestructor()
    {
        PaletteWidget testObject(m_rgbColorSpace);
    }

    void testConstructorDefaultValues()
    {
        PaletteWidget testObject(m_rgbColorSpace);
        // Verify that initially one of the colors of the palette
        // is actually selected (no -1 as index):
        QVERIFY(testObject.d_pointer->m_selectedBasicColor >= 0);
        QVERIFY(testObject.d_pointer->m_selectedTintShade >= 0);
        // Verify that the initial color conforms to QColorDialog
        QColorDialog reference;
        QCOMPARE(testObject.currentColor(), reference.currentColor());
    }

    void testMinimalSizeHint()
    {
        PaletteWidget testWidget(m_rgbColorSpace);
        QVERIFY2(testWidget.minimumSizeHint().width() > 0, "minimalSizeHint width is implemented.");
        QVERIFY2(testWidget.minimumSizeHint().height() > 0, "minimalSizeHint height is implemented.");
    }

    void testSizeHint()
    {
        PaletteWidget testWidget(m_rgbColorSpace);
        QVERIFY2(testWidget.sizeHint().width() >= testWidget.minimumSizeHint().width(), "sizeHint width is bigger than or equal to minimalSizeHint width.");
        QVERIFY2(testWidget.sizeHint().height() >= testWidget.minimumSizeHint().height(),
                 "sizeHint height is bigger than or equal to minimalSizeHint "
                 "height.");
    }

    void testCurrentColor()
    {
        PaletteWidget testWidget(m_rgbColorSpace);
        // Prepare test
        QObject scopeMarker;
        QColor lastSignalColor;
        int signalCount = 0;
        connect( //
            &testWidget,
            &PaletteWidget::currentColorChanged,
            &scopeMarker,
            [&lastSignalColor, &signalCount](const QColor &newCurrentColor) {
                lastSignalColor = newCurrentColor;
                ++signalCount;
            });
        // Initialize the palette widget and lastSignalColor to a defined state
        testWidget.d_pointer->selectColorFromPalette(0, 0);

        // Test
        const QColor oldColor = lastSignalColor;
        testWidget.d_pointer->selectColorFromPalette(0, 1);
        QVERIFY(oldColor != lastSignalColor); // Signal has been emitted

        testWidget.setCurrentColor(Qt::red);
        QCOMPARE(testWidget.currentColor(), Qt::red);
        QCOMPARE(lastSignalColor, Qt::red);
        const int oldSignalCount = signalCount;
        testWidget.setCurrentColor(Qt::green);
        QCOMPARE(testWidget.currentColor(), Qt::green);
        QCOMPARE(signalCount, oldSignalCount + 1);
        QCOMPARE(lastSignalColor, Qt::green);
        // Setting it again should not trigger a new signal
        testWidget.setCurrentColor(Qt::green);
        QCOMPARE(testWidget.currentColor(), Qt::green);
        QCOMPARE(signalCount, oldSignalCount + 1);
        QCOMPARE(lastSignalColor, Qt::green);

        // Test conformance with QColorDialog when assigning invalid colors
        testWidget.setCurrentColor(Qt::blue);
        QColorDialog myQColorDialog;
        myQColorDialog.setCurrentColor(Qt::blue);
        testWidget.setCurrentColor(QColor());
        myQColorDialog.setCurrentColor(QColor());
        QCOMPARE(testWidget.currentColor(), myQColorDialog.currentColor());
        QCOMPARE(lastSignalColor, myQColorDialog.currentColor());
    }

    void testPatchSpacing_data()
    {
        provideStyleNamesAsData();
    }

    void testPatchSpacing()
    {
        QFETCH(QString, styleName);
        QStyle *style = QStyleFactory::create(styleName);
        {
            // Own block to make sure style will be deleted _after_ testWidget
            // has been destroyed.
            PaletteWidget testWidget(m_rgbColorSpace);
            testWidget.setStyle(style);
            QVERIFY(testWidget.d_pointer->horizontalPatchSpacing() > 0);
            QVERIFY(testWidget.d_pointer->verticalPatchSpacing() > 0);
            QVERIFY(testWidget.d_pointer->horizontalPatchSpacing() > testWidget.d_pointer->verticalPatchSpacing());
        }
        delete style;
    }

    void testPatchSize_data()
    {
        provideStyleNamesAsData();
    }

    void testPatchSize()
    {
        QFETCH(QString, styleName);
        QStyle *style = QStyleFactory::create(styleName);
        {
            // Own block to make sure style will be deleted _after_ testWidget
            // has been destroyed.
            PaletteWidget testWidget(m_rgbColorSpace);
            testWidget.setStyle(style);
            QVERIFY(!testWidget.d_pointer->patchSizeInner().isEmpty());
            QVERIFY(!testWidget.d_pointer->patchSizeOuter().isEmpty());
            QVERIFY(testWidget.d_pointer->patchSizeOuter().width() > testWidget.d_pointer->patchSizeInner().width());
            QVERIFY(testWidget.d_pointer->patchSizeOuter().height() > testWidget.d_pointer->patchSizeInner().height());

            // Test also some design properties:
            QVERIFY(testWidget.d_pointer->patchSizeInner().width() > testWidget.d_pointer->horizontalPatchSpacing());
            QVERIFY(testWidget.d_pointer->patchSizeInner().height() > testWidget.d_pointer->verticalPatchSpacing());
        }
        delete style;
    }

    void testRetranslateUI()
    {
        PaletteWidget testWidget(m_rgbColorSpace);
        // Test that function call does not crash:
        testWidget.d_pointer->retranslateUi();
    }

    void testInitStyleOptions()
    {
        PaletteWidget testWidget(m_rgbColorSpace);

        // Test that function call does not crash with regular object:
        QStyleOptionFrame temp;
        testWidget.d_pointer->initStyleOption(&temp);

        // Test that function does not crash with nullptr:
        testWidget.d_pointer->initStyleOption(nullptr);
    }

    void testOffset_data()
    {
        provideStyleNamesAsData();
    }

    void testOffset()
    {
        QFETCH(QString, styleName);
        QStyle *style = QStyleFactory::create(styleName);
        {
            // Own block to make sure style will be deleted _after_ testWidget
            // has been destroyed.
            PaletteWidget testWidget(m_rgbColorSpace);
            testWidget.setStyle(style);
            QStyleOptionFrame temp;
            testWidget.d_pointer->initStyleOption(&temp);
            QVERIFY(testWidget.d_pointer->offset(temp).x() >= 0);
            QVERIFY(testWidget.d_pointer->offset(temp).y() >= 0);
        }
        delete style;
    }

    void testKeyboard()
    {
        PaletteWidget testWidget(m_rgbColorSpace);
        const QListSizeType count = //
            qMax(testWidget.d_pointer->m_paletteColors.count(), //
                 testWidget.d_pointer->m_paletteColors.at(0).count())
            // Add 1 to exceed the possible number of fields (crash test)
            + 1;

        // Starting point is (0, 0) when no palette color was selected before
        testWidget.setCurrentColor(
            // A color that is not in the palette:
            QColor(1, 2, 3));
        QTest::keyClick(&testWidget, Qt::Key_Left);
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, 0);

        // Test keys LTR
        testWidget.setLayoutDirection(Qt::LayoutDirection::LeftToRight);
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Right);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 testWidget.d_pointer->m_paletteColors.count() - 1);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Left);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        QTest::keyClick(&testWidget, Qt::Key_End);
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 testWidget.d_pointer->m_paletteColors.count() - 1);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        QTest::keyClick(&testWidget, Qt::Key_Home);
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Left);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);

        // Key tests RTL
        testWidget.setLayoutDirection(Qt::LayoutDirection::RightToLeft);
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Left);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 testWidget.d_pointer->m_paletteColors.count() - 1);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Right);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        QTest::keyClick(&testWidget, Qt::Key_End);
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 testWidget.d_pointer->m_paletteColors.count() - 1);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        QTest::keyClick(&testWidget, Qt::Key_Home);
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);

        // Key tests vertical
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Down);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 testWidget.d_pointer->m_paletteColors.at(0).count() - 1);
        for (int i = 0; i < count; ++i) {
            QTest::keyClick(&testWidget, Qt::Key_Up);
        }
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
        QTest::keyClick(&testWidget, Qt::Key_PageDown);
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 testWidget.d_pointer->m_paletteColors.at(0).count() - 1);
        QTest::keyClick(&testWidget, Qt::Key_PageUp);
        QCOMPARE(testWidget.d_pointer->m_selectedBasicColor, //
                 0);
        QCOMPARE(testWidget.d_pointer->m_selectedTintShade, //
                 0);
    }
};

} // namespace PerceptualColor

QTEST_MAIN(PerceptualColor::TestPaletteWidget)

// The following “include” is necessary because we do not use a header file:
#include "testpalettewidget.moc"
