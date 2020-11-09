// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QStandardPaths>
#include <QTest>
#include <QJsonDocument>

#include <functional>

#include <device.h>
#include <smartmonitor.h>

class MockCtl : public AbstractSMARTCtl
{
public:
    void run(const QString &devicePath) override
    {
        qDebug() << devicePath;
        emit finished(devicePath, m_docs.value(devicePath));
    }

    QMap<QString, QJsonDocument> m_docs;
};

class SMARTMonitorTest : public QObject
{
    Q_OBJECT

    struct Payload
    {
        QJsonDocument doc;
        bool err = true;
    };

private Q_SLOTS:
    void load(const QString &fixture, Payload &payload)
    {
        payload.doc = QJsonDocument();
        payload.err = true;

        QFile file(QFINDTESTDATA(fixture));
        QVERIFY(file.open(QFile::ReadOnly));

        payload.doc = QJsonDocument::fromJson(file.readAll());
        payload.err = false;
    }

    void testRun()
    {
        // Mock smartctl. We want fixed behavior.
        auto ctl = new MockCtl; // new; monitor takes ownership!
        Payload payload;
        load("fixtures/pass.json", payload);
        if (payload.err) { return; }
        ctl->m_docs["/dev/testfoobarpass"] = payload.doc;
        load("fixtures/fail.json", payload);
        if (payload.err) { return; }
        ctl->m_docs["/dev/testfoobarfail"] = payload.doc;

        // NOTE: monitor still talks to solid but we aren't interested in its results
        //   to also inject our fixtures we manually product device discoveries here.
        SMARTMonitor monitor(ctl);
        // don't start it, that'd only run solid stuff that we do not test here

        monitor.checkDevice(new Device {"udi-pass", "product", "/dev/testfoobarpass"});
        // discover this twice to ensure notifications aren't duplicated!
        monitor.checkDevice(new Device {"udi-fail", "product", "/dev/testfoobarfail"});
        monitor.checkDevice(new Device {"udi-fail", "product", "/dev/testfoobarfail"});

        bool sawPass = false;
        bool sawFail = false;
        for (const auto *device : monitor.devices()) {
            if (device->path() == "/dev/testfoobarpass") {
                QVERIFY(!device->failed());
                sawPass = true;
            }
            if (device->path() == "/dev/testfoobarfail") {
                QVERIFY(device->failed());
                sawFail = true;
            }
        }
        QVERIFY(sawPass);
        QVERIFY(sawFail);

        // Ensure removing works as well.
        // https://bugs.kde.org/show_bug.cgi?id=428746

        monitor.removeUDI("udi-pass");
        QCOMPARE(monitor.devices().size(), 1);
    }
};

QTEST_MAIN(SMARTMonitorTest)

#include "smartmonitortest.moc"
