/*
 * Copyright (c) 2014 Hiroaki Iwashita
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "NumlinWithLenZdd.hpp"

static const int ENDPOINT_LENGTH = 1; // 端点セルの長さ
static const int PATH_CELL_LENGTH = 1; // 経路セルの長さ

int NumlinZddWithLen::getRoot(CellState* state) const {
    for (int j = 0; j < n; ++j) {
        int t = quiz.number[0][j];
        state[j].mate = (t > 0) ? n + t : j;
        state[j].len = (t > 0) ? ENDPOINT_LENGTH : PATH_CELL_LENGTH;
    }
    return top_level;
}

int NumlinZddWithLen::getChild(CellState* state, int level, int take) const {
    int i = (top_level - level) / (n - 1);
    int j = (top_level - level) % (n - 1);

    if (take) { // 横辺 e_h(i, j) を【採用】する
        int k = j + 1;
        int mj = state[j].mate;
        int mk = state[k].mate;

        if (mj == n || mk == n) return 0; // 分岐禁止
        if (mj == k) return 0;            // サイクル禁止

        // ★ 横辺を追加したことによる新しい経路長の計算
        // 既に含まれているセル数をそのまま足し合わせる
        int new_len = state[j].len + state[k].len;
        
        if (new_len > K) return 0; // ★ クリティカルパス長制約による枝刈り！

        state[j].mate = state[k].mate = n; // 内側になったマスは使用済みに

        if (mj > n && mk > n) { // パスが完成（両端が数字マス）
            if (mj != mk) return 0; // 違う数字が繋がったらダメ
        } 
        else { // 接続関係とパス長を更新
            if (mj < n) {
                state[mj].mate = mk;
                state[mj].len  = new_len; // ★ パスの反対側の端点に新しい長さを記録
            }
            if (mk < n) {
                state[mk].mate = mj;
                state[mk].len  = new_len; // ★ パスの反対側の端点に新しい長さを記録
            }
        }
    }
    // take == 0 の場合（横辺不採用）は何も長さは変わらないので省略

    // 列 j から列 jj までの縦辺の処理
    int jj = (j < n - 2) ? j : n - 1;
    if (i < m - 1) { // 最終行でないとき
        for (int k = j; k <= jj; ++k) { 
            int mk = state[k].mate;
            int t = quiz.number[i + 1][k];
            
            if (mk != k && mk != n) { // 縦辺 e_v(i, k) が【採用】される場合（必ず下に降りる）
                
                // ★ 縦辺1本分の長さを足す
                int new_len = state[k].len + 1;
                
                if (new_len > K) return 0; // ★ クリティカルパス長制約による枝刈り！

                if (t > 0) { // 降りた先が数字マス(ラベル付き)の場合
                    state[k].mate = n;
                    if (mk > n) { // パス完成
                        if (mk != n + t) return 0; 
                    } else {
                        state[mk].mate = n + t;
                        state[mk].len  = new_len; // ★ 更新
                    }
                } 
                else { // 降りた先が空白マスの場合
                    // mate の参照先は変わらないので長さだけ更新
                    state[k].len = new_len; // ★ 更新
                    if (mk < n) {
                        state[mk].len = new_len; // ★ 反対側の端点も更新
                    }
                }
            } 
            else { // 縦辺が採用されない場合
                state[k].mate = (t > 0) ? n + t : k;
                state[k].len  = (t > 0) ? ENDPOINT_LENGTH : PATH_CELL_LENGTH;
            }
        }
    }

    return (--level >= 1) ? level : -1;
}
