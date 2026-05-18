
#include <FlippyGem/FlippyGemTypeIds.h>
#include <FlippyGemModuleInterface.h>
#include "FlippyGemSystemComponent.h"

namespace FlippyGem
{
    class FlippyGemModule
        : public FlippyGemModuleInterface
    {
    public:
        AZ_RTTI(FlippyGemModule, FlippyGemModuleTypeId, FlippyGemModuleInterface);
        AZ_CLASS_ALLOCATOR(FlippyGemModule, AZ::SystemAllocator);
    };
}// namespace FlippyGem

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), FlippyGem::FlippyGemModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_FlippyGem, FlippyGem::FlippyGemModule)
#endif
