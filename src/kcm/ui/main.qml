/*
 *  SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQml.Models // TODO: Needed?

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

import SMART as SMART

KCM.AbstractKCM {
    id: root

    // Done in module.cpp, move here?
    // KCM.ConfigModule.buttons: KCM.ConfigModule.Default | KCM.ConfigModule.Apply | KCM.ConfigModule.Help

    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: Kirigami.Units.gridUnit * 35
    framedView: false // from powerdevil, needed?

    SMART.ServiceRunner { id: partitionManagerRunner; name: "org.kde.partitionmanager" }
    SMART.ServiceRunner { id: kupRunner; name: "kcm_kup" }

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
        }
    ]

    // TODO: From powerdevil, better way?
    Rectangle {
        // We're using AbstractKCM so that the automatic scrolling support of SimpleKCM
        // doesn't get in the way of the "tab content" Flickable below.
        // Use this lovely rectangle instead to still give the view a default background color.
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
    }

    SMART.DeviceModel { id: deviceModel }

    states: [
        State {
            name: "noData"
            when: !deviceModel.valid
        },
        State {
            name: "waitingForDevices"
            when: deviceModel.waiting
        },
        State {
            name: "noDevices"
            // TODO: set to model
            when: deviceModel.rowCount() == 0
        },
        State {
            name: "ready"
            when: deviceModel.rowCount() > 0
        }
    ]

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent

        width: parent.width - Kirigami.Units.gridUnit * 2

        visible: root.state == "noData" || root.state == "noDevices"
        icon.name: root.state == "noData" ? "data-warning-symbolic"
                                          : "edit-none-symbolic"
        text: root.state == "noData" ? i18nc("@info:placeholder", "Unable to obtain data")
                                     : i18nc("@info:placeholder", "No S.M.A.R.T. devices found")
        explanation: root.state == "noData" ? ""
                                            : i18nc("@info:usagetip", "KDED is not running")
    }

    ColumnLayout {
        anchors.fill: parent

        visible: root.state == "ready"
        spacing: 0

        Kirigami.NavigationTabBar {
            id: deviceTabBar
            Layout.fillWidth: true

            //onActionsChanged: if (currentIndex == -1) currentIndex = 0

            Instantiator {
                model: deviceModel
                delegate: Kirigami.Action {
                    icon.name: {
                        if (model.ignore) {
                            return "drive-harddisk-symbolic"
                        } else if (model.failed) {
                            return "disk-quota-high-symbolic"
                        } else if (model.instabilities.length !== 0) {
                            return "data-quota-low-symboic"
                        } else {
                            return "disk-quota-symbolic"
                        }
                    }
                    text: "%1\n%2".arg(model.product).arg(model.path)
                    onTriggered: deviceInfo.text = model.advancedReport
                    //displayComponent: Rectangle { color: "red"; width: 50; height: 50 }
                }

                onObjectAdded: (_, object) => deviceTabBar.actions.push(object)
                onObjectRemoved: (_, object) => deviceTabBar.actions.remove(object)
            }
        }

        QQC2.TextArea {
            id: deviceInfo

            Layout.fillWidth: true
            Layout.fillHeight: true

            background: null // render this without frame so it looks neatly integrated into the kcm page
            readOnly: true
            font.family: "monospace"
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
            text: "TODO"
        }
    }

    /*
    Kirigami.CardsListView {
        id: listView
        width: 110
        height: 160
        model: SMART.DeviceModel { id: deviceModel }

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
            banner.titleIcon: {
                if (failed) {
                    return "data-warning"
                }
                if (instabilities.length !== 0) {
                    return "data-information"
                }
                return ""
            }

            contentItem: Label {
                anchors.fill: parent
                wrapMode: Text.Wrap
                text: {
                    if (failed) {
                        return i18nc("@info",
                            "The SMART system of this device is reporting problems. This may be a sign of imminent device failure or data reliability being compromised. " +
                            "Back up your data and replace this drive as soon as possible to avoid losing any data.")
                    }
                    if (instabilities.length !== 0) {
                        var items = instabilities.map(item => "<li>%1</li>".arg(item))
                        return i18nc("@info %1 is a bunch of <li> with the strings from instabilities.cpp",
                            "<p>The SMART firmware is not reporting a failure, but there are early signs of malfunction. " +
                            "This might not point at imminent device failure but requires longer term analysis. " +
                            "Back up your data and contact the manufacturer of this disk, or replace it preemptively just to be safe.</p>" +
                            "<ul>%1</ul>", items.join(''))
                    }
                    return i18nc("@info",
                            "This device appears to be working as expected.")
                }
            }

            actions: [
                Kirigami.Action {
                    text: ignore
                          ? i18nc("@action/button action button to monitor a device for smart failure", "Monitor")
                          : i18nc("@action/button action button to ignore smart failures for a device", "Ignore")
                    icon.name: ignore ? "view-visible" : "view-hidden"
                    onTriggered: {
                        model.ignore = !ignore
                    }
                },
                Kirigami.Action {
                    visible: model.advancedReport !== ''
                    text: i18nc("@action/button show detailed smart report", "Detailed Information")
                    icon.name: "dialog-scripts"
                    onTriggered: {
                        // NB: push daftly hardcodes ui/ as prefix....
                        kcm.push("ReportPage.qml", {title: product, text: advancedReport})
                    }
                }
            ]
        }
    }
    */
}
