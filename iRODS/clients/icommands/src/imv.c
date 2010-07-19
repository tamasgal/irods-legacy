/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * imv - The irods mv utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "mvUtil.h"
void usage (char *program);

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    rodsPathInp_t rodsPathInp;
    

    optStr = "hvV";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);
    if (status) {
       printf("Use -h for help.\n");
       exit(1);
    }
    if (myRodsArgs.help==True) {
       usage(argv[0]);
       exit(0);
    }

    if (argc - optind <= 1) {
        rodsLog (LOG_ERROR, "imv: no input");
	printf("Use -h for help.\n");
        exit (2);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      UNKNOWN_OBJ_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
	printf("Use -h for help.\n");
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 1, &errMsg);

    if (conn == NULL) {
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
       rcDisconnect(conn);
        exit (7);
    }

    status = mvUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

    rcDisconnect(conn);

    if (status < 0) {
	exit (3);
    } else {
        exit(0);
    }

}

void 
usage (char *program)
{
   int i;
   char *msgs[]={
"imv moves/renames an irods data-object (file) or collection (directory) to",
"another, data-object or collection. The move works if both the source and",
"target are normal (registered in the iCAT) iRODS objects. It also works when",
"the source object is in a mounted collection (object not registered in the",
"iCAT) and the target is a normal object. In fact, this may provide a way",
"to design a drop box where data can be uploaded quickly into a mounted",
"collection and then in the background, moved to the eventual target",
"collection (where data are registered in the iCAT). But currently, the", 
"move from a normal collection to a mounted collection is not supported.",
"Options are:",
"-v verbose - display various messages while processing",
"-V very verbose",
"-h help - this help",
""};
    printf ("Usage : %s [-hvV] srcDataObj|srcColl ...  destDataObj|destColl\n", program);
    for (i=0;;i++) {
       if (strlen(msgs[i])==0) break;
       printf("%s\n",msgs[i]);
    }
    printReleaseInfo("imv");
}
