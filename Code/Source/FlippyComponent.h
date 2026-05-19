#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialAssignment.h>
#include <chrono>

namespace FlippyGem
{
    struct FlippyAnimation
    {
        AZ_TYPE_INFO(FlippyAnimation, "{6230413b-d706-4dd4-a104-ee6ac766709a}");
        AZ_CLASS_ALLOCATOR(FlippyAnimation, AZ::SystemAllocator);

        AZStd::string m_name = "";
        int m_startRow = 0;
        int m_startColumn = 0;
        int m_frameCount = 1;
        float m_fps = 12.0f;
    };

    class FlippyComponentRequests : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(FlippyComponentRequests, "{b4150cdb-7d4d-4933-aebc-14800ae75826}");
        virtual ~FlippyComponentRequests() = default;

        virtual void PlayAnimation(const AZStd::string& animationName) = 0;
        virtual void StopAnimation() = 0;
    };
    using FlippyComponentRequestBus = AZ::EBus<FlippyComponentRequests>;

    class FlippyComponent
        : public AZ::Component
        , protected AZ::TickBus::Handler
        , protected AZ::SystemTickBus::Handler
        , public FlippyComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FlippyComponent, "{8fa5e056-1605-41db-82ec-8bccb23f2f55}");

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
        AZStd::vector<AZ::Render::MaterialAssignmentId> GetActiveMaterialIds(AZ::EntityId targetEntity);

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

        AZStd::vector<FlippyAnimation> m_animations = { FlippyAnimation() };
        AZStd::string m_defaultAnimation = "";

        // Runtime State
        bool m_isPlaying = false;
        FlippyAnimation m_currentAnim;
        float m_timeAccumulator = 0.0f;
        int m_currentFrame = 0;

        // Tick Management
        bool m_isGameActive = false;
        std::chrono::steady_clock::time_point m_lastSystemTickTime;

        // Material caching variables
        bool m_isMaterialInitialized = false;
        float m_lastTileU = -1.0f;
        float m_lastTileV = -1.0f;
    };
}