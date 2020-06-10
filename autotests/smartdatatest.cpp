// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QStandardPaths>
#include <QTest>
#include <QJsonDocument>

#include <KSycoca>
#include <smartdata.h>

class SMARTDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPass()
    {
        QFile file(QFINDTESTDATA("fixtures/pass.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/testfoobarpass");
        QCOMPARE(data.m_status.m_passed, true);
    }

    void testFail()
    {
        // NB: fixture isn't actually of a failure, so fields needs tweaking as necessary
        QFile file(QFINDTESTDATA("fixtures/fail.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/testfoobarfail");
        QCOMPARE(data.m_status.m_passed, false);
    }
};

QTEST_MAIN(SMARTDataTest)

#include "smartdatatest.moc"
