
//OpenSCADA system module Archive.DBArch file: val.cpp
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <math.h>

#include <tsys.h>
#include <tmess.h>
#include "arch.h"
#include "val.h"

using namespace DBArch;

//*************************************************
//* DBArch::ModVArch - Value archivator           *
//*************************************************
ModVArch::ModVArch( const string &iid, const string &idb, TElem *cf_el ) : 
    TVArchivator(iid,idb,cf_el), m_max_size(cfg("DBArchSize").getRd())
{
    setAddr("*.*");
}

ModVArch::~ModVArch( )
{
    try{ stop(); }catch(...){}
}

void ModVArch::setValPeriod( double iper )
{
    TVArchivator::setValPeriod(iper);
}

void ModVArch::load( )
{
    TVArchivator::load();
    if( addr().empty() ) setAddr("*.*");
}

void ModVArch::start()
{
    //- Start getting data cycle -
    TVArchivator::start();
}

void ModVArch::stop()
{
    //- Stop getting data cicle an detach archives -
    TVArchivator::stop();
}

TVArchEl *ModVArch::getArchEl( TVArchive &arch )
{   
    return new ModVArchEl(arch,*this); 
}

void ModVArch::cntrCmdProc( XMLNode *opt )
{
    string grp = owner().owner().subId();
    //- Get page info -
    if( opt->name() == "info" )
    {
        TVArchivator::cntrCmdProc(opt);
        if( ctrMkNode("area",opt,1,"/bs",_("Additional options"),0444,"root",grp.c_str()) )
    	    ctrMkNode("fld",opt,-1,"/bs/sz",cfg("DBArchSize").fld().descr(),0664,"root",grp.c_str(),1,"tp","real");
        return;
    }
    
    //- Process command to page -
    string a_path = opt->attr("path");
    if( a_path == "/bs/sz" )
    {
        if( ctrChkNode(opt,"get",0664,"root",grp.c_str(),SEQ_RD) ) opt->setText(TSYS::real2str(m_max_size));
        if( ctrChkNode(opt,"set",0664,"root",grp.c_str(),SEQ_WR) ) m_max_size = atof(opt->text().c_str());
    }
    else TVArchivator::cntrCmdProc(opt);
}

//*************************************************
//* DBArch::ModVArchEl - Value archive element    *
//*************************************************
ModVArchEl::ModVArchEl( TVArchive &iachive, TVArchivator &iarchivator ) :
    TVArchEl(iachive,iarchivator), m_beg(0), m_end(0), m_per(0)
{
    //- Load message archive parameters -
    TConfig cfg(&mod->archEl());
    cfg.cfg("TBL").setS(archTbl());
    if(SYS->db().at().dataGet(archivator().addr()+"."+mod->mainTbl(),"",cfg))
    {
        m_beg = strtoll(cfg.cfg("BEGIN").getS().c_str(),NULL,10);
        m_end = strtoll(cfg.cfg("END").getS().c_str(),NULL,10);
	m_per = strtoll(cfg.cfg("PRM1").getS().c_str(),NULL,10);
    }
    if( !m_per ) m_per = (long long)(archivator().valPeriod()*1000000.);
}

ModVArchEl::~ModVArchEl( )
{

}

string ModVArchEl::archTbl( )
{ 
    return "DBAVl_"+archivator().id()+"_"+archive().id(); 
}

void ModVArchEl::fullErase()
{
    //- Remove info record -
    TConfig cfg(&mod->archEl());
    cfg.cfg("TBL").setS(archTbl());
    SYS->db().at().dataDel(archivator().addr()+"."+mod->mainTbl(),"",cfg);
    
    //- Remove archive's DB table -
    SYS->db().at().open( archivator().addr()+"."+archTbl() );
    SYS->db().at().close( archivator().addr()+"."+archTbl(), true );
}

void ModVArchEl::getVal( TValBuf &buf, long long ibeg, long long iend )
{
    //- Going border to period time -
    ibeg = (ibeg/period())*period();
    iend = (iend/period())*period();    

    //- Prepare border -
    ibeg = vmax( ibeg, begin() );    
    iend = vmin( iend, end() );
    
    if( iend < ibeg )	return;
    
    //- Get values -
    switch(archive().valType())
    {
	case TFld::Boolean: case TFld::Integer:
	{
	    TConfig cfg(&mod->vlIntEl());
	    for( long long c_tm = ibeg; c_tm <= iend; c_tm+=period() )
	    {
		cfg.cfg("TM").setI(c_tm/1000000);
		cfg.cfg("TMU").setI(c_tm%1000000);
		if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		{
		    if( archive().valType() == TFld::Integer )	buf.setI(cfg.cfg("VAL").getI(),c_tm);
		    else buf.setB(cfg.cfg("VAL").getI(),c_tm);
		}
		else buf.setI(EVAL_INT,c_tm);
	    }
	    break;
	}
	case TFld::Real:
	{
	    TConfig cfg(&mod->vlRealEl());
	    for( long long c_tm = ibeg; c_tm <= iend; c_tm+=period() )
	    {
		cfg.cfg("TM").setI(c_tm/1000000);
		cfg.cfg("TMU").setI(c_tm%1000000);
		if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		    buf.setR(cfg.cfg("VAL").getR(),c_tm);
		else buf.setR(EVAL_REAL,c_tm);
	    }
	    break;
	}
	case TFld::String:
	{
	    TConfig cfg(&mod->vlStrEl());
	    for( long long c_tm = ibeg; c_tm <= iend; c_tm+=period() )
	    {
		cfg.cfg("TM").setI(c_tm/1000000);
		cfg.cfg("TMU").setI(c_tm%1000000);
		if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		    buf.setS(cfg.cfg("VAL").getS(),c_tm);
		else buf.setS(EVAL_STR,c_tm);
	    }
	    break;
	}
    }
}

string ModVArchEl::getS( long long *tm, bool up_ord )
{   
    switch(archive().valType())
    {
	case TFld::Boolean: 	return TSYS::int2str(getB(tm,up_ord));
	case TFld::Integer:	return TSYS::int2str(getI(tm,up_ord));
	case TFld::Real:	return TSYS::real2str(getR(tm,up_ord));
	case TFld::String:
	{
	    long long itm = tm?*tm:SYS->curTime();
	    itm = (itm/period())*period()+((up_ord && itm%period())?period():0);

	    TConfig cfg(&mod->vlStrEl());
	    cfg.cfg("TM").setI(itm/1000000);
	    cfg.cfg("TMU").setI(itm%1000000);
	    if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		return cfg.cfg("VAL").getS();    
	    return EVAL_STR;
	}
    }
}

double ModVArchEl::getR( long long *tm, bool up_ord )
{
    switch(archive().valType())
    {
	case TFld::Boolean: 	return getB(tm,up_ord);
	case TFld::Integer:	return getI(tm,up_ord);
	case TFld::String:	return atof(getS(tm,up_ord).c_str());
	case TFld::Real:
	{
	    long long itm = tm?*tm:SYS->curTime();
	    itm = (itm/period())*period()+((up_ord && itm%period())?period():0);

	    TConfig cfg(&mod->vlRealEl());
	    cfg.cfg("TM").setI(itm/1000000);
	    cfg.cfg("TMU").setI(itm%1000000);
	    if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		return cfg.cfg("VAL").getR();    
	    return EVAL_REAL;
	}
    }
}

int ModVArchEl::getI( long long *tm, bool up_ord )
{
    switch(archive().valType())
    {
	case TFld::Boolean: 	return getB(tm,up_ord);
	case TFld::Real:	return (int)getR(tm,up_ord);
	case TFld::String:	return atoi(getS(tm,up_ord).c_str());
	case TFld::Integer:
	{
	    long long itm = tm?*tm:SYS->curTime();
	    itm = (itm/period())*period()+((up_ord && itm%period())?period():0);

	    TConfig cfg(&mod->vlIntEl());
	    cfg.cfg("TM").setI(itm/1000000);
	    cfg.cfg("TMU").setI(itm%1000000);
	    if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		return cfg.cfg("VAL").getI();    
	    return EVAL_INT;
	}
    }
}

char ModVArchEl::getB( long long *tm, bool up_ord )
{
    switch(archive().valType())
    {
	case TFld::Real:	return (int)getR(tm,up_ord);
	case TFld::String:	return atoi(getS(tm,up_ord).c_str());
	case TFld::Integer:	return getI(tm,up_ord);	
	case TFld::Boolean:
	{
	    long long itm = tm?*tm:SYS->curTime();
	    itm = (itm/period())*period()+((up_ord && itm%period())?period():0);

	    TConfig cfg(&mod->vlIntEl());
	    cfg.cfg("TM").setI(itm/1000000);
	    cfg.cfg("TMU").setI(itm%1000000);
	    if( SYS->db().at().dataGet(archivator().addr()+"."+archTbl(),"",cfg) )
		return cfg.cfg("VAL").getI();
    	    return EVAL_BOOL;
	}
    }
}

void ModVArchEl::setVal( TValBuf &buf, long long beg, long long end )
{    
    long long ctm;
    
    //- Check border -
    if( !buf.vOK(beg,end) )	return;
    beg = vmax(beg,buf.begin());
    end = vmin(end,buf.end());

    //- Table struct init -
    TConfig cfg( (archive().valType()==TFld::Real) ? (&mod->vlRealEl()) : 
		 (archive().valType()==TFld::String) ? (&mod->vlStrEl()) : &mod->vlIntEl() );    

    //- Write data to table -
    for( long long ctm; beg <= end; beg++ )
    {	    
	switch( archive().valType() )
	{
    	    case TFld::Boolean:	cfg.cfg("VAL").setI(buf.getB(&beg,true));	break;
	    case TFld::Integer:	cfg.cfg("VAL").setI(buf.getI(&beg,true));       break;
	    case TFld::Real:	cfg.cfg("VAL").setR(buf.getR(&beg,true));       break;
	    case TFld::String:	cfg.cfg("VAL").setS(buf.getS(&beg,true));       break;
	}
	ctm = (beg/period())*period();
	cfg.cfg("TM").setI(ctm/1000000);
	cfg.cfg("TMU").setI(ctm%1000000);
	SYS->db().at().dataSet(archivator().addr()+"."+archTbl(),"",cfg);
        //- Archive time border update -
        m_beg=m_beg?vmin(m_beg,ctm):ctm;
        m_end=m_end?vmax(m_end,ctm):ctm;
    }
    
    //- Archive size limit process -
    if( (m_end-m_beg) > (long long)(archivator().maxSize()*3600000000.) )
    {
        long long n_end = ((m_end-(long long)(archivator().maxSize()*3600000000.))/period())*period();
        for( long long t_c = m_beg; t_c < n_end; t_c+=period() )
	{
	    cfg.cfg("TM").setI(t_c/1000000);
	    cfg.cfg("TMU").setI(t_c%1000000);
            SYS->db().at().dataDel(archivator().addr()+"."+archTbl(),"",cfg);
        }
	m_beg=n_end;
    }
 
    //- Update archive info -
    cfg.setElem(&mod->archEl());
    cfg.cfgViewAll(false);
    cfg.cfg("TBL").setS(archTbl(),true);
    cfg.cfg("BEGIN").setS(TSYS::ll2str(m_beg),true);
    cfg.cfg("END").setS(TSYS::ll2str(m_end),true);
    cfg.cfg("PRM1").setS(TSYS::ll2str(m_per),true);    
    SYS->db().at().dataSet(archivator().addr()+"."+mod->mainTbl(),"",cfg);
}
