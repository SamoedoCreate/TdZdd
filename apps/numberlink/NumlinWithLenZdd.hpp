#pragma once // 多重インクルード防止

#include <tdzdd/DdSpec.hpp>
#include "Board.hpp"

// 新しい状態：各マスの接続先(mate)と、そこまで伸びているパスの長さ(len)
struct CellState {
    short mate;
    short len;
    
    // TdZddが状態の合流判定をするための等価演算子
    bool operator==(const CellState& o) const {
        return mate == o.mate && len == o.len;
    }
};

class NumlinZddWithLen : public tdzdd::PodArrayDdSpec<NumlinZddWithLen, CellState, 2> {
    Board const& quiz;
    int const m, n, top_level;
    int const K; // ★ 追加：パス長の上限制約

public:
    NumlinZddWithLen(Board const& quiz, int max_len) 
        : quiz(quiz), m(quiz.rows), n(quiz.cols), 
          top_level(m * (n - 1)), K(max_len) {
        setArraySize(quiz.cols);
    }
    
    int getRoot(CellState* state) const;
    int getChild(CellState* state, int level, int take) const;
};
