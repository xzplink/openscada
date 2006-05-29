
//OpenSCADA system file: tsys.h
/***************************************************************************
 *   Copyright (C) 2003-2006 by Roman Savochenko                           *
 *   rom_as@fromru.com                                                     *
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

#ifndef TSYS_H
#define TSYS_H

#define TO_FREE         NULL // Object free
#define STR_BUF_LEN     3000 // Len of string buffers (no string class)
#define STD_WAIT_DELAY  100 // Standart wait dalay (ms)

#include <stdio.h>

#include <string>
#include <vector>

#include "tbds.h"
#include "tuis.h"
#include "tarchives.h"
#include "tdaqs.h"
#include "tprotocols.h"
#include "ttransports.h"
#include "tspecials.h"
#include "tparams.h"
#include "tmodschedul.h"
#include "tsecurity.h"

#define __func__ __PRETTY_FUNCTION__

#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vmax(a,b) ((a) > (b) ? (a) : (b))

using std::string;
using std::vector;

//======================================================================================
//====================== TSYS ==========================================================
//======================================================================================
class TSYS : public TCntrNode 
{
    // Public methods:
    public:
	enum Code	{ Path, PathEl, HttpURL, Html, JavaSc, SQL, Custom, base64 };
	enum IntView 	{ Dec, Oct, Hex,   };
    
	TSYS( int argi, char ** argb, char **env );
	~TSYS(  );

	void load( );
	void save( );
	int start( );
	void stop( );
	
	int stopSignal( )	{ return stop_signal; }
	
        // Programms options
	string id()	{ return m_id; }	
	string name()	{ return m_name; }
	string user()	{ return m_user; }               //Run user name 

	void list( vector<string> &list )	{ chldList(m_subst,list); }
        bool present( const string &name )	{ return chldPresent(m_subst,name); }
        void add( TSubSYS *sub )		{ chldAdd(m_subst,sub); }
        void del( const string &name )      	{ chldDel(m_subst,name); }
        AutoHD<TSubSYS> at( const string &name ){ return chldAt(m_subst,name); }

	AutoHD<TUIS> 		ui()     	{ return at("UI"); }
	AutoHD<TArchiveS>	archive()	{ return at("Archive"); } 
	AutoHD<TBDS>		db()     	{ return at("BD"); }
	AutoHD<TDAQS> 		daq()		{ return at("DAQ"); }
	AutoHD<TProtocolS> 	protocol()   	{ return at("Protocol"); }
	AutoHD<TTransportS>	transport()  	{ return at("Transport"); }
	AutoHD<TSpecialS>    	special()    	{ return at("Special"); }
	AutoHD<TParamS>      	param()      	{ return at("Params"); }
	AutoHD<TModSchedul>  	modSchedul() 	{ return at("ModSched"); }
	AutoHD<TSecurity>	security()  	{ return at("Security"); }
	
	//Config file functions
	string cfgFile() 	{ return m_confFile; }
	XMLNode &cfgRoot()	{ return root_n; }

	//BD default prepare
	string workDB()		{ return mWorkDB; }
	
	//Get system options from DB
	bool sysOptCfg()	{ return m_sysOptCfg; }
	
	// Print comand line options!
	string optDescr( );	

	static void sighandler( int signal );
	
        // Short time dimensions
        unsigned long long sysClk( )	{ return m_sysclc; }
	void clkCalc( )
	{
	    unsigned long long st_pnt = shrtCnt( );
	    usleep(100000);
	    m_sysclc = 10*(shrtCnt( )-st_pnt);
	}
        unsigned long long shrtCnt( )
        {
    	    unsigned int cntl, cnth;
    	    asm volatile("rdtsc; movl %%eax,%0; movl %%edx,%1;":"=r"(cntl),"=r"(cnth)::"%eax","%edx");
	    return ((unsigned long long)cnth<<32)+cntl;	
        }
	static long HZ();
	
    // Public static methods:
    public:
        //========= System function ====================
	//Current system time (usec)
	static long long curTime();
	
	// Wait event with timeout support
	static bool eventWait( bool &m_mess_r_stat, bool exempl, const string &loc, time_t time = 0 );
	
	// Convert value to string
        static string int2str( int val, IntView view = Dec );
	static string ll2str( long long val, IntView view = Dec );
        static string real2str( double val );
	
	// Path and string parse
	static string fNameFix( const string &fname );
	static bool strEmpty( const string &val );
        static string strSepParse( const string &path, int level, char sep );
	static string pathLev( const string &path, int level, bool encode = true );
        static string strCode( const string &in, Code tp, const string &symb = " \t\n");
        static string strEncode( const string &in, Code tp = Custom );
	
    public:	
	const int argc;		// A comand line seting counter.	
	const char **argv;	// A comand line seting buffer.	
	const char **envp;	// A system environment.

    private:
	string nodeName(){ return id(); }
	bool cfgFileLoad();
	void cfgPrmLoad();
	void cfgFileScan( bool first = false );
	void cntrCmd_( const string &a_path, XMLNode *opt, TCntrNode::Command cmd );

	static unsigned char getBase64Code(unsigned char asymb);
    
    private:    
	string 	m_user,		// A owner user name!
	 	m_confFile,	// Config file name
		m_id,		// Station id
		m_name;		// Station name

	bool	m_sysOptCfg;	// Get system options from config only

	string	mWorkDB;	// Work DB
	
	XMLNode root_n;		// Root of the config file tree
	
	int    	stop_signal,	// Stop station signal
	 	m_subst;	// Subsystem tree id

	unsigned long long m_sysclc;
};

extern TSYS *SYS;

#endif // TSYS_H
