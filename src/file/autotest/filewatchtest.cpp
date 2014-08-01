/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "filewatchtest.h"
#include "filewatch.h"
#include "fileindexerconfigutils.h"
#include "database.h"
#include "fileindexerconfig.h"
#include "pendingfilequeue.h"
#include "../lib/autotests/xattrdetector.h"
#include "../lib/baloo_xattr_p.h"
#include "filemapping.h"

#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>

using namespace Baloo;

namespace {
    bool createFile(const QString& fileUrl) {
        QFile f1(fileUrl);
        f1.open(QIODevice::WriteOnly);
        f1.close();
        return QFile::exists(fileUrl);
    }

    void modifyFile(const QString& fileUrl) {
        QFile f1(fileUrl);
        f1.open(QIODevice::Append | QIODevice::Text);

        QTextStream stream(&f1);
        stream << "1";
    }
}
void FileWatchTest::testFileCreation()
{
    QTemporaryDir includeDir;

    QStringList includeFolders;
    includeFolders << includeDir.path();

    QStringList excludeFolders;
    Test::writeIndexerConfig(includeFolders, excludeFolders);

    QTemporaryDir dbDir;
    Database db;
    db.setPath(dbDir.path());
    db.init();

    FileIndexerConfig config;

    FileWatch fileWatch(&db, &config);
    fileWatch.m_pendingFileQueue->setTimeout(0);
    fileWatch.m_pendingFileQueue->setWaitTimeout(0);

    /*QEventLoop loop;
    connect(&fileWatch, &FileWatch::installedWatches, &loop, &QEventLoop::quit);
    loop.exec();
    */

    QSignalSpy spyIndex(&fileWatch, SIGNAL(indexFile(QString)));
    QSignalSpy spyIndexXattr(&fileWatch, SIGNAL(indexXAttr(QString)));
    QSignalSpy spyIndexRemove(&fileWatch, SIGNAL(fileRemoved(int)));

    // Create a file and see if it is indexed
    QString fileUrl(includeDir.path() + "/t1");
    QVERIFY(createFile(fileUrl));

    QTest::qWait(50);

    QCOMPARE(spyIndex.count(), 1);
    QCOMPARE(spyIndexXattr.count(), 0);
    QCOMPARE(spyIndexRemove.count(), 0);

    spyIndex.clear();
    spyIndexXattr.clear();
    spyIndexRemove.clear();

    //
    // Modify the file
    //
    modifyFile(fileUrl);
    QTest::qWait(1100);

    QCOMPARE(spyIndex.count(), 1);
    QCOMPARE(spyIndexXattr.count(), 0);
    QCOMPARE(spyIndexRemove.count(), 0);

    spyIndex.clear();
    spyIndexXattr.clear();
    spyIndexRemove.clear();

    //
    // Set an Xattr
    //
    XattrDetector detector;
    if (!detector.isSupported(dbDir.path())) {
        qWarning() << "Xattr not supported on this filesystem";
        return;
    }

    const QString userComment(QLatin1String("UserComment"));
    QVERIFY(baloo_setxattr(fileUrl, QLatin1String("user.xdg.comment"), userComment) != -1);

    QTest::qWait(1100);

    QCOMPARE(spyIndex.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 1);
    QCOMPARE(spyIndexRemove.count(), 0);

    spyIndex.clear();
    spyIndexXattr.clear();
    spyIndexRemove.clear();

    //
    // Delete the file
    //
    FileMapping fileMap(fileUrl);
    fileMap.create(db.sqlDatabase());

    QFile(fileUrl).remove();

    QTest::qWait(1100);

    QCOMPARE(spyIndex.count(), 0);
    QCOMPARE(spyIndexXattr.count(), 0);
    QCOMPARE(spyIndexRemove.count(), 1);
}

QTEST_MAIN(FileWatchTest);
