
#include <AzCore/Serialization/SerializeContext.h>
#include "FlippyGemEditorSystemComponent.h"

#include <FlippyGem/FlippyGemTypeIds.h>

namespace FlippyGem
{
    AZ_COMPONENT_IMPL(FlippyGemEditorSystemComponent, "FlippyGemEditorSystemComponent",
        FlippyGemEditorSystemComponentTypeId, BaseSystemComponent);

    void FlippyGemEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlippyGemEditorSystemComponent, FlippyGemSystemComponent>()
                ->Version(0);
        }
    }

    FlippyGemEditorSystemComponent::FlippyGemEditorSystemComponent() = default;

    FlippyGemEditorSystemComponent::~FlippyGemEditorSystemComponent() = default;

    void FlippyGemEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("FlippyGemEditorService"));
    }

    void FlippyGemEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("FlippyGemEditorService"));
    }

    void FlippyGemEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void FlippyGemEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void FlippyGemEditorSystemComponent::Activate()
    {
        FlippyGemSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void FlippyGemEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        FlippyGemSystemComponent::Deactivate();
    }

} // namespace FlippyGem
