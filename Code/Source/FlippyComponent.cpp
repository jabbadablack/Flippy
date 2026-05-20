#include "FlippyComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h> 
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/std/any.h>
#include <AzCore/Debug/Trace.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AzCore/Asset/AssetSerializer.h>

namespace FlippyGem
{
    void FlippyComponent::Reflect(AZ::ReflectContext* context)
    {
        AZ_Assert(context != nullptr, "ReflectContext is null! Cannot reflect FlippyComponent.");

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlippyAnimation>()
                ->Version(5)
                ->Field("Name", &FlippyAnimation::m_name)
                ->Field("StartColumn", &FlippyAnimation::m_startColumn)
                ->Field("StartRow", &FlippyAnimation::m_startRow)
                ->Field("FrameCount", &FlippyAnimation::m_frameCount)
                ->Field("FPS", &FlippyAnimation::m_fps)
                ->Field("PlayBackwards", &FlippyAnimation::m_playBackwards);

            serializeContext->Class<FlippySpriteSheet>()
                ->Version(1)
                ->Field("SheetName", &FlippySpriteSheet::m_sheetName)
                ->Field("SpriteAsset", &FlippySpriteSheet::m_spriteAsset)
                ->Field("Columns", &FlippySpriteSheet::m_columns)
                ->Field("Rows", &FlippySpriteSheet::m_rows)
                ->Field("Animations", &FlippySpriteSheet::m_animations);

            serializeContext->Class<FlippyComponent, AZ::Component>()
                ->Version(5)
                ->Field("MaterialEntity", &FlippyComponent::m_materialEntityId)
                ->Field("UVTileUProperty", &FlippyComponent::m_uvTileUProperty)
                ->Field("UVTileVProperty", &FlippyComponent::m_uvTileVProperty)
                ->Field("UVOffsetUProperty", &FlippyComponent::m_uvOffsetUProperty)
                ->Field("UVOffsetVProperty", &FlippyComponent::m_uvOffsetVProperty)
                ->Field("PreviewInEditor", &FlippyComponent::m_previewInEditor)
                ->Field("UseMultipleSpriteSheets", &FlippyComponent::m_useMultipleSpriteSheets)
                ->Field("EnableDefaultAnimation", &FlippyComponent::m_enableDefaultAnimation)
                ->Field("DefaultAnimation", &FlippyComponent::m_defaultAnimation)
                ->Field("SingleSpriteAsset", &FlippyComponent::m_singleSpriteAsset)
                ->Field("SingleColumns", &FlippyComponent::m_singleColumns)
                ->Field("SingleRows", &FlippyComponent::m_singleRows)
                ->Field("SingleAnimations", &FlippyComponent::m_singleAnimations)
                ->Field("SpriteSheets", &FlippyComponent::m_spriteSheets);

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FlippyAnimation>("Animation State", "A single animation sequence")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_name, "Name", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_startColumn, "Start Column", "Column index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_startRow, "Start Row", "Row index (Starts at 0)")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_frameCount, "Frame Count", "Total frames in this animation")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_fps, "FPS", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyAnimation::m_playBackwards, "Play Backwards", "Plays the animation sequence in reverse");

                editContext->Class<FlippySpriteSheet>("Sprite Sheet", "A texture containing an animation grid.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippySpriteSheet::m_sheetName, "Sheet Name", "Identifier for this spritesheet")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippySpriteSheet::m_spriteAsset, "Sprite Asset", "The image file to feed to the material")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippySpriteSheet::m_columns, "Columns", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippySpriteSheet::m_rows, "Rows", "")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippySpriteSheet::m_animations, "Animations", "");

                editContext->Class<FlippyComponent>("Flippy Animator", "Plays specific animations from a spritesheet.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Rendering")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Component Settings")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_materialEntityId, "Target Entity", "Leave blank to target this entity.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_previewInEditor, "Preview In Editor", "Play the Animation in editor")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_useMultipleSpriteSheets, "Use Multiple Sprite Sheets", "Toggle arrayed list of images")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Default Animation Behavior")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_enableDefaultAnimation, "Enable Default Animation", "Auto-play an animation on startup")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_defaultAnimation, "Default State", "Name of the animation to auto-play")
                    ->Attribute(AZ::Edit::Attributes::Visibility, &FlippyComponent::IsDefaultAnimationEnabled)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Single Sprite Configuration")
                    ->Attribute(AZ::Edit::Attributes::Visibility, &FlippyComponent::IsSingleMode)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_singleSpriteAsset, "Sprite Asset", "The image file to feed to the material")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_singleColumns, "Columns", "Number of vertical columns")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_singleRows, "Rows", "Number of horizontal rows")
                    ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_singleAnimations, "Animation List", "")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &FlippyComponent::OnEditorPropertiesChanged)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Multiple Sprite Sheets Configuration")
                    ->Attribute(AZ::Edit::Attributes::Visibility, &FlippyComponent::IsMultiMode)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &FlippyComponent::m_spriteSheets, "Sprite Sheets", "Arrayed list of images and their specific animations")
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
        m_lastGameTickTime = std::chrono::time_point<std::chrono::steady_clock>();
    }

    void FlippyComponent::Activate()
    {
        FlippyComponentRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        AZ::SystemTickBus::Handler::BusConnect();

        m_lastSystemTickTime = std::chrono::steady_clock::now();
        m_lastGameTickTime = std::chrono::time_point<std::chrono::steady_clock>();
        m_isGameActive = false;
        m_isMaterialInitialized = false;

        SetupInitialVisualState();
    }

    void FlippyComponent::Deactivate()
    {
        m_isGameActive = false;

        AZ::TickBus::Handler::BusDisconnect();
        FlippyComponentRequestBus::Handler::BusDisconnect();
    }

    bool FlippyComponent::IsSingleMode() const { return !m_useMultipleSpriteSheets; }
    bool FlippyComponent::IsMultiMode() const { return m_useMultipleSpriteSheets; }
    bool FlippyComponent::IsDefaultAnimationEnabled() const { return m_enableDefaultAnimation; }

    void FlippyComponent::SetupInitialVisualState()
    {
        StopAnimation();

        if (m_enableDefaultAnimation && !m_defaultAnimation.empty())
        {
            PlayAnimation(m_defaultAnimation);

            if (!m_previewInEditor && !m_isGameActive)
            {
                StopAnimation();
                RefreshMaterial();
            }
        }
        else
        {
            if (m_useMultipleSpriteSheets && !m_spriteSheets.empty())
            {
                SetMaterialTexture(m_spriteSheets[0].m_spriteAsset);
                m_activeSpriteId = m_spriteSheets[0].m_spriteAsset.GetId();
                m_activeColumns = m_spriteSheets[0].m_columns;
                m_activeRows = m_spriteSheets[0].m_rows;
            }
            else if (!m_useMultipleSpriteSheets)
            {
                SetMaterialTexture(m_singleSpriteAsset);
                m_activeSpriteId = m_singleSpriteAsset.GetId();
                m_activeColumns = m_singleColumns;
                m_activeRows = m_singleRows;
            }

            m_currentFrame = 0;
            m_isMaterialInitialized = false;
            RefreshMaterial();
        }
    }

    void FlippyComponent::PlayAnimation(const AZStd::string& animationName)
    {
        if (animationName.empty()) return;

        const FlippyAnimation* targetAnim = nullptr;
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> targetSprite;
        int targetCols = 1;
        int targetRows = 1;

        if (m_useMultipleSpriteSheets)
        {
            for (const auto& sheet : m_spriteSheets)
            {
                for (const auto& anim : sheet.m_animations)
                {
                    if (anim.m_name == animationName)
                    {
                        targetAnim = &anim;
                        targetSprite = sheet.m_spriteAsset;
                        targetCols = sheet.m_columns;
                        targetRows = sheet.m_rows;
                        break;
                    }
                }
                if (targetAnim) break;
            }
        }
        else
        {
            for (const auto& anim : m_singleAnimations)
            {
                if (anim.m_name == animationName)
                {
                    targetAnim = &anim;
                    targetSprite = m_singleSpriteAsset;
                    targetCols = m_singleColumns;
                    targetRows = m_singleRows;
                    break;
                }
            }
        }

        if (!targetAnim) return;

        if (m_isPlaying && m_currentAnim.m_name == targetAnim->m_name)
        {
            m_currentAnim = *targetAnim;
            return;
        }

        m_currentAnim = *targetAnim;

        if (targetSprite.GetId() != m_activeSpriteId)
        {
            SetMaterialTexture(targetSprite);
            m_activeSpriteId = targetSprite.GetId();
            m_activeColumns = targetCols;
            m_activeRows = targetRows;
            m_isMaterialInitialized = false;
        }

        if (m_activeColumns <= 0 || m_activeRows <= 0) return;

        int startFrameCalc = (m_currentAnim.m_startRow * m_activeColumns) + m_currentAnim.m_startColumn;
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
        SetupInitialVisualState();
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void FlippyComponent::RefreshMaterial()
    {
        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();

        if (!AZ::Render::MaterialComponentRequestBus::HasHandlers(targetEntity) || m_activeColumns <= 0 || m_activeRows <= 0)
        {
            return;
        }

        float tileU = 1.0f / static_cast<float>(m_activeColumns);
        float tileV = 1.0f / static_cast<float>(m_activeRows);

        if (!m_isMaterialInitialized || m_lastTileU != tileU || m_lastTileV != tileV)
        {
            ApplyMaterialScale(tileU, tileV);
            m_lastTileU = tileU;
            m_lastTileV = tileV;
            m_isMaterialInitialized = true;
        }

        int currentColumn = m_currentFrame % m_activeColumns;
        int currentRow = m_currentFrame / m_activeColumns;

        float offsetX = static_cast<float>(currentColumn) * tileU;
        float offsetY = static_cast<float>(currentRow) * tileV;

        ApplyMaterialOffset(offsetX, offsetY);
    }

    void FlippyComponent::AdvanceFrame(float deltaTime)
    {
        if (!m_isPlaying || m_activeColumns <= 0 || m_activeRows <= 0 || m_currentAnim.m_frameCount <= 0 || m_currentAnim.m_fps <= 0.0f)
        {
            return;
        }

        int globalStartFrame = (m_currentAnim.m_startRow * m_activeColumns) + m_currentAnim.m_startColumn;
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
        if (deltaTime <= 0.0f)
        {
            return;
        }

        m_lastGameTickTime = std::chrono::steady_clock::now();
        m_isGameActive = true;

        AdvanceFrame(deltaTime);
    }

    void FlippyComponent::OnSystemTick()
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - m_lastSystemTickTime;
        m_lastSystemTickTime = now;

        std::chrono::duration<float> timeSinceGameTick = now - m_lastGameTickTime;
        if (timeSinceGameTick.count() < 0.5f)
        {
            return;
        }

        if (!m_previewInEditor)
        {
            return;
        }

        AdvanceFrame(delta.count());
    }

    AZStd::vector<AZ::Render::MaterialAssignmentId> FlippyComponent::GetActiveMaterialIds(AZ::EntityId targetEntity) const
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

    void FlippyComponent::SetMaterialTexture(const AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& spriteAsset)
    {
        if (!spriteAsset.IsReady())
        {
            return;
        }

        AZ::EntityId targetEntity = m_materialEntityId.IsValid() ? m_materialEntityId : GetEntityId();
        auto materialIds = GetActiveMaterialIds(targetEntity);

        for (const auto& id : materialIds)
        {
            AZ::Render::MaterialComponentRequestBus::Event(
                targetEntity, &AZ::Render::MaterialComponentRequests::SetPropertyValue,
                id, AZStd::string("baseColor.textureMap"), AZStd::any(spriteAsset));
        }
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
        if (m_uvOffsetUProperty.empty() || m_uvOffsetVProperty.empty())
        {
            return;
        }

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

} // namespace FlippyGem