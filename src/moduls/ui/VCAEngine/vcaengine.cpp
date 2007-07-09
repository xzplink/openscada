
//OpenSCADA system module UI.VCAEngine file: vcaengine.cpp
/***************************************************************************
 *   Copyright (C) 2006-2007 by Roman Savochenko
 *   rom_as@diyaorg.dp.ua
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/

#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>

#include <tsys.h>
#include <tmess.h>

#include "origwidg.h"
#include "vcaengine.h"

//============ Modul info! =====================================================
#define MOD_ID      "VCAEngine"
#define MOD_NAME    "Visual control area engine"
#define MOD_TYPE    "UI"
#define MOD_SUBTYPE "VCAEngine"
#define VER_TYPE    VER_UI
#define VERSION     "0.0.1"
#define AUTORS      "Roman Savochenko"
#define DESCRIPTION "Generic visual control area engine."
#define LICENSE     "GPL"
//==============================================================================

VCA::Engine *VCA::mod;

extern "C"
{
    TModule::SAt module( int n_mod )
    {
    	TModule::SAt AtMod;

	if(n_mod==0)
	{
	    AtMod.id	= MOD_ID;
	    AtMod.type  = MOD_TYPE;
    	    AtMod.t_ver = VER_TYPE;
	}
	else AtMod.id	= "";

	return AtMod;
    }

    TModule *attach( const TModule::SAt &AtMod, const string &source )
    {
	VCA::Engine *self_addr = NULL;

	if( AtMod.id == MOD_ID && AtMod.type == MOD_TYPE && AtMod.t_ver == VER_TYPE )
	    self_addr = VCA::mod = new VCA::Engine( source );

	return self_addr;
    }
}

using namespace VCA;

//************************************************
//* VCA::Engine                                  *
//************************************************
Engine::Engine( string name )
{
    mId		= MOD_ID;
    mName       = MOD_NAME;
    mType  	= MOD_TYPE;
    mVers      	= VERSION;
    mAutor    	= AUTORS;
    mDescr  	= DESCRIPTION;
    mLicense   	= LICENSE;
    mSource    	= name;

    id_wlb = grpAdd("wlb_");
    id_prj = grpAdd("prj_");
    id_ses = grpAdd("ses_");
}

Engine::~Engine()
{
    nodeDelAll();
}

void Engine::modInfo( vector<string> &list )
{
    TModule::modInfo(list);
    list.push_back("SubType");
}

string Engine::modInfo( const string &name )
{
    if( name == "SubType" ) return MOD_SUBTYPE;
    else return TModule::modInfo( name);
}

string Engine::optDescr( )
{
    char buf[STR_BUF_LEN];

    snprintf(buf,sizeof(buf),_(
        "======================= The module <%s:%s> options =======================\n"
        "---------- Parameters of the module section <%s> in config file ----------\n\n"),
    	    MOD_TYPE,MOD_ID,nodePath().c_str());

    return buf;
}

void Engine::postEnable( int flag )
{
    TModule::postEnable( flag );
    
    if( !(flag&TCntrNode::NodeConnect) ) return;
    
    //- Make lib's DB structure -
    lbwdg_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"20") );
    lbwdg_el.fldAdd( new TFld("NAME",_("Name"),TFld::String,TFld::NoFlag,"50") );
    lbwdg_el.fldAdd( new TFld("DESCR",_("Description"),TFld::String,TFld::NoFlag,"300") );
    lbwdg_el.fldAdd( new TFld("DB_TBL",_("Data base"),TFld::String,TFld::NoFlag,"30") );
    lbwdg_el.fldAdd( new TFld("ICO",_("Icon"),TFld::String,TFld::NoFlag,"10000") );
    lbwdg_el.fldAdd( new TFld("USER",_("User"),TFld::String,TFld::NoFlag,"20","root") );
    lbwdg_el.fldAdd( new TFld("GRP",_("Group"),TFld::String,TFld::NoFlag,"20","UI") );
    lbwdg_el.fldAdd( new TFld("PERMIT",_("Permision"),TFld::Integer,TFld::OctDec,"3","436") );

    //- Make library widgets data container -    
    wdgdata_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"30") );
    wdgdata_el.fldAdd( new TFld("MIME",_("Mime type"),TFld::String,TFld::NoFlag,"30") );
    wdgdata_el.fldAdd( new TFld("DATA",_("Mime data"),TFld::String,TFld::NoFlag,"500000") );
    
    //- Make widgets DB structure -
    wdg_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"20") );
    wdg_el.fldAdd( new TFld("ICO",_("Icon"),TFld::String,TFld::NoFlag,"10000") );
    wdg_el.fldAdd( new TFld("PARENT",_("Parent widget"),TFld::String,TFld::NoFlag,"200") );
    wdg_el.fldAdd( new TFld("PROC",_("Procedure text and language"),TFld::String,TFld::NoFlag,"10000") );
    wdg_el.fldAdd( new TFld("USER",_("User"),TFld::String,TFld::NoFlag,"20","root") );
    wdg_el.fldAdd( new TFld("GRP",_("Group"),TFld::String,TFld::NoFlag,"20","UI") );
    wdg_el.fldAdd( new TFld("PERMIT",_("Permision"),TFld::Integer,TFld::OctDec,"3","436") );

    //- Make include widgets DB structure -
    inclwdg_el.fldAdd( new TFld("IDW",_("IDW"),TFld::String,TCfg::Key,"100") );
    inclwdg_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"20") );
    inclwdg_el.fldAdd( new TFld("PARENT",_("Parent widget"),TFld::String,TFld::NoFlag,"200") );

    //- Make widget's IO DB structure -
    wdgio_el.fldAdd( new TFld("IDW",_("Widget ID"),TFld::String,TCfg::Key,"100") );
    wdgio_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"40") );
    wdgio_el.fldAdd( new TFld("IO_VAL",_("Attribute value"),TFld::String,TFld::NoFlag,"100000") );
    wdgio_el.fldAdd( new TFld("SELF_FLG",_("Attribute self flags"),TFld::Integer,TFld::NoFlag,"5") );
    wdgio_el.fldAdd( new TFld("CFG_TMPL",_("Configuration template"),TFld::String,TFld::NoFlag,"30") );
    wdgio_el.fldAdd( new TFld("CFG_VAL",_("Configuration value"),TFld::String,TFld::NoFlag,"1000") );
    
    //- Make widget's user IO DB structure -
    wdguio_el.fldAdd( new TFld("IDW",_("Widget ID"),TFld::String,TCfg::Key,"100") );
    wdguio_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"40") );
    wdguio_el.fldAdd( new TFld("NAME",_("Name"),TFld::String,TFld::NoFlag,"50") );
    wdguio_el.fldAdd( new TFld("IO_TYPE",_("Attribute generic flags and type"),TFld::Integer,TFld::NoFlag,"10") );
    wdguio_el.fldAdd( new TFld("IO_VAL",_("Attribute value"),TFld::String,TFld::NoFlag,"100000") );
    wdguio_el.fldAdd( new TFld("SELF_FLG",_("Attribute self flags"),TFld::Integer,TFld::NoFlag,"5") );
    wdguio_el.fldAdd( new TFld("CFG_TMPL",_("Configuration template"),TFld::String,TFld::NoFlag,"30") );
    wdguio_el.fldAdd( new TFld("CFG_VAL",_("Configuration value"),TFld::String,TFld::NoFlag,"1000") );    

    //- Make project's DB structure -
    prj_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"20") );
    prj_el.fldAdd( new TFld("NAME",_("Name"),TFld::String,TFld::NoFlag,"50") );
    prj_el.fldAdd( new TFld("DESCR",_("Description"),TFld::String,TFld::NoFlag,"300") );
    prj_el.fldAdd( new TFld("DB_TBL",_("Data base"),TFld::String,TFld::NoFlag,"30") );
    prj_el.fldAdd( new TFld("ICO",_("Icon"),TFld::String,TFld::NoFlag,"10000") );
    prj_el.fldAdd( new TFld("USER",_("User"),TFld::String,TFld::NoFlag,"20","root") );
    prj_el.fldAdd( new TFld("GRP",_("Group"),TFld::String,TFld::NoFlag,"20","UI") );
    prj_el.fldAdd( new TFld("PERMIT",_("Permision"),TFld::Integer,TFld::OctDec,"3","436") );

    //- Make pages DB structure -
    page_el.fldAdd( new TFld("OWNER",_("Owner"),TFld::String,TCfg::Key,"100") );
    page_el.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"20") );
    page_el.fldAdd( new TFld("ICO",_("Icon"),TFld::String,TFld::NoFlag,"10000") );
    page_el.fldAdd( new TFld("PARENT",_("Parent widget"),TFld::String,TFld::NoFlag,"200") );
    page_el.fldAdd( new TFld("PROC",_("Procedure text and language"),TFld::String,TFld::NoFlag,"10000") );
    page_el.fldAdd( new TFld("USER",_("User"),TFld::String,TFld::NoFlag,"20","root") );
    page_el.fldAdd( new TFld("GRP",_("Group"),TFld::String,TFld::NoFlag,"20","UI") );
    page_el.fldAdd( new TFld("PERMIT",_("Permision"),TFld::Integer,TFld::OctDec,"3","436") );
    page_el.fldAdd( new TFld("FLGS",_("Flags"),TFld::Integer,TFld::NoFlag,"1","0") );

    //- Init original widgets library -
    wlbAdd("originals",_("Original widget's library"));
    //-- Set default library icon --
    if( TUIS::icoPresent("VCA.lwdg_root") )
        wlbAt("originals").at().setIco(TSYS::strEncode(TUIS::icoGet("VCA.lwdg_root"),TSYS::base64));
    //-- Add main original widgets --
    wlbAt("originals").at().add( new OrigElFigure() );
    wlbAt("originals").at().add( new OrigFormEl() );
    wlbAt("originals").at().add( new OrigText() );
    wlbAt("originals").at().add( new OrigMedia() );
    wlbAt("originals").at().add( new OrigTrend() );
    wlbAt("originals").at().add( new OrigProtocol() );
    wlbAt("originals").at().add( new OrigDocument() );
    wlbAt("originals").at().add( new OrigFunction() );
    wlbAt("originals").at().add( new OrigUserEl() );
    wlbAt("originals").at().add( new OrigLink() );
}

void Engine::preDisable( int flag )
{
    modStop();

    TModule::preDisable( flag );
}

void Engine::modLoad( )
{
#if OSC_DEBUG
    mess_debug(nodePath().c_str(),_("Load module."));
#endif    

    //- Load parameters from command line -
    int next_opt;
    char *short_opt="h";
    struct option long_opt[] =
    {
        {"help"    ,0,NULL,'h'},
        {NULL      ,0,NULL,0  }
    };

    optind=opterr=0;
    do
    {
        next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
        switch(next_opt)
        {
    	    case 'h': fprintf(stdout,optDescr().c_str()); break;
            case -1 : break;
        }
    } while(next_opt != -1);

    //- Load parameters from config file and DB -
    //-- Load widget's libraries --
    try
    {
        //--- Search and create new libraries ---
        TConfig c_el(&elWdgLib());
	c_el.cfgViewAll(false);
        vector<string> tdb_ls, db_ls;

	//---- Search into DB ----
	SYS->db().at().modList(tdb_ls);
        for( int i_tp = 0; i_tp < tdb_ls.size(); i_tp++ )
        {
            SYS->db().at().at(tdb_ls[i_tp]).at().list(db_ls);
            for( int i_db = 0; i_db < db_ls.size(); i_db++ )
            {
                string wbd=tdb_ls[i_tp]+"."+db_ls[i_db];
                int lib_cnt = 0;
                while(SYS->db().at().dataSeek(wbd+"."+wlbTable(),"",lib_cnt++,c_el) )
                {
                    string l_id = c_el.cfg("ID").getS();
        	    c_el.cfg("ID").setS("");		    
                    if(!wlbPresent(l_id)) wlbAdd(l_id,"",(wbd==SYS->workDB())?"*.*":wbd);
                }
            }
        }

	//---- Search into config file ----
        int lib_cnt = 0;
        while(SYS->db().at().dataSeek("",nodePath()+"lib/",lib_cnt++,c_el) )
        {
            string l_id = c_el.cfg("ID").getS();
    	    c_el.cfg("ID").setS("");	    
            if( !wlbPresent(l_id) )	wlbAdd(l_id,"","*.*");
	}

	//--- Load present libraries ---
        wlbList(tdb_ls);
        for( int l_id = 0; l_id < tdb_ls.size(); l_id++ )
    	    wlbAt(tdb_ls[l_id]).at().load();
    }catch( TError err )
    {
	mess_err(err.cat.c_str(),"%s",err.mess.c_str());
        mess_err(nodePath().c_str(),_("Load widget's libraries error."));
    }
    
    //-- Load projects --
    try
    {
        //--- Search and create new projects ---
        TConfig c_el(&elProject());
	c_el.cfgViewAll(false);
        vector<string> tdb_ls, db_ls;

	//---- Search into DB ----
	SYS->db().at().modList(tdb_ls);
        for( int i_tp = 0; i_tp < tdb_ls.size(); i_tp++ )
        {
            SYS->db().at().at(tdb_ls[i_tp]).at().list(db_ls);
            for( int i_db = 0; i_db < db_ls.size(); i_db++ )
            {
                string wbd=tdb_ls[i_tp]+"."+db_ls[i_db];
                int lib_cnt = 0;
                while(SYS->db().at().dataSeek(wbd+"."+prjTable(),"",lib_cnt++,c_el) )
                {
                    string prj_id = c_el.cfg("ID").getS();
        	    c_el.cfg("ID").setS("");		    
                    if( !prjPresent(prj_id) )	prjAdd(prj_id,"",(wbd==SYS->workDB())?"*.*":wbd);
                }
            }
        }

	//---- Search into config file ----
        int el_cnt = 0;
        while(SYS->db().at().dataSeek("",nodePath()+"prj/",el_cnt++,c_el) )
        {
            string prj_id = c_el.cfg("ID").getS();
    	    c_el.cfg("ID").setS("");	    
            if( !prjPresent(prj_id) )	prjAdd(prj_id,"","*.*");
	}

	//--- Load present libraries ---
        prjList(tdb_ls);
        for( int el_id = 0; el_id < tdb_ls.size(); el_id++ )
    	    prjAt(tdb_ls[el_id]).at().load();
    }catch( TError err )
    {
	mess_err(err.cat.c_str(),"%s",err.mess.c_str());
        mess_err(nodePath().c_str(),_("Load projects error."));
    }    
}

void Engine::modSave( )
{
#if OSC_DEBUG
    mess_debug(nodePath().c_str(),_("Save module."));
#endif

    //- Save parameters -
    //-- Save widget's libraries --
    vector<string> ls;
    wlbList(ls);
    for( int l_id = 0; l_id < ls.size(); l_id++ )
	wlbAt(ls[l_id]).at().save();
    //-- Save projects --
    prjList(ls);
    for( int p_id = 0; p_id < ls.size(); p_id++ )
	prjAt(ls[p_id]).at().save();
}

void Engine::modStart()
{
#if OSC_DEBUG
    mess_debug(nodePath().c_str(),_("Start module."));
#endif

    //- Libraries start -
    vector<string> ls;
    wlbList(ls);
    for( int l_id = 0; l_id < ls.size(); l_id++ )
	wlbAt(ls[l_id]).at().setEnable(true);
	
    //- Projects start -
    prjList(ls);
    for( int l_id = 0; l_id < ls.size(); l_id++ )
	prjAt(ls[l_id]).at().setEnable(true);

    run_st = true;
}

void Engine::modStop()
{
#if OSC_DEBUG
    mess_debug(nodePath().c_str(),_("Stop module."));
#endif

    vector<string> ls;
    
    //- Stop sessions -
    sesList(ls);
    for( int l_id = 0; l_id < ls.size(); l_id++ )
	sesAt(ls[l_id]).at().setEnable(false);
	
    //- Libraries stop -
    wlbList(ls);
    for( int l_id = 0; l_id < ls.size(); l_id++ )
	wlbAt(ls[l_id]).at().setEnable(false);

    //- Projects stop -
    prjList(ls);
    for( int l_id = 0; l_id < ls.size(); l_id++ )
	prjAt(ls[l_id]).at().setEnable(false);

    run_st = false;
}

void Engine::wlbAdd( const string &iid, const string &inm, const string &idb )
{
    if(wlbPresent(iid))	return;
    chldAdd(id_wlb, new WidgetLib(iid,inm,idb));
}

AutoHD<WidgetLib> Engine::wlbAt( const string &id )
{
    return chldAt(id_wlb,id);
}

void Engine::prjAdd( const string &iid, const string &inm, const string &idb )
{
    if(prjPresent(iid))	return;
    chldAdd(id_prj, new Project(iid,inm,idb));
}

AutoHD<Project> Engine::prjAt( const string &id )
{
    return chldAt(id_prj,id);
}

void Engine::sesAdd( const string &iid, const string &iproj )
{
    if(sesPresent(iid))	return;
    chldAdd(id_ses, new Session(iid,iproj));
}

AutoHD<Session> Engine::sesAt( const string &id )
{
    return chldAt(id_ses,id);
}

void Engine::cntrCmdProc( XMLNode *opt )
{
    //- Get page info -
    if( opt->name() == "info" )
    {
        TUI::cntrCmdProc(opt);
        ctrMkNode("grp",opt,-1,"/br/prj_",_("Project"),0444,"root","root",1,"list","/prm/cfg/prj");	
        ctrMkNode("grp",opt,-1,"/br/wlb_",_("Widget's library"),0444,"root","root",1,"list","/prm/cfg/wlb");
        ctrMkNode("grp",opt,-1,"/br/ses_",_("Session"),0444,"root","root",1,"list","/ses/ses");	
	if(ctrMkNode("area",opt,-1,"/prm/cfg",_("Configuration"),0444,"root","root"))
	{
    	    ctrMkNode("list",opt,-1,"/prm/cfg/prj",_("Project"),0664,"root","UI",4,"tp","br","idm","1","s_com","add,del","br_pref","prj_");
    	    ctrMkNode("list",opt,-1,"/prm/cfg/wlb",_("Widget's libraries"),0664,"root","UI",4,"tp","br","idm","1","s_com","add,del","br_pref","wlb_");	    
	    ctrMkNode("comm",opt,-1,"/prm/cfg/load",_("Load"),0660);
    	    ctrMkNode("comm",opt,-1,"/prm/cfg/save",_("Save"),0660);
	}
	if(ctrMkNode("area",opt,1,"/ses",_("Sessions"),0444,"root","root"))
    	    ctrMkNode("list",opt,-1,"/ses/ses",_("Sessions"),0664,"root","UI",3,"tp","br","s_com","add,del","br_pref","ses_");
        return;
    }
    //- Process command for page -
    string a_path = opt->attr("path");
    if( a_path == "/prm/cfg/prj" )
    {
        if( ctrChkNode(opt,"get",0664,"root","root",SEQ_RD) )
        {
            vector<string> lst;
            prjList(lst);
            for( unsigned i_a=0; i_a < lst.size(); i_a++ )
                opt->childAdd("el")->setAttr("id",lst[i_a])->setText(prjAt(lst[i_a]).at().name());
        }
        if( ctrChkNode(opt,"add",0664,"root","root",SEQ_WR) )
	{
	    prjAdd(opt->attr("id"),opt->text());
	    prjAt(opt->attr("id")).at().setUser(opt->attr("user"));
	}
        if( ctrChkNode(opt,"del",0664,"root","root",SEQ_WR) )   prjDel(opt->attr("id"),true);
    }    
    else if( a_path == "/prm/cfg/wlb" )
    {
        if( ctrChkNode(opt,"get",0664,"root","root",SEQ_RD) )
        {
            vector<string> lst;
            wlbList(lst);
            for( unsigned i_a=0; i_a < lst.size(); i_a++ )
                opt->childAdd("el")->setAttr("id",lst[i_a])->setText(wlbAt(lst[i_a]).at().name());
        }
        if( ctrChkNode(opt,"add",0664,"root","root",SEQ_WR) )
	{
	    wlbAdd(opt->attr("id"),opt->text());
	    wlbAt(opt->attr("id")).at().setUser(opt->attr("user"));
	}
        if( ctrChkNode(opt,"del",0664,"root","root",SEQ_WR) )   wlbDel(opt->attr("id"),true);
    }    
    else if( a_path == "/prm/cfg/load" && ctrChkNode(opt,"set",0660,"root","root",SEQ_WR) )	modLoad();
    else if( a_path == "/prm/cfg/save" && ctrChkNode(opt,"set",0660,"root","root",SEQ_WR) )	modSave();
    else if( a_path == "/ses/ses" )
    {
        if( ctrChkNode(opt,"get",0664,"root","root",SEQ_RD) )
        {
            vector<string> lst;
            sesList(lst);
            for( unsigned i_a=0; i_a < lst.size(); i_a++ )
                opt->childAdd("el")->setText(lst[i_a]);
        }
        if( ctrChkNode(opt,"add",0664,"root","root",SEQ_WR) )
	{
	    sesAdd(opt->text());
	    sesAt(opt->text()).at().setUser(opt->attr("user"));
	}
        if( ctrChkNode(opt,"del",0664,"root","root",SEQ_WR) )   sesDel(opt->text(),true);
    }        
    else TUI::cntrCmdProc(opt);
}