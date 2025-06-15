/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "server/ranch/RanchDirector.hpp"
#include  "server/ServerInstance.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "libserver/util/Util.hpp"

#include <ranges>

#include <spdlog/spdlog.h>

namespace alicia
{

RanchDirector::RanchDirector(soa::ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
  // Handlers

  // EnterRanch handler
  _commandServer.RegisterCommandHandler<RanchCommandEnterRanch>(
    CommandId::RanchEnterRanch,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRanch(clientId, message);
    });

  // Snapshot handler
  _commandServer.RegisterCommandHandler<RanchCommandRanchSnapshot>(
    CommandId::RanchSnapshot,
    [this](ClientId clientId, const auto& message)
    {
      HandleSnapshot(clientId, message);
    });

  // RanchCmdAction handler
  _commandServer.RegisterCommandHandler<RanchCommandRanchCmdAction>(
    CommandId::RanchCmdAction,
    [this](ClientId clientId, const auto& message)
    {
      HandleCmdAction(clientId, message);
    });

  // RanchStuff handler
  _commandServer.RegisterCommandHandler<RanchCommandRanchStuff>(
    CommandId::RanchStuff,
    [this](ClientId clientId, const auto& message)
    {
      HandleRanchStuff(clientId, message);
    });

  _commandServer.RegisterCommandHandler<RanchCommandUpdateBusyState>(
    CommandId::RanchUpdateBusyState,
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateBusyState(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandSearchStallion>(
    CommandId::RanchSearchStallion,
    [this](ClientId clientId, auto& command)
    {
      HandleSearchStallion(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandEnterBreedingMarket>(
    CommandId::RanchEnterBreedingMarket,
    [this](ClientId clientId, auto& command)
    {
      HandleEnterBreedingMarket(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandTryBreeding>(
    CommandId::RanchTryBreeding,
    [this](ClientId clientId, auto& command)
    {
      HandleTryBreeding(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandBreedingWishlist>(
    CommandId::RanchBreedingWishlist,
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingWishlist(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandUpdateMountNickname>(
    CommandId::RanchUpdateMountNickname,
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateMountNickname(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandRequestStorage>(
    CommandId::RanchRequestStorage,
    [this](ClientId clientId, auto& command)
    {
      HandleRequestStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandGetItemFromStorage>(
    CommandId::RanchGetItemFromStorage,
    [this](ClientId clientId, auto& command)
    {
      HandleGetItemFromStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandChat>(
    CommandId::RanchChat,
    [this](ClientId clientId, auto& command)
    {
      HandleChat(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandWearEquipment>(
    CommandId::RanchWearEquipment,
    [this](ClientId clientId, auto& command)
    {
      HandleWearEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandRemoveEquipment>(
    CommandId::RanchRemoveEquipment,
    [this](ClientId clientId, auto& command)
    {
      HandleRemoveEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<RanchCommandUseItem>(
    CommandId::RanchUseItem,
    [this](ClientId clientId, auto& command)
    {
      HandleUseItem(clientId, command);
    });
}

void RanchDirector::Initialize()
{
  spdlog::debug(
    "Ranch server listening on {}:{}",
    GetSettings().address.to_string(),
    GetSettings().port);

  _commandServer.RegisterCommandHandler<RanchCommandRequestNpcDressList>(
    CommandId::RanchRequestNpcDressList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestNpcDressList(clientId, message);
    });

  // Host the server.
  _commandServer.BeginHost(GetSettings().address, GetSettings().port);
}

void RanchDirector::Terminate()
{
  _commandServer.EndHost();
}

void RanchDirector::Tick()
{
}

void RanchDirector::BroadcastSetIntroductionNotify(
  uint32_t characterUid,
  const std::string& introduction)
{
  // Find the ranch the character is on.
  auto ranchUid = soa::data::InvalidUid;
  ClientId characterClientId = 0;
  for (const auto& [clientId, clientContext] : _clientContext)
  {
    if (clientContext.characterUid == characterUid)
    {
      ranchUid = clientContext.ranchUid;
      characterClientId = clientId;
      break;
    }
  }

  RanchCommandSetIntroductionNotify notify{
    .characterUid = characterUid,
    .introduction = introduction};

  for (const ClientId& ranchClientId : _ranches[ranchUid]._clients)
  {
    // Prevent broadcast to self.
    if (ranchClientId == characterClientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      CommandId::RanchSetIntroductionNotify,
      [notify]()
      {
        return notify;
      });
  }
}

soa::ServerInstance& RanchDirector::GetServerInstance()
{
  return _serverInstance;
}

soa::Settings::RanchSettings& RanchDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._ranchSettings;
}

void RanchDirector::HandleEnterRanch(
  ClientId clientId,
  const RanchCommandEnterRanch& enterRanch)
{
  // todo verify the OTP against the character UID

  auto& clientContext = _clientContext[clientId];

  clientContext.characterUid = enterRanch.characterUid;
  clientContext.ranchUid = enterRanch.ranchUid;

  RanchCommandEnterRanchOK response{
    .ranchId = enterRanch.ranchUid,
    .unk0 = "unk0",
    .unk11 = {
      .unk0 = 1,
      .unk1 = 1}};

  // Get the ranch the user is connecting to.
  const auto ranchRecord = GetServerInstance().GetDataDirector().GetRanches().Get(
    enterRanch.ranchUid);
  if (not ranchRecord)
  {
    throw std::runtime_error(
      std::format("Ranch [{}] not available",
        enterRanch.characterUid));
  }

  ranchRecord->Immutable([&response](const soa::data::Ranch& ranch)
  {
    response.ranchName = ranch.name();
  });

  auto& ranchInstance = _ranches[enterRanch.ranchUid];

  // Add the character to the ranch.
  ranchInstance._worldTracker.AddCharacter(
    enterRanch.characterUid);

  RanchCharacter enteringRanchPlayer;

  // Add the ranch horses.
  for (auto [horseUid, horseEntityId] : ranchInstance._worldTracker.GetHorseEntities())
  {
    auto& ranchHorse = response.horses.emplace_back();
    ranchHorse.ranchIndex = horseEntityId;

    auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(horseUid);
    if (not horseRecord)
    {
      throw std::runtime_error(
        std::format("Horse [{}] not available", horseUid));
    }

    horseRecord->Immutable([&ranchHorse](const soa::data::Horse& horse)
    {
      protocol::BuildProtocolHorse(ranchHorse.horse, horse);
    });
  }

  // Add the ranch characters.
  for (auto [characterUid, characterEntityId] : ranchInstance._worldTracker.GetCharacterEntities())
  {
    auto& ranchCharacter = response.characters.emplace_back();
    ranchCharacter.ranchIndex = characterEntityId;
    ranchCharacter.playerRelatedThing = {
      .val1 = 1};

    auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(characterUid);
    if (not characterRecord)
    {
      throw std::runtime_error(
        std::format("Character [{}] not available", characterUid));
    }

    characterRecord->Immutable([this, &ranchCharacter](const soa::data::Character& character)
    {
      ranchCharacter.uid = character.uid();
      ranchCharacter.name = character.name();
      ranchCharacter.gender = Gender::Unspecified;
      ranchCharacter.introduction = character.introduction();

      ranchCharacter.unk0 = 1;
      ranchCharacter.unk1 = 1;

      protocol::BuildProtocolCharacter(ranchCharacter.character, character);

      // Character's equipment.
      const auto equipment = GetServerInstance().GetDataDirector().GetItems().Get(
        character.characterEquipment());
      if (not equipment)
      {
        throw std::runtime_error(
          std::format(
            "Character's [{}] equipment is not available",
            character.uid()));
      }

      protocol::BuildProtocolItems(ranchCharacter.characterEquipment, *equipment);

      // Character's mount.
      const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
        character.mountUid());
      if (not mountRecord)
      {
        throw std::runtime_error(
          std::format(
            "Character's [{}] mount [{}] is not available",
            character.uid(),
            character.mountUid()));
      }

      mountRecord->Immutable([&ranchCharacter](const soa::data::Horse& horse)
      {
        protocol::BuildProtocolHorse(ranchCharacter.mount, horse);
        ranchCharacter.anotherPlayerRelatedThing = {
          .mountUid = horse.uid(),
          .val1 = 0x12};
      });
      spdlog::info("aa");
    });

    if (enterRanch.characterUid == characterUid)
    {
      enteringRanchPlayer = ranchCharacter;
    }
  }

  spdlog::debug("{} is entering ranch with:", enterRanch.characterUid);
  for (const auto& ranchCharacter : response.characters)
  {
    spdlog::debug(
      "Character '{}' ({}), index {}",
      ranchCharacter.name,
      ranchCharacter.uid,
      ranchCharacter.ranchIndex);
  }

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchEnterRanchOK,
    [response]()
    {
      return response;
    });

  // Notify to all other players of the entering player.
  const RanchCommandEnterRanchNotify ranchJoinNotification {
    .character = enteringRanchPlayer
  };

  // Iterate over all the clients connected
  // to the ranch and broadcast join notification.
  for (ClientId ranchClient : ranchInstance._clients)
  {
    spdlog::debug(
      "Sending notification to {}, player {} ('{}') index {} is entering the ranch.",
      _clientContext[ranchClient].characterUid,
      ranchJoinNotification.character.name,
      ranchJoinNotification.character.uid,
      ranchJoinNotification.character.ranchIndex);

    _commandServer.QueueCommand<decltype(ranchJoinNotification)>(
      ranchClient,
      CommandId::RanchEnterRanchNotify,
      [ranchJoinNotification](){
        return ranchJoinNotification;
      });
  }

  ranchInstance._clients.emplace(clientId);
}

void RanchDirector::HandleSnapshot(
  ClientId clientId,
  const RanchCommandRanchSnapshot& snapshot)
{
  const auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  RanchCommandRanchSnapshotNotify response {
    .ranchIndex = ranchInstance._worldTracker.GetCharacterEntityId(
      clientContext.characterUid),
    .type = snapshot.type,
  };

  switch (snapshot.type)
  {
    case RanchCommandRanchSnapshot::Full:
    {
      response.full = snapshot.full;
      break;
    }
    case RanchCommandRanchSnapshot::Partial:
    {
      response.partial = snapshot.partial;
      break;
    }
  }

  for (const auto& ranchClient : ranchInstance._clients)
  {
    // Do not broadcast to the client that sent the snapshot.
    if (ranchClient == clientId)
      continue;

    _commandServer.QueueCommand<decltype(response)>(
      ranchClient,
      CommandId::RanchSnapshotNotify,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleCmdAction(ClientId clientId, const RanchCommandRanchCmdAction& action)
{
  RanchCommandRanchCmdActionNotify response{
    .unk0 = 2,
    .unk1 = 3,
    .unk2 = 1,};

  // TODO: Actual implementation of it
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchCmdActionNotify,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRanchStuff(ClientId clientId, const RanchCommandRanchStuff& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  RanchCommandRanchStuffOK response{
    command.eventId,
    command.value};

  // Todo: needs validation
  characterRecord->Mutable([&command, &response](soa::data::Character& character)
  {
    character.carrots() += command.value;
    response.totalMoney = character.carrots();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchStuffOK,
    [response]
    {
      return response;
    });
}

void RanchDirector::HandleUpdateBusyState(
  ClientId clientId,
  const RanchCommandUpdateBusyState& command)
{
  auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  RanchCommandUpdateBusyStateNotify response {
    .characterId = clientContext.characterUid,
    .busyState = command.busyState};

  clientContext.busyState = command.busyState;

  for (auto ranchClientId : ranchInstance._clients)
  {
    _commandServer.QueueCommand<decltype(response)>(
      ranchClientId,
      CommandId::RanchSnapshotNotify,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const RanchCommandSearchStallion& command)
{
  RanchCommandSearchStallionOK response{
    .unk0 = 0,
    .unk1 = 0};

  const auto horseUids = GetServerInstance().GetDataDirector().GetHorses().GetKeys();
  for (const auto& horseUid : horseUids)
  {
    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
      horseUid);

    auto& stallion = response.stallions.emplace_back();
    horseRecord->Immutable([&stallion](const soa::data::Horse& horse)
    {
      stallion.unk0 = "unk";
      stallion.uid = horse.uid();
      stallion.tid = horse.tid();

      stallion.chance = 0xFF;
      stallion.unk7 = 0xFF;
      stallion.unk8 = 0xFF;

      stallion.name = horse.name();
      stallion.grade = horse.grade();
      stallion.unk11 = 0xFF;

      protocol::BuildProtocolHorseStats(stallion.stats, horse.stats);
      protocol::BuildProtocolHorseParts(stallion.parts, horse.parts);
      protocol::BuildProtocolHorseAppearance(stallion.appearance, horse.appearance);
    });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchSearchStallionOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleEnterBreedingMarket(ClientId clientId, const RanchCommandEnterBreedingMarket& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  RanchCommandEnterBreedingMarketOK response;

  characterRecord->Immutable([this, &response](const soa::data::Character& character)
  {
    const auto horseRecords = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.horses());

    for (const auto& horseRecord : *horseRecords)
    {
      auto& protocolHorse = response.availableHorses.emplace_back();
      horseRecord.Immutable([&protocolHorse](const soa::data::Horse& horse)
      {
        protocolHorse.uid = horse.uid();
        protocolHorse.tid = horse.tid();

        // todo figure out the rest
      });
    }
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchEnterBreedingMarketOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleTryBreeding(
  ClientId clientId,
  const RanchCommandTryBreeding& command)
{
  RanchCommandTryBreedingOK response{
    .uid = command.unk0, // wild guess
    .tid = command.unk1, // lmao
    .val = 0,
    .count = 0,
    .unk0 = 0,
    .parts = {
      .skinId = 1,
      .maneId = 4,
      .tailId = 4,
      .faceId = 5},
    .appearance = {.scale = 4, .legLength = 4, .legVolume = 5, .bodyLength = 3, .bodyVolume = 4},
    .stats = {.agility = 9, .control = 9, .speed = 9, .strength = 9, .spirit = 9},
    .unk1 = 0,
    .unk2 = 0,
    .unk3 = 0,
    .unk4 = 0,
    .unk5 = 0,
    .unk6 = 0,
    .unk7 = 0,
    .unk8 = 0,
    .unk9 = 0,
    .unk10 = 0,
  };

  // TODO: Actually do something
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchTryBreedingOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleBreedingWishlist(
  ClientId clientId,
  const RanchCommandBreedingWishlist& command)
{
  RanchCommandBreedingWishlistOK response{};

  // TODO: Actually do something
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchBreedingWishlistOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUpdateMountNickname(
  ClientId clientId,
  const RanchCommandUpdateMountNickname& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  bool canRenameHorse = false;
  characterRecord->Mutable([this, &canRenameHorse, horseUid = command.horseUid](soa::data::Character& character)
  {
    const bool ownsHorse = std::ranges::contains(character.horses(), horseUid);
    if (not ownsHorse)
      return;

    constexpr soa::data::Tid HorseRenameItem = 45003;
    const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
      character.inventory());

    // Find the horse rename item.
    auto horseRenameItemUid = soa::data::InvalidUid;
    for (const auto& itemRecord : *itemRecords)
    {
      itemRecord.Immutable([&horseRenameItemUid](const soa::data::Item& item)
      {
        if (item.tid() == HorseRenameItem)
        {
          horseRenameItemUid = item.uid();
        }
      });

      // Break early if the item was found.
      if (horseRenameItemUid != soa::data::InvalidUid)
        break;
    }

    // Find the item in the inventory.
    const auto itemInventoryIter = std::ranges::find(
      character.inventory(), horseRenameItemUid);

    // Remove the item from the inventory.
    character.inventory().erase(itemInventoryIter);
    canRenameHorse = true;
  });

  if (not canRenameHorse)
  {
    RanchCommandUpdateMountNicknameCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      CommandId::RanchUpdateMountNicknameCancel,
      [response]()
      {
        return response;
      });
    return;
  }

  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
    command.horseUid);

  horseRecord->Mutable([horseName = command.name](soa::data::Horse& horse)
  {
    horse.name() = horseName;
  });

  RanchCommandUpdateMountNicknameOK response{
    .horseUid = command.horseUid,
    .nickname = command.name,
    .unk1 = command.unk1,
    .unk2 = 0};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchUpdateMountNicknameOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestStorage(
  ClientId clientId,
  const RanchCommandRequestStorage& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  static uint32_t idx = 0;
  RanchCommandRequestStorageOK response{
    .category = command.category,
    .page = 1,
    .pageCountAndNotification = 0};

  const bool showPurchases = command.category == RanchCommandRequestStorage::Category::Purchases;

  // Fill the stored items, either from the purchase category or the gift category.

  characterRecord->Immutable([this, showPurchases, &response](const soa::data::Character& character)
  {
    const auto storedItemRecords = GetServerInstance().GetDataDirector().GetStoredItems().Get(
      showPurchases ? character.purchases() : character.gifts());

    // todo pagination of stored item records with std::ranges::chunks
    protocol::BuildProtocolStoredItems(response.storedItems, *storedItemRecords);
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchRequestStorageOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleGetItemFromStorage(
  ClientId clientId,
  const RanchCommandGetItemFromStorage& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  bool storedItemIsValid = true;

  // Try to remove the stored item from the character.
  characterRecord->Mutable([this, &storedItemIsValid, storedItemUid = command.storedItemUid](
    soa::data::Character& character)
    {
      // The stored item is either a gift or a purchase.
      // Try to remove from both gift and purchase vectors.

      const auto storedGiftIter = std::ranges::find(character.gifts(), storedItemUid);
      if (storedGiftIter != character.gifts().cend())
      {
        character.gifts().erase(storedGiftIter);
        return;
      }

      const auto storedPurchaseIter = std::ranges::find(character.purchases(), storedItemUid);
      if (storedGiftIter != character.purchases().cend())
      {
        character.purchases().erase(storedGiftIter);
        return;
      }

      storedItemIsValid = false;
    });

  // If the stored item is invalid cancel the takeout.
  if (not storedItemIsValid)
  {
    RanchCommandGetItemFromStorageCancel response{
      .storedItemUid = command.storedItemUid,
      .status = 0};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      CommandId::RanchGetItemFromStorageCancel,
      [response]()
      {
        return response;
      });
    return;
  }

  RanchCommandGetItemFromStorageOK response{
    .storedItemUid = command.storedItemUid,
    .member0 = 0};

  // Get the items assigned to the stored item and fill the protocol command.
  characterRecord->Mutable([this, &response](
    soa::data::Character& character)
    {
      const auto storedItemRecord = GetServerInstance().GetDataDirector().GetStoredItems().Get(
        response.storedItemUid);

      // Collection of the items received from the stored item.
      std::vector<soa::data::Uid> items;

      storedItemRecord->Immutable([this, &items, &response](const soa::data::StoredItem& storedItem)
      {
        items = storedItem.items();
        const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
          items);

        protocol::BuildProtocolItems(response.items, *itemRecords);
      });

      // Add the items to the character's inventory.
      character.inventory().insert(
        character.inventory().end(),
        items.begin(),
        items.end());
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchGetItemFromStorageOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestNpcDressList(ClientId clientId, const RanchCommandRequestNpcDressList& requestNpcDressList)
{
  RanchCommandRequestNpcDressListOK response{
    .unk0 = requestNpcDressList.unk0,
    .dressList = {} // TODO: Fetch dress list from somewhere
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchRequestNpcDressListOK,
    [response]()
    {
      return response;
    });;
}

void RanchDirector::HandleChat(
  ClientId clientId,
  const RanchCommandChat& chat)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  auto& ranchInstance = _ranches[clientContext.ranchUid];

  if (chat.message.starts_with("//"))
  {
    RanchCommandChatNotify response{
      .author = "system",
      .isBlue = true};
    response.message = HandleCommand(clientId, chat.message);

    SendChat(clientId, response);
    return;
  }

  RanchCommandChatNotify response{
    .message = chat.message};

  characterRecord->Immutable([&response](const soa::data::Character& character)
                             { response.author = character.name(); });

  for (const auto ranchClientId : ranchInstance._clients)
  {
    SendChat(ranchClientId, response);
  }
}

std::string RanchDirector::HandleCommand(
  ClientId clientId,
  const std::string& message)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  const auto command = soa::util::TokenizeString(
    message.substr(2), ' ');

  if (command.empty())
    return "No command provided.";

  if (command[0] == "give")
  {
    if (command.size() < 2)
      return "Invalid command sub-literal. (//give <item/horse>)";

    if (command[1] == "item")
    {
      if (command.size() < 4)
        return "Invalid command arguments. (//give item <count> <uid>)";

      uint32_t itemCount = std::atoi(command[2].c_str());
      soa::data::Uid createdItemTid = std::atoi(command[3].c_str());

      // Create the item.
      auto createdItemUid = soa::data::InvalidUid;
      auto createdItemRecord = GetServerInstance().GetDataDirector().CreateItem();
      createdItemRecord.Mutable([createdItemTid, itemCount, &createdItemUid](soa::data::Item& item)
      {
        item.tid() = createdItemTid;
        item.count() = itemCount;

        createdItemUid = item.uid();
      });

      // Create the stored item.
      auto giftUid = soa::data::InvalidUid;
      auto storedItem = GetServerInstance().GetDataDirector().CreateStoredItem();
      storedItem.Mutable([this, &giftUid, createdItemUid, createdItemTid](soa::data::StoredItem& storedItem)
      {
        storedItem.items().emplace_back(createdItemUid);
        storedItem.sender() = "System";
        storedItem.message() = std::format("Item '{}'", createdItemTid);

        giftUid = storedItem.uid();
      });

      // Add the stored item as a gift.

      characterRecord->Mutable([giftUid](soa::data::Character& character)
      {
        character.gifts().emplace_back(giftUid);
      });

      return "Item given. Check your gifts in inventory!";
    }

    if (command[1] == "horse")
    {
      if (command.size() < 3)
      {
        return "Not implemented";
      }
    }
  }

  return "Invalid command";
}

void RanchDirector::SendChat(
  ClientId clientId,
  const RanchCommandChatNotify& chat)
{
  _commandServer.QueueCommand<RanchCommandChatNotify>(
    clientId,
    CommandId::RanchChatNotify,
    [chat]()
    {
      return chat;
    });
}

void RanchDirector::HandleWearEquipment(
  ClientId clientId,
  const RanchCommandWearEquipment& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  bool equipSuccessful = false;
  characterRecord->Mutable([&command, &equipSuccessful](soa::data::Character& character)
  {
    const bool hasEquippedItem = std::ranges::contains(
      character.inventory(), command.itemUid);
    const bool hasMountedHorse = std::ranges::contains(
      character.horses(), command.itemUid);

    // Make sure the equip UID is either a valid item or a horse.
    equipSuccessful = hasEquippedItem || hasMountedHorse;

    if (hasMountedHorse)
      character.mountUid() = command.itemUid;
    else if (hasEquippedItem)
      character.characterEquipment().emplace_back(command.itemUid);
  });

  if (equipSuccessful)
  {
    RanchCommandWearEquipmentOK response{
      .itemUid = command.itemUid,
      .member = command.member};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      CommandId::RanchWearEquipmentOK,
      [response]()
      {
        return response;
      });
    BroadcastEquipmentUpdate(clientId);

    return;
  }

  RanchCommandWearEquipmentCancel response{
    .itemUid = command.itemUid,
    .member = command.member};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchWearEquipmentCancel,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRemoveEquipment(
  ClientId clientId,
  const RanchCommandRemoveEquipment& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  characterRecord->Mutable([&command](soa::data::Character& character)
  {
    const bool ownsItem = std::ranges::contains(
      character.inventory(), command.itemUid);

    // You can't really unequip a horse. You can only switch to a different one.
    // At least in Alicia 1.0.

    if (ownsItem)
    {
      const auto range = std::ranges::remove(
        character.characterEquipment(), command.itemUid);
      character.characterEquipment().erase(range.begin(), range.end());
    }
  });

  // We really don't need to cancel the unequip.
  // Always respond with OK.
  RanchCommandRemoveEquipmentOK response{
    .uid = command.itemUid};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchRemoveEquipmentOK,
    [response]()
    {
      return response;
    });

  BroadcastEquipmentUpdate(clientId);
}

void RanchDirector::BroadcastEquipmentUpdate(ClientId clientId)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  RanchCommandUpdateEquipmentNotify notify{
    .characterUid = clientContext.characterUid};

  characterRecord->Immutable([this, &notify](const soa::data::Character& character)
  {
    // Character equipment
    const auto characterEquipment = GetServerInstance().GetDataDirector().GetItems().Get(
      character.characterEquipment());
    protocol::BuildProtocolItems(notify.characterEquipment, *characterEquipment);

    // Mount equipment
    const auto mountEquipment = GetServerInstance().GetDataDirector().GetItems().Get(
      character.mountEquipment());
    protocol::BuildProtocolItems(notify.mountEquipment, *mountEquipment);

    // Mount
    const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.mountUid());

    mountRecord->Immutable([&notify](const soa::data::Horse& mount)
    {
      protocol::BuildProtocolHorse(notify.mount, mount);
    });
  });

  // Broadcast to all the ranch clients.
  const auto& ranchInstance = _ranches[clientContext.ranchUid];
  for (ClientId ranchClientId : ranchInstance._clients)
  {
    // Prevent broadcasting to self.
    if (ranchClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      CommandId::RanchUpdateEquipmentNotify,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::HandleUseItem(
  ClientId clientId,
  const RanchCommandUseItem& command)
{
  RanchCommandUseItemOK response
  {
    response.unk0 = command.unk0,
    response.unk1 = command.unk1,
    response.type = RanchCommandUseItemOK::ActionType::Empty
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchUseItemOK,
    [response]()
    {
      return response;
    });
}

} // namespace alicia
