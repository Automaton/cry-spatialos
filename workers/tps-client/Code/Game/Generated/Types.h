#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/spawner.h>
#include <automaton/player/player.h>
#include <automaton/player/score.h>
#include <automaton/player/bullet.h>
#include <automaton/tree.h>
#include <automaton/quaternion.h>
#include <improbable/vector3.h>
#include <improbable/standard_library.h>
#include <automaton/player/movement.h>

namespace automaton { 
static void ReflectType(Schematyc::CTypeDesc<automaton::SpawnPlayerRequest>& desc)
{
	desc.SetGUID("{0482FB80-472B-4BC3-97C5-E87635F0F40E}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("SpawnPlayerRequest");
}
}
namespace automaton { 
static void ReflectType(Schematyc::CTypeDesc<automaton::SpawnPlayerResponse>& desc)
{
	desc.SetGUID("{B52B9C77-8102-46F0-BF3B-C769F067C9F1}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("SpawnPlayerResponse");
}
}
namespace automaton { namespace player { 
static void ReflectType(Schematyc::CTypeDesc<automaton::player::Heartbeat>& desc)
{
	desc.SetGUID("{B5F2063C-62B7-4DE1-B5DD-6E0966C22589}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("Heartbeat");
}
}}
namespace automaton { namespace player { 
static void ReflectType(Schematyc::CTypeDesc<automaton::player::KilledPlayer>& desc)
{
	desc.SetGUID("{13C4CA15-6192-4AC4-98F0-F36CA55A482E}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("KilledPlayer");
}
}}



namespace automaton { 
static void ReflectType(Schematyc::CTypeDesc<automaton::Quaternion>& desc)
{
	desc.SetGUID("{BD103A6A-5622-4F1F-B190-780D1DC52EAC}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("Quaternion");
}
}
namespace improbable { 
static void ReflectType(Schematyc::CTypeDesc<improbable::Vector3f>& desc)
{
	desc.SetGUID("{23613545-1144-4848-BD90-C4CC51ACF230}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("Vector3f");
}
}
namespace improbable { 
static void ReflectType(Schematyc::CTypeDesc<improbable::Vector3d>& desc)
{
	desc.SetGUID("{27FA9FD8-8DAC-46A2-8913-3E9D995697EA}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("Vector3d");
}
}
namespace improbable { 
static void ReflectType(Schematyc::CTypeDesc<improbable::WorkerAttributeSet>& desc)
{
	desc.SetGUID("{DD185344-224D-4A69-9EB1-31FF8CF55F90}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("WorkerAttributeSet");
}
}
namespace improbable { 
static void ReflectType(Schematyc::CTypeDesc<improbable::WorkerRequirementSet>& desc)
{
	desc.SetGUID("{C36CC576-DEB2-48D2-978A-57CC34DA8303}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("WorkerRequirementSet");
}
}
namespace improbable { 
static void ReflectType(Schematyc::CTypeDesc<improbable::Coordinates>& desc)
{
	desc.SetGUID("{0294E8D0-5E41-4203-9973-85E472684FF7}"_cry_guid);
	desc.SetEditorCategory("SpatialOS");
	desc.SetLabel("Coordinates");
}
}


namespace {
    class SpatialOsTypes {
    public:
        static void RegisterSpatialOs(Schematyc::IEnvRegistrar& registrar)
        {
            Schematyc::CEnvRegistrationScope baseScope = registrar.RootScope().Register(SCHEMATYC_MAKE_ENV_MODULE("{18331838-0FA2-4906-95A3-439787C01B27}"_cry_guid, "SpatialOS"));
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(automaton::SpawnPlayerRequest));
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<const ::improbable::Coordinates& (automaton::SpawnPlayerRequest::*)() const>(&automaton::SpawnPlayerRequest::position, "{388C414A-2D89-4C7A-8E58-7D68510E9674}"_cry_guid, "GetPosition", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "Position");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&automaton::SpawnPlayerRequest::set_position, "{F3E35991-618B-484B-B2F7-0FC94C0BE1A8}"_cry_guid, "SetPosition");
                    pFunction->BindInput(1, 1, "Position", nullptr, improbable::Coordinates::Create());
                    scope.Register(pFunction);
                }
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(automaton::SpawnPlayerResponse));
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<bool (automaton::SpawnPlayerResponse::*)() const>(&automaton::SpawnPlayerResponse::success, "{AAA7555A-B258-42C3-92B4-8EECBF35D83F}"_cry_guid, "GetSuccess", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "Success");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&automaton::SpawnPlayerResponse::set_success, "{404B4F53-34C9-49F5-99E0-D78D7F3AF0A2}"_cry_guid, "SetSuccess");
                    pFunction->BindInput(1, 1, "Success", nullptr, false);
                    scope.Register(pFunction);
                }
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(automaton::player::Heartbeat));
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(automaton::player::KilledPlayer));
            }



            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(automaton::Quaternion));
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (automaton::Quaternion::*)() const>(&automaton::Quaternion::w, "{F89A4A6A-42A2-4333-AEED-7190C0B7BB0A}"_cry_guid, "GetW", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "W");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&automaton::Quaternion::set_w, "{34ADF626-57DB-44A2-8935-B8CD50D69F12}"_cry_guid, "SetW");
                    pFunction->BindInput(1, 1, "W", nullptr, 0);
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (automaton::Quaternion::*)() const>(&automaton::Quaternion::x, "{25094378-4B6B-413B-9F42-2AE640667EBF}"_cry_guid, "GetX", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "X");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&automaton::Quaternion::set_x, "{72262E91-30B0-4FCE-A21C-7269C0A74897}"_cry_guid, "SetX");
                    pFunction->BindInput(1, 1, "X", nullptr, 0);
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (automaton::Quaternion::*)() const>(&automaton::Quaternion::y, "{67578901-874C-4C4B-9CE4-5503628BDC73}"_cry_guid, "GetY", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "Y");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&automaton::Quaternion::set_y, "{1C7200A6-60BD-475A-A5DD-B4031518D558}"_cry_guid, "SetY");
                    pFunction->BindInput(1, 1, "Y", nullptr, 0);
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (automaton::Quaternion::*)() const>(&automaton::Quaternion::z, "{8834ECB2-41EE-4CEF-8D3A-F80A3A9E31F8}"_cry_guid, "GetZ", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "Z");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&automaton::Quaternion::set_z, "{E80D32FB-50F0-4B3C-B30A-74B366805B2C}"_cry_guid, "SetZ");
                    pFunction->BindInput(1, 1, "Z", nullptr, 0);
                    scope.Register(pFunction);
                }
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(improbable::Vector3f));
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (improbable::Vector3f::*)() const>(&improbable::Vector3f::x, "{9B5BC511-9037-4986-8603-C852C656B170}"_cry_guid, "GetX", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "X");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&improbable::Vector3f::set_x, "{38B0C0B9-256E-4AF3-A4CF-BBA91E997E58}"_cry_guid, "SetX");
                    pFunction->BindInput(1, 1, "X", nullptr, 0);
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (improbable::Vector3f::*)() const>(&improbable::Vector3f::y, "{F2195D1E-D9E7-4D60-B617-ED9DADBEB01E}"_cry_guid, "GetY", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "Y");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&improbable::Vector3f::set_y, "{61713D2C-708C-4105-A5FB-9E6CD54BD6E8}"_cry_guid, "SetY");
                    pFunction->BindInput(1, 1, "Y", nullptr, 0);
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = Schematyc::EnvFunction::MakeShared<float (improbable::Vector3f::*)() const>(&improbable::Vector3f::z, "{11A2DA3E-FA47-4F26-8D4A-375C66D74732}"_cry_guid, "GetZ", SCHEMATYC_SOURCE_FILE_INFO);
                    pFunction->BindOutput(0, 0, "Z");
                    scope.Register(pFunction);
                }
                {
                    auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&improbable::Vector3f::set_z, "{94621F79-0778-4262-B928-B5B8DD4A66B2}"_cry_guid, "SetZ");
                    pFunction->BindInput(1, 1, "Z", nullptr, 0);
                    scope.Register(pFunction);
                }
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(improbable::Vector3d));
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(improbable::WorkerAttributeSet));
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(improbable::WorkerRequirementSet));
            }
            {
                Schematyc::CEnvRegistrationScope scope = baseScope.Register(SCHEMATYC_MAKE_ENV_DATA_TYPE(improbable::Coordinates));
            }

        }
    };
}