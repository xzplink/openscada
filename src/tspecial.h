
#ifndef TSPECIAL_H
#define TSPECIAL_H

#include <string>

#include "tgrpmodule.h"

class TSpecial : public TGRPModule
{

/** Public methods: */
public:
     TSpecial(  );

    void Init(  );

    void Start(  );

    void CheckCommandLine(  );
/** Private methods: */
private:
    void pr_opt_descr( FILE * stream );

/** Private atributes: */
private:
};

#endif // TSPECIAL_H
