#include "interface_utils.h"

bool CustomInputDoubleWithLabel(std::string label, double *v, double step, double step_fast, const char *format, ImGuiInputTextFlags flags)
{
    if (v == nullptr)
    {
        return false;
    }

    ImGui::InputDouble(getLabelForValue(label, *v).c_str(), v, step, step_fast, format, flags);

    return true;
}

float getAutoWidthForChildren(int nChildren)
{
    float w = (ImGui::GetContentRegionAvail().x - 20) / nChildren;
    if (w < 2.0f)
    {
        w = 2.0f;
    }
    return w;
}