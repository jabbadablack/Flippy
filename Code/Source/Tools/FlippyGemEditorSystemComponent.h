
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/FlippyGemSystemComponent.h>

namespace FlippyGem
{
    /// System component for FlippyGem editor
    class FlippyGemEditorSystemComponent
        : public FlippyGemSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = FlippyGemSystemComponent;
    public:
        AZ_COMPONENT_DECL(FlippyGemEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        FlippyGemEditorSystemComponent();
        ~FlippyGemEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace FlippyGem
