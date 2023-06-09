﻿// SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
// SPDX-License-Identifier: BSD-2-Clause OR MIT

// Own headers
// First the interface, which forces the header to be self-contained.
#include "asyncimageproviderbase.h"

namespace PerceptualColor
{
/** @brief Constructor
 * @param parent The object’s parent object. This parameter will be passed
 * to the base class’s constructor. */
AsyncImageProviderBase::AsyncImageProviderBase(QObject *parent)
    : QObject(parent)
{
}

/** @brief Destructor */
AsyncImageProviderBase::~AsyncImageProviderBase() noexcept
{
}

} // namespace PerceptualColor
