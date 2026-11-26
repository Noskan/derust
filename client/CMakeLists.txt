cmake_minimum_required(VERSION 3.24)

set(_tm_directx_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Directx/Include")
set(_tm_directx_lib_dir "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Directx/Lib")
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

add_executable(TMProject WIN32
  Projects/TMProject/Basedef.cpp
  Projects/TMProject/CFrame.cpp
  Projects/TMProject/CMesh.cpp
  Projects/TMProject/CPSock.cpp
  Projects/TMProject/D3DDevice.cpp
  Projects/TMProject/D3DEnumeration.cpp
  Projects/TMProject/DirShow.cpp
  Projects/TMProject/dsutil.cpp
  Projects/TMProject/DXUtil.cpp
  Projects/TMProject/EventTranslator.cpp
  Projects/TMProject/GeomObject.cpp
  Projects/TMProject/JBlur.cpp
  Projects/TMProject/MeshManager.cpp
  Projects/TMProject/Mission.cpp
  Projects/TMProject/MrItemMix.cpp
  Projects/TMProject/NewApp.cpp
  Projects/TMProject/ObjectManager.cpp
  Projects/TMProject/RenderDevice.cpp
  Projects/TMProject/SControl.cpp
  Projects/TMProject/SControlContainer.cpp
  Projects/TMProject/SGrid.cpp
  Projects/TMProject/TextureManager.cpp
  Projects/TMProject/TimerManager.cpp
  Projects/TMProject/TMArrow.cpp
  Projects/TMProject/TMBike.cpp
  Projects/TMProject/TMButterFly.cpp
  Projects/TMProject/TMCamera.cpp
  Projects/TMProject/TMCannon.cpp
  Projects/TMProject/TMDemoScene.cpp
  Projects/TMProject/TMDrop.cpp
  Projects/TMProject/TMDust.cpp
  Projects/TMProject/TMEffect.cpp
  Projects/TMProject/TMEffectBillBoard.cpp
  Projects/TMProject/TMEffectBillBoard2.cpp
  Projects/TMProject/TMEffectBillBoard3.cpp
  Projects/TMProject/TMEffectBillBoard4.cpp
  Projects/TMProject/TMEffectCharge.cpp
  Projects/TMProject/TMEffectDust.cpp
  Projects/TMProject/TMEffectFirework.cpp
  Projects/TMProject/TMEffectGold.cpp
  Projects/TMProject/TMEffectLevelUp.cpp
  Projects/TMProject/TMEffectMesh.cpp
  Projects/TMProject/TMEffectMeshRotate.cpp
  Projects/TMProject/TMEffectParticle.cpp
  Projects/TMProject/TMEffectSkinMesh.cpp
  Projects/TMProject/TMEffectSpark.cpp
  Projects/TMProject/TMEffectSpark2.cpp
  Projects/TMProject/TMEffectStart.cpp
  Projects/TMProject/TMEffectSWSwing.cpp
  Projects/TMProject/TMFieldScene.cpp
  Projects/TMProject/TMFireEffect.cpp
  Projects/TMProject/TMFish.cpp
  Projects/TMProject/TMFlail.cpp
  Projects/TMProject/TMFloat.cpp
  Projects/TMProject/TMFont2.cpp
  Projects/TMProject/TMFont3.cpp
  Projects/TMProject/TMGate.cpp
  Projects/TMProject/TMGlobal.cpp
  Projects/TMProject/TMGround.cpp
  Projects/TMProject/TMHouse.cpp
  Projects/TMProject/TMHuman.cpp
  Projects/TMProject/TMItem.cpp
  Projects/TMProject/TMLeaf.cpp
  Projects/TMProject/TMLight.cpp
  Projects/TMProject/TMLog.cpp
  Projects/TMProject/TMMesh.cpp
  Projects/TMProject/TMObject.cpp
  Projects/TMProject/TMObjectContainer.cpp
  Projects/TMProject/TMProject.cpp
  Projects/TMProject/TMRain.cpp
  Projects/TMProject/TMScene.cpp
  Projects/TMProject/TMSea.cpp
  Projects/TMProject/TMSelectCharScene.cpp
  Projects/TMProject/TMSelectServerScene.cpp
  Projects/TMProject/TMShade.cpp
  Projects/TMProject/TMShip.cpp
  Projects/TMProject/TMSkillBash.cpp
  Projects/TMProject/TMSkillCure.cpp
  Projects/TMProject/TMSkillDoubleSwing.cpp
  Projects/TMProject/TMSkillExplosion2.cpp
  Projects/TMProject/TMSkillFire.cpp
  Projects/TMProject/TMSkillFlash.cpp
  Projects/TMProject/TMSkillFreezeBlade.cpp
  Projects/TMProject/TMSkillHaste.cpp
  Projects/TMProject/TMSkillHeal.cpp
  Projects/TMProject/TMSkillHeavenDust.cpp
  Projects/TMProject/TMSkillHolyTouch.cpp
  Projects/TMProject/TMSkillIceSpear.cpp
  Projects/TMProject/TMSkillJudgement.cpp
  Projects/TMProject/TMSkillLusterFurnish.cpp
  Projects/TMProject/TMSkillMagicArrow.cpp
  Projects/TMProject/TMSkillMagicShield.cpp
  Projects/TMProject/TMSkillMeteorStorm.cpp
  Projects/TMProject/TMSkillPoison.cpp
  Projects/TMProject/TMSkillSlowSlash.cpp
  Projects/TMProject/TMSkillSnow.cpp
  Projects/TMProject/TMSkillSpChange.cpp
  Projects/TMProject/TMSkillSpeedUp.cpp
  Projects/TMProject/TMSkillThunderBolt.cpp
  Projects/TMProject/TMSkillTownPortal.cpp
  Projects/TMProject/TMSkinMesh.cpp
  Projects/TMProject/TMSky.cpp
  Projects/TMProject/TMSnow.cpp
  Projects/TMProject/TMSun.cpp
  Projects/TMProject/TMTree.cpp
  Projects/TMProject/TMUtil.cpp
  Projects/TMProject/TMVideoWnd.cpp
  Projects/TMProject/TreeNode.cpp
  Projects/TMProject/TMProject.rc
)

set_target_properties(TMProject PROPERTIES
  VS_USER_PROPS "${_tm_directx_props}"
)

target_include_directories(TMProject
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Projects/TMProject
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
    ${CMAKE_CURRENT_SOURCE_DIR}/Projects/TMProject/pch.h
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/Projects/TMProject/D3DEnumeration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Projects/TMProject/DXUtil.cpp
  PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
)

target_link_libraries(TMProject
  PRIVATE
    tm_directx
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
