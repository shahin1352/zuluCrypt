/*
 *
 *  Copyright (c) 2012
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "deviceoffset.h"
#include "mountpartition.h"
#include "ui_mountpartition.h"
#include <QDebug>

#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCloseEvent>
#include <QByteArray>
#include <QFileDialog>
#include <QFont>
#include <QTableWidget>
#include <QAction>
#include <QMenu>
#include <QCursor>
#include <QAction>

#include "task.h"
#include "../zuluCrypt-gui/dialogmsg.h"
#include "../zuluCrypt-gui/userfont.h"
#include "../zuluCrypt-gui/tablewidget.h"
#include "../zuluCrypt-gui/utility.h"
#include "mountoptions.h"

mountPartition::mountPartition( QWidget * parent,QTableWidget * table ) : QWidget( parent ),m_ui( new Ui::mountPartition )
{
	m_ui->setupUi( this ) ;
	m_ui->checkBoxShareMountPoint->setToolTip( utility::shareMountPointToolTip() ) ;
	m_table = table ;

	this->setFixedSize( this->size() ) ;
	this->setWindowFlags( Qt::Window | Qt::Dialog ) ;
	this->setFont( parent->font() ) ;

	m_ui->pbMount->setFocus() ;

	m_ui->checkBoxMountReadOnly->setChecked( utility::getOpenVolumeReadOnlyOption( QString( "zuluMount-gui" ) ) ) ;

	connect( m_ui->pbOptions,SIGNAL( clicked() ),this,SLOT( pbOptions() ) ) ;
	connect( m_ui->pbMount,SIGNAL( clicked() ),this,SLOT( pbMount() ) ) ;
	connect( m_ui->pbMountFolder,SIGNAL( clicked() ),this,SLOT( pbOpenMountPath() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( pbCancel() ) ) ;
	connect( m_ui->checkBox,SIGNAL( stateChanged( int ) ),this,SLOT( stateChanged( int ) ) ) ;
	connect( m_ui->checkBoxMountReadOnly,SIGNAL( stateChanged(int) ),this,SLOT( checkBoxReadOnlyStateChanged( int ) ) ) ;

	m_ui->pbMountFolder->setIcon( QIcon( QString( ":/folder.png" ) ) ) ;

	userfont F( this ) ;
	this->setFont( F.getFont() ) ;

	m_ui->pbMountFolder->setVisible( false ) ;

	QAction * ac = new QAction( this ) ;
	QKeySequence s( Qt::CTRL + Qt::Key_F ) ;
	ac->setShortcut( s ) ;
	connect( ac,SIGNAL( triggered() ),this,SLOT( showOffSetWindowOption() ) ) ;
	this->addAction( ac ) ;

	m_menu = new QMenu( this ) ;

	m_menu->addAction( tr( "set file system options" ) ) ;
	m_menu->addAction( tr( "set volume offset" ) ) ;

	connect( m_menu,SIGNAL( triggered( QAction * ) ),this,SLOT( doAction( QAction * ) ) ) ;

	this->installEventFilter( this ) ;
}

bool mountPartition::eventFilter( QObject * watched,QEvent * event )
{
	if( utility::eventFilter( this,watched,event ) ){
		this->HideUI() ;
		return true ;
	}else{
		return false ;
	}
}

void mountPartition::checkBoxReadOnlyStateChanged( int state )
{
	m_ui->checkBoxMountReadOnly->setEnabled( false ) ;
	m_ui->checkBoxMountReadOnly->setChecked( utility::setOpenVolumeReadOnly( this,state == Qt::Checked,QString( "zuluMount-gui" ) ) ) ;

	m_ui->checkBoxMountReadOnly->setEnabled( true ) ;
	if( m_ui->lineEdit->text().isEmpty() ){
		m_ui->lineEdit->setFocus() ;
	}else{
		m_ui->pbMount->setFocus() ;
	}
}

void mountPartition::enableAll()
{
	if( m_label != QString( "Nil" ) ){
		m_ui->checkBox->setEnabled( true ) ;
	}
	m_ui->checkBoxMountReadOnly->setEnabled( true ) ;
	m_ui->checkBoxShareMountPoint->setEnabled( true ) ;
	m_ui->labelMountPoint->setEnabled( true ) ;
	m_ui->lineEdit->setEnabled( true ) ;
	m_ui->pbCancel->setEnabled( true ) ;
	m_ui->pbMount->setEnabled( true ) ;
	m_ui->pbMountFolder->setEnabled( true ) ;
	m_ui->pbOptions->setEnabled( true ) ;
}

void mountPartition::disableAll()
{
	m_ui->pbMount->setEnabled( false ) ;
	m_ui->checkBox->setEnabled( false ) ;
	m_ui->checkBoxMountReadOnly->setEnabled( false ) ;
	m_ui->checkBoxShareMountPoint->setEnabled( false ) ;
	m_ui->labelMountPoint->setEnabled( false ) ;
	m_ui->lineEdit->setEnabled( false ) ;
	m_ui->pbCancel->setEnabled( false ) ;
	m_ui->pbMountFolder->setEnabled( false ) ;
	m_ui->pbOptions->setEnabled( false ) ;
}

void mountPartition::pbCancel()
{
	emit cancel() ;
	this->HideUI() ;
}

void mountPartition::pbMount()
{
	QString test_mount = m_ui->lineEdit->text() ;

	if( test_mount.contains( QString( "/" ) ) ){
		if( this->isVisible() ){
			DialogMsg msg( this ) ;
			msg.ShowUIOK( tr( "ERROR" ),tr( "\"/\" character is not allowed in the mount name field" ) ) ;
			m_ui->lineEdit->setFocus() ;
		}else{
			this->deleteLater() ;
		}
		return ;
	}

	this->disableAll() ;

	Task * t = new Task() ;
	t->setDevice( m_path ) ;

	QString addr = utility::keyPath() ;
	t->setKeySource( QString( "-f ") + addr ) ;

	if( !m_deviceOffSet.isEmpty() ){
		Task * e = new Task() ;
		e->setKey( m_key ) ;
		e->setKeyPath( addr ) ;
		e->start( Task::sendKey ) ;
	}
	if( m_options.isEmpty() ){
		if( m_ui->checkBoxMountReadOnly->isChecked() ){
			t->setMode( QString( "ro" ) ) ;
		}else{
			t->setMode( QString( "rw" ) ) ;
		}
	}else{
		if( m_ui->checkBoxMountReadOnly->isChecked() ){
			t->setMode( QString( "ro -Y %1" ).arg( m_options ) ) ;
		}else{
			t->setMode( QString( "rw -Y %1" ).arg( m_options ) ) ;
		}
	}

	m_point = m_ui->lineEdit->text() ;

	t->setMountPoint( utility::mountPath( m_point ) ) ;
	t->setDeviceOffSet( m_deviceOffSet ) ;

	connect( t,SIGNAL( signalMountComplete( int,QString ) ),this,SLOT( slotMountComplete( int,QString ) ) ) ;

	t->setMakeMountPointPublic( m_ui->checkBoxShareMountPoint->isChecked() ) ;
	t->start( Task::Mount ) ;
}

void mountPartition::showOffSetWindowOption()
{
	deviceOffset * d = new deviceOffset( this ) ;
	connect( d,SIGNAL( offSetValue( QString,QString ) ),this,SLOT( deviceOffSet( QString,QString ) ) ) ;
	d->ShowUI() ;
}

void mountPartition::showFileSystemOptionWindow()
{
	mountOptions * m = new mountOptions( &m_options,this ) ;
	m->ShowUI() ;
}

void mountPartition::doAction( QAction * ac )
{
	if( ac->text() == tr( "set file system options" ) ){
		this->showFileSystemOptionWindow() ;
	}else{
		this->showOffSetWindowOption() ;
	}
}

void mountPartition::pbOptions()
{
	m_menu->exec( QCursor::pos() ) ;
}

void mountPartition::pbOpenMountPath()
{
	QString p = tr( "Select Path to mount point folder" ) ;
	QString Z = QFileDialog::getExistingDirectory( this,p,QDir::homePath(),QFileDialog::ShowDirsOnly ) ;

	if( !Z.isEmpty() ){
		Z = Z + QString( "/" ) + m_ui->lineEdit->text().split( "/" ).last() ;
		m_ui->lineEdit->setText( Z ) ;
	}
}

void mountPartition::ShowUI( QString path,QString label )
{
	m_path = path ;
	m_label = label ;
	m_point = m_path.split( QString( "/" ) ).last() ;
	m_ui->lineEdit->setText( m_point ) ;

	if( label == QString( "Nil" ) ){
		m_ui->checkBox->setEnabled( false ) ;
	}
	this->show() ;
}

void mountPartition::AutoMount( QStringList entry )
{
	m_path = entry.at( 0 ) ;
	QString label = entry.at( 3 ) ;
	if( label != QString( "Nil" ) ) {
		m_point = label ;
	}else{
		m_point = m_path.split( QString( "/" ) ).last() ;
	}
	m_ui->lineEdit->setText( m_point ) ;
	this->pbMount() ;
}

void mountPartition::stateChanged( int i )
{
	Q_UNUSED( i ) ;
	m_ui->checkBox->setEnabled( false ) ;
	if( m_ui->checkBox->isChecked() ){
		m_ui->lineEdit->setText( m_label ) ;
	}else{
		m_ui->lineEdit->setText( m_path.split( QString( "/" ) ).last() ) ;
	}
	m_ui->checkBox->setEnabled( true ) ;
}

void mountPartition::deviceOffSet( QString deviceOffSet,QString key )
{
	m_deviceOffSet = deviceOffSet ;
	m_key = key ;
}

void mountPartition::slotMountComplete( int status,QString msg )
{
	if( status ){
		if( this->isVisible() ){
			DialogMsg m( this ) ;
			m.ShowUIOK( tr( "ERROR" ),msg ) ;
			this->enableAll() ;
		}else{
			this->deleteLater() ;
		}
	}else{
		emit openMountPoint( utility::mountPath( m_point ) ) ;
		this->HideUI() ;
	}
}

void mountPartition::HideUI()
{
	this->hide() ;
	this->deleteLater() ;
}

void mountPartition::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbCancel() ;
}

mountPartition::~mountPartition()
{
	delete m_ui ;
}


