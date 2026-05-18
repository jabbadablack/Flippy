#pragma once
#include <AZCore/Component/ComponentBus.h>
#include <AZCore/std/string/string.h>

namespace FlippyGem
{
    class FlippyComponentRequests : public AZ::ComponentBus
    {
    public:
        virtual void PlayAnimation(const AZStd::string& animationName) = 0;
        virtual void StopAnimation() = 0;
        virtual void SetPlaybackRate(float rate) = 0;
        virtual void SetFrame(int frameIndex) = 0;
    };

    using FlippyComponentRequestBus = AZ::EBus<FlippyComponentRequests>;
}