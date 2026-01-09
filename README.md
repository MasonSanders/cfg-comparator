# CFG Comparator

## What is it?

The CFG Comparator is a small application that uses heuristics to attempt to find counter examples between two user-given context-free grammars (CFGs) that may look similar. Proving whether two context free grammars are equivalent is **undecidable**, therefore, the best approach is to try and estimate equivalence by finding strings of terminals that are accepted by one grammar, but not accepted by the other.

## How does it work?

The CFG Comparator works by first converting both grammars into **Chomsky Normal Form** (CNF). This means that A grammar rule can either produce two nonterminals or a single terminal. An epsilon production is allowed for the start symbol if the start symbol of the original grammar is nullable. CNF is important because it allows for the use of the **Cocke-Younger-Kasami** algorithm (CYK), which uses bottom-up parsing to determine if a given string of terminals is accepted by a grammar.

The program uses heuristics in the form of **grammar-guided generation** to generate a large number of strings using random derivations of a grammar and then using CYK to check whether it is accepted by the other grammar. This is done both ways to ensure that it's not just checking whether one grammar is a subset of the other. A search budget is used to ensure efficient generation of strings through random derivations. A limit is set on the number of derivation steps, string length, and number of trials to avoid getting stuck in extremely long derivations, as some grammars could theoritcally produce infinitely long strings.

If the program finds a string that is accepted by one grammar but not the other, then the grammars are not equal. However, if it is unable to find a counterexample, that does not necessarily mean the grammars are equal, only that the program failed to find a counterexample.

## How to use the program?

### Downloading and compiling

To use the CFG Comparator, you need to have the g++ compiler with C++23 as well as make. I developed the program on Manjaro Linux 26.0.0, but any operating system should be fine as long as you have both make and g++.

Start by downloading the zip and extracting it, or by cloning this repository. Open a terminal and navigate to the directory containing the source code and run the following command:

`make all`

### Running the program

To run the program, you'll also need at least a pair of grammars to input into the program. I've included 10 pairs of grammars to test. These are the .txt files labeled test1_1, test1_2, test2_1, test2_2, etc. The other .txt files can be ignored as they were used simply to test correct conversion into CNF. You can also create your own grammar files. Please note that if you choose to create your own grammar files, each grammar must be a separate file, and they must follow the syntax of the grammars that I have defined later in this readme. You can run the program by running the command as follows:

`./cfg_comparator <filename1> <filename2>`

Here is a quick example using the test1 pair of grammars:

`./cfg_comparator test1_1.txt test1_2.txt`

### Creating your own grammar files

Creating your own grammars to test is easy, but I am assuming you have some prior knowledge of how context-free grammars work and how to read them. The syntax/meta grammar for writing CFGs for the program is as follows:

```
grammar     -> ruleList END_OF_FILE
ruleList    -> rule | rule ruleList
rule        -> ID ARROW rhs SEMICOLON
rhs         -> alternative | alternative OR rhs
alternative -> symbolList | EPSILON
symbolList  -> symbol | symbol symbolList
symbol      -> ID | STRING
```

In plain english, CFGs written for the program have a list of rules, where each rule is terminated by a semicolon. Each rule has a left-hand side, which is a single nonterminal, followed by an arrow `->` and then a right-hand side. The right hand side is a list of alternative productions separated by `|`, and each alternative is either the word `epsilon` or a list of symbols. These symbols can be terminals, which are strings of any character surrounded by quotes, or nonterminals which are sequences of alphanumeric characters and underscores that are not the word 'epsilon'. Nonterminals are not surrounded by quotes. The following is an example of a valid CFG that can be used by the program:

```
S -> "a" A "b" | C | epsilon;
A -> "a" A "c" | "b" B;
B -> "b" B | epsilon;
C -> "c" C | epsilon;
```

Note that in the example, as well as in all of the test files, the nonterminals are lone capital letters, but they don't have to be. They can be any length sequence of alphanumeric characters as long as they aren't the word epsilon. The start symbol also does not have to be S.

# References
The following sources were used to assist with the development of this project:

[1] D. H. Younger, "Recognition and parsing of context-free languages in time n^3", Information and Control, Vol. 10, no. 2 pp. 189-208, 1967.

[2] D. Jurafsky and J. H. Marting, Speech and Language Processing, 3rd ed. draft. Upper saddle River, NJ, USA: Prentice Hall, 2023.

[3] B. McKenzie, "Generating strings at random from a context-free grammar", Journal of Functional Programming, vol. 7, no. 4, pp. 373-386, 1997.

[4] T. Hickey and J. Cohen, "Uniform random generation of strings in a context free language", SIAM Journal on Computing, vol. 12, no. 4, pp. 645-655, Nov. 1983.
