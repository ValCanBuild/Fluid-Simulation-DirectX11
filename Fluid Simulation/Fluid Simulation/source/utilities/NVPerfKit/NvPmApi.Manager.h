#ifndef _NVPMAPI_MANAGER_H_
#define _NVPMAPI_MANAGER_H_

#if defined(_WIN32)
#include "NvPmApi.InterfaceManager.Win.h"
#elif defined(__GNUC__)
#include "NvPmApi.InterfaceManager.Linux.h"
#endif

class NvPmApiManager : public InterfaceManager<NvPmApi>
{
public:
    NvPmApiManager() : InterfaceManager<NvPmApi>(ETID_NvPmApi) {}
};

#endif