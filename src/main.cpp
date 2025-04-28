#include <Geode/modify/EditorUI.hpp>

#include <geode.custom-keybinds/include/Keybinds.hpp>

#include "CustomSetting.hpp"

#if defined(GEODE_IS_ANDROID) || defined(GEODE_IS_IOS)
bool isMobile = true;
#else
bool isMobile = false;
#endif

$on_mod(Loaded) {

    (void)Mod::get()->registerCustomSettingType("keybinds", &MyButtonSettingV3::parse);

    using namespace keybinds;

    for (auto const& [id, name, desc, key] : {
        std::tuple{"place_marker"_spr, "Place Marker", "Place the marker.", Keybind::create(KEY_X)},
        std::tuple{"remove_marker"_spr, "Remove Marker", "Remove the marker.", Keybind::create(KEY_Z)},
        std::tuple{"place_marker_cursor"_spr, "Place Marker At Cursor", "Place the marker at the cursor's position.", Keybind::create(KEY_X, Modifier::Control)}
    }) {
        BindManager::get()->registerBindable({id, name, desc, {key}});
    }

}

class $modify(MyEditorUI, EditorUI) {

    struct Fields {
        CCSprite* marker = nullptr;
    };

    void onPlaceMarker(CCObject*) {
        placeMarker();
    }

    void placeMarker(bool cursor = false) {
        if (!m_fields->marker) {
            m_fields->marker = CCSprite::create("marker.png"_spr);
            m_fields->marker->setScale(0.5f);
            m_fields->marker->setOpacity(120);
            m_editorLayer->m_objectLayer->addChild(m_fields->marker);
        }

        cocos2d::CCPoint pos = getMousePos();

        if (!cursor || isMobile) {
            cocos2d::CCSize winSize = CCDirector::sharedDirector()->getWinSize();

            if (CCNode* spr = getChildByID("background-sprite"))
                winSize.height += spr->getContentSize().height - spr->getPositionY();

            pos = winSize / 2.f;
        }

        m_fields->marker->setPosition(m_editorLayer->m_objectLayer->convertToNodeSpace(pos));
        m_fields->marker->setVisible(true);
    }

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        if (isMobile || Mod::get()->getSettingValue<bool>("show-button"))
            if (CCNode* menu = getChildByID("undo-menu")) {
                CCSprite* spr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
                spr->setScale(0.75f);

                CCSprite* spr2 = CCSprite::create("marker-2.png"_spr);
                spr2->setScale(0.9f);
                spr2->setPosition(spr->getContentSize() / 2.f);

                spr->addChild(spr2);

                CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyEditorUI::onPlaceMarker));

                menu->addChild(btn);
                menu->updateLayout();
            }

        using namespace keybinds;

        for (auto const& [bind, cursor] : {
            std::pair{"place_marker"_spr, false},
            std::pair{"remove_marker"_spr, false},
            std::pair{"place_marker_cursor"_spr, true}
        }) {
            this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {

            if (event->isDown() && bind != "remove_marker"_spr)
                placeMarker(cursor);
            else if (event->isDown() && m_fields->marker)
                m_fields->marker->setVisible(false);

            return ListenerResult::Propagate;
            }, bind);
        }

        return true;
    }

    void onPlayback(CCObject* obj) {
        if (!m_fields->marker) return EditorUI::onPlayback(obj);
        if (!m_fields->marker->isVisible()) return EditorUI::onPlayback(obj);

        cocos2d::CCPoint ogPos = m_editorLayer->m_objectLayer->getPosition();
        float ogScale = m_editorLayer->m_objectLayer->getScale();

        m_editorLayer->m_objectLayer->setPosition(-m_fields->marker->getPosition());
        m_editorLayer->m_objectLayer->setScale(1.f);
        
        EditorUI::onPlayback(obj);

        m_editorLayer->m_objectLayer->setPosition(ogPos);
        m_editorLayer->m_objectLayer->setScale(ogScale);
    }

};