// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

import org.kde.kcm 1.2 as KCM
import QtQuick 2.14
import QtQml.Models 2.14
import SMART 1.0 as SMART
import org.kde.kirigami 2.12 as Kirigami
import QtQuick.Controls 2.14

KCM.SimpleKCM {
    Kirigami.CardsListView {
        id: listView
        width: 110
        height: 160
        model: SMART.DeviceModel { id: deviceModel }

        SMART.ServiceRunner {
            id: partitionManagerRunner
            name: "org.kde.partitionmanager"
        }

        SMART.ServiceRunner {
            id: kupRunner
            name: "kcm_kup"
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)

            opacity: !deviceModel.valid
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad } }

            icon.name: "messagebox_warning"
            text: i18nc("@info/status", "Unable to obtain data. KDED is not running.")
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)

            opacity: deviceModel.valid && !deviceModel.waiting && parent.count <= 0
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad } }

            icon.name: "edit-none"
            text: i18nc("@info/status", "No S.M.A.R.T. devices found.")
        }

        delegate: Kirigami.Card {
            banner.title: "%1 (%2)".arg(product).arg(path)
            banner.titleIcon: failed ? "data-warning" : ""
            actions: [
                Kirigami.Action {
                    visible: partitionManagerRunner.canRun
                    text: i18nc("@action/button action button to start partition manager", "Partition Manager")
                    icon.name: "partitionmanager"
                    onTriggered: partitionManagerRunner.run()
                },
                Kirigami.Action {
                    visible: kupRunner.canRun
                    text: i18nc("@action/button action button to start backup program", "Backup")
                    icon.name: "kup"
                    onTriggered: kupRunner.run()
                },
                Kirigami.Action {
                    text: ignore
                          ? i18nc("@action/button action button to monitor a device for smart failure", "Monitor")
                          : i18nc("@action/button action button to ignore smart failures for a device", "Ignore")
                    icon.name: ignore ? "view-visible" : "view-hidden"
                    onTriggered: {
                        model.ignore = !ignore
                    }
                }
            ]
            contentItem: Label {
                width: parent.width
                wrapMode: Text.Wrap
                text: failed
                      ? i18nc("@info",
                              "The SMART system of this device is reporting problems. This may be a sign of impending device failure or data reliablity being compromised. It is highly recommended that you backup your data and replace this drive as soon as possible to avoid losing any data.")
                      : i18nc("@info",
                             "This device appears to be working as expected.")
            }
        }
    }
}
