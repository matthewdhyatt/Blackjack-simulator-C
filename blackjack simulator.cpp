#include <iostream>
#include <numeric>
#include <vector>
#include <tuple>
#include <algorithm>
#include <ctime>
#include <unordered_map>
#include <map>

#include "strategies.h"

//constants
int number_of_decks=6, number_of_shoes=pow(10,1);
float penetration=0.75, bj_payout=1.5, profit=0;

//globals
std::vector<int> shoe, dealer_score, player_scores;
int hand_counter=0;

class Hand {
    std::vector<int> cards;
    public:
        Hand(std::vector<int>);
        std::vector<int> see_cards() {return cards;}
        std::tuple<std::string,int> total() {
            int s=std::accumulate(cards.begin(), cards.end(), 0);
            if ((cards.size()==2) && (cards[0]==cards[1])) return std::make_tuple ("pair",s);
            else if ((std::find(cards.begin(),cards.end(),1) != cards.end()) && (s<12)) return std::make_tuple ("soft",s+10);
            else if (s<22) return std::make_tuple ("hard",s);
            else return std::make_tuple ("bust",0);
        }
};
Hand::Hand(std::vector<int> x){
    cards=x;
}
//Hand globals
Hand dealer(std::vector<int> {1,1});
std::vector<Hand> player_queue, player_done;

void start_shoe(){
    std::vector<int> suit {1,2,3,4,5,6,7,8,9,10,10,10,10};
    std::vector<int> deck;
    for(int i=0; i<4; i++ ) deck.insert( deck.end(), suit.begin(), suit.end() );
    for(int i=0; i<number_of_decks; i++ ) shoe.insert( shoe.end(), deck.begin(), deck.end() );
    std::srand(std::time(0));
    std::random_shuffle (shoe.begin(),shoe.end());
}

void deal(){
    std::vector<int> x;
    x.push_back(shoe.back());
    shoe.pop_back();
    x.push_back(shoe.back());
    shoe.pop_back();
    dealer=Hand(x);

    x.clear();
    x.push_back(shoe.back());
    shoe.pop_back();
    x.push_back(shoe.back());
    shoe.pop_back();
    player_queue.push_back(Hand(x));
}

void dealer_play(){

    int keep_playing=1;

    if ((dealer.see_cards()==std::vector<int> {1,10}) || (dealer.see_cards()==std::vector<int> {10,1})){
        dealer_score.push_back(50); //using 50 for blackjack
        keep_playing=0;
    }

    while (keep_playing==1){
        auto current=dealer.total();
        if (std::get<0>(current)=="bust"){
            dealer_score.push_back(std::get<1>(current));
            keep_playing=0;
        }
        else if (dealer_strategy.at(current)=="stand"){
            dealer_score.push_back(std::get<1>(current));
            keep_playing=0;
        }
        else if (dealer_strategy.at(current)=="hit"){
            auto hand_update=dealer.see_cards();
            hand_update.push_back(shoe.back());
            shoe.pop_back();
            dealer=Hand(hand_update);
        }
    }
}

void player_play(){
    while (player_queue.size()>0){
        auto x=player_queue.back();
        player_queue.pop_back();
        int keep_playing=1;

        if ((x.see_cards()==std::vector<int> {1,10}) || (x.see_cards()==std::vector<int> {10,1})){
            player_scores.push_back(50); //using 50 for blackjack
            player_done.push_back(x);
            keep_playing=0;
        }

        while (keep_playing==1){
            auto current=x.total();
            if (std::get<0>(current)=="bust"){
                player_scores.push_back(std::get<1>(current));
                player_done.push_back(x);
                keep_playing=0;
            }
            else if (player_strategy.at(std::tuple_cat(current,std::make_tuple(dealer.see_cards()[0])))=="stand"){
                player_scores.push_back(std::get<1>(current));
                player_done.push_back(x);
                keep_playing=0;
            }
            else if (player_strategy.at(std::tuple_cat(current,std::make_tuple(dealer.see_cards()[0])))=="hit"){
                auto hand_update=x.see_cards();
                hand_update.push_back(shoe.back());
                shoe.pop_back();
                x=Hand(hand_update);
            }
            else if (player_strategy.at(std::tuple_cat(current,std::make_tuple(dealer.see_cards()[0])))=="split"){
                std::vector<int> hand_update {x.see_cards()[0],shoe.back()};
                shoe.pop_back();
                player_queue.push_back(Hand(hand_update));

                hand_update.pop_back();
                hand_update.push_back(shoe.back());
                shoe.pop_back();
                player_queue.push_back(Hand(hand_update));
                keep_playing=0;
            }
            else if (player_strategy.at(std::tuple_cat(current,std::make_tuple(dealer.see_cards()[0])))=="double"){
                if (x.see_cards().size()==2){
                    auto hand_update=x.see_cards();
                    hand_update.push_back(shoe.back());
                    shoe.pop_back();
                    x=Hand(hand_update);
                    current=x.total();

                    player_scores.push_back(std::get<1>(current));
                    player_scores.push_back(std::get<1>(current));
                    player_done.push_back(x);
                    keep_playing=0;
                }
                else {
                    auto hand_update=x.see_cards();
                    hand_update.push_back(shoe.back());
                    shoe.pop_back();
                    x=Hand(hand_update);
                }
            }
            else if (player_strategy.at(std::tuple_cat(current,std::make_tuple(dealer.see_cards()[0])))=="split_aces"){
                std::vector<int> hand_update {1,shoe.back()};
                shoe.pop_back();
                x=Hand(hand_update);
                current=x.total();
                player_scores.push_back(std::get<1>(current));
                player_done.push_back(x);

                hand_update.pop_back();
                hand_update.push_back(shoe.back());
                shoe.pop_back();
                x=Hand(hand_update);
                current=x.total();
                player_scores.push_back(std::get<1>(current));
                player_done.push_back(x);

                keep_playing=0;
            }
        }
    }
}

void play_shoe(){
    start_shoe();
    while (shoe.size()>(1-penetration)*number_of_decks*52){
        hand_counter+=1;
        dealer_score.clear();
        player_queue.clear();
        player_done.clear();
        player_scores.clear();

        //std::cout << "current shoe: ";
        //for (auto x : shoe) std::cout << x << " ";
        //std::cout << std::endl;
        //std::cout << std::endl;

        deal();
        //std::cout << "dealer starting cards: ";
        //for (auto x : dealer.see_cards()) std::cout << x << " ";
        //std::cout << std::endl;
        //std::cout << "player starting cards: ";
        //for (auto x : player_queue[0].see_cards()) std::cout << x << " ";
        //std::cout << std::endl;
        //std::cout << std::endl;

        dealer_play();
        //std::cout << "results of dealer play" << std::endl;
        //std::cout << "score: " << dealer_score[0] << std::endl;
        //std::cout << "hand: ";
        //for (auto x : dealer.see_cards()) std::cout << x << " ";
        //std::cout << std::endl;
        //std::cout << std::endl;

        player_play();
        //std::cout << "results of player play" << std::endl;
        //std::cout << "scores: ";
        //for (auto x : player_scores) std::cout << x << " ";
        //std::cout << std::endl;
        //for (int i=0; i<player_done.size(); i++){
            //std::cout << "hand " << i+1 << ": ";
            //for (auto x : player_done[i].see_cards()) std::cout << x << " ";
            //std::cout << std::endl;
        //}
        //std::cout << std::endl;

        for (auto x : player_scores){
            if (x>dealer_score[0]){
                if (x==50){
                    profit+=bj_payout;
                }
                else profit+=1;
            }
            else if ((x<dealer_score[0]) || (x==0)) profit-=1;
        }
        //std::cout << "current profit: " << profit << std::endl;
        //std::cout << "-----------------------------------------------------------" << std::endl << std::endl;
    }
}



int main(){
    make_dealer_strategy();
    make_player_strategy();
    for (int i=0; i<number_of_shoes; i++) play_shoe();
    std::cout << "profit (in units of base bet): " << profit << std::endl;
    std::cout << "number of hands played: " << hand_counter << std::endl;
    std::cout << "profit per hand: " << profit/hand_counter << std::endl;

    return 0;
}
