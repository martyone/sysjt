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

#include "systemtrayicon.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionFrame>
#include <QtGui/QVBoxLayout>


#include "job.h"
#include "joblistview.h"
#include "jobmanager.h"

namespace {
const int ICON_SIZE = 16;
const int SPOT_SIZE = 12;
}

class SystemTrayIcon::Popup : public QWidget
{
public:
	virtual QSize sizeHint() const
	{
		const QRect screenGeometry = qApp->desktop()->screenGeometry(this);
		return QSize(screenGeometry.width()/2.6, (screenGeometry.width()/2.6)/2.6);
	}

	virtual bool event(QEvent *event)
	{
		const bool retv = QWidget::event(event);

		if (event->type() == QEvent::LayoutRequest) {
			resize(sizeHint());
		}

		return retv;
	}

	virtual void mousePressEvent(QMouseEvent *event)
	{
		if (!rect().contains(event->pos()))
			close();
		else
			QWidget::mousePressEvent(event);
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		Q_UNUSED(e);

		QPainter p(this);
		QRegion emptyArea = QRegion(rect());

		QStyleOptionMenuItem menuOpt;
		menuOpt.initFrom(this);
		menuOpt.state = QStyle::State_None;
		menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
		menuOpt.maxIconWidth = 0;
		menuOpt.tabWidth = 0;
		style()->drawPrimitive(QStyle::PE_PanelMenu, &menuOpt, &p, this);

		const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
		if (fw) {
			QRegion borderReg;
			borderReg += QRect(0, 0, fw, height()); //left
			borderReg += QRect(width()-fw, 0, fw, height()); //right
			borderReg += QRect(0, 0, width(), fw); //top
			borderReg += QRect(0, height()-fw, width(), fw); //bottom
			p.setClipRegion(borderReg);
			emptyArea -= borderReg;
			QStyleOptionFrame frame;
			frame.rect = rect();
			frame.palette = palette();
			frame.state = QStyle::State_None;
			frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
			frame.midLineWidth = 0;
			style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
		}

		p.setClipRegion(emptyArea);
		menuOpt.state = QStyle::State_None;
		menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
		menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
		menuOpt.rect = rect();
		menuOpt.menuRect = rect();
		style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
	}

	virtual void resizeEvent(QResizeEvent *event)
	{
			QStyleHintReturnMask menuMask;
			QStyleOption option;
			option.initFrom(this);
			if (style()->styleHint(QStyle::SH_Menu_Mask, &option, this, &menuMask)) {
					setMask(menuMask.region);
			}

			QWidget::resizeEvent(event);
	}
};

SystemTrayIcon::SystemTrayIcon(QObject *parent)
	: QObject(parent),
		m_icon(new QSystemTrayIcon(this)),
		m_popup(new Popup),
		m_popupView(new JobListView),
		m_nActive(0),
		m_nSucceed(0),
		m_nFailed(0)
{
	updateIcon();

	m_popup->setWindowFlags(Qt::Popup);
	m_popup->setLayout(new QVBoxLayout);
	const int fw = m_popup->style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, 
			m_popup);
	m_popup->layout()->setContentsMargins(fw, fw, fw, fw);
	m_popup->layout()->addWidget(m_popupView);

	m_popupView->setFrameShape(QFrame::NoFrame);
	m_popupView->setFocusPolicy(Qt::NoFocus);
	m_popupView->viewport()->setAutoFillBackground(false);
	m_popupView->setShowGrid(false);

	connect(m_icon,
			SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this,
			SLOT(onIconActivated(QSystemTrayIcon::ActivationReason)));

  connect(JobManager::instance(), SIGNAL(jobStarted(Job*)),
      this, SLOT(onJobStarted(Job*)));
  connect(JobManager::instance(), SIGNAL(jobFinished(Job*)),
      this, SLOT(onJobFinished(Job*)));
  connect(JobManager::instance(), SIGNAL(jobRemoved(Job*)),
      this, SLOT(onJobRemoved(Job*)));

	m_icon->show();
}

SystemTrayIcon::~SystemTrayIcon()
{
}

void SystemTrayIcon::onJobStarted(Job *job)
{
	m_jobs.insert(job);
	m_nActive++;
	updateIcon();
	m_popupView->onJobStarted(job);
}

void SystemTrayIcon::onJobFinished(Job *job)
{
	m_nActive--;
	if (job->result() == 0)
		m_nSucceed++;
	else
		m_nFailed++;
	updateIcon();
	m_popupView->onJobFinished(job);
}

void SystemTrayIcon::onJobRemoved(Job *job)
{
	m_jobs.remove(job);
	if (job->result() == 0)
		m_nSucceed--;
	else
		m_nFailed--;
	updateIcon();
	m_popupView->onJobRemoved(job);
}

void SystemTrayIcon::onIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason != QSystemTrayIcon::Trigger)
		return;

	const QRect iconGeometry = m_icon->geometry();
	const QRect availableGeometry 
		= qApp->desktop()->availableGeometry(iconGeometry.topLeft());
	QRect geometry = QRect(QPoint(0, 0), m_popup->sizeHint());

	if (iconGeometry.left() + geometry.width() < availableGeometry.right())
		geometry.moveLeft(iconGeometry.left());
	else
		geometry.moveRight(iconGeometry.right());

	if (iconGeometry.center().y() < availableGeometry.center().y())
		geometry.moveTop(iconGeometry.bottom());
	else
		geometry.moveBottom(iconGeometry.top());

	m_popup->setGeometry(geometry);

	m_popup->show();
}

void SystemTrayIcon::updateIcon()
{
	m_icon->setIcon(icon(m_nActive, m_nFailed, m_nSucceed));
	m_icon->setToolTip(QString("%1 active, %2 succeed, %3 failed jobs")
			.arg(m_nActive).arg(m_nSucceed).arg(m_nFailed));
}

QIcon SystemTrayIcon::icon(int nActive, int nFailed, int nSucceed)
{
	Q_ASSERT(nActive >= 0 && nFailed >= 0 && nSucceed >= 0);

  static const QPixmap activeSpot = spot(Qt::blue);
  static const QPixmap failedSpot = spot(Qt::red);
  static const QPixmap succeedSpot = spot(Qt::green);
  static const QPixmap inactiveSpot = spot(Qt::gray);

	static const QColor windowColor = QPalette().color(QPalette::Window);

	QPixmap icon(ICON_SIZE, ICON_SIZE);
	icon.fill(windowColor);
	QPainter painter(&icon);
	painter.setRenderHint(QPainter::Antialiasing);

	const int nTotal = nActive + nFailed + nSucceed;

	const double x = static_cast<double>(SPOT_SIZE) / qMax(1, nTotal);

	int activeHeight = static_cast<int>(x * nActive);
	int failedHeight = static_cast<int>(x * nFailed);
	int succeedHeight = static_cast<int>(x * nSucceed);

	const int MIN_NONZERO_HEIGHT = 2;

	if (nActive != 0 && activeHeight < MIN_NONZERO_HEIGHT)
		activeHeight = 2;
	if (nFailed != 0 && failedHeight < MIN_NONZERO_HEIGHT)
		failedHeight = 2;
	if (nSucceed != 0 && succeedHeight < MIN_NONZERO_HEIGHT)
		succeedHeight = 1;

	int remainer = SPOT_SIZE - activeHeight - failedHeight - succeedHeight;

	if (remainer < 0 && activeHeight > MIN_NONZERO_HEIGHT) {
		activeHeight--;
		remainer++;
	}
	if (remainer < 0 && failedHeight > MIN_NONZERO_HEIGHT) {
		failedHeight--;
		remainer++;
	}
	if (remainer < 0 && succeedHeight > MIN_NONZERO_HEIGHT) {
		succeedHeight--;
		remainer++;
	}

	Q_ASSERT(remainer >= 0);

	int bottomOffset = ICON_SIZE - (ICON_SIZE-SPOT_SIZE)/2;
  QPixmap remainerSpot = inactiveSpot;

	if (failedHeight != 0) {
    const QRect rect(0, bottomOffset - failedHeight, ICON_SIZE, failedHeight);
    painter.drawPixmap(rect, failedSpot, rect);
		bottomOffset -= failedHeight;
    remainerSpot = failedSpot;
	}

	if (succeedHeight != 0) {
    const QRect rect(0, bottomOffset - succeedHeight, ICON_SIZE, succeedHeight);
    painter.drawPixmap(rect, succeedSpot, rect);
		bottomOffset -= succeedHeight;
    remainerSpot = succeedSpot;
	}

	if (activeHeight != 0) {
    const QRect rect(0, bottomOffset - activeHeight, ICON_SIZE, activeHeight);
    painter.drawPixmap(rect, activeSpot, rect);
		bottomOffset -= activeHeight;
    remainerSpot = activeSpot;
	}

	if (remainer > 0) {
    const QRect rect(0, bottomOffset - remainer, ICON_SIZE, remainer);
    painter.drawPixmap(rect, remainerSpot, rect);
	}

	if (nTotal > 1) {
		QFont font = painter.font();
		font.setPixelSize(7);
		const QRect rect = icon.rect().translated(0, QFontMetrics(font).descent());
		painter.setFont(font);
		painter.setPen(windowColor);
		painter.drawText(rect.translated(-1, -1), Qt::AlignRight | Qt::AlignBottom, 
				QString::number(nTotal));
		painter.setPen(Qt::black);
		painter.drawText(rect, Qt::AlignRight | Qt::AlignBottom, 
				QString::number(nTotal));
	}

	return icon;
}

QPixmap SystemTrayIcon::spot(Qt::GlobalColor color)
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
