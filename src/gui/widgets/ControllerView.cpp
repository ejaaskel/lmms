/*
 * ControllerView.cpp - view-component for an controller
 *
 * Copyright (c) 2008-2009 Paul Giblock <drfaygo/at/gmail.com>
 * Copyright (c) 2011-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include <QContextMenuEvent>
#include <QLabel>
#include <QPushButton>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPainter>
#include <QInputDialog>
#include <QLayout>

#include "ControllerView.h"

#include "CaptionMenu.h"
#include "ControllerDialog.h"
#include "gui_templates.h"
#include "embed.h"
#include "GuiApplication.h"
#include "LedCheckbox.h"
#include "MainWindow.h"
#include "ToolTip.h"


ControllerView::ControllerView( Controller * _model, QWidget * _parent ) :
	QFrame( _parent ),
	ModelView( _model, this ),
	m_subWindow( nullptr ),
	m_controllerDlg( nullptr ),
	m_show( true )
{
	this->setFocusPolicy( Qt::StrongFocus );

	QVBoxLayout *vBoxLayout = new QVBoxLayout(this);

	QHBoxLayout *hBox = new QHBoxLayout();
	vBoxLayout->addLayout(hBox);

	QLabel *label = new QLabel( "<b>" + _model->displayName() + "</b>", this);
	QSizePolicy sizePolicy = label->sizePolicy();
	sizePolicy.setHorizontalStretch(1);
	label->setSizePolicy(sizePolicy);

	hBox->addWidget(label);

	QPushButton * controlsButton = new QPushButton( tr( "Controls" ), this );
	connect( controlsButton, SIGNAL( clicked() ), SLOT( editControls() ) );

	hBox->addWidget(controlsButton);

	m_nameLabel = new QLabel(_model->name(), this);
	vBoxLayout->addWidget(m_nameLabel);

	// Context menu
	Controller* c = castModel<Controller>();
	m_contextMenu = new CaptionMenu(c->name(), this);

	QAction* moveUpAction = new QAction(embed::getIconPixmap("arp_up"), tr("Move &up"), this);
	moveUpAction->setShortcut(Qt::Key_Up | Qt::AltModifier);
	moveUpAction->setShortcutContext(Qt::WidgetShortcut);
	connect(moveUpAction, &QAction::triggered, this, [this]{ emit moveUp(this); } );
	m_contextMenu->addAction(moveUpAction);
	addAction(moveUpAction);

	QAction* moveDownAction = new QAction(embed::getIconPixmap("arp_down"), tr("Move &down"), this);
	moveDownAction->setShortcut(Qt::Key_Down | Qt::AltModifier);
	moveDownAction->setShortcutContext(Qt::WidgetShortcut);
	connect(moveDownAction, &QAction::triggered, this, [this]{ emit moveDown(this); } );
	m_contextMenu->addAction(moveDownAction);
	addAction(moveDownAction);

	m_contextMenu->addSeparator();

	QAction* deleteAction = new QAction(embed::getIconPixmap("cancel"), tr("&Remove this controller"), this);
	deleteAction->setShortcut(Qt::Key_Delete);
	connect(deleteAction, &QAction::triggered, this, [this]{ emit deleteController(this); } );
	m_contextMenu->addAction(deleteAction);
	addAction(deleteAction);

	m_contextMenu->addAction(tr("Re&name this controller"), this, SLOT(renameController()));

	m_controllerDlg = getController()->createDialog( getGUI()->mainWindow()->workspace() );

	m_subWindow = getGUI()->mainWindow()->addWindowedWidget( m_controllerDlg );
	
	Qt::WindowFlags flags = m_subWindow->windowFlags();
	flags &= ~Qt::WindowMaximizeButtonHint;
	m_subWindow->setWindowFlags( flags );
	m_subWindow->setFixedSize( m_subWindow->size() );

	m_subWindow->setWindowIcon( m_controllerDlg->windowIcon() );

	connect( m_controllerDlg, SIGNAL( closed() ),
		this, SLOT( closeControls() ) );

	m_subWindow->hide();

	setModel( _model );
}




ControllerView::~ControllerView()
{
	if (m_subWindow)
	{
		delete m_subWindow;
	}
}




void ControllerView::editControls()
{
	if( m_show )
	{
		m_subWindow->show();
		m_subWindow->raise();
		m_show = false;
	}
	else
	{
		m_subWindow->hide();
		m_show = true;
	}
}




void ControllerView::closeControls()
{
	m_subWindow->hide();
	m_show = true;
}



void ControllerView::renameController()
{
	bool ok;
	Controller * c = castModel<Controller>();
	QString new_name = QInputDialog::getText( this,
			tr( "Rename controller" ),
			tr( "Enter the new name for this controller" ),
			QLineEdit::Normal, c->name() , &ok );
	if( ok && !new_name.isEmpty() )
	{
		c->setName( new_name );
		if( getController()->type() == Controller::LfoController )
		{
			m_controllerDlg->setWindowTitle( tr( "LFO" ) + " (" + new_name + ")" );
		}
		m_nameLabel->setText( new_name );
	}
}


void ControllerView::mouseDoubleClickEvent( QMouseEvent * event )
{
	renameController();
}



void ControllerView::modelChanged()
{
}



void ControllerView::contextMenuEvent(QContextMenuEvent* event)
{
	m_contextMenu->popup(mapToGlobal(event->pos()));
}
