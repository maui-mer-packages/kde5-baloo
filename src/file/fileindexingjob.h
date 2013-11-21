/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BALOO_INDEXING_JOB_H_
#define _BALOO_INDEXING_JOB_H_

#include <KJob>
#include <KUrl>
#include <QProcess>

#include "filemapping.h"

class QTimer;
class Database;

namespace Baloo
{

/**
 * \brief The one entry point to indexing files.
 *
 * The %Indexer can be used to index any file on the local file system
 * or stored in some other place like an email attachment. The resulting
 * data is automatically stored into the %Nepomuk database. The %Indexer
 * automatically removes previous data and makes sure the resource URI
 * does not change.
 *
 * This class is also used by the %Nepomuk file indexing service.
 *
 * \author Sebastian Trueg <trueg@kde.org>
 */
class FileIndexingJob : public KJob
{
    Q_OBJECT

public:
    FileIndexingJob(Database* db, const FileMapping& file, QObject* parent = 0);

    QString url() const {
        return m_file.url();
    }

    virtual void start();

    /**
     * Error codes: IndexerFailed is emitted when the indexer returns 1
     *              IndexerCrashed is emitted when the indexer crashed
     */
    enum {
        IndexerFailed = UserDefinedError,
        IndexerCrashed
    };

private Q_SLOTS:
    void slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcessTimerTimeout();
    void slotProcessNonExistingFile();

private:
    Database* m_db;

    FileMapping m_file;
    QProcess* m_process;
    int m_exitCode;
    QTimer* m_processTimer;
};
}

#endif
