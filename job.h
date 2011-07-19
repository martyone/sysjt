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

class Job : public QObject
{
	Q_OBJECT

public:
	Job(int id, const QString &command, QObject *parent = 0);

public:
	int id() const { return m_id; }
	QString command() const { return m_command; }
	bool hasFinished() const { return m_hasFinished; }
	int result() const { return m_result; }

public slots:
	void stop(int result);

signals:
	void finished(Job *job);

private:
	const int m_id;
	const QString m_command;
	bool m_hasFinished;
	int m_result;
};
