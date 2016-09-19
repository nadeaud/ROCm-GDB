//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief Class to manage the agent binary
//==============================================================================
#include <cassert>
#include <cstring>
#include <cstdlib>

#include <hsa_ext_amd.h>
#include <amd_hsa_kernel_code.h>

#include <libelf.h>

#include "AgentBinary.h"
#include "AgentConfiguration.h"
#include "AgentISABuffer.h"
#include "AgentLogging.h"
#include "AgentNotifyGdb.h"
#include "AgentUtils.h"
#include "CommunicationControl.h"
#include "HSADebugAgent.h"

#include "AMDGPUDebug.h"

namespace HwDbgAgent
{

/// Default constructor
AgentBinary::AgentBinary():
    m_pBinary(nullptr),
    m_binarySize(0),
    m_llSymbolName(""),
    m_hlSymbolName(""),
    m_kernelName(""),
    m_codeObjBufferShmKey(-1),
    m_codeObjBufferMaxSize(0),
    m_pIsaBuffer(nullptr),
    m_enableISADisassemble(true)
{
    HsailAgentStatus status = HSAIL_AGENT_STATUS_FAILURE;
    status = GetActiveAgentConfig()->GetConfigShmKey(HSAIL_DEBUG_CONFIG_CODE_OBJ_SHM, m_codeObjBufferShmKey);
    if (status != HSAIL_AGENT_STATUS_SUCCESS)
    {
        AGENT_ERROR("Could not get shared mem key");
    }
    status = GetActiveAgentConfig()->GetConfigShmSize(HSAIL_DEBUG_CONFIG_CODE_OBJ_SHM, m_codeObjBufferMaxSize);
    if (status != HSAIL_AGENT_STATUS_SUCCESS)
    {
        AGENT_ERROR("Could not get shared mem max size");
    }

    m_pIsaBuffer = new (std::nothrow) AgentISABuffer;
    if (m_pIsaBuffer == nullptr)
    {
        AGENT_ERROR("Could not allocate the ISA buffer");
    }

    char* pDisableISAEnvVar = nullptr;
    pDisableISAEnvVar = std::getenv("ROCM_GDB_DISABLE_ISA_DISASSEMBLE");
    if (pDisableISAEnvVar != nullptr)
    {
        AGENT_LOG("Disable GPU ISA disassemble," <<
                  " ROCM_GDB_DISABLE_ISA_DISASSEMBLE = " << pDisableISAEnvVar);
        AGENT_OP("Disable GPU ISA disassemble," <<
                  " ROCM_GDB_DISABLE_ISA_DISASSEMBLE = " << pDisableISAEnvVar);

        m_enableISADisassemble = false;
    }
}

AgentBinary::~AgentBinary()
{
    if(m_pIsaBuffer != nullptr)
    {
        delete m_pIsaBuffer;
        m_pIsaBuffer = nullptr;
    }
}

// Read the binary buffer and get the HL and LL symbol names
bool AgentBinary::GetDebugSymbolsFromBinary()
{

    if ((nullptr == m_pBinary) || (0 == m_binarySize))
    {
        return false;
    }

    // Get the symbol list:
    std::vector<std::pair<std::string, uint64_t>> elfSymbols;
    ExtractSymbolListFromELFBinary(m_pBinary, m_binarySize, elfSymbols);

    // No symbols = nothing found:
    if (elfSymbols.empty())
    {
        return false;
    }

    bool isllSymbolFound = false;

    // The matchable string
    static const std::string kernelNamePrefix1 = "__debug_isa__";   // ISA DWARF symbol
    static const size_t kernelNamePrefix1Length = kernelNamePrefix1.length();   // Pre-calculate string lengths

    // Iterate the symbols to look for matches:
    const size_t symbolsCount = elfSymbols.size();

    for (size_t i = 0; symbolsCount > i; ++i)
    {
        const std::string& curSym = elfSymbols[i].first;

        if (0 == curSym.compare(0, kernelNamePrefix1Length, kernelNamePrefix1))
        {
            // Found a level 1 match (the first one). It overrides any other matches, so return it!
            m_llSymbolName.assign(curSym);
            isllSymbolFound = true;
            break;
        }
    }

    // The HL symbol is always the same
    m_hlSymbolName.assign("__debug_brig__");

    if (isllSymbolFound)
    {
        return true;
    }

    AGENT_ERROR("GetDebugSymbolsFromBinary:Could not HL and LL symbols correctly");

    return false;


}

// Call the DBE and set up the buffer
HsailAgentStatus AgentBinary::PopulateBinaryFromDBE(HwDbgContextHandle dbgContextHandle,
                                                    const hsa_kernel_dispatch_packet_t* pAqlPacket)
{
    AGENT_LOG("Initialize a new binary");
    assert(dbgContextHandle != nullptr);

    HsailAgentStatus status = HSAIL_AGENT_STATUS_FAILURE;

    if (nullptr == dbgContextHandle)
    {
        AGENT_ERROR("Invalid DBE Context handle");
        return status;
    }

    // Note: Even though the DBE only gets a pointer for the binary,
    // the size of the binary is generated by the HwDbgHSAContext
    // by using ACL

    // Get the debugged kernel binary from DBE
    // A pointer to constant data
    HwDbgStatus dbeStatus = HwDbgGetKernelBinary(dbgContextHandle,
                                                 &m_pBinary,
                                                 &m_binarySize);

    assert(dbeStatus == HWDBG_STATUS_SUCCESS);
    assert(m_pBinary != nullptr);

    if (dbeStatus != HWDBG_STATUS_SUCCESS ||
        m_pBinary == nullptr)
    {
        AGENT_ERROR(GetDBEStatusString(dbeStatus) <<
                    "PopulateBinaryFromDBE: Error in HwDbgGeShaderBinary");

        // Something was wrong we should exit without writing the binary
        status = HSAIL_AGENT_STATUS_FAILURE;
        return status;
    }
    else
    {
        status = HSAIL_AGENT_STATUS_SUCCESS;
    }

    // get the kernel name for the active dispatch
    const char* pKernelName(nullptr);
    dbeStatus = HwDbgGetDispatchedKernelName(dbgContextHandle, &pKernelName);
    assert(dbeStatus == HWDBG_STATUS_SUCCESS);
    assert(pKernelName != nullptr);

    if (dbeStatus != HWDBG_STATUS_SUCCESS ||
        pKernelName == nullptr)
    {
        AGENT_ERROR("PopulateBinaryFromDBE: Could not get the name of the kernel");
        status = HSAIL_AGENT_STATUS_FAILURE;
        return status;
    }
    else
    {
        status = HSAIL_AGENT_STATUS_SUCCESS;
    }

    m_kernelName.assign(pKernelName);

    AGENT_LOG("PopulateBinaryFromDBE: Kernel Name found " << m_kernelName);

    if (m_enableISADisassemble)
    {
        m_pIsaBuffer->PopulateISAFromCodeObj(m_binarySize, m_pBinary);
    }

    return status;
}

const std::string AgentBinary::GetKernelName()const
{
    return m_kernelName;
}

/// Validate parameters of the binary, write the binary to shmem
/// and let gdb know we have a new binary
HsailAgentStatus AgentBinary::NotifyGDB(const hsa_kernel_dispatch_packet_t* pAqlPacket,
                                        const uint64_t                      queueID,
                                        const uint64_t                      packetID) const
{

    HsailAgentStatus status;

    // Check that kernel name is not empty
    if (m_kernelName.empty())
    {
        AGENT_LOG("NotifyGDB: Kernel name may not have not been populated");
    }

    // Call function in AgentNotify
    // Let gdb know we have a new binary
    status = WriteBinaryToSharedMem();

    if (HSAIL_AGENT_STATUS_FAILURE == status)
    {
        AGENT_ERROR("NotifyGDB: Could not write binary to shared mem");
        return status;
    }

    status = AgentNotifyNewBinary(m_binarySize,
                                  m_hlSymbolName, m_llSymbolName,
                                  m_kernelName,
                                  pAqlPacket,
                                  queueID,
                                  packetID);

    if (HSAIL_AGENT_STATUS_FAILURE == status)
    {
        AGENT_ERROR("NotifyGDB: Couldnt not notify gdb");
    }

    return status;
}

HsailAgentStatus AgentBinary::WriteBinaryToSharedMem() const
{
    HsailAgentStatus status = HSAIL_AGENT_STATUS_FAILURE;

    if (m_pBinary == nullptr)
    {
        AGENT_ERROR("WriteBinaryToShmem: Error Binary is null");
        return status;
    }

    if (m_binarySize <= 0)
    {
        AGENT_ERROR("WriteBinaryToShmem: Error Binary size is 0");
        return status;
    }

    if (m_binarySize > m_codeObjBufferMaxSize)
    {
        AGENT_ERROR("WriteBinaryToShmem: Error Binary is larger than the shared mem allocated");
        return status;
    }

    // The shared mem segment needs place for a size_t value and the binary
    if ((m_binarySize + sizeof(size_t)) > m_codeObjBufferMaxSize)
    {
        AGENT_ERROR("WriteBinaryToShmem: Binary size is too big");
        return status;
    }

    // Get the pointer to the shmem segment
    void* pShm = AgentMapSharedMemBuffer(m_codeObjBufferShmKey, m_codeObjBufferMaxSize);

    if (pShm == (int*) - 1)
    {
        AGENT_ERROR("WriteBinaryToShmem: Error with AgentMapSharedMemBuffer");
        return status;
    }

    // Clear the memory fist
    memset(pShm, 0, m_codeObjBufferMaxSize);

    // Write the size first
    size_t* pShmSizeLocation = (size_t*)pShm;
    pShmSizeLocation[0] = m_binarySize;

    AGENT_LOG("DBE Code object size: " << pShmSizeLocation[0]);

    // Write the binary after the size_t info
    void* pShmBinaryLocation = (size_t*)pShm + 1;
    if (m_binarySize < m_codeObjBufferMaxSize)
    {
        // Copy the binary
        memcpy(pShmBinaryLocation, m_pBinary, m_binarySize);
    }
    else
    {
        AGENT_WARNING("WriteBinaryToSharedMem: Did not copy code object to shared memory");
        AGENT_WARNING("Binary Size is =" << pShmSizeLocation[0] <<
                      " but shared memory size = " << m_codeObjBufferMaxSize << "bytes");
    }

    //printf("Debug OP");
    //for(int i = 0; i<10;i++)
    //{
    //    printf("%d \t %d\n",i,*((int*)pShmBinaryLocation + i));
    //}

    // Detach shared memory
    status = AgentUnMapSharedMemBuffer(pShm);

    if (status != HSAIL_AGENT_STATUS_SUCCESS)
    {
        AGENT_ERROR("WriteBinaryToShmem: Error with AgentUnMapSharedMemBuffer");
        return status;
    }

    return status;
}

HsailAgentStatus AgentBinary::WriteBinaryToFile(const char* pFilename) const
{

    HsailAgentStatus status = AgentWriteBinaryToFile(m_pBinary, m_binarySize, pFilename);

    if (status == HSAIL_AGENT_STATUS_SUCCESS)
    {
        AGENT_LOG("DBE Binary Saved to " << pFilename);
    }
    return status;
}
} // End Namespace HwDbgAgent
