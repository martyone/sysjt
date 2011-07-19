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

#include "jobmanager.h"

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>

#include "job.h"

#include "jobview_adaptor.h"

namespace {
const int CLEANUP_TIMEOUT = 5000;
}

SINGLETON_DEFINE_INSTANCE(JobManager)

JobManager::JobManager()
	: QObject(0),
		m_nextJobId(0)
{
}

JobManager::~JobManager()
{
}

QDBusObjectPath JobManager::startJob(const QString &command)
{
	if (command.isEmpty()) {
		qWarning("%s: 'command' must be specified", Q_FUNC_INFO);
		return QDBusObjectPath();
	}

	const int jobId = m_nextJobId++;

	m_jobs[jobId] = new Job(jobId, command, this);
	connect(m_jobs[jobId], SIGNAL(finished(Job*)),
			this, SLOT(onJobFinished(Job*)));
	new JobViewAdaptor(m_jobs[jobId]);
	const QString path = QString("/com/tieto/sysjt/JobView%1").arg(jobId);
	QDBusConnection::sessionBus().registerObject(path, m_jobs[jobId]);

	emit jobStarted(m_jobs[jobId]);

	return QDBusObjectPath(path);
}

void JobManager::onJobFinished(Job *job)
{
	Q_ASSERT(m_jobs.contains(job->id()));
	Q_ASSERT(!m_finishedJobs.contains(job->id()));

	m_finishedJobs.enqueue(job->id());

	emit jobFinished(job);

	QProcess::execute("aplay -q /usr/share/sounds/pop.wav");

	QTimer::singleShot(CLEANUP_TIMEOUT, this, SLOT(cleanup()));
}

void JobManager::cleanup()
{
	Q_ASSERT(!m_finishedJobs.isEmpty());

	Job *const job = m_jobs.take(m_finishedJobs.dequeue());

	emit jobRemoved(job);

	delete job;
}
