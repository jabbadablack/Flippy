#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialAssignment.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
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

    //! Groups a texture asset with its grid layout and specific animations.
    struct FlippySpriteSheet
    {
        AZ_TYPE_INFO(FlippySpriteSheet, FlippySpriteSheetTypeId);
        AZ_CLASS_ALLOCATOR(FlippySpriteSheet, AZ::SystemAllocator);

        AZStd::string m_sheetName = "New Sprite Sheet";
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_spriteAsset;
        int m_columns = 1;
        int m_rows = 1;
        AZStd::vector<FlippyAnimation> m_animations;
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

    //! Cycles through material UV offsets and texture maps to animate 2D sprite sheets.
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
        void SetupInitialVisualState();

        void SetMaterialTexture(const AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& spriteAsset);
        void ApplyMaterialScale(float tileU, float tileV);
        void ApplyMaterialOffset(float offsetU, float offsetV);

        void AdvanceFrame(float deltaTime);
        void RefreshMaterial();
        AZ::u32 OnEditorPropertiesChanged();

        // UI Visibility Helpers
        bool IsSingleMode() const;
        bool IsMultiMode() const;
        bool IsDefaultAnimationEnabled() const;

        AZ::EntityId m_materialEntityId;

        // Hidden UV properties
        AZStd::string m_uvTileUProperty = "uv.tileU";
        AZStd::string m_uvTileVProperty = "uv.tileV";
        AZStd::string m_uvOffsetUProperty = "uv.offsetU";
        AZStd::string m_uvOffsetVProperty = "uv.offsetV";

        // Toggles
        bool m_previewInEditor = true;
        bool m_useMultipleSpriteSheets = false;
        bool m_enableDefaultAnimation = true;
        AZStd::string m_defaultAnimation = "";

        // Single Mode Data
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_singleSpriteAsset;
        int m_singleColumns = 1;
        int m_singleRows = 1;
        AZStd::vector<FlippyAnimation> m_singleAnimations;

        // Multi Mode Data
        AZStd::vector<FlippySpriteSheet> m_spriteSheets;

        // Runtime State
        bool m_isPlaying = false;
        FlippyAnimation m_currentAnim;
        float m_timeAccumulator = 0.0f;
        int m_currentFrame = 0;
        int m_activeColumns = 1;
        int m_activeRows = 1;
        AZ::Data::AssetId m_activeSpriteId;

        bool m_isGameActive = false;
        std::chrono::steady_clock::time_point m_lastSystemTickTime;
        std::chrono::steady_clock::time_point m_lastGameTickTime;

        bool m_isMaterialInitialized = false;
        float m_lastTileU = -1.0f;
        float m_lastTileV = -1.0f;
    };
} // namespace FlippyGem