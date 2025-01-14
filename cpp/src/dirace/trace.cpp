#include "dirace.hpp"

namespace dirace {
    using namespace std;
    using namespace ualg;

    string pos_to_string(const TermPos& pos) {
        if (pos.size() == 0) {
            return "()";
        }
        string res = "";
        for (int i = 0; i < pos.size(); ++i) {
            res += to_string(pos[i]);
            if (i != pos.size() - 1) {
                res += ", ";
            }
        }
        return "(" + res + ")";
    }

    std::map<PosRewritingRule, std::string> rule_name = {
        {R_COMPO_SS, "R_COMPO_SS"},
        {R_COMPO_SK, "R_COMPO_SK"},
        {R_COMPO_SB, "R_COMPO_SB"},
        {R_COMPO_SO, "R_COMPO_SO"},
        {R_COMPO_KS, "R_COMPO_KS"},
        {R_COMPO_KK, "R_COMPO_KK"},
        {R_COMPO_KB, "R_COMPO_KB"},
        {R_COMPO_BS, "R_COMPO_BS"},
        {R_COMPO_BK, "R_COMPO_BK"},
        {R_COMPO_BB, "R_COMPO_BB"},
        {R_COMPO_BO, "R_COMPO_BO"},
        {R_COMPO_OS, "R_COMPO_OS"},
        {R_COMPO_OK, "R_COMPO_OK"},
        {R_COMPO_OO, "R_COMPO_OO"},
        {R_COMPO_DD, "R_COMPO_DD"},
        {R_COMPO_ARROW, "R_COMPO_ARROW"},
        {R_COMPO_FORALL, "R_COMPO_FORALL"},
        {R_STAR_PROD, "R_STAR_PROD"},
        {R_STAR_MULS, "R_STAR_MULS"},
        {R_STAR_TSRO, "R_STAR_TSRO"},
        {R_STAR_CATPROD, "R_STAR_CATPROD"},
        {R_STAR_LTSR, "R_STAR_LTSR"},
        {R_ADDG_ADDS, "R_ADDG_ADDS"},
        {R_ADDG_ADD, "R_ADDG_ADD"},
        {R_SSUM, "R_SSUM"},
        {R_BETA_ARROW, "R_BETA_ARROW"},
        {R_BETA_INDEX, "R_BETA_INDEX"},
        {R_DELTA, "R_DELTA"},
        {R_FLATTEN, "R_FLATTEN"},
        {R_ADDSID, "R_ADDSID"},
        {R_MULSID, "R_MULSID"},
        {R_ADDS0, "R_ADDS0"},
        {R_MULS0, "R_MULS0"},
        {R_MULS1, "R_MULS1"},
        {R_MULS2, "R_MULS2"},
        {R_CONJ0, "R_CONJ0"},
        {R_CONJ1, "R_CONJ1"},
        {R_CONJ2, "R_CONJ2"},
        {R_CONJ3, "R_CONJ3"},
        {R_CONJ4, "R_CONJ4"},
        {R_CONJ5, "R_CONJ5"},
        {R_CONJ6, "R_CONJ6"},
        {R_DOT0, "R_DOT0"},
        {R_DOT1, "R_DOT1"},
        {R_DOT2, "R_DOT2"},
        {R_DOT3, "R_DOT3"},
        {R_DOT4, "R_DOT4"},
        {R_DOT5, "R_DOT5"},
        {R_DOT6, "R_DOT6"},
        {R_DOT7, "R_DOT7"},
        {R_DOT8, "R_DOT8"},
        {R_DOT9, "R_DOT9"},
        {R_DOT10, "R_DOT10"},
        {R_DOT11, "R_DOT11"},
        {R_DOT12, "R_DOT12"},
        {R_DELTA0, "R_DELTA0"},
        {R_DELTA1, "R_DELTA1"},
        {R_SCR0, "R_SCR0"},
        {R_SCR1, "R_SCR1"},
        {R_SCR2, "R_SCR2"},
        {R_SCRK0, "R_SCRK0"},
        {R_SCRK1, "R_SCRK1"},
        {R_SCRB0, "R_SCRB0"},
        {R_SCRB1, "R_SCRB1"},
        {R_SCRO0, "R_SCRO0"},
        {R_SCRO1, "R_SCRO1"},
        {R_ADDID, "R_ADDID"},
        {R_ADD0, "R_ADD0"},
        {R_ADD1, "R_ADD1"},
        {R_ADD2, "R_ADD2"},
        {R_ADD3, "R_ADD3"},
        {R_ADDK0, "R_ADDK0"},
        {R_ADDB0, "R_ADDB0"},
        {R_ADDO0, "R_ADDO0"},
        {R_ADJ0, "R_ADJ0"},
        {R_ADJ1, "R_ADJ1"},
        {R_ADJ2, "R_ADJ2"},
        {R_ADJ3, "R_ADJ3"},
        {R_ADJK0, "R_ADJK0"},
        {R_ADJK1, "R_ADJK1"},
        {R_ADJK2, "R_ADJK2"},
        {R_ADJB0, "R_ADJB0"},
        {R_ADJB1, "R_ADJB1"},
        {R_ADJB2, "R_ADJB2"},
        {R_ADJO0, "R_ADJO0"},
        {R_ADJO1, "R_ADJO1"},
        {R_ADJO2, "R_ADJO2"},
        {R_ADJO3, "R_ADJO3"},
        {R_TSR0, "R_TSR0"},
        {R_TSR1, "R_TSR1"},
        {R_TSR2, "R_TSR2"},
        {R_TSR3, "R_TSR3"},
        {R_TSRK0, "R_TSRK0"},
        {R_TSRK1, "R_TSRK1"},
        {R_TSRK2, "R_TSRK2"},
        {R_TSRB0, "R_TSRB0"},
        {R_TSRB1, "R_TSRB1"},
        {R_TSRB2, "R_TSRB2"},
        {R_TSRO0, "R_TSRO0"},
        {R_TSRO1, "R_TSRO1"},
        {R_TSRO2, "R_TSRO2"},
        {R_TSRO3, "R_TSRO3"},
        {R_MULK0, "R_MULK0"},
        {R_MULK1, "R_MULK1"},
        {R_MULK2, "R_MULK2"},
        {R_MULK3, "R_MULK3"},
        {R_MULK4, "R_MULK4"},
        {R_MULK5, "R_MULK5"},
        {R_MULK6, "R_MULK6"},
        {R_MULK7, "R_MULK7"},
        {R_MULK8, "R_MULK8"},
        {R_MULK9, "R_MULK9"},
        {R_MULK10, "R_MULK10"},
        {R_MULK11, "R_MULK11"},
        {R_MULB0, "R_MULB0"},
        {R_MULB1, "R_MULB1"},
        {R_MULB2, "R_MULB2"},
        {R_MULB3, "R_MULB3"},
        {R_MULB4, "R_MULB4"},
        {R_MULB5, "R_MULB5"},
        {R_MULB6, "R_MULB6"},
        {R_MULB7, "R_MULB7"},
        {R_MULB8, "R_MULB8"},
        {R_MULB9, "R_MULB9"},
        {R_MULB10, "R_MULB10"},
        {R_MULB11, "R_MULB11"},
        {R_OUTER0, "R_OUTER0"},
        {R_OUTER1, "R_OUTER1"},
        {R_OUTER2, "R_OUTER2"},
        {R_OUTER3, "R_OUTER3"},
        {R_OUTER4, "R_OUTER4"},
        {R_OUTER5, "R_OUTER5"},
        {R_MULO0, "R_MULO0"},
        {R_MULO1, "R_MULO1"},
        {R_MULO2, "R_MULO2"},
        {R_MULO3, "R_MULO3"},
        {R_MULO4, "R_MULO4"},
        {R_MULO5, "R_MULO5"},
        {R_MULO6, "R_MULO6"},
        {R_MULO7, "R_MULO7"},
        {R_MULO8, "R_MULO8"},
        {R_MULO9, "R_MULO9"},
        {R_MULO10, "R_MULO10"},
        {R_MULO11, "R_MULO11"},
        {R_MULO12, "R_MULO12"},
        {R_SET0, "R_SET0"},
        {R_SUM_CONST0, "R_SUM_CONST0"},
        {R_SUM_CONST1, "R_SUM_CONST1"},
        {R_SUM_CONST2, "R_SUM_CONST2"},
        {R_SUM_CONST3, "R_SUM_CONST3"},
        {R_SUM_CONST4, "R_SUM_CONST4"},
        {R_SUM_ELIM0, "R_SUM_ELIM0"},
        {R_SUM_ELIM1, "R_SUM_ELIM1"},
        {R_SUM_ELIM2, "R_SUM_ELIM2"},
        {R_SUM_ELIM3, "R_SUM_ELIM3"},
        {R_SUM_ELIM4, "R_SUM_ELIM4"},
        {R_SUM_ELIM5, "R_SUM_ELIM5"},
        {R_SUM_ELIM6, "R_SUM_ELIM6"},
        {R_SUM_ELIM7, "R_SUM_ELIM7"},
        // {R_SUM_ELIM8, "R_SUM_ELIM8"},
        {R_SUM_PUSH0, "R_SUM_PUSH0"},
        {R_SUM_PUSH1, "R_SUM_PUSH1"},
        {R_SUM_PUSH2, "R_SUM_PUSH2"},
        {R_SUM_PUSH3, "R_SUM_PUSH3"},
        {R_SUM_PUSH4, "R_SUM_PUSH4"},
        {R_SUM_PUSH5, "R_SUM_PUSH5"},
        {R_SUM_PUSH6, "R_SUM_PUSH6"},
        {R_SUM_PUSH7, "R_SUM_PUSH7"},
        {R_SUM_PUSH8, "R_SUM_PUSH8"},
        {R_SUM_PUSH9, "R_SUM_PUSH9"},
        {R_SUM_PUSH10, "R_SUM_PUSH10"},
        {R_SUM_PUSH11, "R_SUM_PUSH11"},
        {R_SUM_PUSH12, "R_SUM_PUSH12"},
        {R_SUM_PUSH13, "R_SUM_PUSH13"},
        {R_SUM_PUSH14, "R_SUM_PUSH14"},
        {R_SUM_PUSH15, "R_SUM_PUSH15"},
        {R_SUM_PUSH16, "R_SUM_PUSH16"},
        {R_SUM_ADDS0, "R_SUM_ADDS0"},
        {R_SUM_ADDS1, "R_SUM_ADDS1"},
        {R_SUM_ADD0, "R_SUM_ADD0"},
        {R_SUM_ADD1, "R_SUM_ADD1"},
        {R_SUM_INDEX0, "R_SUM_INDEX0"},
        {R_SUM_INDEX1, "R_SUM_INDEX1"},
        {R_SUM_FACTOR, "R_SUM_FACTOR"},
        
        {R_BIT_DELTA, "R_BIT_DELTA"},
        {R_BIT_ONEO, "R_BIT_ONEO"},
        {R_BIT_SUM, "R_BIT_SUM"},
        {R_LABEL_EXPAND, "R_LABEL_EXPAND"},
        {R_ADJD0, "R_ADJD0"},
        {R_ADJD1, "R_ADJD1"},
        {R_SCRD0, "R_SCRD0"},
        {R_SCRD1, "R_SCRD1"},
        {R_SCRD2, "R_SCRD2"},
        {R_TSRD0, "R_TSRD0"},
        {R_DOTD0, "R_DOTD0"},
        {R_DOTD1, "R_DOTD1"},
        {R_SUM_PUSHD0, "R_SUM_PUSHD0"},
        {R_SUM_PUSHD1, "R_SUM_PUSHD1"},
        {R_SUM_PUSHD2, "R_SUM_PUSHD2"},
        {R_L_SORT0, "R_L_SORT0"},
        {R_L_SORT1, "R_L_SORT1"},
        {R_L_SORT2, "R_L_SORT2"},
        {R_L_SORT3, "R_L_SORT3"},
        {R_L_SORT4, "R_L_SORT4"}
    };

    string record_to_string(Kernel& kernel, const PosReplaceRecord& record) {
        string res = "";
        res += "[Step]\t\t" + record.step + "\n";
        res += "[Position]\t" + pos_to_string(record.pos) + "\n";
        res += "[Initial Term]\t" + kernel.term_to_string(record.init_term) + "\n";

        // The matched term and replacement may be null
        if (record.matched_term != nullptr) {   
            res += "[Matched Term]\t" + kernel.term_to_string(record.matched_term) + "\n";
        }
        if (record.replacement != nullptr) {
            res += "[Replacement]\t" + kernel.term_to_string(record.replacement) + "\n";
        }

        res += "[Final Term]\t" + kernel.term_to_string(record.final_term) + "\n";
        return res;
    }

}; // namespace dirace