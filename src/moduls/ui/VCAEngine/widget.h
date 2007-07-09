
//OpenSCADA system module UI.VCAEngine file: widget.h
/***************************************************************************
 *   Copyright (C) 2006 by Roman Savochenko
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

#ifndef WIDGET_H
#define WIDGET_H

#include <tcntrnode.h>
#include <tconfig.h>

namespace VCA
{

//************************************************
//* Widget attribute                             *
//************************************************
class Widget;

class Attr : public TCntrNode
{
    public:
        //Data
	//- Attribute flags -
        enum GlobalAttrFlgs
        {
	    Active  = 0x0100,	//Active attribute for primitives process
            Image   = 0x0200,	//Store image link to DB or to file
            Color   = 0x0400,	//Store color
            Font    = 0x0800,	//Store font
	    Address = 0x1000,	//Store address to a parameter or an attribute
	    IsUser  = 0x2000,	//User created element
            IsInher = 0x4000,	//Inherit attribute
	    Mutable = 0x8000	//Mutable attribute, present depend from "Active" attribute value
        };

	//- Link types -
	enum SelfAttrFlgs
	{
	    CfgConst   	= 0x01,	//Constant
	    CfgLnkIn   	= 0x02,	//Input link
	    CfgLnkOut  	= 0x04,	//Output link
	    ProcAttr	= 0x08	//Process attribute
	};

	//Methods
	//- Main -
	Attr( TFld &fld );
	~Attr( );

	string id( );
	string name( );
        TFld::Type type( );
	int flgGlob( );		//Global attribite's flags
	SelfAttrFlgs flgSelf( )	{ return self_flg; }
	unsigned modifVal( )	{ return vl_modif; }
	unsigned modifCfg( )	{ return cfg_modif; }
	string cfgTempl( )	{ return cfg_tmpl; }
	string cfgVal( )	{ return cfg_val; }
	
	void setFlgSelf( SelfAttrFlgs flg );
	void setModifVal( unsigned set )	{ vl_modif = set; }
        void setModifCfg( unsigned set )	{ cfg_modif = set; }
	void setCfgTempl( const string &vl );
	void setCfgVal( const string &vl );

	//- Get value -
	string getSEL( );
	string getS( );
	double getR( );
	int    getI( );
	bool   getB( );

	//- Set value -
	void setSEL( const string &val, unsigned mod_vl = 0, bool strongPrev = false );
	void setS( const string &val, unsigned mod_vl = 0, bool strongPrev = false );
	void setR( double val, unsigned mod_vl = 0, bool strongPrev = false );
	void setI( int val, unsigned mod_vl = 0, bool strongPrev = false );
	void setB( bool val, unsigned mod_vl = 0, bool strongPrev = false );

	TFld &fld()  			{ return *m_fld; }
	
	Widget *owner();

    protected:
	string nodeName()	{ return id(); }

    private:
        //Data
	//- Storing -
        union
        {
            string *s_val;		//String
            double r_val;		//Real
            int    i_val;		//Integer
            bool   b_val;		//Boolean
        }m_val;
        //- Attributes -
        TFld	*m_fld;			//Base field
	unsigned vl_modif,		//Value modify counter
		 cfg_modif;		//Configuration modify counter
	SelfAttrFlgs self_flg;		//Self attributes flags
	
	string 	cfg_tmpl, cfg_val;	//Config template and value
};

//************************************************
//* Widget                                       *
//************************************************
class Widget : public TCntrNode, public TValElem
{
    friend class Attr;

    public:
        //Methods
	Widget( const string &id, const string &isrcwdg = "" );
	~Widget();

	string id( )               	{ return m_id; }	//Identifier
	virtual string path( )     	{ return m_id; }	//Curent widget path
	virtual string name( );          			//Name
	virtual string descr( );          			//Description
	virtual string ico( )      	{ return ""; }		//Icon
	virtual string type( )		{ return "Generic"; }	//Widget hierarchy type
	virtual string user( )     	{ return "root"; }	//Widget user
        virtual string grp( )      	{ return "UI"; }	//Widget group
	virtual short  permit( )   	{ return 0644; }	//Permition for access to widget
	virtual string calcId( )   	{ return m_id; }	//Compile function identifier
	virtual string calcLang( ) 	{ return ""; }		//Calc procedure language
	virtual string calcProg( ) 	{ return ""; }		//Calc procedure
	virtual bool isContainer( );				//Is container (Is define of the terminator)
        virtual bool isLink( )     	{ return m_lnk; }	//Widget as link

        virtual void setName( const string &inm );
	virtual void setDescr( const string &idscr );
	virtual void setIco( const string &ico )      { };
	virtual void setUser( const string &iuser )   { };
	virtual void setGrp( const string &igrp )     { };
	virtual void setPermit( short iperm )         { };
	virtual void setCalcLang( const string &ilng ){ };
        virtual void setCalcProg( const string &iprg ){ };

	//- Storing -
        virtual void load( )    { }     //Load widget
	virtual void loadIO( )  { }     //Load widget's IO
        virtual void save( )    { }     //Save widget
	virtual void saveIO( )  { }     //Save widget's IO

	//- Enable stat -
        bool enable( );
        virtual void setEnable( bool val );

        //- Inheritance methods -
	virtual string parentNm( )	{ return m_parent_nm; }	//Parent widget name
	virtual string rootId( );                       	//Root widget id
	AutoHD<Widget> parent( );				//Parent widget
	AutoHD<Widget> parentNoLink( );				//Parent no link widget
	void heritReg( Widget *wdg );				//Register heritator
	void heritUnreg( Widget *wdg );				//Unregister heritator
	virtual void setParentNm( const string &isw )	{ m_parent_nm = isw; }
        void inheritAttr( const string &attr = "" );		//Inherit parent attributes
        void inheritIncl( const string &wdg = "" );		//Inherit parent include widgets

        //- Widget's attributes -
	void attrList( vector<string> &list )		{ chldList(attrId,list); }
	void attrAdd( TFld *attr );
	void attrDel( const string &attr );
	bool attrPresent( const string &attr )		{ return chldPresent(attrId,attr); }
	AutoHD<Attr> attrAt( const string &attr )	{ return chldAt(attrId,attr); }

        //- Include widgets -
        void wdgList( vector<string> &list );
        bool wdgPresent( const string &wdg );
	virtual void wdgAdd( const string &wid, const string &name, const string &path );
        void wdgDel( const string &wid, bool full = false );
        AutoHD<Widget> wdgAt( const string &wdg );
	
	//- Data access -
	virtual string resourceGet( const string &id, string *mime = NULL )	{ return ""; }

    protected:
	//Methods
	string nodeName()	{ return m_id; }

	void postEnable( int flag );
	void preDisable( int flag );
	
	bool cntrCmdGeneric( XMLNode *opt );
	bool cntrCmdAttributes( XMLNode *opt );
	bool cntrCmdLinks( XMLNode *opt );
	bool cntrCmdProcess( XMLNode *opt );	

        virtual bool attrChange( Attr &cfg );   //Process attribute change local and into terminator

        void addFld( TElem *el, unsigned id );
        void delFld( TElem *el, unsigned id );
        void detElem( TElem *el );

	//Attributes	
	//- Generic data -
	string	m_id;                   //Widget identifier

	bool	m_enable;               //Enable status
        bool    m_lnk;                  //Widget as link
	int 	attrId, inclWdg;	//The widget's container id
        string  m_parent_nm;            //Parent widget name
	AutoHD<Widget>	m_parent;	//Parent widget
	vector< AutoHD<Widget> > m_herit;	//Heritators
	
	//- Attributes data -
	TElem	attr_cfg;		//Attributes structure element
};

}

#endif //WIDGET_H