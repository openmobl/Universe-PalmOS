/********************************************************************************
 * Universe Web Browser                                                         *
 * Copyright (c) 2007 OpenMobl Systems                                          *
 * Copyright (c) 2006-2007 Donald C. Kirker                                     *
 * Portions Copyright (c) 1999-2007 Filip Onkelinx                              *
 *                                                                              *
 * http://www.openmobl.com/                                                     *
 * dev-support@openmobl.com                                                     *
 *                                                                              *
 * This program is free software; you can redistribute it and/or                *
 * modify it under the terms of the GNU General Public License                  *
 * as published by the Free Software Foundation; either version 2               *
 * of the License, or (at your option) any later version.                       *
 *                                                                              *
 * This program is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
 * GNU General Public License for more details.                                 *
 *                                                                              *
 * You should have received a copy of the GNU General Public License            *
 * along with this program; if not, write to the Free Software                  *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 	*
 *                                                                              *
 ********************************************************************************/
#include 	<PalmOS.h>
#include	<VFSMgr.h>
#include	<NetMgr.h>
#include	<SslLib.h>
#include	"debug.h"
#include 	"BrowserAPI.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"Layout.h"
#include	"Form.h"
#include	"variable.h"
#include	"Browser.h"
#include	"libhtmlparse.h"
#include	"Cache.h"
#include	"Net.h"


typedef struct {
	UInt16	number;
	Char 	*error;
} NetError;

/* TODO: Complete me */
static NetError errorList[] =
{
	{ netErrAlreadyOpen, 		"Network (library) already open"	},
	{ netErrNotOpen, 		"Network (library) not open"		},
	{ netErrStillOpen, 		"Network (library) still open"		},
	{ netErrParamErr, 		"Parameter error"			},
	{ netErrNoMoreSockets, 		"No more available sockets"		},
	{ netErrOutOfResources, 	"Out of resources"			},
	{ netErrOutOfMemory, 		"Out of memory"				},
	{ netErrSocketNotOpen, 		"Socket not open"			},
	{ netErrSocketBusy, 		"Socket busy"				},
	{ netErrMessageTooBig, 		"Message too big"			},
	{ netErrSocketNotConnected, 	"Socket not connected"			},
	{ netErrNoInterfaces, 		"No available interfaces"		},
	{ netErrBufTooSmall, 		"Buffer too small"			},
	{ netErrUnimplemented, 		"Unimplemented"				},
	{ netErrPortInUse, 		"Specified port in use"			},
	{ netErrQuietTimeNotElapsed, 	"Quite time not elapsed"		},
	{ netErrInternal, 		"Internal error"			},
	{ netErrTimeout, 		"Transaction timed out"			},
	{ netErrSocketAlreadyConnected,	"Socket already connected"		},
	{ netErrSocketClosedByRemote, 	"Socket closed by remote host"		},
	{ netErrOutOfCmdBlocks, 	"Out of command blocks"			},
	{ netErrWrongSocketType, 	"Wrong socket type"			},
	{ netErrSocketNotListening, 	"Socket not listening"			},
	{ netErrUnknownSetting, 	"Unknown setting"			},
	{ netErrInvalidSettingSize, 	"Invalid setting size"			},
	{ netErrPrefNotFound, 		"Preference not found"			},
	{ netErrInvalidInterface, 	"Invalid interface"			},
	{ netErrInterfaceNotFound, 	"Interface not found"			},
	{ netErrTooManyInterfaces, 	"Too many interfaces"			},
	{ netErrBufWrongSize, 		"Wrong buffer size"			},
	{ netErrUserCancel, 		"User canceled"				},
	{ netErrBadScript, 		NULL },
	{ netErrNoSocket, 		NULL },
	{ netErrSocketRcvBufFull, 	NULL },
	{ netErrNoPendingConnect, 	NULL },
	{ netErrUnexpectedCmd, 		NULL },
	{ netErrNoTCB, 			NULL },
	{ netErrNilRemoteWindowSize, 	NULL },
	{ netErrNoTimerProc, 		NULL },
	{ netErrSocketInputShutdown, 	NULL },
	{ netErrCmdBlockNotCheckedOut, 	NULL },
	{ netErrCmdNotDone, 		NULL },
	{ netErrUnknownProtocol, 	NULL },
	{ netErrUnknownService, 	NULL },
	{ netErrUnreachableDest, 	NULL },
	{ netErrReadOnlySetting, 	NULL },
	{ netErrWouldBlock, 		NULL },
	{ netErrAlreadyInProgress, 	NULL },
	{ netErrPPPTimeout, 		NULL },
	{ netErrPPPBroughtDown, 	NULL },
	{ netErrAuthFailure, 		NULL },
	{ netErrPPPAddressRefused, 	NULL },
	{ netErrDNSNameTooLong, 	"Host name too long"			},
	{ netErrDNSBadName, 		"Bad host name"				},
	{ netErrDNSBadArgs, 		"Bad args (DNS)"			},
	{ netErrDNSLabelTooLong, 	NULL },
	{ netErrDNSAllocationFailure, 	NULL },
	{ netErrDNSTimeout, 		"Host name lookup timeout"		},
	{ netErrDNSUnreachable, 	"DNS server unreachable"		},
	{ netErrDNSFormat, 		NULL },
	{ netErrDNSServerFailure, 	NULL },
	{ netErrDNSNonexistantName, 	"Host name does not exist"		},
	{ netErrDNSNIY, 		NULL },
	{ netErrDNSRefused, 		NULL },
	{ netErrDNSImpossible, 		NULL },
	{ netErrDNSNoRRS, 		NULL },
	{ netErrDNSAborted, 		"Host name lookup aborted"		},
	{ netErrDNSBadProtocol, 	NULL },
	{ netErrDNSTruncated, 		NULL },
	{ netErrDNSNoRecursion, 	NULL },
	{ netErrDNSIrrelevant, 		NULL },
	{ netErrDNSNotInLocalCache, 	NULL },
	{ netErrDNSNoPort, 		NULL },
	{ netErrIPCantFragment, 	NULL },
	{ netErrIPNoRoute, 		NULL },
	{ netErrIPNoSrc, 		NULL },
	{ netErrIPNoDst, 		NULL },
	{ netErrIPktOverflow, 		NULL },
	{ netErrTooManyTCPConnections, 	"Too many connections"			},
	{ netErrNoDNSServers, 		"No configured DNS servers"		},
	{ netErrInterfaceDown, 		"Interface down"			},
	{ netErrDeviceNotReady, 	NULL },
	{ sslErrOk,			NULL },
	{ sslErrFailed,			NULL },
	{ sslErrEof,			NULL },
	{ sslErrOutOfMemory,		NULL },
	{ sslErrCbAbort,		NULL },
	{ sslErrIo,			NULL },
	{ sslErrNotFound,		NULL },
	{ sslErrDivByZero,		NULL },
	{ sslErrNoModInverse,		NULL },
	{ sslErrBadDecode,		NULL },
	{ sslErrInitNotCalled,		NULL },
	{ sslErrBufferTooSmall,		NULL },
	{ sslErrNullArg,		NULL },
	{ sslErrBadLength,		NULL },
	{ sslErrNoDmem,			NULL },
	{ sslErrNoMethodSet,		NULL },
	{ sslErrNoRandom,		NULL },
	{ sslErrBadArgument,		NULL },
	{ sslErrBadOption,		NULL },
	{ sslErrNotImplemented,		NULL },
	{ sslErrReallocStaticData,	NULL },
	{ sslErrInternalError,		NULL },
	{ sslErrRecordError,		NULL },
	{ sslErrCsp,			NULL },
	{ sslErrCert,			NULL },
	{ sslErrHandshakeEncoding,	NULL },
	{ sslErrMissingProvider,	NULL },
	{ sslErrHandshakeProtocol,	NULL },
	{ sslErrExtraHandshakeData,	NULL },
	{ sslErrWrongMessage,		NULL },
	{ sslErrFatalAlert,		NULL },
	{ sslErrBadPeerFinished,	NULL },
	{ sslErrBadSignature,		NULL },
	{ sslErrUnexpectedRecord,	NULL },
	{ sslErrReadAppData,		NULL },
	{ sslErrCertDecodeError,	NULL },
	{ sslErrUnsupportedCertType,	NULL },
	{ sslErrUnsupportedSignatureType,		NULL },
	{ sslErrVerifyBadSignature,	NULL },
	{ sslErrVerifyNoTrustedRoot,	NULL },
	{ sslErrVerifyNotAfter,		NULL },
	{ sslErrVerifyNotBefore,	NULL },
	{ sslErrVerifyConstraintViolation,		NULL },
	{ sslErrVerifyUnknownCriticalExtension,		NULL },
	{ sslErrVerifyCallback,		NULL },
	{ 0,				NULL 					}
};

Char *NetLookupError(Int16 error)
{
	int i = 0;

	while (errorList[i].number && (errorList[i].number != error)) {
		i++;
	}

	return errorList[i].error;
}

void NetDisplayError(Char *msg, Int16 err, Char *file, Int16 line)
{
	Char		*details;
	Int16		tapped;
	Char		*netErr = NULL;
	GlobalsType	*g = NULL;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	details = Malloc(1024);
	if (!details) {
		MyErrorFunc(msg, NULL);
		return;
	}
	
	tapped = FrmCustomAlert(alNetError, msg, "", NULL);
	switch (tapped) {
		case 1:
			netErr = NetLookupError(err);
			StrPrintF(details, "%s:%d\n%s [%d]\n\nURL: %s", file, line, (netErr) ? netErr:" ", err, g->Url);
			FrmCustomAlert(alNetErrorDetails, details, "", NULL);
			break;
		default:
			break;
	}

	Free(details);
}

Err NetOpenNetwork(GlobalsType *g)
{
    	Err	error;
	
	if ((StrNCaselessCompare("mtch:", g->Url, 5) != 0) &&
		(CacheFindCachedFile(g->Url, g->isImage, g) == true)) // not the best method??
			return(0);

	if (SysLibFind ("Net.lib", &AppNetRefnum) == 0) {
        	error = NetLibOpen (AppNetRefnum, &errno);
        	if (errno || (error && (error != netErrAlreadyOpen))) {
			if ((errno != netErrUserCancel) && (error != netErrUserCancel))
				NetDisplayError("Could not open the network connection.", (errno)?errno:error, __FILE__, __LINE__);

			if (!error && errno)
				error = errno;
        		NetLibClose(AppNetRefnum, true);
			g->NetLibOpen = false;
			g->cache.cacheDoNotRead = false;
			return error;
        	} else if (!error || (error == netErrAlreadyOpen)) {
            		g->NetLibOpen = true;

			if ((error = NetLibRefresh())) {
				g->NetLibOpen = false;
				return(error);
			}

            		return(0);
        	}
        	return(error);
    	} else {
        	MyErrorFunc("Could not open the Network library. Maybe it is missing or corrupt? Please try a soft reset.", NULL);   
        	return(-1);
    	}
}
// The next two fnctions are slightly modified from the GiantDisc project
Err NetLibRefresh(void)
{
	UInt16 		netIFErr;
	Boolean 	allUp;
	Err 		err;
	GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

      	err = NetLibConnectionRefresh(AppNetRefnum, true, &allUp, &netIFErr);
      	if (!err && !netIFErr && allUp){

		g->NetLibOpen = true;

        	return errNone;
      	}

	g->NetLibOpen = false;

	return err;
}

// NetLibIsConnected
Boolean NetLibConnected(void)
{
	UInt16 		openCount, netIFErr;
	Boolean 	allUp;
	Err 		err;

	if (NetLibOpenCount(AppNetRefnum, &openCount) == 0){
    		if (openCount) {
      			err = NetLibConnectionRefresh(AppNetRefnum, false, &allUp, &netIFErr);
      			if (!err && !netIFErr && allUp){
        			return true;
      			}
    		}
  	}

  	NetLibClose(AppNetRefnum, true);
  	return false;
}

void NetCloseSocket(NetSocketRef *sock)
{
	Err	errP = 0;

	if (*sock != -1)
		NetLibSocketClose(AppNetRefnum, *sock, 2 * SysTicksPerSecond(), &errP);
	*sock = -1;
}

NetSocketRef NetUTCP2Open(const Char * host, const Char * service, Int16 port)
{
	NetSocketRef		socket = 0;
	GlobalsType 		*g;
	NetHostInfoBufPtr	AppHostInfo = NULL;
	NetHostInfoPtr		HostInfo = NULL;
	//struct sockaddr_in 	sockAddrP;
	UInt32 			AppNetTimeout = 1 * SysTicksPerSecond();
	UInt32			AppSysTimeout = g_prefs.timeout * SysTicksPerSecond();
	UInt32			iTime = 0;
	UInt32			time = 0;
	UInt32			connectTime = 0;
	Err 			errP, saveErr;
	Int16			sockConnect = -1;
	NetIPAddr 		address;
	NetIPAddr 		*addressPtr = NULL;
	NetSocketAddrINType	sockAddrP;
	NetSocketLingerType	sockLinger;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	showProgress(g);

	DebugOutComms("NetUTCP2Open", (Char *)host, port, __FILE__, __LINE__, g);

	address = NetLibAddrAToIN(AppNetRefnum, host);
	if (address != -1) goto connect;

	DebugOutComms("NetUTCP2Open", "got address", 0, __FILE__, __LINE__, g);

	AppHostInfo = (NetHostInfoBufPtr) Malloc(sizeof(NetHostInfoBufType));
	if (!AppHostInfo) {
		if (g->isImage != true)
			NetDisplayError("Not enough dynamic heap for network communications.", netErrOutOfMemory, __FILE__, __LINE__);
		return -1;
	}

	DebugOutComms("NetUTCP2Open", "get host by name", 0, __FILE__, __LINE__, g);

	iTime = TimGetTicks();
	do {
		HostInfo = NetLibGetHostByName(AppNetRefnum, host, AppHostInfo, AppNetTimeout, &errP);
		time = TimGetTicks();
	} while (!HostInfo && (errP == netErrTimeout) && (time < (iTime + AppSysTimeout)));

	// for some reason some NetLib calls don't set errP to errNone
	if (!HostInfo /*&& (errP != errNone)*/) {
		if (!g->isImage)
			NetDisplayError("Host lookup failed. Try again later.", errP, __FILE__, __LINE__);
		if (AppHostInfo) Free(AppHostInfo);
		DebugOutComms("NetUTCP2Open", "DNS Error", (Int32)errP, __FILE__, __LINE__, g);
		{
			char	temp[500];

			StrPrintF(temp, "DNS Err stuff: HostInfo: %ld, time: %ld, iTime: %ld, tot: %ld",
				  (Int32)HostInfo, (Int32)time, (Int32)iTime, (Int32)iTime + AppSysTimeout);
			DebugOutComms("NetUTCP2Open", temp, 0, __FILE__, __LINE__, g);
		}
		return -1;
	}

	DebugOutComms("NetUTCP2Open", "Got DNS", 0, __FILE__, __LINE__, g);

	connectTime = TimGetTicks();

	showProgress(g);

	MemSet(&sockAddrP, sizeof(sockAddrP), 0);

	DebugOutComms("NetUTCP2Open", "sockAddrP init\'d", 0, __FILE__, __LINE__, g);

	if (address == -1) {
		addressPtr = ( NetIPAddr*)HostInfo->addrListP[0];
		address = NetNToHL(*addressPtr);
	}

	DebugOutComms("NetUTCP2Open", "address init\'d", 0, __FILE__, __LINE__, g);

	//MemMove(&sockAddrP.sin_addr,&AppHostInfo.address[0][0],sizeof(AppHostInfo.address[0]));
	/*MemMove(&sockAddrP.sin_addr,&address,sizeof(address));
   	sockAddrP.sin_family = AF_INET;
  	sockAddrP.sin_port = htons(port);*/

	sockAddrP.family = netSocketAddrINET;
	sockAddrP.port   = NetHToNS(port);
	MemMove(&sockAddrP.addr, &address, sizeof(address));

	// Maybe, if we cannot connect to one address, try for other addresses??
connect:

	DebugOutComms("NetUTCP2Open", "sockAddrP setup, open socket", 0, __FILE__, __LINE__, g);

	socket = NetLibSocketOpen(AppNetRefnum, netSocketAddrINET, netSocketTypeStream, 0, AppNetTimeout, &errP);

	if (socket == -1) {
		if (!g->isImage)
			NetDisplayError("Failed to prepare network communications. Try again later.", errP, __FILE__, __LINE__);
		if (AppHostInfo) Free(AppHostInfo);
		DebugOutComms("NetUTCP2Open", "Socket error", (Int32)errP, __FILE__, __LINE__, g);
		return -2;
	}

	showProgress(g);

	DebugOutComms("NetUTCP2Open", "setup socket", 0, __FILE__, __LINE__, g);

	sockLinger.onOff = true;
	sockLinger.time = 0;
	sockConnect = NetLibSocketOptionSet(AppNetRefnum, socket, netSocketOptLevelSocket, netSocketOptSockLinger,
					    &sockLinger, sizeof(sockLinger), AppNetTimeout, &errP);
	if (sockConnect == -1) {
		saveErr = errP;
		NetLibSocketClose(AppNetRefnum, socket, AppNetTimeout, &errP);
		socket = -1;

		if (!g->isImage)
			NetDisplayError("Failed to connect to the remote host or proxy. Try again later.", saveErr, __FILE__, __LINE__);
		if (AppHostInfo) Free(AppHostInfo);

		DebugOutComms("NetUTCP2Open", "failed to setup socket", (Int32)saveErr, __FILE__, __LINE__, g);

		return -2;
	}

	DebugOutComms("NetUTCP2Open", "Connect Socket", 0, __FILE__, __LINE__, g);

	//sockConnect = NetLibSocketConnect(AppNetRefnum, socket, (void*)&sockAddrP, sizeof(NetSocketAddrType), AppNetTimeout, &errP);
	sockConnect = NetLibSocketConnect(AppNetRefnum, socket, (NetSocketAddrType *)&sockAddrP, sizeof(sockAddrP), AppNetTimeout, &errP);

	if (sockConnect == -1) {
		saveErr = errP;
		NetLibSocketClose(AppNetRefnum, socket, AppNetTimeout, &errP);
		socket = -1;

		DebugOutComms("NetUTCP2Open", "failed to connect", (Int32)saveErr, __FILE__, __LINE__, g);
		if ((saveErr == netErrTimeout) && (TimGetTicks() < (connectTime + AppSysTimeout))) {
			goto connect;
		}

		if (!g->isImage)
			NetDisplayError("Failed to connect to the remote host or proxy. Try again later.", saveErr, __FILE__, __LINE__);
		if (AppHostInfo) Free(AppHostInfo);

		DebugOutComms("NetUTCP2Open", "failed to connect, post timeout", (Int32)saveErr, __FILE__, __LINE__, g);

		return -2;
	}

	DebugOutComms("NetUTCP2Open", "connected!", 0, __FILE__, __LINE__, g);

	showProgress(g);

	if (AppHostInfo) Free(AppHostInfo);

	return socket;
}
