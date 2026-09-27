/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/load-balancing-ecmp.h"

#include "ns3/assert.h"
#include "ns3/hash.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("ECMPLoadBalancing");

namespace ns3 {

namespace {

void GetFlowPorts(const CustomHeader& ch, uint16_t& sourcePort, uint16_t& destinationPort) {
    sourcePort = 0;
    destinationPort = 0;

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
}

std::string GetFlowKey(const CustomHeader& ch) {
    uint16_t sourcePort;
    uint16_t destinationPort;
    GetFlowPorts(ch, sourcePort, destinationPort);

    // Serialize the five tuple explicitly so padding and host byte order cannot
    // make the hash vary across compilers or platforms.
    char key[13] = {
        static_cast<char>(ch.sip >> 24),
        static_cast<char>(ch.sip >> 16),
        static_cast<char>(ch.sip >> 8),
        static_cast<char>(ch.sip),
        static_cast<char>(ch.dip >> 24),
        static_cast<char>(ch.dip >> 16),
        static_cast<char>(ch.dip >> 8),
        static_cast<char>(ch.dip),
        static_cast<char>(sourcePort >> 8),
        static_cast<char>(sourcePort),
        static_cast<char>(destinationPort >> 8),
        static_cast<char>(destinationPort),
        static_cast<char>(ch.l3Prot),
    };

    return std::string(key, sizeof(key));
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
    const std::string flowKey = GetFlowKey(ch);
    const uint32_t flowHash = Hash32(flowKey.data(), flowKey.size());
    const uint32_t pathIndex = flowHash % nextHops.size();
    const uint32_t outDev = nextHops[pathIndex];

    // Print only the first three observations of each flow at each switch. The
    // repeated lines make hash/path consistency visible without producing one
    // log line per packet. This is diagnostic output for the debug build.
    uint32_t& observationCount = m_flowLogCounts[flowKey];
    if (observationCount < 3) {
        uint16_t sourcePort;
        uint16_t destinationPort;
        GetFlowPorts(ch, sourcePort, destinationPort);
        NS_LOG_UNCOND("ECMP_HASH switch=" << m_switchId
                                           << " observation=" << (observationCount + 1)
                                           << " sip=" << ch.sip
                                           << " dip=" << ch.dip
                                           << " sport=" << sourcePort
                                           << " dport=" << destinationPort
                                           << " protocol=" << ch.l3Prot
                                           << " hash=" << flowHash
                                           << " pathIndex=" << pathIndex
                                           << " pathCount=" << nextHops.size()
                                           << " outDev=" << outDev);
    }
    ++observationCount;

    DoSwitchSend(p, ch, outDev, GetQueueIndex(ch));
}

}  // namespace ns3
