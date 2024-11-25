#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

//это закомменчено, потому что пока не реализовано
//#include <../include/database.h>

//а это временно, пока databse не реализовано
#include <../include/table.h>


void printTable(const Table& table) {
    std::cout << "Table: " << table.name << std::endl;
    for (const auto& column : table.columns) {
        std::cout << column.first << "  ";
        if (column.second.type == 3) {
            std:: cout << "                                                      ";
        }
    }
    std::cout << std::endl;

    size_t numRows = table.columns.begin()->second.cells.size();
    for (size_t i = 0; i < numRows; ++i) {
        for (const auto& column : table.columns) {
            if (column.second.cells[i] == nullptr) {
                std::cerr << "Error: nullptr detected in column '" << column.first << "' at row " << i << std::endl;
                continue;
            }
            if (column.first == "id") {
                std::cout << std::static_pointer_cast<CellInt>(column.second.cells[i])->data << "\t";
            } else if (column.first == "login") {
                std::cout << std::static_pointer_cast<CellString>(column.second.cells[i])->data << "\t";
            } else if (column.first == "is_admin") {
                std::cout << std::static_pointer_cast<CellBool>(column.second.cells[i])->data << "\t";
            } else if (column.first == "password_hash") {
                auto bytes = std::static_pointer_cast<CellBytes>(column.second.cells[i])->data;
                for (auto byte : bytes) {
                    std::cout << std::hex << static_cast<int>(byte);
                }
                std::cout << "\t";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl << std::endl << std::endl;
}







int main() {
    // Создаем таблицу
    Table users("users");

    //printTable(users);

    std::cout << "testTableCreation passed." << std::endl;
    // Добавляем столбцы
    users.addColumn("id", 0);
    users.addColumn("is_admin", 1);
    users.addColumn("login", 2);
    users.addColumn("password_hash", 3);

    std::cout << "testAddColumn passed." << std::endl;
    
    // Вставляем строки
    Line line1;
    line1.addCell("id", std::make_shared<CellInt>(1));
    line1.addCell("is_admin", std::make_shared<CellBool>(false));
    line1.addCell("login", std::make_shared<CellString>("vasya"));
    line1.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xde, 0xad, 0xbe, 0xef}));
    users.insert(line1);

    std::cout << "testAddRow passed." << std::endl;


    
    Line line2;
    line2.addCell("id", std::make_shared<CellInt>(2));
    line2.addCell("is_admin", std::make_shared<CellBool>(true));
    line2.addCell("login", std::make_shared<CellString>("admin"));
    line2.addCell("password_hash", std::make_shared<CellBytes>(std::vector<uint8_t>{0xca, 0xfe, 0xba, 0xbe}));
    users.insert(line2);

    std::cout << "second testAddRow passed." << std::endl;
    
    printTable(users);

    std::cout << "testPrintTable passed." << std::endl;
    
    auto selected = users.select("selected_users", {"id", "login"}, [](const Line& line) {
        try {
            bool isAdmin = std::static_pointer_cast<CellBool>(line.cells.at("is_admin"))->data;
            int id = std::static_pointer_cast<CellInt>(line.cells.at("id"))->data;
            return isAdmin || id < 10;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    });
    // Печатаем результаты SELECT
    std::cout << "Selected users:" << std::endl;
    printTable(selected);

    // Выполняем UPDATE
    std::cout << "Testing UPDATE..." << std::endl;

    // Создаем словарь трансформаций
    std::unordered_map<std::string, std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>)>> transformations = {
        {"login", [](std::shared_ptr<Cell> cell) {
            auto strCell = std::static_pointer_cast<CellString>(cell);
            return std::make_shared<CellString>(strCell->data + "_updated");
        }},
        {"is_admin", [](std::shared_ptr<Cell> cell) {
            auto boolCell = std::static_pointer_cast<CellBool>(cell);
            return std::make_shared<CellBool>(!boolCell->data); // инвертируем значение
        }}
    };

    // Обновляем данные
    users.update(transformations, [](const Line& line) {
        return std::static_pointer_cast<CellString>(line.cells.at("login"))->data == "vasya";
    });

    std::cout << "After UPDATE:" << std::endl;
    printTable(users);
    
    // Выполняем DELETE
    users.remove([](const Line& line) {
        return static_pointer_cast<CellString>(line.cells.at("login"))->data == "admin";
    });

    // печатаем результаты DELETE
    std::cout << "Users after delete:" << std::endl;
    printTable(users);
    
    users.~Table();

    std::cout << "All tests passed." << std::endl;

    return 0;
}