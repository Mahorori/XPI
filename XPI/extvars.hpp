#ifndef EXT_VARS_HPP_
#define EXT_VARS_HPP_

#include <list>
#include <map>
#include <memory>

// opcode specs
#define OP_ALIAS_MAXCC        17
#define OP_COMMENT_MAXCC      100

struct OPCODE_INFO
{
	WCHAR     wszAlias[OP_ALIAS_MAXCC];
	WCHAR     wszComment[OP_COMMENT_MAXCC];
	BOOL      bIgnore;
	BOOL      bBlock;
	COLORREF  crColor;
};

typedef std::map<WORD, OPCODE_INFO> OPCODE_MAP;

// class references for pointer declarations
class CResourceString;
class CLog;
class CInstanceManager;
class CHookManager;
class CMaplePacket;
//
struct CClientSocket;
struct CChatSocket;

extern CResourceString *pStrings;
#ifdef _DEBUG
extern CLog *pLog;
#endif
extern CInstanceManager *pInstances;
extern CHookManager *pHookManager;
extern OPCODE_MAP *pOpcodeInfo;
extern CChatSocket *pChatSocket;
extern BOOL	bLogging;
extern BOOL bAutoscroll;
extern PVOID pMapleBase;
extern DWORD dwMapleSize;
extern WORD uSeqBase;

/***/
extern std::list<std::shared_ptr<CMaplePacket>> lPacketPool;

#endif // EXT_VARS_HPP_
