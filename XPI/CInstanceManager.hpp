#ifndef CINSTANCE_MANAGER_HPP
#define CINSTANCE_MANAGER_HPP

#include <map>
#include <memory>
#include "CMaplePacket.hpp"

typedef std::map<PVOID, std::shared_ptr<CMaplePacket>> PACKET_INSTANCES;

class CInstanceManager
{
private:
	PACKET_INSTANCES m_Instances;
public:
	std::shared_ptr<CMaplePacket> Find(__in PVOID pInstance);
	VOID Add(__in PVOID pInstance, __in std::shared_ptr<CMaplePacket> Packet);
	VOID Remove(__in PVOID pInstance);
	VOID Clear();
	std::shared_ptr<CMaplePacket> operator[](__in PVOID pInstance);
};

#endif // CINSTANCE_MANAGER_HPP