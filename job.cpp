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

#include "job.h"

Job::Job(int id, const QString &command, QObject *parent)
	: QObject(parent),
		m_id(id),
		m_command(command),
		m_hasFinished(false),
		m_result(0)
{
}

void Job::stop(int result)
{
	if (m_hasFinished) {
		qWarning("%s: Job has already finished <%d>", Q_FUNC_INFO, m_id);
		return;
	}

	m_hasFinished = true;
	m_result = result;
	emit finished(this);
}
