//
// Created by rgnter on 26/11/2024.
//

#include "server/lobby/LoginHandler.hpp"

#include "spdlog/spdlog.h"

namespace alicia
{

LoginHandler::LoginHandler(DataDirector& dataDirector)
    : _dataDirector(dataDirector)
{
}

void LoginHandler::Authenticate(
  const std::string& userName,
  const std::string& authenticationToken,
  std::function<void(Result result)> resultCallback)
{
  // Both strings must be valid.
  if (userName.empty() || authenticationToken.empty())
  {
    resultCallback(Result{
      .verdict = Result::Verdict::Rejected});
    return;
  }

  _dataDirector.GetToken(
    userName,
    [userName, authenticationToken, resultCallback = std::move(resultCallback)](
      auto tokenRecord)
    {
      const auto [userUid, token] = tokenRecord();

      // If the tokens do not match reject the login.
      if (authenticationToken != token)
      {
        spdlog::info(
          "User '{}' ({}) authentication rejected.",
          userName,
          userUid);

        resultCallback(Result{
          .verdict = Result::Verdict::Rejected});
        return;
      }

      spdlog::info(
        "User '{}' ({}) authentication accepted.",
        userName,
        userUid);
      resultCallback(Result{
        .verdict = Result::Verdict::Accepted,
        .userUid = userUid});
    },
    [userName, resultCallback = std::move(resultCallback)]()
    {
      spdlog::error(
        "User '{}' couldn't authenticate due to error",
        userName);

      resultCallback(Result{
        .verdict = Result::Verdict::Rejected});
    });
}

}


