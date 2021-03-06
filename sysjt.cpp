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

#include <QtGui/QApplication>

#include "jobmanager.h"
#include "systemtrayicon.h"

#include "jobmanager_adaptor.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	QDBusConnection connection = QDBusConnection::sessionBus();
	connection.registerService("com.tieto.sysjt");

  JobManager::singletonInit();

	const QPointer<SystemTrayIcon> systemTrayIcon = new SystemTrayIcon;

	new JobManagerAdaptor(JobManager::instance());
	connection.registerObject("/com/tieto/sysjt/JobManager", 
			JobManager::instance());

  const int result = app.exec();

  delete systemTrayIcon;
  JobManager::singletonDestroy();

  return result;
}
