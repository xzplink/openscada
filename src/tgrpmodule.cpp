
#include <stdarg.h>
#include <unistd.h>

#include "tsys.h"
#include "tkernel.h"
#include "tmessage.h"
#include "tgrpmodule.h"

const char *TGRPModule::o_name = "TGRPModule";
const char *TGRPModule::i_cntr = 
	"<oscada_cntr>"
	" <area id='a_mod' dscr='Modules'>"
	"  <list id='mod_br' dscr='Modules' tp='br' mode='att' acs='0555'/>"
        " </area>"  
	"</oscada_cntr>"; 
 

TGRPModule::TGRPModule( TKernel *app, char *NameT ) : 
	NameType(NameT), DirPath(""), owner(app), m_hd(o_name), TContr( i_cntr )
{

}

TGRPModule::~TGRPModule(  )
{
    vector<string> list;
    m_hd.lock();
    gmd_list(list);
    for(unsigned i_m = 0; i_m < list.size(); i_m++) 
    {
	try{ gmd_del(list[i_m]); } catch(...){ }
    }
}

void TGRPModule::gmd_add( TModule *modul )
{
    m_hd.obj_add( modul, &modul->mod_Name() );
    modul->mod_connect(this);
#if OSC_DEBUG 
    m_put_s("DEBUG",MESS_DEBUG,"-------------------------------------");
    vector<string> list;
    modul->mod_info( list );
    for( unsigned i_opt = 0; i_opt < list.size(); i_opt++)
    	m_put("DEBUG",MESS_DEBUG,"| %s: %s",list[i_opt].c_str(),modul->mod_info(list[i_opt]).c_str());
    m_put_s("DEBUG",MESS_DEBUG,"-------------------------------------");
#endif
}

void TGRPModule::gmd_del( string name )
{
#if OSC_DEBUG 
    m_put("DEBUG",MESS_INFO,"Disconnect modul <%s>!",name.c_str() );
#endif

    delete (TModule *)m_hd.obj_del( name );
    
#if OSC_DEBUG 
    m_put("DEBUG",MESS_DEBUG,"Disconnect modul <%s> ok!",name.c_str() );
#endif
}

TModule *TGRPModule::gmd_FUse(unsigned int hd, char * func, void (TModule::**offptr)())
{
    TModule &mod = gmd_at(hd);
    mod.mod_GetFunc(func, offptr);
    return(&mod);
}

void TGRPModule::gmd_CheckCommandLineMods()
{
    vector<string> list;
    gmd_list(list);
    for(unsigned i_m=0; i_m < list.size(); i_m++)
    {
	unsigned hd = gmd_att(list[i_m]);
	gmd_at(hd).mod_CheckCommandLine( );
	gmd_det(hd);
    }
}

void TGRPModule::gmd_UpdateOptMods()
{
    vector<string> list;
    gmd_list(list);
    for(unsigned i_m=0; i_m < list.size(); i_m++)
    {
	unsigned hd = gmd_att(list[i_m]);
       	gmd_at(hd).mod_UpdateOpt();	    
	gmd_det(hd);
    }	
}

XMLNode *TGRPModule::gmd_XMLCfgNode()
{
    int i_k = 0;
    while(true)
    {
	XMLNode *t_n = Owner().XMLCfgNode()->get_child("section",i_k++);
	if( t_n->get_attr("id") == gmd_Name() ) return( t_n );
    }
}

void TGRPModule::gmd_CheckCommandLine( )
{
#if OSC_DEBUG
    m_put_s("DEBUG",MESS_INFO,"Read commandline options!" );
#endif
}

void TGRPModule::gmd_UpdateOpt()
{
#if OSC_DEBUG
    m_put_s("DEBUG",MESS_INFO,"Read config options!");
#endif
}

//==============================================================
//================== Controll functions ========================
//==============================================================
void TGRPModule::ctr_fill_info( XMLNode *inf )
{
    inf->set_text(string(Name()+" subsystem"));    
}

void TGRPModule::ctr_din_get_( string path, XMLNode *opt )
{
    string t_id = ctr_path_l(path,0);
    if( t_id == "a_mod" )
	if( ctr_path_l(path,1) == "mod_br" )
	{
	    vector<string> list;
	    gmd_list(list);
	    for( unsigned i_a=0; i_a < list.size(); i_a++ )
		ctr_opt_setS( opt, list[i_a], i_a );         
	}
} 

unsigned TGRPModule::ctr_att( string br ) 
{
    if( ctr_path_l(br,0) == "a_mod" && ctr_path_l(br,1) == "mod_br" )
    	return( gmd_att( ctr_path_l(br,2) ) ); 
    throw TError("(%s) Branch %s error!",o_name,br.c_str());
}

void TGRPModule::ctr_det( string br, unsigned hd ) 
{ 
    if( ctr_path_l(br,0) == "a_mod" && ctr_path_l(br,1) == "mod_br" )
    { gmd_det( hd ); return; }
    throw TError("(%s) Branch %s error!",o_name,br.c_str());
}

TContr &TGRPModule::ctr_at( string br, unsigned hd )   
{ 
    if( ctr_path_l(br,0) == "a_mod" && ctr_path_l(br,1) == "mod_br" )
	return( (TContr&)gmd_at(hd) ); 
    throw TError("(%s) Branch %s error!",o_name,br.c_str());
} 

//==============================================================
//================== Message functions ========================
//==============================================================
void TGRPModule::m_put( string categ, int level, char *fmt,  ... )
{
    char str[STR_BUF_LEN];
    va_list argptr;

    va_start (argptr,fmt);
    vsnprintf(str,sizeof(str),fmt,argptr);
    va_end(argptr);
    m_put_s( categ, level, str );
}

void TGRPModule::m_put_s( string categ, int level, string mess )
{
    Owner().m_put_s( categ, level, gmd_Name()+":"+mess );
}


