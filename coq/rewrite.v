From HB Require Import structures.
From mathcomp Require Import all_ssreflect all_algebra.

Import Order.TTheory GRing.Theory Num.Theory Num.Def.

Set   Implicit Arguments.
Unset Strict Implicit.
Unset Printing Implicit Defensive.
Unset SsrOldRewriteGoalsOrder.

Local Open Scope ring_scope.

Section Map2.

Variables (T1 T2 T : Type) (f : T1 -> T2 -> T).

Fixpoint map2 s1 s2 := 
  match s1, s2 with
  | h1 :: t1, h2 :: t2 => f h1 h2 :: (map2 t1 t2)
  | _, _ => [::]
  end.

Lemma map2_map s1 s2 : map2 s1 s2 = map (fun a => f a.1 a.2) (zip s1 s2).
Proof. by elim: s1 s2=>[[]//|a l IH [//|b s /=]]; rewrite IH. Qed.

End Map2.

Section scalar.
Variable (R : numClosedFieldType).

Inductive S_syn := 
  | S_zero
  | S_one
  | S_var (n : nat)
  | S_adds of (seq S_syn)
  | S_muls of (seq S_syn)
  | S_conj of S_syn.

Local Notation "0" := S_zero.
Local Notation "1" := S_one.
Notation "[ + ]" := (S_adds nil) (at level 0, format "[ + ]").
Notation "[ + x1 ]" := (S_adds [:: x1]) (at level 0, format "[ +  x1 ]").
Notation "[ + x1 ; x2 ; .. ; xn ]" := (S_adds (x1 :: x2 :: .. [:: xn] ..))
  (at level 0, format "[ + '['  x1 ; '/'  x2 ; '/'  .. ; '/'  xn ']' ]").
Notation "[ * ]" := (S_muls nil) (at level 0, format "[ * ]").
Notation "[ * x1 ]" := (S_muls [:: x1]) (at level 0, format "[ *  x1 ]").
Notation "[ * x1 ; x2 ; .. ; xn ]" := (S_muls (x1 :: x2 :: .. [:: xn] ..))
  (at level 0, format "[ * '['  x1 ; '/'  x2 ; '/'  .. ; '/'  xn ']' ]").

Fixpoint S_depth (s : S_syn) : nat :=
  match s with
  | 0 => 0%N 
  | 1 => 0%N 
  | S_var n => 0%N
  | S_adds s => (\sum_(i <- (map S_depth s)) i).+1
  | S_muls s => (\sum_(i <- (map S_depth s)) i).+1
  | S_conj s => (S_depth s).+1
  end.

Lemma S_ind s (P : S_syn -> Prop) :
  (P 0) -> (P 1) -> (forall n, P (S_var n)) ->
  (forall n, (forall s, (S_depth s <= n)%N -> P s) -> (forall s, S_depth s = n.+1 -> P s))
  -> P s.
Proof.
move=>P0 P1 Pvar IH.
set t := S_depth s.
have : (S_depth s <= t)%N by [].
move: t=>t; elim: t s; first by case.
move=>n Pn s.
rewrite leq_eqVlt=>/orP[/eqP|]; last by rewrite ltnS=>/Pn.
by apply/IH.
Qed.

Fixpoint eq_S_syn (s1 s2 : S_syn) :=
  match s1, s2 with
  | S_zero, S_zero => true
  | S_one, S_one => true
  | S_var n1, S_var n2 => (n1 == n2)
  | S_adds s1, S_adds s2 => (size s1 == size s2) && \big[andb/true]_(i <- map2 eq_S_syn s1 s2) i
  | S_muls s1, S_muls s2 => (size s1 == size s2) && \big[andb/true]_(i <- map2 eq_S_syn s1 s2) i
  | S_conj s1, S_conj s2 => eq_S_syn s1 s2
  | _, _ => false
  end.

Lemma S_syn_eqP : Equality.axiom eq_S_syn.
Proof.
move=>s1 s2; apply/(iffP idP).
  move: s2; apply (S_ind s1).
  - by case.
  - by case.
  - by move=>n; case=>//= m /eqP ->.
  - move=>n IH; case=>//=.
    - move=>t1 /eq_add_S Pt1; case=>// t2 /andP[] /eqP Pt.
      move: Pt1 => /eqP; rewrite map2_map !big_map eq_le=>/andP[] + _.
      elim: t1 t2 Pt=>//=[|a1 t1 IH1]; case=>//= a2 t2 /eq_add_S Pt.
      rewrite !big_cons/==> P /andP[] /(IH _ (leq_trans (leq_addr _ _) P))->.
      by move=>/(IH1 _  Pt (leq_trans (leq_addl _ _) P)) Pv; inversion Pv.
    - move=>t1 /eq_add_S Pt1; case=>// t2 /andP[] /eqP Pt.
      move: Pt1 => /eqP; rewrite map2_map !big_map eq_le=>/andP[] + _.
      elim: t1 t2 Pt=>//=[|a1 t1 IH1]; case=>//= a2 t2 /eq_add_S Pt.
      rewrite !big_cons/==> P /andP[] /(IH _ (leq_trans (leq_addr _ _) P))->.
      by move=>/(IH1 _  Pt (leq_trans (leq_addl _ _) P)) Pv; inversion Pv.
    - move=>s /eq_add_S /eqP; rewrite eq_le=>/andP[] /IH P1 _.
      by case=>// t /P1 ->.
move=>->; apply (S_ind s2)=>//= n IH; case=>//=.
- move=>l /eq_add_S /eqP; rewrite eq_le=>/andP[] + _.
  rewrite eqxx/= map2_map !big_map.
  elim: l=>/=[|a l IH1]; first by rewrite !big_nil.
  rewrite !big_cons/==>P; rewrite (IH _ (leq_trans (leq_addr _ _) P))/=.
  by rewrite  (IH1 (leq_trans (leq_addl _ _) P)).
- move=>l /eq_add_S /eqP; rewrite eq_le=>/andP[] + _.
  rewrite eqxx/= map2_map !big_map.
  elim: l=>/=[|a l IH1]; first by rewrite !big_nil.
  rewrite !big_cons/==>P; rewrite (IH _ (leq_trans (leq_addr _ _) P))/=.
  by rewrite  (IH1 (leq_trans (leq_addl _ _) P)).
- by move=>s /eq_add_S Pn; apply/IH; rewrite Pn.
Qed.

HB.instance Definition _ := hasDecEq.Build S_syn S_syn_eqP.

Inductive AC_symbol := | A_adds | A_muls.

Fixpoint R_flatten1_rec (a : AC_symbol) (s r : seq S_syn) :=
  match s with
  | [::] => r
  | h :: t => match a, h with
              | A_adds, S_adds s => R_flatten1_rec a t (r ++ s)
              | A_muls, S_muls s => R_flatten1_rec a t (r ++ s)
              | _, _ =>  R_flatten1_rec a t (rcons r h)
              end
  end.

Definition R_flatten1 a s := R_flatten1_rec a s [::].

Fixpoint R_flatten (s : S_syn) :=
  match s with
  | S_adds s => S_adds (R_flatten1 A_adds (map R_flatten s))
  | S_muls s => S_muls (R_flatten1 A_muls (map R_flatten s))
  | _ => s
  end.

(* Compute R_flatten [+ 0; [+ 0; 1; [* 0; 1; [* 1; [+ 0; 1]; 1]]; 1; [+ 0; 1]]]. *)

Definition R_addsid (s : S_syn) :=
  match s with
  | S_adds [:: s] => s
  | _ => s
  end.

Fixpoint remove_rec (a : S_syn) (s r : seq S_syn) :=
  match s with
  | [::] => r
  | h :: t => if eq_S_syn h a then remove_rec a t r
              else remove_rec a t (rcons r h)
  end.

Definition remove a (s : seq S_syn) := remove_rec a s [::].

Definition R_adds0 (s : S_syn) := 
  match s with
  | S_adds s => S_adds (remove 0 s)
  | _ => s
  end.

Definition R_mulsid (s : S_syn) :=
  match s with
  | S_muls [:: s] => s
  | _ => s
  end.

Fixpoint has_0 (s : seq S_syn) :=
  match s with
  | [::] => false
  | h :: t => match h with
              | 0 => true
              | _ => has_0 t
              end
  end.

Definition R_muls0 (s : S_syn) := 
  match s with
  | S_muls s => if has_0 s then 0 else S_muls s
  | _ => s
  end.

Definition R_muls1 (s : S_syn) := 
  match s with
  | S_muls s => S_muls (remove 1 s)
  | _ => s
  end.

Definition R_muls2 (s : S_syn) :=
  match s with
  | S_muls s => (fix rec (s1 s2 : seq S_syn) {struct s2} :=
                    match s2 with
                    | [::] => S_muls s1
                    | h :: t => match h with
                                | S_adds sa => S_adds (map (fun x => S_muls (s1 ++ x :: t)) sa)
                                | _ => rec (rcons s1 h) t
                                end
                    end
                ) [::] s
  | _ => s
  end.

(* Compute R_muls2 [* 1; 0; [+ 1; 0; 1]; 0; [+ 1; 0]; [* 1; 0]]. *)

Definition R_conj0 (s : S_syn) :=
  match s with
  | S_conj 0 => 0
  | _ => s
  end.

Definition R_conj1 (s : S_syn) :=
  match s with
  | S_conj 1 => 1
  | _ => s
  end.

Definition R_conj2 (s : S_syn) :=
  match s with
  | S_conj (S_adds s) => S_adds (map S_conj s)
  | _ => s
  end.

Definition R_conj3 (s : S_syn) :=
  match s with
  | S_conj (S_muls s) => S_muls (map S_conj s)
  | _ => s
  end.

Definition R_conj4 (s : S_syn) :=
  match s with
  | S_conj (S_conj s) => s
  | _ => s
  end.

(* Compute take 0 [:: 0; 1; 0; 1; 0; 1; 0; 1; 0].
Compute drop 0 [:: 0; 1; 0; 1; 0; 1; 0; 1; 0].
Compute nth 0 [:: 0; 1; 0; 1] 3. *)

Definition pos_sub (s : seq S_syn) (n : nat) (f : S_syn -> S_syn) :=
  if (n < size s)%nat then
    (take n s) ++ f (nth 0 s n) :: drop (n.+1) s
  else s.

Inductive rules :=
  | R_FLATTEN
  | R_ADDSID
  | R_ADDS0
  | R_MULSID
  | R_MULS0
  | R_MULS1
  | R_MULS2
  | R_CONJ0
  | R_CONJ1
  | R_CONJ2
  | R_CONJ3
  | R_CONJ4.

Definition rules_sem (r : rules) :=
  match r with
  | R_FLATTEN => R_flatten
  | R_ADDSID  => R_addsid
  | R_ADDS0   => R_adds0
  | R_MULSID  => R_mulsid
  | R_MULS0   => R_muls0
  | R_MULS1   => R_muls1
  | R_MULS2   => R_muls2
  | R_CONJ0   => R_conj0
  | R_CONJ1   => R_conj1
  | R_CONJ2   => R_conj2
  | R_CONJ3   => R_conj3
  | R_CONJ4   => R_conj4
  end.

Inductive position := 
  | P_all
  | P_ac & nat & position
  | P_unary & position.

Fixpoint R_apply (p : position) (f : rules) (s : S_syn) :=
  match p with
  | P_all => rules_sem f s
  | P_unary p => match s with 
                | S_conj s => S_conj (R_apply p f s)
                | _ => s
                end
  | P_ac n p => match s with
                | S_adds s => S_adds (pos_sub s n (R_apply p f))
                | S_muls s => S_muls (pos_sub s n (R_apply p f))
                | _ => s
                end
  end.

Fixpoint R_apply_seq (pr : seq (position * rules)) (s : S_syn) :=
  match pr with
  | [::] => s
  | (p, r) :: t => R_apply_seq t (R_apply p r s)
  end.

Notation a := (S_var 0%N).
Notation b := (S_var 1%N).
Notation c := (S_var 2%N).

(* Let A0 := [* a; [+ b; c]; b; 1; [+ a; b; 0]].
Compute A0.
Let A1 := R_apply P_all R_MULS1 A0.
Compute A1.
Let A2 := R_apply P_all R_MULS2 A1.
Compute A2.
Let A3 := R_apply (P_adds 0 P_all) R_MULS2 A2.
Compute A3.
Let A4 := R_apply P_all R_FLATTEN A3.
Compute A4.
Let A5 := R_apply (P_adds 2 P_all) R_MULS0 A4.
Compute A5.
Let A6 := R_apply P_all R_ADDS0 A5.
Compute A6.
Let A7 := R_apply (P_adds 2 P_all) R_MULS2 A6.
Compute A7.
Let A8 := R_apply P_all R_FLATTEN A7.
Compute A8.
Let A9 := R_apply (P_adds 4 P_all) R_MULS0 A8.
Compute A9.
Let A10 := R_apply P_all R_ADDS0 A9.
Compute A10. *)

Compute R_apply_seq 
  [:: (P_all, R_MULS1); 
      (P_all, R_MULS2);
      (P_ac 0 P_all, R_MULS2);
      (P_all, R_FLATTEN);
      (P_ac 2 P_all, R_MULS0);
      (P_all, R_ADDS0);
      (P_ac 2 P_all, R_MULS2);
      (P_all, R_FLATTEN);
      (P_ac 4 P_all, R_MULS0);
      (P_all, R_ADDS0)]
    [* a; [+ b; c]; b; 1; [+ a; b; 0]].

Section soundness.
Variable (context : nat -> R).

(* semantics of syntax *)
Fixpoint S_sem (s : S_syn) : R :=
  match s with
  | 0 => 0%R
  | 1 => 1%R
  | S_var n => context n
  | S_adds s => \sum_(i <- (map S_sem s)) i
  | S_muls s => \prod_(i <- (map S_sem s)) i
  | S_conj s => (S_sem s)^*
  end.

Notation "s1 =s s2" := (S_sem s1 = S_sem s2) (at level 70).

Lemma R_flatten1_rec_nil a s r :
  R_flatten1_rec a s r = r ++ (R_flatten1_rec a s [::]).
Proof.
elim: s r=>//=[ r | h l IH r]; first by rewrite cats0.
by case: a IH=> IH; case: h=>//=[||?|?|?|?]; rewrite IH [in RHS]IH catA// cats1.
Qed.

Lemma R_flatten_correct s : R_flatten s =s s.
Proof.
apply (S_ind s)=>//= n IH t.
case: t=>//= l /eq_add_S /eqP; rewrite !big_map eq_le=>/andP[] + _;
elim: l=>//= a l IH1; rewrite !big_cons=>H.
all: rewrite -IH1; first by (apply/(leq_trans _ H)/leq_addl).
all: rewrite -[S_sem a]IH; first by apply/(leq_trans _ H)/leq_addr.
all: rewrite /R_flatten1/=; case: (R_flatten a)=>/=[||?|?|?|?];
  by rewrite R_flatten1_rec_nil  big_cat/= ?big_seq1// big_map.
Qed.

Lemma R_addsid_correct s : R_addsid s =s s.
Proof.
case: s=>//= l; rewrite big_map.
case: l=>/=[|a]; first by rewrite !big_nil.
case=>//=[|b l]; first by rewrite big_seq1.
by rewrite !big_cons big_map.
Qed.

Lemma remove_rec_nil a s r :
  remove_rec a s r = r ++ (remove_rec a s [::]).
Proof.
elim: s r=>//=[ r | h l IH r]; first by rewrite cats0.
by case E: (eq_S_syn h a); rewrite IH// [in RHS]IH catA cats1.
Qed.

Lemma R_adds0_correct s : R_adds0 s =s s.
Proof.
case: s=>//= l; rewrite !big_map.
elim: l=>//= a l; rewrite /remove/=.
case E: (eq_S_syn a 0)=>//=; rewrite big_cons=><-;
rewrite remove_rec_nil big_cat/= ?big_seq1// big_nil.
by move: E=>/eqP->; rewrite /= !add0r.
Qed.

Lemma R_mulsid_correct s : R_mulsid s =s s.
Proof.
case: s=>//= l; rewrite big_map.
case: l=>/=[|a]; first by rewrite !big_nil.
case=>//=[|b l]; first by rewrite big_seq1.
by rewrite !big_cons big_map.
Qed.

Lemma R_muls0_correct s : R_muls0 s =s s.
Proof.
case: s=>//=; elim=>//= a l IH.
case: a=>//=[||?|?|?|?]; first by rewrite big_cons mul0r.
all: by move: IH; case: (has_0 l)=>//=; rewrite big_cons=><-; rewrite mulr0.
Qed.

Lemma R_muls1_correct s : R_muls1 s =s s.
Proof.
case: s=>//= l; rewrite !big_map.
elim: l=>//= a l; rewrite /remove/=.
case E: (eq_S_syn a 1)=>//=; rewrite big_cons=><-;
rewrite remove_rec_nil big_cat/= ?big_seq1// big_nil.
by move: E=>/eqP->; rewrite /= !mul1r.
Qed.

Lemma R_muls2_correct s : R_muls2 s =s s.
Proof.
case: s=>//=.
pose f := (fix rec (s1 s2 : seq S_syn) {struct s2} : S_syn :=
      match s2 with
      | [::] => S_muls s1
      | h :: t =>
          match h with
          | S_adds sa => S_adds [seq S_muls (s1 ++ x :: t) | x <- sa]
          | _ => rec (rcons s1 h) t
          end
      end).
rewrite -/f.
suff Pf s1 s2 : S_sem (f s1 s2) = (\prod_(i <- s1) S_sem i) * S_sem (f [::] s2).
  elim=>//= + l; case=>//=[||?|t|?|?].
  1-3,5-6: by rewrite big_cons=><-; rewrite Pf big_seq1.
  rewrite big_cons !big_map /==>P.
  by rewrite mulr_suml; apply eq_bigr=>i _; rewrite big_cons big_map.
elim: s2 s1=>//=.
  by move=>s1; rewrite big_map big_nil mulr1.
move=>a l IH s1; case: a=>[||?|?|?|?].
1,2,3,5,6: by rewrite IH [in RHS]IH mulrA -cats1 big_cat/=.
rewrite /= !big_map mulr_sumr; apply eq_bigr=>i _.
by rewrite /= big_cons !big_map big_cat/= big_cons.
Qed.

Lemma R_conj0_correct s : R_conj0 s =s s.
Proof. by case: s=>//= [[]]//=; rewrite conjC0. Qed.

Lemma R_conj1_correct s : R_conj1 s =s s.
Proof. by case: s=>//= [[]]//=; rewrite conjC1. Qed.

Lemma R_conj2_correct s : R_conj2 s =s s.
Proof. by case: s=>//= [[]]//= s; rewrite !big_map raddf_sum. Qed.

Lemma R_conj3_correct s : R_conj3 s =s s.
Proof. by case: s=>//= [[]]//= s; rewrite !big_map rmorph_prod. Qed.

Lemma R_conj4_correct s : R_conj4 s =s s.
Proof. by case: s=>// [[]]//= s; rewrite conjCK. Qed.

Lemma rules_correct r s : rules_sem r s =s s.
Proof.
case: r=>/=.
apply R_flatten_correct.
apply R_addsid_correct.
apply R_adds0_correct.
apply R_mulsid_correct.
apply R_muls0_correct.
apply R_muls1_correct.
apply R_muls2_correct.
apply R_conj0_correct.
apply R_conj1_correct.
apply R_conj2_correct.
apply R_conj3_correct.
apply R_conj4_correct.
Qed.

Lemma R_apply_correct p r s :
  R_apply p r s =s s.
Proof.
elim: p s.
- move=>s; apply/rules_correct.
- move=>n p IH []//= l; rewrite !big_map /pos_sub;
  case E: (n < size l)%N=>//; rewrite big_cat/= big_cons IH;
  (have {4} ->: l = take n l ++ (nth 0 l n) :: drop n.+1 l
    by rewrite -drop_nth// cat_take_drop);
  by rewrite big_cat/= big_cons.
- by move=>p IH /= []//= s; rewrite IH.
Qed.

Lemma R_apply_seq_correct pr s :
  s =s R_apply_seq pr s.
Proof. by elim: pr s =>//= [[p r]] l IH s; rewrite -IH R_apply_correct. Qed.

Goal [* a; [+ b; c]; b; 1; [+ a; b; 0]] =s 
  [+ [* a; b; b; a]; [* a; b; b; b]; [* a; c; b; a]; [* a; c; b; b]].
Proof.
pose pr :=   [:: (P_all, R_MULS1); 
      (P_all, R_MULS2);
      (P_ac 0 P_all, R_MULS2);
      (P_all, R_FLATTEN);
      (P_ac 2 P_all, R_MULS0);
      (P_all, R_ADDS0);
      (P_ac 2 P_all, R_MULS2);
      (P_all, R_FLATTEN);
      (P_ac 4 P_all, R_MULS0);
      (P_all, R_ADDS0)].
by rewrite (R_apply_seq_correct pr).
Qed.

End soundness.
End scalar.


(*
=====================================
(rule + position)
1. R_MLTS1 all
2. R_MLTS2 all
3. R_MLTS2 [+ 0]
4. R_FLATTEN all
5. R_MLTS0 [+ 2]
6. R_ADDS0 all
7. R_MLTS2 [+ 2]
8. R_FLATTEN all
9. R_MLTS0 [+ 4]
10. R_ADDS0 all

Initial term:

[* a [+ b c] b 1 [+ a b 0]]

1. R_MLTS1 all

[* a [+ b c] b [+ a b 0]]

2. R_MLTS2 all

[+ [* a b b [+ a b 0]] [* a c b [+ a b 0]] ]

3. R_MLTS2 [+ 0]

[+ [+ [* a b b a] [* a b b b] [* a b b 0]] [* a c b [+ a b 0]] ]

4. R_FLATTEN all

[+ [* a b b a] [* a b b b] [* a b b 0] [* a c b [+ a b 0]] ]

5. R_MLTS0 [+ 2]

[+ [* a b b a] [* a b b b] 0 [* a c b [+ a b 0]] ]

6. R_ADDS0 all

[+ [* a b b a] [* a b b b] [* a c b [+ a b 0]] ]

7. R_MLTS2 [+ 2]

[+ [* a b b a] [* a b b b] [+ [* a c b a] [* a c b b] [* a c b 0]] ]

8. R_FLATTEN all

[+ [* a b b a] [* a b b b] [* a c b a] [* a c b b] [* a c b 0] ]

9. R_MLTS0 [+ 4]

[+ [* a b b a] [* a b b b] [* a c b a] [* a c b b] 0 ]

10. R_ADDS0 all

[+ [* a b b a] [* a b b b] [* a c b a] [* a c b b] ]


Normal form

[+ [* b b b a]; [* b b c a] ; [* b b a a]; [* b c a a] ]

ADDS(MLTS(b b b a) MLTS(b b c a) MLTS(b b a a) MLTS(b c a a))



R_FLATTEN ADDS(MLTS(a b b a) MLTS(a b b b) ADDS(MLTS(a c b a) MLTS(a c b b) MLTS(a c b 0))).
R_MLTS0 MLTS(a c b 0).
R_ADDS0 ADDS(MLTS(a b b a) MLTS(a b b b) MLTS(a c b a) MLTS(a c b b) 0).
R_C_EQ [1:[1:E 2:E 3:E 0:E] 3:[2:E 3:E 1:E 0:E] 0:[1:E 2:E 0:E 3:E] 2:[2:E 1:E 0:E 3:E]].

这是中间语言的一个例子

*)