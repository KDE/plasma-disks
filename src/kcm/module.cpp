// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "module.h"

#include <KPluginFactory>
#include "devicemodel.h"
#include "servicerunner.h"
#include "version.h"

K_PLUGIN_CLASS_WITH_JSON(Module, "smart.json")

Module::Module(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickConfigModule(parent, data, args)
{
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

#include "module.moc"
