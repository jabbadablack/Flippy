
#include <FlippyGem/FlippyGemTypeIds.h>
#include <FlippyGemModuleInterface.h>
#include "FlippyGemEditorSystemComponent.h"

namespace FlippyGem
{
    class FlippyGemEditorModule
        : public FlippyGemModuleInterface
    {
    public:
        AZ_RTTI(FlippyGemEditorModule, FlippyGemEditorModuleTypeId, FlippyGemModuleInterface);
        AZ_CLASS_ALLOCATOR(FlippyGemEditorModule, AZ::SystemAllocator);

        FlippyGemEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                FlippyGemEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<FlippyGemEditorSystemComponent>(),
            };
        }
    };
}// namespace FlippyGem

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), FlippyGem::FlippyGemEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_FlippyGem_Editor, FlippyGem::FlippyGemEditorModule)
#endif
