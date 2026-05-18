#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialAssignment.h>

namespace FlippyGem
{
    struct FlippyAnimation
    {
        AZ_TYPE_INFO(FlippyAnimation, "{00ae072a-dbfc-41ac-aa40-324d58f6ff3e}");
        AZ_CLASS_ALLOCATOR(FlippyAnimation, AZ::SystemAllocator);

        AZStd::string m_name = "Idle";
        int m_startRow = 0;
        int m_startColumn = 0;
        int m_frameCount = 1;
        float m_fps = 12.0f;
    };

    // The Bus is now declared directly inside the header
    class FlippyComponentRequests : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(FlippyComponentRequests, "{4cbefb6c-38f3-4a39-80f4-51019836440a}");
        virtual ~FlippyComponentRequests() = default;

        virtual void PlayAnimation(const AZStd::string& animationName) = 0;
        virtual void StopAnimation() = 0;
    };
    using FlippyComponentRequestBus = AZ::EBus<FlippyComponentRequests>;

    class FlippyComponent
        : public AZ::Component
        , protected AZ::TickBus::Handler
        , public FlippyComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FlippyComponent, "{334d8e3b-5f54-4dde-8201-e632c4508fd2}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

    protected:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void PlayAnimation(const AZStd::string& animationName) override;
        void StopAnimation() override;

    private:
        AZStd::vector<AZ::Render::MaterialAssignmentId> GetActiveMaterialIds(AZ::EntityId targetEntity);

        void ApplyMaterialScale(float tileU, float tileV);
        void ApplyMaterialOffset(float offsetU, float offsetV);

        AZ::EntityId m_materialEntityId;

        AZStd::string m_uvTileUProperty = "uv.tileU";
        AZStd::string m_uvTileVProperty = "uv.tileV";
        AZStd::string m_uvOffsetUProperty = "uv.offsetU";
        AZStd::string m_uvOffsetVProperty = "uv.offsetV";

        int m_columns = 1;
        int m_rows = 1;

        AZStd::vector<FlippyAnimation> m_animations;
        AZStd::string m_defaultAnimation = "Idle";

        bool m_isPlaying = false;
        bool m_isMaterialInitialized = false;
        FlippyAnimation m_currentAnim;
        float m_timeAccumulator = 0.0f;
        int m_currentFrame = 0;
    };
}