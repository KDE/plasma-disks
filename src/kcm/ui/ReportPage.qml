// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias text: textArea.text

    QQC2.TextArea { // scrolling is implemented in the simplekcm
        id: textArea
        background: null // render this without frame so it looks neatly integrated into the kcm page
        readOnly: true
        font.family: "monospace"
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap
    }
}
