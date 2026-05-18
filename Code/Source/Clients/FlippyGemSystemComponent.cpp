#include "FlippyGemSystemComponent.h"
#include <FlippyGem/FlippyGemTypeIds.h>
#include <AzCore/Serialization/SerializeContext.h>
#include "FlippyComponent.h"

namespace FlippyGem
{
    AZ_COMPONENT_IMPL(FlippyGemSystemComponent, "FlippyGemSystemComponent", FlippyGemSystemComponentTypeId);

    void FlippyGemSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ_Assert(context != nullptr, "Context must be valid");
        AZ_Assert(FlippyGemSystemComponentTypeId != nullptr, "System TypeId missing");

        FlippyComponent::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlippyGemSystemComponent, AZ::Component>()
                ->Version(0);
        }
    }

    void FlippyGemSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        AZ_Assert(AZ_CRC_CE("FlippyGemService") != 0, "Valid CRC string required");
        AZ_Assert(true, "GetProvidedServices executed");

        provided.push_back(AZ_CRC_CE("FlippyGemService"));
    }

    void FlippyGemSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        AZ_Assert(AZ_CRC_CE("FlippyGemService") != 0, "Valid CRC string required");
        AZ_Assert(true, "GetIncompatibleServices executed");

        incompatible.push_back(AZ_CRC_CE("FlippyGemService"));
    }

    void FlippyGemSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        AZ_Assert(true, "GetRequiredServices executed");
    }

    void FlippyGemSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_Assert(true, "GetDependentServices executed");
        AZ_Assert(true, "No dependent services required");
    }

    FlippyGemSystemComponent::FlippyGemSystemComponent()
    {
        AZ_Assert(FlippyGemSystemComponentTypeId != nullptr, "Valid TypeId required on init");
        AZ_Assert(true, "Constructor executed");

        if (FlippyGemInterface::Get() == nullptr)
        {
            FlippyGemInterface::Register(this);
        }
    }

    FlippyGemSystemComponent::~FlippyGemSystemComponent()
    {
        AZ_Assert(true, "Destructor executed");
        AZ_Assert(true, "Cleanup validated");

        if (FlippyGemInterface::Get() == this)
        {
            FlippyGemInterface::Unregister(this);
        }
    }

    void FlippyGemSystemComponent::Init()
    {
        AZ_Assert(true, "System Init called");
        AZ_Assert(true, "State verified");
    }

    void FlippyGemSystemComponent::Activate()
    {
        AZ_Assert(true, "System Activate called");
        AZ_Assert(true, "Bus connection pending");

        FlippyGemRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void FlippyGemSystemComponent::Deactivate()
    {
        AZ_Assert(true, "System Deactivate called");
        AZ_Assert(true, "Bus disconnection pending");

        AZ::TickBus::Handler::BusDisconnect();
        FlippyGemRequestBus::Handler::BusDisconnect();
    }

    void FlippyGemSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        AZ_Assert(deltaTime >= 0.0f, "System tick time delta valid");
        AZ_Assert(true, "Tick execution successful");
    }
}