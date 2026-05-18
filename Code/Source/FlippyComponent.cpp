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
        AZ_Assert(AZ::Environment::GetInstance() != nullptr, "O3DE Environment is not fully initialized.");

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlippyAnimation>()
                ->Version(2)
                ->Field("Name", &FlippyAnimation::m_name)
                ->Field("StartRow", &FlippyAnimation::m_startRow)
                ->Field("StartColumn", &FlippyAnimation::m_startColumn)
                ->Field("FrameCount", &FlippyAnimation::m_frameCount)
                ->Field("FPS", &FlippyAnimation::m_fps);

            serializeContext->Class<FlippyComponent, AZ::Component>()
                ->Version(2)
                ->Field("MaterialEntity", &FlippyComponent::m_materialEntityId)
                ->Field("UVTileUProperty", &FlippyComponent::m_uvTileUProperty)
                ->Field("UVTileVProperty", &FlippyComponent::m_uvTileVProperty)
                ->Field("UVOffsetUProperty", &FlippyComponent::m_uvOffsetUProperty)
                ->Field("UVOffsetVProperty", &FlippyComponent::m_uvOffsetVProperty)
                ->Field("Columns", &FlippyComponent::m_columns)
                ->Field("Rows", &FlippyComponent::m_rows)
                ->Field("DefaultAnimation", &FlippyComponent::m_defaultAnimation)
                ->Field("Animations", &FlippyComponent::m_animations);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FlippyAnimation>("Animation State", "A single animation sequence")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_name, "Name", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_startRow, "Start Row", "Row index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_startColumn, "Start Column", "Column index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_frameCount, "Frame Count", "Total frames in this animation")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_fps, "FPS", "");

                editContext->Class<FlippyComponent>("Flippy Animator", "Plays specific animations from a spritesheet.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Rendering")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Grid Settings")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_columns, "Columns", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_rows, "Rows", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Material Integration")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_materialEntityId, "Target Entity", "Leave blank to target this entity.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvTileUProperty, "Tile U String", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvTileVProperty, "Tile V String", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvOffsetUProperty, "Offset U String", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_uvOffsetVProperty, "Offset V String", "")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Animations")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_defaultAnimation, "Default State", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_animations, "Animation List", "");
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
        AZ_Assert(GetEntityId().IsValid(), "Entity ID is invalid during FlippyComponent::Init!");
        AZ_Assert(GetEntity() != nullptr, "Entity pointer is null during FlippyComponent::Init!");
    }

    void FlippyComponent::Activate()
    {
        AZ_Assert(GetEntityId().IsValid(), "Entity ID is invalid during FlippyComponent::Activate!");
        AZ_Assert(m_columns > 0 && m_rows > 0, "Grid columns and rows must be strictly greater than zero to safely activate!");

        m_isMaterialInitialized = false;

        FlippyComponentRequestBus::Handler::BusConnect(GetEntityId());
        PlayAnimation(m_defaultAnimation);
        AZ::TickBus::Handler::BusConnect();
    }

    void FlippyComponent::Deactivate()
    {
        AZ_Assert(GetEntityId().IsValid(), "Entity ID is invalid during FlippyComponent::Deactivate!");
        AZ_Assert(GetEntity() != nullptr, "Entity pointer is null during FlippyComponent::Deactivate!");

        AZ::TickBus::Handler::BusDisconnect();
        FlippyComponentRequestBus::Handler::BusDisconnect();
    }

    void FlippyComponent::PlayAnimation(const AZStd::string& animationName)
    {
        AZ_Assert(!animationName.empty(), "Animation name passed to PlayAnimation cannot be empty!");
        AZ_Assert(m_columns > 0 && m_rows > 0, "Grid columns and rows must be valid before attempting to calculate frame indexes!");

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
        AZ_Assert(GetEntityId().IsValid(), "Entity ID is invalid during FlippyComponent::StopAnimation!");
        AZ_Assert(GetEntity() != nullptr, "Entity pointer is null during FlippyComponent::StopAnimation!");

        m_isPlaying = false;
    }

    AZStd::vector<AZ::Render::MaterialAssignmentId> FlippyComponent::GetActiveMaterialIds(AZ::EntityId targetEntity)
    {
        AZ_Assert(targetEntity.IsValid(), "Target entity passed to GetActiveMaterialIds is invalid!");
        AZ_Assert(AZ::Render::MaterialComponentRequestBus::HasHandlers(targetEntity), "Target entity does not have a connected MaterialComponentRequestBus!");

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
        AZ_Assert(tileU > 0.0f, "Tile U scale calculated as zero or negative!");
        AZ_Assert(tileV > 0.0f, "Tile V scale calculated as zero or negative!");

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
        AZ_Assert(!m_uvOffsetUProperty.empty(), "UV Offset U property string is empty! Material update will fail.");
        AZ_Assert(!m_uvOffsetVProperty.empty(), "UV Offset V property string is empty! Material update will fail.");

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

    void FlippyComponent::OnTick(float deltaTime, AZ::ScriptTimePoint /*time*/)
    {
        AZ_Assert(deltaTime >= 0.0f, "Delta time in OnTick is negative!");

        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();

        if (!AZ::Render::MaterialComponentRequestBus::HasHandlers(targetEntity))
        {
            return;
        }

        if (!m_isMaterialInitialized)
        {
            if (m_columns > 0 && m_rows > 0)
            {
                float tileU = 1.0f / static_cast<float>(m_columns);
                float tileV = 1.0f / static_cast<float>(m_rows);
                ApplyMaterialScale(tileU, tileV);
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

            if (m_currentFrame > globalEndFrame)
            {
                m_currentFrame = globalStartFrame;
            }

            float scaleX = 1.0f / static_cast<float>(m_columns);
            float scaleY = 1.0f / static_cast<float>(m_rows);

            int currentColumn = m_currentFrame % m_columns;
            int currentRow = m_currentFrame / m_columns;

            float offsetX = currentColumn * scaleX;
            float offsetY = currentRow * scaleY;

            ApplyMaterialOffset(offsetX, offsetY);
        }
    }
}