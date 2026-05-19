#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialAssignment.h>
#include <chrono>
#include <FlippyGem/FlippyGemTypeIds.h>

namespace FlippyGem
{
    //! Defines a single animation sequence extracted from a spritesheet.
    struct FlippyAnimation
    {
        AZ_TYPE_INFO(FlippyAnimation, FlippyAnimationTypeId);
        AZ_CLASS_ALLOCATOR(FlippyAnimation, AZ::SystemAllocator);

        AZStd::string m_name = "";
        int m_startColumn = 0;
        int m_startRow = 0;
        int m_frameCount = 1;
        float m_fps = 12.0f;
        bool m_playBackwards = false;
    };

    //! Bus interface for external scripts or components to control the Flippy Animator.
    class FlippyComponentRequests : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(FlippyComponentRequests, FlippyGemRequestsTypeId);
        virtual ~FlippyComponentRequests() = default;

        virtual void PlayAnimation(const AZStd::string& animationName) = 0;
        virtual void StopAnimation() = 0;
    };
    using FlippyComponentRequestBus = AZ::EBus<FlippyComponentRequests>;

    //! Cycles through material UV offsets to animate 2D sprite sheets.
    class FlippyComponent
        : public AZ::Component
        , protected AZ::TickBus::Handler
        , protected AZ::SystemTickBus::Handler
        , public FlippyComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FlippyComponent, FlippyComponentTypeId);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        ~FlippyComponent() override;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

    protected:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        void OnSystemTick() override;

        void PlayAnimation(const AZStd::string& animationName) override;
        void StopAnimation() override;

    private:
        AZStd::vector<AZ::Render::MaterialAssignmentId> GetActiveMaterialIds(AZ::EntityId targetEntity) const;
        int GetStartFrameForAnimation(const AZStd::string& name) const;

        void ApplyMaterialScale(float tileU, float tileV);
        void ApplyMaterialOffset(float offsetU, float offsetV);

        void AdvanceFrame(float deltaTime);
        void RefreshMaterial();
        AZ::u32 OnEditorPropertiesChanged();

        AZ::EntityId m_materialEntityId;

        AZStd::string m_uvTileUProperty = "uv.tileU";
        AZStd::string m_uvTileVProperty = "uv.tileV";
        AZStd::string m_uvOffsetUProperty = "uv.offsetU";
        AZStd::string m_uvOffsetVProperty = "uv.offsetV";

        int m_columns = 1;
        int m_rows = 1;

        AZStd::vector<FlippyAnimation> m_animations;
        AZStd::string m_defaultAnimation = "";

        bool m_isPlaying = false;
        FlippyAnimation m_currentAnim;
        float m_timeAccumulator = 0.0f;
        int m_currentFrame = 0;

        bool m_isGameActive = false;
        std::chrono::steady_clock::time_point m_lastSystemTickTime;

        bool m_isMaterialInitialized = false;
        float m_lastTileU = -1.0f;
        float m_lastTileV = -1.0f;
    };
} // namespace FlippyGem