#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdSpecOp.hpp>
#include "Board.hpp"
#include "DegreeZdd.hpp"
#include "NumlinWithLenZdd.hpp"

using namespace std;
using namespace tdzdd;

int ROWS;
int COLS;
int TARGET_PATHS;
vector<vector<int>> board;
mt19937 rng(random_device{}());
long long dfs_steps = 0;
const long long MAX_DFS_STEPS = 100000;

bool isValidMove(int r, int c, int path_id) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    if (board[r][c] != 0) return false;

    int count = 0;
    if (r > 0 && board[r - 1][c] == path_id) count++;
    if (r + 1 < ROWS && board[r + 1][c] == path_id) count++;
    if (c > 0 && board[r][c - 1] == path_id) count++;
    if (c + 1 < COLS && board[r][c + 1] == path_id) count++;

    return count == 1;
}

bool dfs(int r, int c, int path_id, int current_len, int filled_count) {
    dfs_steps++;
    if (dfs_steps > MAX_DFS_STEPS) return false;

    board[r][c] = path_id;
    filled_count++;

    if (filled_count == ROWS * COLS) {
        if (path_id == TARGET_PATHS && current_len >= 3) return true;
        board[r][c] = 0;
        return false;
    }

    bool can_stop = (current_len >= 3 && path_id < TARGET_PATHS);
    vector<pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    shuffle(dirs.begin(), dirs.end(), rng);

    vector<int> choices = {0, 1, 2, 3};
    if (can_stop) choices.push_back(4);
    shuffle(choices.begin(), choices.end(), rng);

    for (int choice : choices) {
        if (choice < 4) {
            int nr = r + dirs[choice].first;
            int nc = c + dirs[choice].second;
            if (isValidMove(nr, nc, path_id)) {
                if (dfs(nr, nc, path_id, current_len + 1, filled_count)) return true;
            }
        } else {
            int next_r = -1;
            int next_c = -1;
            vector<pair<int,int>> empties;
            empties.reserve(ROWS * COLS);
            for (int i = 0; i < ROWS; ++i) {
                for (int j = 0; j < COLS; ++j) {
                    if (board[i][j] == 0) {
                        empties.emplace_back(i, j);
                    }
                }
            }
            if (!empties.empty()) {
                shuffle(empties.begin(), empties.end(), rng);
                next_r = empties[0].first;
                next_c = empties[0].second;
            }
            if (next_r != -1) {
                if (dfs(next_r, next_c, path_id + 1, 1, filled_count)) return true;
            }
        }
    }

    board[r][c] = 0;
    return false;
}

int computeCriticalPathLength() {
    vector<vector<bool>> visited(ROWS, vector<bool>(COLS, false));
    int maxLen = 0;

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (board[i][j] == 0 || visited[i][j]) continue;
            int id = board[i][j];
            int neighbors = 0;
            if (i > 0 && board[i - 1][j] == id) neighbors++;
            if (i + 1 < ROWS && board[i + 1][j] == id) neighbors++;
            if (j > 0 && board[i][j - 1] == id) neighbors++;
            if (j + 1 < COLS && board[i][j + 1] == id) neighbors++;
            if (neighbors != 1) continue;

            int length = 0;
            int ci = i;
            int cj = j;
            int pi = -1;
            int pj = -1;

            while (true) {
                visited[ci][cj] = true;
                int next_i = -1;
                int next_j = -1;

                if (ci > 0 && board[ci - 1][cj] == id && !(pi == ci - 1 && pj == cj)) {
                    next_i = ci - 1;
                    next_j = cj;
                }
                if (ci + 1 < ROWS && board[ci + 1][cj] == id && !(pi == ci + 1 && pj == cj)) {
                    next_i = ci + 1;
                    next_j = cj;
                }
                if (cj > 0 && board[ci][cj - 1] == id && !(pi == ci && pj == cj - 1)) {
                    next_i = ci;
                    next_j = cj - 1;
                }
                if (cj + 1 < COLS && board[ci][cj + 1] == id && !(pi == ci && pj == cj + 1)) {
                    next_i = ci;
                    next_j = cj + 1;
                }

                if (next_i < 0) break;
                ++length;
                pi = ci;
                pj = cj;
                ci = next_i;
                cj = next_j;
            }

            maxLen = max(maxLen, length);
        }
    }

    return maxLen;
}

Board makeQuizBoard() {
    Board quiz;
    quiz.rows = ROWS;
    quiz.cols = COLS;
    quiz.init();

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int id = board[i][j];
            if (id == 0) continue;

            int neighbors = 0;
            if (i > 0 && board[i - 1][j] == id) neighbors++;
            if (i + 1 < ROWS && board[i + 1][j] == id) neighbors++;
            if (j > 0 && board[i][j - 1] == id) neighbors++;
            if (j + 1 < COLS && board[i][j + 1] == id) neighbors++;
            if (neighbors == 1) {
                quiz.number[i][j] = id;
            }
        }
    }

    return quiz;
}

bool validateWithLongestPath(Board const& quiz, int maxLen) {
    DegreeZdd degree(quiz);
    DdStructure<2> dd(zddLookahead(degree));
    dd.zddReduce();

    NumlinZddWithLen numlinLen(quiz, maxLen);
    dd.zddSubset(zddLookahead(numlinLen));
    dd.zddReduce();

    double count = dd.evaluate(ZddCardinality<double,2>());
    return count > 0.0;
}

string nextOutputPath() {
    const string dir = "BoardData";
    if (mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST) {
        throw runtime_error("cannot create directory BoardData");
    }

    int index = 1;
    while (true) {
        string path = dir + "/test" + to_string(index) + ".dat";
        ifstream ifs(path.c_str());
        if (!ifs.good()) return path;
        ++index;
    }
}

void writeBoardToFile(Board const& quiz, const string& path) {
    ofstream ofs(path.c_str());
    if (!ofs) throw runtime_error("cannot open output file");
    ofs << quiz.cols << " " << quiz.rows << "\n";
    for (int i = 0; i < quiz.rows; ++i) {
        for (int j = 0; j < quiz.cols; ++j) {
            if (j) ofs << " ";
            if (quiz.number[i][j] == 0) ofs << "-";
            else ofs << quiz.number[i][j];
        }
        ofs << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "usage: " << argv[0] << " <rows> <cols> <pairs>\n";
        return 1;
    }

    ROWS = stoi(argv[1]);
    COLS = stoi(argv[2]);
    TARGET_PATHS = stoi(argv[3]);

    if (ROWS < 3 || COLS < 3 || TARGET_PATHS < 1) {
        cerr << "error: rows and cols must be at least 3, pairs must be at least 1\n";
        return 1;
    }

    cout << "Generating " << ROWS << "x" << COLS << " board with " << TARGET_PATHS << " pairs...\n";

    bool success = false;
    string output_path;
    for (int attempt = 1; attempt <= 1000; ++attempt) {
        board.assign(ROWS, vector<int>(COLS, 0));
        dfs_steps = 0;

        int start_r = 0;
        int start_c = 0;
        if (!dfs(start_r, start_c, 1, 1, 0)) {
            continue;
        }

        int maxLen = computeCriticalPathLength();
        Board quiz = makeQuizBoard();

        if (validateWithLongestPath(quiz, maxLen)) {
            output_path = nextOutputPath();
            writeBoardToFile(quiz, output_path);
            cout << "saved: " << output_path << " (max path length = " << maxLen << ")\n";
            success = true;
            break;
        }
    }

    if (!success) {
        cerr << "failed: could not generate a valid board after multiple attempts\n";
        return 1;
    }

    return 0;
}
