#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <sstream>

#include "table.h"

class Database
{
public:
    std::unordered_map<std::string, Table> tables;

    Database() = default;

    void clear() 
    {
        tables.clear();
    }

    void readFromFile(const std::string& csv_filename)
    {
        // Очищаем текущую базу данных
        clear();

        std::ifstream file(csv_filename);
        if (!file.is_open()) 
        {
            throw std::runtime_error("Could not open file");
        }

        std::string line;
        std::getline(file, line);
        int numTables = std::stoi(line);

        for (int t = 0; t < numTables; ++t) 
        {
            // Читаем заголовок таблицы
            std::getline(file, line);
            std::istringstream ss(line);
            std::string tableName;
            std::getline(ss, tableName, ',');

            std::string numColumnsStr, numRowsStr;
            std::getline(ss, numColumnsStr, ',');
            int numColumns = std::stoi(numColumnsStr);
            std::getline(ss, numRowsStr);
            int numRows = std::stoi(numRowsStr);

            std::vector<std::pair<std::string, int>> columns;
            for (int c = 0; c < numColumns; ++c) 
            {
                std::string columnName;
                int columnType;
                std::getline(ss, columnName, ',');
                std::string columnTypeStr;
                std::getline(ss, columnTypeStr, ',');
                columnType = std::stoi(columnTypeStr);
                columns.emplace_back(columnName, columnType);
            }

            createTable(tableName, columns);

            // Читаем данные таблицы
            for (int r = 0; r < numRows; ++r) 
            {
                std::getline(file, line);
                std::istringstream dataStream(line);
                Line dataLine;
                for (const auto& [columnName, columnType] : columns) 
                {
                    std::string cellData;
                    std::getline(dataStream, cellData, ',');
                    if (columnType == 0) {
                        dataLine.addCell(columnName, std::make_shared<CellInt>(std::stoi(cellData)));
                    } else if (columnType == 1) {
                        dataLine.addCell(columnName, std::make_shared<CellBool>(cellData == "1"));
                    } else if (columnType == 2) {
                        dataLine.addCell(columnName, std::make_shared<CellString>(cellData));
                    } else if (columnType == 3) {
                        std::string bytes = std::string(cellData.begin(), cellData.end());
                        /*for (size_t i = 0; i < cellData.size(); i += 2) {
                            bytes.push_back(cellData[i]);
                        }*/
                        dataLine.addCell(columnName, std::make_shared<CellBytes>(bytes));
                    }
                }
                insert(tableName, dataLine);
            }
            // Пропускаем пустую строку между таблицами
            std::getline(file, line);
        }

        file.close();
    }

    void saveToFile(const std::string& filename) 
    {
        std::ofstream file(filename);
        if (!file.is_open()) 
        {
            throw std::runtime_error("Could not open file");
        }

        // Записываем количество таблиц
        file << tables.size() << "\n";

        for (const auto& [tableName, table] : tables) 
        {
            // Записываем заголовок таблицы
            file << tableName << "," << table.columns.size() << "," << table.columns.begin()->second.cells.size() << "\n";

            for (const auto& [columnName, column] : table.columns) 
            {
                file << columnName << "," << column.type << ",";
            }
            file << "\n";

            // Записываем данные таблицы
            size_t numRows = table.columns.begin()->second.cells.size();
            for (size_t i = 0; i < numRows; ++i) 
            {
                for (const auto& [columnName, column] : table.columns) 
                {
                    if (column.type == 0) {
                        file << std::static_pointer_cast<CellInt>(column.cells[i])->data;
                    } else if (column.type == 1) {
                        file << std::static_pointer_cast<CellBool>(column.cells[i])->data;
                    } else if (column.type == 2) {
                        file << std::static_pointer_cast<CellString>(column.cells[i])->data;
                    } else if (column.type == 3) {
                        auto bytes = std::static_pointer_cast<CellBytes>(column.cells[i])->data;
                        file << std::string(bytes.begin(), bytes.end());
                        /*for (auto byte : bytes) {
                            file << static_cast<int>(byte);
                        }*/
                    }
                    file << ",";
                }
                file << "\n";
            }
            file << "\n"; // Разделяем таблицы пустой строкой
        }

        file.close();
    }

    /* Database(const std::string& csv_filename) 
    {
        readFromFile(csv_filename);
    }*/

    ~Database() = default;

    void printTable(const std::string& tableName) 
    {
        if (tables.find(tableName) == tables.end()) 
        {
            throw std::invalid_argument("Table not found: " + tableName);
        }
        tables.at(tableName).printTable();
    }

    void createTable(const std::string tableName, const std::vector <std::pair<std::string, int>>& colums)
    {   
        tables[tableName] = Table(tableName);
        for (auto& [columnName, columnType] : colums) 
        {
            tables[tableName].addColumn(columnName, columnType);
        }
    }

    Table select(const std::string& newTablename, const std::string& tableName, std::vector <std::string> columnNames, const std::function<bool(const Line&)>& condition)
    {
        return tables[newTablename] = tables.at(tableName).select(newTablename, columnNames, condition);
    }

    void insert(const std::string& tableName, Line& line)
    {
        tables.at(tableName).insert(line);
    }

    void update(const std::string& tableName, const std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>>& transformations, const std::function<bool(const Line&)>& condition)
    {
        tables.at(tableName).update(transformations, condition);
    }

    void remove(const std::string& tableName, const std::function<bool(const Line&)>& condition)
    {
        tables.at(tableName).remove(condition);
    }

    Table join(const std::string& newTableName, const std::string& tableName1, const std::string& tableName2, const std::function<bool(const Line&, const Line&)>& condition)
    {   
        if (tables.find(tableName1) == tables.end()) {
            throw std::invalid_argument("Table not found: " + tableName1);
        }
        if (tables.find(tableName2) == tables.end()) {
            throw std::invalid_argument("Table not found: " + tableName2);
        }
        return tables[newTableName] = tables.at(tableName1).join(newTableName, tables.at(tableName2), condition);
    }
};