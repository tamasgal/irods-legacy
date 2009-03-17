/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* clientLogin.c - client login
 *
 * Perform a series of calls to complete a client login; to
 * authenticate.
 */

#include "rodsClient.h"

int printError(rcComm_t *Conn, int status, char *routineName) {
   char *mySubName;
   char *myName;
   rError_t *Err;
   rErrMsg_t *ErrMsg;
   int i, len;
   if (Conn) {
      if (Conn->rError) {
	 Err = Conn->rError;
	 len = Err->len;
	 for (i=0;i<len;i++) {
	    ErrMsg = Err->errMsg[i];
	    fprintf(stderr, "Level %d: %s\n",i, ErrMsg->msg);
	 }
      }
   }
   myName = rodsErrorName(status, &mySubName);
   fprintf(stderr, "%s failed with error %d %s %s\n", routineName,
	    status, myName, mySubName);

   return (0);
}

#ifdef GSI_AUTH
int clientLoginGsi(rcComm_t *Conn) 
{
   int status;
   gsiAuthRequestOut_t *gsiAuthReqOut;
   char *myName;
   char *serverDN;

   status = igsiSetupCreds(Conn, NULL, NULL, &myName);

   if (status) {
      printError(Conn, status, "igsiSetupCreds");
      return(status);
   }

   /*   printf("Client-side DN is:%s\n",myName); */

   status = rcGsiAuthRequest(Conn, &gsiAuthReqOut);
   if (status) {
      printError(Conn, status, "rcGsiAuthRequest");
      return(status);
   }

   /*   printf("Server-side DN is:%s\n", gsiAuthReqOut->serverDN); */

   serverDN = getenv("irodsServerDn"); /* Use irodsServerDn if defined */
   if (serverDN == NULL) {
      serverDN = getenv("SERVER_DN");  /* NULL or the SERVER_DN string */
   }

   status = igsiEstablishContextClientside(Conn, serverDN,
			       0);
   if (status) {
      printError(Conn, status, "igsiEstablishContextClientside");
      return(status);
   }

   /* Now, check if it actually succeeded */
   status = rcGsiAuthRequest(Conn, &gsiAuthReqOut);
   if (status) {
      printf("Error from iRODS Server:\n");
      printError(Conn, status, "GSI Authentication");
      return(status);
   }

   Conn->loggedIn = 1;

   return(0);
}
#endif

#ifdef KRB_AUTH
int clientLoginKrb(rcComm_t *Conn) 
{
   int status;
   krbAuthRequestOut_t *krbAuthReqOut;
   char *myName;
   char *serverDN;

   status = ikrbSetupCreds(Conn, NULL, NULL, &myName);
   if (status) {
      printError(Conn, status, "ikrbSetupCreds");
      return(status);
   }

   printf("Client-side DN is:%s\n",myName);

   status = rcKrbAuthRequest(Conn, &krbAuthReqOut);
   if (status) {
      printError(Conn, status, "rcKrbAuthRequest");
      return(status);
   }

   printf("Server-side DN is:%s\n", krbAuthReqOut->serverDN);

#if 0
   serverDN = getenv("irodsServerDn"); /* Use irodsServerDn if defined */
   if (serverDN == NULL) {
      serverDN = getenv("SERVER_DN");  /* NULL or the SERVER_DN string */
   }
#endif
   serverDN=  krbAuthReqOut->serverDN; /* // ? */
   status = ikrbEstablishContextClientside(Conn, serverDN, 0);
   /* //   status = ikrbEstablishContextClientside(Conn, 0, 0); */
   if (status) {
      printError(Conn, status, "ikrbEstablishContextClientside");
      return(status);
   }

   /* Now, check if it actually succeeded */
   status = rcKrbAuthRequest(Conn, &krbAuthReqOut);
   if (status) {
      printf("Error from iRODS Server:\n");
      printError(Conn, status, "KRB Authentication");
      return(status);
   }

   Conn->loggedIn = 1;

   return(0);
}
#endif

int 
clientLogin(rcComm_t *Conn) 
{   
   int status, len, i;
   authRequestOut_t *authReqOut;
   authResponseInp_t authRespIn;
   char md5Buf[CHALLENGE_LEN+MAX_PASSWORD_LEN+2];
   char digest[RESPONSE_LEN+2];
   char userNameAndZone[NAME_LEN*2];
   struct stat statbuf;
   MD5_CTX context;

   if (Conn->loggedIn == 1) {
      /* already logged in */
      return (0);
   }

#ifdef GSI_AUTH
   if (ProcessType==CLIENT_PT) {
      char *getVar;
      getVar = getenv("irodsAuthScheme");
      if (getVar != NULL && strncmp("GSI",getVar,3)==0) {
	 status = clientLoginGsi(Conn);
	 return(status);
      }
   }
#endif

#ifdef KRB_AUTH
   if (ProcessType==CLIENT_PT) {
      char *getVar;
      getVar = getenv("irodsAuthScheme");
      if (getVar != NULL) {
	 if (strncmp("Kerberos",getVar,8)==0 ||
	     strncmp("kerberos",getVar,8)==0 ||
	     strncmp("KRB",getVar,3)==0) {
	    status = clientLoginKrb(Conn);
	    return(status);
	 }
      }
   }
#endif

   status = rcAuthRequest(Conn, &authReqOut);
   if (status) {
      printError(Conn, status, "rcAuthRequest");
      return(status);
   }

   memset(md5Buf, 0, sizeof(md5Buf));
   strncpy(md5Buf, authReqOut->challenge, CHALLENGE_LEN);
   if (strncmp(ANONYMOUS_USER, Conn->proxyUser.userName, NAME_LEN) == 0) {
      md5Buf[CHALLENGE_LEN+1]='\0';
      i = 0;
   }
   else {
      i = obfGetPw(md5Buf+CHALLENGE_LEN);
   }

   if (i != 0) {
	   int doStty=0;

#ifdef windows_platform
	  if (ProcessType != CLIENT_PT)
		return i;
#endif
      
      if (stat ("/bin/stty", &statbuf) == 0) {
	 system("/bin/stty -echo");
	 doStty=1;
      }
      printf("Enter your current iRODS password:");
      fgets(md5Buf+CHALLENGE_LEN, MAX_PASSWORD_LEN, stdin);
      if (doStty) {
	 system("/bin/stty echo");
	 printf("\n");
      }
      len = strlen(md5Buf);
      md5Buf[len-1]='\0'; /* remove trailing \n */
   }
   MD5Init (&context);
   MD5Update (&context, (unsigned char*)md5Buf, CHALLENGE_LEN+MAX_PASSWORD_LEN);
   MD5Final ((unsigned char*)digest, &context);
   for (i=0;i<RESPONSE_LEN;i++) {
      if (digest[i]=='\0') digest[i]++;  /* make sure 'string' doesn't
					    end early*/
   }

   /* free the array and structure allocated by the rcAuthRequest */
   if (authReqOut != NULL) {
      if (authReqOut->challenge != NULL) {
         free(authReqOut->challenge);
      }
      free(authReqOut);
   }

   authRespIn.response=digest;
   /* the authentication is always for the proxyUser. */
   authRespIn.username = Conn->proxyUser.userName;
   strncpy(userNameAndZone, Conn->proxyUser.userName, NAME_LEN);
   strncat(userNameAndZone, "#", NAME_LEN);
   strncat(userNameAndZone, Conn->proxyUser.rodsZone, NAME_LEN*2);
   authRespIn.username = userNameAndZone;
   status = rcAuthResponse(Conn, &authRespIn);

   if (status) {
      printError(Conn, status, "rcAuthResponse");
      return(status);
   }
   Conn->loggedIn = 1;

   return(0);
}

int 
clientLoginWithPassword(rcComm_t *Conn, char* password) 
{   
   int status, len, i;
   authRequestOut_t *authReqOut;
   authResponseInp_t authRespIn;
   char md5Buf[CHALLENGE_LEN+MAX_PASSWORD_LEN+2];
   char digest[RESPONSE_LEN+2];
   char userNameAndZone[NAME_LEN*2];
   MD5_CTX context;

   if (Conn->loggedIn == 1) {
      /* already logged in */
      return (0);
   }
   status = rcAuthRequest(Conn, &authReqOut);
   if (status) {
      printError(Conn, status, "rcAuthRequest");
      return(status);
   }

   memset(md5Buf, 0, sizeof(md5Buf));
   strncpy(md5Buf, authReqOut->challenge, CHALLENGE_LEN);

   len = strlen(password);
   sprintf(md5Buf+CHALLENGE_LEN, "%s", password);
   md5Buf[CHALLENGE_LEN+len]='\0'; /* remove trailing \n */

   MD5Init (&context);
   MD5Update (&context, (unsigned char*)md5Buf, CHALLENGE_LEN+MAX_PASSWORD_LEN);
   MD5Final ((unsigned char*)digest, &context);
   for (i=0;i<RESPONSE_LEN;i++) {
      if (digest[i]=='\0') digest[i]++;  /* make sure 'string' doesn't
					    end early*/
   }

   /* free the array and structure allocated by the rcAuthRequest */
   if (authReqOut != NULL) {
      if (authReqOut->challenge != NULL) {
         free(authReqOut->challenge);
      }
      free(authReqOut);
   }

   authRespIn.response=digest;
   /* the authentication is always for the proxyUser. */
   authRespIn.username = Conn->proxyUser.userName;
   strncpy(userNameAndZone, Conn->proxyUser.userName, NAME_LEN);
   strncat(userNameAndZone, "#", NAME_LEN);
   strncat(userNameAndZone, Conn->proxyUser.rodsZone, NAME_LEN*2);
   authRespIn.username = userNameAndZone;
   status = rcAuthResponse(Conn, &authRespIn);

   if (status) {
      printError(Conn, status, "rcAuthResponse");
      return(status);
   }
   Conn->loggedIn = 1;

   return(0);
}
