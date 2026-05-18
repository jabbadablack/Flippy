
#pragma once

#include "../Include/FlippyGem/FlippyGemTypeIds.h"

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace FlippyGem
{
    class FlippyGemRequests
    {
    public:
        AZ_RTTI(FlippyGemRequests, FlippyGemRequestsTypeId);
        virtual ~FlippyGemRequests() = default;
        // Put your public methods here
    };

    class FlippyGemBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using FlippyGemRequestBus = AZ::EBus<FlippyGemRequests, FlippyGemBusTraits>;
    using FlippyGemInterface = AZ::Interface<FlippyGemRequests>;

} // namespace FlippyGem
