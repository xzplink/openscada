#ifndef TEST_ARHIV_H
#define TEST_ARHIV_H

#include "../../tmodule.h"

class TArhivTest: public TModule
{
    public:
	TArhivTest(char *name);
	~TArhivTest();

	void mod_CheckCommandLine( );
    public:

    private:
	void mod_connect(  );
	
	void pr_opt_descr( FILE * stream );
    private:
	static SExpFunc ExpFuncLc[];
};

#endif //TEST_ARHIV_H

