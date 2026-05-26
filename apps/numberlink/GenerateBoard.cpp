#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>
#include "GenerateBoard.hpp"
#include "Board.hpp"



void generatePairedMatrix(std::string filename, int m, int n, int pair_count) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    int total_cells = m * n;
    if (pair_count * 2 > total_cells) {
        std::cerr << "Error: Not enough cells for " << pair_count << " pairs!" << std::endl;
        return;
    }

    // 必要な数字ペアを作る
    std::vector<std::string> cells;
    for (int i = 1; i <= pair_count; ++i) {
        cells.push_back(std::to_string(i));
        cells.push_back(std::to_string(i));
    }

    // 残りを "-" で埋める
    while (cells.size() < static_cast<unsigned>(total_cells)) {
        cells.push_back("-");
    }

    // シャッフルする
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cells.begin(), cells.end(), g);

    // n×mで書き出す
    int idx = 0;
    ofs << n << " ";
    ofs << m << "\n";
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            ofs << cells[idx++] << " ";
        }
        ofs << "\n";
    }

    ofs.close();

}
/*
int main() {
    int m = 5;         // 行数
    int n = 6;         // 列数
    int pairs = 2;    // 置きたいペア数（2つで1ペア）
    
    generatePairedMatrix("test3.dat", m, n, pairs);
    std::cout << "Matrix with " << pairs << " pairs written to test3.dat!" << std::endl;
}
*/