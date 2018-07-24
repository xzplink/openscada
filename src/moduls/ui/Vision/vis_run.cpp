
//OpenSCADA system module UI.Vision file: vis_run.cpp
/***************************************************************************
 *   Copyright (C) 2007-2018 by Roman Savochenko, <rom_as@oscada.org>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>

#include <linux/input.h>

#include <QApplication>
#include <QLocale>
#include <QDesktopWidget>
#include <QTimer>
#include <QMenuBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QStatusBar>
#include <QWhatsThis>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPainter>
#include <QToolBar>
#include <QPrintDialog>
#include <QDateTime>
#include <QTextStream>

// #include <config.h>
#include <tsys.h>
#include "tvision.h"
#include "vis_run_widgs.h"
#include "vis_shapes.h"
#include "vis_run.h"

using namespace VISION;

VisRun::VisRun( const string &iprjSes_it, const string &open_user, const string &user_pass, const string &VCAstat,
		bool icrSessForce, unsigned iScr ) :
    QMainWindow(QDesktopWidget().screen(iScr)),
#ifndef QT_NO_PRINTER
    prPg(NULL), prDiag(NULL), prDoc(NULL),
#endif
    fileDlg(NULL),
    winClose(false), conErr(NULL), crSessForce(icrSessForce), keepAspectRatio(false), prjSes_it(iprjSes_it),
    master_pg(NULL), mPeriod(1000), mConId(0), mScreen(iScr), wPrcCnt(0), reqtm(1), expDiagCnt(1), expDocCnt(1), x_scale(1), y_scale(1),
    mAlrmSt(0xFFFFFF), updPage(false), host(NULL)
{
    QImage ico_t;

    connect(this, SIGNAL(makeStarterMenu()), qApp, SLOT(makeStarterMenu()));
    setAttribute(Qt::WA_DeleteOnClose, true);
    mod->regWin(this);

    setWindowTitle(_("Vision runtime"));
    setWindowIcon(mod->icon());

    //Create actions
    // Generic actions
    //  Print
    if(!ico_t.load(TUIS::icoPath("print").c_str())) ico_t.load(":/images/print.png");
    QMenu *menuPrint = new QMenu(_("&Print"), this);
    menuPrint->setIcon(QPixmap::fromImage(ico_t));
    menuPrint->menuAction()->setShortcut(QKeySequence::Print);
    menuPrint->menuAction()->setToolTip(_("Print the master page"));
    menuPrint->menuAction()->setWhatsThis(_("The button for printing of the master page by default"));
    menuPrint->menuAction()->setStatusTip(_("Press for printing of the master page by default."));
    connect(menuPrint->menuAction(), SIGNAL(triggered()), this, SLOT(print()));
    QAction *actPrintPg = new QAction(_("Page"),this);
    actPrintPg->setToolTip(_("Print the selected page"));
    actPrintPg->setWhatsThis(_("The button for printing of the selected page"));
    actPrintPg->setStatusTip(_("Press for printing of the selected page."));
    connect(actPrintPg, SIGNAL(triggered()), this, SLOT(printPg()));
    menuPrint->addAction(actPrintPg);
    QAction *actPrintDiag = new QAction(_("Diagram"),this);
    actPrintDiag->setToolTip(_("Print the selected diagram"));
    actPrintDiag->setWhatsThis(_("The button for printing of the selected diagram"));
    actPrintDiag->setStatusTip(_("Press for printing of the selected diagram."));
    connect(actPrintDiag, SIGNAL(triggered()), this, SLOT(printDiag()));
    menuPrint->addAction(actPrintDiag);
    QAction *actPrintDoc = new QAction(_("Document"),this);
    actPrintDoc->setToolTip(_("Print the selected document"));
    actPrintDoc->setWhatsThis(_("The button for printing of the selected document"));
    actPrintDoc->setStatusTip(_("Press for printing of the selected document."));
    connect(actPrintDoc, SIGNAL(triggered()), this, SLOT(printDoc()));
    menuPrint->addAction(actPrintDoc);
    //  Export
    if(!ico_t.load(TUIS::icoPath("export").c_str())) ico_t.load(":/images/export.png");
    QMenu *menuExport = new QMenu(_("&Export"), this);
    menuExport->setIcon(QPixmap::fromImage(ico_t));
    menuExport->menuAction()->setToolTip(_("Export the master page"));
    menuExport->menuAction()->setWhatsThis(_("The button for exporting of the master page by default"));
    menuExport->menuAction()->setStatusTip(_("Press for exporting of the master page by default."));
    connect(menuExport->menuAction(), SIGNAL(triggered()), this, SLOT(exportDef()));
    QAction *actExpPg = new QAction(_("Page"),this);
    actExpPg->setToolTip(_("Export the selected page"));
    actExpPg->setWhatsThis(_("The button for exporting of the selected page"));
    actExpPg->setStatusTip(_("Press for exporting of the selected page."));
    connect(actExpPg, SIGNAL(triggered()), this, SLOT(exportPg()));
    menuExport->addAction(actExpPg);
    QAction *actExpDiag = new QAction(_("Diagram"),this);
    actExpDiag->setToolTip(_("Export the selected diagram"));
    actExpDiag->setWhatsThis(_("The button for exporting of the selected diagram"));
    actExpDiag->setStatusTip(_("Press for exporting of the selected diagram."));
    connect(actExpDiag, SIGNAL(triggered()), this, SLOT(exportDiag()));
    menuExport->addAction(actExpDiag);
    QAction *actExpDoc = new QAction(_("Document"),this);
    actExpDoc->setToolTip(_("Export the selected document"));
    actExpDoc->setWhatsThis(_("The button for exporting of the selected document"));
    actExpDoc->setStatusTip(_("Press for exporting of the selected document."));
    connect(actExpDoc, SIGNAL(triggered()), this, SLOT(exportDoc()));
    menuExport->addAction(actExpDoc);
    //  Close
    if(!ico_t.load(TUIS::icoPath("close").c_str())) ico_t.load(":/images/close.png");
    QAction *actClose = new QAction(QPixmap::fromImage(ico_t),_("&Close"),this);
    actClose->setShortcut(Qt::CTRL+Qt::Key_W);
    actClose->setToolTip(_("Close the Vision window"));
    actClose->setWhatsThis(_("The button for closing the Vision runtime window"));
    actClose->setStatusTip(_("Press to close of the current Vision runtime window."));
    connect(actClose, SIGNAL(triggered()), this, SLOT(close()));
    //  Quit
    if(!ico_t.load(TUIS::icoPath("exit").c_str())) ico_t.load(":/images/exit.png");
    QAction *actQuit = new QAction(QPixmap::fromImage(ico_t),_("&Quit"),this);
    actQuit->setShortcut(Qt::CTRL+Qt::Key_Q);
    actQuit->setToolTip(_("Quit the program"));
    actQuit->setWhatsThis(_("The button for complete quit the program"));
    actQuit->setStatusTip(_("Press for complete quit the program."));
    connect(actQuit, SIGNAL(triggered()), this, SLOT(quitSt()));
    // View actions
    //  Fullscreen
    actFullScr = new QAction(_("Full screen"),this);
    actFullScr->setCheckable(true);
    actFullScr->setToolTip(_("Full screen mode toggling"));
    actFullScr->setWhatsThis(_("The button for full screen mode toggling"));
    actFullScr->setStatusTip(_("Press for toggling the full screen mode."));
    connect(actFullScr, SIGNAL(toggled(bool)), this, SLOT(fullScreen(bool)));

    // Help actions
    //  About "System info"
    if(!ico_t.load(TUIS::icoPath("help").c_str())) ico_t.load(":/images/help.png");
    QAction *actAbout = new QAction(QPixmap::fromImage(ico_t),_("&About"),this);
    actAbout->setShortcut(Qt::Key_F1);
    actAbout->setToolTip(_("Program and OpenSCADA information"));
    actAbout->setWhatsThis(_("The button of the information of the program and OpenSCADA"));
    actAbout->setStatusTip(_("Press for information of the program and OpenSCADA."));
    connect(actAbout, SIGNAL(triggered()), this, SLOT(about()));
    //  About Qt
    QAction *actQtAbout = new QAction(_("About &Qt"),this);
    actQtAbout->setToolTip(_("Qt information"));
    actQtAbout->setWhatsThis(_("The button for getting the using Qt information"));
    actQtAbout->setStatusTip(_("Press for getting the using Qt information."));
    connect(actQtAbout, SIGNAL(triggered()), this, SLOT(aboutQt()));
    //  What is
    //if(!ico_t.load(TUIS::icoPath("contexthelp").c_str())) ico_t.load(":/images/contexthelp.png");
    //QAction *actWhatIs = new QAction(QPixmap::fromImage(ico_t),_("What's &This"),this);
    //actWhatIs->setToolTip(_("The button for requestion about GUI elements"));
    //actWhatIs->setWhatsThis(_("Get request about user interface elements"));
    //actWhatIs->setStatusTip(_("Press for requesting about user interface elements."));
    //connect(actWhatIs, SIGNAL(triggered()), this, SLOT(enterWhatsThis()));

    // Alarms actions
    //  Alarm level display button and full alarms quietance
    if(!ico_t.load(TUIS::icoPath("alarmLev").c_str())) ico_t.load(":/images/alarmLev.png");
    actAlrmLev = new QAction(QPixmap::fromImage(ico_t), _("Alarm level"), this);
    actAlrmLev->setObjectName("alarmLev");
    actAlrmLev->setToolTip(_("Alarm level"));
    actAlrmLev->setWhatsThis(_("The button for quietance all alarms"));
    actAlrmLev->setStatusTip(_("Press for quietance all alarms."));
    //  Alarm by Light
    if(!ico_t.load(TUIS::icoPath("alarmLight").c_str())) ico_t.load(":/images/alarmLight.png");
    actAlrmLight = new QAction(QPixmap::fromImage(ico_t), _("Blink alarm"), this);
    actAlrmLight->setObjectName("alarmLight");
    actAlrmLight->setToolTip(_("Blink alarm"));
    actAlrmLight->setWhatsThis(_("The button for all blink alarms quietance"));
    actAlrmLight->setStatusTip(_("Press for all blink alarms quietance."));
    actAlrmLight->setVisible(false);
    //  Alarm by mono sound (PC speaker)
    if(!ico_t.load(TUIS::icoPath("alarmAlarm").c_str())) ico_t.load(":/images/alarmAlarm.png");
    actAlrmAlarm = new QAction(QPixmap::fromImage(ico_t), _("Speaker alarm"), this);
    actAlrmAlarm->setObjectName("alarmAlarm");
    actAlrmAlarm->setToolTip(_("PC speaker alarm"));
    actAlrmAlarm->setWhatsThis(_("The button for all PC speaker alarms quietance"));
    actAlrmAlarm->setStatusTip(_("Press for all PC speaker alarms quietance."));
    actAlrmAlarm->setVisible(false);
    //  Alarm by sound or synthesis of speech
    if(!ico_t.load(TUIS::icoPath("alarmSound").c_str())) ico_t.load(":/images/alarmSound.png");
    actAlrmSound = new QAction(QPixmap::fromImage(ico_t), _("Sound/speech alarm"), this);
    actAlrmSound->setObjectName("alarmSound");
    actAlrmSound->setToolTip(_("Sound or speech alarm"));
    actAlrmSound->setWhatsThis(_("The button for all sound or speech alarms quietance"));
    actAlrmSound->setStatusTip(_("Press for all sound or speech alarms quietance."));
    actAlrmSound->setVisible(false);

    menuFile.setTitle(_("&File"));
    menuFile.addAction(menuPrint->menuAction());
    menuFile.addAction(menuExport->menuAction());
    menuFile.addSeparator();
    menuFile.addAction(actClose);
    menuFile.addAction(actQuit);
    menuAlarm.setTitle(_("&Alarm"));
    menuAlarm.addAction(actAlrmLev);
    menuAlarm.addAction(actAlrmLight);
    menuAlarm.addAction(actAlrmAlarm);
    menuAlarm.addAction(actAlrmSound);
    menuView.setTitle(_("&View"));
    menuView.addAction(actFullScr);
    menuHelp.setTitle(_("&Help"));
    menuHelp.addAction(actAbout);
    menuHelp.addAction(actQtAbout);
    menuHelp.addSeparator();
    //menuHelp.addAction(actWhatIs);

    //Init tool bars
    // Generic tools bar
    toolBarStatus = new QToolBar(_("Generic (status)"),this);
    connect(toolBarStatus, SIGNAL(actionTriggered(QAction*)), this, SLOT(alarmAct(QAction*)));
    toolBarStatus->setIconSize(QSize(icoSize(),icoSize()));
    toolBarStatus->addSeparator();
    toolBarStatus->addAction(menuPrint->menuAction());
    toolBarStatus->addAction(menuExport->menuAction());
    toolBarStatus->addSeparator();
    toolBarStatus->addAction(actAlrmLev);
    toolBarStatus->addAction(actAlrmLight);
    toolBarStatus->addAction(actAlrmAlarm);
    toolBarStatus->addAction(actAlrmSound);

    //Init status bar
    mWTime = new QLabel(this);
    mWTime->setVisible(false);
    mWTime->setAlignment(Qt::AlignCenter);
    mWTime->setWhatsThis(_("This label shows the current system time."));
    statusBar()->insertPermanentWidget(0, mWTime);
    mWUser = new UserStBar(open_user.c_str(), user_pass.c_str(), VCAstat.c_str(), this);
    mWUser->setWhatsThis(_("This label displays the current user."));
    mWUser->setToolTip(_("Field for displaying the current user."));
    mWUser->setStatusTip(_("Double click to change the user."));
    connect(mWUser, SIGNAL(userChanged(const QString&,const QString&)), this, SLOT(userChanged(const QString&,const QString&)));
    statusBar()->insertPermanentWidget(0, mWUser);
    mWStat = new QLabel(VCAStation().c_str(), this);
    mWStat->setWhatsThis(_("This label displays the used VCA engine station."));
    mWStat->setToolTip(_("Field for displaying the used VCA engine station."));
    mWStat->setVisible( VCAStation() != "." );
    statusBar()->insertPermanentWidget(0,mWStat);
    mStlBar = new StylesStBar(-1, this);
    mStlBar->setWhatsThis(_("This label displays the used interface style."));
    mStlBar->setToolTip(_("Field for displaying the used interface style."));
    mStlBar->setStatusTip(_("Double click to change the style."));
    connect(mStlBar, SIGNAL(styleChanged()), this, SLOT(styleChanged()));
    statusBar()->insertPermanentWidget(0, mStlBar);
    statusBar()->insertPermanentWidget(0, toolBarStatus);
    statusBar()->setVisible(mod->runPrjsSt());

    //Init scroller
    QScrollArea *scrl = new QScrollArea;
    scrl->setFocusPolicy(Qt::NoFocus);
    scrl->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrl);

    //Create timers
    // End run timer
    endRunTimer   = new QTimer(this);
    endRunTimer->setSingleShot(false);
    connect(endRunTimer, SIGNAL(timeout()), this, SLOT(endRunChk()));
    endRunTimer->start(STD_WAIT_DELAY);
    // Update timer
    updateTimer = new QTimer(this);
    updateTimer->setSingleShot(false);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePage()));

    alrmPlay = new SndPlay(this);

    resize(600, 400);

    //Establish connection to the remote station
    // !!!! Disable by default the requesting into different thread before resolve the mouse release event in the same widget processing the press event.
    if(SYS->cmdOptPresent("ReqInDifThread")) initHost();

    initSess(prjSes_it, crSessForce);	//init session

    //mWStat->setText(host.st_nm.c_str());
    statusBar()->showMessage(_("Ready"), 2000);

    //Restore main window position
    if(mod->winPosCntrSave() && masterPg()) {
	string xPos, yPos;
	if((xPos=wAttrGet(masterPg()->id(),i2s(screen())+"geomX",true)).size() &&
		(yPos=wAttrGet(masterPg()->id(),i2s(screen())+"geomY",true)).size())
	    move(s2i(xPos), s2i(yPos));
	else if(abs(masterPg()->posF().x()) || abs(masterPg()->posF().y()))
	    move(masterPg()->posF().x(), masterPg()->posF().y());
    }

    alarmSet(0);
}

VisRun::~VisRun( )
{
    winClose = true;

    endRunTimer->stop();
    updateTimer->stop();

    alarmSet(0);
    alrmPlay->wait();

    //Disconnect/delete session
    XMLNode req("disconnect");
    req.setAttr("path", "/%2fserv%2fsess")->setAttr("sess", work_sess)->setAttr("conId", i2s(mConId));
    cntrIfCmd(req);

    //Unregister window
    mod->unregWin(this);

    //Clear cache
    pgCacheClear();

#ifndef QT_NO_PRINTER
    //Print objects free
    if(prPg)	delete prPg;
    if(prDiag)	delete prDiag;
    if(prDoc)	delete prDoc;
    if(fileDlg)	delete fileDlg;
#endif

    if(host && host->inHostReq)
	mess_err(mod->nodePath().c_str(), _("Session '%s(%s)' using the remote host %d times."),
	    workSess().c_str(), srcProject().c_str(), host->inHostReq);

    if(host) {
	delete host;

	// Push down all Qt events of the window to free the module
	for(int iTr = 0; iTr < 5; iTr++) qApp->processEvents();
    }
}

bool VisRun::winMenu( )	{ return menuBar()->actions().length(); }

void VisRun::setWinMenu( bool act )
{
    menuBar()->clear();

    //Create menu
    if(act) {
	menuBar()->addMenu(&menuFile);
	menuBar()->addMenu(&menuAlarm);
	menuBar()->addMenu(&menuView);
	menuBar()->addMenu(&menuHelp);
	emit makeStarterMenu();
    }
}

string VisRun::user( )		{ return mWUser->user(); }

bool VisRun::userSel( const string &hint )	{ return mWUser->userSel(hint); }

string VisRun::password( )	{ return mWUser->pass(); }

string VisRun::VCAStation( )	{ return mWUser->VCAStation(); }

int VisRun::style( )		{ return mStlBar->style(); }

void VisRun::setStyle( int istl )		{ mStlBar->setStyle(istl); }

void VisRun::initHost( )
{
    if(!host) {
	host = new SCADAHost(this);
	host->start();
    }
}

int VisRun::cntrIfCmd( XMLNode &node, bool glob, bool main )
{
    if(masterPg() && conErr && (!main || (time(NULL)-conErr->property("tm").toLongLong()) < conErr->property("tmRest").toInt())) {
	if(main && conErr->property("labTmpl").toString().size())
	    conErr->setText(conErr->property("labTmpl").toString().arg(conErr->property("tmRest").toInt()-(time(NULL)-conErr->property("tm").toLongLong())));
	return 10;
    }

    int rez = 0;
    if(host) {
	host->inHostReq++;
	while(host->reqBusy()) {
	    qApp->processEvents();
	    TSYS::sysSleep(0.01);
	}
	//Do and wait for the request
	bool done = false;
	if(!host->reqDo(node,done,glob))
	    while(!done) {
		qApp->processEvents();
		TSYS::sysSleep(0.01);
	    }
	host->inHostReq--;
	if(winClose && !host->inHostReq) close();

	rez = s2i(node.attr("rez"));
    }
    else rez = mod->cntrIfCmd(node, user(), password(), VCAStation(), glob);

    //Display error message about connection error
    if(rez == 10 && main && masterPg()) {
	if(!conErr) {
	    //Create error message
	    conErr = new QLabel(masterPg());
	    conErr->setAlignment(Qt::AlignCenter);
	    conErr->setWordWrap(true);
	    //Prepare message's style
	    conErr->setFrameStyle(QFrame::StyledPanel|QFrame::Raised);
	    conErr->setAutoFillBackground(true);
	    QPalette plt(conErr->palette());
	    QBrush brsh = plt.brush(QPalette::Background);
	    brsh.setColor(Qt::red);
	    brsh.setStyle(Qt::SolidPattern);
	    plt.setBrush(QPalette::Background, brsh);
	    conErr->setPalette(plt);
	    //Calc size and position
	    conErr->resize(300, 100);
	    conErr->move((masterPg()->size().width()-conErr->size().width())/2, (masterPg()->size().height()-conErr->size().height())/2);
	    //conErr->show();
	    conErr->setProperty("tmRest", 0);
	} else conErr->setProperty("tmRest", vmin(mod->restoreTime(),conErr->property("tmRest").toInt()+1));
	conErr->setProperty("tm", (long long)time(NULL));
	if(conErr->property("tmRest").toInt() > 3) {
	    if(!conErr->isVisible()) conErr->show();
	    conErr->setProperty("labTmpl",
		QString(_("Error connecting to the visualization server '%1': %2.\nThe next recovery attempt after %3s!"))
		    .arg(VCAStation().c_str()).arg(node.text().c_str()).arg("%1"));
	    conErr->setText(conErr->property("labTmpl").toString().arg(conErr->property("tmRest").toInt()));
	}
    }
    //Remove the error message about the connection error
    else if(rez != 10 && main && conErr) {
	if(masterPg()) conErr->deleteLater();
	conErr = NULL;
    }

    return rez;
}

QString VisRun::getFileName( const QString &caption, const QString &dir, const QString &filter, QFileDialog::AcceptMode mode )
{
    if(!fileDlg) fileDlg = new QFileDialog(this);
    fileDlg->setFileMode(QFileDialog::AnyFile);
    fileDlg->setAcceptMode(mode);
    fileDlg->setWindowTitle(caption);
    fileDlg->setNameFilter(filter);
    if(dir.size()) { QString dirF = dir; fileDlg->selectFile(dirF.replace("\"","")); }
#if QT_VERSION >= 0x040500
    fileDlg->setReadOnly(!winMenu());
#endif
    if(fileDlg->exec() && !fileDlg->selectedFiles().empty()) return fileDlg->selectedFiles()[0];

    return "";
}

void VisRun::closeEvent( QCloseEvent* ce )
{
    winClose = true;

    //Call for next processing by the events handler for the real closing after release all background requests
    if(host && host->inHostReq) { ce->ignore(); /*QCoreApplication::postEvent(this, new QCloseEvent());*/ return; }

    if(endRunTimer->isActive()) {
	//Save main window position
	if(mod->winPosCntrSave() && masterPg()) {
	    wAttrSet(masterPg()->id(), i2s(screen())+"geomX", i2s(pos().x()), true);
	    wAttrSet(masterPg()->id(), i2s(screen())+"geomY", i2s(pos().y()), true);
	}

	//Exit on close last run project
	if(mod->exitLstRunPrjCls() && masterPg()) {
	    unsigned winCnt = 0;
	    for(int iW = 0; iW < QApplication::topLevelWidgets().size(); iW++)
		if(qobject_cast<QMainWindow*>(QApplication::topLevelWidgets()[iW]) && QApplication::topLevelWidgets()[iW]->isVisible())
		    winCnt++;

	    if(winCnt <= 1 && !qApp->property("closeToTray").toBool()) SYS->stop();
	}

	endRunTimer->stop();
	updateTimer->stop();
    }

    ce->accept();
}

void VisRun::resizeEvent( QResizeEvent *ev )
{
    if(masterPg()) {
	float x_scale_old = x_scale;
	float y_scale_old = y_scale;
	if(windowState()&(Qt::WindowMaximized|Qt::WindowFullScreen)) {
	    x_scale = (float)((QScrollArea*)centralWidget())->maximumViewportSize().width()/(masterPg()->sizeOrigF().width()*masterPg()->xScale());
	    y_scale = (float)((QScrollArea*)centralWidget())->maximumViewportSize().height()/(masterPg()->sizeOrigF().height()*masterPg()->yScale());
	    if(x_scale > 1 && x_scale < 1.02) x_scale = 1;
	    if(y_scale > 1 && y_scale < 1.02) y_scale = 1;
	    if(keepAspectRatio) x_scale = y_scale = vmin(x_scale, y_scale);
	} else x_scale = y_scale = 1;
	if(x_scale_old != x_scale || y_scale_old != y_scale) fullUpdatePgs();

	// Fit to the master page size
	if((x_scale_old != x_scale || y_scale_old != y_scale || !ev || !ev->oldSize().isValid()) && !(windowState()&(Qt::WindowMaximized|Qt::WindowFullScreen))) {
	    QRect ws = QApplication::desktop()->availableGeometry(this);
	    resize(fmin(ws.width()-10,masterPg()->size().width()+(centralWidget()->parentWidget()->width()-centralWidget()->width())+5),
		fmin(ws.height()-10,masterPg()->size().height()+(centralWidget()->parentWidget()->height()-centralWidget()->height())+5));
	}

	mess_debug(mod->nodePath().c_str(), _("Scale of the root page [%f:%f]."), x_scale, y_scale);
    }
    mWTime->setVisible(windowState()==Qt::WindowFullScreen);
}

void VisRun::endRunChk( )
{
    if(mod->endRun()) close();
}

void VisRun::quitSt( )
{
    SYS->stop();
}

void VisRun::print( )
{
    if(masterPg()) {
	//Check for the single and big document present for default the printing
	RunPageView *rpg;
	RunWdgView *rwdg;
	vector<string> lst;
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		rpg->shapeList("Document",lst);
	if(lst.size() == 1 && (rwdg=findOpenWidget(lst[0])) &&
		((masterPg()->width()/vmax(1,rwdg->width())) < 2 || (masterPg()->height()/vmax(1,rwdg->height())) < 2))
	    printDoc(rwdg->id());
	//Print master page
	else printPg(masterPg()->id());
    }
}

void VisRun::printPg( const string &ipg )
{
#ifndef QT_NO_PRINTER
    RunPageView *rpg;
    string pg = ipg;

    if(pgList.empty())	{ QMessageBox::warning(this,_("Printing a page"),_("There is no page to print!")); return; }

    if(pg.empty() && pgList.size() == 1)	pg = pgList[0];
    if(pg.empty() && pgList.size() > 1) {
	//Make select page dialog
	QImage ico_t;
	if(!ico_t.load(TUIS::icoPath("print").c_str())) ico_t.load(":/images/print.png");
	InputDlg sdlg(this, QPixmap::fromImage(ico_t), _("Select a page to print."), _("Printing a page"), false, false);
	sdlg.edLay()->addWidget(new QLabel(_("Pages:"),&sdlg), 2, 0);
	QComboBox *spg = new QComboBox(&sdlg);
	sdlg.edLay()->addWidget(spg, 2, 1);
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		spg->addItem((rpg->name()+" ("+pgList[i_p]+")").c_str(),pgList[i_p].c_str());
	if(sdlg.exec() != QDialog::Accepted)	return;
	pg = spg->itemData(spg->currentIndex()).toString().toStdString();
    }

    //Find need page
    rpg = master_pg;
    if(rpg->id() != pg)	rpg = findOpenPage(pg);
    if(!rpg) return;

    string pnm = rpg->name();
    if(!prPg)	prPg = new QPrinter(QPrinter::HighResolution);
    QPrintDialog dlg(prPg, this);
    dlg.setWindowTitle(QString(_("Printing the page: \"%1\" (%2)")).arg(pnm.c_str()).arg(pg.c_str()));
    if(dlg.exec() == QDialog::Accepted) {
	int fntSize = 35;
	QSize papl(2048,2048*prPg->paperRect().height()/prPg->paperRect().width());
	QSize pagl(papl.width()*prPg->pageRect().width()/prPg->paperRect().width(), papl.height()*prPg->pageRect().height()/prPg->paperRect().height());

	QPainter painter;
	painter.begin(prPg);
	painter.setWindow(QRect(QPoint(0,0),papl));
	painter.setViewport(prPg->paperRect());

	//Draw image
# if QT_VERSION >= 0x050000
	QImage im = rpg->grab().toImage();
# else
	QImage im = QPixmap::grabWidget(rpg).toImage();
# endif
	im = im.scaled(QSize(vmin(im.width()*4,pagl.width()),vmin(im.height()*4,pagl.height()-2*fntSize)),Qt::KeepAspectRatio/*,Qt::SmoothTransformation*/);
	painter.drawImage((pagl.width()-im.width())/2,fntSize,im);

	//Draw notes
	painter.setPen(Qt::black);
	QFont fnt("Arial");
	fnt.setPixelSize(fntSize-5);
	painter.setFont(fnt);
	painter.drawText(QRect(0,0,pagl.width(),fntSize),Qt::AlignLeft,QString(_("OpenSCADA project: \"%1\"")).arg(windowTitle()));
	painter.drawText(QRect(0,0,pagl.width(),fntSize),Qt::AlignRight,QString(_("User: \"%1\"")).arg(user().c_str()));
	painter.drawText(QRect(0,im.height()+fntSize,pagl.width(),fntSize),Qt::AlignLeft,(pnm+" ("+pg+")").c_str());
	QDateTime dt;
	dt.setTime_t(time(NULL));
	painter.drawText(QRect(0,im.height()+fntSize,pagl.width(),fntSize),Qt::AlignRight,dt.toString("d.MM.yyyy h:mm:ss"));

	painter.end();
    }
#endif
}

void VisRun::printDiag( const string &idg )
{
#ifndef QT_NO_PRINTER
    RunWdgView *rwdg;
    string dg = idg;

    if(pgList.empty())	{ QMessageBox::warning(this,_("Printing a diagram"),_("There is no page!")); return; }

    if(dg.empty()) {
	RunPageView *rpg;
	vector<string> lst;
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		rpg->shapeList("Diagram",lst);
	if(lst.empty())	{ QMessageBox::warning(this,_("Printing a diagram"),_("There is no diagram!")); return; }
	if(lst.size() == 1)	dg = lst[0];
	else {
	    //Make select diagrams dialog
	    QImage ico_t;
	    if(!ico_t.load(TUIS::icoPath("print").c_str())) ico_t.load(":/images/print.png");
	    InputDlg sdlg(this, QPixmap::fromImage(ico_t), _("Select a diagram to print."), _("Printing a diagram"), false, false);
	    sdlg.edLay()->addWidget(new QLabel(_("Diagrams:"),&sdlg), 2, 0);
	    QComboBox *spg = new QComboBox(&sdlg);
	    sdlg.edLay()->addWidget(spg, 2, 1);
	    for(unsigned i_l = 0; i_l < lst.size(); i_l++)
		if((rwdg=findOpenWidget(lst[i_l])))
		    spg->addItem((rwdg->name()+" ("+lst[i_l]+")").c_str(),lst[i_l].c_str());
	    if(sdlg.exec() != QDialog::Accepted) return;
	    dg = spg->itemData(spg->currentIndex()).toString().toStdString();
	}
    }

    if(!(rwdg=findOpenWidget(dg)))	return;

    string dgnm = rwdg->name();
    if(!prDiag)	prDiag = new QPrinter(QPrinter::HighResolution);
    QPrintDialog dlg(prDiag, this);
    dlg.setWindowTitle(QString(_("Printing the diagram: \"%1\" (%2)")).arg(dgnm.c_str()).arg(dg.c_str()));
    if(dlg.exec() == QDialog::Accepted) {
	int fntSize = 35;
	QSize papl(2048,2048*prDiag->paperRect().height()/prDiag->paperRect().width());
	QSize pagl(papl.width()*prDiag->pageRect().width()/prDiag->paperRect().width(), papl.height()*prDiag->pageRect().height()/prDiag->paperRect().height());

	ShapeDiagram::ShpDt *sD = (ShapeDiagram::ShpDt*)rwdg->shpData;
	int elLine = sD->prms.size()/2+((sD->prms.size()%2)?1:0);

	QPainter painter;
	painter.begin(prDiag);
	painter.setWindow(QRect(QPoint(0,0),papl));
	painter.setViewport(prDiag->paperRect());

	//Draw image
# if QT_VERSION >= 0x050000
	QImage im = rwdg->grab().toImage();
# else
	QImage im = QPixmap::grabWidget(rwdg).toImage();
# endif
	im = im.scaled(QSize(vmin(im.width()*4,pagl.width()),vmin(im.height()*4,pagl.height()-(2+elLine)*fntSize)),Qt::KeepAspectRatio/*,Qt::SmoothTransformation*/);
	painter.drawImage((pagl.width()-im.width())/2,fntSize*2,im);

	//Draw notes
	painter.setPen(Qt::black);
	QFont fnt("Arial");
	fnt.setPixelSize(fntSize-5);
	painter.setFont(fnt);
	QDateTime dt;
	dt.setTime_t(time(NULL));
	painter.drawText(QRect(0,0,pagl.width(),fntSize*2),Qt::AlignLeft,QString(_("OpenSCADA project: \"%1\"\n%2 (%3)")).arg(windowTitle()).arg(dgnm.c_str()).arg(dg.c_str()));
	painter.drawText(QRect(0,0,pagl.width(),fntSize*2),Qt::AlignRight,QString(_("User: \"%1\"\n%2")).arg(user().c_str()).arg(dt.toString("d.MM.yyyy h:mm:ss")));

	// Draw trend's elements
	XMLNode reqName("name");
	for(unsigned i_e = 0; i_e < sD->prms.size(); i_e++) {
	    QPoint pnt((i_e/elLine)*(pagl.width()/2),im.height()+fntSize*(2+i_e%elLine));
	    if(sD->prms[i_e].val().empty() || !sD->prms[i_e].color().isValid()) continue;
	    //  Trend name request
	    reqName.setAttr("path",sD->prms[i_e].addr()+"/%2fserv%2fval");
	    if(cntrIfCmd(reqName,true) || reqName.text().empty())	reqName.setText(sD->prms[i_e].addr());

	    painter.fillRect(QRect(pnt.x()+2,pnt.y()+2,fntSize-5,fntSize-5),QBrush(sD->prms[i_e].color()));
	    painter.drawRect(QRect(pnt.x()+2,pnt.y()+2,fntSize-5,fntSize-5));
	    painter.drawText(QRect(pnt.x()+fntSize,pnt.y(),pagl.width()/2,fntSize),Qt::AlignLeft,
		QString("%1 [%2...%3]").arg(reqName.text().c_str()).arg(sD->prms[i_e].bordL()).arg(sD->prms[i_e].bordU()));
	}

	painter.end();
    }
#endif
}

void VisRun::printDoc( const string &idoc )
{
#ifndef QT_NO_PRINTER
    RunWdgView *rwdg;
    string doc = idoc;

    if(pgList.empty())	{ QMessageBox::warning(this,_("Printing a document"),_("There is no page!")); return; }

    if(doc.empty()) {
	RunPageView *rpg;
	vector<string> lst;
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		rpg->shapeList("Document",lst);
	if(lst.empty())	{ QMessageBox::warning(this,_("Printing a document"),_("There is no document!")); return; }
	if(lst.size() == 1)	doc = lst[0];
	else {
	    //Make select diagrams dialog
	    QImage ico_t;
	    if(!ico_t.load(TUIS::icoPath("print").c_str())) ico_t.load(":/images/print.png");
	    InputDlg sdlg(this, QPixmap::fromImage(ico_t), _("Select a document to print."), _("Printing a document"), false, false);
	    sdlg.edLay()->addWidget(new QLabel(_("Document:"),&sdlg), 2, 0);
	    QComboBox *spg = new QComboBox(&sdlg);
	    sdlg.edLay()->addWidget(spg, 2, 1);
	    for(unsigned i_l = 0; i_l < lst.size(); i_l++)
		if((rwdg=findOpenWidget(lst[i_l])))
		    spg->addItem((rwdg->name()+" ("+lst[i_l]+")").c_str(),lst[i_l].c_str());
	    if(sdlg.exec() != QDialog::Accepted)	return;
	    doc = spg->itemData(spg->currentIndex()).toString().toStdString();
	}
    }

    if(!(rwdg=findOpenWidget(doc)))	return;

    string docnm = rwdg->name();
    if(!prDoc) prDoc = new QPrinter(QPrinter::HighResolution);
    QPrintDialog dlg(prDoc, this);
    dlg.setWindowTitle(QString(_("Printing the document: \"%1\" (%2)")).arg(docnm.c_str()).arg(doc.c_str()));
    if(dlg.exec() == QDialog::Accepted) ((ShapeDocument::ShpDt*)rwdg->shpData)->print(prDoc);
#endif
}

void VisRun::exportDef( )
{
    if(master_pg) {
	//Check for the single and big document present for default the exporting
	RunPageView *rpg;
	RunWdgView *rwdg;
	vector<string> lstDoc, lstDiagr;
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p]))) {
		rpg->shapeList("Document", lstDoc);
		rpg->shapeList("Diagram", lstDiagr);
	    }
	if(lstDoc.size() == 1 && (rwdg=findOpenWidget(lstDoc[0])) &&
		((masterPg()->width()/vmax(1,rwdg->width())) < 2 || (masterPg()->height()/vmax(1,rwdg->height())) < 2))
	    exportDoc(rwdg->id());
	else if(lstDiagr.size() == 1 && (rwdg=findOpenWidget(lstDiagr[0])) &&
		((masterPg()->width()/vmax(1,rwdg->width())) < 2 || (masterPg()->height()/vmax(1,rwdg->height())) < 2))
	    exportDiag(rwdg->id());
	//Export master page
	else exportPg(master_pg->id());
    }
}

void VisRun::exportPg( const string &ipg )
{
    RunPageView *rpg;
    string pg = ipg;

    if(pgList.empty())	{ QMessageBox::warning(this,_("Exporting a page"),_("There is no page for exporting!")); return; }

    if(pg.empty() && pgList.size() == 1)	pg = pgList[0];
    if(pg.empty() && pgList.size() > 1) {
	//Make select page dialog
	QImage ico_t;
	if(!ico_t.load(TUIS::icoPath("export").c_str())) ico_t.load(":/images/export.png");
	InputDlg sdlg(this, QPixmap::fromImage(ico_t), _("Select a page to export."), _("Exporting a page"), false, false);
	sdlg.edLay()->addWidget( new QLabel(_("Pages:"),&sdlg), 2, 0 );
	QComboBox *spg = new QComboBox(&sdlg);
	sdlg.edLay()->addWidget( spg, 2, 1 );
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		spg->addItem((rpg->name()+" ("+pgList[i_p]+")").c_str(),pgList[i_p].c_str());
	if(sdlg.exec() != QDialog::Accepted)	return;
	pg = spg->itemData(spg->currentIndex()).toString().toStdString();
    }

    //Find need page
    rpg = master_pg;
    if(rpg->id() != pg)	rpg = findOpenPage(pg);
    if(!rpg) return;

#if QT_VERSION >= 0x050000
    QPixmap img = rpg->grab();
#else
    QPixmap img = QPixmap::grabWidget(rpg);
#endif
    QString fn = getFileName(_("Saving the page image"), (rpg->name()+".png").c_str(), _("Images (*.png *.xpm *.jpg)"), QFileDialog::AcceptSave);
    if(fn.size() && !img.save(fn)) mod->postMess(mod->nodePath().c_str(), QString(_("Error saving to the file '%1'.")).arg(fn), TVision::Error, this);
}

void VisRun::exportDiag( const string &idg )
{
    RunWdgView *rwdg;
    string dg = idg;

    if(pgList.empty())	{ QMessageBox::warning(this,_("Exporting a diagram"),_("There is no page!")); return; }

    if(dg.empty()) {
	RunPageView *rpg;
	vector<string> lst;
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		rpg->shapeList("Diagram",lst);
	if(lst.empty())	{ QMessageBox::warning(this,_("Exporting a diagram"),_("There is no diagram!")); return; }
	if(lst.size() == 1) dg = lst[0];
	else {
	    //Make select diagrams dialog
	    QImage ico_t;
	    if(!ico_t.load(TUIS::icoPath("print").c_str())) ico_t.load(":/images/export.png");
	    InputDlg sdlg(this, QPixmap::fromImage(ico_t), _("Select a diagram to export."), _("Exporting a diagram"), false, false);
	    sdlg.edLay()->addWidget(new QLabel(_("Diagrams:"),&sdlg), 2, 0);
	    QComboBox *spg = new QComboBox(&sdlg);
	    sdlg.edLay()->addWidget(spg, 2, 1);
	    for(unsigned i_l = 0; i_l < lst.size(); i_l++)
		if((rwdg=findOpenWidget(lst[i_l])))
		    spg->addItem((rwdg->name()+" ("+lst[i_l]+")").c_str(),lst[i_l].c_str());
	    if(sdlg.exec() != QDialog::Accepted) return;
	    dg = spg->itemData(spg->currentIndex()).toString().toStdString();
	}
    }

    if(!(rwdg=findOpenWidget(dg))) return;

#if QT_VERSION >= 0x050000
    QPixmap img = rwdg->grab();
#else
    QPixmap img = QPixmap::grabWidget(rwdg);
#endif
    QString fileName = getFileName(_("Saving a diagram"), QString(_("Trend %1.png")).arg(expDiagCnt++),
	_("Images (*.png *.xpm *.jpg);;CSV file (*.csv)"), QFileDialog::AcceptSave);
    if(!fileName.isEmpty()) {
	// Export to CSV
	if(fileName.indexOf(QRegExp("\\.csv$")) != -1) {
	    //  Open destination file
	    int fd = open(fileName.toStdString().c_str(), O_WRONLY|O_CREAT|O_TRUNC, SYS->permCrtFiles());
	    if(fd < 0) {
		mod->postMess(mod->nodePath().c_str(),QString(_("Error saving to the file '%1'.")).arg(fileName),TVision::Error,this);
		return;
	    }

	    ShapeDiagram::ShpDt *dgDt = (ShapeDiagram::ShpDt*)rwdg->shpData;
	    string CSVr;
	    //  Trend type process
	    if(dgDt->type == 0) {
		int firstPrm = -1, vPos = 0;
		//  Prepare header
		CSVr += _("\"Date and time\";\"us\"");
		for(unsigned i_p = 0; i_p < dgDt->prms.size(); i_p++)
		    if(dgDt->prms[i_p].val().size() && dgDt->prms[i_p].color().isValid()) {
			CSVr += ";\""+TSYS::path2sepstr(dgDt->prms[i_p].addr())+"\"";
			if(firstPrm < 0) firstPrm = i_p;
		    }
		CSVr += "\x0D\x0A";
		if(firstPrm < 0) return;
		//  Place data
		deque<ShapeDiagram::TrendObj::SHg> &baseVls = dgDt->prms[firstPrm].val();
		int64_t eTmVl = dgDt->tTime;
		int64_t bTmVl = eTmVl - 1e6*dgDt->tSize;
		for(unsigned i_v = 0; i_v < baseVls.size() && baseVls[i_v].tm <= eTmVl; i_v++) {
		    if(baseVls[i_v].tm < bTmVl) continue;
		    CSVr += atm2s(baseVls[i_v].tm/1000000, "\"%d/%m/%Y %H:%M:%S\"") + ";" + i2s(baseVls[i_v].tm%1000000);
		    for(unsigned i_p = 0; i_p < dgDt->prms.size(); i_p++) {
			ShapeDiagram::TrendObj &cPrm = dgDt->prms[i_p];
			if(cPrm.val().size() && cPrm.color().isValid()) {
			    vPos = cPrm.val(baseVls[i_v].tm);
			    CSVr = CSVr + ";"+((vPos < (int)cPrm.val().size())?QLocale().toString(cPrm.val()[vPos].val).toStdString():"");
			}
		    }
		    CSVr += "\x0D\x0A";
		}
	    }
	    //  Frequency spectrum type
	    else if(dgDt->type == 1) {
#if HAVE_FFTW3_H
		//  Prepare header
		CSVr += _("\"Frequency (Hz)\"");
		for(unsigned i_p = 0; i_p < dgDt->prms.size(); i_p++)
		    if(dgDt->prms[i_p].fftN && dgDt->prms[i_p].color().isValid())
			CSVr += ";\""+TSYS::path2sepstr(dgDt->prms[i_p].addr())+"\"";
		CSVr += "\x0D\x0A";
		//  Place data
		int fftN = rwdg->size().width();		//Samples number
		double fftBeg = 1/dgDt->tSize;			//Minimum frequency or maximum period time (s)
		double fftEnd = (double)fftN*fftBeg/2;		//Maximum frequency or minimum period time (s)
		for(double i_frq = fftBeg; i_frq <= fftEnd; i_frq += fftBeg) {
		    CSVr += QLocale().toString(i_frq).toStdString();
		    for(unsigned i_p = 0; i_p < dgDt->prms.size(); i_p++) {
			ShapeDiagram::TrendObj &cPrm = dgDt->prms[i_p];
			if(cPrm.fftN && cPrm.color().isValid()) {
			    int vpos = (int)((i_frq*cPrm.fftN)/(fftBeg*fftN));
			    double val = EVAL_REAL;
			    if(vpos >= 1 && vpos < (cPrm.fftN/2+1))
				val = cPrm.fftOut[0][0]/cPrm.fftN + pow(pow(cPrm.fftOut[vpos][0],2)+pow(cPrm.fftOut[vpos][1],2),0.5)/(cPrm.fftN/2+1);
			    CSVr += ";"+QLocale().toString(val).toStdString();
			}
		    }
		    CSVr += "\x0D\x0A";
		}
#endif
	    }
	    //  Save to file
	    bool fOK = (write(fd,CSVr.data(),CSVr.size()) == (int)CSVr.size());
	    ::close(fd);
	    if(!fOK) mod->postMess(mod->nodePath().c_str(), QString(_("Error writing to: %1.")).arg(fileName), TVision::Error, this);
	}
	// Export to image
	else if(!img.save(fileName))
	    mod->postMess(mod->nodePath().c_str(),QString(_("Error saving to the file '%1'.")).arg(fileName),TVision::Error,this);
    }
}

void VisRun::exportDoc( const string &idoc )
{
    RunWdgView *rwdg;
    string doc = idoc;

    if(pgList.empty())	{ QMessageBox::warning(this,_("Exporting a document"),_("There is no page!")); return; }

    if(doc.empty()) {
	RunPageView *rpg;
	vector<string> lst;
	for(unsigned i_p = 0; i_p < pgList.size(); i_p++)
	    if((rpg=findOpenPage(pgList[i_p])))
		rpg->shapeList("Document",lst);
	if(lst.empty())	{ QMessageBox::warning(this,_("Exporting a document"),_("There is no document!")); return; }
	if(lst.size() == 1) doc = lst[0];
	else {
	    //Make select diagrams dialog
	    QImage ico_t;
	    if(!ico_t.load(TUIS::icoPath("print").c_str())) ico_t.load(":/images/export.png");
	    InputDlg sdlg(this, QPixmap::fromImage(ico_t), _("Select a document to export."), _("Exporting a document"), false, false);
	    sdlg.edLay()->addWidget(new QLabel(_("Document:"),&sdlg), 2, 0);
	    QComboBox *spg = new QComboBox(&sdlg);
	    sdlg.edLay()->addWidget( spg, 2, 1 );
	    for(unsigned i_l = 0; i_l < lst.size(); i_l++)
		if((rwdg=findOpenWidget(lst[i_l])))
		    spg->addItem((rwdg->name()+" ("+lst[i_l]+")").c_str(),lst[i_l].c_str());
	    if(sdlg.exec() != QDialog::Accepted) return;
	    doc = spg->itemData(spg->currentIndex()).toString().toStdString();
	}
    }

    if(!(rwdg=findOpenWidget(doc))) return;
    QString fileName = getFileName(_("Saving a document"), QString(_("Document %1.html")).arg(expDocCnt++),
	_("XHTML (*.html);;CSV file (*.csv)"), QFileDialog::AcceptSave);
    if(!fileName.isEmpty()) {
	int fd = open(fileName.toStdString().c_str(), O_WRONLY|O_CREAT|O_TRUNC, SYS->permCrtFiles());
	if(fd < 0) {
	    mod->postMess(mod->nodePath().c_str(),QString(_("Error saving to the file '%1'.")).arg(fileName),TVision::Error,this);
	    return;
	}
	string rez;
	// Export to CSV
	if(fileName.indexOf(QRegExp("\\.csv$")) != -1) {
	    //  Parse document
	    XMLNode docTree;
	    docTree.load(((ShapeDocument::ShpDt*)rwdg->shpData)->doc, true);
	    XMLNode *curNode = &docTree;
	    vector<unsigned> treeStk;
	    treeStk.push_back(0);
	    while(!(!curNode->parent() && treeStk.back() >= curNode->childSize())) {
		if(treeStk.back() < curNode->childSize()) {
		    curNode = curNode->childGet(treeStk.back());
		    treeStk.push_back(0);
		    //  Check for marked table and process it
		    if(strcasecmp(curNode->name().c_str(),"table") == 0 && s2i(curNode->attr("export"))) {
			map<int,int>	rowSpn;
			XMLNode *tblN = NULL, *tblRow;
			string val;
			for(int i_st = 0; i_st < 4; i_st++) {
			    switch(i_st) {
				case 0:	tblN = curNode->childGet("thead", 0, true);	break;
				case 1:	tblN = curNode->childGet("tbody", 0, true);	break;
				case 2:	tblN = curNode->childGet("tfoot", 0, true);	break;
				case 3:	tblN = curNode;					break;
				default: tblN = NULL;
			    }
			    if(!tblN)	continue;
			    //  Rows process
			    for(unsigned i_n = 0; i_n < tblN->childSize(); i_n++) {
				if(strcasecmp(tblN->childGet(i_n)->name().c_str(),"tr") != 0)	continue;
				tblRow = tblN->childGet(i_n);
				for(unsigned iC = 0, iCl = 0; iC < tblRow->childSize(); iC++) {
				    if(!(strcasecmp(tblRow->childGet(iC)->name().c_str(),"th") == 0 ||
					    strcasecmp(tblRow->childGet(iC)->name().c_str(),"td") == 0))
					continue;
				    while(rowSpn[iCl] > 1) { rez += ";"; rowSpn[iCl]--; iCl++; }
				    rowSpn[iCl] = s2i(tblRow->childGet(iC)->attr("rowspan",false));
				    val = tblRow->childGet(iC)->text(true,true);
				    for(size_t i_sz = 0; (i_sz=val.find("\"",i_sz)) != string::npos; i_sz += 2) val.replace(i_sz,1,2,'"');
				    rez += "\""+sTrm(val)+"\";";
				    //   Colspan process
				    int colSpan = s2i(tblRow->childGet(iC)->attr("colspan",false));
				    for(int iCs = 1; iCs < colSpan; iCs++) rez += ";";
				    iCl++;
				}
				rez += "\x0D\x0A";
			    }
			}
			rez += "\x0D\x0A";
		    }
		    else continue;
		}
		curNode = curNode->parent();
		treeStk.pop_back();
		treeStk.back()++;
	    }
	}
	// Export to XHTML
	else rez = ((ShapeDocument::ShpDt*)rwdg->shpData)->toHtml();

	bool fOK = true;
	if(rez.empty())	mod->postMess(mod->nodePath().c_str(),QString(_("No data to export.")),TVision::Error,this);
	else fOK = (write(fd,rez.data(),rez.size()) == (int)rez.size());
	::close(fd);
	if(!fOK) mod->postMess(mod->nodePath().c_str(), QString(_("Error writing to: %1.")).arg(fileName), TVision::Error, this);
    }
}

void VisRun::about( )
{
    QMessageBox::about(this,windowTitle(),
	QString(_("%1 v%2.\n%3\nAuthor: %4\nLicense: %5\n\n%6 v%7.\n%8\nLicense: %9\nAuthor: %10\nWeb site: %11")).
	arg(mod->modInfo("Name").c_str()).arg(mod->modInfo("Version").c_str()).arg(mod->modInfo("Description").c_str()).
	arg(mod->modInfo("Author").c_str()).arg(mod->modInfo("License").c_str()).
	arg(PACKAGE_NAME).arg(VERSION).arg(_(PACKAGE_DESCR)).arg(PACKAGE_LICENSE).arg(_(PACKAGE_AUTHOR)).arg(PACKAGE_SITE));
}

void VisRun::userChanged( const QString &oldUser, const QString &oldPass )
{
    //Try second connect to the session for permission check
    XMLNode req("connect");
    req.setAttr("path","/%2fserv%2fsess")->setAttr("sess",workSess())->setAttr("userChange","1");
    if(cntrIfCmd(req)) {
	mWUser->setUser(oldUser.toStdString());
	mWUser->setPass(oldPass.toStdString());
	mod->postMess(req.attr("mcat").c_str(), req.text().c_str(), TVision::Error, this);
	return;
    }
    if(req.attr("userIsRoot").size()) setWinMenu(s2i(req.attr("userIsRoot")));
    else setWinMenu(SYS->security().at().access(user(),SEC_WR,"root","root",RWRWR_));
    int oldConId = mConId;
    mConId = s2i(req.attr("conId"));
    req.clear()->setName("disconnect")->setAttr("path","/%2fserv%2fsess")->setAttr("sess",workSess())->setAttr("conId", i2s(oldConId));
    cntrIfCmd(req);

    //Update pages after an user change
    pgCacheClear();
    bool oldMenuVis = winMenu();
    QApplication::processEvents();
    if(masterPg()) {
	if(oldMenuVis != winMenu() && (windowState() == Qt::WindowMaximized || windowState() == Qt::WindowFullScreen)) {
	    x_scale *= (float)((QScrollArea*)centralWidget())->maximumViewportSize().width()/(float)master_pg->size().width();
	    y_scale *= (float)((QScrollArea*)centralWidget())->maximumViewportSize().height()/(float)master_pg->size().height();
	    if(x_scale > 1 && x_scale < 1.05) x_scale = 1;
	    if(y_scale > 1 && y_scale < 1.05) y_scale = 1;
	    if(keepAspectRatio) x_scale = y_scale = vmin(x_scale, y_scale);
	    mess_debug(mod->nodePath().c_str(), _("Scale of the root page [%f:%f]."), x_scale, y_scale);
	}
	isResizeManual = true;
	fullUpdatePgs();
	isResizeManual = false;

	//Resize
	resizeEvent(NULL);
    }
}

void VisRun::styleChanged( )
{
    //Get current style
    XMLNode req("set");
    req.setAttr("path","/ses_"+work_sess+"/%2fobj%2fcfg%2fstyle")->setText(i2s(style()));
    if(cntrIfCmd(req)) {
	mod->postMess(req.attr("mcat").c_str(),req.text().c_str(),TVision::Error,this);
	return;
    }
    fullUpdatePgs();
}

void VisRun::aboutQt( )		{ QMessageBox::aboutQt(this, mod->modInfo("Name").c_str()); }

void VisRun::fullScreen( bool vl )
{
    if(vl) setWindowState(Qt::WindowFullScreen);
    else setWindowState(Qt::WindowNoState);
}

/*void VisRun::enterWhatsThis()
{
    QWhatsThis::enterWhatsThisMode();
}*/

void VisRun::alarmAct( QAction *alrm )
{
    if(alrm == NULL) return;

    int quietance = 0;
    string qwdg;
    if(alrm->objectName() == "alarmLev")	quietance = 0xFF;
    else if(alrm->objectName() == "alarmLight")	quietance = 0x01;
    else if(alrm->objectName() == "alarmAlarm")	quietance = 0x02;
    else if(alrm->objectName() == "alarmSound") {
	quietance = 0x04;
	qwdg = alrmPlay->widget( );
    }
    else return;

    XMLNode req("quietance");
    req.setAttr("path","/ses_"+work_sess+"/%2fserv%2falarm")->
	setAttr("tmpl",u2s(quietance))->
	setAttr("wdg",qwdg);
    cntrIfCmd(req);

    //Send event to master page
    if(master_pg) master_pg->attrSet("event", ("ws_"+alrm->objectName()).toStdString(), A_NO_ID, true);
}

void VisRun::usrStatus( const string &val, RunPageView *pg )
{
    UserItStBar *userSt;
    if(!pg) pg = masterPg();
    if(!pg || pg != masterPg()) return;

    //Presence mark clean
    for(int iC = 0; iC < statusBar()->children().size(); iC++)
	if((userSt=qobject_cast<UserItStBar*>(statusBar()->children().at(iC))) && userSt->objectName().indexOf("usr_") == 0)
	    userSt->setProperty("usrStPresent", false);

    //Items list parse
    string iLn;
    for(int off = 0, lCnt = 0; (iLn=TSYS::strLine(val,0,&off)).size(); lCnt++) {
	// Parse line in the format: "{id}:{label}:{tip}:{color}:{ico}
	int offIt = 0;
	string	itId  = TSYS::strParse(iLn, 0, ":", &offIt),
		itLab = TSYS::strParse(iLn, 0, ":", &offIt),
		itTip = TSYS::strParse(iLn, 0, ":", &offIt),
		itColor = TSYS::strParse(iLn, 0, ":", &offIt),
		itIco = TSYS::strParse(iLn, 0, ":", &offIt);
	if(itTip.empty()) itTip = itId;
	if(itColor.empty()) itColor = "black";

	// Try presence yet
	userSt = statusBar()->findChild<UserItStBar*>(("usr_"+itId).c_str());
	// Create new one
	if(!userSt) {
	    userSt = new UserItStBar(this);
	    userSt->setObjectName(("usr_"+itId).c_str());
	    userSt->setAlignment(Qt::AlignCenter);
	    statusBar()->insertPermanentWidget(0, userSt);
	}
	// Set properties
	userSt->setProperty("usrStPresent", true);
	userSt->setProperty("usrStPos", lCnt);
	userSt->setToolTip(itTip.c_str());

	QImage ico_t;
	if(!itIco.empty()) {
	    itIco = pg->resGet(itIco);
	    ico_t.loadFromData((const uchar*)itIco.data(), itIco.size());
	}
	userSt->setPixmap(QPixmap::fromImage(ico_t));

	if(itLab.size()) userSt->setText(QString("<font color='%1'>%2</font>").arg(itColor.c_str()).arg(itLab.c_str()));
    }

    //Check for remove and order
    for(int iC = 0; iC < statusBar()->children().size(); iC++)
	if((userSt=qobject_cast<UserItStBar*>(statusBar()->children().at(iC))) && userSt->objectName().indexOf("usr_") == 0) {
	    if(!userSt->property("usrStPresent").toBool())
		//delete userSt;
		userSt->deleteLater();
	    /*else for(int iC1 = iC; iC1 > 0 ; iC1--) {
		if(!(userSt1=qobject_cast<UserItStBar*>(statusBar()->children().at(iC1-1))) ||
		    userSt1->objectName().indexOf("usr_") != 0) continue;
		if(userSt->property("usrStPos").toInt() < userSt1->property("usrStPos").toInt()) userSt1->stackUnder(userSt);
	    }*/
	}
}

void VisRun::initSess( const string &iprjSes_it, bool icrSessForce )
{
    bool isSess = false;
    src_prj = work_sess = "";
    string openPgs;

    //Connect/create session
    int off = 0;
    if((src_prj=TSYS::pathLev(iprjSes_it,0,true,&off)).empty()) return;
    if(off > 0 && off < iprjSes_it.size()) openPgs = iprjSes_it.substr(off);
    // Check for ready session connection or project
    if(src_prj.compare(0,4,"ses_") == 0) { work_sess = src_prj.substr(4); src_prj = ""; isSess = true; }
    else if(src_prj.compare(0,4,"prj_") == 0) src_prj.erase(0,4);
    else return;

    //Get opened sessions list for our page and put dialog for connection
    XMLNode req("list");
    req.setAttr("path","/%2fserv%2fsess")->setAttr("prj",src_prj);
    if(!isSess && !icrSessForce && !cntrIfCmd(req) && req.childSize()) {
	// Prepare and execute a session selection dialog
	QImage ico_t;
	if(!ico_t.load(TUIS::icoPath("vision_prj_run").c_str())) ico_t.load(":/images/prj_run.png");
	InputDlg conreq(this,QPixmap::fromImage(ico_t),
	    QString(_("Several sessions are already opened for the project \"%1\".\n"
		"You can create a new or connect to the present session. Please select the one you want or click \"Cancel\".")).arg(src_prj.c_str()),
	    _("Selecting a session for connection or a new one creating"), false, false);
	QListWidget *ls_wdg = new QListWidget(&conreq);
	conreq.edLay()->addWidget(ls_wdg, 0, 0);
	ls_wdg->addItem(_("<Create new session>"));
	for(unsigned iCh = 0; iCh < req.childSize(); iCh++)
	    ls_wdg->addItem(req.childGet(iCh)->text().c_str());
	ls_wdg->setCurrentRow(0);

	if(conreq.exec() == QDialog::Accepted && ls_wdg->currentItem())
	    work_sess = (ls_wdg->currentRow() > 0) ? ls_wdg->currentItem()->text().toStdString() : "";
	else { close(); return; }
    }

    req.clear()->setName("connect")->
		 setAttr("path", "/%2fserv%2fsess")->
		 setAttr("conTm", i2s(mod->restoreTime()*1000));	//Initial for allow the project loading and
									//the session creation on the server side mostly/.
    if(work_sess.empty()) req.setAttr("prj", src_prj);
    else req.setAttr("sess", work_sess);
    if(cntrIfCmd(req)) {
	if(!(conErr && s2i(req.attr("rez")) == 10)) {	//Need check for prevent the warning dialog and the run closing by the session creation wait
	    mod->postMess(req.attr("mcat").c_str(), req.text().c_str(), TVision::Error, this);
	    close();
	}
	return;
    }
    if(req.attr("userIsRoot").size()) setWinMenu(s2i(req.attr("userIsRoot")));
    else setWinMenu(SYS->security().at().access(user(),SEC_WR,"root","root",RWRWR_));

    if(work_sess.empty()) work_sess = req.attr("sess");
    if(src_prj.empty()) src_prj = req.attr("prj");

    mConId = s2i(req.attr("conId"));
    bool toRestore = master_pg;
    if(openPgs.size()) openPgs = "/ses_"+work_sess+openPgs;

    //Prepare group for parameters request and apply
    setWindowTitle(QString(_("Running project: %1")).arg(src_prj.c_str()));	//Default title
    setWindowIcon(mod->icon());
    req.clear()->setName("CntrReqs")->setAttr("path","");
    req.childAdd("get")->setAttr("path","/prj_"+src_prj+"/%2fobj%2fcfg%2fname");
    req.childAdd("get")->setAttr("path","/ses_"+work_sess+"/%2fico");
    req.childAdd("get")->setAttr("path","/ses_"+work_sess+"/%2fobj%2fcfg%2fper");
    req.childAdd("get")->setAttr("path","/ses_"+work_sess+"/%2fobj%2fcfg%2fstyle");
    req.childAdd("get")->setAttr("path","/ses_"+work_sess+"/%2fobj%2fcfg%2fstLst");
    req.childAdd("get")->setAttr("path","/prj_"+src_prj+"/%2fobj%2fcfg%2fflgs");
    req.childAdd("openlist")->setAttr("path","/ses_"+work_sess+"/%2fserv%2fpg")->setAttr("tm","0")->setAttr("conId", i2s(mConId));
    if(!cntrIfCmd(req)) {
	// Title
	XMLNode *pN = req.childGet(0);
	setWindowTitle(pN->text().c_str());

	// Icon
	pN = req.childGet(1);
	QImage img;
	string simg = TSYS::strDecode(pN->text(), TSYS::base64);
	if(img.loadFromData((const uchar*)simg.c_str(),simg.size()))
	    setWindowIcon(QPixmap::fromImage(img));

	// Period
	pN = req.childGet(2);
	mPeriod = s2i(pN->text());

	// Style
	pN = req.childGet(3);
	setStyle(s2i(pN->text()));
	pN = req.childGet(4);
	if(style() < 0 && pN->childSize() <= 1) mStlBar->setVisible(false);
	// Flags
	pN = req.childGet(5);
	int flgs = s2i(pN->text());
	if(flgs&0x01)		setWindowState(Qt::WindowMaximized);
	else if(flgs&0x02)	actFullScr->setChecked(true);
	keepAspectRatio = flgs&0x04;

	// Clean up the previous pages for clean reconnection
	if(toRestore) {
	    pgCacheClear();
	    openPgs = "";
	    for(unsigned iP = 0; iP < pgList.size(); iP++) {
		RunPageView *pg = master_pg->findOpenPage(pgList[iP]);
		if(pg) { pg->deleteLater(); openPgs += pgList[iP] + ";"; }
	    }
	    master_pg->deleteLater();
	    master_pg = NULL;
	    setXScale(1); setYScale(1);
	}

	// Open pages list
	pN = req.childGet(6);
	pgList.clear();
	for(unsigned iCh = 0; iCh < pN->childSize(); iCh++) {
	    pgList.push_back(pN->childGet(iCh)->text());
	    callPage(pN->childGet(iCh)->text()/*, toRestore*/);
	}
	reqtm = strtoul(pN->attr("tm").c_str(), NULL, 10);
    }

    //Open direct-selected page or openned before ones
    if(openPgs.size()) {
	req.clear()->setName("CntrReqs")->setAttr("path", "/ses_"+work_sess);
	string pIt;
	for(off = 0; (pIt=TSYS::strParse(openPgs,0,";",&off)).size(); )
	    req.childAdd("open")->setAttr("path","/%2fserv%2fpg")->setAttr("pg",pIt);
	cntrIfCmd(req);
	// Force call for blinks prevent
	for(off = 0; (pIt=TSYS::strParse(openPgs,0,";",&off)).size(); )
	    callPage(pIt);
    }

    QCoreApplication::processEvents();

    //Start timer
    updateTimer->start(period());
}

void VisRun::fullUpdatePgs( )
{
    for(unsigned i_p = 0; i_p < pgList.size(); i_p++) {
	RunPageView *pg = master_pg->findOpenPage(pgList[i_p]);
	if(pg) pg->update(true);
    }
}

void VisRun::callPage( const string& pg_it, bool updWdg )
{
    vector<int> idst;
    string stmp;

    //Scan and update opened page
    if(master_pg) {
	RunPageView *pg = master_pg->findOpenPage(pg_it);
	if(pg) {
	    if(!(wPrcCnt%(5000/vmin(5000,period()))))	pg->update(false, NULL, true);
	    else if(updWdg) pg->update(false);
	    return;
	}
    }

    //Get group and parent page
    string pgGrp = wAttrGet(pg_it,"pgGrp");
    string pgSrc = wAttrGet(pg_it,"pgOpenSrc");

    //Check for master page replace
    if(!master_pg || pgGrp == "main" || master_pg->pgGrp() == pgGrp) {
	// Send close command
	if(master_pg) {
	    XMLNode req("close"); req.setAttr("path","/ses_"+work_sess+"/%2fserv%2fpg")->setAttr("pg",master_pg->id());
	    cntrIfCmd(req);
	}

	// Get and activate for specific attributes to the master-page
	XMLNode reqSpc("CntrReqs"); reqSpc.setAttr("path", pg_it);
	reqSpc.childAdd("activate")->setAttr("path", "/%2fserv%2fattr%2fstatLine")->
				     setAttr("aNm", _("Status line items"))->
				     setAttr("aTp", i2s(TFld::String))->setAttr("aFlg", i2s(TFld::FullText));
	reqSpc.childAdd("activate")->setAttr("path", "/%2fserv%2fattr%2fuserSetVis");
	cntrIfCmd(reqSpc);

	// Create widget view
	master_pg = new RunPageView(pg_it, this, centralWidget());
	conErr = NULL;			//possible a connection error status clean up
	//master_pg->load("");
	master_pg->setFocusPolicy(Qt::StrongFocus);
	((QScrollArea *)centralWidget())->setWidget(master_pg);
	if(!(windowState()&(Qt::WindowFullScreen|Qt::WindowMaximized))) {
	    QRect ws = QApplication::desktop()->availableGeometry(this);
	    resize(vmin(master_pg->size().width()+10,ws.width()-10), vmin(master_pg->size().height()+55,ws.height()-10));
	}
	else x_scale = y_scale = 1.0;
    }
    //Put to check for include
    else master_pg->callPage(pg_it, pgGrp, pgSrc);
}

void VisRun::pgCacheClear( )
{
    while(!cachePg.empty()) {
	cachePg.front()->deleteLater();	//delete cachePg.front();
	cachePg.pop_front();
    }
}

void VisRun::pgCacheAdd( RunPageView *wdg )
{
    if(!wdg) return;
    cachePg.push_front(wdg);
    while(cachePg.size() > 100) {
	cachePg.back()->deleteLater();	//delete cachePg.back();
	cachePg.pop_back();
    }
}

RunPageView *VisRun::pgCacheGet( const string &id )
{
    RunPageView *pg = NULL;

    for(unsigned iPg = 0; iPg < cachePg.size(); iPg++)
	if(cachePg[iPg]->id() == id) {
	    pg = cachePg[iPg];
	    cachePg.erase(cachePg.begin()+iPg);
	    break;
	}

    return pg;
}

RunPageView *VisRun::findOpenPage( const string &pg )
{
    if(!master_pg) return NULL;

    return master_pg->findOpenPage(pg);
}

RunWdgView *VisRun::findOpenWidget( const string &wdg )
{
    int woff = 0;
    for(int off = 0; true; woff = off) {
	string sel = TSYS::pathLev(wdg,0,true,&off);
	if(sel.empty() || sel.substr(0,4) == "wdg_")	break;
    }
    RunPageView *rpg = findOpenPage(wdg.substr(0,woff));
    if(!rpg )	return NULL;
    if(woff >= (int)wdg.size())	return rpg;

    return rpg->findOpenWidget(wdg);
}

string VisRun::wAttrGet( const string &path, const string &attr, bool sess )
{
    XMLNode req("get");
    if(sess) req.setAttr("path",path+"/%2fserv%2fattrSess%2f"+attr);
    else req.setAttr("path",path+"/%2fattr%2f"+attr);
    return cntrIfCmd(req) ? "" : req.text();
}

bool VisRun::wAttrSet( const string &path, const string &attr, const string &val, bool sess )
{
    XMLNode req("set");
    if(sess) req.setAttr("path",path+"/%2fserv%2fattrSess%2f"+attr)->setText(val);
    else req.setAttr("path",path+"/%2fserv%2fattr")->
	    childAdd("el")->setAttr("id",attr)->setText(val);
    return !cntrIfCmd(req);
}

void VisRun::alarmSet( unsigned alarm )
{
    unsigned ch_tp = alarm^mAlrmSt;

    //Check for early this session running equalent project
    bool isMaster = true;
    MtxAlloc res(mod->dataRes(), true);
    for(unsigned iW = 0; iW < mod->mnWinds.size(); iW++)
	if(qobject_cast<VisRun*>(mod->mnWinds[iW]) && ((VisRun*)mod->mnWinds[iW])->srcProject() == srcProject()) {
	    if(((VisRun*)mod->mnWinds[iW])->workSess() != workSess()) isMaster = false;
	    break;
	}
    res.unlock();

    //Alarm types init
    // Set monotonic sound alarm
    if(isMaster && (ch_tp>>16)&TVision::Alarm) {
	const char *spkEvDev = "/dev/input/by-path/platform-pcspkr-event-spkr";
	int hd = open(spkEvDev,O_WRONLY);
	if(hd < 0) mess_warning(mod->nodePath().c_str(),_("Error open: %s"),spkEvDev);
	else {
	    input_event ev;
	    ev.time.tv_sec = time(NULL);
	    ev.type = EV_SND;
	    ev.code = SND_TONE;
	    ev.value = ((alarm>>16)&TVision::Alarm) ? 1000 : 0;
	    bool fOK = (write(hd,&ev,sizeof(ev)) == sizeof(ev));
	    ::close(hd);
	    if(!fOK) mess_warning(mod->nodePath().c_str(), _("Error write to: %s"), spkEvDev);
	}
    }
    // Set speach or sound alarm
    if(isMaster && (alarm>>16)&TVision::Sound && !alrmPlay->isRunning() && !alrmPlay->playData().empty()) alrmPlay->start();

    //Alarm action indicators update
    // Alarm level icon update
    if(ch_tp&0xFF || (alarm>>16)&(TVision::Light|TVision::Alarm|TVision::Sound) || !alrLevSet) {
	int alarmLev = alarm&0xFF;
	actAlrmLev->setToolTip(QString(_("Alarm level: %1")).arg(alarmLev));

	QImage lens(":/images/alarmLev.png");
	QImage levImage(lens.size(),lens.format());

	QPainter painter(&levImage);
	//QColor lclr( alarmLev ? 224 : 0, alarmLev ? 224-(int)(0.87*alarmLev) : 224, 0 );
	QColor lclr( alarmLev ? 255 : 0, alarmLev ? 255-alarmLev : 255, 0 );

	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(levImage.rect(),Qt::transparent);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	if(!((alarm>>16)&(TVision::Light|TVision::Alarm|TVision::Sound) && alrLevSet)) {
	    for(int i_x = 0; i_x < lens.size().width(); i_x++)
		for(int i_y = 0; i_y < lens.size().height(); i_y++)
		    if(lens.pixel(i_x,i_y)&0xFF000000)	levImage.setPixel(i_x,i_y,lclr.rgba());
	    alrLevSet = true;
	} else alrLevSet = false;

	painter.drawImage(0, 0, lens);
	painter.end();
	actAlrmLev->setIcon(QPixmap::fromImage(levImage));
    }
    //Alarm buttons status process
    for(int i_b = 0; i_b < 3; i_b++) {
	QAction *actAlrm = (i_b==0) ? actAlrmLight : ((i_b==1) ? actAlrmAlarm : actAlrmSound);
	if((ch_tp>>8)&(0x01<<i_b))	actAlrm->setVisible((alarm>>8)&(0x01<<i_b));
	if((ch_tp>>16)&(0x01<<i_b))	actAlrm->setEnabled((alarm>>16)&(0x01<<i_b));
    }

    mAlrmSt = alarm;
}

string VisRun::cacheResGet( const string &res )
{
    map<string,CacheEl>::iterator ires = mCacheRes.find(res);
    if(ires == mCacheRes.end()) return "";
    ires->second.tm = time(NULL);
    return ires->second.val;
}

void VisRun::cacheResSet( const string &res, const string &val )
{
    if(val.size() > USER_FILE_LIMIT) return;
    mCacheRes[res] = CacheEl(time(NULL), val);
    if(mCacheRes.size() > (STD_CACHE_LIM+STD_CACHE_LIM/10)) {
	vector< pair<time_t,string> > sortQueue;
	for(map<string,CacheEl>::iterator itr = mCacheRes.begin(); itr != mCacheRes.end(); ++itr)
	    sortQueue.push_back(pair<time_t,string>(itr->second.tm,itr->first));
	sort(sortQueue.begin(), sortQueue.end());
	for(unsigned i_del = 0; i_del < (STD_CACHE_LIM/10); ++i_del) mCacheRes.erase(sortQueue[i_del].second);
    }
}

void VisRun::updatePage( )
{
    if(winClose || updPage) return;

    int rez;

#if OSC_DEBUG >= 3
    int64_t t_cnt = TSYS::curTime();
#endif
    updPage = true;

    //Pages update
    XMLNode req("openlist");
    req.setAttr("tm", u2s(reqtm))->setAttr("conId", i2s(mConId))->
	setAttr("path", "/ses_"+work_sess+"/%2fserv%2fpg");

    if(!(rez=cntrIfCmd(req,false,true))) {
	// Check for delete the pages
	RunPageView *pg;
	for(unsigned iP = 0, iCh; iP < pgList.size(); iP++) {
	    for(iCh = 0; iCh < req.childSize(); iCh++)
		if(pgList[iP] == req.childGet(iCh)->text())
		    break;
	    if(iCh < req.childSize() || !(master_pg && (pg=master_pg->findOpenPage(pgList[iP])))) continue;
	    if(!pg->property("cntPg").toString().isEmpty())
		((RunWdgView*)TSYS::str2addr(pg->property("cntPg").toString().toStdString()))->setPgOpenSrc("");
	    else {
		if(pg != master_pg)	pg->deleteLater();
		else {
		    ((QScrollArea*)centralWidget())->setWidget(new QWidget());
		    master_pg = NULL;
		    conErr = NULL;		//possible a connection error status clean up
		}
	    }
	}

	// Process the opened pages
	pgList.clear();
	for(unsigned iCh = 0; iCh < req.childSize(); iCh++) {
	    pgList.push_back(req.childGet(iCh)->text());
	    callPage(req.childGet(iCh)->text(), s2i(req.childGet(iCh)->attr("updWdg")));
	}
    }
    // Restore closed session of used project.
    else if(rez == 2) {
	mess_warning(mod->nodePath().c_str(),_("Restore the session creation for '%s'."),prjSes_it.c_str());
	updateTimer->stop();
	initSess(prjSes_it, crSessForce);
	updPage = false;
	return;
    }

    reqtm = strtoul(req.attr("tm").c_str(),NULL,10);

    //Alarms update (0.5 second update)
    if((wPrcCnt%(500/vmin(500,period()))) == 0) {
	// Get alarm status
	unsigned wAlrmSt = alarmSt();
	req.clear()->
	    setName("get")->
	    setAttr("mode", "stat")->
	    setAttr("path", "/ses_"+work_sess+"/%2fserv%2falarm");
	if(!cntrIfCmd(req,false,true)) wAlrmSt = s2i(req.attr("alarmSt"));

	// Get sound resources for play
	if(alarmTp(TVision::Sound,true) && !alrmPlay->isRunning()) {
	    req.clear()->
		setName("get")->
		setAttr("mode", "sound")->
		setAttr("path", "/ses_"+work_sess+"/%2fserv%2falarm")->
		setAttr("tm", u2s(alrmPlay->time()))->
		setAttr("wdg", alrmPlay->widget());
	    if(!cntrIfCmd(req)) {
		alrmPlay->setTime(strtoul(req.attr("tm").c_str(),NULL,10));
		alrmPlay->setWidget(req.attr("wdg"));
		alrmPlay->setData(TSYS::strDecode(req.text(),TSYS::base64));
	    }
	}

	// Set alarm
	alarmSet(wAlrmSt);
    }

    //Old pages from cache for close checking
    for(unsigned iPg = 0; iPg < cachePg.size(); )
	if(mod->cachePgLife() > 0.01 && (period()*(reqTm()-cachePg[iPg]->reqTm())/1000) > (unsigned)(mod->cachePgLife()*60*60)) {
	    cachePg[iPg]->deleteLater();	//delete cachePg[iPg];
	    cachePg.erase(cachePg.begin()+iPg);
	}
	else iPg++;

#if OSC_DEBUG >= 3
    upd_tm += 1e-3*(TSYS::curTime()-t_cnt);
    if(!(1000/vmin(1000,period()) && wPrcCnt%(1000/vmin(1000,period()))))
    {
	mess_debug("VCA DEBUG",_("Time of updating the session '%s': %f ms."),workSess().c_str(),upd_tm);
	upd_tm = 0;
    }
#endif

    //Time update
    if(mWTime->isVisible() && !(wPrcCnt%vmax(1000/vmin(1000,period()),1))) {
	QDateTime dtm = QDateTime::currentDateTime();
	mWTime->setText( locale().toString(dtm,"hh:mm:ss\nddd, d MMM") );
	mWTime->setToolTip( locale().toString(dtm,"dddd, dd-MMM-yyyy") );
    }

    //Scale for full screen check
    if(centralWidget() && master_pg && !(wPrcCnt%vmax(1000/vmin(1000,period()),1)) &&
	(windowState() == Qt::WindowMaximized || windowState() == Qt::WindowFullScreen))
    {
	float xSc = (float)((QScrollArea*)centralWidget())->maximumViewportSize().width()/(float)master_pg->size().width();
	float ySc = (float)((QScrollArea*)centralWidget())->maximumViewportSize().height()/(float)master_pg->size().height();
	if(xSc < 1 || ySc < 1) {
	    x_scale *= xSc;
	    y_scale *= ySc;
	    //Proportional scale
	    if(keepAspectRatio) x_scale = y_scale = vmin(x_scale,y_scale);

	    fullUpdatePgs();
	}
    }

    wPrcCnt++;
    updPage = false;
}

//***********************************************
// SHost - Host thread's control object         *
SCADAHost::SCADAHost( QObject *p ) :
    QThread(p), inHostReq(0), endRun(false), reqDone(false), glob(false), req(NULL), done(NULL), pid(0)
{

}

SCADAHost::~SCADAHost( )
{
    endRun = true;
    while(!wait(100)) sendSIGALRM();
}

void SCADAHost::sendSIGALRM( )
{
    if(pid) pthread_kill(pid, SIGALRM);
}

bool SCADAHost::reqDo( XMLNode &node, bool &idone, bool iglob )
{
    if(req) return false;

    //Set the request
    mtx.lock();
    reqDone = false;
    glob = iglob;
    req = &node;
    done = &idone; *done = false;
    cond.wakeOne();
    cond.wait(mtx, 10);
    if(!reqDone) { mtx.unlock(); return false; }
    *done = true;
    done = NULL;
    req = NULL;
    reqDone = false;
    mtx.unlock();

    return true;
}

bool SCADAHost::reqBusy( )
{
    if(req && !reqDone)	return true;

    //Free done status
    if(reqDone) {
	mtx.lock();
	done = NULL;
	req = NULL;
	reqDone = false;
	mtx.unlock();
    }

    return false;
}

void SCADAHost::run( )
{
    pid = pthread_self();

    while(!endRun) {
	//Interface's requests processing
	mtx.lock();
	if(!req || reqDone) cond.wait(mtx, 1000);
	if(req && !reqDone) {
	    mtx.unlock();
	    mod->cntrIfCmd(*req, owner()->user(), owner()->password(), owner()->VCAStation(), glob);
	    mtx.lock();
	    reqDone = *done = true;
	    cond.wakeOne();
	}
	mtx.unlock();
    }
}

VisRun *SCADAHost::owner( ) const	{ return dynamic_cast<VisRun*>(parent()); }
