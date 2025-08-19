#include <imgui.h>
#include <string>
#include <functional>

class UIFrame {
public:
    std::string title;
    bool isOpen = true;
    bool closable = true;
    ImVec2 size = ImVec2(0, 0);

    std::function<void()> drawContent;

    UIFrame(const std::string& title, std::function<void()> content, bool closable = true)
        : title(title), drawContent(content), closable(closable) {}

    void setSize(float w, float h) {
        size = ImVec2(w, h);
    }

    void setClosable(bool value) {
        closable = value;
    }

    bool isClosable() const {
        return closable;
    }

    void draw() {
        if (!isOpen) return;

        if (size.x > 0 && size.y > 0)
            ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

        // Only pass &isOpen if closable, otherwise pass nullptr
        if (ImGui::Begin(title.c_str(), closable ? &isOpen : nullptr)) {
            if (drawContent) drawContent();
        }
        ImGui::End();
    }
};
