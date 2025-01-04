#include "gui_visitor.h"
#include "imgui.h"
#include "../raytracer/hittable.h"
#include "../raytracer/sphere3d.h"
#include "../raytracer/vector3d.h"

void ImGuiVisitor::visit(Sphere3d *sphere)
{
    ImGui::InputDouble("Radius", &sphere->radius);
    ImGui::InputDouble("Center X", &sphere->center.e[0]);
    ImGui::InputDouble("Center Y", &sphere->center.e[1]);
    ImGui::InputDouble("Center Z", &sphere->center.e[2]);
}

void ImGuiVisitor::visit(Vector3d *vector)
{
    ImGui::InputDouble("X", &vector->e[0]);
    ImGui::InputDouble("Y", &vector->e[1]);
    ImGui::InputDouble("Z", &vector->e[2]);
}

void ImGuiVisitor::visit(IHittable *object)
{
    ImGui::Text("Unknown object");
}

void ImGuiVisitor::visit(IMaterial *material)
{
    ImGui::Text("Unknown material");
}

void ImGuiVisitor::visit(Lambertian *material)
{
    float albedo[3] = { static_cast<float>(material->albedo.e[0]), static_cast<float>(material->albedo.e[1]), static_cast<float>(material->albedo.e[2]) };
    if (ImGui::ColorEdit3("Albedo", albedo))
    {
        material->albedo.e[0] = static_cast<double>(albedo[0]);
        material->albedo.e[1] = static_cast<double>(albedo[1]);
        material->albedo.e[2] = static_cast<double>(albedo[2]);
    }
}

void ImGuiVisitor::visit(Metal *material)
{
    float albedo[3] = { static_cast<float>(material->albedo.e[0]), static_cast<float>(material->albedo.e[1]), static_cast<float>(material->albedo.e[2]) };
    if (ImGui::ColorEdit3("Albedo", albedo))
    {
        material->albedo.e[0] = static_cast<double>(albedo[0]);
        material->albedo.e[1] = static_cast<double>(albedo[1]);
        material->albedo.e[2] = static_cast<double>(albedo[2]);
    }
    ImGui::InputDouble("Fuzz", &material->fuzz);
}

void ImGuiVisitor::visit(Dielectric *material)
{
    float tint[3] = { static_cast<float>(material->tint.e[0]), static_cast<float>(material->tint.e[1]), static_cast<float>(material->tint.e[2]) };
    if (ImGui::ColorEdit3("Tint", tint))
    {
        material->tint.e[0] = static_cast<double>(tint[0]);
        material->tint.e[1] = static_cast<double>(tint[1]);
        material->tint.e[2] = static_cast<double>(tint[2]);
    }
    ImGui::InputDouble("Refraction index", &material->refraction_index);
}