
#include <getopt.h>

#include "tsys.h"
#include "tkernel.h"
#include "tmessage.h"
#include "tarhives.h"

 TArhiveS::TArhiveS( TKernel *app ) : TGRPModule(app,"Arhiv") 
{

}


 TArhiveS::~TArhiveS(  )
{

}

void TArhiveS::pr_opt_descr( FILE * stream )
{
    fprintf(stream,
    "========================= Arhive options ==================================\n"
    "    --ArhModPath=<path>  Set moduls <path>;\n"
    "\n");
}

void TArhiveS::gmd_CheckCommandLine( )
{
    int next_opt;
    char *short_opt="h";
    struct option long_opt[] =
    {
	{"ArhModPath" ,1,NULL,'m'},
	{NULL        ,0,NULL,0  }
    };

    optind=0,opterr=0;	
    do
    {
	next_opt=getopt_long(SYS->argc,(char * const *)SYS->argv,short_opt,long_opt,NULL);
	switch(next_opt)
	{
	    case 'h': pr_opt_descr(stdout); break;
	    case 'm': DirPath = optarg;     break;
	    case -1 : break;
	}
    } while(next_opt != -1);
}

void TArhiveS::gmd_UpdateOpt()
{


}

