#pragma once
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>

namespace FlippyGem
{
    class FlippyComponentRequests : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(FlippyComponentRequests, "{21B35C86-5F2D-4DF3-B875-14EE3BAE64CA}");
        virtual ~FlippyComponentRequests() = default;

        virtual void PlayAnimation(const AZStd::string& animationName) = 0;
        virtual void StopAnimation() = 0;
    };
    using FlippyComponentRequestBus = AZ::EBus<FlippyComponentRequests>;
}