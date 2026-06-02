#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

const int MIN_PATH_LEN = 3; // 1つの線の最低長さ(3マス以上)

int ROWS, COLS, TARGET_PATHS;
vector<vector<int>> board;
mt19937 rng(random_device{}());

long long dfs_steps = 0;
const long long MAX_DFS_STEPS = 100000;

// 周囲の状況を見て進めるか判定 (2x2禁止・自己隣接禁止)
bool isValidMove(int r, int c, int path_id) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    if (board[r][c] != 0) return false; // 既に線がある、または数字があるマスは不可

    int count = 0;
    if (r > 0 && board[r-1][c] == path_id) count++;
    if (r < ROWS-1 && board[r+1][c] == path_id) count++;
    if (c > 0 && board[r][c-1] == path_id) count++;
    if (c < COLS-1 && board[r][c+1] == path_id) count++;

    // 周囲に自分の線が「1つだけ」なら進める
    return count == 1;
}

// 深さ優先探索(DFS) - 空白マスを許容するバージョン
bool dfs(int r, int c, int path_id, int current_len) {
    dfs_steps++;
    if (dfs_steps > MAX_DFS_STEPS) return false;

    board[r][c] = path_id;

    // 現在の線が最低長さ以上になったら、「線をここで切る」選択肢を許可
    bool can_stop = (current_len >= MIN_PATH_LEN);

    vector<pair<int, int>> dirs = {{1,0}, {-1,0}, {0,1}, {0,-1}};
    shuffle(dirs.begin(), dirs.end(), rng);
    
    // 0~3は上下左右に伸ばす。4は「切る」
    vector<int> choices = {0, 1, 2, 3}; 
    if (can_stop) {
        // 空白マスができやすくなるよう、線を切る確率を少し高くする(選択肢を増やす)
        choices.push_back(4);
        choices.push_back(4); 
    }
    shuffle(choices.begin(), choices.end(), rng);

    for (int choice : choices) {
        if (choice < 4) { // 線を伸ばす
            int nr = r + dirs[choice].first;
            int nc = c + dirs[choice].second;
            if (isValidMove(nr, nc, path_id)) {
                if (dfs(nr, nc, path_id, current_len + 1)) return true;
            }
        } 
        else { // 線をここで切る
            if (path_id == TARGET_PATHS) {
                // 目標のペア数まで全て引き終わった！(空白マスが残っていてもここで成功として終了)
                return true;
            } else {
                // 次のパス(path_id + 1)のスタート地点となる「ランダムな空きマス」を探す
                vector<pair<int, int>> empty_cells;
                for(int i = 0; i < ROWS; i++) {
                    for(int j = 0; j < COLS; j++) {
                        if(board[i][j] == 0) empty_cells.push_back({i, j});
                    }
                }
                
                if (!empty_cells.empty()) {
                    shuffle(empty_cells.begin(), empty_cells.end(), rng);
                    int next_r = empty_cells[0].first;
                    int next_c = empty_cells[0].second;
                    if (dfs(next_r, next_c, path_id + 1, 1)) return true;
                }
            }
        }
    }

    // バックトラック
    board[r][c] = 0;
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "使用方法: " << argv[0] << " <行数> <列数> <端点ペアの数>" << endl;
        return 1;
    }

    ROWS = stoi(argv[1]);
    COLS = stoi(argv[2]);
    TARGET_PATHS = stoi(argv[3]);

    // ペア数が多すぎると絶対に配置できないので簡単なチェック
    if (TARGET_PATHS * MIN_PATH_LEN > ROWS * COLS) {
        cout << "エラー: ペア数が多すぎます。マスが足りません。" << endl;
        return 1;
    }

    cout << "空白許容 " << ROWS << "x" << COLS << " (ペア数:" << TARGET_PATHS << ") の盤面を生成中..." << endl;
    
    bool success = false;
    for (int attempt = 1; attempt <= 1000; attempt++) {
        board.assign(ROWS, vector<int>(COLS, 0));
        dfs_steps = 0;
        
        // パス1のスタート地点もランダムに決める
        int start_r = rng() % ROWS;
        int start_c = rng() % COLS;

        if (dfs(start_r, start_c, 1, 1)) {
            success = true;
            break;
        }
    }

    if (!success) {
        cout << "エラー: 盤面の生成に失敗しました。" << endl;
        return 1;
    }

    // 端点の抽出
    vector<vector<string>> puzzle(ROWS, vector<string>(COLS, "-"));
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int id = board[r][c];
            if (id == 0) continue; // 空白マスは無視

            int count = 0;
            if (r > 0 && board[r-1][c] == id) count++;
            if (r < ROWS-1 && board[r+1][c] == id) count++;
            if (c > 0 && board[r][c-1] == id) count++;
            if (c < COLS-1 && board[r][c+1] == id) count++;

            if (count == 1) puzzle[r][c] = to_string(id);
        }
    }

    // 保存処理
    string dir_name = "BoardData";
    if (!fs::exists(dir_name)) fs::create_directory(dir_name);

    int file_num = 1;
    string filename;
    while (true) {
        filename = dir_name + "/test" + to_string(file_num) + ".dat";
        if (!fs::exists(filename)) break;
        file_num++;
    }

    ofstream ofs(filename);
    ofs << ROWS << " " << COLS << "\n";
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            ofs << puzzle[r][c] << (c == COLS - 1 ? "" : " ");
            cout << puzzle[r][c] << " ";
        }
        ofs << "\n";
        cout << "\n";
    }
    ofs.close();

    cout << "成功: " << filename << " に保存しました！" << endl;
    return 0;
}