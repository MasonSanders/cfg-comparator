#include <fstream>
#include <utility>
#include <iostream>
#include <string>
#include <iterator>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include "parser.h"
#include "rule.h"
#include "cyk.h"

bool isUnitProduction(const std::vector<Symbol>& prod)
{
	return prod.size() == 1 && !prod[0].isTerminal;
}

bool isEpsilonProduction(const std::vector<Symbol>& prod)
{
	return prod.size() == 1 && prod[0].isTerminal && prod[0].name == "epsilon";
}

std::string altKey(const std::vector<Symbol>& alt)
{
	std::string k;
	k.reserve(alt.size() * 8);
	for (const auto& s : alt)
	{
		k += (s.isTerminal ? "T:" : "N:");
		k += s.name;
		k += "|";
	}
	return k;
}

void rebuildSymbolSets(Grammar& g)
{
	g.terminals.clear();
	g.nonterminals.clear();

	for (const auto& rule : g.rules)
	{
		g.nonterminals.insert(rule.lhs);

		for (const auto& prod : rule.rhs)
		{
			for (const auto& symbol : prod)
			{
				if (symbol.isTerminal)
				{
					if (symbol.name != "epsilon")
						g.terminals.insert(symbol.name);
				}
				else
					g.nonterminals.insert(symbol.name);
			}
		}
	}
}

std::unordered_map<std::string, size_t> buildRuleIndex(const Grammar& g)
{
	std::unordered_map<std::string, size_t> idx;

	for (size_t i = 0; i < g.rules.size(); ++i)
	{
		idx[g.rules[i].lhs] = i;
	}
	return idx;
}


std::string prodKey(const std::vector<Symbol>& prod)
{
	std::ostringstream oss;
	for (auto& s : prod)
	{
		oss << (s.isTerminal ? "T:" : "N:") << s.name << "|"; 
	}

	return oss.str();

}

void buildInitialNullables(const Grammar& g, std::unordered_set<std::string>& nullable)
{
	for (auto& r : g.rules)
	{
		for (auto& prod : r.rhs)
		{
			if (prod.size() == 1 && prod[0].name == "epsilon")
			{
				nullable.insert(r.lhs);
			}
		}	
	}  
}

// breadth first search to compute unit closure
std::unordered_set<std::string> unitClosure(
	const Grammar& g,
	const std::unordered_map<std::string, size_t>& idx,
	const std::string& start)
{
	std::unordered_set<std::string> seen;
	std::vector<std::string> q;
	seen.insert(start);
	q.push_back(start);

	while (!q.empty())
	{
		std::string a = q.back();
		q.pop_back();

		auto it = idx.find(a);
		if (it == idx.end())
			continue;

		const Rule& r = g.rules[it->second];
		for (const auto& prod : r.rhs)
		{
			if (isUnitProduction(prod))
			{
				const std::string& b = prod[0].name;
				if (seen.insert(b).second)
					q.push_back(b);
			}
		}
	}

	return seen;
}


void removeUnitProductions(Grammar& g)
{
	auto idx = buildRuleIndex(g);
	std::unordered_map<std::string, std::unordered_set<std::string>> closureMap;
	closureMap.reserve(g.rules.size());

	for (const auto& r : g.rules)
	{
		closureMap[r.lhs] = unitClosure(g, idx, r.lhs);
	}

	for (auto& rA : g.rules)
	{
		std::vector<std::vector<Symbol>> newRhs;
		std::unordered_set<std::string> seenAlt;

		for (const auto& B : closureMap[rA.lhs])
		{
			auto it = idx.find(B);
			if (it == idx.end())
				continue;

			const Rule& rB = g.rules[it->second];

			for (const auto& prod : rB.rhs)
			{
				if (isUnitProduction(prod))
					continue;

				std::string key = altKey(prod);
				if (seenAlt.insert(key).second)
					newRhs.push_back(prod);
			}
		}
		rA.rhs = std::move(newRhs);
	}
}

std::unordered_set<std::string> calcNullableSet(const Grammar& g)
{
	std::unordered_set<std::string> nullable;
	
	buildInitialNullables(g, nullable);

	bool changed = true;
	while (changed) 
	{
		changed = false;
		for (auto& r : g.rules) 
		{
			if (nullable.find(r.lhs) == nullable.end()) 
			{
				for (auto& prod : r.rhs) 
				{
					bool allNullable = true;
					for (auto& symbol : prod)
					{
						if (symbol.isTerminal && symbol.name != "epsilon")
						{
							allNullable = false;
							break;
						}
						else if (symbol.name == "epsilon")
						{
							std::cerr << "epsilon production should appear by itself" << std::endl;
							exit(1);
						}

						if (nullable.find(symbol.name) == nullable.end())
						{
							allNullable = false;
							break;
						}
					}

					if (allNullable && nullable.find(r.lhs) == nullable.end())
					{
						nullable.insert(r.lhs);
						changed = true;
						break;
					}
				}
			}	
		}
	}

	return nullable;
}

std::string addFreshStartSymbol(Grammar& g, const std::string& oldStart)
{
	auto freshStartName = [](const Grammar& g, const std::string& base = "S0") -> std::string
	{
		if (g.nonterminals.find(base) == g.nonterminals.end())
			return base;
		for (int i = 1; ; ++i)
		{
			std::string candidate = base + "_" + std::to_string(i);
			if (g.nonterminals.find(candidate) == g.nonterminals.end())
				return candidate; 
		}
	};

	auto newStart = freshStartName(g, "S0");
	
	Rule r;
	
	r.lhs = newStart;

	Symbol s;
	s.isTerminal = false;
	s.name = oldStart;

	r.rhs.push_back(std::vector<Symbol>{s});
	g.rules.insert(g.rules.begin(), r);
	g.nonterminals.insert(newStart);

	return newStart;
}

bool startDerivesEpsilon(const std::unordered_set<std::string>& nullable, const std::string& startSymbol)
{
	return nullable.find(startSymbol) != nullable.end();
}

void removeEpsilonProductions(Grammar& g, const std::string& startSymbol)
{
	// calculate the nullable set
	std::unordered_set nullable = calcNullableSet(g);
	bool keepStartEpsilon = startDerivesEpsilon(nullable, startSymbol);

	for (auto& rule : g.rules)
	{
		// declare a set to build the new alts
		std::vector<std::vector<Symbol>> newAlts;
		std::unordered_set<std::string> seen; // keep track of what's already been seen

		for (auto& prod : rule.rhs)
		{
			// skip explicit epsilon productions for now
			if (prod.size() == 1 && prod[0].name == "epsilon")
				continue;
			
			// epsilon should not appear mixed with other symbols in the production
			for (auto& symbol : prod)
			{
				if (symbol.name == "epsilon")
				{
					std::cerr << "Epsilon symbol appeard in non-epsilon production" << std::endl;
					exit(1);
				}
			}

			// find nullable non-terminal positions
			std::vector<size_t> nullablePositions;

			for (size_t i = 0; i < prod.size(); ++i)
			{
				const auto& symbol = prod[i];
				if (!symbol.isTerminal && nullable.find(symbol.name) != nullable.end())
					nullablePositions.push_back(i);
			}

			// include the original production
			if (seen.insert(prodKey(prod)).second)
				newAlts.push_back(prod);
			
			// generate productions by deleting any subset of nullable positions
			// iterate masks 1..(2^m - 1)
			const size_t m = nullablePositions.size();
			if (m >= sizeof(size_t) * 8)
			{
				std::cerr << "Too many nullable symbols for bitmask\n";
				exit(1);
			}
			const size_t totalMasks = 1 << m;

			for (size_t mask = 1; mask < totalMasks; ++mask)
			{
				std::vector<Symbol> candidate;
				candidate.reserve(prod.size());
				for (size_t i = 0; i < prod.size(); ++i)
				{
					bool deleteThis = false;
					// check if i is among nullablePositions selected by mask
					for (size_t j = 0; j < m; ++j)
					{
						if (nullablePositions[j] == i && (mask & (size_t{1} << j)))
						{
							deleteThis = true;
							break;
						}
					}
					if (!deleteThis) candidate.push_back(prod[i]);
				}
				
				// if we delete everything, this is epsilon
				if (candidate.empty())
				{
					if (keepStartEpsilon && rule.lhs == startSymbol)
					{
						std::vector<Symbol> eps{Symbol{true, "epsilon"}};
						if (seen.insert(prodKey(eps)).second)
							newAlts.push_back(eps);
					}
					continue;
				}
				
				if (seen.insert(prodKey(candidate)).second)
					newAlts.push_back(candidate);
			}
		}

		// if rule is start symbol, keep epsilon
		if (keepStartEpsilon && rule.lhs == startSymbol)
		{
			std::vector<Symbol> eps{Symbol{true, "epsilon"}};
			std::string key = prodKey(eps);
			bool exists = false;
			for (auto& alt : newAlts)
			{
				if (prodKey(alt) == key)
				{
					exists = true;
					break;
				}
			}
			if (!exists) 
				newAlts.push_back(eps);
		}

		// replace rhs with newAlts
		rule.rhs = std::move(newAlts);
	} 
}



std::unordered_set<std::string> computeGenerating(const Grammar& g)
{
	std::unordered_set<std::string> GEN;
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (const Rule& r : g.rules)
		{
			for (const auto& prod : r.rhs)
			{
				bool ok = true;

				if (prod.size() == 1 && prod[0].isTerminal && prod[0].name == "epsilon")
				{
					ok = true;
				}
				else
				{
					for (const Symbol& s : prod)
					{
						if (s.isTerminal)
							continue;

						if (!GEN.count(s.name))
						{
							ok = false;
							break;
						}
					}
				}
				if (ok)
				{
					if (GEN.insert(r.lhs).second)
						changed = true;
					break;
				}
			}
		}
	}
	return GEN;
}

void removeNonGenerating(Grammar& g, const std::unordered_set<std::string>& GEN)
{
	std::vector<Rule> newRules;
	for (const Rule& r : g.rules)
	{
		if (!GEN.count(r.lhs))
			continue;

		Rule nr;
		nr.lhs = r.lhs;

		for (const auto& prod : r.rhs)
		{
			bool ok = true;
			if (!(prod.size() == 1 && prod[0].isTerminal && prod[0].name == "epsilon"))
			{
				for (const Symbol& s : prod)
				{
					if (!s.isTerminal && !GEN.count(s.name))
					{
						ok = false;
						break;
					}
				}
			}
			if (ok)
				nr.rhs.push_back(prod);
		}

		if (!nr.rhs.empty())
			newRules.push_back(std::move(nr));
	}

	g.rules = std::move(newRules);
}

std::unordered_set<std::string> computeReachable(const Grammar& g, const std::string& start)
{
	std::unordered_set<std::string> REACH;
	std::vector<std::string> stack;

	REACH.insert(start);
	stack.push_back(start);

	std::unordered_map<std::string, const Rule*> idx;
	for (const Rule& r : g.rules)
	{
		idx[r.lhs] = &r;
	}

	while (!stack.empty())
	{
		std::string A = stack.back();
		stack.pop_back();
		auto it = idx.find(A);
		if (it == idx.end())
			continue;

		for (const auto& prod : it->second->rhs)
		{
			for (const Symbol& s : prod)
			{
				if (!s.isTerminal && REACH.insert(s.name).second)
					stack.push_back(s.name);
			}
		}
	}

	return REACH;
}

void removeUnreachable(Grammar& g, const std::unordered_set<std::string>& REACH)
{
	std::vector<Rule> newRules;
	for (const Rule& r : g.rules)
	{
		if (!REACH.count(r.lhs))
			continue;

		Rule nr;
		nr.lhs = r.lhs;

		for (const auto& prod : r.rhs)
		{
			bool ok = true;
			for (const Symbol& s : prod)
			{
				if (!s.isTerminal && !REACH.count(s.name))
				{
					ok = false;
					break;
				}
			}
			if (ok)
				nr.rhs.push_back(prod);
		}

		if (!nr.rhs.empty())
			newRules.push_back(std::move(nr));
	}

	g.rules = std::move(newRules);
}

void removeUselessSymbols(Grammar& g, const std::string& startSymbol)
{
	auto GEN = computeGenerating(g);
	removeNonGenerating(g, GEN);

	auto REACH = computeReachable(g, startSymbol);
	removeUnreachable(g, REACH);

	rebuildSymbolSets(g);
}

void printSymbols(const Grammar& g)
{
	std::cout << "Nonterminals:\n";
	for (const auto& nt : g.nonterminals)
		std::cout << " " << nt << "\n";

	std::cout << "Terminals:\n";
	for (const auto& t : g.terminals)
		std::cout << " " << t << "\n";
}

std::string makeFreshNonterminal(const Grammar& g, const std::string& base)
{
	if (g.nonterminals.find(base) == g.nonterminals.end())
		return base;

	for (int i = 1; ; ++i)
	{
		std::string cand = base + "_" + std::to_string(i);
		if (g.nonterminals.find(cand) == g.nonterminals.end())
			return cand;
	}
}

std::string sanitize(const std::string& t)
{
	std::string out;
	for (unsigned char c : t)
	{
		if (std::isalnum(c))
			out.push_back((char)c);
		else
			out.push_back('_');
	}

	if (out.empty())
		out = "tok";
	return out;
}


void eliminateTerminalsFromLong(Grammar& g)
{
	std::unordered_map<std::string, std::string> termToNT;

	std::vector<Rule> newRules;

	for (auto& r : g.rules)
	{
		for (auto& prod : r.rhs)
		{
			if (isEpsilonProduction(prod))
				continue;

			if (prod.size() < 2)
				continue;

			for (auto& symbol : prod)
			{
				if (!symbol.isTerminal)
					continue;

				if (symbol.name == "epsilon")
				{
					std::cerr << "epsilon appears in a long RHS production" << std::endl;
					exit(1);
				}

				auto it = termToNT.find(symbol.name);
				if (it == termToNT.end())
				{
					std::string base = "T_" + sanitize(symbol.name);
					std::string helper = makeFreshNonterminal(g, base);
					termToNT[symbol.name] = helper;


					Rule tr;
					tr.lhs = helper;
					Symbol termSym;
					termSym.isTerminal = true;
					termSym.name = symbol.name;

					tr.rhs.push_back(std::vector<Symbol>{ termSym });
					newRules.push_back(tr);

					g.nonterminals.insert(helper);
					it = termToNT.find(symbol.name);
				}

				symbol.isTerminal = false;
				symbol.name = it->second;
			}
		}
	}

	for (auto& nr : newRules)
		g.rules.push_back(std::move(nr));

	rebuildSymbolSets(g);
}


void binarizeRules(Grammar& g)
{
	std::vector<Rule> extraRules;
	extraRules.reserve(64);

	for (auto& r : g.rules)
	{
		std::vector<std::vector<Symbol>> newRhs;
		newRhs.reserve(r.rhs.size());

		for (const auto& prod : r.rhs)
		{
			if (prod.size() <= 2)
			{
				newRhs.push_back(prod);
				continue;
			}

			Symbol first = prod[0];
			std::string prevHelper = makeFreshNonterminal(g, "X");

			g.nonterminals.insert(prevHelper);

			newRhs.push_back(std::vector<Symbol>{ first, Symbol{ false, prevHelper } });

			for (size_t i = 1; i < prod.size(); ++i)
			{
				if (i == prod.size() - 2)
				{
					Rule rr;
					rr.lhs = prevHelper;
					rr.rhs.push_back(std::vector<Symbol>{ prod[i], prod[i + 1] });
					extraRules.push_back(std::move(rr));
					break;
				}
				else
				{
					std::string nextHelper = makeFreshNonterminal(g, "X");
					g.nonterminals.insert(nextHelper);

					Rule rr;
					rr.lhs = prevHelper;
					rr.rhs.push_back(std::vector<Symbol>{ prod[i], Symbol{ false, nextHelper }});
					extraRules.push_back(std::move(rr));

					prevHelper = nextHelper;
				}
			}
		}

		r.rhs = std::move(newRhs);
	}

	for (auto& rr : extraRules)
	{
		g.rules.push_back(std::move(rr));
	}

	rebuildSymbolSets(g);
}

// function to convert a grammar to chomsky normal form
Grammar CNF(Grammar& g) 
{
	std::string start = g.rules[0].lhs;
	addFreshStartSymbol(g, start);

	start = g.rules[0].lhs;
	removeEpsilonProductions(g, start);
	removeUnitProductions(g);

	start = g.rules[0].lhs;
	removeUselessSymbols(g, start);

	eliminateTerminalsFromLong(g);
	binarizeRules(g);

	return g;
}


void printGrammar(const Grammar& g)
{
	for (const auto& rule : g.rules)
	{
		std::cout << rule.lhs << " -> ";

		for (size_t i = 0; i < rule.rhs.size(); ++i)
		{
			for (const auto& symbol : rule.rhs[i])
			{
				if (symbol.isTerminal && symbol.name != "epsilon")
					std::cout << "\"" << symbol.name << "\" ";
				else
					std::cout << symbol.name << " ";
			}
			if (i != rule.rhs.size() - 1)
				std::cout << "| ";
			else
				std::cout << ";\n";
		}
	}
}


void testGrammars(const Grammar& g1, const Grammar& g2)
{
	std::cout << "Building CYK index for grammar 1...\n";
	CykIndex idx1 = buildCykIndex(g1);
	std::cout << "Index for grammar 1 built successfully!\n";
	std::cout << "Building CYK index for grammar 2...\n";
	CykIndex idx2 = buildCykIndex(g2);
	std::cout << "Index for grammar 2 built successfully!\n";

	GenSettings cfg;
	cfg.maxSteps = 200;
	cfg.maxLen = 40;
	cfg.targetMin = 1;
	cfg.targetMax = 20;

	std::cout << "Attempting to find equivalence counterexamples...\n";
	auto res = findCounterExample(g1, g1.rules[0].lhs, idx1,
								 g2, g2.rules[0].lhs, idx2,
								 5000, 1874592, cfg);

	if (res.found)
	{
		std::cout << "Grammars are NOT equivalent.\n";
		std::cout << "Witness: " << res.witness << "\n";
		std::cout << "G1 accepts: " << res.g1Accepts << "\n";
		std::cout << "G2 accepts: " << res.g2Accepts << "\n";
	}
	else
	{
		std::cout << "No counterexample found in budget.\n";
	}
}


int main(int argc, char* argv[])
{
	// get the inputs
	

	// error if user puts the incorrect number of args
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <input filename 1> <input filename 2>" << std::endl;
		return 1;	
	}

	std::cout << "Attempting to open grammar files...\n";

	std::string filename1 = argv[1];
	std::string filename2 = argv[2];

	std::ifstream inFile1(filename1);
	std::ifstream inFile2(filename2);
	
	
	// error if program can't read one of the files.
	if (!inFile1)
	{
		std::cerr << "Error: Could not open file '" << filename1 << "'" << std::endl;
		return 1;
	}
	// put the entire file contents into a string and create the parser
	std::string input1 { std::istreambuf_iterator<char>(inFile1), std::istreambuf_iterator<char>() };

	std::cout << filename1 << " opened successfully!\n";

	if (!inFile2)
	{
		std::cerr << "Error: Could not open file '" << filename2 << "'" << std::endl;
		return 1;
	}

	std::string input2 { std::istreambuf_iterator<char>(inFile2), std::istreambuf_iterator<char>() };

	std::cout << filename2 << " opened successfully!\n";

	Parser parser1{input1};
	Parser parser2{input2};

	std::cout << "Parsing grammar 1...\n";

	Grammar grammar1 = parser1.parseGrammar();

	std::cout << "Grammar 1 parsed successfully!\n";
	std::cout << "Parsing grammar 2...\n";

	Grammar grammar2 = parser2.parseGrammar();

	std::cout << "Grammar 2 parsed successfully!\n";

	// convert the grammars to Chomsky normal form.
	std::cout << "Converting grammar 1 into Chomsky Normal Form...\n";
	CNF(grammar1);
	std::cout << "Grammar 1 converted successfully!\n";
	std::cout << "Converting grammar 2 into Chomsky Normal Form...\n";
	CNF(grammar2);
	std::cout << "Grammar 2 converted successfully!\n";

	testGrammars(grammar1, grammar2);

	return 0;
}
