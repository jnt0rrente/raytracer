#ifndef INTERFACE_UTILS_H
#define INTERFACE_UTILS_H

#include <string>
#include <sstream>
#include <cstdint>
#include <vector>
#include "imgui.h"

template <typename T>
std::string getLabelForValue(std::string label, T &value)
{
    std::stringstream ss;
    ss << label << "##" << reinterpret_cast<uintptr_t>(&value);
    return ss.str();
}

bool CustomInputDoubleWithLabel(std::string label, double *v, double step = (0.0), double step_fast = (0.0), const char *format = "%.6f", ImGuiInputTextFlags flags = 0);

float getAutoWidthForChildren(int nChildren);

#endif