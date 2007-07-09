
//OpenSCADA system module UI.Vision file: vis_run.h
/***************************************************************************
 *   Copyright (C) 2007 by Roman Savochenko                                *
 *   rom_as@diyaorg.dp.ua                                                  *
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

#ifndef VIS_RUN_H
#define VIS_RUN_H

#include <string>
#include <deque>

#include <QMainWindow>

using std::string;
using std::deque;

namespace VISION
{

class UserStBar;
class RunPageView;
class RunWdgView;

class VisRun : public QMainWindow
{
    Q_OBJECT
    public:
	//Public methods
	VisRun( const string &prj_it, string open_user );
	~VisRun( );
	
	string user( );
	int    period( )	{ return m_period; }
	
	void initSess( const string &prj_it );	//Init session for project's item path
	void callPage( const string &ses_it );	//Call session page
	
	//- Cache commands -
	void pgCacheAdd( RunWdgView *wdg );
	RunWdgView *pgCacheGet( const string &id );
	
	//- Attributes commands -
	string wAttrGet( const string &path, const string &attr );
	bool wAttrSet( const string &path, const string &attr, const string &val );

    protected:
	//Protected methods
    	void closeEvent( QCloseEvent* );	//Close runtime window event

    private slots:
	//Private slots	    
        void quitSt( );				//Full quit OpenSCADA

	void about( );				//About at programm
        void aboutQt( );			//About at QT library
	void enterWhatsThis( );			//What is GUI components
	void updatePage( );			//Update page data

    private:
	//Private attributes
	//- Menu root items -
	QMenu 	*mn_file, 			//Menu "File"
		*mn_help;			//Menu "Help"

	//- Main components -
	QTimer		*updateTimer;	
	bool		winClose;		//Close window flag
	bool		proc_st;		//Timer process stat
	UserStBar 	*w_user;		//User status widget
	string 		work_sess, src_page;	//Work session and source page
	RunPageView 	*master_pg;		//Master page of runtime session
	int 		m_period;		//Clock's period
	unsigned	w_prc_cnt;		//Process counter
	
	deque<RunWdgView *>  cache_pg;		//Pages cache
};

}

#endif //VIS_RUN