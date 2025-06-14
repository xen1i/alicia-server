//
// Created by rgnter on 31/05/2025.
//

#ifndef PROTOCOLHELPER_HPP
#define PROTOCOLHELPER_HPP


#include "libserver/data/DataDefinitions.hpp"
#include "libserver/data/DataStorage.hpp"
#include "libserver/network/command/proto/CommonStructureDefinitions.hpp"

namespace alicia
{

namespace protocol
{

void BuildProtocolCharacter(
  Character& protocolCharacter,
  const soa::data::Character& character);

void BuildProtocolHorse(
  Horse& protocolHorse,
  const soa::data::Horse& horse);

void BuildProtocolHorses(
    std::vector<Horse>& protocolHorses,
    const std::vector<soa::Record<soa::data::Horse>>& horses);

void BuildProtocolItem(
  Item& protocolItem,
  const soa::data::Item& item);

void BuildProtocolItems(
  std::vector<Item>& protocolItems,
  const std::vector<soa::Record<soa::data::Item>>& items);

void BuildProtocolStoredItem(
  StoredItem& protocolStoredItem,
  const soa::data::StoredItem& storedItem);

void BuildProtocolStoredItems(
  std::vector<StoredItem>& protocolStoredItems,
  const std::vector<soa::Record<soa::data::StoredItem>>& storedItems);

} // namespace protocol

} // namespace alicia

#endif //PROTOCOLHELPER_HPP
