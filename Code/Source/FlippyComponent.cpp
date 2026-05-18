#include "FlippyComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h> 
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/std/any.h>
#include <AzCore/Debug/Trace.h> 
#include <AzCore/Module/Environment.h>

namespace FlippyGem
{
    void FlippyComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ_Assert(context != nullptr, "ReflectContext is null! Cannot reflect FlippyComponent.");

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AnimationData>()
                ->Version(1)
                ->Field("Name", &AnimationData::m_name)
                ->Field("StartRow", &AnimationData::m_startRow)
                ->Field("StartColumn", &AnimationData::m_startColumn)
                ->Field("FrameCount", &AnimationData::m_frameCount)
                ->Field("FPS", &AnimationData::m_fps);

            serializeContext->Class<FlippyComponent, AZ::Component>()
                ->Version(1)
                ->Field("MaterialEntity", &FlippyComponent::m_materialEntityId)
                ->Field("UVTileUProperty", &FlippyComponent::m_uvTileUProperty)
                ->Field("UVTileVProperty", &FlippyComponent::m_uvTileVProperty)
                ->Field("UVOffsetUProperty", &FlippyComponent::m_uvOffsetUProperty)
                ->Field("UVOffsetVProperty", &FlippyComponent::m_uvOffsetVProperty)
                ->Field("Columns", &FlippyComponent::m_columns)
                ->Field("Rows", &FlippyComponent::m_rows)
                ->Field("PreviewInEditor", &FlippyComponent::m_previewInEditor)
                ->Field("DefaultAnimation", &FlippyComponent::m_defaultAnimation)
                ->Field("Animations", &FlippyComponent::m_animations);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<AnimationData>("Animation State", "A single animation sequence")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_name, "Name", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_startRow, "Start Row", "Row index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_startColumn, "Start Column", "Column index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_frameCount, "Frame Count", "Total frames in this animation")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AnimationData::m_fps, "FPS", "");

                editContext->Class<FlippyComponent>("Flippy Animator", "Plays specific animations from a spritesheet.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Rendering")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Grid Settings")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_columns, "Columns", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_rows, "Rows", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Material Integration")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_materialEntityId, "Target Entity", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvTileUProperty, "Tile U String", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvTileVProperty, "Tile V String", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvOffsetUProperty, "Offset U String", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvOffsetVProperty, "Offset V String", "")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Animations")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_previewInEditor, "Preview in Editor", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_defaultAnimation, "Default State", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_animations, "Animation List", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged);
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<FlippyComponentRequestBus>("FlippyComponentRequestBus")
                ->Event("PlayAnimation", &FlippyComponentRequestBus::Events::PlayAnimation)
                ->Event("StopAnimation", &FlippyComponentRequestBus::Events::StopAnimation);
        }
    }

    void FlippyComponent::Init()
    {
    }

    void FlippyComponent::Activate()
    {
        AZ_Assert(GetEntityId().IsValid(), "Entity ID is invalid!");

        m_isMaterialInitialized = false;
        m_isGameTicking = false;
        m_lastSystemTickTime = std::chrono::steady_clock::now();

        FlippyComponentRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        AZ::SystemTickBus::Handler::BusConnect();

        OnEditorPropertiesChanged(); // Trigger initial state
    }

    void FlippyComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        AZ::SystemTickBus::Handler::BusDisconnect();
        FlippyComponentRequestBus::Handler::BusDisconnect();
    }

    void FlippyComponent::PlayAnimation(const AZStd::string& animationName)
    {
        if (m_columns <= 0 || m_rows <= 0) return;

        for (const auto& anim : m_animations)
        {
            if (anim.m_name == animationName)
            {
                m_currentAnim = anim;
                m_currentFrame = (anim.m_startRow * m_columns) + anim.m_startColumn;
                m_timeAccumulator = 0.0f;
                m_isPlaying = true;
                return;
            }
        }
    }

    void FlippyComponent::StopAnimation()
    {
        m_isPlaying = false;
    }

    AZStd::vector<AZ::Render::MaterialAssignmentId> FlippyComponent::GetActiveMaterialIds(AZ::EntityId targetEntity)
    {
        AZStd::vector<AZ::Render::MaterialAssignmentId> ids;
        if (!targetEntity.IsValid() || !AZ::Render::MaterialComponentRequestBus::HasHandlers(targetEntity)) return ids;

        AZ::Render::MaterialAssignmentMap materialMap;
        AZ::Render::MaterialComponentRequestBus::EventResult(
            materialMap, targetEntity, &AZ::Render::MaterialComponentRequests::GetMaterialMap);

        if (materialMap.empty())
        {
            AZ::Render::MaterialComponentRequestBus::EventResult(
                materialMap, targetEntity, &AZ::Render::MaterialComponentRequests::GetDefaultMaterialMap);
        }

        for (const auto& pair : materialMap) { ids.push_back(pair.first); }
        if (ids.empty()) { ids.push_back(AZ::Render::MaterialAssignmentId()); }

        return ids;
    }

    void FlippyComponent::ApplyMaterialScale(float tileU, float tileV)
    {
        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();
        auto materialIds = GetActiveMaterialIds(targetEntity);

        for (const auto& id : materialIds)
        {
            AZ::Render::MaterialComponentRequestBus::Event(
                targetEntity, &AZ::Render::MaterialComponentRequests::SetPropertyValue,
                id, m_uvTileUProperty, AZStd::make_any<float>(tileU));
            AZ::Render::MaterialComponentRequestBus::Event(
                targetEntity, &AZ::Render::MaterialComponentRequests::SetPropertyValue,
                id, m_uvTileVProperty, AZStd::make_any<float>(tileV));
        }
    }

    void FlippyComponent::ApplyMaterialOffset(float offsetU, float offsetV)
    {
        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();
        auto materialIds = GetActiveMaterialIds(targetEntity);

        for (const auto& id : materialIds)
        {
            AZ::Render::MaterialComponentRequestBus::Event(
                targetEntity, &AZ::Render::MaterialComponentRequests::SetPropertyValue,
                id, m_uvOffsetUProperty, AZStd::make_any<float>(offsetU));
            AZ::Render::MaterialComponentRequestBus::Event(
                targetEntity, &AZ::Render::MaterialComponentRequests::SetPropertyValue,
                id, m_uvOffsetVProperty, AZStd::make_any<float>(offsetV));
        }
    }

    void FlippyComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FlippyService"));
    }

    void FlippyComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FlippyService"));
    }

    void FlippyComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("MaterialService"));
    }

    void FlippyComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        m_isGameTicking = true; // Tell the Editor tick to step back
        AdvanceFrame(deltaTime);
    }

    void FlippyComponent::OnSystemTick()
    {
        if (m_isGameTicking || !m_previewInEditor) return;

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - m_lastSystemTickTime;
        m_lastSystemTickTime = now;

        AdvanceFrame(delta.count());
    }

    void FlippyComponent::AdvanceFrame(float deltaTime)
    {
        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();
        if (!AZ::Render::MaterialComponentRequestBus::HasHandlers(targetEntity)) return;

        if (!m_isMaterialInitialized)
        {
            if (m_columns > 0 && m_rows > 0)
            {
                ApplyMaterialScale(1.0f / static_cast<float>(m_columns), 1.0f / static_cast<float>(m_rows));
            }
            m_isMaterialInitialized = true;
        }

        if (!m_isPlaying || m_columns <= 0 || m_rows <= 0 || m_currentAnim.m_frameCount <= 0 || m_currentAnim.m_fps <= 0.0f) return;

        m_timeAccumulator += deltaTime;
        float frameDuration = 1.0f / m_currentAnim.m_fps;

        if (m_timeAccumulator >= frameDuration)
        {
            m_timeAccumulator -= frameDuration;
            m_currentFrame++;

            int globalStartFrame = (m_currentAnim.m_startRow * m_columns) + m_currentAnim.m_startColumn;
            int globalEndFrame = globalStartFrame + m_currentAnim.m_frameCount - 1;

            if (m_currentFrame > globalEndFrame || m_currentFrame < globalStartFrame)
            {
                m_currentFrame = globalStartFrame;
            }

            float scaleX = 1.0f / static_cast<float>(m_columns);
            float scaleY = 1.0f / static_cast<float>(m_rows);

            float offsetX = (m_currentFrame % m_columns) * scaleX;
            float offsetY = (m_currentFrame / m_columns) * scaleY;

            ApplyMaterialOffset(offsetX, offsetY);
        }
    }

    AZ::u32 FlippyComponent::OnEditorPropertiesChanged()
    {
        m_isMaterialInitialized = false; // Force a scale recalculation

        if (m_previewInEditor)
        {
            PlayAnimation(m_defaultAnimation);
        }
        else
        {
            StopAnimation();
        }
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }
}