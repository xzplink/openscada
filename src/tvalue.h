#ifndef TVALUE_H
#define TVALUE_H

#include <string>
using std::string;
#include <vector>
using std::vector;

//====  Mode access to value ====
#define VAL_MA_DIRECT  0  //Direct access to controller

#include "terror.h"
#include "tvalueelem.h"

struct STime
{
    time_t s;
    long   us;
};

struct SCale
{
    double max;
    double min;	
};

struct SVal
{
    char       valid:1;    // stat element
    char       r_access:1; // acces for read
    char       w_access:1; // acces for write
    STime      time;       // Time
    union
    { 
	string *val_s;    //string value
	double val_r;     //real value
	int    val_i;     //integer value	
	bool   val_b;     //boolean value
    } val;
    SCale      *scale;
};

struct SBasync
{
    int           i_head;          //id of header
    vector<STime> time;            //element's time
    union
    {
        vector<string> *val_s;     //string value
	vector<double> *val_r;     //real value
	vector<int>    *val_i;     //integer value
	vector<bool>   *val_b;     //boolean value
    }val;
};

struct SBsync
{
    STime t_head;            //element's time
    STime t_per;             //element's period
    int   i_head;
    int   i_tail;    
    union
    {
        vector<string> *val_s;     //string value
	vector<double> *val_r;     //real value
	vector<int>    *val_i;     //integer value
	vector<bool>   *val_b;     //boolean value
    }val;
};

union SBUF 
{
    SBsync  *sync;     
    SBasync *async;
};

#define V_CUR -1
#define V_MAX -2
#define V_MIN -3


#define V_USER_OWN 0x01
#define V_USER_GRP 0x02
#define V_USER_OTH 0x04

#define V_ACC_READ  0x01
#define V_ACC_WRITE 0x02

class TConfig;

class TValue
{
/** Public methods: */
public:
    friend class TValueElem;
    TValue( );
    virtual ~TValue();

    void vl_SetType( TValueElem *ValEl );
    /*
     * Object of element for value
     */
    TValueElem &vl_Elem() 
    { if(elem == NULL) throw(TError("%s: Value without type",o_name)); return(*elem); }
    /*
     * Check access for user (V_USER_OWN - owner, V_USER_GRP - group, V_USER_OTH - other)
     *  and mode (V_ACC_READ - read, V_ACC_WRITE - write) 
     */
    bool vl_access(unsigned element, char user, char mode); 

    bool vl_Valid( unsigned elem );
    /*
     * Read curent value
     */
    string vl_GetSEL( unsigned id_el, STime &time, int arhiv = -1 );
    string vl_GetS( unsigned  id_el, STime &time, int arhiv = -1 );
    double vl_GetR( unsigned  id_el, STime &time, int arhiv = -1 );  // arhiv V_MAX - max, V_MIN - min
    int    vl_GetI( unsigned  id_el, STime &time, int arhiv = -1 );  // arhiv V_MAX - max, V_MIN - min
    bool   vl_GetB( unsigned  id_el, STime &time, int arhiv = -1 );
    /*
     * Set new value(s) into buffer
     */
    string vl_SetSEL( unsigned  id_el, string value, const STime &tm, int arhiv = -1 );
    string vl_SetS( unsigned  id_el, string value, const STime &tm, int arhiv = -1 );
    double vl_SetR( unsigned  id_el, double value, const STime &tm, int arhiv = -1 );
    int    vl_SetI( unsigned  id_el, int value, const STime &tm, int arhiv = -1 );
    bool   vl_SetB( unsigned  id_el, bool value, const STime &tm, int arhiv = -1 );    
/** Protected metods */
protected:
    
    void vl_Valid( unsigned element, bool value );
    /*
     * Read curent value for owner
     */
    string _vl_GetSEL( unsigned id_el, STime &time, int arhiv = -1 );
    string _vl_GetS( unsigned  id_el, STime &time, int arhiv = -1 );
    double _vl_GetR( unsigned  id_el, STime &time, int arhiv = -1 );  // arhiv V_MAX - max, V_MIN - min
    int    _vl_GetI( unsigned  id_el, STime &time, int arhiv = -1 );  // arhiv V_MAX - max, V_MIN - min
    bool   _vl_GetB( unsigned  id_el, STime &time, int arhiv = -1 );
    /*
     * Set new value(s) for owner
     */
    string _vl_SetSEL( unsigned  id_el, string value, const STime &tm, int arhiv = -1 );
    string _vl_SetS( unsigned  id_el, string value, const STime &tm, int arhiv = -1 );
    double _vl_SetR( unsigned  id_el, double value, const STime &tm, int arhiv = -1 );
    int    _vl_SetI( unsigned  id_el, int value, const STime &tm, int arhiv = -1 );
    bool   _vl_SetB( unsigned  id_el, bool value, const STime &tm, int arhiv = -1 );    
/** Private metods */
private:
    /*
     * Set value direct into controller param's
     */
    virtual void vl_Set( int id_elem )
    { throw TError("%s: Direct access to write value no avoid",o_name); }
    /*
     * Get value direct from controller param's
     */    
    virtual void vl_Get( int id_elem )
    { throw TError("%s: Direct access to read value no avoid",o_name); }
    /*
     * Get Cfg for BD
     */    
    virtual TConfig *vl_GetCfg( )
    { throw TError("%s: Access to Cfg no avoid",o_name); }

    void vl_CheckId( unsigned id_el ) const
    { if( id_el >= elem->vle_Size() ) throw TError("%s: element id error!",o_name); }
    void vl_CheckValid( unsigned id_el ) const
    { if( !val[id_el].valid ) throw TError("%s: element no valid!",o_name); }
    /*
     * Add elem into TValueElem
     */
    void vl_AddElem(unsigned id_val); 
    void vl_InitElem(unsigned id_val);
    /*
     * Del elem without TValueElem
     */
    void vl_DelElem(int id_val); 
    void vl_FreeElem(int id_val);

/** Private atributes: */
private:
    vector<SVal>           val;  // curent value
    vector<SBUF>           buf;  // buffer values
    
    TValueElem             *elem;  // elements  
    static const char      *o_name;
};

#endif // TVALUE_H

