﻿// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

#ifndef RGBCOLORSPACE_H
#define RGBCOLORSPACE_H

#include "constpropagatinguniquepointer.h"
#include "lchdouble.h"
#include "rgbdouble.h"
#include <lcms2.h>
#include <qdatetime.h>
#include <qglobal.h>
#include <qmetatype.h>
#include <qobject.h>
#include <qrgb.h>
#include <qsharedpointer.h>
#include <qstring.h>
#include <qversionnumber.h>
class QRgba64;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <qtmetamacros.h>
#else
#include <qobjectdefs.h>
#endif

Q_DECLARE_METATYPE(cmsColorSpaceSignature)
Q_DECLARE_METATYPE(cmsProfileClassSignature)

namespace PerceptualColor
{
class RgbColorSpacePrivate;

/** @internal
 *
 * @brief Provides access to LittleCMS color management
 *
 * This class has no public constructor. Objects can be generated
 * with the static factory functions.
 *
 * @note The maximum accepted Cielch-D50/Cielab-D50 lightness range is
 * 0 to 100, and the maximum Cielch-D50 chroma is
 * @ref CielchD50Values::maximumChroma. Values outside of this
 * range are considered out-of-gamut, even if the profile
 * itself would accept them.
 *
 * @todo Unit tests for @ref RgbColorSpace, especially the to…() functions.
 *
 * @todo Unit tests for @ref profileMaximumCielchD50Chroma and
 *       @ref profileMaximumOklchChroma with all profiles that are available
 *       in the testbed.
 *
 * @todo Allow also other perceptual color spaces instead of CIELAB:
 *       This might be Oklab or Google’s HCT, CAM16 or DIN99. Attention:
 *       The range of valid values for Oklab is identical for L
 *       (0–1, or 0%–100%), but different for a and b:
 *       https://www.w3.org/TR/css-color-4/#ok-lab says up to 0.5, but
 *       we would have to actually test this. Therefore, also the
 *       @ref profileMaximumCielchD50Chroma would have to be provides for all
 *       these color spaces individually. Anyway, we could also
 *       output the data in a new data type for cylindrical coordinates
 *       (angle degree, radial, height), independent of the color
 *       space, which must always be cylindrical anyway as we have
 *       no support for anything else in our widgets.
 *
 *
 * @todo The sRGB colour space object should be implemented as a singleton.
 *       This is possible because it is thread-safe, and therefore it does
 *       not make sense to have more than one object of this class. At the
 *       same time, it is necessary that it implements the common interface
 *       of the colour space objects that are created on-the-fly from ICC
 *       profile files, therefore it cannot be static.
 *       As a consequence, translations within sRGB objects should always
 *       be dynamic instead of being done only once at instantiation time,
 *       because now the instantiation time is out of control of the library
 *       user. (And maybe even for ICC profiles we could provide ALL
 *       translations, be reading ALL possible translations at creating time
 *       and guarding them? Or would this be overkill?)
 *       The singleton pattern has special requirements
 *       for: 1) thread-safety. 2)  dynamic libraries. See Wikipedia
 *       for details!
 *
 * @todo Is it possible to split this into an interface and into
 *       various implementations (a slow but safe implementation for
 *       all valid ICC files, and a fast optimized implementation for sRGB
 *       only? If so, is it possible to get rid of the dependency from
 *       LittleCMS by implementing sRGB ourselves, and providing ICC support
 *       via an optional header-only header that would link against LittleCMS
 *       without injecting this dependency into our shared library? And
 *       this might be faster! The web page
 *       https://bottosson.github.io/misc/colorpicker/#91a7ee is only
 *       JavaScript and works faster than this library! See
 *       https://en.wikipedia.org/wiki/SRGB#From_sRGB_to_CIE_XYZ and
 *       http://www.brucelindbloom.com/index.html?Math.html and
 *       http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
 *       for implementation details.
 *
 * @todo We return double precision values. But doesn’t use LittleCMS
 *       only 16-bit-integer internally? On the other hand: Using double
 *       precision allows to filter out out-of-range values…
 *
 * @todo Do not convert QRgba64 to RgbDouble, but use a transform that
 *       reads QRgba64 directly. While the benefit might not be big in
 *       this function, in general it would be good to review for which data
 *       types we provide transforms and minimize conversions.
 *
 * @todo In the API of this class, clarify the precision. If more than
 *       8 bit per channel, we have to switch from QRgb to QRgb64. But
 *       probably all OS APIs only accept 8 bit anyway? Is it worth the
 *       pain just because @ref ColorDialog can return <tt>QColor</tt>
 *       which provides 16 bit support?
 *
 * @todo Find more efficient ways of in-gamut detection. Maybe provide
 *       a subclass with optimized algorithms just for sRGB-build-in? */
class RgbColorSpace : public QObject
{
    Q_OBJECT

    /** @brief The absolute file path of the profile.
     *
     * @note This is empty for build-in profiles.
     *
     * @sa READ @ref profileAbsoluteFilePath() const */
    Q_PROPERTY(QString profileAbsoluteFilePath READ profileAbsoluteFilePath CONSTANT)

    /** @brief The class of the profile.
     *
     * This type is declared as type to Qt’s type system via
     * <tt>Q_DECLARE_METATYPE</tt>. Depending on your use case (for
     * example if you want to use for <em>queued</em> signal-slot connections),
     * you might consider calling <tt>qRegisterMetaType()</tt> for
     * this type, once you have a QApplication object.
     *
     * @sa READ @ref profileClass() const */
    Q_PROPERTY(cmsProfileClassSignature profileClass READ profileClass CONSTANT)

    /** @brief The color model of the color space which is described by
     * this profile.
     *
     * This type is declared as type to Qt’s type system via
     * <tt>Q_DECLARE_METATYPE</tt>. Depending on your use case (for
     * example if you want to use for <em>queued</em> signal-slot connections),
     * you might consider calling <tt>qRegisterMetaType()</tt> for
     * this type, once you have a QApplication object.
     *
     * @sa READ @ref profileColorModel() const */
    Q_PROPERTY(cmsColorSpaceSignature profileColorModel READ profileColorModel CONSTANT)

    /** @brief The copyright information of the profile.
     *
     * If supported by the underlying profile, this property is localized
     * to the current locale <em>at the moment of the constructor call</em>.
     *
     * @note This is empty if the information is not available.
     *
     * @sa READ @ref profileCopyright() const */
    Q_PROPERTY(QString profileCopyright READ profileCopyright CONSTANT)

    /** @brief The date and time of the creation of the profile.
     *
     * @note This is null if the information is not available.
     *
     * @sa READ @ref profileCreationDateTime() const */
    Q_PROPERTY(QDateTime profileCreationDateTime READ profileCreationDateTime CONSTANT)

    /** @brief The file size of the profile, measured in byte.
     *
     * @note This is <tt>-1</tt> for build-in profiles.
     *
     * @sa READ @ref profileFileSize() const */
    Q_PROPERTY(qint64 profileFileSize READ profileFileSize CONSTANT)

    /** @brief Wether or not the profile has a color lookup table (CLUT).
     *
     * @sa READ @ref profileHasClut() const */
    Q_PROPERTY(bool profileHasClut READ profileHasClut CONSTANT)

    /** @brief Wether or not the profile has a matrix shaper.
     *
     * @sa READ @ref profileHasMatrixShaper() const */
    Q_PROPERTY(bool profileHasMatrixShaper READ profileHasMatrixShaper CONSTANT)

    /** @brief The ICC version of the profile.
     *
     * @note This is null if the information is not available.
     *
     * @sa READ @ref profileIccVersion() const */
    Q_PROPERTY(QVersionNumber profileIccVersion READ profileIccVersion CONSTANT)

    /** @brief The manufacturer information of the profile.
     *
     * If supported by the underlying profile, this property is localized
     * to the current locale <em>at the moment of the constructor call</em>.
     *
     * @note This is empty if the information is not available.
     *
     * @sa READ @ref profileManufacturer() const */
    Q_PROPERTY(QString profileManufacturer READ profileManufacturer CONSTANT)

    /** @brief The maximum CIELch-D50 chroma of the profile.
     *
     * This value is equal or slightly bigger than the actual maximum chroma.
     *
     * @note This is the result of an auto-detection, which might theoretically
     * in very rare cases return a value that is smaller than the actual
     * maximum chroma.
     *
     * @sa READ @ref profileMaximumCielchD50Chroma() const */
    Q_PROPERTY(double profileMaximumCielchD50Chroma READ profileMaximumCielchD50Chroma CONSTANT)

    /** @brief The maximum Oklch chroma of the profile.
     *
     * This value is equal or slightly bigger than the actual maximum chroma.
     *
     * @note This is the result of an auto-detection, which might theoretically
     * in very rare cases return a value that is smaller than the actual
     * maximum chroma.
     *
     * @sa READ @ref profileMaximumOklchChroma() const */
    Q_PROPERTY(double profileMaximumOklchChroma READ profileMaximumOklchChroma CONSTANT)

    /** @brief The model information of the profile.
     *
     * If supported by the underlying profile, this property is localized
     * to the current locale <em>at the moment of the constructor call</em>.
     *
     * @note This is empty if the information is not available.
     *
     * @sa READ @ref profileModel() const */
    Q_PROPERTY(QString profileModel READ profileModel CONSTANT)

    /** @brief The name of the profile.
     *
     * If supported by the underlying profile, this property is localized
     * to the current locale <em>at the moment of the constructor call</em>.
     *
     * @note This is empty if the information is not available.
     *
     * @sa READ @ref profileName() const */
    Q_PROPERTY(QString profileName READ profileName CONSTANT)

    /** @brief The name of the profile.
     *
     * If supported by the underlying profile, this property is localized
     * to the current locale <em>at the moment of the constructor call</em>.
     *
     * This type is declared as type to Qt’s type system via
     * <tt>Q_DECLARE_METATYPE</tt>. Depending on your use case (for
     * example if you want to use for <em>queued</em> signal-slot connections),
     * you might consider calling <tt>qRegisterMetaType()</tt> for
     * this type, once you have a QApplication object.
     *
     * @sa READ @ref profileName() const */
    Q_PROPERTY(cmsColorSpaceSignature profilePcsColorModel READ profilePcsColorModel CONSTANT)

public: // Static factory functions
    [[nodiscard]] Q_INVOKABLE static QSharedPointer<PerceptualColor::RgbColorSpace> createFromFile(const QString &fileName);
    [[nodiscard]] Q_INVOKABLE static QSharedPointer<PerceptualColor::RgbColorSpace> createSrgb();

public:
    virtual ~RgbColorSpace() noexcept override;
    [[nodiscard]] Q_INVOKABLE virtual bool isCielabD50InGamut(const cmsCIELab &lab) const;
    [[nodiscard]] Q_INVOKABLE virtual bool isCielchD50InGamut(const PerceptualColor::LchDouble &lch) const;
    [[nodiscard]] Q_INVOKABLE virtual bool isOklchInGamut(const PerceptualColor::LchDouble &lch) const;
    /** @brief Getter for property @ref profileAbsoluteFilePath
     *  @returns the property @ref profileAbsoluteFilePath */
    [[nodiscard]] QString profileAbsoluteFilePath() const;
    /** @brief Getter for property @ref profileClass
     *  @returns the property @ref profileClass */
    [[nodiscard]] cmsProfileClassSignature profileClass() const;
    /** @brief Getter for property @ref profileColorModel
     *  @returns the property @ref profileColorModel */
    [[nodiscard]] cmsColorSpaceSignature profileColorModel() const;
    /** @brief Getter for property @ref profileCopyright
     *  @returns the property @ref profileCopyright */
    [[nodiscard]] QString profileCopyright() const;
    /** @brief Getter for property @ref profileCreationDateTime
     *  @returns the property @ref profileCreationDateTime */
    [[nodiscard]] QDateTime profileCreationDateTime() const;
    /** @brief Getter for property @ref profileFileSize
     *  @returns the property @ref profileFileSize */
    [[nodiscard]] qint64 profileFileSize() const;
    /** @brief Getter for property @ref profileHasClut
     *  @returns the property @ref profileHasClut */
    [[nodiscard]] bool profileHasClut() const;
    /** @brief Getter for property @ref profileHasMatrixShaper
     *  @returns the property @ref profileHasMatrixShaper */
    [[nodiscard]] bool profileHasMatrixShaper() const;
    /** @brief Getter for property @ref profileIccVersion
     *  @returns the property @ref profileIccVersion */
    [[nodiscard]] QVersionNumber profileIccVersion() const;
    /** @brief Getter for property @ref profileManufacturer
     *  @returns the property @ref profileManufacturer */
    [[nodiscard]] QString profileManufacturer() const;
    /** @brief Getter for property @ref profileMaximumCielchD50Chroma
     *  @returns the property @ref profileMaximumCielchD50Chroma */
    [[nodiscard]] double profileMaximumCielchD50Chroma() const;
    /** @brief Getter for property @ref profileMaximumOklchChroma
     *  @returns the property @ref profileMaximumOklchChroma */
    [[nodiscard]] double profileMaximumOklchChroma() const;
    /** @brief Getter for property @ref profileModel
     *  @returns the property @ref profileModel */
    [[nodiscard]] QString profileModel() const;
    /** @brief Getter for property @ref profileName
     *  @returns the property @ref profileName */
    [[nodiscard]] QString profileName() const;
    /** @brief Getter for property @ref profilePcsColorModel
     *  @returns the property @ref profilePcsColorModel */
    [[nodiscard]] cmsColorSpaceSignature profilePcsColorModel() const;
    [[nodiscard]] Q_INVOKABLE virtual PerceptualColor::LchDouble reduceCielchD50ChromaToFitIntoGamut(const PerceptualColor::LchDouble &cielchD50color) const;
    [[nodiscard]] Q_INVOKABLE virtual PerceptualColor::LchDouble reduceOklchChromaToFitIntoGamut(const PerceptualColor::LchDouble &oklchColor) const;
    [[nodiscard]] Q_INVOKABLE virtual cmsCIELab toCielabD50(const QRgba64 rgbColor) const;
    [[nodiscard]] Q_INVOKABLE virtual PerceptualColor::LchDouble toCielchD50Double(const QRgba64 rgbColor) const;
    [[nodiscard]] Q_INVOKABLE virtual QRgb fromCielchD50ToQRgbBound(const PerceptualColor::LchDouble &lch) const;
    [[nodiscard]] Q_INVOKABLE virtual QRgb fromCielabD50ToQRgbOrTransparent(const cmsCIELab &lab) const;
    [[nodiscard]] Q_INVOKABLE virtual PerceptualColor::RgbDouble fromCielchD50ToRgbDoubleUnbound(const PerceptualColor::LchDouble &lch) const;

private:
    Q_DISABLE_COPY(RgbColorSpace)

    /** @internal
     *
     * @brief Private constructor.
     *
     * @param parent The widget’s parent widget. This parameter will be
     * passed to the base class’s constructor. */
    explicit RgbColorSpace(QObject *parent = nullptr);

    /** @internal
     *
     * @brief Declare the private implementation as friend class.
     *
     * This allows the private class to access the protected members and
     * functions of instances of <em>this</em> class. */
    friend class RgbColorSpacePrivate;
    /** @brief Pointer to implementation (pimpl) */
    ConstPropagatingUniquePointer<RgbColorSpacePrivate> d_pointer;

    /** @internal @brief Only for unit tests. */
    friend class TestRgbColorSpace;
};

} // namespace PerceptualColor

#endif // RGBCOLORSPACE_H
