// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "module.h"

#include <KAboutData>
#include <KLocalizedString>

#include "devicemodel.h"
#include "servicerunner.h"
#include "version.h"

Module::Module(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
{

    auto aboutData = new KAboutData(QStringLiteral("plasma_disks"),
                                    i18nc("@title", "Storage Device Health Monitoring"),
                                    QString::fromLatin1(global_s_versionStringFull),
                                    QString(),
                                    KAboutLicense::LicenseKey::GPL_V3,
                                    i18nc("@info:credit", "Copyright 2020 Harald Sitter"));
    setAboutData(aboutData);
    // We have no help so remove the button from the buttons.
    setButtons(buttons() ^ Help ^ Default ^ Apply);

    qmlRegisterType<DeviceModel>("SMART", 1, 0, "DeviceModel");
    qmlRegisterType<ServiceRunner>("SMART", 1, 0, "ServiceRunner");
}

Module::~Module()
{
}

void Module::load()
{
}

void Module::save()
{
}

void Module::defaults()
{
}
