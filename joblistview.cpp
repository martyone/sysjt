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

#include "joblistview.h"

#include <QtCore/QDateTime>
#include <QtGui/QHeaderView>
#include <QtGui/QPainter>

#include "job.h"

namespace {
const int ICON_SIZE = 16;
const int SPOT_SIZE = 12;
}

JobListView::JobListView(QWidget *parent)
	: QTableWidget(parent)
{
	setSelectionMode(NoSelection);
	setWordWrap(false);
	setColumnCount(COLUMN_COUNT);
	horizontalHeader()->setResizeMode(ColumnJob, QHeaderView::Stretch);
	horizontalHeader()->setResizeMode(ColumnStartTime, 
			QHeaderView::ResizeToContents);
	setHorizontalHeaderLabels(QStringList()
			<<tr("Job")
			<<tr("Since"));
	verticalHeader()->hide();
	verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

void JobListView::onJobStarted(Job *job)
{
	typedef QTableWidgetItem QTWI;

	const int row = m_jobs.count();
	m_jobs.append(job);

	insertRow(row);

	QTWI *const jobItem = new QTWI(iconActive(), job->command());
	jobItem->setFlags(Qt::ItemIsEnabled);
	setItem(row, ColumnJob, jobItem);

	QTWI *const startTimeItem = new QTWI(
			QDateTime::currentDateTime().toString("hh:mm:ss"));
	startTimeItem->setFlags(Qt::ItemIsEnabled);
	setItem(row, ColumnStartTime, startTimeItem);

	if (row == 0) {
		resizeColumnToContents(ColumnStartTime);
		updateGeometry();
	}
}

void JobListView::onJobFinished(Job *job)
{
	const int row = m_jobs.indexOf(job);
	item(row, ColumnJob)->setIcon(
			job->result() == 0 ? iconSucceed() : iconFailed());
}

void JobListView::onJobRemoved(Job *job)
{
	const int row = m_jobs.indexOf(job);
	removeRow(row);
	m_jobs.removeAt(row);
}

QPixmap JobListView::iconActive()
{
  static const QPixmap icon = spot(Qt::blue);
	return icon;
}

QPixmap JobListView::iconFailed()
{
  static const QPixmap icon = spot(Qt::red);
	return icon;
}

QPixmap JobListView::iconSucceed()
{
  static const QPixmap icon = spot(Qt::green);
	return icon;
}

QPixmap JobListView::spot(Qt::GlobalColor color)
{
	Q_ASSERT((ICON_SIZE-SPOT_SIZE) % 2 == 0);

	QPixmap spot(ICON_SIZE, ICON_SIZE);
	spot.fill(Qt::transparent);
	QPainter painter(&spot);
	painter.setRenderHint(QPainter::Antialiasing);
	QPainterPath path;
	path.addEllipse((ICON_SIZE-SPOT_SIZE)/2, (ICON_SIZE-SPOT_SIZE)/2, 
			SPOT_SIZE, SPOT_SIZE);
	QRadialGradient gradient(ICON_SIZE/2, ICON_SIZE/2, SPOT_SIZE/2, SPOT_SIZE, 
			SPOT_SIZE*.8);
  gradient.setColorAt(0, Qt::white);
  gradient.setColorAt(1, color);
  painter.fillPath(path, gradient);
	return spot;
}
