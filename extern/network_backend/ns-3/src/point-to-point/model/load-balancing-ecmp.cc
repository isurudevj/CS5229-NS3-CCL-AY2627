/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/load-balancing-ecmp.h"

#include "ns3/assert.h"
#include "ns3/hash.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("ECMPLoadBalancing");

namespace ns3 {

namespace {

uint32_t GetFlowHash(const CustomHeader& ch) {
    uint16_t sourcePort = 0;
    uint16_t destinationPort = 0;

    if (ch.l3Prot == 0x06) {  // TCP
        sourcePort = ch.tcp.sport;
        destinationPort = ch.tcp.dport;
    } else if (ch.l3Prot == 0x11) {  // UDP/RDMA data
        sourcePort = ch.udp.sport;
        destinationPort = ch.udp.dport;
    } else if (ch.l3Prot == 0xFC || ch.l3Prot == 0xFD) {  // ACK/NACK
        sourcePort = ch.ack.sport;
        destinationPort = ch.ack.dport;
    } else if (ch.l3Prot == 0xFF) {  // CNP
        sourcePort = ch.cnp.fid;
        destinationPort = ch.cnp.qIndex;
    } else if (ch.l3Prot == 0xFE) {  // PFC
        sourcePort = ch.pfc.qIndex;
    }

    // Serialize the five tuple explicitly so padding and host byte order cannot
    // make the hash vary across compilers or platforms.
    uint8_t key[13] = {
        static_cast<uint8_t>(ch.sip >> 24),
        static_cast<uint8_t>(ch.sip >> 16),
        static_cast<uint8_t>(ch.sip >> 8),
        static_cast<uint8_t>(ch.sip),
        static_cast<uint8_t>(ch.dip >> 24),
        static_cast<uint8_t>(ch.dip >> 16),
        static_cast<uint8_t>(ch.dip >> 8),
        static_cast<uint8_t>(ch.dip),
        static_cast<uint8_t>(sourcePort >> 8),
        static_cast<uint8_t>(sourcePort),
        static_cast<uint8_t>(destinationPort >> 8),
        static_cast<uint8_t>(destinationPort),
        static_cast<uint8_t>(ch.l3Prot),
    };

    return Hash32(reinterpret_cast<const char*>(key), sizeof(key));
}

}  // namespace

ECMPLoadBalancing::ECMPLoadBalancing()
    : m_isToR(false), m_switchId(static_cast<uint32_t>(-1)) {}

TypeId ECMPLoadBalancing::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::ECMPLoadBalancing")
                            .SetParent<Object>()
                            .AddConstructor<ECMPLoadBalancing>();
    return tid;
}

void ECMPLoadBalancing::SetSwitchSendCallback(SwitchSendCallback switchSendCallback) {
    m_switchSendCallback = switchSendCallback;
}

void ECMPLoadBalancing::SetSwitchInfo(bool isToR, uint32_t switch_id) {
    m_isToR = isToR;
    m_switchId = switch_id;
}

void ECMPLoadBalancing::DoSwitchSend(Ptr<Packet> p,
                                      CustomHeader& ch,
                                      uint32_t outDev,
                                      uint32_t qIndex) {
    m_switchSendCallback(p, ch, outDev, qIndex);
}

uint32_t ECMPLoadBalancing::GetQueueIndex(const CustomHeader& ch) const {
    if (ch.l3Prot == 0xFF || ch.l3Prot == 0xFE || ch.l3Prot == 0xFD || ch.l3Prot == 0xFC) {
        return 0;  // QCN, PFC, NACK, and ACK have highest priority.
    }
    return ch.l3Prot == 0x06 ? 1 : ch.udp.pg;
}

void ECMPLoadBalancing::RouteInput(Ptr<Packet> p, CustomHeader ch) {
    auto entry = m_dstIPRouting.find(ch.dip);
    NS_ASSERT_MSG(entry != m_dstIPRouting.end(),
                  "No route from switch " << m_switchId << " to destination " << ch.dip);
    NS_ASSERT_MSG(!entry->second.empty(),
                  "No equal-cost next hop from switch " << m_switchId << " to destination " << ch.dip);

    // Keep every packet in a flow on the same path while distributing distinct
    // flows over however many equal-cost next hops are currently installed.
    const auto& nextHops = entry->second;
    const uint32_t outDev = nextHops[GetFlowHash(ch) % nextHops.size()];
    DoSwitchSend(p, ch, outDev, GetQueueIndex(ch));
}

}  // namespace ns3
