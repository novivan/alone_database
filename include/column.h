#include "cells.h"
#include <vector>
#include <algorithm>
#include <stdexcept>


//std::type_info* fnd(std::vector<const std::type_info*> a, )

class Column 
{
public:
    int type; //смотрите выше
    std::vector <Cell*> cells;


    Column (int tp): type(tp){}

    Column(Column *other): type(other->type), cells(other->cells){}

    Column(Column &&other): type(other.type), cells(std::move(other.cells)){}

    Column(Cell& cell) 
    {   
        std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);

        type = std::find(CellTypes.begin(), CellTypes.end(), &typeid(cell)) - CellTypes.begin();

        if (type == 0) {
            cells.push_back(new CellInt(dynamic_cast<CellInt&>(cell)));  // Создаем новый объект CellInt
        } else if (type == 1) {
            cells.push_back(new CellBool(dynamic_cast<CellBool&>(cell)));  // Создаем новый объект CellBool
        } else if (type == 2) {
            cells.push_back(new CellString(dynamic_cast<CellString&>(cell)));  // Создаем новый объект CellString
        } else if (type == 3) {
            cells.push_back(new CellBytes(dynamic_cast<CellBytes&>(cell)));  // Создаем новый объект CellBytes
        } else {
            throw std::runtime_error("Unknown cell type");
        }
    }

    Cell& get_cell(int index) 
    {
        return *cells[index];
    }

    int get_cell_index(Cell &cell) 
    {   
        std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);

        if (type != std::find(CellTypes.begin(), CellTypes.end(), &typeid(cell)) - CellTypes.begin()) {
            throw "there's no cell in this column with such type\n";
            return -1;
        }
        for (int i = 0; i < cells.size(); i++) 
        {
            if (*cells[i] == cell) 
            {
                return i;
            }
        }
        throw "there's no such cell in this column\n";
        return -1;
    }

    void add_cell(Cell& cell) 
    {
        std::vector<const std::type_info*> CellTypes(4);
        CellTypes[0] = &typeid(CellInt);
        CellTypes[1] =  &typeid(CellBool);
        CellTypes[2] =  &typeid(CellString);
        CellTypes[3] =  &typeid(CellBytes);

        if (type != std::find(CellTypes.begin(), CellTypes.end(), &typeid(cell)) - CellTypes.begin()) {
            throw "Unknown cell type\n";
            return;
        }
        if (type == 0) {
            cells.push_back(new CellInt(dynamic_cast<CellInt&>(cell)));  // Создаем новый объект CellInt
        } else if (type == 1) {
            cells.push_back(new CellBool(dynamic_cast<CellBool&>(cell)));  // Создаем новый объект CellBool
        } else if (type == 2) {
            cells.push_back(new CellString(dynamic_cast<CellString&>(cell)));  // Создаем новый объект CellString
        } else if (type == 3) {
            cells.push_back(new CellBytes(dynamic_cast<CellBytes&>(cell)));  // Создаем новый объект CellBytes
        } else {
            throw std::runtime_error("Unknown cell type");
        }
    }

    ~Column() 
    {
        // Освобождаем память для объектов ячеек
        for (Cell* cell : cells) {
            delete cell;
        }
    }
};