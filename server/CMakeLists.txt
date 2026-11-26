cmake_minimum_required(VERSION 3.24)

add_library(w2pp_common STATIC
  Code/Basedef.cpp
  Code/CPSock.cpp
  Code/Basedef.h
  Code/CPSock.h
  Code/ItemEffect.h
)

target_include_directories(w2pp_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Code
)

target_compile_features(w2pp_common PUBLIC cxx_std_17)

target_compile_definitions(w2pp_common
  PUBLIC
    WIN32
    _WINDOWS
    _MBCS
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG;_PACKET_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(w2pp_common
  PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /source-charset:utf-8>
)

target_link_libraries(w2pp_common
  PUBLIC
    ws2_32
    winmm
)

add_executable(DBSrv WIN32
  Code/DBSrv/CFileDB.cpp
  Code/DBSrv/CRanking.cpp
  Code/DBSrv/CReadFiles.cpp
  Code/DBSrv/CUser.cpp
  Code/DBSrv/Server.cpp
  Code/DBSrv/DBSrv.rc
)

target_include_directories(DBSrv
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Code/DBSrv
)

target_link_libraries(DBSrv PRIVATE w2pp_common)

add_executable(TMSrv WIN32
  Code/TMSrv/CCastleZakum.cpp
  Code/TMSrv/CItem.cpp
  Code/TMSrv/CMob.cpp
  Code/TMSrv/CNPCGene.cpp
  Code/TMSrv/CReadFiles.cpp
  Code/TMSrv/CUser.cpp
  Code/TMSrv/CWarTower.cpp
  Code/TMSrv/GetFunc.cpp
  Code/TMSrv/imple.cpp
  Code/TMSrv/MobKilled.cpp
  Code/TMSrv/_MSG_AcceptParty.cpp
  Code/TMSrv/_MSG_AccountLogin.cpp
  Code/TMSrv/_MSG_AccountSecure.cpp
  Code/TMSrv/_MSG_Action.cpp
  Code/TMSrv/_MSG_ApplyBonus.cpp
  Code/TMSrv/_MSG_Attack.cpp
  Code/TMSrv/_MSG_Buy.cpp
  Code/TMSrv/_MSG_CapsuleInfo.cpp
  Code/TMSrv/_MSG_Challange.cpp
  Code/TMSrv/_MSG_ChallangeConfirm.cpp
  Code/TMSrv/_MSG_ChangeCity.cpp
  Code/TMSrv/_MSG_CharacterLogin.cpp
  Code/TMSrv/_MSG_CharacterLogout.cpp
  Code/TMSrv/_MSG_CombineItem.cpp
  Code/TMSrv/_MSG_CombineItemAgatha.cpp
  Code/TMSrv/_MSG_CombineItemAilyn.cpp
  Code/TMSrv/_MSG_CombineItemAlquimia.cpp
  Code/TMSrv/_MSG_CombineItemEhre.cpp
  Code/TMSrv/_MSG_CombineItemExtracao.cpp
  Code/TMSrv/_MSG_CombineItemLindy.cpp
  Code/TMSrv/_MSG_CombineItemOdin.cpp
  Code/TMSrv/_MSG_CombineItemShany.cpp
  Code/TMSrv/_MSG_CombineItemTiny.cpp
  Code/TMSrv/_MSG_CreateCharacter.cpp
  Code/TMSrv/_MSG_DeleteCharacter.cpp
  Code/TMSrv/_MSG_DeleteItem.cpp
  Code/TMSrv/_MSG_Deposit.cpp
  Code/TMSrv/_MSG_Deprivate.cpp
  Code/TMSrv/_MSG_DropItem.cpp
  Code/TMSrv/_MSG_GetItem.cpp
  Code/TMSrv/_MSG_GuildAlly.cpp
  Code/TMSrv/_MSG_InviteGuild.cpp
  Code/TMSrv/_MSG_MessageChat.cpp
  Code/TMSrv/_MSG_MessageWhisper.cpp
  Code/TMSrv/_MSG_Motion.cpp
  Code/TMSrv/_MSG_NoViewMob.cpp
  Code/TMSrv/_MSG_PKMode.cpp
  Code/TMSrv/_MSG_PutoutSeal.cpp
  Code/TMSrv/_MSG_Quest.cpp
  Code/TMSrv/_MSG_QuitTrade.cpp
  Code/TMSrv/_MSG_RemoveParty.cpp
  Code/TMSrv/_MSG_ReqBuy.cpp
  Code/TMSrv/_MSG_ReqRanking.cpp
  Code/TMSrv/_MSG_REQShopList.cpp
  Code/TMSrv/_MSG_ReqTeleport.cpp
  Code/TMSrv/_MSG_ReqTradeList.cpp
  Code/TMSrv/_MSG_Restart.cpp
  Code/TMSrv/_MSG_Sell.cpp
  Code/TMSrv/_MSG_SendAutoTrade.cpp
  Code/TMSrv/_MSG_SendReqParty.cpp
  Code/TMSrv/_MSG_SetShortSkill.cpp
  Code/TMSrv/_MSG_SplitItem.cpp
  Code/TMSrv/_MSG_Trade.cpp
  Code/TMSrv/_MSG_TradingItem.cpp
  Code/TMSrv/_MSG_UpdateItem.cpp
  Code/TMSrv/_MSG_UseItem.cpp
  Code/TMSrv/_MSG_War.cpp
  Code/TMSrv/_MSG_Withdraw.cpp
  Code/TMSrv/ProcessClientMessage.cpp
  Code/TMSrv/ProcessDBMessage.cpp
  Code/TMSrv/ProcessSecMinTimer.cpp
  Code/TMSrv/SendFunc.cpp
  Code/TMSrv/Server.cpp
  Code/TMSrv/TMSrv.rc
)

target_include_directories(TMSrv
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Code/TMSrv
)

target_link_libraries(TMSrv PRIVATE w2pp_common)

if(W2PP_BUILD_CLIENT_PATCH)
  add_library(ClientPatch_v7662 SHARED
    Code/ClientPatch_v7662/main.cpp
  )
  target_compile_definitions(ClientPatch_v7662
    PRIVATE
      WIN32
      _WINDOWS
      _MBCS
      $<$<CONFIG:Debug>:_DEBUG>
      $<$<CONFIG:Release>:NDEBUG>
  )
  target_compile_options(ClientPatch_v7662
    PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W3>
  )
endif()
