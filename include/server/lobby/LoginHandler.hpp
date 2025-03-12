//
// Created by rgnter on 26/11/2024.
//

#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP

#include "server/DataDirector.hpp"

namespace alicia
{

//! Login handler.
class LoginHandler
{
public:
  struct Result
  {
    //! Verdict of the result.
    enum class Verdict
    {
      Accepted,
      Rejected
    } verdict{Verdict::Rejected};

    //! Unique identifier of the authenticated user.
    uint32_t userUid{0};
  };

  explicit LoginHandler(DataDirector& dataDirector);

  //!
  void Authenticate(
    const std::string& userName,
    const std::string& providedUserToken,
    std::function<void(Result)> resultCallback);

private:


  //!
  DataDirector& _dataDirector;
};

} // namespace alicia

#endif //LOGINHANDLER_HPP
