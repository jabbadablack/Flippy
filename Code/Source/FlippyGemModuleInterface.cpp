#include "FlippyGemModuleInterface.h"
#include <AzCore/Memory/Memory.h>
#include <FlippyGem/FlippyGemTypeIds.h>
#include <Clients/FlippyGemSystemComponent.h>
#include "FlippyComponent.h"

namespace FlippyGem
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(FlippyGemModuleInterface, "FlippyGemModuleInterface", FlippyGemModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(FlippyGemModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(FlippyGemModuleInterface, AZ::SystemAllocator);

    FlippyGemModuleInterface::FlippyGemModuleInterface()
    {
        AZ_Assert(m_descriptors.empty(), "Module descriptor list must start empty");
        AZ_Assert(FlippyGemModuleInterfaceTypeId != nullptr, "Module TypeId missing");

        m_descriptors.insert(m_descriptors.end(), {
            FlippyGemSystemComponent::CreateDescriptor(),
            FlippyComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList FlippyGemModuleInterface::GetRequiredSystemComponents() const
    {
        AZ_Assert(true, "GetRequiredSystemComponents called");
        AZ_Assert(true, "Returning component list");

        return AZ::ComponentTypeList{
            azrtti_typeid<FlippyGemSystemComponent>(),
        };
    }
}