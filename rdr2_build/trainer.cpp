#include <windows.h>
#include <cstdio>
#include <cmath>
#include "types.h"
#include "natives.h"
#include "main.h"

static bool g_menu = false;
static int g_index = 0;
static bool g_god = false;
static bool g_invisible = false;
static bool g_noRagdoll = false;

static bool Pressed(int vk) {
    static SHORT prev[256]{};
    SHORT now = GetAsyncKeyState(vk);
    bool down = (now & 0x8000) != 0;
    bool was = (prev[vk] & 0x8000) != 0;
    prev[vk] = now;
    return down && !was;
}

static void DrawText(const char* text, float x, float y, float sx = 0.32f, float sy = 0.32f, bool centered = false) {
    UI::SET_TEXT_COLOR_RGBA(255, 255, 255, 255);
    UI::SET_TEXT_SCALE(sx, sy);
    UI::SET_TEXT_CENTRE(centered);
    const char* literal = GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", text);
    UI::DRAW_TEXT(literal, x, y);
}

static void DrawRect(float x, float y, float w, float h, int r, int g, int b, int a) {
    GRAPHICS::DRAW_RECT(x + w / 2.0f, y + h / 2.0f, w, h, r, g, b, a, true);
}

static void HealPlayer() {
    Ped ped = PLAYER::PLAYER_PED_ID();
    if (!ENTITY::DOES_ENTITY_EXIST(ped)) return;
    ENTITY::SET_ENTITY_HEALTH(ped, ENTITY::GET_ENTITY_MAX_HEALTH(ped, 0), 0);
}

static void TeleportForward() {
    Ped ped = PLAYER::PLAYER_PED_ID();
    if (!ENTITY::DOES_ENTITY_EXIST(ped)) return;
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, 0, 0);
    float heading = ENTITY::GET_ENTITY_HEADING(ped);
    const float pi = 3.14159265358979323846f;
    pos.x += std::sin(-heading * (pi / 180.0f)) * 20.0f;
    pos.y += std::cos( heading * (pi / 180.0f)) * 20.0f;
    ENTITY::SET_ENTITY_COORDS(ped, pos.x, pos.y, pos.z, 1, 0, 1, 0);
}

static void FeatureTick() {
    Ped ped = PLAYER::PLAYER_PED_ID();
    if (!ENTITY::DOES_ENTITY_EXIST(ped)) return;

    PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), g_god);
    if (g_god) {
        ENTITY::SET_ENTITY_HEALTH(ped, ENTITY::GET_ENTITY_MAX_HEALTH(ped, 0), 0);
    }

    ENTITY::SET_ENTITY_VISIBLE(ped, !g_invisible);
    PED::SET_PED_CAN_RAGDOLL(ped, !g_noRagdoll);
}

struct MenuItem {
    const char* name;
    int type; // 0 toggle, 1 action
    bool* toggle;
    void (*action)();
};

static MenuItem g_items[] = {
    {"God Mode", 0, &g_god, nullptr},
    {"Invisible", 0, &g_invisible, nullptr},
    {"No Ragdoll", 0, &g_noRagdoll, nullptr},
    {"Heal Player", 1, nullptr, HealPlayer},
    {"Teleport Forward 20m", 1, nullptr, TeleportForward},
};

static const int g_itemCount = sizeof(g_items) / sizeof(g_items[0]);

static void DrawMenu() {
    if (!g_menu) return;

    const float x = 0.035f;
    const float y = 0.08f;
    const float w = 0.29f;
    const float row = 0.042f;

    DrawRect(x, y - 0.055f, w, 0.055f, 95, 15, 15, 235);
    DrawText("RDR2 ULTIMATE TRAINER", x + 0.012f, y - 0.045f, 0.38f, 0.38f, false);

    DrawRect(x, y, w, row * g_itemCount + 0.018f, 8, 8, 8, 220);

    for (int i = 0; i < g_itemCount; ++i) {
        float iy = y + 0.009f + row * i;
        if (i == g_index) DrawRect(x + 0.006f, iy - 0.002f, w - 0.012f, row - 0.004f, 110, 25, 25, 190);

        char label[128]{};
        if (g_items[i].type == 0) {
            std::snprintf(label, sizeof(label), "%s   [%s]", g_items[i].name, *g_items[i].toggle ? "ON" : "OFF");
        } else {
            std::snprintf(label, sizeof(label), "%s", g_items[i].name);
        }
        DrawText(label, x + 0.014f, iy, 0.31f, 0.31f, false);
    }

    DrawText("F5 Close   Arrows Navigate   Enter Select", x + 0.012f, y + row * g_itemCount + 0.014f, 0.24f, 0.24f, false);
}

static void InputTick() {
    if (Pressed(VK_F5)) g_menu = !g_menu;
    if (!g_menu) return;

    if (Pressed(VK_UP) || Pressed(VK_NUMPAD8)) {
        --g_index;
        if (g_index < 0) g_index = g_itemCount - 1;
    }
    if (Pressed(VK_DOWN) || Pressed(VK_NUMPAD2)) {
        ++g_index;
        if (g_index >= g_itemCount) g_index = 0;
    }
    if (Pressed(VK_BACK) || Pressed(VK_NUMPAD0)) g_menu = false;

    if (Pressed(VK_RETURN) || Pressed(VK_NUMPAD5)) {
        MenuItem& item = g_items[g_index];
        if (item.type == 0 && item.toggle) *item.toggle = !*item.toggle;
        if (item.type == 1 && item.action) item.action();
    }
}

void ScriptMain() {
    while (true) {
        InputTick();
        FeatureTick();
        DrawMenu();
        WAIT(0);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            scriptRegister(hModule, ScriptMain);
            break;
        case DLL_PROCESS_DETACH:
            scriptUnregister(hModule);
            break;
    }
    return TRUE;
}
