//
// Created by maros on 4/7/25.
//

#ifndef SQLUTILS_HPP
#define SQLUTILS_HPP

#include <format>
#include <functional>
#include <string>

namespace soa
{

template<typename T>
class SqlComposer
{
public:
  using PresencePredicate = std::function<bool(const T&)>;
  using ValueSupplier = std::function<std::string(const T&)>;

  //! Adds a field.
  //! @param name Name of the parameter.
  //! @param valuePredicate Predicate indicating whether the parameter should be present.
  //! @param valueSupplier Value supplier.
  SqlComposer& Field(
    const std::string& name,
    PresencePredicate valuePredicate,
    ValueSupplier valueSupplier)
  {
    _parameters[name] = Value{
      .isPresent = std::move(valuePredicate),
      .get = std::move(valueSupplier)};
    return *this;
  }

  std::string ComposeUpsert(
    const T& value,
    const std::string& table)
  {
    const std::string sqlNames = GenerateFormattedSql(
      value,
      _parameters,
      [](const auto& name, const auto& value)
      {
        return std::format("{}", name);
      });
    const std::string sqlValues = GenerateFormattedSql(
      value,
      _parameters,
      [](const auto& name, const auto& value)
      {
        return std::format("{}", value);
      });

    const std::string sql = std::format(
      "INSERT INTO {} ({}) VALUES ({})",
      table, sqlNames, sqlValues);
    return sql;
  }

  std::string BuildUpdate(
     const T& value,
    const std::string& table)
  {
    const std::string sqlParameters = GenerateFormattedSql(
      value,
      _parameters,
      [](const auto& name, const auto& value)
      {
        return std::format("{} = {}", name, value);
      });

    const std::string sql = std::format(
      "UPDATE {} SET {}",
      table,
      sqlParameters);

    return sql;
  }

private:
  struct Value
  {
    PresencePredicate isPresent;
    ValueSupplier get;
  };

  std::string GenerateFormattedSql(
    const T& data,
    const std::unordered_map<std::string, Value>& values,
    std::function<std::string(const std::string& name, const std::string& value)> formatter)
  {
    std::vector<std::string> valueArray;
    for (const auto& [name, parameter] : values)
    {
      if (not parameter.isPresent(data))
        continue;
      valueArray.emplace_back(formatter(
        name, std::format("{}", parameter.get(data))));
    }

    std::string sql;
    for (auto it = valueArray.begin(); it != valueArray.end();)
    {
      sql += *it;

      ++it;
      if (it == valueArray.end())
        break;

      sql += ", ";
    }

    return sql;
  }

  std::unordered_map<std::string, Value> _parameters;
};

} // namespace soa

#endif //SQLUTILS_HPP
