
//OpenSCADA system file: tmess.cpp
/***************************************************************************
 *   Copyright (C) 2003-2018 by Roman Savochenko, <rom_as@oscada.org>      *
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

#include <sys/types.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <errno.h>

#include "tsys.h"
#include "resalloc.h"
#include "tmess.h"

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif
#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif
#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#endif

using namespace OSCADA;

//*************************************************
//* TMess                                         *
//*************************************************
TMess::TMess( ) : IOCharSet("UTF-8"), mMessLevel(Info), mLogDir(DIR_STDOUT|DIR_ARCHIVE),
    mConvCode(true), mIsUTF8(true), mRes(true), mLang2CodeBase(mRes), mLang2Code(mRes)
{
    openlog(PACKAGE, 0, LOG_USER);

    setenv("LC_NUMERIC", "C", 1);
    setlocale(LC_ALL, "");
#ifdef HAVE_LANGINFO_H
    IOCharSet = nl_langinfo(CODESET);
#endif

#ifdef HAVE_LIBINTL_H
    bindtextdomain(PACKAGE,LOCALEDIR);
    textdomain(PACKAGE);
#endif

    string tLng = lang();
    mLang2Code = tLng;
    if(mLang2Code.size() < 2 || mLang2Code.getVal() == "POSIX" || mLang2Code.getVal() == "C") mLang2Code = "en";
    else mLang2Code = mLang2Code.getVal().substr(0,2);
    mIsUTF8 = (IOCharSet == "UTF-8" || IOCharSet == "UTF8" || IOCharSet == "utf8");

    if(tLng == "C" || (mLang2Code.getVal() == "en" && (IOCharSet == "ISO-8859-1" || IOCharSet == "ANSI_X3.4-1968" || IOCharSet == "ASCII" || IOCharSet == "US-ASCII")))
	mConvCode = false;
}

TMess::~TMess( )
{
    closelog();
}

void TMess::setMessLevel( int level )
{
    mMessLevel = vmax(Debug, vmin(Crit,level));
    SYS->modif();
}

void TMess::setLogDirect( int dir )
{
    mLogDir = dir;
    SYS->modif();
}

void TMess::put( const char *categ, int8_t level, const char *fmt,  ... )
{
    char mess[STR_BUF_LEN];
    va_list argptr;

    va_start(argptr,fmt);
    vsnprintf(mess,sizeof(mess),fmt,argptr);
    va_end(argptr);

    level = vmin(Emerg, vmax(-Emerg,level));
    if(abs(level) < messLevel()) return;

    int64_t ctm = TSYS::curTime();
    //string sMess = i2s(level) + "|" + categ + " | " + mess;
    string sMess = i2s(level) + "[" + categ + "] " + mess;

    if(mLogDir&DIR_SYSLOG) {
	int level_sys;
	switch((int8_t)abs(level)) {
	    case Debug:		level_sys = LOG_DEBUG;	break;
	    case Info:		level_sys = LOG_INFO;	break;
	    case Notice:	level_sys = LOG_NOTICE;	break;
	    case Warning:	level_sys = LOG_WARNING;break;
	    case Error:		level_sys = LOG_ERR;	break;
	    case Crit:		level_sys = LOG_CRIT;	break;
	    case Alert:		level_sys = LOG_ALERT;	break;
	    case Emerg:		level_sys = LOG_EMERG;	break;
	    default: 		level_sys = LOG_DEBUG;
	}
	syslog(level_sys, "%s", sMess.c_str());
    }
    if(mLogDir&(DIR_STDOUT|DIR_STDERR) && SYS->cmdOpt("consoleCharSet").size())
	sMess = Mess->codeConvOut(SYS->cmdOpt("consoleCharSet"), sMess);
    if(mLogDir&DIR_STDOUT)	fprintf(stdout, "%s %s\n", atm2s(time(NULL),"%Y-%m-%dT%H:%M:%S").c_str(), sMess.c_str());
    if(mLogDir&DIR_STDERR)	fprintf(stderr, "%s %s\n", atm2s(time(NULL),"%Y-%m-%dT%H:%M:%S").c_str(), sMess.c_str());

    if((mLogDir&DIR_ARCHIVE) && SYS->present("Archive"))
	SYS->archive().at().messPut(ctm/1000000, ctm%1000000, categ, level, mess);
}

void TMess::get( time_t b_tm, time_t e_tm, vector<TMess::SRec> &recs, const string &category, int8_t level )
{
    if(mLogDir & DIR_ARCHIVE)	SYS->archive().at().messGet(b_tm, e_tm, recs, category, level);
}

string TMess::lang( )
{
    char *lng = NULL;
    if( ((lng=getenv("LANGUAGE")) && strlen(lng)) ||
	    ((lng=getenv("LC_MESSAGES")) && strlen(lng)) ||
	    ((lng=getenv("LANG")) && strlen(lng)) )
	return lng;
    else return "C";
}

void TMess::setLang( const string &lng, bool init )
{
    char *prvLng = NULL;
    if((prvLng=getenv("LANGUAGE")) && strlen(prvLng)) setenv("LANGUAGE", lng.c_str(), 1);
    //else setenv("LC_MESSAGES", lng.c_str(), 1);
    else setenv("LANG", lng.c_str(), 1);	//!!!! May be use further for the miss environment force set
    setlocale(LC_ALL, "");

#ifdef HAVE_LANGINFO_H
    IOCharSet = nl_langinfo(CODESET);
#endif

    string tLng = lang();
    mLang2Code = tLng;
    if(mLang2Code.size() < 2 || mLang2Code.getVal() == "POSIX" || mLang2Code.getVal() == "C") mLang2Code = "en";
    else mLang2Code = mLang2Code.getVal().substr(0, 2);
    mIsUTF8 = (IOCharSet == "UTF-8" || IOCharSet == "UTF8" || IOCharSet == "utf8");

    mConvCode = !(tLng == "C" || (mLang2Code.getVal() == "en" && (IOCharSet == "ISO-8859-1" || IOCharSet == "ANSI_X3.4-1968" || IOCharSet == "ASCII" || IOCharSet == "US-ASCII")));

    if(init) SYS->sysModifFlgs &= ~TSYS::MDF_LANG;
    else { SYS->sysModifFlgs |= TSYS::MDF_LANG; SYS->modif(); }
}

string TMess::codeConv( const string &fromCH, const string &toCH, const string &mess )
{
    if(!mConvCode || fromCH == toCH) return mess;

#ifdef HAVE_ICONV_H
    //Make convert to blocks 1000 bytes
    string buf;
    buf.reserve(mess.size());
# if defined(__UCLIBC__)
    const char	*ibuf;
# else
    char	*ibuf;
# endif
    char	outbuf[1000], *obuf;
    size_t ilen, olen, chwrcnt = 0;
    iconv_t hd;

    hd = iconv_open(toCH.c_str(), fromCH.c_str());
    if(hd == (iconv_t)(-1)) {
	mess_crit("IConv", _("Error opening 'iconv': %s"), strerror(errno));
	return mess;
    }

    ibuf = (char *)mess.data();
    ilen = mess.size();

    while(ilen) {
	obuf = outbuf;
	olen = sizeof(outbuf)-1;
	size_t rez = iconv(hd, &ibuf, &ilen, &obuf, &olen);
	if(rez == (size_t)(-1) && (errno == EINVAL || errno == EBADF)) {
	    mess_crit("IConv", _("Error converting input sequence: %s"), strerror(errno));
	    buf = mess;
	    break;
	}
	if(obuf > outbuf) buf.append(outbuf, obuf-outbuf);
	if(rez == (size_t)(-1) && errno == EILSEQ) { buf += '?'; ilen--; ibuf++; chwrcnt++; }
    }
    iconv_close(hd);

    //> Deadlock possible on the error message print
    //if(chwrcnt)	mess_err("IConv", _("Error converting %d symbols from '%s' to '%s' for the message part: '%s'(%d)"),
    //		    chwrcnt, fromCH.c_str(), toCH.c_str(), mess.substr(0,20).c_str(), mess.size());

    return buf;
#else
    return mess;
#endif
}

const char *TMess::I18N( const char *mess, const char *d_name )
{
#ifdef HAVE_LIBINTL_H
    return dgettext(d_name, mess);
#else
    return mess;
#endif
}

void TMess::setLang2CodeBase( const string &vl )
{
    mLang2CodeBase = vl;
    if((!mLang2CodeBase.empty() && mLang2CodeBase.size() < 2) || mLang2CodeBase.getVal() == "POSIX" || mLang2CodeBase.getVal() == "C")
	mLang2CodeBase = "en";
    else mLang2CodeBase = mLang2CodeBase.getVal().substr(0,2);

    SYS->modif();
}

void TMess::load( )
{
    //Load params from command line
    string argVl;
    if((argVl=SYS->cmdOpt("lang")).size()) setLang(argVl, true);
    if((argVl=SYS->cmdOpt("messLev")).size()) {
	int i = s2i(argVl);
	if(i >= Debug && i <= Emerg) setMessLevel(i);
    }
    if((argVl=SYS->cmdOpt("log")).size()) setLogDirect(s2i(argVl));

    //Load params config-file
    setMessLevel(s2i(TBDS::genDBGet(SYS->nodePath()+"MessLev",i2s(messLevel()),"root",TBDS::OnlyCfg)));
    setLogDirect(s2i(TBDS::genDBGet(SYS->nodePath()+"LogTarget",i2s(logDirect()),"root",TBDS::OnlyCfg)));
    setLang(TBDS::genDBGet(SYS->nodePath()+"Lang",lang(),"root",TBDS::OnlyCfg), true);
    mLang2CodeBase = TBDS::genDBGet(SYS->nodePath()+"Lang2CodeBase",mLang2CodeBase,"root",TBDS::OnlyCfg);
}

void TMess::save( )
{
    TBDS::genDBSet(SYS->nodePath()+"MessLev",i2s(messLevel()),"root",TBDS::OnlyCfg);
    TBDS::genDBSet(SYS->nodePath()+"LogTarget",i2s(logDirect()),"root",TBDS::OnlyCfg);
    if(SYS->sysModifFlgs&TSYS::MDF_LANG) TBDS::genDBSet(SYS->nodePath()+"Lang",lang(),"root",TBDS::OnlyCfg);
    TBDS::genDBSet(SYS->nodePath()+"Lang2CodeBase",mLang2CodeBase,"root",TBDS::OnlyCfg);
}

const char *TMess::labDB( )
{
    return _("DB address in format \"{DB module}.{DB name}\".\n"
	     "Set '*.*' for use the main work DB.");
}

const char *TMess::labSecCRON( )
{
    return _("Schedule wrotes in seconds periodic form or in the standard CRON form.\n"
	     "The seconds periodic form is one real number (1.5, 1e-3).\n"
	     "Cron is the standard form \"* * * * *\".\n"
	     "  Where items by order:\n"
	     "    - minutes (0-59);\n"
	     "    - hours (0-23);\n"
	     "    - days (1-31);\n"
	     "    - month (1-12);\n"
	     "    - week day (0[Sunday]-6).\n"
	     "  Where an item variants:\n"
	     "    - \"*\" - any value;\n"
	     "    - \"1,2,3\" - allowed values list;\n"
	     "    - \"1-5\" - raw range of allowed values;\n"
	     "    - \"*/2\" - range divider for allowed values.\n"
	     "Examples:\n"
	     "  \"1e-3\" - call with a period of one millisecond;\n"
	     "  \"* * * * *\" - each minute;\n"
	     "  \"10 23 * * *\" - only at 23 hour and 10 minute for any day and month;\n"
	     "  \"*/2 * * * *\" - for minutes: 0,2,4,...,56,58;\n"
	     "  \"* 2-4 * * *\" - for any minutes in hours from 2 to 4(include).");
}

const char *TMess::labSecCRONsel( )	{ return "1;1e-3;* * * * *;10 * * * *;10-20 2 */2 * *"; }

const char *TMess::labTaskPrior( )
{
    return _("Task priority level (-1...199), where:\n"
	     "  -1        - lowest priority batch policy;\n"
	     "  0         - standard userspace priority;\n"
	     "  1...99    - realtime priority level (round-robin), often allowed only for \"root\";\n"
	     "  100...199 - realtime priority level (FIFO), often allowed only for \"root\".");
}

const char *TMess::labMessCat( )
{
    return _("Specifies the category of the requested messages.\n"
	     "Use template's symbols for group selection:\n  '*' - any substring;\n  '?' - any character.\n"
	     "Regular expression enclosed between the symbols '/' (/mod_(System|LogicLev)/).");
}

int TMess::getUTF8( const string &str, int off, int32_t *symb )
{
    if(off < 0 || off >= (int)str.size())	return 0;
    if(!isUTF8() || !(str[off]&0x80)) {
	if(symb) *symb = (uint8_t)str[off];
	return 1;
    }
    int len = 0;
    int32_t rez = 0;
    if((str[off]&0xE0) == 0xC0)		{ len = 2; rez = str[off]&0x1F; }
    else if((str[off]&0xF0) == 0xE0)	{ len = 3; rez = str[off]&0x0F; }
    else if((str[off]&0xF8) == 0xF0)	{ len = 4; rez = str[off]&0x07; }
    if((off+len) > (int)str.size())	return 0;
    for(int iSmb = 1; iSmb < len; iSmb++)
	if((str[off+iSmb]&0xC0) != 0x80) return 0;
	else rez = (rez<<6) | (str[off+iSmb]&0x3F);
    if(symb) *symb = rez;
    return len;
}
