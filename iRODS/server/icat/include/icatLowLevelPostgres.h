/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*
  header file for the Postgresql version of the icat low level routines.
 */

#ifndef CLL_PSQ_H
#define CLL_PSQ_H

#ifdef NEW_ODBC
#include "sql.h"
#include "sqlext.h"
#else
/*
 These two are in the postgresql include directory, for example
 ../../../pgsql/include :
*/
#include "iodbc.h"
#include "isqlext.h"
/* Seems to be missing: */
#define SQL_SQLSTATE_SIZE     6
#endif

#include "rods.h"
#include "icatMidLevelRoutines.h"

#define MAX_BIND_VARS 20

extern int cllBindVarCount;
extern char *cllBindVars[20];


/* The name in the various 'odbc.ini' files for the catalog: */
#define CATALOG_ODBC_ENTRY_NAME "PostgreSQL"

/* The name in the various 'odbc.ini' files for the RDA: */
#define RDA_ODBC_ENTRY_NAME "iRODS_RDA"

int cllOpenEnv(icatSessionStruct *icss);
int cllCloseEnv(icatSessionStruct *icss);
int cllConnect(icatSessionStruct *icss);
int cllConnectRda(icatSessionStruct *icss);
int cllDisconnect(icatSessionStruct *icss);
int cllExecSqlNoResult(icatSessionStruct *icss, char *sql);
int cllExecSqlWithResult(icatSessionStruct *icss, int *stmtNum, char *sql);
int cllExecSqlWithResultBV(icatSessionStruct *icss, int *stmtNum, char *sql,
			     char *bindVar1, char *bindVar2, char *bindVar3,
			     char *bindVar4, char *bindVar5, char *bindVar6);
int cllGetRow(icatSessionStruct *icss, int statementNumber);
int cllFreeStatement(icatSessionStruct *icss, int statementNumber);
int cllNextValueString(char *itemName, char *outString, int maxSize);
int cllTest();
int cllCurrentValueString(char *itemName, char *outString, int maxSize);
int cllGetRowCount(icatSessionStruct *icss, int statementNumber);
int cllCheckPending(char *sql, int option);

#endif	/* CLL_PSQ_H */
