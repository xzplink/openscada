
#ifndef TGRPMODULE_H
#define TGRPMODULE_H

#define GRM_ST_OFF 0
#define GRM_ST_ON  1

#include <string>
#include <vector>

class TModule;
class TModSchedul;

struct SModul
{
    int       stat;
    string    name;
    TModule * modul;
    int	      id_hd;
//    void    * hd;
    int       resource;
    int       access;
};

//For a multi moduls declaration into once a shared lib
//No made while!
struct SHD
{
    void    * hd;
    int       use;
    time_t    modif;
    string    path;
};

class TGRPModule
{

/** Public methods: */
public:
     TGRPModule( char * NameT );

     ~TGRPModule(  );

     friend class TModSchedul;
    /**
      * Search and loading all external modul (plugin) 
      * @param NameMod
      *        Modul's name!
      */
    int LoadAll(const string & Paths);

    /**
      * Init all moduls.
      * @param ModObject
      *        A Object's adres for the modul's tip.
      */
    int InitAll( );

    string List(  );

    /**
      * Registring function.
      * @param addr
      *        Addres for save adress registring function.
      */
    int RegFunc( string NameFunc, void * addr, string SrcModName, string NameMod );

    virtual int PutCom(char * NameMod, string command ); // = 0;
    virtual int PutCom(int  idMod, string command ); // = 0;

    int AddShLib( char *name );
    virtual int AddM(TModule *modul );
    virtual int DelM( int hd );
// Convert Name moduls to id into vector!
    int name_to_id(string & name);

/**Attributes: */
public:
//    SNameUser * users;

/** Protected methods: */
protected:
//    virtual void AddM(TModule & module);

protected:
    vector<SModul *> Moduls;
    vector<SHD *> SchHD;
    char *DirPath;
//    vector<TModule *> Moduls;
/** Private methods: */
private:
    int  RegHDShLb(const void* hd, char *path, time_t modif );
    int  FreeHDshLb(int id);	
    void ScanDir( const string & Paths, string & Mods );
    bool CheckFile(char * name);
private:
    char *NameType;
};

#endif // TGRPMODULE_H
