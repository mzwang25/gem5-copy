#ifndef __DYNAMIC_CACHE_CTRL__
#define __DYNAMIC_CACHE_CTRL__

#include "mem/port.hh"
#include "mem/cache/cache.hh"
#include "cpu/base.hh"
#include "params/DynamicCacheCtrl.hh"
#include "sim/sim_object.hh"

#include <iostream>


class DynamicCacheCtrl : public SimObject
{
    private:
        class CPUSidePort : public SlavePort
        {
            private:
                DynamicCacheCtrl* owner;

            public:
                CPUSidePort(const std::string& name, DynamicCacheCtrl* owner):
                    SlavePort(name, owner), owner(owner)
                    {}

                Tick recvAtomic(PacketPtr pkt) {return 0;};
                bool recvTimingReq(PacketPtr pkt);
                void recvRespRetry() {};
                void recvFunctional(PacketPtr pkt) 
                    {owner->handleFunctional(pkt);};
                
                AddrRangeList getAddrRanges() const 
                    {return owner->getAddrRanges(); };
        };

        class MemSidePort : public MasterPort
        {
            private:
                DynamicCacheCtrl* owner;
            public:
                MemSidePort(const std::string& name, DynamicCacheCtrl* owner):
                    MasterPort(name, owner), owner(owner)
                    {}
                Tick recvAtomic(PacketPtr pkt) {return 0;};
                bool recvTimingResp(PacketPtr pkt);
                void recvReqRetry();
                
                AddrRangeList getAddrRanges() const 
                    {return this->getAddrRanges();};
                void recvRangeChange()
                    {owner->sendRangeChange();};
        };

        CPUSidePort cpu_side;
        MemSidePort mem_side;
        MemSidePort cache_side_small;
        MemSidePort cache_side_medium;
        MemSidePort cache_side_large;

        //connected cache
        Cache* cache_small;
        Cache* cache_medium;
        Cache* cache_large;

        //connected cpu
        BaseCPU* cpu_object;

        //blocked_packet should be a nullptr unless a retry is needed
        PacketPtr blocked_packet;

        //This function determines which port to use
        MemSidePort* mem_port_to_use();

        //Stores what the module is currently doing (using Cache or not)
        bool current_state;

        //Some stats here
        Stats::Scalar invalidation_ticks;

        int lastStatDump;

    public:
        DynamicCacheCtrl(DynamicCacheCtrlParams* params);
        Port& getPort(const std::string& if_name, PortID idx);
        AddrRangeList getAddrRanges() const {return mem_side.getAddrRanges();}
        void handleFunctional(PacketPtr pkt) {mem_side.sendFunctional(pkt);}
        bool handleTimingReq(PacketPtr pkt);
        bool handleRecvTimingResp(PacketPtr pkt);
        void sendRangeChange() 
        {
            cpu_side.sendRangeChange();
        }

        void regStats() override;
};

#endif
