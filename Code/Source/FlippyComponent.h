#pragma once
#include <AZCore/Component/Component.h>
#include <AZCore/Component/TickBus.h>
#include <AZCore/std/string/string.h>
#include <AZCore/std/containers/vector.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include "FlippyComponentBus.h"
#include <FlippyGem/FlippyGemTypeIds.h>

namespace FlippyGem
{
    struct AnimationData
    {
        AZ_TYPE_INFO(AnimationData, AnimationDataTypeId);
        AZ_CLASS_ALLOCATOR(AnimationData, AZ::SystemAllocator, 0);

        AZStd::string m_name;
        int m_startFrame = 0;
        int m_endFrame = 0;
        float m_fps = 12.0f;
    };

    class FlippyComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public FlippyComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(FlippyComponent, FlippyComponentTypeId);

        static void Reflect(AZ::ReflectContext* context);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint /*time*/) override;

        void PlayAnimation(const AZStd::string& animationName) override;
        void StopAnimation() override;
        void SetPlaybackRate(float rate) override;
        void SetFrame(int frameIndex) override;

    private:
        void UpdateMaterialUVs();
        const AnimationData* GetAnimation(const AZStd::string& name) const;

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_spriteSheet;
        int m_columns = 1;
        int m_rows = 1;
        AZStd::vector<AnimationData> m_animations;

        bool m_isPlaying = false;
        float m_currentTime = 0.0f;
        int m_currentFrame = 0;
        float m_playbackRate = 1.0f;
        const AnimationData* m_currentAnimation = nullptr;
    };
}