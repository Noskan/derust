cmake_minimum_required(VERSION 3.24)

set(_tm_directx_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/third_party/directx/Include")
set(_tm_directx_lib_dir "${CMAKE_CURRENT_SOURCE_DIR}/third_party/directx/Lib")
file(TO_NATIVE_PATH "${_tm_directx_include_dir}" _tm_directx_include_native)
file(TO_NATIVE_PATH "${_tm_directx_lib_dir}" _tm_directx_lib_native)

set(_tm_directx_props "${CMAKE_CURRENT_BINARY_DIR}/tm_directx.props")
file(WRITE "${_tm_directx_props}" "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
  <PropertyGroup>
    <IncludePath>$(IncludePath);${_tm_directx_include_native}</IncludePath>
    <LibraryPath>$(LibraryPath);${_tm_directx_lib_native}</LibraryPath>
  </PropertyGroup>
</Project>
")

add_library(tm_directx INTERFACE)

target_link_libraries(tm_directx
  INTERFACE
    d3d9
    d3dx9
    dinput8
    dsound
    dxguid
    $<$<CXX_COMPILER_ID:MSVC>:legacy_stdio_definitions>
)

add_library(tm_crypto INTERFACE)

target_include_directories(tm_crypto
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(TMPROJECT_SOURCES
  src/audio/DirShow.cpp
  src/audio/dsutil.cpp
  src/audio/TMVideoWnd.cpp
  src/common/Basedef.cpp
  src/common/ProtocolLayoutCheck.cpp
  src/core/EventTranslator.cpp
  src/core/NewApp.cpp
  src/core/TMGlobal.cpp
  src/core/TMLog.cpp
  src/core/TMProject.cpp
  src/core/TMUtil.cpp
  src/core/TimerManager.cpp
  src/effects/TMEffect.cpp
  src/effects/TMEffectBillBoard.cpp
  src/effects/TMEffectBillBoard2.cpp
  src/effects/TMEffectBillBoard3.cpp
  src/effects/TMEffectBillBoard4.cpp
  src/effects/TMEffectCharge.cpp
  src/effects/TMEffectDust.cpp
  src/effects/TMEffectFirework.cpp
  src/effects/TMEffectGold.cpp
  src/effects/TMEffectLevelUp.cpp
  src/effects/TMEffectMesh.cpp
  src/effects/TMEffectMeshRotate.cpp
  src/effects/TMEffectParticle.cpp
  src/effects/TMEffectSWSwing.cpp
  src/effects/TMEffectSkinMesh.cpp
  src/effects/TMEffectSpark.cpp
  src/effects/TMEffectSpark2.cpp
  src/effects/TMEffectStart.cpp
  src/effects/TMFireEffect.cpp
  src/entities/TMArrow.cpp
  src/entities/TMBike.cpp
  src/entities/TMButterFly.cpp
  src/entities/TMCannon.cpp
  src/entities/TMDrop.cpp
  src/entities/TMDust.cpp
  src/entities/TMFish.cpp
  src/entities/TMFlail.cpp
  src/entities/TMFloat.cpp
  src/entities/TMHuman.cpp
  src/entities/TMHuman_Animation.cpp
  src/entities/TMHuman_Combat.cpp
  src/entities/TMHuman_Effects.cpp
  src/entities/TMHuman_Packets.cpp
  src/entities/TMItem.cpp
  src/entities/TMLeaf.cpp
  src/entities/TMShade.cpp
  src/gameplay/Mission.cpp
  src/gameplay/MrItemMix.cpp
  src/net/CPSock.cpp
  src/render/CFrame.cpp
  src/render/CMesh.cpp
  src/render/D3DDevice.cpp
  src/render/D3DEnumeration.cpp
  src/render/DXUtil.cpp
  src/render/GeomObject.cpp
  src/render/JBlur.cpp
  src/render/MeshManager.cpp
  src/render/RenderDevice.cpp
  src/render/TMCamera.cpp
  src/render/TMFont2.cpp
  src/render/TMFont3.cpp
  src/render/TMLight.cpp
  src/render/TMMesh.cpp
  src/render/TMSkinMesh.cpp
  src/render/TextureManager.cpp
  src/scene/ObjectManager.cpp
  src/scene/TMDemoScene.cpp
  src/scene/TMFieldScene.cpp
  src/scene/TMFieldScene_Combat.cpp
  src/scene/TMFieldScene_Input.cpp
  src/scene/TMFieldScene_Packets.cpp
  src/scene/TMFieldScene_UI.cpp
  src/scene/TMObject.cpp
  src/scene/TMObjectContainer.cpp
  src/scene/TMScene.cpp
  src/scene/TMSelectCharScene.cpp
  src/scene/TMSelectServerScene.cpp
  src/scene/TreeNode.cpp
  src/skills/TMSkillBash.cpp
  src/skills/TMSkillCure.cpp
  src/skills/TMSkillDoubleSwing.cpp
  src/skills/TMSkillExplosion2.cpp
  src/skills/TMSkillFire.cpp
  src/skills/TMSkillFlash.cpp
  src/skills/TMSkillFreezeBlade.cpp
  src/skills/TMSkillHaste.cpp
  src/skills/TMSkillHeal.cpp
  src/skills/TMSkillHeavenDust.cpp
  src/skills/TMSkillHolyTouch.cpp
  src/skills/TMSkillIceSpear.cpp
  src/skills/TMSkillJudgement.cpp
  src/skills/TMSkillLusterFurnish.cpp
  src/skills/TMSkillMagicArrow.cpp
  src/skills/TMSkillMagicShield.cpp
  src/skills/TMSkillMeteorStorm.cpp
  src/skills/TMSkillPoison.cpp
  src/skills/TMSkillSlowSlash.cpp
  src/skills/TMSkillSnow.cpp
  src/skills/TMSkillSpChange.cpp
  src/skills/TMSkillSpeedUp.cpp
  src/skills/TMSkillThunderBolt.cpp
  src/skills/TMSkillTownPortal.cpp
  src/ui/SControl.cpp
  src/ui/SControlContainer.cpp
  src/ui/SGrid.cpp
  src/ui/SGrid_Input.cpp
  src/ui/SGrid_Tooltip.cpp
  src/ui/SGrid_Trade.cpp
  src/world/TMGate.cpp
  src/world/TMGround.cpp
  src/world/TMGround_Render.cpp
  src/world/TMGround_Tables.cpp
  src/world/TMHouse.cpp
  src/world/TMRain.cpp
  src/world/TMSea.cpp
  src/world/TMShip.cpp
  src/world/TMSky.cpp
  src/world/TMSnow.cpp
  src/world/TMSun.cpp
  src/world/TMTree.cpp
  src/resources/TMProject.rc
)

add_executable(TMProject WIN32 ${TMPROJECT_SOURCES})

set_target_properties(TMProject PROPERTIES
  VS_USER_PROPS "${_tm_directx_props}"
)

target_include_directories(TMProject
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(TMProject PRIVATE cxx_std_17)

target_compile_definitions(TMProject
  PRIVATE
    WIN32
    _WINDOWS
    _MBCS
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(TMProject
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive->
)

target_precompile_headers(TMProject
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/pch.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/render/D3DEnumeration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/render/DXUtil.cpp
  PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
)

target_link_libraries(TMProject
  PRIVATE
    tm_directx
    tm_crypto
    wininet
    wsock32
    imm32
    winmm
    iphlpapi
    strmiids
)

target_link_options(TMProject
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/SAFESEH:NO>
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${TMPROJECT_SOURCES})
