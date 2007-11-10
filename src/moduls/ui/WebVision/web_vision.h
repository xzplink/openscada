
//OpenSCADA system module UI.WebVision file: web_vision.h
/***************************************************************************
 *   Copyright (C) 2007 by Roman Savochenko                                *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
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

#ifndef WEB_VISION_H
#define WEB_VISION_H

#include <string>
#include <vector>

#include <tuis.h>

#undef _
#define _(mess) mod->I18N(mess)

using std::string;
using std::vector;

namespace WebVision
{

//************************************************
//* TWEB                                         *
//************************************************
class TWEB: public TUI
{
    public:
	//Methods
	TWEB( string name );
	~TWEB( );

	void modLoad( );
	void modSave( );
    
    private:
	//Methods
	void HttpGet( const string &url, string &page, const string &sender, vector<string> &vars );
	void HttpPost( const string &url, string &page, const string &sender, vector<string> &vars, const string &contein );
	    
	string optDescr( );
	string modInfo( const string &name );
	void   modInfo( vector<string> &list );
 
	void cntrCmdProc( XMLNode *opt );       //Control interface command process
	
	string http_head( const string &rcode, int cln, const string &cnt_tp = "text/html", const string &addattr = "" );
	string w_head( );
	string w_tail( );
};    
    
extern TWEB *mod;
}
#endif //WEB_VISION_H 