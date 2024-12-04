
//#include "../PbAudio.h"
#include "pba_asio_ext.h"

#pragma mark -- ASIO.cpp

//Eliminated

#pragma mark -- ASIO Driver List

#define ASIODRV_DESC		"description"
#define INPROC_SERVER		"InprocServer32"
#define ASIO_PATH			"software\\asio"
#define COM_CLSID			"clsid"

// ******************************************************************
// Local Functions 
// ******************************************************************
static LONG findDrvPath (char *clsidstr,char *dllpath,int dllpathsize)
{
	HKEY			hkEnum,hksub,hkpath;
	char			databuf[512];
	LONG 			cr,rc = -1;
	DWORD			datatype,datasize;
	DWORD			index;
	OFSTRUCT		ofs;
	HFILE			hfile;
	BOOL			found = FALSE;

	CharLowerBuffA(clsidstr,strlen(clsidstr));
	if ((cr = RegOpenKeyA(HKEY_CLASSES_ROOT,COM_CLSID,&hkEnum)) == ERROR_SUCCESS) {

		index = 0;
		while (cr == ERROR_SUCCESS && !found) {
			cr = RegEnumKeyA(hkEnum,index++,(LPTSTR)databuf,512);
			if (cr == ERROR_SUCCESS) {
				CharLowerBuffA(databuf,strlen(databuf));
				if (!(strcmp(databuf,clsidstr))) {
					if ((cr = RegOpenKeyExA(hkEnum,(LPCTSTR)databuf,0,KEY_READ,&hksub)) == ERROR_SUCCESS) {
						if ((cr = RegOpenKeyExA(hksub,(LPCTSTR)INPROC_SERVER,0,KEY_READ,&hkpath)) == ERROR_SUCCESS) {
							datatype = REG_SZ; datasize = (DWORD)dllpathsize;
							cr = RegQueryValueExA(hkpath,0,0,&datatype,(LPBYTE)dllpath,&datasize);
							if (cr == ERROR_SUCCESS) {
								memset(&ofs,0,sizeof(OFSTRUCT));
								ofs.cBytes = sizeof(OFSTRUCT); 
								hfile = OpenFile(dllpath,&ofs,OF_EXIST);
								if (hfile) rc = 0; 
							}
							RegCloseKey(hkpath);
						}
						RegCloseKey(hksub);
					}
					found = TRUE;	// break out 
				}
			}
		}				
		RegCloseKey(hkEnum);
	}
	return rc;
}


static LPASIODRVSTRUCT newDrvStruct (HKEY hkey,char *keyname,int drvID,LPASIODRVSTRUCT lpdrv)
{
	HKEY	hksub;
	char	databuf[256];
	char	dllpath[MAXPATHLEN];
	WORD	wData[100];
	CLSID	clsid;
	DWORD	datatype,datasize;
	LONG	cr,rc;

	if (!lpdrv) 
	{
		if ((cr = RegOpenKeyExA(hkey,(LPCTSTR)keyname,0,KEY_READ,&hksub)) == ERROR_SUCCESS) 
		{

			datatype = REG_SZ; datasize = 256;
			cr = RegQueryValueExA(hksub,COM_CLSID,0,&datatype,(LPBYTE)databuf,&datasize);
			if (cr == ERROR_SUCCESS) 
			{
				rc = findDrvPath (databuf,dllpath,MAXPATHLEN);
				if (rc == 0) 
				{
					lpdrv = (LPASIODRVSTRUCT)malloc(sizeof(ASIODRVSTRUCT)); // new ASIODRVSTRUCT[1];
					if (lpdrv) 
					{
						memset(lpdrv,0,sizeof(ASIODRVSTRUCT));
						lpdrv->drvID = drvID;
						MultiByteToWideChar(CP_ACP,0,(LPCSTR)databuf,-1,(LPWSTR)wData,100);
						if ((cr = CLSIDFromString((LPOLESTR)wData,(LPCLSID)&clsid)) == S_OK) 
						{
							memcpy(&lpdrv->clsid,&clsid,sizeof(CLSID));
						}

						datatype = REG_SZ; datasize = 256;
						cr = RegQueryValueExA(hksub,ASIODRV_DESC,0,&datatype,(LPBYTE)databuf,&datasize);
						if (cr == ERROR_SUCCESS) 
						{
							strcpy(lpdrv->drvname,databuf);
						}
						else strcpy(lpdrv->drvname,keyname);
					}
				}
			}
			RegCloseKey(hksub);
		}
	}	
	else lpdrv->next = newDrvStruct(hkey,keyname,drvID+1,lpdrv->next);

	return lpdrv;
}

static void deleteDrvStruct (LPASIODRVSTRUCT lpdrv)
{
	IASIO	*iasio;

	if (lpdrv != 0) 
	{
		deleteDrvStruct(lpdrv->next);
		if (lpdrv->asiodrv) {
			iasio = (IASIO *)lpdrv->asiodrv;
			IASIO_Release(iasio);// iasio->Release();
		}
		free(lpdrv); //delete lpdrv;
	}
}

static LPASIODRVSTRUCT getDrvStruct (int drvID,LPASIODRVSTRUCT lpdrv)
{
	while (lpdrv)
	{
		if (lpdrv->drvID == drvID) return lpdrv;
		lpdrv = lpdrv->next;
	}
	return 0;
}

// ******************************************************************
//	AsioDriverList "Class"
// ******************************************************************

// protected members
LPASIODRVSTRUCT	lpdrvlist;
int				numdrv;

void asio_init_driver_list()
{
	HKEY			hkEnum = 0;
	char			keyname[MAXDRVNAMELEN];
	LPASIODRVSTRUCT	pdl;
	LONG 			cr;
	DWORD			index = 0;
	BOOL			fin = FALSE;

	numdrv = 0;
	lpdrvlist = 0;

	cr = RegOpenKeyA(HKEY_LOCAL_MACHINE, ASIO_PATH, &hkEnum);
	while (cr == ERROR_SUCCESS) {
		if ((cr = RegEnumKeyA(hkEnum, index++, (LPTSTR)keyname, MAXDRVNAMELEN)) == ERROR_SUCCESS) {
			lpdrvlist = newDrvStruct(hkEnum, keyname, 0, lpdrvlist);
		}
		else fin = TRUE;
	}
	if (hkEnum) RegCloseKey(hkEnum);

	pdl = lpdrvlist;
	while (pdl)
	{
		numdrv++;
		pdl = pdl->next;
	}

	//if (numdrv) CoInitialize(0); //Single Threaded COM Apartment is Mandatory for IASIO implementation
}

void asio_release_driver_list()
{
	if (numdrv)
	{
		deleteDrvStruct(lpdrvlist);
	}
}

LONG asio_get_num_devices(void)
{
	return (LONG)numdrv;
}

#include <assert.h>

LONG asio_open_driver(int drvID, LPVOID *asiodrv)
{
	LPASIODRVSTRUCT	lpdrv = 0;
	long			rc;

	if (!asiodrv) return DRVERR_INVALID_PARAM;
	if ((lpdrv = getDrvStruct(drvID,lpdrvlist)) != 0) 
	{
		if (!lpdrv->asiodrv) 
		{
			rc = CoCreateInstance((REFIID)&(lpdrv->clsid),0, CLSCTX_INPROC_SERVER, (REFIID) & (lpdrv->clsid), asiodrv);
			if (rc == S_OK)
			{
				lpdrv->asiodrv = *asiodrv;
				return 0;
			}
			else if (rc == REGDB_E_CLASSNOTREG) printf("Driver not registered in the Registration Database!");
			else if (rc == E_NOINTERFACE) assert(1 == 0);
		}
		else rc = DRVERR_DEVICE_ALREADY_OPEN;
	}
	else rc = DRVERR_DEVICE_NOT_FOUND;
	
	return rc;
}


LONG asio_close_driver(int drvID)
{
	LPASIODRVSTRUCT	lpdrv = 0;
	IASIO			*iasio;

	if ((lpdrv = getDrvStruct(drvID,lpdrvlist)) != 0) {
		if (lpdrv->asiodrv) {
			iasio = (IASIO *)lpdrv->asiodrv;
			IASIO_Release(iasio);// iasio->Release();
			lpdrv->asiodrv = 0;
		}
	}

	return 0;
}

LONG asio_get_driver_name(int drvID, char* drvname, int drvnamesize)
{
    LPASIODRVSTRUCT			lpdrv = 0;

    if (!drvname) return DRVERR_INVALID_PARAM;

    if ((lpdrv = getDrvStruct(drvID, lpdrvlist)) != 0) {
        if (strlen(lpdrv->drvname) < (unsigned int)drvnamesize) {
            strcpy(drvname, lpdrv->drvname);
        }
        else {
            memcpy(drvname, lpdrv->drvname, drvnamesize - 4);
            drvname[drvnamesize - 4] = '.';
            drvname[drvnamesize - 3] = '.';
            drvname[drvnamesize - 2] = '.';
            drvname[drvnamesize - 1] = 0;
        }
        return 0;
    }
    return DRVERR_DEVICE_NOT_FOUND;
}


#pragma mark -- ASIO Drivers

// ******************************************************************
//	AsioDrivers "Class"
// ******************************************************************

//protected members
unsigned long connID = 0; //i don't know what this is used for
//long curIndex = -1;	  //active index is a property on the stream

long asio_get_driver_names(char** names, long maxDrivers)
{
	for (long i = 0; i < asio_get_num_devices() && i < maxDrivers; i++) asio_get_driver_name(i, names[i], 32);
	return asio_get_num_devices() < maxDrivers ? asio_get_num_devices() : maxDrivers;
}


/*
int asio_get_current_driver_name(char *name)
{
	if(curIndex >= 0) return asio_get_driver_name(curIndex, name, 32) == 0 ? 1 : 0;
	name[0] = 0;
	return 0;
}

void asio_remove_current_driver(void)
{
	if (curIndex != -1) asio_close_driver(curIndex); curIndex = -1;
}

int asio_load_driver(char* name)
{
	char dname[64];
	char curName[64];

	for (long i = 0; i < asio_get_num_devices(); i++)
	{
		if (!asio_get_driver_name(i, dname, 32) && !strcmp(name, dname))
		{
			curName[0] = 0;
			asio_get_current_driver_name(curName);	// in case we fail...
			asio_remove_current_driver();

			if (!asio_open_driver(i, (void**)&theAsioDriver))
			{
				curIndex = i;
				return 1;
			}
			else
			{
				theAsioDriver = 0;
				if (curName[0] && strcmp(dname, curName))
					asio_load_driver(curName);	// try restore
			}
			break;
		}
	}
	return 0;
}
*/

#pragma mark -- ASIO .cpp

//-----------------------------------------------------------------------------------------------------
ASIOError asio_init(IASIO* driver, ASIODriverInfo* info)
{
#if MAC || SGI || SUN || BEOS || LINUX
	if (theAsioDriver)
	{
		delete theAsioDriver;
		theAsioDriver = 0;
	}
	info->driverVersion = 0;
	strcpy(info->name, "No ASIO Driver");
	theAsioDriver = getDriver();
	if (!theAsioDriver)
	{
		strcpy(info->errorMessage, "Not enough memory for the ASIO driver!");
		return ASE_NotPresent;
	}
	if (!theAsioDriver->init(info->sysRef))
	{
		theAsioDriver->getErrorMessage(info->errorMessage);
		delete theAsioDriver;
		theAsioDriver = 0;
		return ASE_NotPresent;
	}
	strcpy(info->errorMessage, "No ASIO Driver Error");
	theAsioDriver->getDriverName(info->name);
	info->driverVersion = theAsioDriver->getDriverVersion();
	return ASE_OK;

#else

	info->driverVersion = 0;
	strcpy(info->name, "No ASIO Driver");
	if (driver)	// must be loaded!
	{
		if (!CALL(init, driver, info->sysRef))
		{
			CALL(getErrorMessage, driver, info->errorMessage);
			driver = 0;
			return ASE_NotPresent;
		}

		strcpy(info->errorMessage, "No ASIO Driver Error");
		CALL(getDriverName, driver, info->name);
		info->driverVersion = CALL(getDriverVersion, driver);
		return ASE_OK;
	}
	return ASE_NotPresent;

#endif	// !MAC
}

ASIOError asio_shutdown(void** driverRef, int* driverID)
{
	if (*driverRef)
	{
#ifdef _WIN32
		if (*driverID != -1) asio_close_driver(*driverID); *driverID = -1;
#else
		free(*driverRef);// delete theAsioDriver;
#endif
	}
	*driverRef = 0;
	return ASE_OK;
}

ASIOError asio_start(IASIO* driver)
{
	if (!driver) return ASE_NotPresent;
	return CALL(start, driver);
}

ASIOError asio_stop(IASIO* driver)
{
	if (!driver) return ASE_NotPresent;
	return CALL(stop, driver);
}


ASIOError asio_get_channels(IASIO* driver, long* numInputChannels, long* numOutputChannels)
{
	if (!driver)
	{
		*numInputChannels = *numOutputChannels = 0;
		return ASE_NotPresent;
	}
	return CALL(getChannels, driver, numInputChannels, numOutputChannels);
}


ASIOError asio_get_latencies(IASIO* driver, long* inputLatency, long* outputLatency)
{
	if (!driver)
	{
		*inputLatency = *outputLatency = 0;
		return ASE_NotPresent;
	}
	return CALL(getLatencies, driver, inputLatency, outputLatency);
}


ASIOError asio_get_buffer_size(IASIO* driver, long* minSize, long* maxSize, long* preferredSize, long* granularity)
{
	if (!driver)
	{
		*minSize = *maxSize = *preferredSize = *granularity = 0;
		return ASE_NotPresent;
	}
	return CALL(getBufferSize, driver, minSize, maxSize, preferredSize, granularity);
}

ASIOError asio_can_sample_rate(IASIO* driver, ASIOSampleRate sampleRate)
{
	if (!driver) return ASE_NotPresent;
	return CALL(canSampleRate, driver, sampleRate);
}

ASIOError asio_get_sample_rate(IASIO* driver, ASIOSampleRate* currentRate)
{
	if (!driver) return ASE_NotPresent;
	return CALL(getSampleRate, driver, currentRate);
}

ASIOError asio_set_sample_rate(IASIO* driver, ASIOSampleRate sampleRate)
{
	if (!driver) return ASE_NotPresent;
	return CALL(setSampleRate, driver, sampleRate);
}


ASIOError asio_get_clock_source(IASIO* driver, ASIOClockSource* clocks, long* numSources)
{
	if (!driver)
	{
		*numSources = 0;
		return ASE_NotPresent;
	}
	return CALL(getClockSources, driver, clocks, numSources);
}

ASIOError asio_set_clock_source(IASIO* driver, long reference)
{
	if (!driver) return ASE_NotPresent;
	return CALL(setClockSource, driver, reference);
}

ASIOError asio_get_sample_position(IASIO* driver, ASIOSamples* sPos, ASIOTimeStamp* tStamp)
{
	if (!driver) return ASE_NotPresent;
	return CALL(getSamplePosition, driver, sPos, tStamp);
}

ASIOError asio_get_channel_info(IASIO* driver, ASIOChannelInfo* info)
{
	if (!driver)
	{
		info->channelGroup = -1;
		info->type = ASIOSTInt16MSB;
		strcpy(info->name, "None");
		return ASE_NotPresent;
	}
	return CALL(getChannelInfo, driver, info);
}

ASIOError asio_create_buffers(IASIO* driver, ASIOBufferInfo* bufferInfos, long numChannels, long bufferSize, ASIOCallbacks* callbacks)
{
	if (!driver)
	{
		ASIOBufferInfo* info = bufferInfos;
		for (long i = 0; i < numChannels; i++, info++)
			info->buffers[0] = info->buffers[1] = 0;
		return ASE_NotPresent;
	}
	return CALL(createBuffers, driver, bufferInfos, numChannels, bufferSize, callbacks);
}

ASIOError asio_dispose_buffers(IASIO* driver)
{
	if (!driver) return ASE_NotPresent;
	return CALL(disposeBuffers, driver);
}


ASIOError asio_control_panel(IASIO* driver)
{
	if (!driver) return ASE_NotPresent;
	return CALL(controlPanel, driver);
}

ASIOError asio_future(IASIO* driver, long selector, void* opt)
{
	if (!driver) return ASE_NotPresent;
	return CALL(future, driver, selector, opt);
}

ASIOError asio_output_ready(IASIO* driver)
{
	if (!driver) return ASE_NotPresent;
	return CALL(outputReady, driver);
}