#include "stdafx.h"

#include "extvars.hpp"

CResourceString *pStrings = NULL;
#ifdef _DEBUG
CLog *pLog = NULL;
#endif
CInstanceManager *pInstances = NULL;
CHookManager *pHookManager	= NULL;
OPCODE_MAP *pOpcodeInfo	= NULL;
CChatSocket *pChatSocket	= NULL;

BOOL bLogging = TRUE;
BOOL bAutoscroll = FALSE;
PVOID pMapleBase = NULL;
DWORD dwMapleSize = 0;
WORD uSeqBase = 83;

/***/
std::list<std::shared_ptr<CMaplePacket>> lPacketPool;