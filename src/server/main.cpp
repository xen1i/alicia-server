#include "Version.hpp"

#include "server/lobby/LobbyDirector.hpp"
#include "server/race/RaceDirector.hpp"
#include "server/ranch/RanchDirector.hpp"
#include "server/Settings.hpp"

#include <libserver/base/Server.hpp>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <server/Scheduler.hpp>
#include <thread>

namespace
{

std::unique_ptr<server::Scheduler> g_scheduler;

std::unique_ptr<alicia::DataDirector> g_dataDirector;
std::unique_ptr<alicia::LobbyDirector> g_loginDirector;
std::unique_ptr<alicia::RanchDirector> g_ranchDirector;
std::unique_ptr<alicia::RaceDirector> g_raceDirector;

} // namespace

int main()
{
  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log.log", 0, 0);

  // Console sink.
  const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  auto applicationLogger = std::make_shared<spdlog::logger>(
    "abc",
    spdlog::sinks_init_list{fileSink, consoleSink});

  applicationLogger->set_level(spdlog::level::debug);
  applicationLogger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(applicationLogger);

  spdlog::info("Running Alicia server v{}.", alicia::BuildVersion);

  // Parsing settings file
  alicia::Settings settings;
  settings.LoadFromFile("resources/settings.json5");

  g_scheduler = std::make_unique<server::Scheduler>();

  g_scheduler->RunOnMainThread([&settings]()
  {
    // Data director.
    g_dataDirector = std::make_unique<alicia::DataDirector>(
      settings._dataSourceSettings);

    // Lobby director.
    g_loginDirector = std::make_unique<alicia::LobbyDirector>(
      *g_dataDirector,
      settings._lobbySettings);

    // Ranch director.
    g_ranchDirector = std::make_unique<alicia::RanchDirector>(
    *g_dataDirector,
      settings._ranchSettings);

    // Race director.
    g_raceDirector = std::make_unique<alicia::RaceDirector>(
      *g_dataDirector,
      settings._raceSettings);

    // TODO: Messenger
    alicia::CommandServer messengerServer("Messenger");
    messengerServer.Host(boost::asio::ip::address_v4::any(), 10032);
  });

  // Synchronize the main thread with the main server thread.
  g_scheduler->GetMainThreadExecutor().Synchronize();

  return 0;
}

