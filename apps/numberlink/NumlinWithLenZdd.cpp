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

/**
 * mate 配列の意味
 * mate[j] = j  degree=0
 * mate[j] = n  degree=2
 * mate[j] > n  数字マスに繋がっている (mate[j]-n が数字)
 * その他        mate[j] と繋がっている
 */

int NumlinZddWithLen::getRoot(CellState* state) const {
    for (int j = 0; j < n; ++j) {
        int t = quiz.number[0][j];
        state[j].mate = (t > 0) ? n + t : j; //j列のマスが数字マスなら n+t, そうでなければ j
        state[j].len = (t > 0) ? ENDPOINT_LENGTH : PATH_CELL_LENGTH; //j列のマスが数字マスなら端点の長さ, そうでなければ経路セルの長さ
    }
    return top_level;
}

int NumlinZddWithLen::getChild(CellState* state, int level, int take) const {
    int i = (top_level - level) / (n - 1);
    int j = (top_level - level) % (n - 1);

    if (take) { // 横辺 e_h(i, j) を採用する
        int k = j + 1;
        int mj = state[j].mate;
        int mk = state[k].mate;

        if (mj == n || mk == n) return 0; // 分岐禁止
        if (mj == k) return 0;            // サイクル禁止

        // 横辺を追加したことによる新しい経路長の計算
        // 既に含まれているセル数をそのまま足し合わせる
        int new_len = state[j].len + state[k].len;
        
        if (new_len > K) return 0; // CP長制約による枝刈り!

        state[j].mate = state[k].mate = n; 
        // 両方とも数字マスに繋がることになるので mate を n にする

        if (mj > n && mk > n) { // パスが完成（両端が数字マス）
            if (mj != mk) return 0; // 違う数字が繋がったらダメ
        } 
        else { // 接続関係とパス長を更新
            if (mj < n) {
                state[mj].mate = mk;
                state[mj].len  = new_len; // パスの反対側の端点に新しい長さ(2点のパス長の和)を記録
            }
            if (mk < n) {
                state[mk].mate = mj;
                state[mk].len  = new_len; // パスの反対側の端点に新しい長さ(2点のパス長の和)を記録
            }
        }
    }
    // take == 0 の場合（横辺不採用）は何も長さは変わらない。

    // 列 j から列 jj までの縦辺の処理
    // jj は j と j+1 のどちらか小さい方 
    // 最後の列の場合分けのためにj+1ではなくjjとしている
    int jj = (j < n - 2) ? j : n - 1;
    if (i < m - 1) { // 最終行でないとき
        for (int k = j; k <= jj; ++k) { 
            int mk = state[k].mate;
            int t = quiz.number[i + 1][k]; // 次の行の k 列目のマスに書いてある数字（0なら空白）
            
            if (mk != k && mk != n) { // 縦辺 e_v(i, k) が採用される場合（必ず下に降りる）
                
                // 縦辺1本分の長さを足す
                int new_len = state[k].len + PATH_CELL_LENGTH; // 縦辺を追加することで、フロンティア以上にある経路の長さが1増える
                
                if (new_len > K) return 0; // CP長制約による枝刈り!

                if (t > 0) { // 降りた先が数字マス(ラベル付き)の場合
                    state[k].mate = n;
                    if (mk > n) { // パス完成
                        if (mk != n + t) return 0; 
                    } else {
                        state[mk].mate = n + t;
                        state[mk].len  = new_len; // 更新
                    }
                } 
                else { // 降りた先が空白マスの場合
                    // mate の参照先は変わらないので長さだけ更新
                    state[k].len = new_len; // 更新
                    if (mk < n) { // すでに数字マスに繋がっている場合は、反対側の端点の長さも更新する必要がある
                        state[mk].len = new_len; // 反対側の端点も更新
                    }
                }
            } 
            else { // 縦辺が採用されない場合
                // 次の行のマスが数字マスならk列目のマスの mate を n+t に、そうでなければ k にする
                state[k].mate = (t > 0) ? n + t : k;
                state[k].len  = (t > 0) ? ENDPOINT_LENGTH : PATH_CELL_LENGTH;
            }
        }
    }
    return (--level >= 1) ? level : -1;
}
