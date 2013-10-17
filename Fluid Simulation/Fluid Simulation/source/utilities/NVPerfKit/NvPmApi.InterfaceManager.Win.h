#ifndef _NVPMAPI_INTERFACEMANAGER_WIN_H_
#define _NVPMAPI_INTERFACEMANAGER_WIN_H_

#include <windows.h>
#include <assert.h>
#include "NvPmApi.h"

//  For lifetime manager of NVPMAPI DLL
template<class EXPORTED_API> class InterfaceManager
{
public:
    InterfaceManager(NVPM_UUID InterfaceUUID) : m_hModule(0), m_InterfaceUUID(InterfaceUUID)
    {
        ClearApiTables();
    }

    ~InterfaceManager()
    {
        if (m_hModule)
        {
            ClearApiTables();
            FreeLibrary(m_hModule);
            m_hModule = 0;
        }
    }

    NVPMRESULT Construct(const wchar_t* pLibraryPath)
    {
        if(pLibraryPath == NULL)
        {
            return NVPM_ERROR_INVALID_PARAMETER;
        }

        if (m_hModule != NULL)
        {
            return NVPM_WARNING_DUPLICATE;
        }

        if((m_hModule = LoadLibraryW(pLibraryPath)) == NULL)
        {
            return NVPM_FAILURE;
        }

        NVPMGetExportTable_Pfn pfnGetExportTable = (NVPMGetExportTable_Pfn)GetProcAddress(m_hModule, "NVPMGetExportTable");

        if(pfnGetExportTable == NULL)
        {
            return NVPM_FAILURE;
        }

        if(pfnGetExportTable(&m_InterfaceUUID, (void **) &m_pNvPmApi) != NVPM_OK)
        {
            return NVPM_FAILURE;
        }

        if(m_pNvPmApi->struct_size < sizeof(EXPORTED_API))
        {
            return NVPM_FAILURE;
        }

        return NVPM_OK;
    }

    const EXPORTED_API* Api() const
    {
        return m_pNvPmApi;
    }

private:
    HMODULE m_hModule;
    const EXPORTED_API *m_pNvPmApi;
    NVPM_UUID m_InterfaceUUID;

private:
    void ClearApiTables()
    {
        m_pNvPmApi = NULL;
    }
};

#endif
