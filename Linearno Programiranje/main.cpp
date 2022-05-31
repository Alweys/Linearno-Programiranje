#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <implot.h>
#include <implot_internal.h>
#include <d3d9.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int n;
std::vector<int> x;
std::vector<int> y;

double maxg(double s) {
    double g = 0;
    for (int i = 0; i < n; i++)
        if (s == 0)
            g += y[i];
        else if (x[i] <= s)
            s -= x[i];
        else {
            g += (1 - s / y[i]) * x[i];
            s = 0;
        }
    return g;
}

double value(std::vector <int> _x, std::vector <int> _y) {
    n = _x.size();
    x = _x;
    y = _y;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (x[j] * y[i] > x[i] * y[j]) {
                std::swap(x[i], x[j]);
                std::swap(y[i], y[j]);
            }
    double lo = 0;
    double hi = 51 * 100;
    double D = 1e-10;
    while (lo + D < hi && lo * (1 + D) < hi) {
        double mid = (lo + hi) / 2;
        if (mid <= maxg(mid))
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}

struct equation {
    // Ax operator_ By jednakost C
    float A;
    float x;
    float B;
    float y;
    float C;
    int operator_;
    int jednakost;
    float xs[100001];
    float ys[100001];
    bool valid;
    equation()
    {
        A = 0.0f;
        x = 0.0f;
        B = 0.0f;
        y = 0.0f;
        C = 0.0f;
        operator_ = 0;
        jednakost = 0;
        for (int i = 0; i < 100001; i++)
        {
            xs[i] = -1;
            ys[i] = -1;
        }
        valid = false;
    }
};

struct problems
{
    float A;
    float B;
    bool valid;
    
    problems()
    {
        A = 0.0f;
        B = 0.0f;
        valid = false;
    }
};

problems problem;
equation equations[5];

bool calculateEquasion(double A, double x, int operator_, double B, double y, int jednakost, double C)
{
    switch (operator_)
    {
    case 0:
    {
        if (jednakost == 0)
        {
            return (roundf((A * x) + (B * y)) == C);
        }
        else if (jednakost == 1)
        {
            return (roundf((A * x) + (B * y)) >= C);
        }
        else if (jednakost == 2)
        {
            return (roundf((A * x) + (B * y)) <= C);
        }
        break;
    }
    case 1:
    {
        if (jednakost == 0)
        {
            return (roundf((A * x) - (B * y)) == C);
        }
        else if (jednakost == 1)
        {
            return (roundf((A * x) - (B * y)) >= C);
        }
        else if (jednakost == 2)
        {
            return (roundf((A * x) - (B * y)) <= C);
        }
        break;
    }
    }
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 2)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}
#define mk_pair std::pair<double, double>

mk_pair intersection(mk_pair A, mk_pair B, mk_pair C, mk_pair D) {
    // Line AB represented as a1x + b1y = c1
    double a = B.second - A.second;
    double b = A.first - B.first;
    double c = a * (A.first) + b * (A.second);
    // Line CD represented as a2x + b2y = c2
    double a1 = D.second - C.second;
    double b1 = C.first - D.first;
    double c1 = a1 * (C.first) + b1 * (C.second);
    double det = a * b1 - a1 * b;
    if (det == 0) {
        return std::make_pair(FLT_MAX, FLT_MAX);
    }
    else {
        double x = (b1 * c - b * c1) / det;
        double y = (a * c1 - a1 * c) / det;
        return std::make_pair(x, y);
    }
}
void display(mk_pair par) {
    std::cout << "(" << par.first << ", " << par.second << ")" << std::endl;
}

void ApplyDefaultStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->FrameRounding = 6;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(1.0f, 0.24f, 0.24f, 0.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.24f, 0.24f, 0.50f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.24f, 0.24f, 0.70f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.24f, 0.24f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.0f, 0.24f, 0.24f, 0.50f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 0.24f, 0.24f, 0.70f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.24f, 0.24f, 0.90f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.24f, 0.24f, 0.50f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.24f, 0.24f, 0.90f);
    colors[ImGuiCol_Button] = ImVec4(1.0f, 0.24f, 0.24f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(1.0f, 0.24f, 0.24f, 0.50f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(1.0f, 0.24f, 0.24f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 0.24f, 0.24f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 0.24f, 0.24f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

    colors[ImGuiCol_Tab] = ImVec4(1.0f, 0.24f, 0.24f, 0.40f);
    colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(1.0f, 0.24f, 0.24f, 0.20f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 0.24f, 0.24f, 0.30f);
}

int main(int, char**)
{

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Linearno Progamiranje"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Linearno programiranje"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 900, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //ImGui::StyleColorsDark();
    ApplyDefaultStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    /*equations[0].A = 1.0f;
    equations[0].B = 2.0f;
    equations[0].operator_ = 0;
    equations[0].jednakost = 1;
    equations[0].C = 10.0f;

    equations[1].A = 2.0f;
    equations[1].B = 2.0f;
    equations[1].operator_ = 0;
    equations[1].jednakost = 1;
    equations[1].C = 12.0f;
    
    equations[2].A = 3.0f;
    equations[2].B = 1.0f;
    equations[2].operator_ = 0;
    equations[2].jednakost = 1;
    equations[2].C = 8.0f;
    
    equations[3].A = 2.0f;
    equations[3].B = 8.0f;
    equations[3].operator_ = 0;
    equations[3].jednakost = 1;
    equations[3].C = 30.0f;
    
    equations[4].A = 0.1f;
    equations[4].B = 2.0f;
    equations[4].operator_ = 0;
    equations[4].jednakost = 1;
    equations[4].C = 4.0f;*/


    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y ));
        ImGui::Begin("Linearno programiranje", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        //ImPlot::ShowDemoWindow();
        //ImGui::ShowDemoWindow();
        static int jednadzbe = 1;
        static int size = 10001;
        ImGui::Text("Velicina: "); ImGui::SameLine();  ImGui::SliderInt("  ", &size, 10, 100001);
        ImGui::Text("Broj jednadzbi: "); ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f); ImGui::Combo("   ", &jednadzbe, " 1\0 2\0 3\0 4\0 5\0\0", 5);
        for (int i = 0; i < jednadzbe+1; i++)
        {
            std::string text = std::to_string(i + 1);
            text += ": ";
            ImGui::BeginChild(std::to_string(i).c_str(), ImVec2(400, 40), true);
            {
                ImGui::Text(text.c_str());
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::InputFloat("x", &equations[i].A, 0, 0, "%.1f"); ImGui::SameLine();
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::Combo("##", &equations[i].operator_, " +\0 -\0\0", 2);
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::InputFloat("y", &equations[i].B, 0, 0, "%.1f"); ImGui::SameLine();
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::Combo(" ", &equations[i].jednakost, " =\0 >=\0 <=\0\0", 3);
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::InputFloat("", &equations[i].C, 0, 0, "%.1f");
            }ImGui::EndChild();
        }
        
        static int valid_equations = 0;
        static float scale = 1.0f;
        static double xs1[100001], ys1[100001];
        static int shade_mode = 0;
        static bool setDots = false;
        static bool once = true;
        static int dotIndex = 0;
        static float xs2[100], ys2[100];
        ImGui::BeginChild("GrafChild", ImVec2(ImGui::GetIO().DisplaySize.x / 1.9, ImGui::GetIO().DisplaySize.y / 1.9), true);
        {
            if (ImPlot::BeginPlot("Graf", ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2.1))) {
                ImPlot::SetupAxes("x", "y");
                ImPlot::SetupAxisLimits(ImAxis_X1, -1000, 1000);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -700, 800);
                for (int eq = 0; eq < jednadzbe + 1; eq++)
                {
                    if ((equations[eq].A != 0.0f || equations[eq].B != 0.0f) && equations[eq].operator_ != -1 && equations[eq].jednakost != -1)
                    {
                        if (!equations[eq].valid)
                            valid_equations++;
                        equations[eq].valid = true;
                        int startX = size / 2;
                        for (int i = 0; i < size; i++)
                        {
                            float A = equations[eq].A, x = -startX + i, B = equations[eq].B, C = equations[eq].C;
                            // Ax + By = C
                            // y = -((A * x) / B) + (C / B)
                            // Ax - By = C
                            // y = (-((A * x) / B) + (C / B)) * -1.0f;
                            if (equations[eq].operator_ == 0)
                            {
                                float y;
                                if (B != 0)
                                {
                                    y = -((A * x) / B) + (C / B);
                                    xs1[i] = x * scale;
                                    ys1[i] = y * scale;
                                    equations[eq].xs[i] = xs1[i];
                                    equations[eq].ys[i] = ys1[i];
                                }
                                else
                                {
                                    xs1[i] = C * scale;
                                    ys1[i] = x * scale;
                                    equations[eq].xs[i] = xs1[i];
                                    equations[eq].ys[i] = ys1[i];
                                }
                            }
                            else if (equations[eq].operator_ == 1)
                            {
                                float y;
                                if (B != 0)
                                {
                                    y = (-((A * x) / B) + (C / B)) * -1.0f;
                                    xs1[i] = x * scale;
                                    ys1[i] = y * scale;
                                    equations[eq].xs[i] = xs1[i];
                                    equations[eq].ys[i] = ys1[i];
                                }
                                else
                                {
                                    xs1[i] = C * scale;
                                    ys1[i] = x * scale;
                                    equations[eq].xs[i] = xs1[i];
                                    equations[eq].ys[i] = ys1[i];
                                }
                            }
                        }
                        std::string text = "";
                        text += to_string_with_precision(equations[eq].A);
                        text += "x";
                        text += equations[eq].operator_ ? " - " : " + ";
                        text += to_string_with_precision(equations[eq].B);
                        text += "y";
                        switch (equations[eq].jednakost)
                        {
                        case 0:
                        {
                            text += " = ";
                            break;
                        }
                        case 1:
                        {
                            text += " >= ";
                            break;
                        }
                        case 2:
                        {
                            text += " <= ";
                            break;
                        }
                        }
                        text += to_string_with_precision(equations[eq].C);
                        //ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                        ImPlot::PlotLine(text.c_str(), xs1, ys1, size);
                        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
                        if (equations[eq].jednakost != 0)
                        {
                            ImPlot::PlotShaded(text.c_str(), xs1, ys1, size, equations[eq].jednakost == 2 ? -INFINITY : INFINITY);
                        }
                        ImPlot::PopStyleVar();
                    }
                    else
                    {
                        if (equations[eq].valid)
                            valid_equations--;
                        equations[eq].valid = false;
                    }

                    if (valid_equations >= 2)
                    {
                        if (equations[eq].valid)
                        {
                            static bool draw = false;
                            if (setDots && once)
                            {
                                draw = false;
                                dotIndex = 0;
                                once = false;
                                if (jednadzbe + 1 == 2)
                                {
                                    mk_pair q = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s = std::make_pair(0.0f, 0.0f);
                                    mk_pair t = std::make_pair(0.0f, 10.0f);

                                    mk_pair q2 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r2 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s2 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t2 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q3 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r3 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s3 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair t3 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);

                                    mk_pair q4 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r4 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s4 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t4 = std::make_pair(10, 0);

                                    mk_pair q5 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r5 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s5 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t5 = std::make_pair(10, 0);

                                    mk_pair inter = intersection(q, r, s, t);
                                    mk_pair inter2 = intersection(q2, r2, s2, t2);
                                    mk_pair inter3 = intersection(q3, r3, s3, t3);
                                    mk_pair inter4 = intersection(q4, r4, s4, t4);
                                    mk_pair inter5 = intersection(q5, r5, s5, t5);

                                    if (calculateEquasion(equations[0].A, inter.first, equations[0].operator_, equations[0].B, inter.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter.first, equations[1].operator_, equations[1].B, inter.second, equations[1].jednakost, equations[1].C))
                                    {
                                        if (inter.first == FLT_MAX && inter.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "The intersection of the given lines AB and CD is: ";
                                            display(inter);
                                            xs2[dotIndex] = inter.first;
                                            ys2[dotIndex] = inter.second;
                                            dotIndex++;
                                        }
                                    }
                                    if (calculateEquasion(equations[0].A, inter2.first, equations[0].operator_, equations[0].B, inter2.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter2.first, equations[1].operator_, equations[1].B, inter2.second, equations[1].jednakost, equations[1].C))
                                    {
                                        if (inter2.first == FLT_MAX && inter2.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "The intersection of the given lines AB and CD is: ";
                                            display(inter2);
                                            xs2[dotIndex] = inter2.first;
                                            ys2[dotIndex] = inter2.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (inter3.first == FLT_MAX && inter3.second == FLT_MAX) {
                                        std::cout << "The given lines AB and CD are parallel.\n";
                                    }
                                    else {
                                        std::cout << "The intersection of the given lines AB and CD is: ";
                                        display(inter3);
                                        xs2[dotIndex] = inter3.first;
                                        ys2[dotIndex] = inter3.second;
                                        dotIndex++;
                                    }

                                    if (calculateEquasion(equations[0].A, inter4.first, equations[0].operator_, equations[0].B, inter4.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter4.first, equations[1].operator_, equations[1].B, inter4.second, equations[1].jednakost, equations[1].C))
                                    {
                                        if (inter4.first == FLT_MAX && inter4.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "The intersection of the given lines AB and CD is: ";
                                            display(inter4);
                                            xs2[dotIndex] = inter4.first;
                                            ys2[dotIndex] = inter4.second;
                                            dotIndex++;
                                        }
                                    }
                                    if (calculateEquasion(equations[0].A, inter5.first, equations[0].operator_, equations[0].B, inter5.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter5.first, equations[1].operator_, equations[1].B, inter5.second, equations[1].jednakost, equations[1].C))
                                    {
                                        if (inter5.first == FLT_MAX && inter5.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "The intersection of the given lines AB and CD is: ";
                                            display(inter5);
                                            xs2[dotIndex] = inter5.first;
                                            ys2[dotIndex] = inter5.second;
                                            dotIndex++;
                                        }
                                    }

                                    draw = true;
                                }
                                if (jednadzbe + 1 == 3)
                                {
                                    mk_pair q = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s = std::make_pair(0.0f, 0.0f);
                                    mk_pair t = std::make_pair(0.0f, 10.0f);

                                    mk_pair q2 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r2 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s2 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t2 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q3 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r3 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s3 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t3 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q4 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r4 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s4 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair t4 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);

                                    mk_pair q9 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r9 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s9 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair t9 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);

                                    mk_pair q5 = std::make_pair(equations[1].xs[0], equations[2].ys[0]);
                                    mk_pair r5 = std::make_pair(equations[1].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s5 = std::make_pair(equations[2].xs[0], equations[1].ys[0]);
                                    mk_pair t5 = std::make_pair(equations[2].xs[size-1], equations[1].ys[size-1]);

                                    mk_pair q6 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r6 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s6 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t6 = std::make_pair(10, 0);

                                    mk_pair q7 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r7 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s7 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t7 = std::make_pair(10, 0);

                                    mk_pair q8 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r8 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s8 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t8 = std::make_pair(10, 0);

                                    mk_pair inter = intersection(q, r, s, t);
                                    mk_pair inter2 = intersection(q2, r2, s2, t2);
                                    mk_pair inter3 = intersection(q3, r3, s3, t3);
                                    mk_pair inter4 = intersection(q4, r4, s4, t4);
                                    mk_pair inter5 = intersection(q5, r5, s5, t5);
                                    mk_pair inter6 = intersection(q6, r6, s6, t6);
                                    mk_pair inter7 = intersection(q7, r7, s7, t7);
                                    mk_pair inter8 = intersection(q8, r8, s8, t8);
                                    mk_pair inter9 = intersection(q9, r9, s9, t9);

                                    if (calculateEquasion(equations[0].A, inter.first, equations[0].operator_, equations[0].B, inter.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter.first, equations[1].operator_, equations[1].B, inter.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter.first, equations[2].operator_, equations[2].B, inter.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter.first == FLT_MAX && inter.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "1 The intersection of the given lines AB and CD is: ";
                                            display(inter);
                                            xs2[dotIndex] = inter.first;
                                            ys2[dotIndex] = inter.second;
                                            dotIndex++;
                                        }
                                    }
                                    if (calculateEquasion(equations[0].A, inter2.first, equations[0].operator_, equations[0].B, inter2.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter2.first, equations[1].operator_, equations[1].B, inter2.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter2.first, equations[2].operator_, equations[2].B, inter2.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter2.first == FLT_MAX && inter2.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "2 The intersection of the given lines AB and CD is: ";
                                            display(inter2);
                                            xs2[dotIndex] = inter2.first;
                                            ys2[dotIndex] = inter2.second;
                                            dotIndex++;
                                        }
                                    }
                                    if (calculateEquasion(equations[0].A, inter3.first, equations[0].operator_, equations[0].B, inter3.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter3.first, equations[1].operator_, equations[1].B, inter3.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter3.first, equations[2].operator_, equations[2].B, inter3.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter3.first == FLT_MAX && inter3.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "3 The intersection of the given lines AB and CD is: ";
                                            display(inter3);
                                            xs2[dotIndex] = inter3.first;
                                            ys2[dotIndex] = inter3.second;
                                            dotIndex++;
                                        }
                                    }
                                    if (calculateEquasion(equations[0].A, inter4.first, equations[0].operator_, equations[0].B, inter4.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter4.first, equations[1].operator_, equations[1].B, inter4.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter4.first, equations[2].operator_, equations[2].B, inter4.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter4.first == FLT_MAX && inter4.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "4 The intersection of the given lines AB and CD is: ";
                                            display(inter4);
                                            xs2[dotIndex] = inter4.first;
                                            ys2[dotIndex] = inter4.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter5.first, equations[0].operator_, equations[0].B, inter5.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter5.first, equations[1].operator_, equations[1].B, inter5.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter5.first, equations[2].operator_, equations[2].B, inter5.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter5.first == FLT_MAX && inter5.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "5 The intersection of the given lines AB and CD is: ";
                                            display(inter5);
                                            xs2[dotIndex] = inter5.first;
                                            ys2[dotIndex] = inter5.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter6.first, equations[0].operator_, equations[0].B, inter6.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter6.first, equations[1].operator_, equations[1].B, inter6.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter6.first, equations[2].operator_, equations[2].B, inter6.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter6.first == FLT_MAX && inter6.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "6 The intersection of the given lines AB and CD is: ";
                                            display(inter6);
                                            xs2[dotIndex] = inter6.first;
                                            ys2[dotIndex] = inter6.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter7.first, equations[0].operator_, equations[0].B, inter7.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter7.first, equations[1].operator_, equations[1].B, inter7.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter7.first, equations[2].operator_, equations[2].B, inter7.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter7.first == FLT_MAX && inter7.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "7 The intersection of the given lines AB and CD is: ";
                                            display(inter7);
                                            xs2[dotIndex] = inter7.first;
                                            ys2[dotIndex] = inter7.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter8.first, equations[0].operator_, equations[0].B, inter8.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter8.first, equations[1].operator_, equations[1].B, inter8.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter8.first, equations[2].operator_, equations[2].B, inter8.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter8.first == FLT_MAX && inter8.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "8 The intersection of the given lines AB and CD is: ";
                                            display(inter8);
                                            xs2[dotIndex] = inter8.first;
                                            ys2[dotIndex] = inter8.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter9.first, equations[0].operator_, equations[0].B, inter9.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter9.first, equations[1].operator_, equations[1].B, inter9.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter9.first, equations[2].operator_, equations[2].B, inter9.second, equations[2].jednakost, equations[2].C))
                                    {
                                        if (inter9.first == FLT_MAX && inter9.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "9 The intersection of the given lines AB and CD is: ";
                                            display(inter9);
                                            xs2[dotIndex] = inter9.first;
                                            ys2[dotIndex] = inter9.second;
                                            dotIndex++;
                                        }
                                    }

                                    draw = true;
                                }

                                if (jednadzbe + 1 == 4)
                                {
                                    mk_pair q = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s = std::make_pair(0.0f, 0.0f);
                                    mk_pair t = std::make_pair(0.0f, 10.0f);

                                    mk_pair q2 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r2 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s2 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t2 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q3 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r3 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s3 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t3 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q4 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair r4 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);
                                    mk_pair s4 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t4 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q5 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r5 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s5 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair t5 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);

                                    mk_pair q6 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r6 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s6 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair t6 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);

                                    mk_pair q7 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r7 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s7 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair t7 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);

                                    mk_pair q8 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r8 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s8 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair t8 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);

                                    mk_pair q9 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r9 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s9 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair t9 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);

                                    mk_pair q10 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r10 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s10 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t10 = std::make_pair(10, 0);

                                    mk_pair q11 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r11 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s11 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t11 = std::make_pair(10, 0);

                                    mk_pair q12 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r12 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s12 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t12 = std::make_pair(10, 0);

                                    mk_pair q13 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair r13 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);
                                    mk_pair s13 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t13 = std::make_pair(10, 0);

                                    mk_pair inter = intersection(q, r, s, t);
                                    mk_pair inter2 = intersection(q2, r2, s2, t2);
                                    mk_pair inter3 = intersection(q3, r3, s3, t3);
                                    mk_pair inter4 = intersection(q4, r4, s4, t4);
                                    mk_pair inter5 = intersection(q5, r5, s5, t5);
                                    mk_pair inter6 = intersection(q6, r6, s6, t6);
                                    mk_pair inter7 = intersection(q7, r7, s7, t7);
                                    mk_pair inter8 = intersection(q8, r8, s8, t8);
                                    mk_pair inter9 = intersection(q9, r9, s9, t9);
                                    mk_pair inter10 = intersection(q10, r10, s10, t10);
                                    mk_pair inter11 = intersection(q11, r11, s11, t11);
                                    mk_pair inter12 = intersection(q12, r12, s12, t12);
                                    mk_pair inter13 = intersection(q13, r13, s13, t13);

                                    if (calculateEquasion(equations[0].A, inter.first, equations[0].operator_, equations[0].B, inter.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter.first, equations[1].operator_, equations[1].B, inter.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter.first, equations[2].operator_, equations[2].B, inter.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter.first, equations[3].operator_, equations[3].B, inter.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter.first == FLT_MAX && inter.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "1 The intersection of the given lines AB and CD is: ";
                                            display(inter);
                                            xs2[dotIndex] = inter.first;
                                            ys2[dotIndex] = inter.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter2.first, equations[0].operator_, equations[0].B, inter2.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter2.first, equations[1].operator_, equations[1].B, inter2.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter2.first, equations[2].operator_, equations[2].B, inter2.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter2.first, equations[3].operator_, equations[3].B, inter2.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter2.first == FLT_MAX && inter2.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "2 The intersection of the given lines AB and CD is: ";
                                            display(inter2);
                                            xs2[dotIndex] = inter2.first;
                                            ys2[dotIndex] = inter2.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter3.first, equations[0].operator_, equations[0].B, inter3.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter3.first, equations[1].operator_, equations[1].B, inter3.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter3.first, equations[2].operator_, equations[2].B, inter3.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter3.first, equations[3].operator_, equations[3].B, inter3.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter3.first == FLT_MAX && inter3.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "3 The intersection of the given lines AB and CD is: ";
                                            display(inter3);
                                            xs2[dotIndex] = inter3.first;
                                            ys2[dotIndex] = inter3.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter4.first, equations[0].operator_, equations[0].B, inter4.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter4.first, equations[1].operator_, equations[1].B, inter4.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter4.first, equations[2].operator_, equations[2].B, inter4.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter4.first, equations[3].operator_, equations[3].B, inter4.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter4.first == FLT_MAX && inter4.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "4 The intersection of the given lines AB and CD is: ";
                                            display(inter4);
                                            xs2[dotIndex] = inter4.first;
                                            ys2[dotIndex] = inter4.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter5.first, equations[0].operator_, equations[0].B, inter5.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter5.first, equations[1].operator_, equations[1].B, inter5.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter5.first, equations[2].operator_, equations[2].B, inter5.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter5.first, equations[3].operator_, equations[3].B, inter5.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter5.first == FLT_MAX && inter5.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "5 The intersection of the given lines AB and CD is: ";
                                            display(inter5);
                                            xs2[dotIndex] = inter5.first;
                                            ys2[dotIndex] = inter5.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter6.first, equations[0].operator_, equations[0].B, inter6.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter6.first, equations[1].operator_, equations[1].B, inter6.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter6.first, equations[2].operator_, equations[2].B, inter6.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter6.first, equations[3].operator_, equations[3].B, inter6.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter6.first == FLT_MAX && inter6.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "6 The intersection of the given lines AB and CD is: ";
                                            display(inter6);
                                            xs2[dotIndex] = inter6.first;
                                            ys2[dotIndex] = inter6.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter7.first, equations[0].operator_, equations[0].B, inter7.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter7.first, equations[1].operator_, equations[1].B, inter7.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter7.first, equations[2].operator_, equations[2].B, inter7.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter7.first, equations[3].operator_, equations[3].B, inter7.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter7.first == FLT_MAX && inter7.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "7 The intersection of the given lines AB and CD is: ";
                                            display(inter7);
                                            xs2[dotIndex] = inter7.first;
                                            ys2[dotIndex] = inter7.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter8.first, equations[0].operator_, equations[0].B, inter8.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter8.first, equations[1].operator_, equations[1].B, inter8.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter8.first, equations[2].operator_, equations[2].B, inter8.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter8.first, equations[3].operator_, equations[3].B, inter8.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter8.first == FLT_MAX && inter8.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "8 The intersection of the given lines AB and CD is: ";
                                            display(inter8);
                                            xs2[dotIndex] = inter8.first;
                                            ys2[dotIndex] = inter8.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter9.first, equations[0].operator_, equations[0].B, inter9.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter9.first, equations[1].operator_, equations[1].B, inter9.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter9.first, equations[2].operator_, equations[2].B, inter9.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter9.first, equations[3].operator_, equations[3].B, inter9.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter9.first == FLT_MAX && inter9.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "9 The intersection of the given lines AB and CD is: ";
                                            display(inter9);
                                            xs2[dotIndex] = inter9.first;
                                            ys2[dotIndex] = inter9.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter10.first, equations[0].operator_, equations[0].B, inter10.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter10.first, equations[1].operator_, equations[1].B, inter10.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter10.first, equations[2].operator_, equations[2].B, inter10.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter10.first, equations[3].operator_, equations[3].B, inter10.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter10.first == FLT_MAX && inter10.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "10 The intersection of the given lines AB and CD is: ";
                                            display(inter10);
                                            xs2[dotIndex] = inter10.first;
                                            ys2[dotIndex] = inter10.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter11.first, equations[0].operator_, equations[0].B, inter11.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter11.first, equations[1].operator_, equations[1].B, inter11.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter11.first, equations[2].operator_, equations[2].B, inter11.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter11.first, equations[3].operator_, equations[3].B, inter11.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter11.first == FLT_MAX && inter11.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "11 The intersection of the given lines AB and CD is: ";
                                            display(inter11);
                                            xs2[dotIndex] = inter11.first;
                                            ys2[dotIndex] = inter11.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter12.first, equations[0].operator_, equations[0].B, inter12.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter12.first, equations[1].operator_, equations[1].B, inter12.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter12.first, equations[2].operator_, equations[2].B, inter12.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter12.first, equations[3].operator_, equations[3].B, inter12.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter12.first == FLT_MAX && inter12.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "12 The intersection of the given lines AB and CD is: ";
                                            display(inter12);
                                            xs2[dotIndex] = inter12.first;
                                            ys2[dotIndex] = inter12.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter13.first, equations[0].operator_, equations[0].B, inter13.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter13.first, equations[1].operator_, equations[1].B, inter13.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter13.first, equations[2].operator_, equations[2].B, inter13.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter13.first, equations[3].operator_, equations[3].B, inter13.second, equations[3].jednakost, equations[3].C))
                                    {
                                        if (inter13.first == FLT_MAX && inter13.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "13 The intersection of the given lines AB and CD is: ";
                                            display(inter13);
                                            xs2[dotIndex] = inter13.first;
                                            ys2[dotIndex] = inter13.second;
                                            dotIndex++;
                                        }
                                    }

                                    draw = true;
                                }
                                if (jednadzbe + 1 == 5)
                                {
                                    mk_pair q = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s = std::make_pair(0.0f, 0.0f);
                                    mk_pair t = std::make_pair(0.0f, 10.0f);

                                    mk_pair q2 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r2 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s2 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t2 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q3 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r3 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s3 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t3 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q4 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair r4 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);
                                    mk_pair s4 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t4 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q5 = std::make_pair(equations[4].xs[0], equations[4].ys[0]);
                                    mk_pair r5 = std::make_pair(equations[4].xs[size-1], equations[4].ys[size-1]);
                                    mk_pair s5 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t5 = std::make_pair(0.0f, 10.0f);

                                    mk_pair q6 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r6 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s6 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair t6 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);

                                    mk_pair q7 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r7 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s7 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair t7 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);

                                    mk_pair q8 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r8 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s8 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair t8 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);

                                    mk_pair q9 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r9 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s9 = std::make_pair(equations[4].xs[0], equations[4].ys[0]);
                                    mk_pair t9 = std::make_pair(equations[4].xs[size-1], equations[4].ys[size-1]);

                                    mk_pair q10 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r10 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s10 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair t10 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);

                                    mk_pair q11 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r11 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s11 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair t11 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);

                                    mk_pair q12 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r12 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s12 = std::make_pair(equations[4].xs[0], equations[4].ys[0]);
                                    mk_pair t12 = std::make_pair(equations[4].xs[size-1], equations[4].ys[size-1]);

                                    mk_pair q13 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r13 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s13 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair t13 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);

                                    mk_pair q14 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r14 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s14 = std::make_pair(equations[4].xs[0], equations[4].ys[0]);
                                    mk_pair t14 = std::make_pair(equations[4].xs[size-1], equations[4].ys[size-1]);

                                    mk_pair q15 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair r15 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);
                                    mk_pair s15 = std::make_pair(equations[4].xs[0], equations[4].ys[0]);
                                    mk_pair t15 = std::make_pair(equations[4].xs[size-1], equations[4].ys[size-1]);

                                    mk_pair q16 = std::make_pair(equations[0].xs[0], equations[0].ys[0]);
                                    mk_pair r16 = std::make_pair(equations[0].xs[size-1], equations[0].ys[size-1]);
                                    mk_pair s16 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t16 = std::make_pair(10, 0);

                                    mk_pair q17 = std::make_pair(equations[1].xs[0], equations[1].ys[0]);
                                    mk_pair r17 = std::make_pair(equations[1].xs[size-1], equations[1].ys[size-1]);
                                    mk_pair s17 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t17 = std::make_pair(10, 0);

                                    mk_pair q18 = std::make_pair(equations[2].xs[0], equations[2].ys[0]);
                                    mk_pair r18 = std::make_pair(equations[2].xs[size-1], equations[2].ys[size-1]);
                                    mk_pair s18 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t18 = std::make_pair(10, 0);

                                    mk_pair q19 = std::make_pair(equations[3].xs[0], equations[3].ys[0]);
                                    mk_pair r19 = std::make_pair(equations[3].xs[size-1], equations[3].ys[size-1]);
                                    mk_pair s19 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t19 = std::make_pair(10, 0);

                                    mk_pair q20 = std::make_pair(equations[4].xs[0], equations[4].ys[0]);
                                    mk_pair r20 = std::make_pair(equations[4].xs[size-1], equations[4].ys[size-1]);
                                    mk_pair s20 = std::make_pair(0.0f, 0.0f);
                                    mk_pair t20 = std::make_pair(10, 0);

                                    mk_pair inter = intersection(q, r, s, t);
                                    mk_pair inter2 = intersection(q2, r2, s2, t2);
                                    mk_pair inter3 = intersection(q3, r3, s3, t3);
                                    mk_pair inter4 = intersection(q4, r4, s4, t4);
                                    mk_pair inter5 = intersection(q5, r5, s5, t5);
                                    mk_pair inter6 = intersection(q6, r6, s6, t6);
                                    mk_pair inter7 = intersection(q7, r7, s7, t7);
                                    mk_pair inter8 = intersection(q8, r8, s8, t8);
                                    mk_pair inter9 = intersection(q9, r9, s9, t9);
                                    mk_pair inter10 = intersection(q10, r10, s10, t10);
                                    mk_pair inter11 = intersection(q11, r11, s11, t11);
                                    mk_pair inter12 = intersection(q12, r12, s12, t12);
                                    mk_pair inter13 = intersection(q13, r13, s13, t13);
                                    mk_pair inter14 = intersection(q14, r14, s14, t14);
                                    mk_pair inter15 = intersection(q15, r15, s15, t15);
                                    mk_pair inter16 = intersection(q16, r16, s16, t16);
                                    mk_pair inter17 = intersection(q17, r17, s17, t17);
                                    mk_pair inter18 = intersection(q18, r18, s18, t18);
                                    mk_pair inter19 = intersection(q19, r19, s19, t19);
                                    mk_pair inter20 = intersection(q20, r20, s20, t20);

                                    if (calculateEquasion(equations[0].A, inter.first, equations[0].operator_, equations[0].B, inter.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter.first, equations[1].operator_, equations[1].B, inter.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter.first, equations[2].operator_, equations[2].B, inter.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter.first, equations[3].operator_, equations[3].B, inter.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter.first, equations[4].operator_, equations[4].B, inter.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter.first == FLT_MAX && inter.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "1 The intersection of the given lines AB and CD is: ";
                                            display(inter);
                                            xs2[dotIndex] = inter.first;
                                            ys2[dotIndex] = inter.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter2.first, equations[0].operator_, equations[0].B, inter2.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter2.first, equations[1].operator_, equations[1].B, inter2.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter2.first, equations[2].operator_, equations[2].B, inter2.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter2.first, equations[3].operator_, equations[3].B, inter2.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter2.first, equations[4].operator_, equations[4].B, inter2.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter2.first == FLT_MAX && inter2.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "2 The intersection of the given lines AB and CD is: ";
                                            display(inter2);
                                            xs2[dotIndex] = inter2.first;
                                            ys2[dotIndex] = inter2.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter3.first, equations[0].operator_, equations[0].B, inter3.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter3.first, equations[1].operator_, equations[1].B, inter3.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter3.first, equations[2].operator_, equations[2].B, inter3.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter3.first, equations[3].operator_, equations[3].B, inter3.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter3.first, equations[4].operator_, equations[4].B, inter3.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter3.first == FLT_MAX && inter3.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "3 The intersection of the given lines AB and CD is: ";
                                            display(inter3);
                                            xs2[dotIndex] = inter3.first;
                                            ys2[dotIndex] = inter3.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter4.first, equations[0].operator_, equations[0].B, inter4.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter4.first, equations[1].operator_, equations[1].B, inter4.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter4.first, equations[2].operator_, equations[2].B, inter4.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter4.first, equations[3].operator_, equations[3].B, inter4.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter4.first, equations[4].operator_, equations[4].B, inter4.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter4.first == FLT_MAX && inter4.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "4 The intersection of the given lines AB and CD is: ";
                                            display(inter4);
                                            xs2[dotIndex] = inter4.first;
                                            ys2[dotIndex] = inter4.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter5.first, equations[0].operator_, equations[0].B, inter5.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter5.first, equations[1].operator_, equations[1].B, inter5.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter5.first, equations[2].operator_, equations[2].B, inter5.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter5.first, equations[3].operator_, equations[3].B, inter5.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter5.first, equations[4].operator_, equations[4].B, inter5.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter5.first == FLT_MAX && inter5.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "5 The intersection of the given lines AB and CD is: ";
                                            display(inter5);
                                            xs2[dotIndex] = inter5.first;
                                            ys2[dotIndex] = inter5.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter6.first, equations[0].operator_, equations[0].B, inter6.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter6.first, equations[1].operator_, equations[1].B, inter6.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter6.first, equations[2].operator_, equations[2].B, inter6.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter6.first, equations[3].operator_, equations[3].B, inter6.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter6.first, equations[4].operator_, equations[4].B, inter6.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter6.first == FLT_MAX && inter6.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "6 The intersection of the given lines AB and CD is: ";
                                            display(inter6);
                                            xs2[dotIndex] = inter6.first;
                                            ys2[dotIndex] = inter6.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter7.first, equations[0].operator_, equations[0].B, inter7.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter7.first, equations[1].operator_, equations[1].B, inter7.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter7.first, equations[2].operator_, equations[2].B, inter7.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter7.first, equations[3].operator_, equations[3].B, inter7.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter7.first, equations[4].operator_, equations[4].B, inter7.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter7.first == FLT_MAX && inter7.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "7 The intersection of the given lines AB and CD is: ";
                                            display(inter7);
                                            xs2[dotIndex] = inter7.first;
                                            ys2[dotIndex] = inter7.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter8.first, equations[0].operator_, equations[0].B, inter8.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter8.first, equations[1].operator_, equations[1].B, inter8.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter8.first, equations[2].operator_, equations[2].B, inter8.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter8.first, equations[3].operator_, equations[3].B, inter8.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter8.first, equations[4].operator_, equations[4].B, inter8.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter8.first == FLT_MAX && inter8.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "8 The intersection of the given lines AB and CD is: ";
                                            display(inter8);
                                            xs2[dotIndex] = inter8.first;
                                            ys2[dotIndex] = inter8.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter9.first, equations[0].operator_, equations[0].B, inter9.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter9.first, equations[1].operator_, equations[1].B, inter9.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter9.first, equations[2].operator_, equations[2].B, inter9.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter9.first, equations[3].operator_, equations[3].B, inter9.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter9.first, equations[4].operator_, equations[4].B, inter9.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter9.first == FLT_MAX && inter9.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "9 The intersection of the given lines AB and CD is: ";
                                            display(inter9);
                                            xs2[dotIndex] = inter9.first;
                                            ys2[dotIndex] = inter9.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter10.first, equations[0].operator_, equations[0].B, inter10.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter10.first, equations[1].operator_, equations[1].B, inter10.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter10.first, equations[2].operator_, equations[2].B, inter10.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter10.first, equations[3].operator_, equations[3].B, inter10.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter10.first, equations[4].operator_, equations[4].B, inter10.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter10.first == FLT_MAX && inter10.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "10 The intersection of the given lines AB and CD is: ";
                                            display(inter10);
                                            xs2[dotIndex] = inter10.first;
                                            ys2[dotIndex] = inter10.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter11.first, equations[0].operator_, equations[0].B, inter11.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter11.first, equations[1].operator_, equations[1].B, inter11.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter11.first, equations[2].operator_, equations[2].B, inter11.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter11.first, equations[3].operator_, equations[3].B, inter11.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter11.first, equations[4].operator_, equations[4].B, inter11.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter11.first == FLT_MAX && inter11.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "11 The intersection of the given lines AB and CD is: ";
                                            display(inter11);
                                            xs2[dotIndex] = inter11.first;
                                            ys2[dotIndex] = inter11.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter12.first, equations[0].operator_, equations[0].B, inter12.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter12.first, equations[1].operator_, equations[1].B, inter12.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter12.first, equations[2].operator_, equations[2].B, inter12.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter12.first, equations[3].operator_, equations[3].B, inter12.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter12.first, equations[4].operator_, equations[4].B, inter12.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter12.first == FLT_MAX && inter12.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "12 The intersection of the given lines AB and CD is: ";
                                            display(inter12);
                                            xs2[dotIndex] = inter12.first;
                                            ys2[dotIndex] = inter12.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter13.first, equations[0].operator_, equations[0].B, inter13.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter13.first, equations[1].operator_, equations[1].B, inter13.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter13.first, equations[2].operator_, equations[2].B, inter13.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter13.first, equations[3].operator_, equations[3].B, inter13.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter13.first, equations[4].operator_, equations[4].B, inter13.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter13.first == FLT_MAX && inter13.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "13 The intersection of the given lines AB and CD is: ";
                                            display(inter13);
                                            xs2[dotIndex] = inter13.first;
                                            ys2[dotIndex] = inter13.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter14.first, equations[0].operator_, equations[0].B, inter14.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter14.first, equations[1].operator_, equations[1].B, inter14.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter14.first, equations[2].operator_, equations[2].B, inter14.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter14.first, equations[3].operator_, equations[3].B, inter14.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter14.first, equations[4].operator_, equations[4].B, inter14.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter14.first == FLT_MAX && inter14.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "14 The intersection of the given lines AB and CD is: ";
                                            display(inter14);
                                            xs2[dotIndex] = inter14.first;
                                            ys2[dotIndex] = inter14.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter15.first, equations[0].operator_, equations[0].B, inter15.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter15.first, equations[1].operator_, equations[1].B, inter15.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter15.first, equations[2].operator_, equations[2].B, inter15.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter15.first, equations[3].operator_, equations[3].B, inter15.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter15.first, equations[4].operator_, equations[4].B, inter15.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter15.first == FLT_MAX && inter15.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "15 The intersection of the given lines AB and CD is: ";
                                            display(inter15);
                                            xs2[dotIndex] = inter15.first;
                                            ys2[dotIndex] = inter15.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter16.first, equations[0].operator_, equations[0].B, inter16.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter16.first, equations[1].operator_, equations[1].B, inter16.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter16.first, equations[2].operator_, equations[2].B, inter16.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter16.first, equations[3].operator_, equations[3].B, inter16.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter16.first, equations[4].operator_, equations[4].B, inter16.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter16.first == FLT_MAX && inter16.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "16 The intersection of the given lines AB and CD is: ";
                                            display(inter16);
                                            xs2[dotIndex] = inter16.first;
                                            ys2[dotIndex] = inter16.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter17.first, equations[0].operator_, equations[0].B, inter17.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter17.first, equations[1].operator_, equations[1].B, inter17.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter17.first, equations[2].operator_, equations[2].B, inter17.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter17.first, equations[3].operator_, equations[3].B, inter17.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter17.first, equations[4].operator_, equations[4].B, inter17.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter17.first == FLT_MAX && inter17.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "17 The intersection of the given lines AB and CD is: ";
                                            display(inter17);
                                            xs2[dotIndex] = inter17.first;
                                            ys2[dotIndex] = inter17.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter18.first, equations[0].operator_, equations[0].B, inter18.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter18.first, equations[1].operator_, equations[1].B, inter18.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter18.first, equations[2].operator_, equations[2].B, inter18.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter18.first, equations[3].operator_, equations[3].B, inter18.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter18.first, equations[4].operator_, equations[4].B, inter18.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter18.first == FLT_MAX && inter18.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "18 The intersection of the given lines AB and CD is: ";
                                            display(inter18);
                                            xs2[dotIndex] = inter18.first;
                                            ys2[dotIndex] = inter18.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter19.first, equations[0].operator_, equations[0].B, inter19.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter19.first, equations[1].operator_, equations[1].B, inter19.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter19.first, equations[2].operator_, equations[2].B, inter19.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter19.first, equations[3].operator_, equations[3].B, inter19.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter19.first, equations[4].operator_, equations[4].B, inter19.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter19.first == FLT_MAX && inter19.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "19 The intersection of the given lines AB and CD is: ";
                                            display(inter19);
                                            xs2[dotIndex] = inter19.first;
                                            ys2[dotIndex] = inter19.second;
                                            dotIndex++;
                                        }
                                    }

                                    if (calculateEquasion(equations[0].A, inter20.first, equations[0].operator_, equations[0].B, inter20.second, equations[0].jednakost, equations[0].C) && calculateEquasion(equations[1].A, inter20.first, equations[1].operator_, equations[1].B, inter20.second, equations[1].jednakost, equations[1].C) && calculateEquasion(equations[2].A, inter20.first, equations[2].operator_, equations[2].B, inter20.second, equations[2].jednakost, equations[2].C) && calculateEquasion(equations[3].A, inter20.first, equations[3].operator_, equations[3].B, inter20.second, equations[3].jednakost, equations[3].C) && calculateEquasion(equations[4].A, inter20.first, equations[4].operator_, equations[4].B, inter20.second, equations[4].jednakost, equations[4].C))
                                    {
                                        if (inter20.first == FLT_MAX && inter20.second == FLT_MAX) {
                                            std::cout << "The given lines AB and CD are parallel.\n";
                                        }
                                        else {
                                            std::cout << "20 The intersection of the given lines AB and CD is: ";
                                            display(inter20);
                                            xs2[dotIndex] = inter20.first;
                                            ys2[dotIndex] = inter20.second;
                                            dotIndex++;
                                        }
                                    }

                                    draw = true;
                                }
                            }
                            if (draw)
                            {
                                for (int i = 0; i < dotIndex; i++)
                                {
                                    std::string text = "(";
                                    text += to_string_with_precision(xs2[i]);
                                    text += ",";
                                    text += to_string_with_precision(ys2[i]);
                                    text += ")";
                                    ImPlotContext& gp = *GImPlot;
                                    ImPlotPlot& plot = *gp.CurrentPlot;
                                    ImPlotAxis& x_axis = plot.XAxis(ImAxis_Y1);
                                    int num_items = gp.CurrentPlot->Items.GetLegendCount();
                                    bool draw_label = false;
                                    for (int j = 0; j < num_items; j++)
                                    {
                                        std::string itemName(gp.CurrentPlot->Items.GetLegendLabel(j));
                                        if (itemName.find("Intersections") != std::string::npos)
                                        {
                                            ImPlotItem* item = gp.CurrentPlot->Items.GetLegendItem(j);
                                            if (item->Show)
                                                draw_label = true;
                                        }
                                    }
                                    if (draw_label)
                                        ImPlot::PlotText(text.c_str(), xs2[i], ys2[i] + (fabsf(x_axis.Range.Max - x_axis.Range.Min) / 8.0f) / 2.0f);
                                }
                                ImPlot::PlotScatter("Intersections", xs2, ys2, dotIndex);
                            }
                        }
                    }
                    else
                        setDots = false;
                }
                ImPlot::EndPlot();
            }
        }ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("SolutionChild", ImVec2(ImGui::GetIO().DisplaySize.x / 3, ImGui::GetIO().DisplaySize.y / 1.9), true);
        {
            ImGui::BeginChild("Z", ImVec2(400, 40), true);
            {
                ImGui::Text("Z = ");
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::InputFloat("x", &problem.A, 0, 0, "%.1f"); ImGui::SameLine();
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::Text("+");
                ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);  ImGui::InputFloat("y", &problem.B, 0, 0, "%.1f"); ImGui::SameLine();
            }ImGui::EndChild();
            const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
            ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
            static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
            if (ImGui::BeginTable("Solutions", 3, flags, outer_size))
            {
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                ImGui::TableSetupColumn("Corner Point", ImGuiTableColumnFlags_None);
                std::string text = "Z = ";
                text += to_string_with_precision(problem.A);
                text += "x + ";
                text += to_string_with_precision(problem.B);
                text += "y";
                ImGui::TableSetupColumn(text.c_str(), ImGuiTableColumnFlags_None);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                clipper.Begin(dotIndex);
                float min = problem.A * xs2[0] + problem.B * ys2[0], max = problem.A * xs2[0] + problem.B * ys2[0];

                for (int i = 0; i < dotIndex; i++)
                {
                    float Z = problem.A * xs2[i] + problem.B * ys2[i];
                    if (Z > max)
                        max = Z;
                    if (Z < min)
                        min = Z;
                }
                while (clipper.Step())
                {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                    {  
                        ImGui::TableNextRow();
                        for (int column = 0; column < 3; column++)
                        {  
                            ImGui::TableSetColumnIndex(column);
                            if (column == 0)
                            {
                                ImGui::Text("(%.2f, %.2f)", xs2[row], ys2[row]);
                            }
                            if (column == 1)
                            {
                                float Z = problem.A * xs2[row] + problem.B * ys2[row];
                                ImGui::Text("%.2f", Z);
                            }
                        }
                        ImGui::TableSetColumnIndex(2);

                        float Z = problem.A * xs2[row] + problem.B * ys2[row];
                        if (Z == max)
                        {
                            ImGui::Text("MAX");
                        }
                        else if (Z == min)
                        {
                            ImGui::Text("MIN");
                        }
                        else
                        {
                            ImGui::Text("");
                        }
                    }
                }
                //ImGui::Text("MAX %f", max);
                //ImGui::Text("MIN %f", min);
                ImGui::EndTable();
            }
        }ImGui::EndChild();
        
        if (ImGui::Button("Solve"))
        {
            once = true;
            setDots = true;
        }
        ImGui::End();
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*clear_color.w*255.0f), (int)(clear_color.y*clear_color.w*255.0f), (int)(clear_color.z*clear_color.w*255.0f), (int)(clear_color.w*255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);


        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImPlot::DestroyContext();
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}


bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
