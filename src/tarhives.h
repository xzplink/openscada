
#ifndef TARHIVES_H
#define TARHIVES_H

#include <string>
using std::string;
#include "tgrpmodule.h"

/**
  * class TArhiveS
  * Object of arhiv, protokol, and other  tasks protocoling.
  */

class TArhiveS : public TGRPModule
{

/** Public methods: */
public:
    TArhiveS( TKernel *app );

    ~TArhiveS(  );

    void gmd_CheckCommandLine( );
    void gmd_UpdateOpt();
/** Privateds: */
private:
    void pr_opt_descr( FILE * stream );
    
/** Private atributes: */
private:
};

#endif // TARHIVES_H
