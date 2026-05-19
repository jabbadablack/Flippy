#include "FlippyComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h> 
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/std/any.h>
#include <AzCore/Debug/Trace.h> 

namespace FlippyGem
{
    void FlippyComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ_Assert(context != nullptr, "ReflectContext is null! Cannot reflect FlippyComponent.");

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlippyAnimation>()
                ->Version(3)
                ->Field("Name", &FlippyAnimation::m_name)
                ->Field("StartRow", &FlippyAnimation::m_startRow)
                ->Field("StartColumn", &FlippyAnimation::m_startColumn)
                ->Field("FrameCount", &FlippyAnimation::m_frameCount)
                ->Field("FPS", &FlippyAnimation::m_fps)
                ->Field("PlayBackwards", &FlippyAnimation::m_playBackwards);

            serializeContext->Class<FlippyComponent, AZ::Component>()
                ->Version(3)
                ->Field("MaterialEntity", &FlippyComponent::m_materialEntityId)
                ->Field("Columns", &FlippyComponent::m_columns)
                ->Field("Rows", &FlippyComponent::m_rows)
                ->Field("DefaultAnimation", &FlippyComponent::m_defaultAnimation)
                ->Field("Animations", &FlippyComponent::m_animations);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FlippyAnimation>("Animation State", "A single animation sequence")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_name, "Name", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_startRow, "Start Column", "Column index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_startColumn, "Start Row", "Row index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_frameCount, "Frame Count", "Total frames in this animation")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_fps, "FPS", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_playBackwards, "Play Backwards", "Plays the animation sequence in reverse");

                editContext->Class<FlippyComponent>("Flippy", "Cycles through material textures to animate them.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Rendering")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Grid Settings")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_columns, "Rows (Horizontal)", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_rows, "Columns (Vertical)", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Material Integration")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_materialEntityId, "Target Entity", "Leave blank to target this entity.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Animations")
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

    FlippyComponent::~FlippyComponent()
    {
        AZ::SystemTickBus::Handler::BusDisconnect();
    }

    void FlippyComponent::Init()
    {
        if (!AZ::SystemTickBus::Handler::BusIsConnected())
        {
            AZ::SystemTickBus::Handler::BusConnect();
        }
        m_lastSystemTickTime = std::chrono::steady_clock::now();
    }

    void FlippyComponent::Activate()
    {
        FlippyComponentRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        AZ::SystemTickBus::Handler::BusConnect();

        m_lastSystemTickTime = std::chrono::steady_clock::now();
        m_isGameActive = false;
        m_isMaterialInitialized = false;

        PlayAnimation(m_defaultAnimation);
    }

    void FlippyComponent::Deactivate()
    {
        m_isGameActive = false;

        AZ::TickBus::Handler::BusDisconnect();
        FlippyComponentRequestBus::Handler::BusDisconnect();
    }

    void FlippyComponent::PlayAnimation(const AZStd::string& animationName)
    {
        if (m_columns <= 0 || m_rows <= 0 || m_animations.empty()) return;

        const FlippyAnimation* targetAnim = nullptr;

        for (const auto& anim : m_animations)
        {
            if (anim.m_name == animationName)
            {
                targetAnim = &anim;
                break;
            }
        }

        if (!targetAnim)
        {
            targetAnim = &m_animations[0];
        }

        if (m_isPlaying && m_currentAnim.m_name == targetAnim->m_name)
        {
            m_currentAnim = *targetAnim;
            return;
        }

        m_currentAnim = *targetAnim;

        int startFrameCalc = (m_currentAnim.m_startRow * m_columns) + m_currentAnim.m_startColumn;
        m_currentFrame = m_currentAnim.m_playBackwards ? (startFrameCalc + m_currentAnim.m_frameCount - 1) : startFrameCalc;

        m_timeAccumulator = 0.0f;
        m_isPlaying = true;
        RefreshMaterial();
    }

    void FlippyComponent::StopAnimation()
    {
        m_isPlaying = false;
    }

    AZ::u32 FlippyComponent::OnEditorPropertiesChanged()
    {
        m_isMaterialInitialized = false;

        StopAnimation();

        m_currentFrame = 0;
        for (const auto& anim : m_animations)
        {
            if (anim.m_name == m_defaultAnimation)
            {
                int startFrameCalc = (anim.m_startRow * m_columns) + anim.m_startColumn;
                m_currentFrame = anim.m_playBackwards ? (startFrameCalc + anim.m_frameCount - 1) : startFrameCalc;
                break;
            }
        }
        RefreshMaterial();

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void FlippyComponent::RefreshMaterial()
    {
        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();

        if (!AZ::Render::MaterialComponentRequestBus::HasHandlers(targetEntity) || m_columns <= 0 || m_rows <= 0)
        {
            return;
        }

        float tileU = 1.0f / static_cast<float>(m_columns);
        float tileV = 1.0f / static_cast<float>(m_rows);

        if (!m_isMaterialInitialized || m_lastTileU != tileU || m_lastTileV != tileV)
        {
            ApplyMaterialScale(tileU, tileV);
            m_lastTileU = tileU;
            m_lastTileV = tileV;
            m_isMaterialInitialized = true;
        }

        int currentColumn = m_currentFrame % m_columns;
        int currentRow = m_currentFrame / m_columns;

        float offsetX = static_cast<float>(currentColumn) * tileU;
        float offsetY = static_cast<float>(currentRow) * tileV;

        ApplyMaterialOffset(offsetX, offsetY);
    }

    void FlippyComponent::AdvanceFrame(float deltaTime)
    {
        if (!m_isPlaying || m_columns <= 0 || m_rows <= 0 || m_currentAnim.m_frameCount <= 0 || m_currentAnim.m_fps <= 0.0f) return;

        int globalStartFrame = (m_currentAnim.m_startRow * m_columns) + m_currentAnim.m_startColumn;
        int globalEndFrame = globalStartFrame + m_currentAnim.m_frameCount - 1;

        if (m_currentFrame < globalStartFrame || m_currentFrame > globalEndFrame)
        {
            m_currentFrame = m_currentAnim.m_playBackwards ? globalEndFrame : globalStartFrame;
        }

        m_timeAccumulator += deltaTime;
        float frameDuration = 1.0f / m_currentAnim.m_fps;

        if (m_timeAccumulator >= frameDuration)
        {
            int framesToAdvance = static_cast<int>(m_timeAccumulator / frameDuration);
            m_timeAccumulator -= framesToAdvance * frameDuration;

            if (m_currentAnim.m_playBackwards)
            {
                m_currentFrame -= framesToAdvance;

                if (m_currentFrame < globalStartFrame)
                {
                    int under = globalStartFrame - m_currentFrame - 1;
                    m_currentFrame = globalEndFrame - (under % m_currentAnim.m_frameCount);
                }
            }
            else
            {
                m_currentFrame += framesToAdvance;

                if (m_currentFrame > globalEndFrame)
                {
                    int over = m_currentFrame - globalStartFrame;
                    m_currentFrame = globalStartFrame + (over % m_currentAnim.m_frameCount);
                }
            }

            RefreshMaterial();
        }
    }

    void FlippyComponent::OnTick(float deltaTime, AZ::ScriptTimePoint /*time*/)
    {
        if (deltaTime <= 0.0f) return;

        if (!m_isPlaying)
        {
            PlayAnimation(m_defaultAnimation);
        }

        m_isGameActive = true;
        AdvanceFrame(deltaTime);
    }

    void FlippyComponent::OnSystemTick()
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - m_lastSystemTickTime;
        m_lastSystemTickTime = now;

        if (m_isGameActive) return;

        AdvanceFrame(delta.count());
    }

    AZStd::vector<AZ::Render::MaterialAssignmentId> FlippyComponent::GetActiveMaterialIds(AZ::EntityId targetEntity)
    {
        AZStd::vector<AZ::Render::MaterialAssignmentId> ids;
        AZ::Render::MaterialAssignmentMap materialMap;

        AZ::Render::MaterialComponentRequestBus::EventResult(
            materialMap, targetEntity, &AZ::Render::MaterialComponentRequests::GetMaterialMap);

        if (materialMap.empty())
        {
            AZ::Render::MaterialComponentRequestBus::EventResult(
                materialMap, targetEntity, &AZ::Render::MaterialComponentRequests::GetDefaultMaterialMap);
        }

        for (const auto& pair : materialMap)
        {
            ids.push_back(pair.first);
        }

        if (ids.empty())
        {
            ids.push_back(AZ::Render::MaterialAssignmentId());
        }
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
        if (m_uvOffsetUProperty.empty() || m_uvOffsetVProperty.empty()) return;

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
        provided.push_back(AZ_CRC_CE("FlippyAnimatorService"));
    }

    void FlippyComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("FlippyAnimatorService"));
    }

    void FlippyComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("MaterialService"));
    }
}