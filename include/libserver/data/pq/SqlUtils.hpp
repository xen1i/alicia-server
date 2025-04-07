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

struct SqlDecomposer
{

};

class SqlComposer
{
public:
  using ValuePredicate = std::function<bool()>;
  using ValueSupplier = std::function<std::string()>;

  //! Adds parameter
  //! @param name Name of the parameter.
  //! @param valuePredicate Predicate indicating whether the value is available.
  //! @param valueSupplier Value supplier.
  SqlComposer& Parameter(
    const std::string& name,
    ValuePredicate valuePredicate,
    ValueSupplier valueSupplier)
  {
    _parameters[name] = Value{
      .isPresent = std::move(valuePredicate),
      .get = std::move(valueSupplier)};

    return *this;
  }

  //! Adds condition
  //! @param name Name of the condition.
  //! @param valuePredicate Predicate indicating whether the value is available.
  //! @param valueSupplier Value supplier.
  SqlComposer& Condition(
    const std::string& name,
    ValuePredicate valuePredicate,
    ValueSupplier valueSupplier)
  {
    _conditions[name] = Value{
      .isPresent = std::move(valuePredicate),
      .get = std::move(valueSupplier)};

    return *this;
  }

  //! Builds an SQL insert.
  //! @param table Database table.
  //! @returns Generated SQL insert statement.
  std::string BuildInsert(
    const std::string& table)
  {
    const std::string sqlNames = GenerateFormattedSql(
      _parameters,
      [](const auto& name, const auto& value)
      {
        return std::format("{}", name);
      });
    const std::string sqlValues = GenerateFormattedSql(
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

  //! Builds an SQL update.
  //! @param table Database table.
  //! @returns Generated SQL update statement.
  std::string BuildUpdate(
    const std::string& table)
  {
    const std::string sqlParameters = GenerateFormattedSql(
      _parameters,
      [](const auto& name, const auto& value)
      {
        return std::format("{} = {}", name, value);
      });
    const std::string sqlConditions = GenerateFormattedSql(
      _conditions,
      [](const auto& name, const auto& value)
      {
        return std::format("{} = {}", name, value);
      });

    const std::string sql = std::format(
      "UPDATE {} SET {}{}",
      table,
      sqlParameters,
      sqlConditions.empty()
        ? "" : std::format(" WHERE {}", sqlConditions));

    return sql;
  }

private:
  struct Value
  {
    ValuePredicate isPresent;
    ValueSupplier get;
  };

  std::string GenerateFormattedSql(
    const std::unordered_map<std::string, Value>& values,
    std::function<std::string(const std::string& name, const std::string& value)> formatter)
  {
    std::vector<std::string> valueArray;
    for (const auto& [name, parameter] : values)
    {
      if (not parameter.isPresent)
        continue;
      valueArray.emplace_back(formatter(
        name, std::format("{}", parameter.get())));
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
  std::unordered_map<std::string, Value> _conditions;
};

} // namespace soa

#endif //SQLUTILS_HPP
