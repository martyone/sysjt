/*
 * sysjt - SYStray Job Tracker
 * Copyright (C) 2011  Tieto Corp., Martin Kampas <martin.kampas@tieto.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtDBus/QDBusObjectPath>
#include <QtGui/QSystemTrayIcon>

#include "singleton.h"

class Job;

class JobManager : public QObject, public Singleton<JobManager>
{
	Q_OBJECT

	friend class Singleton<JobManager>;

private:
	JobManager();
	~JobManager();

public slots:
	QDBusObjectPath startJob(const QString &command);

signals:
	void jobStarted(Job *job);
	void jobFinished(Job *job);
	void jobRemoved(Job *job);

private slots:
	void onJobFinished(Job *job);
	void cleanup();

private:
	QMap<int, QPointer<Job> > m_jobs;
	QQueue<int> m_finishedJobs;
	int m_nextJobId;
};
