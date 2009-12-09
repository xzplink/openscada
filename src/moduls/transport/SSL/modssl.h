
//OpenSCADA system module Transport.SSL file: modssl.h
/***************************************************************************
 *   Copyright (C) 2008-2009 by Roman Savochenko                           *
 *   rom_as@oscada.org, rom_as@fromru.com                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef MODSSL_H
#define MODSSL_H

#include <pthread.h>
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#include <ttransports.h>

#undef _
#define _(mess) mod->I18N(mess)

struct CRYPTO_dynlock_value
{
    pthread_mutex_t mutex;
};


namespace MSSL
{

class TSocketIn;

//************************************************
//* Sockets::SSockIn				 *
//************************************************
class SSockIn
{
    public:
	SSockIn( TSocketIn *is, BIO *ibio ) : s(is), bio(ibio)	{ }

	TSocketIn	*s;
	BIO		*bio;
};

//************************************************
//* SSL::TSocketIn				 *
//************************************************
class TSocketIn: public TTransportIn
{
    public:
	TSocketIn( string name, const string &idb, TElem *el );
	~TSocketIn( );

	string getStatus( );

	int bufLen( )		{ return mBufLen; }
	int maxFork( )		{ return mMaxFork; }
	int keepAliveCon( )	{ return mKeepAliveCon; }
	int keepAliveTm( )	{ return mKeepAliveTm; }
	int taskPrior( )	{ return mTaskPrior; }
	string certKey( )	{ return mCertKey; }
	string pKeyPass( )	{ return mKeyPass; }
	int opConnCnt( );

	void setBufLen( int vl )		{ mBufLen = vl; modif(); }
	void setMaxFork( int vl )		{ mMaxFork = vl; modif(); }
	void setKeepAliveCon( int vl )		{ mKeepAliveCon = vl; modif(); }
	void setKeepAliveTm( int vl )		{ mKeepAliveTm = vl; modif(); }
	void setTaskPrior( int vl )		{ mTaskPrior = vmax(-1,vmin(99,vl)); modif(); }
	void setCertKey( const string &val )	{ mCertKey = val; modif(); }
	void setPKeyPass( const string &val )	{ mKeyPass = val; modif(); }

	void start( );
	void stop( );

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Methods
	static void *Task( void * );
	static void *ClTask( void * );

	int clientReg( pthread_t thrid );
	void clientUnreg( pthread_t thrid );

	void messPut( int sock, string &request, string &answer, AutoHD<TProtocolIn> &prot_in );

	void cntrCmdProc( XMLNode *opt );	//Control interface command process

	//Attributes
	Res		sock_res;
	SSL_CTX		*ctx;

	bool		endrun;			// Command for stop task
	bool		endrun_cl;		// Command for stop client tasks

	string		&mAPrms;		// Addon parameters

	int		mMaxFork,		// maximum forking (opened SSL)
			mBufLen,		// input buffer length
			mKeepAliveCon,		// KeepAlive connections
			mKeepAliveTm,		// KeepAlive timeout
			mTaskPrior;		// Requests processing task prioritet
	string		mCertKey,		// SSL certificate
			mKeyPass;		// SSL private key password

	bool		cl_free;		// Clients stoped
	vector<pthread_t>	cl_id;		// Client's pids

	//> Status atributes
	string		stErr;			// Last error messages
	float		trIn, trOut;		// Traffic in and out counter
	int		connNumb, clsConnByLim;	// Close connections by limit
};

//************************************************
//* SSL::TSocketOut				 *
//************************************************
class TSocketOut: public TTransportOut
{
    public:
	TSocketOut( string name, const string &idb, TElem *el );
	~TSocketOut( );

	string getStatus( );

	string certKey( )	{ return mCertKey; }
	string pKeyPass( )	{ return mKeyPass; }

	void start( );
	void stop( );

	void setCertKey( const string &val )	{ mCertKey = val; modif(); }
	void setPKeyPass( const string &val )	{ mKeyPass = val; modif(); }

	int messIO( const char *obuf, int len_ob, char *ibuf = NULL, int len_ib = 0, int time = 0, bool noRes = false );

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Methods
	void cntrCmdProc( XMLNode *opt );	//Control interface command process

	//Attributes
	string		&mAPrms;		// Addon parameters

	string		mCertKey,		// SSL certificate
			mKeyPass;		// SSL private key password

	SSL_CTX		*ctx;
	BIO		*conn;
	SSL		*ssl;

	//- Status atributes -
	float		trIn, trOut;		// Traffic in and out counter
	Res		wres;
};

//************************************************
//* SSL::TTransSock				 *
//************************************************
class TTransSock: public TTipTransport
{
    public:
	TTransSock( string name );
	~TTransSock( );

	TTransportIn  *In( const string &name, const string &idb );
	TTransportOut *Out( const string &name, const string &idb );

    protected:
	void load_( );

    private:
	//Methods
	string optDescr( );
	void cntrCmdProc( XMLNode *opt );       //Control interface command process

	void postEnable( int flag );

	static unsigned long id_function( );
	static void locking_function( int mode, int n, const char * file, int line );
	static struct CRYPTO_dynlock_value *dyn_create_function( const char *file, int line );
	static void dyn_lock_function( int mode, struct CRYPTO_dynlock_value *l, const char *file, int line );
	static void dyn_destroy_function( struct CRYPTO_dynlock_value *l, const char *file, int line );

	//Attributes
	pthread_mutex_t *mutex_buf;
};

extern TTransSock *mod;
}

#endif //MODSSL_H
