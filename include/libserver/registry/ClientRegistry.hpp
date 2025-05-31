//
// Created by rgnter on 30/05/2025.
//

#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <cstdint>
#include <unordered_map>

#include "libserver/data/DataDefinitions.hpp"

namespace soa
{

class ClientRegistry
{
  struct Client
  {
    bool authorized{false};
  };
};

} // namespace soa

#endif //CLIENTREGISTRY_HPP
