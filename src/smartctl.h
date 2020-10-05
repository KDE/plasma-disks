// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTCTL_H
#define SMARTCTL_H

#include <QJsonDocument>
#include <QObject>

#include <queue>

class AbstractSMARTCtl : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractSMARTCtl() = default;
    virtual void run(const QString &devicePath) = 0;

signals:
    void finished(const QString &devicePath, const QJsonDocument &document) const;

protected:
    AbstractSMARTCtl() = default;

private:
    Q_DISABLE_COPY(AbstractSMARTCtl)
};

class SMARTCtl : public AbstractSMARTCtl
{
public:
    /** smartctl manpage
       Bit 0: Command line did not parse.
       Bit 1: Device open failed, device did not return an IDENTIFY DEVICE structure,
              or device is in a  low-power  mode  (see  '-n'  option above).
       Bit 2: Some  SMART  or other ATA command to the disk failed, or there was a
              checksum error in a SMART data structure (see '-b' option above).
       Bit 3: SMART status check returned "DISK FAILING".
       Bit 4: We found prefail Attributes <= threshold.
       Bit 5: SMART status check returned "DISK OK" but we found that some (usage or
              prefail) Attributes have been <= threshold at some time in the past.
       Bit 6: The device error log contains records of errors.
       Bit 7: The  device  self-test  log  contains records of errors.
              [ATA only] Failed self-tests outdated by a newer successful extended
              self-test are ignored.
     */

    enum Failure {
        CmdLineParse = 0x1,
        DeviceOpen = 0x2,
        InternalCommand = 0x4,
        Disk = 0x8,
        Prefail = 0x10,
        PastPrefail = 0x20,
        ErrorsRecorded = 0x40,
        SelfTestErrors = 0x80
        // The entire thing doesn't exceed 8 bits because it's a posix exit code.
    };

    void run(const QString &devicePath) override;

private:
    bool m_busy = false;
    std::queue<QString> m_requestQueue;
};

#endif // SMARTCTL_H
