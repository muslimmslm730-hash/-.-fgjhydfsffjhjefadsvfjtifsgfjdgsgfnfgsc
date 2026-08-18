#include "ICON/Includes.h"
#include "ICON/obfuscate.h"
#include "ICON/Tools.h"
#include "ICON/fake_dlfcn.h"
#include "ICON/Vector3.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "ICON/StrEnc.h"
#include "ICON/plthook.h"
#include "ICON/Arabic.h"
#include "ICON/KittyMemory/MemoryPatch.h"
#include "ICON/MemoryTools.h"
#include "ICON/base64/base64.h"
#include "ICON/Iconcpp.h"
#include "ICON/ImguiPP.cpp"
#include "ICON/Menu.h"
#include <dlfcn.h>
#include "ICON/Font.h"
#include "ICON/Fonts.h"
#include "ICON/Icon.h"/*
namespace settings {
 inline ImVec4 particleColour = ImVec4(0.15f, 0.15f, 0.15f, 255);  // dark gray line particles
}*/
static int MenuTab = 1;
static int EspTab = 0; 
bool GAME; 
bool LOBBY; 

//#include "xhook/xhook.c"
#include "ICON/Includes/Toast.hpp"
#include "ICON/Includes/Logger.h"
#include "ICON/Includes/Utils.h"
#include "ICON/Includes/Macros.h"
//#include "Arabic.h"
#include "ICON/Rect.h"
#include "ICON/json.hpp"
#include "ICON/Items.h"
#include "ICON/fontch.h"
#include "ICON/fontch1.h"
#include "ICON/XorStr.h"


static float tab_alpha = 1.0f;
static int current_tab = 0;
static int target_tab = 0;

std::ifstream file("Coder/Coder.json");
using json = nlohmann::json;


// ===================== LANGUAGE SUPPORT =====================
// ImGui does not perform full Arabic BiDi shaping by itself.
// Arabic labels below are intentionally stored in display order so they
// render correctly with the existing font setup used by this project.
static bool gArabicLanguage = false;
static inline const char* L(const char* en, const char* ar_display)
{
    return gArabicLanguage ? ar_display : en;
}
// ============================================================
#define SLEEP_TIME 1000LL / 60LL
#include "SDK.hpp"
using namespace SDK;
#include <curl/curl.h>
#include "imgui/Image.hpp"
//#include "hooks.cpp"
char extra[32];

using json = nlohmann::json;


// ── Toggle Switch: [ON/OFF]  Label ────────────────────────────────
static void ToggleSwitch(const char* label, bool* v)
{
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2      pos = ImGui::GetCursorScreenPos();

    // ── dimensions ──
    const float switch_w = 52.0f;   // width of the switch track
    const float switch_h = 26.0f;   // height of the switch track
    const float knob_r   = 10.0f;   // radius of the knob
    const float gap      = 10.0f;   // gap between switch and text

    // ── smooth animation ──
    static std::map<const char*, float> s_sw_anim;
    float& anim = s_sw_anim[label];
    float target = *v ? 1.0f : 0.0f;
    float dt = ImGui::GetIO().DeltaTime;
    anim += (target - anim) * (1.0f - expf(-dt * 14.0f));

    // ── colors (lerp between OFF red/gray and ON green/cyan) ──
    auto lerpU8 = [](int a, int b, float t) -> int { return (int)(a + (b - a) * t); };

    // Track background
    int track_r = lerpU8(60,  0,   anim);
    int track_g = lerpU8(60,  180, anim);
    int track_b = lerpU8(60,  80,  anim);
    int track_a = lerpU8(180, 200, anim);

    // Track border
    int bd_r = lerpU8(120, 0,   anim);
    int bd_g = lerpU8(120, 200, anim);
    int bd_b = lerpU8(120, 100, anim);

    // Knob
    int kn_r = lerpU8(200, 50,  anim);
    int kn_g = lerpU8(60,  220, anim);
    int kn_b = lerpU8(60,  80,  anim);

    // Glow
    int gl_r = *v ? 0   : 220;
    int gl_g = *v ? 200 : 0;
    int gl_b = *v ? 70  : 0;
    int gl_a = (int)(45 * (0.7f + 0.3f * sinf((float)ImGui::GetTime() * 2.5f)));

    ImU32 cTrack   = IM_COL32(track_r, track_g, track_b, track_a);
    ImU32 cBorder  = IM_COL32(bd_r, bd_g, bd_b, 220);
    ImU32 cKnob    = IM_COL32(kn_r, kn_g, kn_b, 255);
    ImU32 cGlow    = IM_COL32(gl_r, gl_g, gl_b, gl_a);
    ImU32 cON      = IM_COL32(100, 255, 140, 240);
    ImU32 cOFF     = IM_COL32(255, 140, 140, 200);

    // ── track position ──
    float cx = pos.x + switch_w * 0.5f;
    float cy = pos.y + switch_h * 0.5f;

    // ── 1. glow behind track ──
    dl->AddRectFilled(
        ImVec2(pos.x - 3, pos.y - 3),
        ImVec2(pos.x + switch_w + 3, pos.y + switch_h + 3),
        cGlow, 16.0f);

    // ── 2. track background ──
    dl->AddRectFilled(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + switch_w, pos.y + switch_h),
        cTrack, 13.0f);

    // ── 3. track border ──
    dl->AddRect(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + switch_w, pos.y + switch_h),
        cBorder, 13.0f, 0, 1.8f);

    // ── 4. knob (slides left/right) ──
    float knob_center_x = pos.x + 13.0f + (switch_w - 26.0f) * anim;
    float knob_center_y = pos.y + switch_h * 0.5f;

    // knob shadow
    dl->AddCircleFilled(
        ImVec2(knob_center_x + 1.5f, knob_center_y + 1.5f),
        knob_r + 0.5f, IM_COL32(0, 0, 0, 70), 32);

    // knob fill
    dl->AddCircleFilled(
        ImVec2(knob_center_x, knob_center_y),
        knob_r, cKnob, 32);

    // knob rim
    dl->AddCircle(
        ImVec2(knob_center_x, knob_center_y),
        knob_r, IM_COL32(255, 255, 255, 100), 32, 1.5f);

    // ── 5. ON / OFF text inside knob area or on track ──
    const char* statusText = *v ? "" : "";
    ImVec2 stSize = ImGui::CalcTextSize(statusText);
    float st_x = *v
        ? pos.x + switch_w - stSize.x - 6.0f   // ON on right side
        : pos.x + 6.0f;                         // OFF on left side
    float st_y = pos.y + (switch_h - stSize.y) * 0.5f;
    dl->AddText(NULL, 0, ImVec2(st_x, st_y), *v ? cON : cOFF, statusText);

    // ── 6. label text (right side) ──
    float lx = pos.x + switch_w + gap;
    float ly = pos.y + (switch_h - ImGui::GetTextLineHeight()) * 0.5f;
    ImGui::SetCursorScreenPos(ImVec2(lx, ly));
    ImGui::PushStyleColor(ImGuiCol_Text,
        *v ? ImVec4(0.65f, 1.00f, 0.75f, 1.0f)
           : ImVec4(1.00f, 0.60f, 0.60f, 1.0f));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    // ── 7. hitbox ──
    ImGui::SetCursorScreenPos(pos);
    float tw = ImGui::CalcTextSize(label).x;
    std::string btnId = "##ts_" + std::string(label);
    ImGui::InvisibleButton(btnId.c_str(),
        ImVec2(switch_w + gap + tw + 8.0f, switch_h));

    if (ImGui::IsItemClicked())
        *v = !(*v);

    // hover glow
    if (ImGui::IsItemHovered())
        dl->AddRect(
            ImVec2(pos.x - 2, pos.y - 2),
            ImVec2(pos.x + switch_w + 2, pos.y + switch_h + 2),
            IM_COL32(255, 255, 255, 30), 13.0f, 0, 2.0f);
}
//========{{{{ Offset }}}}=========///
#define GNames_Offset 0x8939470
#define GEngine_Offset 0xf1d5f70 //UEngine
#define GEngine_Offset 0xf1d5f30 //UEngine
#define GUObject_Offset 0xef3b3f0
#define GNativeAndroidApp_Offset 0xec732a8
#define GetActorArrary_Offset 0xa6ca440
#define ProcessEvent_Offset 0x8bac0a8 //Child
//#define ProcessEvent_Offset 0xa211d60 //Main
#define Actors_Offset 0xa0
#define SLEEP_TIME 1000LL / 60LL
//=====================================//
//static std::string mod_status = " ";
//static std::string modname = " ";
//static std::string credit = " ";
//static std::string key = " ";
std::string expiredDate = " ";
//std::string EXP = " NO KEY ";
std::string g_Token, g_Auth;
bool bValid = false;
json items_data;
//=====================================//
bool BYPASS;
//=====================================//
ImFont* real;
ImFont* Arabic;
ImFont* flamee;
float size_child = 0;
static float SliderFloat = 0;
static float isRed = 0.0f, isGreen = 0.01f, isBlue = 0.0f;
//=====================================//
// ==================================Define HOOK====================================== //
#define targetLibName OBFUSCATE("libanort.so")
#define targetLibName OBFUSCATE("libanogs.so")
#define targetLibName OBFUSCATE("libUE4.so")
#define targetLibName OBFUSCATE("libTDataMaster.so")
#define targetLibName OBFUSCATE("libINTLFoundation.so")
#define targetLibName OBFUSCATE("libijkffmpeg.so")
#define targetLibName OBFUSCATE("libCrashSight.so")
#define targetLibName OBFUSCATE("libGCloudVoice.so")
#define targetLibName OBFUSCATE("libmarsxlog.so")
#define targetLibName OBFUSCATE("libgcloud.so")
#define targetLibName OBFUSCATE("libtgpa.so")
#define _BYTE  uint8_t
#define _WORD  uint16_t
#define _DWORD uint32_t
#define _QWORD uint64_t

#define CS
struct patch_t
{
_BYTE nPatchType;
DWORD dwAddress;
};

DWORD libanogsBase = 0;
DWORD libUE4Base = 0;
DWORD libanortBase = 0;
DWORD libEGLBase = 0;
DWORD libanogsAlloc = 0;
DWORD libUE4Alloc = 0;
DWORD libEGLAlloc = 0;
unsigned int libanogsSize = 0x33547D;
unsigned int libUE4Size = 0x7D56590;
char *Offsett;
DWORD NewBase = 0;
size_t Sizeofa3;


static std::unordered_set<uint32_t> AlreadyChangedSet;
uintptr_t GetVirtualFunctionAddress(uintptr_t clazz, uintptr_t index) {
if (!clazz) {return 0;}
uintptr_t vtablePtr = *(uintptr_t*)clazz;
if (!vtablePtr) {return 0;}
if (index < 0) {return 0;}
return *((uintptr_t*)vtablePtr + index);
}



void ChangeItemAVc(uintptr_t thiz, int InItemID) {
if (thiz) {
auto PrechangeitemAvatar_addr = GetVirtualFunctionAddress(thiz, 183); //DeadBoxAvatar index
if (PrechangeitemAvatar_addr) {
return ((void(*)(uintptr_t, int, bool))PrechangeitemAvatar_addr)(thiz, InItemID, true);
}}}
// ==================================END HOOK====================================== //

uintptr_t anort;
uintptr_t UE4;
uintptr_t g_UE4;
uintptr_t ANOGS;
uintptr_t Anogs;
uintptr_t g_Anogs;
uintptr_t anogs;
uintptr_t g_anogs;
uintptr_t TDataMaster;
uintptr_t TDatamaster;
uintptr_t swappy;
uintptr_t CrashSight;
uintptr_t gcloudcore;
uintptr_t cubehawk;
uintptr_t GCloudVoice;
uintptr_t tprt;
uintptr_t INTLCompliance;
uintptr_t ANORT;
uintptr_t gcloud;
uintptr_t g_cloud;
uintptr_t g_gcloud;
uintptr_t Anort;
uintptr_t tgpa;
uintptr_t g_ijkffmpeg;
float screenSizeX = 0;
float screenSizeY = 0;
android_app *g_App = 0;
ASTExtraPlayerCharacter *g_LocalPlayer = 0;
ASTExtraPlayerController *g_LocalController = 0;
bool ModSkinn;
bool KillMessage;
bool DeadBox; 
bool HideName;
#include "Mod/jsonPreferences.h"
int sEmote1 = 2200101;
int sEmote2 = 2200201;
int sEmote3 = 2200301;
static int helmett3 = 0;
static int bag3 = 0;
int ModEmote1 = 0;
namespace Active {
inline int SkinCarDefault = 0;
inline int SkinCarMod = 0;
inline int SkinCarNew = 0;
}
namespace CauserDeadBox {
inline bool Active = false;
inline std::string KillByWeaponID = "";
inline int CurBulletNumInClipNew = 0;
inline int CurBulletNumInClipLast = 0;
inline UDeadBoxAvatarComponent * DeadBoxPointer = 0;
}
#include "Mod/Menu.h"
bool show;
bool HIDEESP = true;
bool initImGui = false;
bool LOGO = true;; 

static bool HIDEWINDOW1 = true;
static bool HIDEWINDOW = true;

int screenWidth = -1, glWidth, screenHeight = -1, glHeight;
float density = -1;
json mItemData; 
//=============///
std::map<int, bool> Items;
std::map<int, float *> ItemColors;

enum EAimTarget {
Head = 0,
Chest = 1,
调节 = 2
};

enum EAimTrigger {
Shooting = 0,
None = 1,
Scoping = 2,
Both = 3,
Any = 4
};



struct sConfig {
bool Bypass = true;
bool Island;
bool killmessage;
bool DeadBox; 
struct sPlayerESP {
bool Line;
bool Box;
bool Health;
bool HealthPC; 
bool GameInfo;
bool GameInfo1;
bool Skeleton;
bool Name;
bool Alert; 
bool Distance;
bool TeamID;
bool Grenade;
bool Vehicle;
bool NoBot;
bool Country;
bool HitEffect;
bool killmessage;
bool DeadBox; 
bool Shake;
bool Ipad;
bool FPS;
bool Small;
bool RGbcro; 
bool HDR;
bool cross;
bool LESS;
bool Weapon;
float Hit;
float Object;
};
sPlayerESP PlayerESP{0};
struct sAimMenu {
bool Enable;
bool Enable1;
EAimTarget Target;
EAimTrigger Trigger;
float Cross;
float Recc;
float Meter;
float Position;
bool IgnoreKnocked;
bool VisCheck;
bool IgnoreBot;
bool Shooting;

};
sAimMenu AimBot{0};
sAimMenu SilentAim{0};

struct sColorsESP {
float *Fov;
};
sColorsESP ColorsESP{0};

struct sOTHER {
bool HIDEESP;
};
sOTHER OTHER{0};
};
sConfig Config{0};
// ======================================================================== //
bool WriteAddr(void *addr, void *buffer, size_t length) {
unsigned long page_size = sysconf(_SC_PAGESIZE);
unsigned long size = page_size * sizeof(uintptr_t);
return mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy(addr, buffer, length) != 0;}
template<typename T>
void Write(uintptr_t addr, T value) {WriteAddr((void *) addr, &value, sizeof(T));}
struct Range {std::string internalName; std::string type; int start;};
std::vector<Range> getRangesList(std::string path) {}
int Read(std::string module, std::string type) {std::vector<Range> Ranges = getRangesList("/");
for (auto& v : Ranges) {if (v.internalName.substr(v.internalName.find_last_of("/") + 1) == module && v.type == type) {return v.start;}}}
struct TableEntry {int address; int value; int flags;};
std::vector<TableEntry> Table;
void Modify(int address, int value, int flags) {Table.push_back({address, value, flags});}
// ======================================================================== //
#define CREATE_COLOR(r, g, b, a) new float[4] {(float)r, (float)g, (float)b, (float)a};

static UEngine *GEngine = 0;
UWorld *GetWorld() {
while (!GEngine) {GEngine = UObject::FindObject<UEngine>("UAEGameEngine Transient.UAEGameEngine_1"); 
sleep(1); }
if (GEngine)  {auto ViewPort = GEngine->GameViewport;
if (ViewPort) {return ViewPort->World;
}}return 0;
}

TNameEntryArray *GetGNames() {return ((TNameEntryArray *(*)()) (UE4 + GNames_Offset))(); }

std::vector<AActor *> getActors() {
auto World = GetWorld();
if (!World)
return std::vector<AActor *>();
auto PersistentLevel = World->PersistentLevel;
if (!PersistentLevel)
return std::vector<AActor *>();
auto Actors = *(TArray<AActor *> *)((uintptr_t) PersistentLevel + Actors_Offset);
std::vector<AActor *> actors;
for (int i = 0; i < Actors.Num(); i++) {auto Actor = Actors[i];
if (Actor) {actors.push_back(Actor); }} return actors; }
struct sRegion {uintptr_t start, end;};
std::vector<sRegion> trapRegions;
bool isObjectInvalid(UObject *obj) {
if (!Tools::IsPtrValid(obj)) {return true;}
if (!Tools::IsPtrValid(obj->ClassPrivate)) {return true;}
if (obj->InternalIndex <= 0) {return true;}
if (obj->NamePrivate.ComparisonIndex <= 0) {return true;}
if ((uintptr_t)(obj) % sizeof(uintptr_t) != 0x0 && (uintptr_t)(obj) % sizeof(uintptr_t) != 0x4) {return true;}
if (std::any_of(trapRegions.begin(), trapRegions.end(), [obj](sRegion region) { return ((uintptr_t) obj) >= region.start && ((uintptr_t) obj) <= region.end; }) || std::any_of(trapRegions.begin(), trapRegions.end(), [obj](sRegion region) { return ((uintptr_t) obj->ClassPrivate) >= region.start && ((uintptr_t) obj->ClassPrivate) <= region.end; })) {return true;}
return false;
}
template<class T>
void GetAllActors(std::vector<T*>& Actors) {
UGameplayStatics* gGameplayStatics = (UGameplayStatics*)gGameplayStatics->StaticClass();
auto GWorld = GetWorld();
if (GWorld) {
TArray<AActor*> Actors2;
gGameplayStatics->GetAllActorsOfClass((UObject*)GWorld, T::StaticClass(), &Actors2);
for (int i = 0; i < Actors2.Num(); i++) {
Actors.push_back((T*)Actors2[i]);
}}}
static UGameViewportClient *GameViewport = 0;
UGameViewportClient *GetGameViewport() {
while (!GameViewport) {
GameViewport = UObject::FindObject<UGameViewportClient>("GameViewportClient Transient.UAEGameEngine_1.GameViewportClient_1");
sleep(1); }
if (GameViewport) {
return GameViewport; }


return 0; }


const char *getObjectPath(UObject *Object) {
std::string s;
for (auto super = Object->ClassPrivate; super; super = (UClass *) super->SuperStruct) {
if (!s.empty())
s += ".";
s += super->GetName();}return s.c_str();}

typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern ImGuiDemoMarkerCallback  GImGuiDemoMarkerCallback;
extern void* GImGuiDemoMarkerCallbackUserData;
ImGuiDemoMarkerCallback GImGuiDemoMarkerCallback = NULL;
void* GImGuiDemoMarkerCallbackUserData = NULL;
#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)
ImGuiStyle& style = ImGui::GetStyle();
static ImGuiStyle ref_saved_style;
 
int32_t ToColor(float *col) {return ImGui::ColorConvertFloat4ToU32(*(ImVec4 *) (col));}
//======================| 𝗪𝗢𝗥𝗟𝗗 𝗘𝗦𝗣 𝗗𝗘𝗙𝗜𝗡𝗘 |==========================//
FRotator ToRotator(FVector local, FVector target) {
FVector rotation = UKismetMathLibrary::Subtract_VectorVector(local, target);
float hyp = sqrt(rotation.X * rotation.X + rotation.Y * rotation.Y);
FRotator newViewAngle = {0};
newViewAngle.Pitch = -atan(rotation.Z / hyp) * (180.f / (float) 3.14159265358979323846);
newViewAngle.Yaw = atan(rotation.Y / rotation.X) * (180.f / (float) 3.14159265358979323846);
newViewAngle.Roll = (float) 0.f;
if (rotation.X >= 0.f) newViewAngle.Yaw += 180.0f;
return newViewAngle;
}
//==========
FVector WorldToRadar(float Yaw, FVector Origin, FVector LocalOrigin, float PosX, float PosY, Vector3 Size, bool &outbuff) {
bool flag = false; double num = (double)Yaw; double num2 = num * 0.017453292519943295; float num3 = (float)std::cos(num2);
float num4 = (float)std::sin(num2); float num5 = Origin.X - LocalOrigin.X; float num6 = Origin.Y - LocalOrigin.Y; struct FVector Xector;
Xector.X = (num6 * num3 - num5 * num4) / 150.f; Xector.Y = (num5 * num3 + num6 * num4) / 150.f; struct FVector Xector2;
Xector2.X = Xector.X + PosX + Size.X / 2.f; Xector2.Y = -Xector.Y + PosY + Size.Y / 2.f; bool flag2 = Xector2.X > PosX + Size.X;
if (flag2) { Xector2.X = PosX + Size.X; }else{ bool flag3 = Xector2.X < PosX; if (flag3) { Xector2.X = PosX; }} bool flag4 = Xector2.Y > PosY + Size.Y;
if (flag4) { Xector2.Y = PosY + Size.Y; }else{ bool flag5 = Xector2.Y < PosY; if (flag5){ Xector2.Y = PosY; }}bool flag6 = Xector2.Y == PosY || Xector2.X == PosX;
if (flag6){ flag = true;} outbuff = flag; return Xector2;}
//==========
#define IM_PI   3.14159265358979323846f
#define RAD2DEG(x) ((float)(x) * (float)(180.f / IM_PI))
#define DEG2RAD(x) ((float)(x) * (float)(IM_PI / 180.f))

void VectorAnglesRadar(Vector3 & forward, FVector & angles) {
if (forward.X == 0.f && forward.Y == 0.f) {
angles.X = forward.Z > 0.f ? -90.f : 90.f;
angles.Y = 0.f;
} else {
angles.X = RAD2DEG(atan2(-forward.Z, forward.Magnitude(forward)));
angles.Y = RAD2DEG(atan2(forward.Y, forward.X));
}angles.Z = 0.f;}
//===========
void RotateTriangle(std::array<Vector3, 3> & points, float rotation) {
const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
for (auto & point : points) {
point = point - points_center;
const auto temp_x = point.X;
const auto temp_y = point.Y;
const auto theta = DEG2RAD(rotation);
const auto c = cosf(theta);
const auto s = sinf(theta);
point.X = temp_x * c - temp_y * s;
point.Y = temp_x * s + temp_y * c;
point = point + points_center;}}
// ========================================================================= //

#define W2S(w, s) UGameplayStatics::ProjectWorldToScreen(localController, w, true, s)
 
bool isInsideFOVs(int x, int y) {
if (!Config.AimBot.Cross) return true;
int circle_x = glWidth / 2; int circle_y = glHeight / 2;
int rad = Config.AimBot.Cross*0.5f;
return (x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= rad * rad;
}

bool isInsideFOV(int x, int y) {
if (!Config.SilentAim.Cross) return true;
int circle_x = glWidth / 2; int circle_y = glHeight / 2;
int rad = Config.SilentAim.Cross*0.5f;
return (x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= rad * rad;
}

auto GetTargetForAimBot() {
ASTExtraPlayerCharacter *result = 0;
float max = std::numeric_limits<float>::infinity();
auto Actors = getActors();
auto localPlayer = g_LocalPlayer;
auto localController = g_LocalController;
if (localPlayer) {
for (int i = 0; i < Actors.size(); i++) {
auto Actor = Actors[i];
if (isObjectInvalid(Actor))
continue;
if (Actor->IsA(ASTExtraPlayerCharacter::StaticClass())) {
auto Player = (ASTExtraPlayerCharacter *)Actor;
auto Target = (ASTExtraPlayerCharacter *) Actor;
float dist = localPlayer->GetDistanceTo(Target) / 100.0f;
if (dist > Config.AimBot.Meter)  continue;
if (Player->PlayerKey == localPlayer->PlayerKey)  continue;
if (Player->TeamID == localPlayer->TeamID)  continue;
if (Player->bDead)continue;
if (Config.AimBot.IgnoreKnocked) {if (Player->Health == 0.0f) continue;}
if (Config.AimBot.VisCheck) {if (!localController->LineOfSightTo(Player, {0, 0, 0}, true)) continue;}
if (Config.AimBot.IgnoreBot) {if (Player->bEnsure) continue;}
auto Root = Player->GetBonePos("Root", {});
auto Head = Player->GetBonePos("Head", {});
FVector2D RootSc, HeadSc;
if (W2S(Root, &RootSc) && W2S(Head, &HeadSc)) {
float height = abs(HeadSc.Y - RootSc.Y);
float width = height * 0.65f;
FVector middlePoint = {HeadSc.X + (width / 2), HeadSc.Y + (height / 2), 0};
if ((middlePoint.X >= 0 && middlePoint.X <= glWidth) && (middlePoint.Y >= 0 && middlePoint.Y <= glHeight)) {
FVector2D v2Middle = FVector2D((float)(glWidth / 2), (float)(glHeight / 2));
FVector2D v2Loc = FVector2D(middlePoint.X, middlePoint.Y);
if(isInsideFOVs((int)middlePoint.X, (int)middlePoint.Y)) {
float dist = FVector2D::Distance(v2Middle, v2Loc);
if (dist < max) {max = dist; result = Player; }}}}}}} return result; }


auto GetTargetByCrossDist() {
ASTExtraPlayerCharacter *result = 0;
float max = std::numeric_limits<float>::infinity();
auto Actors = getActors();
auto localPlayer = g_LocalPlayer;
auto localController = g_LocalController;
if (localPlayer) {
for (int i = 0; i < Actors.size(); i++) {
auto Actor = Actors[i];
if (isObjectInvalid(Actor)) continue;
if (Actor->IsA(ASTExtraPlayerCharacter::StaticClass())) {
auto Player = (ASTExtraPlayerCharacter *)Actor;
auto Target = (ASTExtraPlayerCharacter *) Actor;
float dist = localPlayer->GetDistanceTo(Target) / 100.0f;
if (dist > Config.SilentAim.Meter) continue;
if (Player->PlayerKey == localPlayer->PlayerKey) continue;
if (Player->TeamID == localPlayer->TeamID) continue;
if (Player->bDead)   continue;
if (Config.SilentAim.IgnoreKnocked) {if (Player->Health == 0.0f) continue;}
if (Config.SilentAim.VisCheck) {if (!localController->LineOfSightTo(Player, {0, 0, 0}, true)) continue;}
if (Config.SilentAim.IgnoreBot) {if (Player->bEnsure) continue;}
auto Root = Player->GetBonePos("Root", {});
auto Head = Player->GetBonePos("Head", {});
FVector2D RootSc, HeadSc;
if (W2S(Root, &RootSc) && W2S(Head, &HeadSc)) {
float height = abs(HeadSc.Y - RootSc.Y);
float width = height * 0.65f;
FVector middlePoint = {HeadSc.X + (width / 2), HeadSc.Y + (height / 2), 0};
if ((middlePoint.X >= 0 && middlePoint.X <= glWidth) && (middlePoint.Y >= 0 && middlePoint.Y <= glHeight)) {
FVector2D v2Middle = FVector2D((float)(glWidth / 2), (float)(glHeight / 2));
FVector2D v2Loc = FVector2D(middlePoint.X, middlePoint.Y);
if(isInsideFOVs((int)middlePoint.X, (int)middlePoint.Y)) {
float dist = FVector2D::Distance(v2Middle, v2Loc);
if (dist < max) {max = dist; result = Player; }}}}}}} return result; }


const char *GetVehicleName(ASTExtraVehicleBase *Vehicle) {
switch (Vehicle->VehicleShapeType) {
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Motorbike:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Motorbike_SideCart: return  "Motorbike"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Dacia:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyDacia: return  "Dacia"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_MiniBus: return  "Mini Bus"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_PickUp:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_PickUp01:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyPickup: return  "Pick Up"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Buggy:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyBuggy: return  "Buggy"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ01:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ02:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ03:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyUAZ: return  "UAZ"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_PG117: return  "PG117"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Aquarail: return  "Aquarail"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Mirado:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Mirado01: return  "Mirado"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Rony: return  "Rony"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Scooter: return  "Scooter"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_SnowMobile: return  "Snow Mobile"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_TukTukTuk: return  "Tuk Tuk"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_SnowBike: return  "Snow Bike"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Surfboard: return  "Surf Board"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Snowboard: return  "Snow Board"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Amphibious: return  "Amphibious"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_LadaNiva: return  "Lada Niva"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAV: return  "UAV"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_MegaDrop: return  "Mega Drop"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Lamborghini:
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Lamborghini01: return  "Lamborghini"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_GoldMirado: return  "Gold Mirado"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_BigFoot: return  "Big Foot"; break;
case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyUH60: return  "UH60"; break;
default: return  "Vehicle"; break; } return  "Vehicle";
}

void (*orig_shoot_event)(USTExtraShootWeaponComponent *thiz, FVector start, FRotator rot, void *unk1, int unk2) = 0;
void shoot_event(USTExtraShootWeaponComponent *thiz, FVector start, FRotator rot, ASTExtraShootWeapon *weapon, int unk1) {

if (Config.SilentAim.Enable) {
ASTExtraPlayerCharacter *Target = GetTargetByCrossDist();
if (Target) {
bool triggerOk = false;
if (Config.SilentAim.Shooting) {triggerOk = g_LocalPlayer->bIsWeaponFiring;}
else triggerOk = true; if (triggerOk) {
FVector targetAimPos = Target->GetBonePos("Head", {});
UShootWeaponEntity *ShootWeaponEntityComponent = thiz->ShootWeaponEntityComponent;
if (ShootWeaponEntityComponent) {
ASTExtraVehicleBase *CurrentVehicle = Target->CurrentVehicle;
if (CurrentVehicle) {
FVector LinearVelocity = CurrentVehicle->ReplicatedMovement.LinearVelocity;
float dist = g_LocalPlayer->GetDistanceTo(Target);
auto timeToTravel = dist / ShootWeaponEntityComponent->BulletRange;
targetAimPos = UKismetMathLibrary::Add_VectorVector(targetAimPos, UKismetMathLibrary::Multiply_VectorFloat(LinearVelocity, timeToTravel));
} else {
FVector Velocity = Target->GetVelocity();
float dist = g_LocalPlayer->GetDistanceTo(Target);
auto timeToTravel = dist / ShootWeaponEntityComponent->BulletRange;
targetAimPos = UKismetMathLibrary::Add_VectorVector(targetAimPos, UKismetMathLibrary::Multiply_VectorFloat(Velocity, timeToTravel)); }
FVector fDir = UKismetMathLibrary::Subtract_VectorVector(targetAimPos, g_LocalController->PlayerCameraManager->CameraCache.POV.Location);
rot = UKismetMathLibrary::Conv_VectorToRotator(fDir); }}}}
return orig_shoot_event(thiz, start, rot, weapon, unk1);
}

void Box4Line(ImDrawList *draw, float thicc, int x, int y, int w, int h, int color) {
int iw = w / 4;
int ih = h / 4;
draw->AddRect(ImVec2(x, y),ImVec2(x + iw, y), color, thicc);
draw->AddRect(ImVec2(x + w - iw, y),ImVec2(x + w, y), color, thicc);
draw->AddRect(ImVec2(x, y),ImVec2(x, y + ih), color, thicc);
draw->AddRect(ImVec2(x + w - 1, y),ImVec2(x + w - 1, y + ih), color, thicc);;
draw->AddRect(ImVec2(x, y + h),ImVec2(x + iw, y + h), color, thicc);
draw->AddRect(ImVec2(x + w - iw, y + h),ImVec2(x + w, y + h), color, thicc);
draw->AddRect(ImVec2(x, y + h - ih), ImVec2(x, y + h), color, thicc);
draw->AddRect(ImVec2(x + w - 1, y + h - ih), ImVec2(x + w - 1, y + h), color, thicc);
}

class FPSCounter {
protected:
unsigned int m_fps;
unsigned int m_fpscount;
long m_fpsinterval;
public:
FPSCounter() : m_fps(0), m_fpscount(0), m_fpsinterval(0) {}
void update() {
m_fpscount++;
if (m_fpsinterval < time(0)) {
m_fps = m_fpscount;
m_fpscount = 0;
m_fpsinterval = time(0) + 1; }}
unsigned int get() const {
return m_fps; }};
FPSCounter fps;
// =========================================================================== //
void DrawESP(ImDrawList *draw) {

// ── Background Watermark ──
{
    // Customize these:
    const char* watermarkText = "";
    const char* devText = "Developer: @LUCKY_HATHUNGO_WALA";
    
    float wmSize = ((float) density / 18.0f);
    float devSize = ((float) density / 22.0f);
    
    // Shadow (black outline for readability)
    ImGui::GetBackgroundDrawList()->AddText(NULL, wmSize,
        ImVec2(glWidth / 2 - 2, 20 - 2), IM_COL32(0, 0, 0, 180), watermarkText);
    ImGui::GetBackgroundDrawList()->AddText(NULL, wmSize,
        ImVec2(glWidth / 2 + 2, 20 + 2), IM_COL32(0, 0, 0, 180), watermarkText);
    
    // Main text - centered at top
    ImGui::GetBackgroundDrawList()->AddText(NULL, wmSize,
        ImVec2(glWidth / 2, 20), IM_COL32(0, 255, 200, 220), watermarkText);
    
    // Developer name - bottom right corner or below
    ImGui::GetBackgroundDrawList()->AddText(NULL, devSize,
        ImVec2(15, glHeight - 40), IM_COL32(150, 200, 255, 150), devText);
}

if (LOGO) {
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),0x9434,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x470D2F,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x471018,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x47102F,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x47105E,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x481858,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),0x46EFC8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),0x46F8DC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46D1D4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46D76C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46D990,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DA5C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DAA8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DB68,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DC54,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DD48,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DD4C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DED4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46DFC8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E0C4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E1C0,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E1E4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E2B4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E2DC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E31C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E338,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E3F8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E4AC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E50C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E56C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E5CC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E6BC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E74C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E848,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46E9A8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46EDCC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46EE68,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46EFC8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46F0FC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46F674,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46F6F8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46F744,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46F8DC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46F904,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46FA94,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x46FAF4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),0x0D672C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0E9624,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0E965C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0E968C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0E96AC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0E9864,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0E9898,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x0F3A50,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x107380,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x107DE0,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x1089F8,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10948C,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x109ECC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10A9B4,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10B5AC,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10D454,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10E154,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10EED0,"00 20 70 47").Modify();
MemoryPatch::createWithHex(OBFUSCATE("libgcloud.so"),  0x10F884,"00 20 70 47").Modify();
MemoryPatch::createWithHex("libanogs.so",0x404D50,"00 00 80 D2 C0 03 5F D6").Modify();
MemoryPatch::createWithHex("libanogs.so",0x213000,"00 00 80 D2 C0 03 5F D6").Modify();
PATCH_LIB("libanogs.so", "0x46ED30", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so","0x3458E4", "00 20 70 47");//new 1day/3day fixer
PATCH_LIB("libanogs.so", "0x31DCA0", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x213000", "00 00 80 D2 C0 03 5F D6"); //Crash Fixer
PATCH_LIB("libanogs.so", "0x2E906C", "00 00 80 D2 C0 03 5F D6"); //7 Day Flag delay 
PATCH_LIB("libanogs.so", "0x379688", "00 00 80 D2 C0 03 5F D6"); //10 Year Fix
PATCH_LIB("libanogs.so", "0x50F93C", "00 00 80 D2 C0 03 5F D6"); // 1 Day Violation fix
PATCH_LIB("libanogs.so", "0x31DCB0", "00 00 80 D2 C0 03 5F D6");//حل باند عشره ويوم
PATCH_LIB("libanogs.so", "0x2F2D98", "00 00 80 D2 C0 03 5F D6");//حل ياند عشره
PATCH_LIB("libanogs.so", "0x389C14", "00 00 80 D2 C0 03 5F D6");//case36
PATCH_LIB("libanogs.so", "0x2328F0", "00 00 80 D2 C0 03 5F D6");//كيس16
PATCH_LIB("libanogs.so", "0x3FC5C8", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x1C8094", "00 00 80 D2 C0 03 5F D6");//إصلاح عدم اتصالك لمدة 10 سنوات 
PATCH_LIB("libanogs.so","0x35FC5C","00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x49AA00", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x37E8C8", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x2FE7D0", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x4B0F5C", "00 00 80 D2 C0 03 5F D6");// Flag واحد و 7 أيام حظر إصلاح 64bit
PATCH_LIB("libanogs.so", "0x3F2110", "00 00 80 D2 C0 03 5F D6");
MemoryPatch::createWithHex("libanogs.so",0x3E8508,"00 00 80 D2 C0 03 5F D6").Modify();//حل باند عشره ويوم وثلاثه
PATCH_LIB("libanogs.so", "0x2C6560", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x228560", "00 00 80 D2 C0 03 5F D6");//case37
PATCH_LIB("libanogs.so", "0x228168", "00 00 80 D2 C0 03 5F D6");//case35
PATCH_LIB("libanogs.so", "0x225528", "00 00 80 D2 C0 03 5F D6");//case1
PATCH_LIB("libanogs.so", "0x48F1C4", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x425864", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so","0x371418","00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so","0x373064","1F 20 03 D5");
PATCH_LIB("libanogs.so","0x51FA80","C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x296654", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x4AEED8", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x3E5F14", "00 00 80 D2 C0 03 5F D6"); // 10 YEAR FIX
PATCH_LIB("libanogs.so", "0x34A214", "00 00 80 D2 C0 03 5F D6"); // 10 YEAR FIX
PATCH_LIB("libanogs.so", "0x34A218", "00 00 80 D2 C0 03 5F D6"); // 10 YEAR FIX// MALLOC
PATCH_LIB("libanogs.so", "0x51CFDC", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x32302C", "00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libanogs.so", "0x4BAB98", "00 00 80 D2 C0 03 5F D6");

}

if (Config.Bypass) {
    PATCH_LIB("libUE4.so", "0x40DA1A1", "00 00 80 D2 C0 03 5F D6");
    // يحل 10 سنوات ويحذف البلاغات
    PATCH_LIB("libUE4.so", "0x3E8DFDD", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EAB441", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EAC730", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EB0E36", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3ED49B4", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EE3586", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EF33F4", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EFE6A2", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3F0C4E4", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3F41D21", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3F79CB8", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3FD9298", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x400DC3A", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x402DEBF", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x404AA21", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x405A411", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x407180D", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4092EB2", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x40E3AA6", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x40E913F", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4114E4D", "00 00 80 D2 C0 03 5F D6");
    // يحل 10 سنوات ويحذف البلاغات
    PATCH_LIB("libUE4.so", "0x3E91761", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E9B93C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EBD9FF", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EFEE8C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3F0BD4D", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3F6F46D", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3FB92E4", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4029321", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x406E22E", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4080713", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4093D1B", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x40FF9F8", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4114946", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x411DD08", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x414E00C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x471993", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x481680", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x4816B0", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x484832", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x472A19", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x472AA3", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x472B09", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x472D0A", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x472E3A", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x47A28C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libgcloud.so", "0x48493A", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x25FC", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x2641", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x26A0", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x274E", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x2BFD", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x2C0C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x2C79", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x9FACB", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0xA0E09", "00 00 80 D2 C0 03 5F D6");
    // يحذف الإبلاغات
    PATCH_LIB("libanogs.so", "0x9FAC7", "00 00 80 D2 C0 03 5F D6");
    // يحذف الإبلاغات
    PATCH_LIB("libanogs.so", "0xA51EA", "00 00 80 D2 C0 03 5F D6");
    // يحل الغيابي
    PATCH_LIB("libanogs.so", "0x25F3", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x2671", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0x2952", "00 00 80 D2 C0 03 5F D6");
    // يحل الباند العادي
    PATCH_LIB("libanogs.so", "0x2DF3", "00 00 80 D2 C0 03 5F D6");
    // يحل الباند العادي
    PATCH_LIB("libanogs.so", "0xA04FE", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0xA21BA", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0xA335B", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0xA4ECA", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libanogs.so", "0xA52C9", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3FBB5F7", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x410CA8E", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x42AC83C", "00 00 80 D2 C0 03 5F D6");
    // يحل سكنات
    PATCH_LIB("libUE4.so", "0x3E8C324", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E9225D", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E93B60", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EA24EE", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EA25C1", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3EEDD91", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3FD1B23", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3FD1B3C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x4133156", "00 00 80 D2 C0 03 5F D6");
    // يحل سكنات
    PATCH_LIB("libUE4.so", "0x3E8DB7C", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E93B6D", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E93BC7", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E9BA74", "00 00 80 D2 C0 03 5F D6");
    PATCH_LIB("libUE4.so", "0x3E9D9D3", "00 00 80 D2 C0 03 5F D6");
}

 
  



static float isAutoSlider = -0.20; if(isAutoSlider) { auto isFramese = ImGui::GetFrameCount();
if(isFramese % 20 == 0) { if(isAutoSlider > 1) { isAutoSlider = -0.20; } isAutoSlider += 0.01; }}
// ========================================================================= //
auto isFrames = ImGui::GetFrameCount(); if(isFrames % 1 == 0) {
if(isGreen == 0.01f && isBlue == 0.0f) { isRed += 0.01f; }
if(isRed > 0.99f && isBlue == 0.0f) { isRed = 1.0f; isGreen += 0.01f; }
if(isGreen > 0.99f && isBlue == 0.0f) { isGreen = 1.0f; isRed -= 0.01f; }
if(isRed < 0.01f && isGreen == 1.0f) { isRed = 0.0f; isBlue += 0.01f; }
if(isBlue > 0.99f && isRed == 0.0f) { isBlue = 1.0f; isGreen -= 0.01f; }
if(isGreen < 0.01f && isBlue == 1.0f) { isGreen = 0.0f; isRed += 0.01f; }
if(isRed > 0.99f && isGreen == 0.0f) { isRed = 1.0f; isBlue -= 0.01f; }
if(isBlue < 0.01f && isGreen == 0.0f) { isBlue = 0.0f; isRed -= 0.01f; if(isRed < 0.01f) isGreen = 0.01f; }
}// ========================================================================= //
if (Config.OTHER.HIDEESP) {HIDEESP = false; }else{ HIDEESP = true;} if (HIDEESP) {
auto Actors = getActors();
int totalEnemies = 0, totalBots = 0;
ASTExtraPlayerCharacter *localPlayer = 0;
ASTExtraPlayerController *localController = 0;
// =========================================================================== //
if (!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) {
std::string sExpiredDate = ("   ");
sExpiredDate += expiredDate.c_str();
ImGui::GetForegroundDrawList()->AddText(NULL, ((float) density / 15.0f),{(float) glWidth * isAutoSlider + glWidth / 40,690}, ImColor(isRed, isBlue, isGreen),sExpiredDate.c_str());
}// =========================================================================== //
if (Config.AimBot.Enable) {draw->AddCircle(ImVec2(glWidth / 2.0f, glHeight / 2.0f), Config.AimBot.Cross*0.5f, ToColor(Config.ColorsESP.Fov), 100, 3.0f);}
if (Config.SilentAim.Enable) {draw->AddCircle(ImVec2(glWidth / 2.0f, glHeight / 2.0f), Config.SilentAim.Cross*0.5f, ToColor(Config.ColorsESP.Fov), 100, 3.0f);}
// =========================================================================== //
std::string sFPS = "";
draw->AddText({((float) density / 10.0f), 35}, IM_COL32(0,255,0,255), sFPS.c_str());
// =========================================================================== //
for (int i = 0; i < Actors.size(); i++) { auto Actor = Actors[i]; if (isObjectInvalid(Actor)) continue;
if (Actor->IsA(ASTExtraPlayerController::StaticClass())) {localController = (ASTExtraPlayerController *) Actor; break;}}
// =========================================================================== //
if (localController) { for (int i = 0; i < Actors.size(); i++) { auto Actor = Actors[i]; if (isObjectInvalid(Actor)) continue;
if (Actor->IsA(ASTExtraPlayerCharacter::StaticClass())) { if (((ASTExtraPlayerCharacter *) Actor)->PlayerKey == localController->PlayerKey) {
localPlayer = (ASTExtraPlayerCharacter *) Actor; break; }}} if (localPlayer) {
if (localPlayer->PartHitComponent) {
auto ConfigCollisionDistSqAngles = localPlayer->PartHitComponent->ConfigCollisionDistSqAngles;
for (int j = 0; j < ConfigCollisionDistSqAngles.Num(); j++) {
ConfigCollisionDistSqAngles[j].Angle = 180.0f; }
localPlayer->PartHitComponent->ConfigCollisionDistSqAngles = ConfigCollisionDistSqAngles; }
if (Config.PlayerESP.HitEffect && localController) {
uintptr_t MyHUD = *(uintptr_t *)((uintptr_t)localController + 0x40c);
if (MyHUD) {
uintptr_t HitPerform = *(uintptr_t *)(MyHUD + 0x4b0);
if (HitPerform) {
*(float*)(HitPerform + 0xC) = 99999.0f;
*(float*)(HitPerform + 0x48) = 99999.0f;
*(float*)(HitPerform + 0x84) = 99999.0f;
*(float*)(HitPerform + 0xC0) = 99999.0f;
}}}
if (Config.PlayerESP.RGbcro) {
static float cnt = 0.0f;
const float RainbowSpeed = 9.0f;
const int RainbowColorCount = 7;
const float FullCircle = 360.0f;
const float IncrementValue = 0.02f;

FLinearColor rainbowColors[RainbowColorCount] = {
FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),
FLinearColor(1.0f, 0.5f, 0.0f, 1.0f),
FLinearColor(1.0f, 1.0f, 0.0f, 1.0f),
FLinearColor(0.0f, 1.0f, 0.0f, 1.0f),
FLinearColor(0.0f, 0.0f, 1.0f, 1.0f),
FLinearColor(0.5f, 0.0f, 1.0f, 1.0f),
FLinearColor(1.0f, 0.0f, 1.0f, 1.0f)
};

int rainbowColorIndex = static_cast<int>(fmod(cnt * RainbowSpeed, RainbowColorCount));
FLinearColor color1 = rainbowColors[rainbowColorIndex];
FLinearColor color2 = rainbowColors[(rainbowColorIndex + 1) % RainbowColorCount];

float rainbowPhase = fmod(cnt * RainbowSpeed, 1.0f);
FLinearColor interpolatedColor = FLinearColor(
color1.R + (color2.R - color1.R) * rainbowPhase,
color1.G + (color2.G - color1.G) * rainbowPhase,
color1.B + (color2.B - color1.B) * rainbowPhase,
1.0f
);
localController->CrossHairColor = interpolatedColor;

if (cnt >= FullCircle) {
cnt = 0.0f;
} else {
cnt += IncrementValue;
}
}


if (Config.PlayerESP.HitEffect || Config.PlayerESP.Small) {
auto WeaponManagerComponent = localPlayer->WeaponManagerComponent;
if (WeaponManagerComponent) {
auto CurrentWeaponReplicated = (ASTExtraShootWeapon *) WeaponManagerComponent->CurrentWeaponReplicated;
if (CurrentWeaponReplicated) {
auto ShootWeaponEntityComp = CurrentWeaponReplicated->ShootWeaponEntityComp;
auto ShootWeaponEffectComp = CurrentWeaponReplicated->ShootWeaponEffectComp;
if (ShootWeaponEntityComp && ShootWeaponEffectComp) {
 if (Config.PlayerESP.Small) {
ShootWeaponEntityComp->GameDeviationFactor = 0.0f;
}


}}}}

if (Config.SilentAim.Enable) {
auto WeaponManagerComponent = localPlayer->WeaponManagerComponent;
if (WeaponManagerComponent) {
auto propSlot = WeaponManagerComponent->GetCurrentUsingPropSlot();
if ((int) propSlot.GetValue() >= 1 && (int) propSlot.GetValue() <= 3) {
auto CurrentWeaponReplicated = (ASTExtraShootWeapon *) WeaponManagerComponent->CurrentWeaponReplicated;
if (CurrentWeaponReplicated) {
auto ShootWeaponComponent = CurrentWeaponReplicated->ShootWeaponComponent;
if (ShootWeaponComponent) {
int shoot_event_idx = 174;
auto VTable = (void **) ShootWeaponComponent->VTable;
auto f_mprotect = [](uintptr_t addr, size_t len,
 int32_t prot) -> int32_t {
static_assert(PAGE_SIZE == 4096);
constexpr
size_t page_size = static_cast<size_t>(PAGE_SIZE);
void *start = reinterpret_cast<void *>(addr &
-page_size);
uintptr_t end =
(addr + len + page_size - 20) & -page_size;
return mprotect(start, end -
reinterpret_cast<uintptr_t>(start),
prot);
};
if (VTable && (VTable[shoot_event_idx] != shoot_event)) {
orig_shoot_event = decltype(orig_shoot_event)(
VTable[shoot_event_idx]);

f_mprotect((uintptr_t)(&VTable[shoot_event_idx]),
sizeof(uintptr_t), PROT_READ | PROT_WRITE);
VTable[shoot_event_idx] = (void *) shoot_event;
}
}
}
}
}
}

// ==========================》 DeadBox 《============================= //
if (DeadBox) {
std::vector<APlayerTombBox *> TombBox;
GetAllActors(TombBox);
for (auto actor = TombBox.begin();
actor != TombBox.end(); actor++) {
auto TombBoxx = *actor;
if (TombBoxx && TombBoxx->DamageCauser && TombBoxx->TargetPlayer && localController) {
if (TombBoxx->DamageCauser->PlayerKey == localController->PlayerKey) {
auto PlayerKey = TombBoxx->TargetPlayer->PlayerKey;
if (AlreadyChangedSet.find(PlayerKey) == AlreadyChangedSet.end()) {
if (g_LocalPlayer && g_LocalPlayer->WeaponManagerComponent) {
auto DeadBoxAvatarCompPtr = (uintptr_t*)((uintptr_t)TombBoxx + 0x718); //DeadBoxAvatarComponent_BP_C* DeadBoxAvatarComponent_BP
if (DeadBoxAvatarCompPtr) {
auto DeadBoxAvatarComp = *DeadBoxAvatarCompPtr;
auto CurrentWeaponReplicated = g_LocalPlayer->WeaponManagerComponent->CurrentWeaponReplicated;
if (CurrentWeaponReplicated) {
auto weaponid = g_LocalPlayer->WeaponManagerComponent->CurrentWeaponReplicated->GetWeaponID();
if (weaponid == 101004) {//M416
ChangeItemAVc(DeadBoxAvatarComp, 1101004046); //Glacier m4
AlreadyChangedSet.insert(PlayerKey);
}
}}}}}}}}


// ==========================》AIMBOT SITTING 《============================= //
if (Config.AimBot.Enable) {
ASTExtraPlayerCharacter *Target = GetTargetForAimBot();
if (Target) {
bool triggerOk = false;
if (Config.AimBot.Shooting) {triggerOk = localPlayer->bIsWeaponFiring;
} else triggerOk = true;
if (triggerOk) {
FVector targetAimPos = Target->GetBonePos("Head", {});
bool AimFixed = true;
if (AimFixed) {
targetAimPos.Z -= 29.55f;
targetAimPos.Y -= 1.224f;
}
auto WeaponManagerComponent = localPlayer->WeaponManagerComponent;
if (WeaponManagerComponent) {
auto propSlot = WeaponManagerComponent->GetCurrentUsingPropSlot();
if ((int) propSlot.GetValue() >= 1 && (int) propSlot.GetValue() <= 3) {
auto CurrentWeaponReplicated = (ASTExtraShootWeapon *) WeaponManagerComponent->CurrentWeaponReplicated;
if (CurrentWeaponReplicated) {
auto ShootWeaponComponent = CurrentWeaponReplicated->ShootWeaponComponent;
if (ShootWeaponComponent) {
UShootWeaponEntity *ShootWeaponEntityComponent = ShootWeaponComponent->ShootWeaponEntityComponent;
if (ShootWeaponEntityComponent) {
ASTExtraVehicleBase *CurrentVehicle = Target->CurrentVehicle;
if (CurrentVehicle) {
FVector LinearVelocity = CurrentVehicle->ReplicatedMovement.LinearVelocity;
float dist = localPlayer->GetDistanceTo(Target);
auto timeToTravel = dist / ShootWeaponEntityComponent->BulletRange;
targetAimPos = UKismetMathLibrary::Add_VectorVector(
targetAimPos, UKismetMathLibrary::Multiply_VectorFloat(
LinearVelocity, timeToTravel));
} else {
FVector Velocity = Target->GetVelocity();
float dist = localPlayer->GetDistanceTo(Target);
auto timeToTravel = dist / ShootWeaponEntityComponent->BulletRange;
targetAimPos = UKismetMathLibrary::Add_VectorVector(
targetAimPos, UKismetMathLibrary::Multiply_VectorFloat(
Velocity, timeToTravel));}
if (Config.AimBot.Enable) {
if (g_LocalPlayer->bIsGunADS) {
if (g_LocalPlayer->bIsWeaponFiring) {
float dist = localPlayer->GetDistanceTo(Target)/ 100.0f;
targetAimPos.Z -= dist * Config.AimBot.Recc;}}}
localController->SetControlRotation(ToRotator(localController->PlayerCameraManager->CameraCache.POV.Location, targetAimPos), "");
}}}}}}}}
// ==========================》SILENT SITTING 《============================= //
 if (Config.SilentAim.Enable) {
 auto WeaponManagerComponent = localPlayer->WeaponManagerComponent;
 if (WeaponManagerComponent) {
 auto propSlot = WeaponManagerComponent->GetCurrentUsingPropSlot();
 if ((int) propSlot.GetValue() >= 1 && (int) propSlot.GetValue() <= 3) {
 auto CurrentWeaponReplicated = (ASTExtraShootWeapon *) WeaponManagerComponent->CurrentWeaponReplicated;
 if (CurrentWeaponReplicated) {
 auto ShootWeaponComponent = CurrentWeaponReplicated->ShootWeaponComponent;
 if (ShootWeaponComponent) {
 int shoot_event_idx = 174;
 auto f_mprotect = [](uintptr_t addr, size_t len, int32_t prot) -> int32_t {
 static_assert(PAGE_SIZE == 4096);
 constexpr size_t page_size = static_cast<size_t>(PAGE_SIZE);
 void* start = reinterpret_cast<void*>(addr & -page_size);
 uintptr_t end = (addr + len + page_size - 1) & -page_size;
 return mprotect(start, end - reinterpret_cast<uintptr_t>(start), prot);
 };
 auto VTable = (void **) ShootWeaponComponent->VTable;
 if (VTable && (VTable[shoot_event_idx] != shoot_event)) {
 orig_shoot_event = decltype(orig_shoot_event)(
 VTable[shoot_event_idx]);
 f_mprotect((uintptr_t)(&VTable[shoot_event_idx]), sizeof(uintptr_t), PROT_READ | PROT_WRITE);
 VTable[shoot_event_idx] = (void *) shoot_event;
 }}}}}}
// =========================================================================== //
FVector ViewPosY{0, 0, 0}; if (localPlayer) {ViewPosY = localPlayer->GetBonePos("Head", {}); ViewPosY.Z += 10.f;} 

for (auto &i : Actors) {
auto Actor = i;
if (isObjectInvalid(Actor))   continue;
if (Actor->IsA(ASTExtraPlayerCharacter::StaticClass())) {
long SCOLOR2 = IM_COL32(255,255,255,255);
auto Player = (ASTExtraPlayerCharacter *) Actor;
float Distance = localPlayer->GetDistanceTo(Player) / 100.0f;
if (Distance > 500)  continue;
if (Player->PlayerKey == localController->PlayerKey)   continue;
if (Player->TeamID == localController->TeamID)   continue;
if (Player->bDead)continue;
if (Player->bEnsure)
totalBots++; else totalEnemies++;
if (Config.PlayerESP.NoBot)
if (Player->bEnsure)continue;
float magic_number = (Distance);
float mx = (glWidth / 4) / magic_number;
float healthLength = glWidth / 19;
if (healthLength < mx) healthLength = mx;
auto HeadPos = Player->GetBonePos("Head", {});
ImVec2 HeadPosSC;

auto RootPos = Player->GetBonePos("Root", {});
ImVec2 RootPosSC;
auto upper_r = Player->GetBonePos("upperarm_r", {});
ImVec2 upper_rPoSC;
auto lowerarm_r = Player->GetBonePos("lowerarm_r", {});
ImVec2 lowerarm_rPoSC;
auto hand_r = Player->GetBonePos("hand_r", {});
ImVec2 hand_rPoSC;
auto upper_l = Player->GetBonePos("upperarm_l", {});
ImVec2 upper_lPoSC;
auto lowerarm_l = Player->GetBonePos("lowerarm_l", {});
ImVec2 lowerarm_lSC;
auto hand_l = Player->GetBonePos("hand_l", {});
ImVec2 hand_lPoSC;
auto thigh_l = Player->GetBonePos("thigh_l", {});
ImVec2 thigh_lPoSC;
auto calf_l = Player->GetBonePos("calf_l", {});
ImVec2 calf_lPoSC;
auto foot_l = Player->GetBonePos("foot_l", {});
ImVec2 foot_lPoSC;
auto thigh_r = Player->GetBonePos("thigh_r", {});
ImVec2 thigh_rPoSC;
auto calf_r = Player->GetBonePos("calf_r", {});
ImVec2 calf_rPoSC;
auto foot_r = Player->GetBonePos("foot_r", {});
ImVec2 foot_rPoSC;
auto neck_01 = Player->GetBonePos("neck_01", {});
ImVec2 neck_01PoSC;
auto Pelvis = Player->GetBonePos("pelvis", {});
ImVec2 PelvisPoSC;

bool IsVisible = localController->LineOfSightTo(Player, {0, 0, 0}, true);
int SCOLOR; if (IsVisible) {SCOLOR = IM_COL32(0,255,0,255);}else{SCOLOR = IM_COL32(255,0,0,255);}
if (W2S(HeadPos, (FVector2D *) &HeadPosSC) && W2S(upper_r, (FVector2D *) &upper_rPoSC) && W2S(upper_l, (FVector2D *) &upper_lPoSC) && W2S(lowerarm_r, (FVector2D *) &lowerarm_rPoSC ) && W2S(hand_r, (FVector2D *) &hand_rPoSC ) && W2S(lowerarm_l, (FVector2D *) &lowerarm_lSC ) && W2S(hand_l, (FVector2D *) &hand_lPoSC ) && W2S(thigh_l, (FVector2D *) &thigh_lPoSC ) && W2S(calf_l, (FVector2D *) &calf_lPoSC ) && W2S(foot_l, (FVector2D *) &foot_lPoSC ) && W2S(thigh_r, (FVector2D *) &thigh_rPoSC ) && W2S(calf_r, (FVector2D *) &calf_rPoSC ) && W2S(foot_r, (FVector2D *) &foot_rPoSC ) && W2S(neck_01, (FVector2D *) &neck_01PoSC ) && W2S(Pelvis, (FVector2D *) &PelvisPoSC ) && W2S(RootPos, (FVector2D *) &RootPosSC)){ 
if (Distance < 450.0f && totalEnemies > 0 || totalBots > 0) {


if (Config.PlayerESP.Line) {draw->AddLine({(float) glWidth / 2, 0}, ImVec2(HeadPosSC.x, HeadPosSC.y - 55.0f), SCOLOR, 3.0f);}



if (Config.PlayerESP.Box) {
float boxHeight = abs(HeadPosSC.y - RootPosSC.y); float boxWidth = boxHeight * 0.65f;
Box4Line(draw, 0.2f, HeadPosSC.x - (boxWidth / 2), HeadPosSC.y, boxWidth, boxHeight,  SCOLOR);
}



if (Config.PlayerESP.Skeleton) {
float boxWidth = 7.f - Distance * 0.03;
draw->AddCircle({HeadPosSC.x, HeadPosSC.y}, boxWidth, SCOLOR, 0, 1.5f); }
if (Config.PlayerESP.Skeleton) {
float boxHeight = fabsf(RootPosSC.y - HeadPosSC.y);
float boxWidth = boxHeight * 0.68f;
draw->AddLine({upper_rPoSC.x, upper_rPoSC.y}, neck_01PoSC, SCOLOR, 2.0f);
draw->AddLine({upper_lPoSC.x, upper_lPoSC.y}, neck_01PoSC, SCOLOR, 2.0f);
draw->AddLine({upper_rPoSC.x, upper_rPoSC.y}, lowerarm_rPoSC, SCOLOR, 2.0f);
draw->AddLine({lowerarm_rPoSC.x, lowerarm_rPoSC.y}, hand_rPoSC, SCOLOR , 2.0f);
draw->AddLine({upper_lPoSC.x, upper_lPoSC.y}, lowerarm_lSC, SCOLOR, 2.0f);
draw->AddLine({lowerarm_lSC.x, lowerarm_lSC.y}, hand_lPoSC, SCOLOR, 2.0f);
draw->AddLine({thigh_rPoSC.x, thigh_rPoSC.y}, thigh_lPoSC, SCOLOR, 2.0f);
draw->AddLine({thigh_lPoSC.x, thigh_lPoSC.y}, calf_lPoSC, SCOLOR, 2.0f);
draw->AddLine({calf_lPoSC.x, calf_lPoSC.y}, foot_lPoSC, SCOLOR, 2.0f);
draw->AddLine({thigh_rPoSC.x, thigh_rPoSC.y}, calf_rPoSC, SCOLOR, 2.0f);
draw->AddLine({calf_rPoSC.x, calf_rPoSC.y}, foot_rPoSC, SCOLOR, 2.0f);
draw->AddLine({neck_01PoSC.x, neck_01PoSC.y}, PelvisPoSC, SCOLOR, 2.0f);
draw->AddLine({neck_01PoSC.x, neck_01PoSC.y}, HeadPosSC, SCOLOR, 2.0f);
if (Distance < 50.0f) {
draw->AddCircleFilled(ImVec2(thigh_rPoSC.x, thigh_rPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(calf_rPoSC.x, calf_rPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(thigh_lPoSC.x, thigh_lPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(calf_lPoSC.x, calf_lPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(thigh_rPoSC.x, thigh_rPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(upper_lPoSC.x, upper_lPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(lowerarm_lSC.x, lowerarm_lSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(lowerarm_rPoSC.x, lowerarm_rPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(upper_rPoSC.x, upper_rPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(upper_lPoSC.x, upper_lPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(PelvisPoSC.x, PelvisPoSC.y), boxWidth / 70, SCOLOR2);
draw->AddCircleFilled(ImVec2(neck_01PoSC.x, neck_01PoSC.y), boxWidth / 70, SCOLOR2);
}}
if (Config.PlayerESP.Health) {
  int CurHP = (int)std::max(0, std::min((int)Player->Health, (int)Player->HealthMax));
int MaxHP = (int)Player->HealthMax;
ImU32 color_red = ImColor(255, 25, 25);
ImU32 color_orange = ImColor(255, 180, 0);
ImU32 color_green = ImColor(50, 230, 50);
ImU32 current_color = color_green;
float health = Player->Health;

if (health <= 50.0f) {
current_color = color_orange;
}

if (health <= 25.0f) {
current_color = color_red;
}

float boxWidth = density / 3.0f;
boxWidth -= std::min(((boxWidth / 2) / 00.0f) * Distance, boxWidth / 2);
float boxHeight = boxWidth * 0.07f;
ImVec2 vStart = {HeadPosSC.x - (boxWidth / 2), HeadPosSC.y - (boxHeight * 2.1f)};
ImVec2 vEndFilled = {vStart.x + (CurHP * boxWidth / MaxHP), vStart.y + boxHeight};
ImVec2 vEndRect = {vStart.x + boxWidth, vStart.y + boxHeight};
draw->AddRectFilled(vStart, vEndRect, IM_COL32(0,0,0,255));
draw->AddRectFilled(vStart, vEndFilled, current_color);
  }
  
// =========================================================================== //
if (Config.PlayerESP.HealthPC){
						                                                       int CurHP = (int) std::max(0, std::min((int) Player->Health,
                                                                       (int) Player->HealthMax));
                                int MaxHP = (int) Player->HealthMax;

                                long HPColor = IM_COL32(39, 172, 57, 110);
                                long HPRectColor = IM_COL32(1, 1, 1, 255);

                                if (Player->Health == 0.0f && !Player->bDead) {
                                    HPColor = IM_COL32(255, 0, 0, 110);

                                    CurHP = Player->NearDeathBreath;
                                    if (Player->NearDeatchComponent) {
                                        MaxHP = Player->NearDeatchComponent->BreathMax;
                                    }
                                }

                                float boxWidth = density / 3.2f; // width
                                boxWidth -= std::min(((boxWidth / 2) / 500.0f) * Distance,
                                                     boxWidth / 2);
                                float boxHeight = boxWidth * 0.19f; // height


                                ImVec2 vStart = {HeadPosSC.x - (boxWidth / 2),
                                                 HeadPosSC.y - (boxHeight * 1.9f)}; //ooper neeche

                                ImVec2 vEndFilled = {vStart.x + (CurHP * boxWidth / MaxHP),
                                                     vStart.y + boxHeight};
                                ImVec2 vEndRect = {vStart.x + boxWidth, vStart.y + boxHeight};

                                draw->AddRectFilled(vStart, vEndFilled, HPColor, 0.0f, 240);
                                draw->AddRect(vStart, vEndRect, HPRectColor, 0.0f, 240);
                            

                            
                            

                        }
						
if (Config.PlayerESP.Name) {
                            float boxWidth = density / 1.6f;
                                               boxWidth -= std::min(
                                               ((boxWidth / 2) / 00.0f) * Distance,
                                               boxWidth / 2);
                                               float boxHeight = boxWidth * 0.15f;
                                               std::string s;
                                               if (Player->bIsAI) {
                                               s += "      BOT";
                                               } else {
                                               s += Player->PlayerName.ToString();
                                               }
                                               draw->AddText(NULL, ((float) density / 30.0f),
                                               {HeadPosSC.x - (boxWidth / 3.3),
                                               HeadPosSC.y - (boxHeight * 1.83f)},
                                               IM_COL32(255, 255, 255, 215),
                                               s.c_str());
                        }                                         
           
                 if (Config.PlayerESP.Grenade) {
         if (Actor->IsA(ASTExtraGrenadeBase::StaticClass())) {
                auto Grenade = (ASTExtraGrenadeBase *) Actor;
                  auto RootComponent = Actor->RootComponent;
                          if (!RootComponent)
                         continue;
              float Distance = Grenade->GetDistanceTo(localPlayer) / 100.f;

                                FVector2D grenadePos;

                                if (W2S(Grenade->K2_GetActorLocation(), &grenadePos)) {
                                    std::string s = ICON_FA_BOMB"Grenade";
                                    s += "";
                                    s += std::to_string((int) Distance);
                                    s += "M";
                                    std::string t;
                                t += "!!!...Warning Grenade...!!!";
                            auto textSize = ImGui::CalcTextSize2(t.c_str(), 0, ((float) density / 13.0f));
                        draw->AddText(NULL, ((float) density / 13.0f), ImVec2(glWidth / 2 - (textSize.x / 2), 110), IM_COL32(255, 000, 000, 255), t.c_str());

                                    draw->AddText(NULL, ((float) density / 30.0f),
                                                  {grenadePos.X, grenadePos.Y},
                                                  IM_COL32(255, 000, 000, 255), s.c_str());
                                }
                            }
                        }
  						
if (Config.PlayerESP.GameInfo) {
                         			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 7.0f;
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.784f);
                                if (Actor->IsA(ASTExtraGameStateBase::StaticClass())) {
									auto mareetg = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus/* + ImGuiWindowFlags_NoTitleBar*/;
                                    auto InGame = (ASTExtraGameStateBase*)Actor;
                                    if (ImGui::Begin("Game Information ", 0, mareetg))
                                    {
                                     ImGui::Separator();
                                        if (InGame->AliveTeamNum == 1)
                                        {
											

                                            ImGui::Text("Game Information");
											
                                            ImGui::Separator();
                                            ImGui::Text("WINER WINNER CHICKEN DINNER");
                                        }
                                        else
                                        {
											
											ImGui::SameLine();
                                            ImGui::Text(" Game Information");
                                            ImGui::Separator();
                                            std::string Text1 = std::to_string(InGame->AlivePlayerNum) + " Players Alive " + "With " + std::to_string((int)InGame->AliveTeamNum) + " Teams ";
                                            std::string Text2 = "Real Players : " + std::to_string(InGame->PlayerNum);
                                            std::string TEAMTYPE = "";
                                            if (InGame->PlayerNumPerTeam == 1) TEAMTYPE = "Solo"; else if (InGame->PlayerNumPerTeam == 2) TEAMTYPE = "Duo"; else if (InGame->PlayerNumPerTeam == 4) TEAMTYPE = "Squad"; else TEAMTYPE = "Detecting Mode;";
                                            std::string Text3 = "Team Type : " + TEAMTYPE;
											
                                            ImGui::Spacing;
                                            ImGui::Spacing;
                                            ImGui::Text(Text1.c_str());

                                            ImGui::Text(Text2.c_str());
                                            ImGui::Separator();
                                            ImGui::Text(Text3.c_str());
                                            ImGui::Spacing;
                                        }

                                        ImGui::End();
                                    }
                                    ImGui::PopStyleColor();
                                    ImGui::PopStyleColor();


                                }
                            }
                            
                            
                            				
      ImGuiIO &Io = ImGui::GetIO();
    static auto lund =
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 100));

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 1.0f, 0.0f, 100));

    ImGui::SetNextWindowPos(ImVec2(Io.DisplaySize.x * 0.5f, Io.DisplaySize.y * 0.1f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.1f));
    ImGui::SetNextWindowSize(
    {250, 0});

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

    ImGui::SetNextWindowSize({340, 117});

                      if (Config.PlayerESP.GameInfo1) {

    if (ImGui::Begin(" Game Information", 0, lund | ImGuiWindowFlags_NoTitleBar))
    {
        if (Actor->IsA(ASTExtraGameStateBase::StaticClass()))
        {
            auto InGame = (ASTExtraGameStateBase *)Actor;
            static bool blinkState = true;
            static float blinkTimer = 0.0f;
            blinkTimer += ImGui::GetIO().DeltaTime;
            if (blinkTimer > 0.5f)
            {
                blinkState = !blinkState;
                blinkTimer = 0.0f;
            }
            ImVec4 ballColor = blinkState ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            ImGui::TextColored(ballColor, ICON_FA_CIRCLE);

            ImGui::SameLine();
            ImGui::Text(" Game Information");
			if (InGame->AliveTeamNum == 1)
             {
              ImGui::Separator();
              ImGui::Text("WINER WINNER CHICKEN DINNER");
             }
            ImGui::Separator();
            ImGui::Text("%d Players alive With %d Teams, Real Players: %d", static_cast<int>(InGame->AlivePlayerNum), static_cast<int>(InGame->AliveTeamNum), static_cast<int>(InGame->PlayerNum));
             std::string TEAMTYPE = "";
           if (InGame->PlayerNumPerTeam == 1) TEAMTYPE = "Solo"; else if (InGame->PlayerNumPerTeam == 2) TEAMTYPE = "Duo"; else if (InGame->PlayerNumPerTeam == 4) TEAMTYPE = "Squad"; else TEAMTYPE = "Detecting Mode;";
		    std::string Text3 = "Team Type : " + TEAMTYPE;
		    ImGui::Text("Match ID: %d, Match Time: %ds | %dm", static_cast<int>(InGame->GameID), static_cast<int>(InGame->ElapsedTime), static_cast<int>(InGame->ElapsedTime) / 60);
            ImGui::Separator();
            ImGui::Text("In Match (%ds seconds)  ", static_cast<int>(InGame->ElapsedTime));

           ImGui::End();
        }
    }
    
}

if (Config.PlayerESP.Distance) {
std::string s;
if (Config.PlayerESP.Distance) {
s += "[";
s += std::to_string((int) Player->TeamID);
s += "]";
}
s += " - ";
if (Config.PlayerESP.Distance) {
s += std::to_string((int) Distance);
s += "M";
}
auto textSize = ImGui::CalcTextSize(s.c_str(), 0, ((float) density / 20.0f));
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 4.4 - (textSize.x / 2) ,  RootPosSC.y - 6.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 2.4 - (textSize.x / 2) ,  RootPosSC.y - 4.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 2.4 - (textSize.x / 2) ,  RootPosSC.y - 6.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 4.4 - (textSize.x / 2) ,  RootPosSC.y - 4.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 3.4 - (textSize.x / 2) ,  RootPosSC.y - 6.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 3.4 - (textSize.x / 2) ,  RootPosSC.y - 4.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 4.4 - (textSize.x / 2) ,  RootPosSC.y - 5.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 2.4 - (textSize.x / 2) ,  RootPosSC.y - 5.f),IM_COL32(0,0,0,255),s.c_str());
draw->AddText(flamee,15,ImVec2(RootPosSC.x + 3.4 - (textSize.x / 2) ,  RootPosSC.y - 5.f),IM_COL32(255,255,255,255),s.c_str());
}
}}// =========================================================================== //
if (Config.PlayerESP.Grenade){
bool shit = false;
FVector MyPosition, EntityPosition;
ASTExtraVehicleBase* CurrentVehicle = Player->CurrentVehicle;
if (CurrentVehicle) {MyPosition = CurrentVehicle->RootComponent->RelativeLocation; }else{ MyPosition = Player->RootComponent->RelativeLocation;}
ASTExtraVehicleBase* CurrentVehicleAI = localPlayer->CurrentVehicle;
if (CurrentVehicleAI) {EntityPosition = CurrentVehicleAI->RootComponent->RelativeLocation; }else{ EntityPosition = localPlayer->RootComponent->RelativeLocation;}
FVector EntityPos = WorldToRadar(localController->PlayerCameraManager->CameraCache.POV.Rotation.Yaw, MyPosition, EntityPosition, NULL, NULL, Vector3(glWidth, glHeight, 0),shit);
FVector angle = FVector(); Vector3 toEntity = Vector3((float)(glWidth / 2) - EntityPos.X, (float)(glHeight / 2) - EntityPos.Y, 0.0f); VectorAnglesRadar(toEntity, angle);
const auto angle_yaw_rad = DEG2RAD(angle.Y + 180.f);
const auto new_point_x = (glWidth / 2) +  (55) / 2 * 8 * cosf(angle_yaw_rad);
const auto new_point_y = (glHeight / 2) + (55) / 2 * 8 * sinf(angle_yaw_rad);
const float sphereRadius = 6.0f;
draw->AddCircleFilled(ImVec2(new_point_x, new_point_y), sphereRadius, SCOLOR);
}

}

if (Config.PlayerESP.Vehicle) {
if (i->IsA(ASTExtraVehicleBase::StaticClass())){
auto Vehicle = (ASTExtraVehicleBase *) i;
if (!Vehicle->Mesh) continue;
int CurHP = (int) std::max(0, std::min((int) Vehicle->VehicleCommon->HP, (int) Vehicle->VehicleCommon->HPMax));
int MaxHP = (int) Vehicle->VehicleCommon->HPMax;
long curHP_Color = IM_COL32(std::min(((510 * (MaxHP - CurHP)) / MaxHP), 255), std::min(((510 * CurHP) / MaxHP), 240), 0, 155);
float Distance = Vehicle->GetDistanceTo(localPlayer) / 100.f; if (Distance <= 400.0f) {
FVector2D vehiclePos;
if (W2S(Vehicle->K2_GetActorLocation(), &vehiclePos)) {
auto mWidthScale = std::min(0.10f * Distance, 40.f);
auto mWidth = 70.f - mWidthScale; auto mHeight = 6.15f;
bool IsVisible = localController->LineOfSightTo(Vehicle, ViewPosY, true);
std::string s = GetVehicleName(Vehicle);
s += " ["; s += std::to_string((int)Distance); s += "]";
draw->AddText(NULL, ((float)density / 30.0f), {vehiclePos.X - (mWidth / 2), vehiclePos.Y}, IM_COL32(255,255,255,255), s.c_str());
if (Config.PlayerESP.Vehicle){
ImVec2 vStart = {vehiclePos.X - (mWidth / 2), vehiclePos.Y + 16.1f};
ImVec2 vEndFilled = {vStart.x + (CurHP * mWidth / MaxHP), vStart.y + mHeight};
ImVec2 vEndRect = {vStart.x + mWidth, vStart.y + mHeight};
draw->AddRectFilled(vStart, vEndFilled, curHP_Color, 0.2f, 0);
draw->AddRect(vStart, vEndRect, IM_COL32(000,000,000,255), 0.2f, 0); }
if (Config.PlayerESP.Vehicle){
int CurHP = (int) std::max(0, std::min((int) Vehicle->VehicleCommon->Fuel, (int) Vehicle->VehicleCommon->FuelMax));
int MaxHP = (int) Vehicle->VehicleCommon->FuelMax;
long curHP_Color = IM_COL32(std::min(((510 * (MaxHP - CurHP)) / MaxHP), 255),std::min((510 * CurHP) / MaxHP, 255), 0, 155);
auto mWidthScale = std::min(0.10f * Distance, 40.f);
auto mWidth = 70.f - mWidthScale; auto mHeight = 4.30f;
ImVec2 vStart = {vehiclePos.X - (mWidth / 2), vehiclePos.Y + 23.f};
ImVec2 vEndFilled = {vStart.x + (CurHP * mWidth / MaxHP), vStart.y + mHeight};
ImVec2 vEndRect = {vStart.x + mWidth, vStart.y + mHeight};
draw->AddRectFilled(vStart, vEndFilled, IM_COL32(255, 255, 255, 210));
draw->AddRect(vStart, vEndRect, IM_COL32(000,000,000,255), 0.2f, 0);
}}}}}





if (i->IsA(APickUpWrapperActor::StaticClass())) {
auto PickUp = (APickUpWrapperActor *) i;
if (Items[PickUp->DefineID.TypeSpecificID]) {
auto RootComponent = PickUp->RootComponent;
if (!RootComponent) continue;
float Distance = PickUp->GetDistanceTo(localPlayer) / 100.f;
FVector2D itemPos;
if (W2S(PickUp->K2_GetActorLocation(), &itemPos)) {
std::string s;
uint32_t tc = 0xFF000000;
for (auto &category: items_data) {
for (auto &item: category["Items"]) {
if (item["itemId"] == PickUp->DefineID.TypeSpecificID) {
s = item["itemName"].get<std::string>();
tc = strtoul(item["itemTextColor"].get<std::string>().c_str(), 0, 16);
break; }}}
s += " - ";
s += std::to_string((int) Distance);
s += "m";
draw->AddText(NULL, ((float) density / 15.0f), {itemPos.X, itemPos.Y},IM_COL32(255,255,255,255),s.c_str());
}}}

}}}// ========================================================================= //
g_LocalController = localController;
g_LocalPlayer = localPlayer;
std::string s;
if (g_LocalController == 0) {
LOBBY = true;
GAME = false;
} else {
GAME = true;
LOBBY = false;
}
if (GAME || LOBBY) {
    auto FlagsCounter = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(ImVec2(center.x, 100), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(235.0f, 34.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(45, 170, 210, 220));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(6, 10, 20, 235));
    ImGui::Begin("Counter", 0, FlagsCounter);
    ImGui::PopStyleColor(2);
    
    if (GAME) {
        int totalAll = totalEnemies + totalBots;
        
        // ── BOT count (Teal) ──
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 180, 255));
        ImGui::Text("%s : %d", L("BOT", "توب"), totalBots);
        ImGui::PopStyleColor(1);
        
        ImGui::SameLine();
        ImGui::Text("     "); // spacer
        
        ImGui::SameLine();
        // ── PLAYER count (Gold) ──
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 50, 255));
        ImGui::Text("%s : %d", L("PLAYER", "بعلا"), totalEnemies);
        ImGui::PopStyleColor(1);
        
        ImGui::SameLine();
        ImGui::Text("     "); // spacer
        
        ImGui::SameLine();
        // ── TOTAL count (White) ──
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        ImGui::Text("%s : %d", L("TOTAL", "عومجم"), totalAll);
        ImGui::PopStyleColor(1);
    }
    
    if (LOBBY) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 180, 255));
        ImGui::Text("%s", L("In Lobby", "ةبهوءلا يف"));
        ImGui::PopStyleColor(1);
    }
    
    ImGui::End();
}
}}
/*
DrawImage(glWidth /2- 70, 45, 65, 30, PLAYER.textureId);
std::string s; s = ""; s += std::to_string(totalEnemies);
ImGui::GetForegroundDrawList()->AddText(NULL, ((float) density / 21.9f), {glWidth /2+-26.9f, 49.1f}, ImColor(255,255,255,255),s.c_str());  
}else{  
DrawImage(glWidth / 2- 70, 45, 65, 30, PLAYER.textureId);   
std::string s; s = ""; s += std::to_string(totalEnemies);
ImGui::GetForegroundDrawList()->AddText(NULL, ((float) density / 21.9f), {glWidth /2+-26.9f, 49.1f}, ImColor(255,255,255,255),s.c_str());}

if (totalBots > 0) {
DrawImage(glWidth / 2- 4.6f, 45, 65, 30, ROBOOT.textureId);
std::string s; s = ""; s += std::to_string(totalBots);
ImGui::GetForegroundDrawList()->AddText(NULL, ((float) density / 21.9f), {glWidth /2+30.6f, 49.1f}, ImColor(255,255,255,255),s.c_str()); 
}else{
DrawImage(glWidth / 2- 4.6f, 45, 65, 30, ROBOOT.textureId);
std::string s; s = ""; s += std::to_string(totalBots);
ImGui::GetForegroundDrawList()->AddText(NULL, ((float) density / 21.9f), {glWidth /2+30.6f, 49.1f}, ImColor(255,255,255,255),s.c_str());}
}}*/// ========================================================================= //
int OpenURL(const char* url) {
JavaVM* java_vm = g_App->activity->vm; JNIEnv* java_env = NULL;
jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
if (jni_return == JNI_ERR) return -1;
jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
if (jni_return != JNI_OK) return -2;
jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
if (native_activity_clazz == NULL) return -3;
jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "AndroidThunkJava_LaunchURL", "(Ljava/lang/String;)V");
if (method_id == NULL) return -4;
jstring retStr = java_env->NewStringUTF(url); java_env->CallVoidMethod(g_App->activity->clazz, method_id, retStr);
jni_return = java_vm->DetachCurrentThread();
if (jni_return != JNI_OK) return -5; return 0; }
// ========================================================================= //
std::string getClipboardText() {
if (!g_App) return "";
auto activity = g_App->activity;
if (!activity) return "";
auto vm = activity->vm;
if (!vm) return "";
auto object = activity->clazz;
if (!object) return "";
std::string result; JNIEnv *env;
vm->AttachCurrentThread(&env, 0); {
auto ContextClass = env->FindClass("android/content/Context");
auto getSystemServiceMethod = env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
auto str = env->NewStringUTF("clipboard");
auto clipboardManager = env->CallObjectMethod(object, getSystemServiceMethod, str);
env->DeleteLocalRef(str);
auto ClipboardManagerClass = env->FindClass("android/content/ClipboardManager");
auto getText = env->GetMethodID(ClipboardManagerClass, "getText", "()Ljava/lang/CharSequence;");
auto CharSequenceClass = env->FindClass("java/lang/CharSequence");
auto toStringMethod = env->GetMethodID(CharSequenceClass, "toString", "()Ljava/lang/String;");
auto text = env->CallObjectMethod(clipboardManager, getText);
if (text) {
str = (jstring) env->CallObjectMethod(text, toStringMethod);
result = env->GetStringUTFChars(str, 0);
env->DeleteLocalRef(str);
env->DeleteLocalRef(text);}
env->DeleteLocalRef(CharSequenceClass);
env->DeleteLocalRef(ClipboardManagerClass);
env->DeleteLocalRef(clipboardManager);
env->DeleteLocalRef(ContextClass);}
vm->DetachCurrentThread();
return result;}
// ========================================================================= //
const char *GetAndroidID(JNIEnv *env, jobject context) {
jclass contextClass = env->FindClass(/*android/content/Context*/ StrEnc("`L+&0^[S+-:J^$,r9q92(as", "\x01\x22\x4F\x54\x5F\x37\x3F\x7C\x48\x42\x54\x3E\x3B\x4A\x58\x5D\x7A\x1E\x57\x46\x4D\x19\x07", 23).c_str());
jmethodID getContentResolverMethod = env->GetMethodID(contextClass, /*getContentResolver*/ StrEnc("E8X\\7r7ys_Q%JS+L+~", "\x22\x5D\x2C\x1F\x58\x1C\x43\x1C\x1D\x2B\x03\x40\x39\x3C\x47\x3A\x4E\x0C", 18).c_str(), /*()Landroid/content/ContentResolver;*/ StrEnc("8^QKmj< }5D:9q7f.BXkef]A*GYLNg}B!/L", "\x10\x77\x1D\x2A\x03\x0E\x4E\x4F\x14\x51\x6B\x59\x56\x1F\x43\x03\x40\x36\x77\x28\x0A\x08\x29\x24\x44\x33\x0B\x29\x3D\x08\x11\x34\x44\x5D\x77", 35).c_str());
jclass settingSecureClass = env->FindClass(/*android/provider/Settings$Secure*/ StrEnc("T1yw^BCF^af&dB_@Raf}\\FS,zT~L(3Z\"", "\x35\x5F\x1D\x05\x31\x2B\x27\x69\x2E\x13\x09\x50\x0D\x26\x3A\x32\x7D\x32\x03\x09\x28\x2F\x3D\x4B\x09\x70\x2D\x29\x4B\x46\x28\x47", 32).c_str());
jmethodID getStringMethod = env->GetStaticMethodID(settingSecureClass, /*getString*/ StrEnc("e<F*J5c0Y", "\x02\x59\x32\x79\x3E\x47\x0A\x5E\x3E", 9).c_str(), /*(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;*/ StrEnc("$6*%R*!XO\"m18o,0S!*`uI$IW)l_/_knSdlRiO1T`2sH|Ouy__^}%Y)JsQ:-\"(2_^-$i{?H", "\x0C\x7A\x4B\x4B\x36\x58\x4E\x31\x2B\x0D\x0E\x5E\x56\x1B\x49\x5E\x27\x0E\x69\x0F\x1B\x3D\x41\x27\x23\x7B\x09\x2C\x40\x33\x1D\x0B\x21\x5F\x20\x38\x08\x39\x50\x7B\x0C\x53\x1D\x2F\x53\x1C\x01\x0B\x36\x31\x39\x46\x0C\x15\x43\x2B\x05\x30\x15\x41\x43\x46\x55\x70\x0D\x59\x56\x00\x15\x58\x73", 71).c_str());
auto obj = env->CallObjectMethod(context, getContentResolverMethod);
auto str = (jstring) env->CallStaticObjectMethod(settingSecureClass, getStringMethod, obj, env->NewStringUTF(/*android_id*/ StrEnc("ujHO)8OfOE", "\x14\x04\x2C\x3D\x46\x51\x2B\x39\x26\x21", 10).c_str()));
return env->GetStringUTFChars(str, 0);}
const char *GetDeviceModel(JNIEnv *env) {
jclass buildClass = env->FindClass(/*android/os/Build*/ StrEnc("m5I{GKGWBP-VOxkA", "\x0C\x5B\x2D\x09\x28\x22\x23\x78\x2D\x23\x02\x14\x3A\x11\x07\x25", 16).c_str());
jfieldID modelId = env->GetStaticFieldID(buildClass, /*MODEL*/ StrEnc("|}[q:", "\x31\x32\x1F\x34\x76", 5).c_str(), /*Ljava/lang/String;*/ StrEnc(".D:C:ETZ1O-Ib&^h.Y", "\x62\x2E\x5B\x35\x5B\x6A\x38\x3B\x5F\x28\x02\x1A\x16\x54\x37\x06\x49\x62", 18).c_str());
auto str = (jstring) env->GetStaticObjectField(buildClass, modelId);
return env->GetStringUTFChars(str, 0);}
const char *GetDeviceBrand(JNIEnv *env) {
jclass buildClass = env->FindClass(/*android/os/Build*/ StrEnc("0iW=2^>0zTRB!B90", "\x51\x07\x33\x4F\x5D\x37\x5A\x1F\x15\x27\x7D\x00\x54\x2B\x55\x54", 16).c_str());
jfieldID modelId = env->GetStaticFieldID(buildClass, /*BRAND*/ StrEnc("@{[FP", "\x02\x29\x1A\x08\x14", 5).c_str(), /*Ljava/lang/String;*/ StrEnc(".D:C:ETZ1O-Ib&^h.Y", "\x62\x2E\x5B\x35\x5B\x6A\x38\x3B\x5F\x28\x02\x1A\x16\x54\x37\x06\x49\x62", 18).c_str());
auto str = (jstring) env->GetStaticObjectField(buildClass, modelId);
return env->GetStringUTFChars(str, 0);}
const char *GetPackageName(JNIEnv *env, jobject context) {
jclass contextClass = env->FindClass(StrEnc("`L+&0^[S+-:J^$,r9q92(as", "\x01\x22\x4F\x54\x5F\x37\x3F\x7C\x48\x42\x54\x3E\x3B\x4A\x58\x5D\x7A\x1E\x57\x46\x4D\x19\x07", 23).c_str());
jmethodID getPackageNameId = env->GetMethodID(contextClass,StrEnc("YN4DaP)!{wRGN}", "\x3E\x2B\x40\x14\x00\x33\x42\x40\x1C\x12\x1C\x26\x23\x18", 14).c_str(),StrEnc("VnpibEspM(b]<s#[9cQD", "\x7E\x47\x3C\x03\x03\x33\x12\x5F\x21\x49\x0C\x3A\x13\x20\x57\x29\x50\x0D\x36\x7F", 20).c_str());
auto str = (jstring) env->CallObjectMethod(context, getPackageNameId);
return env->GetStringUTFChars(str, 0);}
const char *GetDeviceUniqueIdentifier(JNIEnv *env, const char *uuid) {
jclass uuidClass = env->FindClass(/*java/util/UUID*/ StrEnc("B/TxJ=3BZ_]SFx", "\x28\x4E\x22\x19\x65\x48\x47\x2B\x36\x70\x08\x06\x0F\x3C", 14).c_str());
auto len = strlen(uuid);
jbyteArray myJByteArray = env->NewByteArray(len);
env->SetByteArrayRegion(myJByteArray, 0, len, (jbyte *) uuid);
jmethodID nameUUIDFromBytesMethod = env->GetStaticMethodID(uuidClass, /*nameUUIDFromBytes*/ StrEnc("P6LV|'0#A+zQmoat,", "\x3E\x57\x21\x33\x29\x72\x79\x67\x07\x59\x15\x3C\x2F\x16\x15\x11\x5F", 17).c_str(), /*([B)Ljava/util/UUID;*/ StrEnc("sW[\"Q[W3,7@H.vT0) xB", "\x5B\x0C\x19\x0B\x1D\x31\x36\x45\x4D\x18\x35\x3C\x47\x1A\x7B\x65\x7C\x69\x3C\x79", 20).c_str());
jmethodID toStringMethod = env->GetMethodID(uuidClass, /*toString*/ StrEnc("2~5292eW", "\x46\x11\x66\x46\x4B\x5B\x0B\x30", 8).c_str(), /*()Ljava/lang/String;*/ StrEnc("P$BMc' #j?<:myTh_*h0", "\x78\x0D\x0E\x27\x02\x51\x41\x0C\x06\x5E\x52\x5D\x42\x2A\x20\x1A\x36\x44\x0F\x0B", 20).c_str());
auto obj = env->CallStaticObjectMethod(uuidClass, nameUUIDFromBytesMethod, myJByteArray);
auto str = (jstring) env->CallObjectMethod(obj, toStringMethod);
return env->GetStringUTFChars(str, 0);}
// ========================================================================= //
struct MemoryStruct {
char *memory; size_t size; };
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
size_t realsize = size * nmemb;
struct MemoryStruct *mem = (struct MemoryStruct *) userp;
mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
if (mem->memory == NULL) {return 0;}
memcpy(&(mem->memory[mem->size]), contents, realsize);
mem->size += realsize;
mem->memory[mem->size] = 0;
return realsize;}
// =============================》ANTI CRACK《=============================== //
int hussienClose() {
JavaVM* java_vm = g_App->activity->vm;
JNIEnv* java_env = NULL;
jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
if (jni_return == JNI_ERR) return -1;
jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
if (jni_return != JNI_OK) return -2;
jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
if (native_activity_clazz == NULL) return -3;
jmethodID method_id = java_env->GetMethodID(native_activity_clazz, OBFUSCATE("AndroidThunkJava_RestartGame"),/*hussien New Restart*/OBFUSCATE("()V"));
if (method_id == NULL) return -4;
java_env->CallVoidMethod(g_App->activity->clazz, method_id);
jni_return = java_vm->DetachCurrentThread();
if (jni_return != JNI_OK) return -5; return 0; }
bool ishussienFolderHere(const std::string& folderPath) { return (access(folderPath.c_str(), F_OK) == 0); }
// ======================================================================== //
void hussienAntiCrack1() {
std::string folderPath = XorStr("/storage/emulated/0/Android/data/com.guoshi.httpcanary");
if (ishussienFolderHere(folderPath)) {hussienClose(); } else {}}
void hussienAntiCrack2() {
std::string folderPath = XorStr("/storage/emulated/0/Android/data/com.guoshi.httpcanary.premium");
if (ishussienFolderHere(folderPath)) {hussienClose(); } else {}}
void hussienAntiCrack3() {
std::string folderPath = XorStr("/storage/emulated/0/Android/data/com.sniffer");
if (ishussienFolderHere(folderPath)) {hussienClose(); } else {}}
void hussienAntiCrack4() {
std::string folderPath = XorStr("/storage/emulated/0/Android/data/com.httpcanary.pro");
if (ishussienFolderHere(folderPath)) {hussienClose(); } else {}}
void hussienAntiCrack5() {
std::string folderPath = XorStr("/storage/emulated/0/Android/data/com.sanmeet");
if (ishussienFolderHere(folderPath)) {hussienClose(); } else {}}
void hussienAntiCrack6() {
std::string folderPath = XorStr("/storage/emulated/0/Android/data/ROKMOD.COM");
if (ishussienFolderHere(folderPath)) {hussienClose(); } else {}}
// ========================================================================= //
std::string Login(const char *user_key) {
if (!g_App) return "Internal Error";
auto activity = g_App->activity;
if (!activity) return "Internal Error";
auto vm = activity->vm;
if (!vm) return "Internal Error";
auto object = activity->clazz;
if (!object) return "Internal Error";
JNIEnv *env; vm->AttachCurrentThread(&env, 0);
std::string hwid = user_key;
hwid += GetAndroidID(env, object);
hwid += GetDeviceModel(env);
hwid += GetDeviceBrand(env);
std::string UUID = GetDeviceUniqueIdentifier(env, hwid.c_str());
vm->DetachCurrentThread();
std::string errMsg;
struct MemoryStruct chunk{};
chunk.memory = (char *) malloc(1);
chunk.size = 0;

CURL *curl;
CURLcode res;
curl = curl_easy_init();

if (curl) {
curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, /*POST*/ StrEnc(",IL=", "\x7C\x06\x1F\x69", 4).c_str());
std::string LoginLastCheat = OBFUSCATE("https://venomkey.com/connect");//سيرفرك
curl_easy_setopt(curl, CURLOPT_URL, LoginLastCheat.c_str());

curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, /*https*/ StrEnc("!mLBO", "\x49\x19\x38\x32\x3C", 5).c_str());
struct curl_slist *headers = NULL;
headers = curl_slist_append(headers, /*Content-Type: application/x-www-form-urlencoded*/ StrEnc("@;Ls\\(KP4Qrop`b#d3094/r1cf<c<=H)AiiBG6i|Ta66s2[", "\x03\x54\x22\x07\x39\x46\x3F\x7D\x60\x28\x02\x0A\x4A\x40\x03\x53\x14\x5F\x59\x5A\x55\x5B\x1B\x5E\x0D\x49\x44\x4E\x4B\x4A\x3F\x04\x27\x06\x1B\x2F\x6A\x43\x1B\x10\x31\x0F\x55\x59\x17\x57\x3F", 47).c_str());
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

char data[4096];
sprintf(data, /*game=PUBG&user_key=%s&serial=%s*/ StrEnc("qu2yXK,YkJyGD@ut0.u~Nb'5(:.:chK", "\x16\x14\x5F\x1C\x65\x1B\x79\x1B\x2C\x6C\x0C\x34\x21\x32\x2A\x1F\x55\x57\x48\x5B\x3D\x44\x54\x50\x5A\x53\x4F\x56\x5E\x4D\x38", 31).c_str(), user_key, UUID.c_str());
curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);

curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

res = curl_easy_perform(curl);
if (res == CURLE_OK) {
try {
json result = json::parse(chunk.memory);
if (result[/*status*/ StrEnc("(>_LBm", "\x5B\x4A\x3E\x38\x37\x1E", 6).c_str()] == true) {
std::string token = result[/*data*/ StrEnc("fAVA", "\x02\x20\x22\x20", 4).c_str()][/*token*/ StrEnc("{>3Lr", "\x0F\x51\x58\x29\x1C", 5).c_str()].get<std::string>();
time_t rng = result[/*data*/ StrEnc("fAVA", "\x02\x20\x22\x20", 4).c_str()][/*rng*/ StrEnc("+n,", "\x59\x00\x4B", 3).c_str()].get<time_t>();
if (rng + 30 > time(0)) {
std::string auth = /*PUBG*/ StrEnc("Q*) ", "\x01\x7F\x6B\x67", 4).c_str();;
auth += "-";
auth += user_key;
auth += "-";
auth += UUID;
auth += "-";
auth += /*Vm8Lk7Uj2JmsjCPVPVjrLa7zgfx3uz9E*/ StrEnc("-2:uwZdV^%]?{{wHs2V,+(^NJU;kC*_{", "\x7B\x5F\x02\x39\x1C\x6D\x31\x3C\x6C\x6F\x30\x4C\x11\x38\x27\x1E\x23\x64\x3C\x5E\x67\x49\x69\x34\x2D\x33\x43\x58\x36\x50\x66\x3E", 32).c_str();
std::string outputAuth = Tools::CalcMD5(auth);

g_Token = token;
g_Auth = outputAuth;

bValid = g_Token == g_Auth;
}
} else {


errMsg = result[/*reason*/ StrEnc("LW(3(c", "\x3E\x32\x49\x40\x47\x0D", 6).c_str()].get<std::string>();
}
} catch (json::exception &e) {
errMsg = "{";
errMsg += e.what();
errMsg += "}\n{";
errMsg += chunk.memory;
errMsg += "}";
}
} else {
errMsg = curl_easy_strerror(res);
}
}
curl_easy_cleanup(curl);

return bValid ? "OK" : errMsg;
}
#include "Mod/draw.h"

// ========================================================================= //
void DrawTextCentered(const char *text)
{
ImGui::Separator();
ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(text).x) / 5.f);
ImGui::Text(text);
ImGui::Separator();
}
// ========================================================================= // 

#define IM_CLAMP(V, MN, MX) ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))
namespace Settings {static int Tab = 1;}



EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

EGLBoolean _eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);
if (glWidth <= 0 || glHeight <= 0)
return orig_eglSwapBuffers(dpy, surface);

if (!g_App)
return orig_eglSwapBuffers(dpy, surface);

screenWidth = ANativeWindow_getWidth(g_App->window);
screenHeight = ANativeWindow_getHeight(g_App->window);
density = AConfiguration_getDensity(g_App->config);


if (!initImGui) {
ImGui::CreateContext();
ImGuiStyle *style = &ImGui::GetStyle();
InitTexture();
style->WindowPadding = ImVec2(8, 8);
style->WindowRounding = 5.50f;
style->FramePadding = ImVec2(4, 3);
style->FrameRounding = 6.2f;
style->FrameBorderSize = 2.0f;
style->WindowBorderSize = 2.0f;
style->TabRounding = 2.0f;

// 🔹 محاذاة
style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
style->ButtonTextAlign  = ImVec2(0.5f, 0.5f);

// 🔹 الشكل العام
style->WindowRounding = 8.0f;
style->FrameRounding  = 6.0f;
style->TabRounding    = 6.0f;

style->Colors[ImGuiCol_WindowBg] = ImColor(7, 10, 20, 255);  // Dark / hidden background

// 🔹 العنوان (فوق)
style->Colors[ImGuiCol_TitleBg]           = ImColor(12, 18, 35, 255);
style->Colors[ImGuiCol_TitleBgActive]     = ImColor(18, 28, 55, 255);
style->Colors[ImGuiCol_TitleBgCollapsed]  = ImColor(10, 15, 30, 255);

// 🔹 الفريمات (Input / Checkbox / إلخ)
style->Colors[ImGuiCol_FrameBg]           = ImColor(15, 22, 42, 230);
style->Colors[ImGuiCol_FrameBgHovered]    = ImColor(25, 45, 75, 245);
style->Colors[ImGuiCol_FrameBgActive]     = ImColor(35, 65, 105, 255);

// 🔹 الأزرار (أزرق داكن)
style->Colors[ImGuiCol_Button]            = ImColor(15, 25, 50, 230);
style->Colors[ImGuiCol_ButtonHovered]     = ImColor(25, 60, 100, 245);
style->Colors[ImGuiCol_ButtonActive]      = ImColor(40, 90, 145, 255);

// 🔹 التابات
style->Colors[ImGuiCol_Tab]               = ImColor(12, 20, 40, 235);
style->Colors[ImGuiCol_TabHovered]        = ImColor(30, 65, 105, 255);
style->Colors[ImGuiCol_TabActive]         = ImColor(25, 50, 90, 255);

// 🔹 الهيدر (Lists / Sections)
style->Colors[ImGuiCol_Header]            = ImColor(15, 25, 50, 230);
style->Colors[ImGuiCol_HeaderHovered]     = ImColor(25, 60, 100, 245);
style->Colors[ImGuiCol_HeaderActive]      = ImColor(40, 90, 145, 255);

// 🔹 Popup
style->Colors[ImGuiCol_PopupBg]           = ImColor(8, 13, 28, 245);

// 🔹 Menu bar
style->Colors[ImGuiCol_MenuBarBg]         = ImColor(10, 16, 32, 255);

// 🔹 Border
style->Colors[ImGuiCol_Border]            = ImColor(45, 100, 150, 180);
style->Colors[ImGuiCol_BorderShadow]      = ImColor(0, 0, 0, 0);

ImGui_ImplAndroid_Init();
 ImGui_ImplOpenGL3_Init("#version 300 es");

ImGuiIO &io = ImGui::GetIO();
io.ConfigWindowsMoveFromTitleBarOnly = true;
io.IniFilename = NULL;
static const ImWchar icons_ranges[] = {0xf000, 0xf3ff, 0};
ImFontConfig icons_config;
icons_config.MergeMode = true;
icons_config.PixelSnapH = true;
icons_config.OversampleH = 2.5;
icons_config.OversampleV = 2.5;
static const ImWchar ranges[] = {0x0020, 0x00FF,0x2010, 0x205E,0x0600, 0x06FF,0xFE00, 0xFEFF,0,};

ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Tahomabd_data, Tahomabd_size, 20.0f, NULL, &ranges[0]); 
io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 20.0f, &icons_config, icons_ranges);

ImFontConfig CustomFont;
CustomFont.FontDataOwnedByAtlas = false;
ImFontConfig cfg;
cfg.SizePixels = ((float)density / 100.0f);

io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 22.f, &CustomFont);
io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 20.0f, &icons_config, icons_ranges);

memset(&Config, 0, sizeof(sConfig));
//===============================| 𝗘𝗦𝗣 𝗖𝗢𝗟𝗢𝗥𝗦 |================================== //
Config.ColorsESP.Fov = CREATE_COLOR(255, 0, 0, 255);

for (auto &i : items_data) {
for (auto &item : i["Items"]) {
int r, g, b;
sscanf(item["itemTextColor"].get<std::string>().c_str(), "#%02X%02X%02X", &r, &g, &b);
ItemColors[item["itemId"].get<int>()] = CREATE_COLOR(r, g, b, 255);
}}
initImGui = true;
}

///https://t.me/NB_Q1https://t.me/NB_Q1https://t.me/NB_Q1https://t.me/NB_Q1https://t.me/NB_Q1
//@QL_T3
 ImGuiIO &io = ImGui::GetIO();


ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
ImGui::NewFrame();

if (ImGui::Begin(OBFUSCATE(" open" ), 0,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground))  
 {

ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding ,50.0f);
ImGui::PushStyleColor( ImGuiCol_Button,ImColor(255, 0, 0, 0).Value);
ImGui::PushStyleColor( ImGuiCol_ButtonHovered,ImColor(255, 0, 0, 0).Value);
ImGui::PushStyleColor( ImGuiCol_ButtonActive,ImColor(255, 0, 0, 0).Value);
if (show == false) {

if(ImGui::ImageButton(LUCKYHUB.textureId, ImVec2(80, 80) )) {
show = true;
}}

ImGui::PopStyleColor(3);
ImGui::PopStyleVar();

ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding ,50.0f);
ImGui::PushStyleColor( ImGuiCol_Button,ImColor(0, 0, 0, 0).Value);
ImGui::PushStyleColor( ImGuiCol_ButtonHovered,ImColor(0, 0, 0, 0).Value);
ImGui::PushStyleColor( ImGuiCol_ButtonActive,ImColor(0, 0, 0, 0).Value);
if (show == true) {
if (ImGui::ImageButton(LUCKYHUB.textureId, ImVec2(80, 80) )) {
}}}

ImGui::PopStyleColor(3);
ImGui::PopStyleVar();

io.MouseDrawCursor = false;
DrawESP(ImGui::GetBackgroundDrawList());
	
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
// ── أنيميشن فتح/إغلاق المنيو ─────────────────────────────────────────────
static float menuAlpha    = 0.0f;  // 0=مخفي 1=ظاهر
static float menuSlideY   = -30.0f; // offset عمودي للـ slide-in
static bool  prevShow     = false;
{
    float dt2 = io.DeltaTime;
    float alphaTarget  = show ? 1.0f : 0.0f;
    float slideTarget  = show ? 0.0f : -30.0f;
    menuAlpha  += (alphaTarget  - menuAlpha)  * (1.0f - expf(-dt2 * 14.0f));
    menuSlideY += (slideTarget  - menuSlideY) * (1.0f - expf(-dt2 * 14.0f));
    prevShow = show;
}

if(show || menuAlpha > 0.02f){
ImGui::SetNextWindowSize(ImVec2((float) glWidth * 0.50f, (float) glHeight * 0.765f), ImGuiCond_Once);

// ── تطبيق الـ Alpha و Slide ──────────────────────────────
ImGui::SetNextWindowBgAlpha(menuAlpha * 0.97f);
// Slide: نحرك الـ window للأسفل عند الظهور ثم يرجع
ImVec2 wPos = ImGui::GetWindowPos();
// نستخدم constraint لنحرك النافذة
ImGui::SetNextWindowPos(ImVec2(wPos.x, wPos.y + menuSlideY), ImGuiCond_Always, ImVec2(0,0));

static ImVec4 active = ImColor(0, 0, 0, 255);
 static ImVec4 inactive = ImColor(0, 0, 0, 255);
static bool p_open = true;
   char buf[128];
sprintf(buf, (OBFUSCATE(ICON_FA_USERS" 𝐒𝐋𝐎𝐎𝐌 𝐌𝐎𝐃 𝐅𝐑𝐄𝐄 𝟔𝟒𝐁𝐈𝐓 | FPS %0.2f ###AnimatedTitle")),(io.Framerate), ImGui::GetFrameCount());


ImGui::Begin(buf, &show, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse /* + ImGuiWindowFlags_NoTitleBar*/);

// English/Arabic switch affects UI labels only; window/layout direction stays unchanged.
// ── SLOOM MOD identity + language switch ──
ImGui::TextColored(ImVec4(0.20f, 0.85f, 1.00f, 1.00f), "𝐒𝐋𝐎𝐎𝐌 𝐌𝐎𝐃 𝐅𝐑𝐄𝐄 𝟔𝟒𝐁𝐈𝐓");
ImGui::SameLine();
ImGui::TextColored(ImVec4(0.60f, 0.45f, 1.00f, 1.00f), "@rv_nw");
ImGui::SameLine();
if (ImGui::Button(gArabicLanguage ? "English" : "العربية")) {
    gArabicLanguage = !gArabicLanguage;
}
ImGui::Separator();


static bool isLogin = false;
  //No Key:-true
  //By Key:-false
static std::string err;
if (!isLogin) {
ImGui::Text("%s", L("Please Login! (Copy Key to Clipboard)", "ةظوفحملا ةخسنل حاتفملا خسنا :لوخدلا ليجست"));

ImGui::PushItemWidth(-1);
static char s[64];
ImGui::InputText("##key", s, sizeof s);
// auto paste
auto key = getClipboardText();
strncpy(s, key.c_str(), sizeof s);
// auto login

err = Login(s);
if (err == "OK") {
isLogin = bValid && g_Auth == g_Token;
}
ImGui::PopItemWidth();

ImGui::PushItemWidth(-1);
if (ImGui::Button(L(" Paste Key  ", "حاتفملا قصل"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
auto key = getClipboardText();
strncpy(s, key.c_str(), sizeof s);
}
ImGui::PopItemWidth();

ImGui::PushItemWidth(-1);


if (ImGui::Button(L("Login", "لوخد"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
err = Login(s);
if (err == "OK") {
isLogin = bValid && g_Auth == g_Token;
}
}
ImGui::PopItemWidth();

if (!err.empty() && err != "OK") {
ImGui::Text("%s : %s", L("Error", "أطخ"), err.c_str());
}

ImGui::PopItemWidth();


} else { 

const auto& pWindowDrawList = ImGui::GetWindowDrawList();
ImDrawList* pDrawList;  
 pDrawList = pWindowDrawList;
// ═══════════════════════════════════════════════════════════════
// ── "SIDHU MOOSE WALA" ORBITING NAME BACKGROUND ─────────────
// ═══════════════════════════════════════════════════════════════
static float orbitTime = 0.0f;
orbitTime += ImGui::GetIO().DeltaTime * 0.6f;

ImVec2 winPos  = ImGui::GetWindowPos();
ImVec2 winSize = ImGui::GetWindowSize();
float cx = winPos.x + winSize.x * 0.5f;
float cy = winPos.y + winSize.y * 0.5f;
float radius = (winSize.x < winSize.y ? winSize.x : winSize.y) * 0.28f;
if (radius < 60.0f) radius = 60.0f;

const char* nameText = "SIDHU MOOSE WALA";
ImVec2 textSize = ImGui::CalcTextSize(nameText);

// ── Draw multiple orbiting instances ──
int count = 6;
for (int i = 0; i < count; i++) {
    float angle = orbitTime + (6.283185f / count) * i;
    float x = cx + cosf(angle) * radius;
    float y = cy + sinf(angle) * radius;
    
    // Size grows/shrinks with depth (3D illusion)
    float depthFactor = 0.6f + 0.4f * (cosf(angle) * 0.5f + 0.5f);
    float fontSize = 16.0f * depthFactor;
    
    // Color fades with depth
    float alpha = 0.08f + 0.12f * depthFactor;
    
    // Golden/amber color cycling per instance
    float hueOffset = (float)i / count;
    float r = 1.0f;
    float g = 0.70f + 0.30f * sinf(orbitTime * 0.5f + hueOffset * 6.283f);
    float b = 0.15f + 0.20f * sinf(orbitTime * 0.7f + hueOffset * 6.283f + 2.0f);
    
    pDrawList->AddText(
        NULL,
        fontSize,
        ImVec2(x - textSize.x * depthFactor * 0.5f,
               y - textSize.y * depthFactor * 0.5f),
        IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(alpha * 255)),
        nameText
    );
}

// ── Center glow pulse ──
float pulse = 0.5f + 0.5f * sinf(orbitTime * 1.2f);
pDrawList->AddCircleFilled(
    ImVec2(cx, cy),
    15.0f + pulse * 12.0f,
    IM_COL32(255, 200, 50, (int)(pulse * 20))
);

// ══════════════════════════════════════════════════════════════════
// ── BOTTOM TAB BAR (Original Button Style) ──────────────────────
// ══════════════════════════════════════════════════════════════════
ImGui::Separator();
ImGui::Spacing();

ImVec2 p_min, p_max;
ImDrawList* draw = ImGui::GetWindowDrawList();

float btnW = (ImGui::GetContentRegionAvailWidth() - 24.0f) / 5.0f;
if (btnW < 55.0f) btnW = 55.0f;

// ── ESP ──
ImGui::PushStyleColor(ImGuiCol_Button, Settings::Tab == 1 ? active : inactive);
if (ImGui::Button((std::string(ICON_FA_EYE_SLASH) + L("ESP", "سبيإ" )).c_str(), ImVec2(btnW, 38)))
    Settings::Tab = 1;
p_min = ImGui::GetItemRectMin();
p_max = ImGui::GetItemRectMax();
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 40), 6.0f, 0, 6.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 25), 6.0f, 0, 10.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 15), 6.0f, 0, 14.0f);
ImGui::PopStyleColor(1);

ImGui::SameLine(0, 4);

// ── AIM ──
ImGui::PushStyleColor(ImGuiCol_Button, Settings::Tab == 2 ? active : inactive);
if (ImGui::Button((std::string(ICON_FA_CROSSHAIRS) + L("AIM", "ميا" )).c_str(), ImVec2(btnW, 38)))
    Settings::Tab = 2;
p_min = ImGui::GetItemRectMin();
p_max = ImGui::GetItemRectMax();
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 40), 6.0f, 0, 6.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 25), 6.0f, 0, 10.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 15), 6.0f, 0, 14.0f);
ImGui::PopStyleColor(1);

ImGui::SameLine(0, 4);

// ── BOLT ──
ImGui::PushStyleColor(ImGuiCol_Button, Settings::Tab == 5 ? active : inactive);
if (ImGui::Button((std::string(ICON_FA_CROSSHAIRS) + L("BOLT", "تلو" )).c_str(), ImVec2(btnW, 38)))
    Settings::Tab = 5;
p_min = ImGui::GetItemRectMin();
p_max = ImGui::GetItemRectMax();
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 40), 6.0f, 0, 6.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 25), 6.0f, 0, 10.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 15), 6.0f, 0, 14.0f);
ImGui::PopStyleColor(1);

ImGui::SameLine(0, 4);

// ── SKIN ──
ImGui::PushStyleColor(ImGuiCol_Button, Settings::Tab == 3 ? active : inactive);
if (ImGui::Button((std::string(ICON_FA_GAMEPAD) + L("SKIN", "نكس" )).c_str(), ImVec2(btnW, 38)))
    Settings::Tab = 3;
p_min = ImGui::GetItemRectMin();
p_max = ImGui::GetItemRectMax();
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 40), 6.0f, 0, 6.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 25), 6.0f, 0, 10.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 15), 6.0f, 0, 14.0f);
ImGui::PopStyleColor(1);

ImGui::SameLine(0, 4);

// ── MEMORY ──
ImGui::PushStyleColor(ImGuiCol_Button, Settings::Tab == 4 ? active : inactive);
if (ImGui::Button((std::string(ICON_FA_ROCKET) + L("MEMORY", "ةركاذ" )).c_str(), ImVec2(btnW, 38)))
    Settings::Tab = 4;
p_min = ImGui::GetItemRectMin();
p_max = ImGui::GetItemRectMax();
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 40), 6.0f, 0, 6.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 25), 6.0f, 0, 10.0f);
draw->AddRect(p_min, p_max, IM_COL32(0, 150, 255, 15), 6.0f, 0, 14.0f);
ImGui::PopStyleColor(1);

ImGui::Spacing();
    
ImGui::NextColumn();


if (Settings::Tab == 1) {
if (ImGui::Button(L("ESP", "سبيإ"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0)));
ImGui::Separator();
//ImGui::RadioButton("VIP BYPASS", &Config.Bypass);
ImGui::Separator();

// ── ESP Toggle List (Switch left, Name right) ──
{
    struct ToggleItem { const char* name; bool* value; };
    ToggleItem toggles[] = {
        {L("LINE", "طخ"),      &Config.PlayerESP.Line     },
        {L("BOX", "قودنص"),       &Config.PlayerESP.Box      },
        {L("NAME", "مسا"),      &Config.PlayerESP.Name     },
        {L("SKELETON", "لكيه"),  &Config.PlayerESP.Skeleton },
        {L("HEALTH", "ةحص"),    &Config.PlayerESP.Health   },
        {L("HEALTH PC", "ةحص ةبسن"), &Config.PlayerESP.HealthPC },
        {L("DISTANCE", "ةفاسم"),  &Config.PlayerESP.Distance },
        {L("ALERT", "هيبنت"),     &Config.PlayerESP.Grenade  },
        {L("VEHICLE", "ةرايس"),   &Config.PlayerESP.Vehicle  },
    };

    for (auto& t : toggles) {
        ToggleSwitch(t.name, t.value);
        ImGui::Spacing();
    }
}

ImGui::Spacing();

}
if (Settings::Tab == 2) {
if (ImGui::Button(L("AIM", "ميا"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0)));
ImGui::Separator();
ToggleSwitch(L("Enable AimBot", "فدهتسملا فدهلا ليعفت"), &Config.AimBot.Enable);
ImGui::Spacing();
ImGui::SliderFloat(L("Fov", "وف"),&Config.AimBot.Cross, 0.0f, 250.0f);
static const char *Targets[] = {"Head", "Body"};
ImGui::SliderFloat(L("Meter", "رتم"),&Config.AimBot.Meter, 0.0f, 250.0f);
ImGui::SliderFloat(L("Recc", "دترم"),&Config.AimBot.Recc, 0.0f, 2.0f);
ImGui::Spacing();
ToggleSwitch(L("Ignore Knocked", "طوقسملا لهاجت"), &Config.AimBot.IgnoreKnocked);
ToggleSwitch(L("Ignore Bot", "توبلا لهاجت"), &Config.AimBot.IgnoreBot);
Config.AimBot.Shooting = true;
Config.AimBot.VisCheck = true;
}
if (Settings::Tab == 5) {
if (ImGui::Button(L("BOLT", "تلو"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0)));
ImGui::Separator();
ToggleSwitch(L("Silent Aim", "تماصلا فدهلا"), &Config.SilentAim.Enable);
ImGui::Spacing();
ImGui::SliderFloat(L("Fov", "وف"),&Config.SilentAim.Cross, 0.0f, 250.0f);
static const char *Targets[] = {"Head", "Body"};
ImGui::SliderFloat(L("Meter", "رتم"),&Config.SilentAim.Meter, 0.0f, 250.0f);
ImGui::SliderFloat(L("Recc", "دترم"),&Config.SilentAim.Recc, 0.0f, 2.0f);
ImGui::Spacing();
ToggleSwitch(L("Ignore Knocked", "طوقسملا لهاجت"), &Config.SilentAim.IgnoreKnocked);
ToggleSwitch(L("Ignore Bot", "توبلا لهاجت"), &Config.SilentAim.IgnoreBot);
Config.SilentAim.Shooting = true;
Config.SilentAim.VisCheck = true;
}
if (Settings::Tab == 3) {
if (ImGui::Button(L("Skin", "نكس"), ImVec2(ImGui::GetContentRegionAvailWidth(), 0)));
ImGui::Separator();
ToggleSwitch(L("Mod Skin", "نكسلا ليدعت"), &ModSkinn);
if (preferences.Outfit) {
start = std::chrono::high_resolution_clock::now();
prevXSuits = preferences.Config.Skin.XSuits;}
if (ImGui::BeginTabBar("Taba1", ImGuiTabBarFlags_FittingPolicyScroll)) {
//=============================================
if (ImGui::BeginTabItem(L("Outfit", "سبالملا"))) {
if (ImGui::BeginTable("##ModOutfit", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner)) {
ImGui::TableSetupColumn("Enable", 0, 50);
ImGui::TableSetupColumn("Skin", 0, 70);
ImGui::TableSetupColumn("Name", 0, 110);
ImGui::TableHeadersRow();
ImGui::TableNextRow();
ImGui::TableNextColumn();
ImGui::Checkbox("PlayerSkin", &preferences.Outfit);
ImGui::TableNextColumn();
ImGui::InputInt("##XSUIT", &preferences.Config.Skin.XSuits);
ImGui::TableNextColumn();
ImGui::Text(SkinSetName.c_str());

ImGui::TableNextColumn();

ImGui::Checkbox("BagSkin", &preferences.Bag);
ImGui::TableNextColumn();
ImGui::InputInt("##backpacks", &preferences.Config.Skin.bag);
ImGui::TableNextColumn();
ImGui::Text(SkinBagName.c_str());

ImGui::TableNextColumn();

ImGui::Checkbox("HelmetSkin", &preferences.Helmet);
ImGui::TableNextColumn();
ImGui::InputInt("##helmets", &preferences.Config.Skin.helmet);
ImGui::TableNextColumn();
ImGui::Text(SkinHelmetName.c_str());

ImGui::TableNextColumn();

ImGui::EndTable();
}
ImGui::EndTabItem();
}
//=============================================
if (ImGui::BeginTabItem(L("Car Skin", "تارايسلا تاناكس"))) {
if (ImGui::BeginTable("##ModCar", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner)) {
ImGui::TableSetupColumn("Enable", 0, 50);
ImGui::TableSetupColumn("Name", 0, 60);
ImGui::TableSetupColumn("Skin", 0, 60);
ImGui::TableHeadersRow();
ImGui::TableNextRow();

ImGui::TableNextColumn();

ImGui::Checkbox("###0", &preferences.Dacia);
ImGui::TableNextColumn();
ImGui::Text("Dacia");
ImGui::TableNextColumn();
ImGui::InputInt("##card", &preferences.Config.Skin.Dacia);
ImGui::TableNextColumn();


ImGui::Checkbox("###1", &preferences.CoupeRB);
ImGui::TableNextColumn();
ImGui::Text("Coupe RB");
ImGui::TableNextColumn();
ImGui::InputInt("##carc", &preferences.Config.Skin.CoupeRP);
ImGui::TableNextColumn();


ImGui::Checkbox("###2", &preferences.UAZ);
ImGui::TableNextColumn();
ImGui::Text("UAZ");
ImGui::TableNextColumn();
ImGui::InputInt("##caru", &preferences.Config.Skin.UAZ);
ImGui::TableNextColumn();

ImGui::Checkbox("###23", &preferences.Moto);
ImGui::TableNextColumn();
ImGui::Text("MotoBike");
ImGui::TableNextColumn();
ImGui::InputInt("##moto", &preferences.Config.Skin.Moto);

ImGui::TableNextColumn();
ImGui::Checkbox("###235", &preferences.BigFoot);
ImGui::TableNextColumn();
ImGui::Text("BigFoot");
ImGui::TableNextColumn();
ImGui::InputInt("##Bigfoot", &preferences.Config.Skin.Bigfoot);

ImGui::TableNextColumn();
ImGui::Checkbox("###2345", &preferences.Mirado);
ImGui::TableNextColumn();
ImGui::Text("Mirado");
ImGui::TableNextColumn();
ImGui::InputInt("##OMirado", &preferences.Config.Skin.Mirado);

ImGui::TableNextColumn();
ImGui::Checkbox("###2365", &preferences.Buggy);
ImGui::TableNextColumn();
ImGui::Text("Buggy");
ImGui::TableNextColumn();
ImGui::InputInt("##carc", &preferences.Config.Skin.Buggy);

ImGui::TableNextColumn();
ImGui::Checkbox("###234995", &preferences.MiniBus);
ImGui::TableNextColumn();
ImGui::Text("MiniBus");
ImGui::TableNextColumn();
ImGui::InputInt("##miniB", &preferences.Config.Skin.MiniBus);

ImGui::TableNextColumn();
ImGui::Checkbox("###23650", &preferences.Boat);
ImGui::TableNextColumn();
ImGui::Text("PG-117");
ImGui::TableNextColumn();
ImGui::InputInt("##bg77", &preferences.Config.Skin.Boat);

ImGui::EndTable();
}
ImGui::EndTabItem();
}
//=============================================
if (ImGui::BeginTabItem(L(" Weapon ", "حلسا " ))) {
ImGui::Checkbox("kill message", &KillMessage);
ImGui::SameLine();
ImGui::Checkbox("DeadBox", &DeadBox);
ImGui::SameLine();
ImGui::Checkbox("HideName", &HideName);
if (ImGui::BeginTable("##Mod Weaponr", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner)) {
ImGui::TableSetupColumn("Name", 0, 60);
ImGui::TableSetupColumn("Skin", 0, 100);
ImGui::TableHeadersRow();
ImGui::TableNextRow();
ImGui::TableNextColumn();
ImGui::Text("M416");
ImGui::TableNextColumn();
const char* m416Skins[] = {"Normal","Glacier - M416","The Fool - M416","Wanderer - M416","Lizard Roar - M416","Call of the Wild - M416","TechnoCore - M416","Imperial Splendor - M416","Silver Guru - M416","Tidal Embrace - M416","Roaring Immolation - M416","Shinobi Kami - M416","Sealed Nether - M416","M416_Fire"};
ImGui::Combo("##M416", &preferences.Config.Skin.M416, m416Skins, IM_ARRAYSIZE(m416Skins));
ImGui::TableNextColumn();
ImGui::Text("AKM");
ImGui::TableNextColumn();
ImGui::InputInt("##2", &preferences.Config.Skin.AKM);
ImGui::TableNextColumn();
ImGui::Text("SCAR-L SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##3", &preferences.Config.Skin.Scar);
ImGui::TableNextColumn();
ImGui::Text("M762 SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##4", &preferences.Config.Skin.M762);
ImGui::TableNextColumn();
ImGui::Text("GROZA SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##5", &preferences.Config.Skin.Groza);
ImGui::TableNextColumn();
ImGui::Text("AUG SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##6", &preferences.Config.Skin.AUG);
ImGui::TableNextColumn();
ImGui::Text("M16A4 SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##7", &preferences.Config.Skin.M16A4);
ImGui::TableNextColumn();
ImGui::Text("ACE32 SKIN ");
ImGui::TableNextColumn();
ImGui::InputInt("##8", &preferences.Config.Skin.ACE32);
ImGui::TableNextColumn();
ImGui::Text("KAR98K SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##9", &preferences.Config.Skin.K98);
ImGui::TableNextColumn();
ImGui::Text("M24 SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##10", &preferences.Config.Skin.M24);
ImGui::TableNextColumn();
ImGui::Text("AWM SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##11", &preferences.Config.Skin.AWM);
ImGui::TableNextColumn();
ImGui::Text("DP28 SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##12", &preferences.Config.Skin.DP28);
ImGui::TableNextColumn();
ImGui::Text("M249 SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##13", &preferences.Config.Skin.M249);
ImGui::TableNextColumn();
ImGui::Text("UZI SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##14", &preferences.Config.Skin.UZI);
ImGui::TableNextColumn();
ImGui::Text("UMP SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##15", &preferences.Config.Skin.UMP);
ImGui::TableNextColumn();
ImGui::Text("Thompson SKIN ");
ImGui::TableNextColumn();
ImGui::InputInt("##16", &preferences.Config.Skin.Thompson);
ImGui::TableNextColumn();
ImGui::Text("BIZON SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##17", &preferences.Config.Skin.Bizon);
ImGui::TableNextColumn();
ImGui::Text("PAN SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##18", &preferences.Config.Skin.Pan);
ImGui::TableNextColumn();
ImGui::Text("MG3 SKIN");
ImGui::TableNextColumn();
ImGui::InputInt("##19", &preferences.Config.Skin.MG3);
ImGui::TableNextColumn();
ImGui::EndTable();
}
ImGui::EndTabItem();
}}}

if (Settings::Tab == 4) {
if (ImGui::Button("SDK", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)));
ImGui::Separator();

// ── MEMORY Toggle List ──
{
    struct ToggleItem { const char* name; bool* value; };
    ToggleItem memToggles[] = {
        {"IPAD VIEW",  &Config.PlayerESP.Ipad      },
        {"HIT EFFECT", &Config.PlayerESP.Small      },
        {"HET-X",      &Config.PlayerESP.HitEffect  },
        {"RGB CROSS",  &Config.PlayerESP.RGbcro     },
        {"SMALL",      &Config.PlayerESP.cross      },
        {"LESS",       &Config.PlayerESP.LESS       },
        {"HDR",        &Config.PlayerESP.HDR        },
        {"FPS 120",    &Config.PlayerESP.FPS        },
    };

    for (auto& t : memToggles) {
        ToggleSwitch(t.name, t.value);
        ImGui::Spacing();
    }
}

ImGui::Separator();
ImGui::Spacing();

ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 0.7f), "→ Developer : LUCKY HATHUNGO WALA");
ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 0.7f), "→ This SRC Works on GL / KR / TW / VN / BGMI");
ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 0.7f), "→ Telegram : @LUCKY_HUB_DEV");
ImGui::Spacing();

}
} 
}

ImGui::End();
ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

return orig_eglSwapBuffers(dpy, surface);
}


void (*orig_onInputEvent)(void *inputEvent, void *ex_ab, void *ex_ac);
void onInputEvent(void *inputEvent, void *ex_ab, void *ex_ac) {
orig_onInputEvent(inputEvent, ex_ab, ex_ac);if (initImGui) {ImGui_ImplAndroid_HandleInputEvent((AInputEvent*)inputEvent, {(float) screenWidth / (float) glWidth, (float) screenHeight / (float) glHeight});}}
// ========================================================================= //  
#define SLEEP_TIME 1000LL / 60LL
// ========================================================================= //
[[noreturn]] void *maps_thread(void *) {
while (true) {
auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
// ========================================================================= //
auto objs = UObject::GetGlobalObjects(); for (int i = 0; i < objs.Num(); i++) {
auto Object = objs.GetByIndex(i); if (isObjectInvalid(Object)) continue;
if (Config.PlayerESP.Ipad) {
if (Object->IsA(ULocalPlayer::StaticClass())) {
auto playerChar = (ULocalPlayer *) Object;
playerChar->AspectRatioAxisConstraint = EAspectRatioAxisConstraint::AspectRatio_MaintainYFOV; }} else
if (Object->IsA(ULocalPlayer::StaticClass())) {
auto playerChar = (ULocalPlayer *) Object;
playerChar->AspectRatioAxisConstraint = EAspectRatioAxisConstraint::AspectRatio_MaintainXFOV; }
}

if (Config.PlayerESP.HDR) {
//PATCH_LIB("libUE4.so", "0x336AAF8", "05 00 A0 E3 1E FF 2F E1"); // HDR  
//PATCH_LIB("libUE4.so", "0x336ADFC", "78 00 A0 E3 1E FF 2F E1"); // 120 FPS
} 

if (Config.PlayerESP.LESS) {
//PATCH_LIB("libUE4.so", "0x2BCE388", "00 00 00 00");
} 

if (Config.PlayerESP.cross) {
//PATCH_LIB("libUE4.so", "0x2BC9A44", "00 00 A0 E3 1E FF 2F E1");
} 

	 if (Config.PlayerESP.FPS) {
auto objs = UObject::GetGlobalObjects();
for (int i = 0; i < objs.Num(); i++) {
auto Object = objs.GetByIndex(i);
if (isObjectInvalid(Object))
continue;
if (Object->IsA(USTExtraGameInstance::StaticClass())) {
auto playerChar = (USTExtraGameInstance *) Object;
playerChar->UserDetailSetting.PUBGDeviceFPSDef = 120; 
playerChar->UserDetailSetting.PUBGDeviceFPSLow = 120;
playerChar->UserDetailSetting.PUBGDeviceFPSMid = 120;
playerChar->UserDetailSetting.PUBGDeviceFPSHigh = 120;
playerChar->UserDetailSetting.PUBGDeviceFPSHDR = 120;
playerChar->UserDetailSetting.PUBGDeviceFPSUltralHigh = 120;
}
}
}// ========================================================================= //
std::vector<sRegion> tmp;
char line[512];
FILE *f = fopen("/proc/self/maps", "r");
if (f) {
while (fgets(line, sizeof line, f)) {
uintptr_t start, end;
char tmpProt[16];
if (sscanf(line, "%" PRIXPTR "-%" PRIXPTR " %16s %*s %*s %*s %*s", &start, &end, tmpProt) > 0) {
if (tmpProt[0] != 'r') {
tmp.push_back({start, end});}}}
fclose(f);}
trapRegions = tmp;
// ========================================================================= //
auto td = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - t1;
std::this_thread::sleep_for(std::chrono::milliseconds(std::max(std::min(0LL, SLEEP_TIME - td), SLEEP_TIME)));}}
////////////////////HOOK/////////////////
void* _UE4(void*)  {
LOGI("UE4 LIBRARY READY (HOOKS)....");
do { sleep(1);
} while (!isLibraryLoaded("libUE4.so"));
libUE4Base = findLibrary("libUE4.so");
//PATCH_LIB("libUE4.so","0x48a95b0","00 00 A0 E3 1E FF 2F E1");//RPC_ClientCoronaLab
return 0;
}

int (*orig_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
int hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
int result = orig_AInputQueue_getEvent(queue, outEvent);
if (result >= 0 && *outEvent != nullptr && initImGui) {
        ImGui_ImplAndroid_HandleInputEvent(*outEvent, {
            (float)screenWidth / (float)glWidth,
            (float)screenHeight / (float)glHeight
        });
    }

    return result;
}

void *main_thread(void *) {
  sleep(5);
system("adb shell am compat disable BLOCK_UNTRUSTED_TOUCHES com.tencent.ig");
UE4 = Tools::GetBaseAddress("libUE4.so");
while (!UE4) {
UE4 = Tools::GetBaseAddress("libUE4.so");
sleep(1);}
anogs = Tools::GetBaseAddress("libanogs.so");
while (!anogs) {
anogs = Tools::GetBaseAddress("libanogs.so");
sleep(1);}
TDataMaster = Tools::GetBaseAddress("libTDataMaster.so");
while (!TDataMaster) {
TDataMaster = Tools::GetBaseAddress("libTDataMaster.so");
sleep(1);}
while (!g_App){
g_App = *(android_app **) (UE4 + GNativeAndroidApp_Offset);
sleep(1);}
void *input = dlopen_ex(OBFUSCATE("libinput.so"), 4);
while (!input) {input = dlopen_ex(OBFUSCATE("libinput.so"), 4);
sleep(1);}void *address = dlsym_ex(input, OBFUSCATE("_ZN7android11MotionEvent8copyFromEPKS0_b"));HOOK(address, onInputEvent, &orig_onInputEvent);dlclose_ex(input);
FName::GNames = GetGNames();
while (!FName::GNames) {
FName::GNames = GetGNames();
sleep(1); }
UObject::GUObjectArray = (FUObjectArray *) (UE4 + GUObject_Offset);
void *egl = dlopen_ex("libEGL.so", 4);
while (!egl) {
egl = dlopen_ex("libEGL.so", 4);
sleep(1);
}

void *libAndroid = dlopen(OBFUSCATE("libandroid.so"), 4);
if (libAndroid) {
void *symEvent = dlsym(libAndroid, OBFUSCATE("AInputQueue_getEvent"));
DobbyHook(symEvent, (void *)hooked_AInputQueue_getEvent, (void **)&orig_AInputQueue_getEvent);
}

Tools::Hook((void *) (Tools::GetBaseAddress("libUE4.so") + ProcessEvent_Offset),         (void *) hhhkProcessEvent, (void **) &lundProcessEvent); //Proces Event Child

RenderPost();

void *addr = dlsym_ex(egl, "eglSwapBuffers");
HOOK(addr, _eglSwapBuffers, &orig_eglSwapBuffers);
dlclose_ex(egl);
pthread_t DEV_MEROX;
pthread_create(&DEV_MEROX, 0, _UE4,0);
pthread_t t;
pthread_create(&t, 0, maps_thread, 0);
items_data = json::parse(JSON_ITEMS);
return 0;
}

__attribute__((constructor)) 
void _init() {
pthread_t t;
pthread_create(&t, 0, main_thread, 0);
}

