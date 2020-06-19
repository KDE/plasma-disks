// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

import org.kde.kcm 1.2 as KCM
import QtQuick 2.14
import QtQml.Models 2.14
import SMART 1.0 as SMART
import org.kde.kirigami 2.4 as Kirigami
import QtQuick.Controls 2.14

KCM.SimpleKCM {
    Kirigami.CardsListView {
        id: listView
        width: 110
        height: 160
        model: SMART.DeviceModel {}

        delegate: Kirigami.Card {
            banner.title: "%1 (%2)".arg(product).arg(path)
            banner.titleIcon: failed ? "data-warning" : ""
            actions: [
                Action {
                    text: i18n("Partition Manager")
                    icon.name: "partitionmanager"
                    onTriggered: {}
                },
                Action {
                    text: i18n("Backup")
                    icon.name: "kup"
                    onTriggered: {}
                }
            ]
            contentItem: Label {
                width: parent.width
                wrapMode: Text.Wrap
                text: failed ?
                          i18n("The SMART system of this device is reporting problems. This may be a sign of impending device failure or data reliablity being compromised. It is highly recommended that you backup your data and replace this drive as soon as possible to avoid losing any data.")
                        :
                          i18n("This device appears to be working as expected.")
            }
        }
    }
}
