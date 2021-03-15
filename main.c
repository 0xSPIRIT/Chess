#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define CELL_SIZE 100

#define BOARD_W 8
#define BOARD_H 8

int screen_width = CELL_SIZE * BOARD_W;
int screen_height = CELL_SIZE * BOARD_H;

enum {
    PIECE_EMPTY = 0,
    PIECE_KING,
    PIECE_QUEEN,
    PIECE_BISHOP,
    PIECE_KNIGHT,
    PIECE_ROOK,
    PIECE_PAWN
};

typedef struct {
    int type;
    bool is_white;
} Piece;

int pawn_that_moved_two_squares_x = -1;
int pawn_that_moved_two_squares_y = -1;
bool did_move_pawn = false;

Piece pieces[BOARD_W][BOARD_H] = {
    { {PIECE_ROOK, 0}, {PIECE_KNIGHT, 0}, {PIECE_BISHOP, 0}, {PIECE_QUEEN, 0}, {PIECE_KING, 0}, {PIECE_BISHOP, 0}, {PIECE_KNIGHT, 0}, {PIECE_ROOK, 0} },
    { {PIECE_PAWN, 0}, {PIECE_PAWN, 0}, {PIECE_PAWN, 0}, {PIECE_PAWN, 0}, {PIECE_PAWN, 0}, {PIECE_PAWN, 0}, {PIECE_PAWN, 0}, {PIECE_PAWN, 0} },
    { {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0} },
    { {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0} },
    { {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0} },
    { {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0}, {PIECE_EMPTY, 0} },
    { {PIECE_PAWN, 1}, {PIECE_PAWN, 1}, {PIECE_PAWN, 1}, {PIECE_PAWN, 1}, {PIECE_PAWN, 1}, {PIECE_PAWN, 1}, {PIECE_PAWN, 1}, {PIECE_PAWN, 1} },
    { {PIECE_ROOK, 1}, {PIECE_KNIGHT, 1}, {PIECE_BISHOP, 1}, {PIECE_QUEEN, 1}, {PIECE_KING, 1}, {PIECE_BISHOP, 1}, {PIECE_KNIGHT, 1}, {PIECE_ROOK, 1} },
};

/* Test board: */
/* Piece pieces[BOARD_H][BOARD_W] = { */
/*     { {PIECE_ROOK, 0}, {PIECE_QUEEN, 0}, {PIECE_KING, 0}, {PIECE_ROOK, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}, */
/*     { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {PIECE_PAWN,0} }, */
/*     { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, */
/*     { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, */
/*     { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, */
/*     { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, */
/*     { {PIECE_PAWN, 1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, */
/*     { {PIECE_ROOK, 1}, {PIECE_QUEEN, 1}, {PIECE_KING, 1}, {PIECE_ROOK, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}} */
/* }; */

int selected_x = -1, selected_y = -1;

bool is_white_turn = true;
int is_game_over = 0;

int sign(int x) {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}

SDL_Rect spritesheet_get_piece_rect(Piece p) {
    SDL_Rect r;
    r.x = (p.type - 1) * CELL_SIZE;
    r.y = !p.is_white * CELL_SIZE;
    r.w = CELL_SIZE;
    r.h = CELL_SIZE;

    return r;
}

void move_piece(int from_x, int from_y, int to_x, int to_y) {
    pieces[to_y][to_x] = pieces[from_y][from_x];

    pieces[from_y][from_x].is_white = 0;
    pieces[from_y][from_x].type = 0;
}

bool can_promote(int px, int py) {
    if (pieces[py][px].type != PIECE_PAWN) return false;

    if (pieces[py][px].is_white && py == 0) return true;
    if (!pieces[py][px].is_white && py == BOARD_H-1) return true;

    return false;
}

/* Note: this doesn't check if castling is a valid move. That's handled in another function. */
bool is_move_valid(int from_x, int from_y, int to_x, int to_y) {
    Piece p;
    bool is_valid = false;
    int temp_from_x;
    int temp_from_y;
    
    if (from_x == to_x && from_y == to_y) return false;
    
    p = pieces[from_y][from_x];

    if (pieces[to_y][to_x].type != PIECE_EMPTY && pieces[to_y][to_x].is_white == p.is_white) return false;

    switch (p.type) {
    case PIECE_KING:
        is_valid = abs(to_x - from_x) <= 1 && abs(to_y - from_y) <= 1;
        break;
    case PIECE_PAWN:
        if (p.is_white) {
            if (to_x == from_x && to_y == from_y-1 && pieces[to_y][to_x].type == PIECE_EMPTY) is_valid = true;
            if (from_y == 6 && to_x == from_x && to_y == from_y-2 && pieces[to_y][to_x].type == PIECE_EMPTY && pieces[to_y+1][to_x].type == PIECE_EMPTY) {
                did_move_pawn = true;
                is_valid = true;
            }
            if (to_x == from_x-1 && to_y == from_y-1 && pieces[to_y][to_x].type != PIECE_EMPTY) is_valid = true;
            if (to_x == from_x+1 && to_y == from_y-1 && pieces[to_y][to_x].type != PIECE_EMPTY) is_valid = true;
        } else {
            if (to_x == from_x && to_y == from_y+1 && pieces[to_y][to_x].type == PIECE_EMPTY) is_valid = true;
            if (from_y == 1 && to_x == from_x && to_y == from_y+2 && pieces[to_y][to_x].type == PIECE_EMPTY && pieces[to_y-1][to_x].type == PIECE_EMPTY) {
                did_move_pawn = true;
                is_valid = true;
            }
            if (to_x == from_x-1 && to_y == from_y+1 && pieces[to_y][to_x].type != PIECE_EMPTY) is_valid = true;
            if (to_x == from_x+1 && to_y == from_y+1 && pieces[to_y][to_x].type != PIECE_EMPTY) is_valid = true;
        }
        break;
    case PIECE_ROOK:
        is_valid = (to_x == from_x || to_y == from_y);
        break;
    case PIECE_BISHOP:
        is_valid = abs(to_x - from_x) == abs(to_y - from_y);
        break;
    case PIECE_QUEEN:
        is_valid = (to_x == from_x || to_y == from_y) || abs(to_x - from_x) == abs(to_y - from_y);
        break;
    case PIECE_KNIGHT:
        is_valid = (abs(to_x - from_x) == 1 && abs(to_y - from_y) == 2) || (abs(to_x - from_x) == 2 && abs(to_y - from_y) == 1);
        break;
    }

    /* We're manipulating from_x and from_y instead of using another variable, so we're gonna reset it. */
    temp_from_x = from_x;
    temp_from_y = from_y;
    
    if (p.type == PIECE_ROOK || p.type == PIECE_QUEEN) {
        if (to_x == from_x) {
            int s = sign(to_y - from_y);

            from_y += s;
            
            while (from_y != to_y) {
                if (pieces[from_y][from_x].type != PIECE_EMPTY) is_valid = false;
                from_y += s;
            }
        } else if (to_y == from_y) {
            int s = sign(to_x - from_x);

            from_x += s;
            
            while (from_x != to_x) {
                if (pieces[from_y][from_x].type != PIECE_EMPTY) is_valid = false;
                from_x += s;
            }
        }
    }
    if (p.type == PIECE_BISHOP || p.type == PIECE_QUEEN) {
        int sx = sign(to_x - from_x);
        int sy = sign(to_y - from_y);

        from_x += sx;
        from_y += sy;

        while (from_x != to_x && from_y != to_y) {
            if (pieces[from_y][from_x].type != PIECE_EMPTY) is_valid = false;
            from_x += sx;
            from_y += sy;
        }
    }
    if (p.type == PIECE_KING) {
        int king_x, king_y;
        for (y = 0; y < BOARD_H; ++y) {
            for (x = 0; x < BOARD_W; ++x) {
                if (pieces[y][x].type == PIECE_KING) {
                    if (piece.is_white) {
                        king_x = x;
                        king_y = y;
                    } else {
                        king_x = x;
                        king_y = y;
                    }
                }
            }
        }
        
        if (to_x == king_x && to_y == king_y) {
            is_valid = true;    /* This is only so that in the is_in_check() function it will evaluate to being in check since this is a valid move. */
        }
    }
    
    from_x = temp_from_x;
    from_y = temp_from_y;

    return is_valid;
}

int is_in_check(void) {
    int white_king_x, white_king_y;
    int black_king_x, black_king_y;
    int x, y;

    for (y = 0; y < BOARD_H; ++y) {
        for (x = 0; x < BOARD_W; ++x) {
            if (pieces[y][x].type == PIECE_KING) {
                if (pieces[y][x].is_white) {
                    white_king_x = x;
                    white_king_y = y;
                } else {
                    black_king_x = x;
                    black_king_y = y;
                }
            }
        }
    }
    
    for (y = 0; y < BOARD_H; ++y) {
        for (x = 0; x < BOARD_W; ++x) {
            if (is_move_valid(x, y, white_king_x, white_king_y)) {
                return 1;
            }
            if (is_move_valid(x, y, black_king_x, black_king_y)) {
                return 2;
            }
        }
    }

    fflush(stdout);
    return 0;
}

void attempt_castle(int from_x, int from_y, int to_x, int to_y) {
    bool white = pieces[from_y][from_x].is_white;
    
    if (pieces[from_y][from_x].type != PIECE_KING) return;
    if (white ? is_in_check() == 1 : is_in_check() == 2) return;

    if (to_x - from_x > 0) {
        /* King's side castle */
        if (from_x == 4 && (white ? from_y == 7 : from_y == 0) && to_x == from_x+2 && to_y == from_y) {
            if (pieces[from_y][to_x+1].type != PIECE_ROOK || pieces[from_y][to_x+1].is_white != white) return;

            if (pieces[from_y][from_x+1].type == PIECE_EMPTY && pieces[from_y][from_x+2].type == PIECE_EMPTY) {
                Piece p[BOARD_H][BOARD_W];
                memcpy(p, pieces, sizeof(Piece) * 8 * 8);
                
                move_piece(from_x, from_y, to_x, to_y); /* King */
                move_piece(to_x+1, from_y, to_x-1, to_y); /* Rook */

                is_white_turn = !is_white_turn;

                if ((white ? is_in_check() == 1 : is_in_check() == 2)) {
                    memcpy(pieces, p, sizeof(Piece) * 8 * 8);
                }
            }
        }
    } else {
        /* Queen's side castle */
        if (from_x == 4 && (white ? from_y == 7 : from_y == 0) && to_x == from_x-2 && to_y == from_y) {
            if (pieces[to_y][to_x-2].type != PIECE_ROOK || pieces[from_y][to_x+1].is_white != white) return;

            if (pieces[from_y][from_x-1].type == PIECE_EMPTY && pieces[from_y][from_x-2].type == PIECE_EMPTY) {
                Piece p[BOARD_H][BOARD_W];
                memcpy(p, pieces, sizeof(Piece) * 8 * 8);
                
                move_piece(from_x, from_y, to_x, to_y); /* King */
                move_piece(to_x-2, from_y, to_x+1, to_y); /* Rook */

                is_white_turn = !is_white_turn;

                if ((white ? is_in_check() == 1 : is_in_check() == 2)) {
                    memcpy(pieces, p, sizeof(Piece) * 8 * 8);
                }
            }
        }
    }
}

void attempt_en_passant(int from_x, int from_y, int to_x, int to_y) {
    Piece p = pieces[from_y][to_x];
    bool white = pieces[from_y][from_x].is_white;

    if (white) {
        if (from_y != 3) return;
    } else {
        if (from_y != 4) return;
    }
        
    if (abs(from_x - to_x) != 1) return;
    if (abs(from_y - to_y) != 1) return;
    
    if (p.type != PIECE_PAWN || p.is_white == pieces[from_y][from_x].is_white) return;

    {
        Piece pp[BOARD_H][BOARD_W];

        memcpy(pp, pieces, sizeof(Piece) * 8 * 8);

        move_piece(from_x, from_y, to_x, to_y);
        pieces[from_y][to_x].type = PIECE_EMPTY;

        if ((white && is_in_check() == 1) || (!white && is_in_check() == 2)) {
            memcpy(pieces, pp, sizeof(Piece) * 8 * 8);
            return;
        }

        is_white_turn = !is_white_turn;
    }
}

bool is_in_checkmate(int who) {
    int x, y;
    int i, j;

    for (y = 0; y < BOARD_H; ++y) {
        for (x = 0; x < BOARD_W; ++x) {
            if (who == 1 && !pieces[y][x].is_white) continue;
            if (who == 2 && pieces[y][x].is_white) continue;

            for (i = 0; i < BOARD_W; ++i) {
                for (j = 0; j < BOARD_H; ++j) {
                    if (is_move_valid(x, y, i, j)) {
                        Piece p[BOARD_H][BOARD_W];
                        memcpy(p, pieces, sizeof(Piece) * BOARD_W * BOARD_H);

                        move_piece(x, y, i, j);
                        
                        if (!is_in_check()) {
                            printf("Not checkmate because of: %d, %d to %d, %d\n", x, y, i, j); fflush(stdout);
                            memcpy(pieces, p, sizeof(Piece) * BOARD_W * BOARD_H);
                            return false;
                        }

                        memcpy(pieces, p, sizeof(Piece) * BOARD_W * BOARD_H);
                    }
                }
            }
        }
    }

    return true;
}

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running = true;

    SDL_Surface *surf, *go1, *go2;
    SDL_Texture *texture, *gow, *gob;
    
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    
    window = SDL_CreateWindow("Chess",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              screen_width,
                              screen_height,
                              0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    
    surf = IMG_Load("pieces.png");
    texture = SDL_CreateTextureFromSurface(renderer, surf);

    go1 = IMG_Load("white_win.bmp");
    gow = SDL_CreateTextureFromSurface(renderer, go1);

    go2 = IMG_Load("black_win.bmp");
    gob = SDL_CreateTextureFromSurface(renderer, go2);

    SDL_FreeSurface(surf);
    SDL_FreeSurface(go1);
    SDL_FreeSurface(go2);

    while (running) {
        SDL_Event event;
        SDL_Rect r = {
            0,
            0,
            CELL_SIZE,
            CELL_SIZE
        };

        int x, y, c;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;

            if (event.type == SDL_MOUSEBUTTONDOWN && !is_game_over) {
                int px = event.button.x / CELL_SIZE;
                int py = event.button.y / CELL_SIZE;

                int was_in_check = is_in_check();
                
                /* if (!is_white_turn) { */
                /*     py = (BOARD_H - 1) - py; */
                /* } */
                if (selected_x == -1) {
                    if (pieces[py][px].type == PIECE_EMPTY || pieces[py][px].is_white != is_white_turn) break;
                
                    selected_x = px;
                    selected_y = py;
                } else {
                    if (is_move_valid(selected_x, selected_y, px, py)) {
                        Piece tmp[BOARD_W][BOARD_H];
                        memcpy(tmp, pieces, sizeof(Piece) * BOARD_W * BOARD_H);
                        move_piece(selected_x, selected_y, px, py);
                        if ((was_in_check == 1 && is_white_turn) ||
                            (was_in_check == 2 && !is_white_turn)) {
                            if ((was_in_check == 1 && is_in_check() == 1) ||
                                (was_in_check == 2 && is_in_check() == 2)) {
                                /* Illegal move. */
                                memcpy(pieces, tmp, sizeof(Piece) * BOARD_W * BOARD_H);
                            } else {
                                is_white_turn = !is_white_turn;
                            }
                        } else {
                            if ((!was_in_check && ((is_white_turn && 1 == is_in_check()) || (!is_white_turn && 2 == is_in_check())))) {
                                /* Illegal move. */
                                memcpy(pieces, tmp, sizeof(Piece) * BOARD_W * BOARD_H);
                            } else {
                                int c = is_in_check();
                                is_white_turn = !is_white_turn;
                                if ((c == 1 && is_white_turn) || (c == 2 && !is_white_turn)) {
                                    bool is = is_in_checkmate(c);
                                    if (is) {
                                        is_game_over = c==1 ? 2 : 1;
                                    }
                                }
                            }
                        }

                        if (can_promote(px, py)) {
                            pieces[py][px].type = PIECE_QUEEN;
                        }

                        if (did_move_pawn) {
                            pawn_that_moved_two_squares_x = px;
                            pawn_that_moved_two_squares_y = py;
                            did_move_pawn = false;
                        } else {
                            pawn_that_moved_two_squares_x = -1;
                            pawn_that_moved_two_squares_y = -1;
                        }
                    }


                    attempt_en_passant(selected_x, selected_y, px, py);
                    attempt_castle(selected_x, selected_y, px, py);
                    
                    selected_x = -1;
                    selected_y = -1;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        c = is_in_check();
        
        for (y = 0; y < BOARD_H; ++y) {
            for (x = 0; x < BOARD_W; ++x) {
                /* int ty = y; */
                /* if (!is_white_turn) { */
                /*     y = (BOARD_H-1) - y; */
                /*     fflush(stdout); */
                /* } */
                
                r.x = x * CELL_SIZE;
                r.y = y * CELL_SIZE;
                /* y = ty; */

                if ((x+y) % 2 == 1) {
                    SDL_SetRenderDrawColor(renderer, 255, 216, 158, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 219, 139, 71, 1);
                }

                if (pieces[y][x].type == PIECE_KING && ((c == 1 && pieces[y][x].is_white) || (c == 2 && !pieces[y][x].is_white))) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }
                
                if (selected_x == x && selected_y == y) {
                    SDL_SetRenderDrawColor(renderer, 216, 255, 90, 255);
                }

                SDL_RenderFillRect(renderer, &r);

                if (pieces[y][x].type != PIECE_EMPTY) {
                    SDL_Rect src = spritesheet_get_piece_rect(pieces[y][x]);
                    SDL_RenderCopy(renderer, texture, &src, &r);
                }
            }
        }

        if (is_game_over == 1) {
            SDL_RenderCopy(renderer, gow, NULL, NULL);
        } else if (is_game_over == 2) {
            SDL_RenderCopy(renderer, gob, NULL, NULL);
        }
        
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(gow);
    SDL_DestroyTexture(gob);
    
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    
    SDL_Quit();

    return 0;
}
