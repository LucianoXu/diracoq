grammar DIRACOQ;


// Lexer rule for multi-line comments
COMMENT: '(*' .*? '*)' -> skip;

expr:   (cmd)+                        # CmdSeq
    |   term                       # FromTerm
    ;

// Parser rules
cmd :   'Def' ID ':=' expr '.'         # Definition0
    |   'Def' ID ':=' expr ':' expr '.'   # Definition1
    |   'Var' ID ':' expr '.'         # Assum
    |   'Check' term '.'               # Check
    |   'Show' ID '.'               # Show
    |   'ShowAll' '.'               # ShowAll
    |   'Normalize' expr '.'         # Normalize
    |   'Normalize' expr 'with' 'trace' '.'         # NormalizeTraced
    |   'CheckEq' expr 'with' expr '.'      # CheckEq
    ;

term:   ID '[' expr (',' expr)* ']'                # Application

    |   '<' term '|'                           # Bra
    |   '|' term '>'                           # Ket
    |   'delta' '(' term ',' term ')'          # Delta
    |   '(' term ',' term ')'                  # Pair
    |   term '.' term                          # Scr
    |   term '^D'                              # Adj
    |   term '^*'                              # Conj
    |   <assoc=left> term '*' term             # Star
    |   <assoc=left> term term             # Compo
    |   <assoc=left> term '+' term             # Add
    |   <assoc=right> term '->' term           # Arrow
    |   'Sum' ID ':' term 'in 'term ',' term   # Sum
    |   'idx' ID '=>' term                     # Idx
    |   'fun' ID ':' term '=>' term            # Fun
    |   'forall' ID '.' term                   # Forall

    |   '(' term ')'                           # Paren
    |   ID                                     # Identifier
    ;

// Lexer rules
ID  :   [a-zA-Z0-9_] [a-zA-Z0-9_]* ;       // standard identifier
WS  :   [ \t\r\n]+ -> skip ;  // Skip whitespace (spaces, tabs, newlines)