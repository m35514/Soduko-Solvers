#include "../src/solver1.hpp"
#include "../src/solver2.hpp"
#include "../src/solver3.hpp"
#include "../src/solver4.hpp"
#include "../src/solver5.hpp"
#include "../src/arraySolver3.hpp"
#include "../src/arraySolver4.hpp"
#include "../src/arraySolver5.hpp"
#include <fstream>
#include <iostream>
#include <string>

bool valid(const Board& b, const Board& given) {
    for (int r=0;r<9;++r) {
        int rm=0,cm=0;
        for (int c=0;c<9;++c) {
            int v=b.cells[r][c],w=b.cells[c][r];
            if(v<1||v>9||w<1||w>9) return false;
            if(given.cells[r][c]&&given.cells[r][c]!=v) return false;
            rm|=1<<v;cm|=1<<w;
        }
        if(rm!=1022||cm!=1022) return false;
    }
    for(int box=0;box<9;++box) {
        int m=0;
        for(int i=0;i<9;++i) m|=1<<b.cells[box/3*3+i/3][box%3*3+i%3];
        if(m!=1022) return false;
    }
    return true;
}
SolveResult run(const Board& b,int s,bool trace) {
    switch(s) {
        case 0:return NaiveSolver::solve(b,trace);
        case 1:return BitmaskSolver::solve(b,trace);
        case 2:return Solver3::solve(b,trace);
        case 3:return Solver4::solve(b,trace);
        case 4:return Solver5::solve(b,trace);
        case 5:return ArraySolver3::solve(b,trace);
        case 6:return ArraySolver4::solve(b,trace);
        default:return ArraySolver5::solve(b,trace);
    }
}
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    std::ifstream in(argv[1]);
    if(!in) return 2;
    const char* names[]={"Naive","Bitmask","Solver3_bitmask","Solver4_bitmask","Solver5_bitmask","Solver3_array","Solver4_array","Solver5_array"};
    std::cout<<"case\tsolver\tsolved\tvalid\tsame_solution\ttrace_parity\tnodes\ttrace_guesses\tnaked_pair_events\txwing_events\ttrace_events\n";
    std::string line;
    int failures=0;
    while(std::getline(in,line)) {
        auto tab=line.find('\t');
        if(tab==std::string::npos) return 2;
        std::string label=line.substr(0,tab),p=line.substr(tab+1);
        if(p.size()!=81) return 2;
        Board b=Board::fromString(p);
        std::string canonical;
        for(int s=0;s<8;++s) {
            auto plain=run(b,s,false),trace=run(b,s,true);
            if(s==0) canonical=plain.board.toString();
            std::uint64_t guesses=0,np=0,xw=0;
            for(const auto& e:trace.trace) {
                guesses+=e.isGuess;
                np+=e.technique==TechniqueType::NakedPair;
                xw+=e.technique==TechniqueType::XWingRow||e.technique==TechniqueType::XWingCol;
            }
            bool v=plain.solved&&valid(plain.board,b);
            bool same=plain.board.toString()==canonical;
            bool parity=trace.solved==plain.solved&&trace.board.toString()==canonical&&trace.nodesExplored==plain.nodesExplored&&guesses==plain.nodesExplored;
            if(label=="known_fixture") same=same&&canonical=="534678912672195348198342567859761423426853791713924856961537284287419635345286179";
            failures+=!v||!same||!parity;
            std::cout<<label<<'\t'<<names[s]<<'\t'<<plain.solved<<'\t'<<v<<'\t'<<same<<'\t'<<parity<<'\t'<<plain.nodesExplored<<'\t'<<guesses<<'\t'<<np<<'\t'<<xw<<'\t'<<trace.trace.size()<<'\n';
        }
    }
    return failures?1:0;
}
