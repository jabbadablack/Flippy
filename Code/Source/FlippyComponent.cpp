#include "FlippyComponent.h"
#include <AZCore/Serialization/SerializeContext.h>
#include <AZCore/Serialization/EditContext.h>
#include <AZCore/Asset/AssetSerializer.h>
#include <AZCore/RTTI/BehaviorContext.h>
#include <AZCore/Debug/Trace.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>

namespace FlippyGem
{
    void FlippyComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ_Assert(context != nullptr, "Context cannot be null");
        AZ_Assert(FlippyComponentTypeId != nullptr, "Component TypeId missing");

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AnimationData>()
                ->Version(1)
                ->Field("Name", &AnimationData::m_name)
                ->Field("StartFrame", &AnimationData::m_startFrame)
                ->Field("EndFrame", &AnimationData::m_endFrame)
                ->Field("FPS", &AnimationData::m_fps);

            serializeContext->Class<FlippyComponent, AZ::Component>()
                ->Version(1)
                ->Field("SpriteSheet", &FlippyComponent::m_spriteSheet)
                ->Field("Columns", &FlippyComponent::m_columns)
                ->Field("Rows", &FlippyComponent::m_rows)
                ->Field("Animations", &FlippyComponent::m_animations);

            // --- NEW: Add the EditContext so the Editor can display the UI ---
            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FlippyComponent>("Flippy", "A custom 2D sprite animation component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Rendering") // Puts it in the Rendering category
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game")) // Makes it show up in the menu!
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_spriteSheet, "Sprite Sheet", "The sprite sheet image asset")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_columns, "Columns", "Number of columns in the sprite sheet")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_rows, "Rows", "Number of rows in the sprite sheet")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_animations, "Animations", "List of animations");

                editContext->Class<AnimationData>("Animation Data", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_name, "Name", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_startFrame, "Start Frame", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_endFrame, "End Frame", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_fps, "FPS", "");
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<FlippyComponentRequestBus>("FlippyComponentRequestBus")
                ->Event("PlayAnimation", &FlippyComponentRequestBus::Events::PlayAnimation)
                ->Event("StopAnimation", &FlippyComponentRequestBus::Events::StopAnimation)
                ->Event("SetPlaybackRate", &FlippyComponentRequestBus::Events::SetPlaybackRate)
                ->Event("SetFrame", &FlippyComponentRequestBus::Events::SetFrame);
        }
    }

    const AnimationData* FlippyComponent::GetAnimation(const AZStd::string& name) const
    {
        AZ_Assert(!name.empty(), "Requested animation name cannot be empty");
        AZ_Assert(m_animations.size() < 10000, "Sanity check: Animation list is unusually large");

        for (const auto& anim : m_animations)
        {
            if (anim.m_name == name)
            {
                return &anim;
            }
        }
        return nullptr;
    }

    void FlippyComponent::Init()
    {
        AZ_Assert(!m_isPlaying, "Component should not start in a playing state");
        AZ_Assert(m_currentFrame == 0, "Initial frame must be zero");
    }

    void FlippyComponent::Activate()
    {
        AZ_Assert(GetEntityId().IsValid(), "Component attached to invalid Entity");
        AZ_Assert(m_columns > 0, "Grid columns must be greater than zero");

        AZ::TickBus::Handler::BusConnect();
        FlippyComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void FlippyComponent::Deactivate()
    {
        AZ_Assert(GetEntityId().IsValid(), "Deactivating on invalid Entity");
        AZ_Assert(FlippyComponentRequestBus::Handler::BusIsConnected(), "Request bus was not connected");

        AZ::TickBus::Handler::BusDisconnect();
        FlippyComponentRequestBus::Handler::BusDisconnect();
    }

    void FlippyComponent::OnTick(float deltaTime, AZ::ScriptTimePoint /*time*/)
    {
        AZ_Assert(deltaTime >= 0.0f, "Delta time cannot be negative");
        AZ_Assert(m_playbackRate >= 0.0f, "Playback rate cannot be negative");

        if (!m_isPlaying || !m_currentAnimation)
        {
            return;
        }

        m_currentTime += deltaTime * m_playbackRate;
        float frameDuration = 1.0f / m_currentAnimation->m_fps;

        if (m_currentTime >= frameDuration)
        {
            m_currentTime -= frameDuration;
            m_currentFrame++;
            if (m_currentFrame > m_currentAnimation->m_endFrame)
            {
                m_currentFrame = m_currentAnimation->m_startFrame;
            }
            UpdateMaterialUVs();
        }
    }

    void FlippyComponent::UpdateMaterialUVs()
    {
        AZ_Assert(m_columns > 0, "Grid columns invalid");
        AZ_Assert(m_rows > 0, "Grid rows invalid");

        float scaleX = 1.0f / static_cast<float>(m_columns);
        float scaleY = 1.0f / static_cast<float>(m_rows);

        int column = m_currentFrame % m_columns;
        int row = m_currentFrame / m_columns;

        float offsetX = static_cast<float>(column) * scaleX;
        float offsetY = static_cast<float>(row) * scaleY;

        AZ::Render::MaterialAssignmentId defaultMaterialId;

        AZ::Render::MaterialComponentRequestBus::Event(
            GetEntityId(),
            &AZ::Render::MaterialComponentRequests::SetPropertyValue,
            defaultMaterialId,
            AZStd::string("uv_offset"),
            AZStd::any(AZ::Vector2(offsetX, offsetY)));

        AZ::Render::MaterialComponentRequestBus::Event(
            GetEntityId(),
            &AZ::Render::MaterialComponentRequests::SetPropertyValue,
            defaultMaterialId,
            AZStd::string("uv_scale"),
            AZStd::any(AZ::Vector2(scaleX, scaleY)));
    }

    void FlippyComponent::PlayAnimation(const AZStd::string& animationName)
    {
        AZ_Assert(!animationName.empty(), "Must provide a valid animation name");
        AZ_Assert(m_columns > 0, "Component not configured correctly");

        m_currentAnimation = GetAnimation(animationName);
        if (m_currentAnimation)
        {
            m_isPlaying = true;
            m_currentFrame = m_currentAnimation->m_startFrame;
            m_currentTime = 0.0f;
            UpdateMaterialUVs();
        }
    }

    void FlippyComponent::StopAnimation()
    {
        AZ_Assert(m_isPlaying == true || m_isPlaying == false, "State check");
        AZ_Assert(m_playbackRate >= 0.0f, "Rate must remain valid");

        m_isPlaying = false;
        m_currentFrame = 0;
        UpdateMaterialUVs();
    }

    void FlippyComponent::SetPlaybackRate(float rate)
    {
        AZ_Assert(rate >= 0.0f, "Negative playback rate invalid");
        AZ_Assert(m_columns > 0, "Valid configuration required");

        m_playbackRate = rate;
    }

    void FlippyComponent::SetFrame(int frameIndex)
    {
        AZ_Assert(frameIndex >= 0, "Frame index cannot be negative");
        AZ_Assert(m_columns > 0, "Valid configuration required");

        m_isPlaying = false;
        m_currentFrame = frameIndex;
        UpdateMaterialUVs();
    }
}