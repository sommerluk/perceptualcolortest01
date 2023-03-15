﻿// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

#ifndef MULTISPINBOX_P_H
#define MULTISPINBOX_P_H

// Include the header of the public class of this private implementation.
// #include "multispinbox.h"

#include "constpropagatingrawpointer.h"
#include "helperqttypes.h"
#include "multispinboxsection.h"
#include <qaccessiblewidget.h>
#include <qglobal.h>
#include <qlist.h>
#include <qobject.h>
#include <qpointer.h>
#include <qstring.h>
class QAccessibleInterface;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <qtmetamacros.h>
#else
#include <qobjectdefs.h>
#endif

namespace PerceptualColor
{
class ExtendedDoubleValidator;
class MultiSpinBox;

/** @internal
 *
 *  @brief Private implementation within the <em>Pointer to
 *  implementation</em> idiom */
class MultiSpinBoxPrivate final : public QObject
{
    Q_OBJECT

public:
    explicit MultiSpinBoxPrivate(MultiSpinBox *backLink);
    /** @brief Default destructor
     *
     * The destructor is non-<tt>virtual</tt> because
     * the class as a whole is <tt>final</tt>. */
    virtual ~MultiSpinBoxPrivate() noexcept override = default;

    /** @brief Only for unit tests. */
    friend class TestMultiSpinBox;

    // constexpr
    /** @brief Default value of a section */
    static constexpr double defaultSectionValue = 0;

    /** @brief Counter for all actions added
     * by @ref MultiSpinBox::addActionButton. */
    int m_actionButtonCount = 0;
    /** @brief Holds the index of the currently selected section.
     * @sa @ref setCurrentIndexAndUpdateTextAndSelectValue
     * @sa @ref setCurrentIndexWithoutUpdatingText */
    QListSizeType m_currentIndex = 0;
    /** @brief Holds the data for the sections.
     *
     * This list is guaranteed to contain at least <em>one</em> section.
     *
     * @sa @ref MultiSpinBox::sectionConfigurations()
     * @sa @ref MultiSpinBox::setSectionConfigurations() */
    QList<MultiSpinBoxSection> m_sectionConfigurations;
    /** @brief Internal storage for
     * property @ref MultiSpinBox::sectionValues. */
    QList<double> m_sectionValues = QList<double>{MultiSpinBoxPrivate::defaultSectionValue};
    /** @brief The string of everything <em>after</em> the value of the
     * current section.
     *
     * This includes the suffix of the current section and everything
     * (prefixes, values and suffixes) of all sections that come after
     * the current sections. */
    QString m_textAfterCurrentValue;
    /** @brief The string of everything <em>before</em> the value of the
     * current section.
     *
     * This includes everything (prefixes, values and suffixes) of all
     * sections that come before the current section, and the prefix
     * of the current section. */
    QString m_textBeforeCurrentValue;
    /** @brief The string of the value of the current section. */
    QString m_textOfCurrentValue;
    /** @brief The validator for the <tt>QLineEdit</tt>.
     *
     * This validator allows changes only to the <em>current</em> section.
     *
     * If the current section changes, also this validator’s configuration
     * will be adapted to cover the new current section.
     *
     * @note It is <em>not</em> possible to change various values at the
     * same time, for example by marking all the current text and use
     * Ctrl-V to past a complete new value from the clipboard. This would
     * be impossible to parse reliably, because the prefixes and suffixes
     * of each section might contain (localized) digits that would be
     * difficult to differentiate from the actual value. */
    QPointer<ExtendedDoubleValidator> m_validator;

    // Functions
    [[nodiscard]] QString formattedValue(QListSizeType index) const;
    [[nodiscard]] bool isCursorPositionAtCurrentSectionValue(const int cursorPosition) const;
    void setCurrentIndexAndUpdateTextAndSelectValue(QListSizeType newIndex);
    void setCurrentIndexToZeroAndUpdateTextAndSelectValue();
    void setCurrentIndexWithoutUpdatingText(QListSizeType newIndex);
    void setSectionValuesWithoutFurtherUpdating(const QList<double> &newSectionValues);
    void updatePrefixValueSuffixText();

public Q_SLOTS:
    void reactOnCursorPositionChange(const int oldPos, const int newPos);
    void updateCurrentValueFromText(const QString &lineEditText);

private:
    Q_DISABLE_COPY(MultiSpinBoxPrivate)

    /** @brief Pointer to the object from which <em>this</em> object
     *  is the private implementation. */
    ConstPropagatingRawPointer<MultiSpinBox> q_pointer;
};

/** @internal
 *
 * @brief Interface for accessible objects. */
class AccessibleMultiSpinBox : public QAccessibleWidget
{
public:
    explicit AccessibleMultiSpinBox(MultiSpinBox *w);
    virtual ~AccessibleMultiSpinBox() override;
    [[nodiscard]] static QAccessibleInterface *factory(const QString &classname, QObject *object);
};

} // namespace PerceptualColor

#endif // MULTISPINBOX_P_H
