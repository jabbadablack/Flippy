#include "FlippyGemSystemComponent.h"

#include <FlippyGem/FlippyGemTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace FlippyGem
{
    AZ_COMPONENT_IMPL(FlippyGemSystemComponent, "FlippyGemSystemComponent",
        FlippyGemSystemComponentTypeId);

    void FlippyGemSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlippyGemSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void FlippyGemSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FlippyGemService"));
    }

    void FlippyGemSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FlippyGemService"));
    }

    void FlippyGemSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void FlippyGemSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    FlippyGemSystemComponent::FlippyGemSystemComponent()
    {
        if (FlippyGemInterface::Get() == nullptr)
        {
            FlippyGemInterface::Register(this);
        }
    }

    FlippyGemSystemComponent::~FlippyGemSystemComponent()
    {
        if (FlippyGemInterface::Get() == this)
        {
            FlippyGemInterface::Unregister(this);
        }
    }

    void FlippyGemSystemComponent::Init()
    {
    }

    void FlippyGemSystemComponent::Activate()
    {
        FlippyGemRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void FlippyGemSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        FlippyGemRequestBus::Handler::BusDisconnect();
    }

    void FlippyGemSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace FlippyGem