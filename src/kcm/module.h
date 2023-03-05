// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once

#include <KQuickAddons/ConfigModule>

class Module : public KQuickAddons::ConfigModule
{
    Q_OBJECT
public:
    explicit Module(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~Module() override;
    void load() override;
    void save() override;
    void defaults() override;
};
