#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialAssignment.h>
#include <chrono>

#include "FlippyComponentBus.h"

namespace FlippyGem
{
    struct AnimationData
    {
        AZ_TYPE_INFO(AnimationData, "{7F13876D-29A8-4D2A-B6A9-D3CA9B3DE44A}");
        AZ_CLASS_ALLOCATOR(AnimationData, AZ::SystemAllocator);

        AZStd::string m_name = "Idle";
        int m_startRow = 0;
        int m_startColumn = 0;
        int m_frameCount = 1;
        float m_fps = 12.0f;
    };

    class FlippyComponent
        : public AZ::Component
        , protected AZ::TickBus::Handler
        , protected AZ::SystemTickBus::Handler // Required for Editor Preview
        , public FlippyComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FlippyComponent, "{D7E2B06F-8D15-4D1C-A1A8-D0623DE211CA}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

    protected:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        void OnSystemTick() override;

        void PlayAnimation(const AZStd::string& animationName) override;
        void StopAnimation() override;

    private:
        AZStd::vector<AZ::Render::MaterialAssignmentId> GetActiveMaterialIds(AZ::EntityId targetEntity);

        void ApplyMaterialScale(float tileU, float tileV);
        void ApplyMaterialOffset(float offsetU, float offsetV);
        void AdvanceFrame(float deltaTime);
        AZ::u32 OnEditorPropertiesChanged();

        AZ::EntityId m_materialEntityId;

        AZStd::string m_uvTileUProperty = "uv.tileU";
        AZStd::string m_uvTileVProperty = "uv.tileV";
        AZStd::string m_uvOffsetUProperty = "uv.offsetU";
        AZStd::string m_uvOffsetVProperty = "uv.offsetV";

        int m_columns = 1;
        int m_rows = 1;
        bool m_previewInEditor = true;

        AZStd::vector<AnimationData> m_animations;
        AZStd::string m_defaultAnimation = "Idle";

        bool m_isPlaying = false;
        bool m_isMaterialInitialized = false;
        AnimationData m_currentAnim;
        float m_timeAccumulator = 0.0f;
        int m_currentFrame = 0;

        // Editor preview timing
        bool m_isGameTicking = false;
        std::chrono::steady_clock::time_point m_lastSystemTickTime;
    };
}