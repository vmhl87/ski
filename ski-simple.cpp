#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <chrono>

bool is_alpha(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_numeric(char c) {
	return '0' <= c && c <= '9';
}

bool is_alphanumeric(char c) {
	return is_alpha(c) || is_numeric(c) || c == '_';
}

enum symbol_t {
	None,
	S,
	K,
	I,
	Integer,
	Plus,
	Minus,
	Times,
	Divide,
	Mod,
	Less,
	LessEq,
	Equal,
	NotEqual,
	GreaterEq,
	Greater,
	END,
};

std::string print(symbol_t s) {
	if (s == S) return "S";
	else if (s == K) return "K";
	else if (s == I) return "I";
	else if (s == Plus) return "[+]";
	else if (s == Minus) return "[-]";
	else if (s == Times) return "[*]";
	else if (s == Divide) return "[/]";
	else if (s == Mod) return "[%]";
	else if (s == Less) return "[<]";
	else if (s == LessEq) return "[<=]";
	else if (s == Equal) return "[==]";
	else if (s == NotEqual) return "[!=]";
	else if (s == GreaterEq) return "[>=]";
	else if (s == Greater) return "[>]";
	else return "{??}";
}

union data_t {
	int Integer;
};

struct node {
	node *left = NULL;
	node *right = NULL;
	symbol_t s;
	data_t d;
	size_t refcount;

	node() {}

	node(symbol_t sym) {
		refcount = 1;
		s = sym;
	}

	node(symbol_t sym, data_t dat) {
		refcount = 1;
		s = sym;
		d = dat;
	}

	node(int i) {
		refcount = 1;
		s = Integer;
		d.Integer = i;
	}

	node(bool b) {
		refcount = 1;
		if (b) {
			s = K;
		} else {
			s = None;
			left = new node(S);
			right = new node(K);
		}
	}

	node(node *l, node *r) {
		refcount = 1;
		left = l;
		right = r;
		s = None;
	}
};

typedef node* node_t;

node_t copy(node_t n) {
	if (n == NULL) return NULL;
	node_t ret = new node();
	ret->left = n->left;
	ret->right = n->right;
	ret->s = n->s;
	ret->d = n->d;
	ret->refcount = 1;
	return ret;
}

bool match(std::string str, size_t i, std::string pattern) {
	if (str.length() < pattern.length() + i) return false;

	for (size_t j=0; j<pattern.length(); j++)
		if (str[i+j] != pattern[j]) return false;

	return true;
}

bool match_2(std::string str, size_t i, std::string pattern) {
	return match(str, i, pattern + ' ') ||
		match(str, i, pattern + '(') ||
		match(str, i, pattern + ')');
}

bool match_int(std::string str, size_t i, int *res) {
	if (str[i] != '[') return false;

	size_t start = i+1;
	int sign = 1, base = 0;

	if (str[start] == '-') sign = -1, ++start;

	while (start < str.length() && is_numeric(str[start])) {
		base = 10*base + str[start++]-'0';
	}

	if (start >= str.length() || str[start] != ']') return false;

	*res = sign*base;

	return true;
}

node_t ski_from_str(std::string str) {
	std::vector<node_t> st;
	node_t root = NULL;

	size_t i = 0;

	while (i < str.length()) {
		if (str[i] == ' ') continue;
		else if (str[i] == '(') {
			i++;
			if (root != NULL) st.push_back(root);
			root = NULL;

		} else if (str[i] == ')') {
			i++;
			if (st.size()) {
				if (root == NULL) return NULL;
				node *right = root;
				root = new node(st.back(), right);
				st.pop_back();
			}

#define PARSE_COMBINATOR(sym) \
		} else if (str[i] == (#sym[0])) { \
			i++; \
			node *next = new node(sym); \
			if (root == NULL) root = next; \
			else { \
				node *left = root; \
				root = new node(left, next); \
			}

		PARSE_COMBINATOR(S)
		PARSE_COMBINATOR(K)
		PARSE_COMBINATOR(I)

#undef PARSE_COMBINATOR

		} else if (str[i] == '[') {
			if (match(str, i, "[True]")) {
				i += 6;
				node *next = new node(true);
				if (root == NULL) root = next;
				else {
					node *left = root;
					root = new node(left, next);
				}

			} else if (match(str, i, "[False]")) {
				i += 7;
				node *next = new node(false);
				if (root == NULL) root = next;
				else {
					node *left = root;
					root = new node(left, next);
				}

			} else {
				symbol_t sym = None;
				data_t dat;

				if (match(str, i, "[+]")) sym = Plus, i+=3;
				else if (match(str, i, "[-]")) sym = Minus, i+=3;
				else if (match(str, i, "[*]")) sym = Times, i+=3;
				else if (match(str, i, "[/]")) sym = Divide, i+=3;
				else if (match(str, i, "[%]")) sym = Mod, i+=3;
				else if (match(str, i, "[<]")) sym = Less, i+=3;
				else if (match(str, i, "[<=]")) sym = LessEq, i+=4;
				else if (match(str, i, "[==]")) sym = Equal, i+=4;
				else if (match(str, i, "[!=]")) sym = NotEqual, i+=4;
				else if (match(str, i, "[>=]")) sym = Greater, i+=4;
				else if (match(str, i, "[>]")) sym = GreaterEq, i+=3;
				else if (match_int(str, i, &dat.Integer)) {
					sym = Integer;
					i++;
					while (str[i] != ']') i++;
					i++;
				} else {
					i++;
					continue;
				}

				node *next = new node(sym, dat);

				if (root == NULL) root = next;
				else {
					node *left = root;
					root = new node(left, next);
				}
			}
		}
	}

	if (st.size()) return NULL;

	return root;
}

std::string print(node_t n);

std::string print_right(node_t n) {
	if (n->s == None) return "(" + print(n) + ")";
	else return print(n);
}

std::string print(node_t n) {
	if (n == NULL) return "()";
	else if (n->s == None) return print(n->left) + print_right(n->right);
	else if (n->s == Integer) return "[" + std::to_string(n->d.Integer) + "]";
	else return print(n->s);
}

std::string print_long(node_t n) {
	if (n == NULL) return "()";
	else if (n->s == None)
		return "(" + print_long(n->left) + print_long(n->right) + ")";
	else return print(n);
}

node_t clone(node_t n) {
	if (n == NULL) return NULL;
	if (n->s == None) return new node(clone(n->left), clone(n->right));
	return copy(n);
}

void cleanup_once(node_t n) {
	n->refcount--;
	if (n->refcount == 0) free(n);
}

void cleanup(node_t n) {
	if (n == NULL) return;
	if (n->s == None) {
		n->refcount--;
		if (n->refcount == 0) cleanup(n->left), cleanup(n->right), free(n);
	} else cleanup_once(n);
}

node_t inner_reduce(node_t n) {
	if (n->refcount > 1) {
		n->refcount--;
		if (n->s == None) n->left->refcount++, n->right->refcount++;
		n = copy(n);
	}

	if (n->s == None) {
		if (n->left->refcount > 1) {
			n->left->refcount--;
			if (n->left->s == None)
				n->left->left->refcount++, n->left->right->refcount++;
			n->left = copy(n->left);
		}

		if (n->left->s == None) {
			if (n->left->left->refcount > 1) {
				n->left->left->refcount--;
				if (n->left->left->s == None)
					n->left->left->left->refcount++,
						n->left->left->right->refcount++;
				n->left->left = copy(n->left->left);
			}
		}
	}

	if (n->s != None) return n;

	n->left = inner_reduce(n->left);

	if (n->left->s == I) {
		node_t ret = n->right;
		cleanup_once(n->left);
		cleanup_once(n);
		return inner_reduce(ret);
	}

	if (n->left->s == None && n->left->left->s == K) {
		node_t ret = n->left->right;
		cleanup(n->right);
		cleanup_once(n->left->left);
		cleanup_once(n->left);
		cleanup_once(n);
		return inner_reduce(ret);
	}

	if (n->left->s == None && n->left->left->s == None &&
			n->left->left->left->s == S) {
		node_t x = n->left->left->right;
		node_t y = n->left->right;
		node_t z = n->right;
		cleanup_once(n->left->left->left);
		cleanup_once(n->left->left);
		cleanup_once(n->left);
		cleanup_once(n);
		z->refcount++;
		return inner_reduce(new node(new node(x, z), new node(y, z)));
	}

#define BIOP_INT(name, op) \
	if (n->left->s == None && n->left->left->s == name) { \
		n->left->right = inner_reduce(n->left->right); \
		n->right = inner_reduce(n->right); \
		if (n->left->right->s == Integer && n->right->s == Integer) { \
			int res = n->left->right->d.Integer op n->right->d.Integer; \
			cleanup_once(n); \
			return new node(res); \
		} \
	}

	BIOP_INT(Plus, +)
	BIOP_INT(Minus, -)
	BIOP_INT(Times, *)
	BIOP_INT(Divide, /)
	BIOP_INT(Mod, %)

#undef BIOP_INT

#define BIOP_INT(name, op) \
	if (n->left->s == None && n->left->left->s == name) { \
		n->left->right = inner_reduce(n->left->right); \
		n->right = inner_reduce(n->right); \
		if (n->left->right->s == Integer && n->right->s == Integer) { \
			bool res = n->left->right->d.Integer op n->right->d.Integer; \
			cleanup_once(n); \
			return new node(res); \
		} \
	}

	BIOP_INT(Less, <)
	BIOP_INT(LessEq, <=)
	BIOP_INT(Equal, ==)
	BIOP_INT(NotEqual, !=)
	BIOP_INT(GreaterEq, >=)
	BIOP_INT(Greater, >)

#undef BIOP_INT

	return n;
}

node_t reduce(node_t n) {
	if (n == NULL) return NULL;
	return inner_reduce(n);
}

enum lambda_term_type {
	Var,
	Int,
	Comb,
	Function,
	Application,
};

std::vector<std::string> string_comp_1;
std::map<std::string, size_t> string_comp_2;
typedef size_t string_index_t;

string_index_t index_of(std::string str) {
	if (string_comp_2.find(str) == string_comp_2.end()) {
		string_comp_2[str] = string_comp_1.size();
		string_comp_1.push_back(str);
	}

	return string_comp_2[str];
}

std::string at_index(string_index_t i) {
	return string_comp_1[i];
}

struct lambda_term_data {
	string_index_t Var;
	int Int;
	symbol_t Comb;
};

struct term {
	lambda_term_type t;
	term *left, *right;
	lambda_term_data d;

	term(std::string str) {
		t = Var;
		left = NULL;
		right = NULL;
		d.Var = index_of(str);
	}

	term(string_index_t str) {
		t = Var;
		left = NULL;
		right = NULL;
		d.Var = str;
	}

	term(int i) {
		t = Int;
		left = NULL;
		right = NULL;
		d.Int = i;
	}

	term(symbol_t s) {
		t = Comb;
		left = NULL;
		right = NULL;
		d.Comb = s;
	}

	term(std::string arg, term *body) {
		t = Function;
		left = new term(arg);
		right = body;
	}

	term(string_index_t arg, term *body) {
		t = Function;
		left = new term(arg);
		right = body;
	}

	term(term *a, term *b) {
		t = Application;
		left = a;
		right = b;
	}
};

typedef term* term_t;

std::string print(term_t n);

std::string print_right(term_t n) {
	if (n->t == Application) return "(" + print(n) + ")";
	else return print(n);
}

std::string print(term_t n) {
	if (n == NULL) return "()";
	else if (n->t == Function)
		return "(L" + print(n->left) + "." + print(n->right) + ")";
	else if (n->t == Var) return at_index(n->d.Var);
	else if (n->t == Int) return std::to_string(n->d.Int);
	else if (n->t == Comb) return print(n->d.Comb);
	else if (n->t == Application)
		return print(n->left) + " " + print_right(n->right);
	else return "{??}";
}

term_t clone(term_t n) {
	if (n == NULL) return NULL;
	if (n->t == Var) return new term(n->d.Var);
	if (n->t == Int) return new term(n->d.Int);
	if (n->t == Comb) return new term(n->d.Comb);
	if (n->t == Application) {
		term_t l = clone(n->left);
		term_t r = clone(n->right);
		if (l == NULL || r == NULL) return NULL;
		return new term(l, r);
	}
	if (n->t == Function) {
		if (n->left == NULL || n->left->t != Var) return NULL;
		string_index_t idx = n->left->d.Var;
		term_t r = clone(n->right);
		if (r == NULL) return NULL;
		return new term(at_index(idx), r);
	}
	return NULL;
}

void cleanup(term_t n) {
	if (n == NULL) return;
	if (n->t == Var || n->t == Int || n->t == Comb) free(n);
	else cleanup(n->left), cleanup(n->right), free(n);
}

enum lambda_parse_node_t {
	StrNode,
	IntNode,
	ArgNode,
	ParenNode,
};

struct lambda_parse_node_data {
	std::string Str;
	std::string Arg;
	int Int;
};

struct lambda_parse_node {
	std::vector<lambda_parse_node*> nodes;
	lambda_parse_node_t type;
	lambda_parse_node_data content;

	lambda_parse_node() {
		type = ParenNode;
	}

	lambda_parse_node(std::string str) {
		content.Str = str;
		type = StrNode;
	}

	lambda_parse_node(std::string str, int x) {
		content.Arg = str;
		type = ArgNode;
	}

	lambda_parse_node(int i) {
		content.Int = i;
		type = IntNode;
	}

	void add_node(lambda_parse_node *node) {
		nodes.push_back(node);
	}
};

void cleanup(lambda_parse_node *n) {
	if (n == NULL) return;
	for (lambda_parse_node *m : n->nodes) cleanup(m);
	free(n);
}

term_t parse(lambda_parse_node *n) {
	if (n->type == ParenNode) {
		term_t root = NULL;

		if (n->nodes.size() == 0) return NULL;
		if (n->nodes[0]->type == ArgNode) {
			if (n->nodes.size() == 1) return NULL;
			std::string name = n->nodes[0]->content.Arg;
			n->nodes.erase(n->nodes.begin());
			if (n->nodes[0]->type == ArgNode) return NULL;
			term_t res = parse(n);
			if (res == NULL) return NULL;
			return new term(name, res);
		}

		for (lambda_parse_node *m : n->nodes) {
			if (m->type != StrNode && m->type != IntNode &&
					m->type != ParenNode) return NULL;
			term_t res = parse(m);
			if (res == NULL) return NULL;
			if (root == NULL) root = res;
			else root = new term(root, res);
		}

		return root;

	} else if (n->type == StrNode) {
		for (int i=0; i<static_cast<int>(END); i++) {
			symbol_t I = static_cast<symbol_t>(i);
			if (n->content.Str == print(I)) {
				return new term(I);
			}
		}

		return new term(n->content.Str);

	} else if (n->type == IntNode) {
		return new term(n->content.Int);

	} else return NULL;
}

term_t lambda_from_str(std::string Str) {
	std::vector<lambda_parse_node*> st;
	lambda_parse_node *root = new lambda_parse_node();
	std::string str = Str + ' ';

	size_t i=0;
	while (i < str.length()) {
		char c = str[i];

		if (c == '(') {
			st.push_back(root);
			root = new lambda_parse_node();
			i++;

		} else if (c == ')') {
			if (st.size() == 0) return NULL;
			if (root->nodes.size() == 0) return NULL;
			lambda_parse_node *old = root;
			root = st.back();
			st.pop_back();
			root->add_node(old);
			i++;

		} else if (c == 'L') {
			size_t j = i+2;
			if (i+1 >= str.length() || !is_alpha(str[i+1])) return NULL;
			while (j < str.length() && is_alphanumeric(str[j])) j++;
			if (j >= str.length() || str[j] != '.') return NULL;
			root->add_node(new lambda_parse_node(str.substr(i+1, j-i-1), 0));
			i = j+1;

		} else if (c == '-' && i+1 < str.length() && is_numeric(str[i+1])) {
			size_t j = i+1;
			int base = 0;
			while (j < str.length() && is_numeric(str[j]))
				base = 10*base + str[j]-'0', j++;
			if (j >= str.length() ||
					(str[j] != ' ' && str[j] != ')' && str[j] != '('))
				return NULL;
			root->add_node(new lambda_parse_node(-base));
			i = j;

		} else if (is_numeric(c)) {
			size_t j = i;
			int base = 0;
			while (j < str.length() && is_numeric(str[j]))
				base = 10*base + str[j]-'0', j++;
			if (j >= str.length() ||
					(str[j] != ' ' && str[j] != ')' && str[j] != '('))
				return NULL;
			root->add_node(new lambda_parse_node(base));
			i = j;

		} else {
			bool is_combinator = false;

			for (int j=0; j<static_cast<int>(END); j++) {
				std::string J = print(static_cast<symbol_t>(j));
				if (match_2(str, i, J)) {
					root->add_node(new lambda_parse_node(J));
					i += J.length();
					is_combinator = true;
					break;
				}
			}

			if (!is_combinator) {
				if (is_alpha(c)) {
					size_t j = i+1;
					while (j < str.length() && is_alphanumeric(str[j])) j++;
					if (j >= str.length() ||
							(str[j] != ' ' && str[j] != ')' && str[j] != '('))
						return NULL;
					root->add_node(new lambda_parse_node(str.substr(i, j-i)));
					i = j;

				} else i++;
			}
		}
	}

	if (st.size()) return NULL;

	term_t res = parse(root);
	cleanup(root);
	return res;
}

bool uses_var(term_t t, string_index_t var) {
	if (t == NULL) return false;
	if (t->t == Int) return false;
	if (t->t == Comb) return false;
	if (t->t == Var) return t->d.Var == var;
	if (t->t == Function) return uses_var(t->right, var);
	if (t->t == Application)
		return uses_var(t->left, var) || uses_var(t->right, var);
	return false;
}

term_t inner_reduce(term_t t) {
	if (t->t == Function) {
		if (t->left == NULL || t->left->t != Var) return NULL;
		string_index_t arg = t->left->d.Var;
		if (t->right == NULL) return NULL;

		if (t->right->t == Function) {
			term_t body = inner_reduce(t->right);
			term_t head = new term(arg, body);
			term_t res = inner_reduce(head);
			cleanup(head);
			return res;
			
		} else if (t->right->t == Application) {
			term_t T = t->right;
			if (T->right->t == Var && T->right->d.Var == arg &&
					!uses_var(T->left, arg)) {
				return inner_reduce(T->left);

			} else {
				term_t l = NULL, r = NULL;

				if (uses_var(T->left, arg)) {
					term_t head = new term(arg, clone(T->left));
					l = inner_reduce(head);
					cleanup(head);

				} else l = new term(new term(K), inner_reduce(T->left));

				if (uses_var(T->right, arg)) {
					term_t head = new term(arg, clone(T->right));
					r = inner_reduce(head);
					cleanup(head);

				} else r = new term(new term(K), inner_reduce(T->right));

				if (l == NULL || r == NULL) return NULL;
				return new term(new term(new term(S), l), r);
			}

		} else if (t->right->t == Var) {
			if (t->right->d.Var == arg) return new term(I);
			else return new term(new term(K), new term(t->right->d.Var));

		} else if (t->right->t == Int) {
			return new term(new term(K), new term(t->right->d.Int));

		} else if (t->right->t == Comb) {
			return new term(new term(K), new term(t->right->d.Comb));

		} else return NULL;

	} else if(t->t == Application) {
		term_t l = inner_reduce(t->left);
		term_t r = inner_reduce(t->right);
		if (l == NULL || r == NULL) return NULL;
		return new term(l, r);

	} else if (t->t == Int) return new term(t->d.Int);
	else if (t->t == Comb) return new term(t->d.Comb);
	else if (t->t == Var) return new term(t->d.Var);
	return NULL;
}

node_t outer_reduce(term_t t) {
	if (t == NULL) return NULL;
	else if (t->t == Application) {
		node_t l = outer_reduce(t->left);
		node_t r = outer_reduce(t->right);
		if (l == NULL || r == NULL) return NULL;
		return new node(l, r);

	} else if (t->t == Comb) return new node(t->d.Comb);
	else if (t->t == Int) return new node(t->d.Int);
	else return NULL;
}

node_t reduce(term_t t) {
	if (t == NULL) return NULL;
	term_t T = inner_reduce(t);
	node_t res = outer_reduce(T);
	cleanup(T);
	return res;
}

void demo() {
	{
		node_t expr = ski_from_str("IK(KSS)");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr) << " -> " << print(expr_reduced) << '\n';
		cleanup(expr);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		node_t expr = ski_from_str("SKSK");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr) << " -> " << print(expr_reduced) << '\n';
		cleanup(expr);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		node_t expr = ski_from_str("SII(SK)");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr) << " -> " << print(expr_reduced) << '\n';
		cleanup(expr);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		node_t expr = ski_from_str("S(KI)I(SK)");
		node_t expr_partial = ski_from_str("KI(SK)(I(SK))");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr) << " -> " << print(expr_partial);
		std::cout << " -> " << print(expr_reduced) << '\n';
		cleanup(expr);
		cleanup(expr_partial);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		node_t expr = ski_from_str("[+][3][15]");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr) << " -> " << print(expr_reduced) << '\n';
		cleanup(expr);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		node_t expr = ski_from_str("([!=][3][5])[1][0]");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr) << " -> " << print(expr_reduced) << '\n';
		cleanup(expr);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		std::string str = "(La.[*] a a)";
		term_t expr = lambda_from_str(str);
		std::cout << '"' << str << '"' << " -> " << print(expr) << '\n';
		cleanup(expr);
	};

	std::cout << '\n';

	{
		std::string str = "(La.[*] a a) (a (Lb.b) c) d";
		term_t expr = lambda_from_str(str);
		std::cout << '"' << str << '"' << " -> " << print(expr) << '\n';
		cleanup(expr);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str("(Lx.[*] x x) 17");
		term_t expr_lam_r1 = lambda_from_str("S (Lx.[*] x) I 17");
		node_t expr = ski_from_str("S(S(K[*])I)I [17]");
		node_t expr_reduced = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  ";
		std::cout << print(expr_lam_r1) << " ->\n  ";
		std::cout << print(expr) << " -> ";
		std::cout << print(expr_reduced) << '\n';
		cleanup(expr_lam);
		cleanup(expr_lam_r1);
		cleanup(expr);
		cleanup(expr_reduced);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str("(Lx.(Ly.[+] y x)) 5 -3");
		node_t expr = reduce(expr_lam);
		node_t expr_r = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  " << print(expr);
		std::cout << " -> " << print(expr_r) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
		cleanup(expr_r);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str("(Lx.[*] x x) 5");
		node_t expr = reduce(expr_lam);
		node_t expr_r = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  " << print(expr);
		std::cout << " -> " << print(expr_r) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
		cleanup(expr_r);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str("(Lx.x (Ly.[*] y 5)) (Lx.x 14)");
		node_t expr = reduce(expr_lam);
		node_t expr_r = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  " << print(expr);
		std::cout << " -> " << print(expr_r) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
		cleanup(expr_r);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str("S (Lx.S (La.a) (K x)) [*] 1111");
		node_t expr = reduce(expr_lam);
		node_t expr_r = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  " << print(expr);
		std::cout << " -> " << print(expr_r) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
		cleanup(expr_r);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str(
			"(Lf.(Lx.f(Lv.x x v))(Lx.f(Lv.x x v))) "
			"(Li.(Lj.[<] j 2 (1) ([*] j (i ([-] j 1))))) "
			"8"
		);
		node_t expr = reduce(expr_lam);
		node_t expr_r = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  " << print(expr);
		std::cout << " -> " << print(expr_r) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
		cleanup(expr_r);
	};

	std::cout << '\n';

	{
		term_t expr_lam = lambda_from_str(
			"(Lf.(Lx.f(Lv.x x v))(Lx.f(Lv.x x v))) "
			"(Li.(Lj.[<] j 2 (1) ([+] (i ([-] j 1)) (i ([-] j 2))))) "
			"6"
		);
		node_t expr = reduce(expr_lam);
		node_t expr_r = reduce(clone(expr));
		std::cout << print(expr_lam) << " ->\n  " << print(expr);
		std::cout << " -> " << print(expr_r) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
		cleanup(expr_r);
	};
}

int main() {
/*
	// exec lambda calc
	std::string s; std::getline(std::cin, s);
	term_t expr_lam = lambda_from_str(s);
	node_t expr = reduce(reduce(expr_lam));
	std::cout << print(expr) << '\n';
*/

/*
	// lower lambda calc to SKI
	std::string s; std::getline(std::cin, s);
	term_t expr_lam = lambda_from_str(s);
	node_t expr = reduce(expr_lam);
	std::cout << print(expr) << '\n';
*/

/*
	// exec SKI
	std::string s; std::getline(std::cin, s);
	auto t1 = std::chrono::steady_clock::now();
	node_t expr = ski_from_str(s);
	auto t2 = std::chrono::steady_clock::now();
	expr = reduce(expr);
	auto t3 = std::chrono::steady_clock::now();
	std::cout << print(expr) << '\n';
	{
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		auto ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
		std::cout << "parse: " << ns << " (" << ms << " ms)\n";
	};
	{
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
		auto ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
		std::cout << "reduce: " << ns << " (" << ms << " ms)\n";
	};
	cleanup(expr);
	return 0;
*/

/*
	// reformat SKI
	std::string s; std::getline(std::cin, s);
	node_t expr = ski_from_str(s);
	std::cout << print(expr) << '\n';
*/

/*
	// exec lambda calc with timing
	std::string s; std::getline(std::cin, s);
	auto t1 = std::chrono::steady_clock::now();
	term_t expr = lambda_from_str(s);
	auto t2 = std::chrono::steady_clock::now();
	node_t expr_r = reduce(expr);
	auto t3 = std::chrono::steady_clock::now();
	expr_r = reduce(expr_r);
	auto t4 = std::chrono::steady_clock::now();
	std::cout << print(expr_r) << '\n';
	{
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		auto ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
		std::cout << "parse: " << ns << " (" << ms << " ms)\n";
	};
	{
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
		auto ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
		std::cout << "lower: " << ns << " (" << ms << " ms)\n";
	};
	{
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();
		auto ms = std::chrono::duration<double, std::milli>(t4 - t3).count();
		std::cout << "reduce: " << ns << " (" << ms << " ms)\n";
	};
	cleanup(expr);
	cleanup(expr_r);
	return 0;
*/

	while (true) {
		std::string s; std::getline(std::cin, s);
		if (s == "exit" || s == "") break;
		if (s == "demo") {
			demo();
			return 0;
		}
		term_t expr_lam = lambda_from_str(s);
		node_t expr = reduce(expr_lam);
		std::cout << print(expr) << " -> "; std::flush(std::cout);
		expr = reduce(expr);
		std::cout << print(expr) << '\n';
		cleanup(expr_lam);
		cleanup(expr);
	}
}
