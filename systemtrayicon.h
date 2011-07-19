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
#include <QtCore/QSet>
#include <QtDBus/QDBusObjectPath>
#include <QtGui/QSystemTrayIcon>

class Job;
class JobListView;

class SystemTrayIcon : public QObject
{
	Q_OBJECT

public:
	SystemTrayIcon(QObject *parent = 0);
	~SystemTrayIcon();

public slots:
	void onJobStarted(Job *job);
	void onJobFinished(Job *job);
	void onJobRemoved(Job *job);

private slots:
	void onIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
	void updateIcon();
	static QIcon icon(int nActive, int nFailed, int nSucceed);
  static QPixmap spot(Qt::GlobalColor color);

private:
	class Popup;
	const QPointer<QSystemTrayIcon> m_icon;
	const QPointer<Popup> m_popup;
	const QPointer<JobListView> m_popupView;
	QSet<Job *> m_jobs;
	int m_nActive;
	int m_nSucceed;
	int m_nFailed;
};
