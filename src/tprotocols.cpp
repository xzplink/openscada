
#include <getopt.h>

#include "tsys.h"
#include "tkernel.h"
#include "tmessage.h"
#include "tprotocols.h"

//================================================================
//=========== TProtocolS =========================================
//================================================================
const char *TProtocolS::o_name = "TProtocolS";
const char *TProtocolS::i_cntr = 
    "<area id='a_pr' dscr='Protocols'>"
    " <area id='a_gn' dscr='Generic control.' acs='0440'>"
    "  <fld id='g_help' dscr='Options help' acs='0440' tp='str' cols='90' rows='5'/>"
    " </area>"    
    "</area>";

TProtocolS::TProtocolS( TKernel *app ) : TGRPModule(app,"Protocol") 
{
    s_name = "Protocols";
}

string TProtocolS::opt_descr(  )
{
    string rez;
    rez = rez+
    	"======================= "+gmd_Name()+" subsystem options ================\n"+
	"    --PRCModPath=<path>  Set moduls <path>;\n";

    return(rez);
}



void TProtocolS::gmd_CheckCommandLine( )
{
    TGRPModule::gmd_CheckCommandLine( );
    
    int next_opt;
    char *short_opt="h";
    struct option long_opt[] =
    {
	{"PRCModPath" ,1,NULL,'m'},
	{NULL        ,0,NULL,0  }
    };

    optind=opterr=0;	
    do
    {
	next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
    	{
	    case 'h': fprintf(stdout,opt_descr().c_str()); break;
	    case 'm': DirPath = optarg;     break;
	    case -1 : break;
	}
    } while(next_opt != -1);
}    

void TProtocolS::gmd_UpdateOpt()
{
    TGRPModule::gmd_UpdateOpt();

}

//=========== Control ==========================================
void TProtocolS::ctr_fill_info( XMLNode *inf )
{
    TGRPModule::ctr_fill_info( inf );
    
    XMLNode *n_add = inf->add_child();
    n_add->load_xml(i_cntr);
}

void TProtocolS::ctr_din_get_( string a_path, XMLNode *opt )
{
    TGRPModule::ctr_din_get_( a_path, opt );
    
    string t_id = ctr_path_l(a_path,0);
    if( t_id == "a_pr" )
    {
	t_id = ctr_path_l(a_path,1);
	if( t_id == "a_gn" )
	{
	    t_id = ctr_path_l(a_path,2);
    	    if( t_id == "g_help" ) ctr_opt_setS( opt, opt_descr() );       
	}   
    }
}

//================================================================
//=========== TProtocol ==========================================
//================================================================
const char *TProtocol::o_name = "TProtocol";

TProtocol::TProtocol() : m_hd(o_name)
{

}

TProtocol::~TProtocol()
{
    m_hd.lock();
    SYS->event_wait( m_hd.obj_free(), true, string(o_name)+": input protocols are closing...." );
}

unsigned TProtocol::open( string name )
{
    TProtocolIn *t_prt = in_open(name);
    try { m_hd.obj_add( t_prt, &t_prt->Name() ); }
    catch(TError err) { delete t_prt; }
    return( m_hd.hd_att( t_prt->Name() ) );
}

void TProtocol::close( unsigned hd )
{
    string name = at(hd).Name();
    m_hd.hd_det( hd );
    if( !m_hd.obj_use( name ) )
	delete (TProtocolIn *)m_hd.obj_del( name );
}

//================================================================
//=========== TProtocolIn ========================================
//================================================================
const char *TProtocolIn::o_name = "TProtocolIn";

TProtocolIn::TProtocolIn( string name, TProtocol *owner ) : m_name(name), m_wait(false), m_owner(owner)
{

}

TProtocolIn::~TProtocolIn()
{

}

